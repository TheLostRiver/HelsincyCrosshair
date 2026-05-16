// Copyright , Helsincy Games. All Rights Reserved.

#include "Library/HelsincyDamageIndicatorRenderLibrary.h"

#include "HelsincyDamageIndicator.h"
#include "Debug/HelsincyDamageIndicatorDebug.h"
#include "GameFramework/HUD.h"

#include "CanvasItem.h"
#include "Components/HelsincyDamageIndicatorComponent.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Library/HelsincyDamageIndicatorPlacementResolver.h"
#include "IndicatorRenderer/HelsincyIndicatorRenderer.h"
#include "Subsystems/HelsincyDamageIndicatorSubsystem.h"

namespace HelsincyDamageIndicatorBridge
{
	/**
	 * Read-only accessor for the HUD canvas during the active draw frame.
	 */
	class FHelsincyHUDCanvasAccessor : public AHUD
	{
	public:
		static UCanvas* GetCanvas(AHUD* HUD)
		{
			UCanvas* AHUD::* CanvasMember = &FHelsincyHUDCanvasAccessor::Canvas;
			return HUD ? HUD->*CanvasMember : nullptr;
		}
	};

	constexpr float BridgeDPIScalingReferenceHeight = 1080.0f;

	static float GetDPIScaling(UCanvas* Canvas)
	{
		if (!Canvas || Canvas->ClipY <= 0.0f) return 1.0f;
		return Canvas->ClipY / BridgeDPIScalingReferenceHeight;
	}

	/**
	 * Compute the current fade alpha for one damage indicator instance.
	 */
	static float CalculateIndicatorFadeAlpha(const FHelsincy_DamageIndicatorProfile& Config, const FHelsincy_ActiveDamageIndicator& Indicator)
	{
		if (Indicator.InitialDuration <= 0.0f)
		{
			return 1.0f;
		}

		float Alpha = 1.0f;
		const float TimeElapsed = Indicator.InitialDuration - Indicator.TimeRemaining;
		if (Config.FadeInTime > 0.0f && TimeElapsed < Config.FadeInTime)
		{
			Alpha = TimeElapsed / Config.FadeInTime;
		}
		else if (Config.FadeOutTime > 0.0f && Indicator.TimeRemaining < Config.FadeOutTime)
		{
			Alpha = Indicator.TimeRemaining / Config.FadeOutTime;
		}

		return FMath::Clamp(Alpha, 0.0f, 1.0f);
	}

	static bool ShouldDrawCircle(const FHelsincy_DamageIndicatorProfile& Profile)
	{
		if (!Profile.bShowCircle)
		{
			return false;
		}

		return Profile.PlacementMode == EHelsincyDamageIndicatorPlacementMode::RadialCircle
			|| !Profile.bHideCircleInWindowEdgeMode;
	}

	/**
	 * Draw all active damage direction indicators for the current frame.
	 */
	static void DrawDamageIndicators(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, const TArray<FHelsincy_ActiveDamageIndicator>& Indicators, FVector2D Center, float Scale, UHelsincyDamageIndicatorSubsystem* Subsystem)
	{
		SCOPE_CYCLE_COUNTER(STAT_HDI_DrawIndicators);

		if (!Canvas)
		{
			return;
		}

		if (!Profile.bEnabled || Indicators.Num() == 0)
		{
			return;
		}

		if (ShouldDrawCircle(Profile))
		{
			const float ScaledRadius = Profile.Radius * Scale;
			const float ScaledThickness = FMath::Max(1.0f, Profile.CircleThickness * Scale);
			const float FinalAlpha = Profile.CircleColor.A * Profile.CircleMaxOpacity;

			FLinearColor FinalCircleColor = Profile.CircleColor;
			// 仅修改 A 通道，不做 RGB 预乘 (SE_BLEND_Translucent 会自动处理)
			// Only modify A channel; skip RGB premultiply (SE_BLEND_Translucent handles it)
			FinalCircleColor.A = FinalAlpha;

			// 运行时强制 Clamp，防止编辑器外直接赋 0 导致除零 | Runtime clamp to prevent division by zero from direct assignment
			const int32 Segments = FMath::Clamp(Profile.CircleSegments, 8, 360);
			const float AngleStep = (PI * 2.0f) / static_cast<float>(Segments);
			FVector2D LastVertex = FVector2D::ZeroVector;

			UTexture2D* SmoothTexture = Subsystem ? Subsystem->GetSmoothLineTexture() : nullptr;
			FTexture* TexRes = SmoothTexture ? SmoothTexture->GetResource() : GWhiteTexture;
			if (!TexRes) TexRes = GWhiteTexture;

			for (int32 i = 0; i <= Segments; i++)
			{
				const float Angle = i * AngleStep;
				const FVector2D CurV = Center + FVector2D(FMath::Cos(Angle) * ScaledRadius, FMath::Sin(Angle) * ScaledRadius);
				if (i > 0)
				{
					const FVector2D P1 = LastVertex;
					const FVector2D P2 = CurV;
					const FVector2D Delta = P2 - P1;
					const float Dist = Delta.Size();
					if (Dist > 0.0f)
					{
						const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
						FCanvasTileItem Tile(P1, TexRes, FVector2D(Dist, ScaledThickness + 1.0f), FinalCircleColor);
						Tile.PivotPoint = FVector2D(0.0f, 0.5f);
						Tile.Rotation = FRotator(0.0f, AngleDeg, 0.0f);
						Tile.BlendMode = SE_BLEND_Translucent;
						Canvas->DrawItem(Tile);
					}
				}
				LastVertex = CurV;
			}
		}

		if (!Subsystem)
		{
			return;
		}

		UHelsincyIndicatorRenderer* Renderer = Subsystem->GetIndicatorRenderer(Profile.IndicatorStyleTag);
		if (!Renderer)
		{
			Renderer = Subsystem->GetIndicatorRenderer(FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow);
		}

		if (!Renderer)
		{
			return;
		}

		for (const auto& Indicator : Indicators)
		{
			const float FadeAlpha = CalculateIndicatorFadeAlpha(Profile, Indicator);
			if (FadeAlpha <= 0.0f)
			{
				continue;
			}

			const FVector2D CanvasSize(Canvas->ClipX, Canvas->ClipY);
			const FVector2D PointerSize = Renderer->GetDesiredPointerSize(Profile, Scale);
			const FHelsincy_ResolvedDamageIndicatorPlacement Placement =
				HelsincyDamageIndicatorPlacement::ResolveDamageIndicatorPlacement(
					Profile,
					CanvasSize,
					Center,
					Indicator.CurrentSmoothAngle,
					Scale,
					PointerSize
				);

			if (!Placement.bShouldDraw)
			{
				continue;
			}

			Renderer->DrawPointerResolved(Canvas, Profile, Placement, FadeAlpha, Scale);
		}
	}
}

bool UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(AHUD* HUD)
{
	if (!HUD)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DrawDamageIndicatorsForHUD failed: HUD is NULL."));
		return false;
	}

	APlayerController* PC = HUD->GetOwningPlayerController();
	if (!PC)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DrawDamageIndicatorsForHUD failed: PlayerController is NULL for HUD '%s'."),
			*GetNameSafe(HUD));
		return false;
	}

	UCanvas* Canvas = HelsincyDamageIndicatorBridge::FHelsincyHUDCanvasAccessor::GetCanvas(HUD);
	if (!Canvas)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DrawDamageIndicatorsForHUD failed: Canvas is NULL. Are you calling outside the draw phase?"));
		return false;
	}

	return DrawDamageIndicatorsForController(PC, Canvas);
}

bool UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForController(APlayerController* PlayerController, UCanvas* Canvas)
{
	SCOPE_CYCLE_COUNTER(STAT_HDI_BridgeDraw);

	if (!PlayerController || !Canvas)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DrawDamageIndicatorsForController failed: PC=%s Canvas=%s"),
			PlayerController ? TEXT("Valid") : TEXT("NULL"),
			Canvas ? TEXT("Valid") : TEXT("NULL"));
		return false;
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DrawDamageIndicatorsForController failed: Pawn is NULL for PC '%s'."),
			*GetNameSafe(PlayerController));
		return false;
	}

	UHelsincyDamageIndicatorComponent* Comp = Pawn->FindComponentByClass<UHelsincyDamageIndicatorComponent>();
	if (!Comp)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Bridge] Component not found on Pawn '%s'. Did you add UHelsincyDamageIndicatorComponent?"),
			*GetNameSafe(Pawn));
		return false;
	}
	if (!Comp->RefreshLocalPlayerGuardForRendering())
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DamageIndicator local-player guard is not ready for Pawn '%s'."),
			*GetNameSafe(Pawn));
		return false;
	}
	if (!Comp->IsEnabled())
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Verbose,
			TEXT("[DI][Bridge] DamageIndicator is disabled on Pawn '%s'."),
			*GetNameSafe(Pawn));
		return false;
	}

	UHelsincyDamageIndicatorSubsystem* Subsystem = GEngine ? GEngine->GetEngineSubsystem<UHelsincyDamageIndicatorSubsystem>() : nullptr;

	const FHelsincy_DamageIndicatorProfile& Profile = Comp->GetIndicatorProfile();
	const float Scale = HelsincyDamageIndicatorBridge::GetDPIScaling(Canvas);
	const FVector2D ScreenCenter(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	HelsincyDamageIndicatorBridge::DrawDamageIndicators(Canvas, Profile, Comp->GetActiveIndicators(), ScreenCenter, Scale, Subsystem);

	return true;
}

bool UHelsincyDamageIndicatorRenderLibrary::IsDamageIndicatorDebugEnabled()
{
	return HelsincyDamageIndicatorDebug::IsEnabled();
}
