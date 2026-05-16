#include "Library/HelsincyCrosshairRenderLibrary.h"

#include "HelsincyCrosshair.h"
#include "Debug/HelsincyCrosshairDebug.h"
#include "GameFramework/HUD.h"
#include "Library/HelsincySingleHitMarkerRenderCore.h"

#include "CanvasItem.h"
#include "Components/HelsincyCrosshairComponent.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Render/HelsincyShapeRenderer.h"
#include "Subsystems/HelsincyCrosshairManagerSubsystem.h"

namespace HelsincyCrosshairBridge
{
	/**
	 * Read-only accessor for the HUD canvas during the active draw frame.
	 * @note AHUD::Canvas is protected, so this helper exposes access from a derived scope
	 *       without doing unsafe object reinterprets.
	 */
	class FHelsincyHUDCanvasAccessor : public AHUD
	{
	public:
		/**
		 * Get the current frame canvas from a HUD instance.
		 * @param HUD The HUD that is currently drawing.
		 * @return The active canvas, or nullptr if the HUD is invalid or not in a draw phase.
		 */
		static UCanvas* GetCanvas(AHUD* HUD)
		{
			// Use a pointer-to-member so we can read AHUD::Canvas without reinterpreting the HUD object.
			UCanvas* AHUD::* CanvasMember = &FHelsincyHUDCanvasAccessor::Canvas;
			return HUD ? HUD->*CanvasMember : nullptr;
		}
	};

	constexpr float BridgeDPIScalingReferenceHeight = 1080.0f;
	constexpr float Diag = 0.70710678f;
	static const FVector2D HitMarkerDirs[4] =
	{
		FVector2D(-Diag, -Diag), FVector2D(Diag, -Diag),
		FVector2D(-Diag, Diag), FVector2D(Diag, Diag)
	};

	/**
	 * Per-frame render state for a single hit marker.
	 * @note This caches derived screen-space values so each draw path can reuse the same result.
	 */
	struct FHelsincyHitMarkerRenderState
	{
		FVector2D BasePosition = FVector2D::ZeroVector;
		FVector2D CalculatedGlobalOffset = FVector2D::ZeroVector;
		float CurrentNormalShakeIntensity = 0.f;
		FLinearColor Color = FLinearColor::White;
		float InnerOffset = 0.f;
		float Length = 0.f;
		float Thickness = 0.f;
		float Scale = 1.f;
		float NormalShakeValues[4] = {0.f, 0.f, 0.f, 0.f};
		float ImpactRotationDegrees = 0.0f;
		float ImpactScaleMultiplier = 1.0f;
		float ImpactArmLengthMultiplier = 1.0f;
		float ImpactMotionAlpha = 0.0f;
	};

	/**
	 * Compute the DPI scaling factor for the current canvas.
	 * @param Canvas The canvas being used for this draw call.
	 * @return A scale relative to 1080p, clamped to a minimum of 0.5. Returns 1.0 when Canvas is null.
	 */
	static float GetDPIScaling(const UCanvas* Canvas)
	{
		if (!Canvas || Canvas->ClipY <= 0.0f)
		{
			return 1.0f;
		}

		return FMath::Clamp(Canvas->ClipY / BridgeDPIScalingReferenceHeight, 0.5f, 4.0f);
	}

	/**
	 * Draw the center dot element.
	 * @param Canvas The canvas being used for this draw call.
	 * @param Profile The active crosshair profile.
	 * @param Center The target screen-space center for the dot.
	 * @param Scale DPI or resolution scaling factor.
	 */
	static void DrawCenterDot(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Center, float Scale)
	{
		if (!Canvas || !Profile.CenterDotConfig.bEnabled)
		{
			return;
		}

		const auto& DotCfg = Profile.CenterDotConfig;
		float Size = FMath::Max(DotCfg.Size * Scale, 1.0f);

		FLinearColor DotColor = DotCfg.Color;
		DotColor.A = DotCfg.Opacity;

		const FVector2D Pos = Center - FVector2D(Size * 0.5f, Size * 0.5f);
		FCanvasTileItem TileItem(Pos, FVector2D(Size, Size), DotColor);
		TileItem.Texture = GWhiteTexture;
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}

	/**
	 * Compute the final render state for a single hit marker instance.
	 * @param Config The hit marker configuration.
	 * @param Marker The active hit marker instance.
	 * @param Center The base screen-space center used to render the marker.
	 * @param Scale DPI or resolution scaling factor.
	 * @param OutState The computed per-frame render state.
	 */
	static void CalculateHitMarkerState(const FHelsincy_HitMarkerProfile& Config, const FHelsincy_ActiveHitMarker& Marker, FVector2D Center, float Scale, FHelsincyHitMarkerRenderState& OutState)
	{
		const float LifeTimeRatio = Marker.TotalDuration > KINDA_SMALL_NUMBER ? Marker.TimeRemaining / Marker.TotalDuration : 0.0f;
		const float AnimProgress = 1.0f - LifeTimeRatio;
		const float AnimAlpha = FMath::InterpEaseOut(0.0f, 1.0f, AnimProgress, 2.0f);

		float DecayFactor = 1.0f;
		if (Config.bShakeDecay)
		{
			DecayFactor = LifeTimeRatio * LifeTimeRatio;
		}

		OutState.BasePosition = Center;
		OutState.Scale = Scale;
		const float Age = FMath::Max(Marker.TotalDuration - Marker.TimeRemaining, 0.0f);
		if (Config.bUseImpactMotion)
		{
			const HelsincyHitMarkerShakeMath::FResolvedImpactMotion ImpactMotion =
				HelsincyHitMarkerShakeMath::ResolveImpactMotion(
					Config,
					Marker.ShakeDirection,
					Age,
					Marker.ImpactMotionEnergy,
					Marker.ImpactDamageScale,
					Marker.ImpactMotionSign,
					Marker.ShakePhase,
					Scale);
			OutState.CalculatedGlobalOffset = ImpactMotion.TranslationOffset;
			OutState.ImpactRotationDegrees = ImpactMotion.RotationDegrees;
			OutState.ImpactScaleMultiplier = ImpactMotion.ScaleMultiplier;
			OutState.ImpactArmLengthMultiplier = ImpactMotion.ArmLengthMultiplier;
			OutState.ImpactMotionAlpha = ImpactMotion.MotionAlpha;
		}
		else
		{
			OutState.CalculatedGlobalOffset = HelsincyHitMarkerShakeMath::ResolveDampedShakeOffset(
				Marker.ShakeDirection,
				Age,
				Config.ShakeIntensity * Scale * DecayFactor * Marker.ShakeEnergy,
				Config.ShakeFrequency,
				Config.ShakeDamping,
				Marker.ShakePhase);
			OutState.ImpactRotationDegrees = 0.0f;
			OutState.ImpactScaleMultiplier = 1.0f;
			OutState.ImpactArmLengthMultiplier = 1.0f;
			OutState.ImpactMotionAlpha = 0.0f;
		}
		const float NormalShakeWeight = Config.bUseImpactMotion ? 0.35f : 1.0f;
		OutState.CurrentNormalShakeIntensity = Config.NormalShakeIntensity * Scale * DecayFactor * Marker.ShakeEnergy * NormalShakeWeight;
		HelsincyHitMarkerShakeMath::ResolveNormalShakeValues(
			Marker.ShakeSeed,
			Age,
			Config.ShakeFrequency,
			Config.ShakeDamping,
			OutState.NormalShakeValues);

		OutState.Thickness = Config.Thickness * Scale * FMath::Lerp(1.0f, 1.12f, OutState.ImpactMotionAlpha);

		// --- 模式分支: 大小/位置/透明度计算 | Mode branch: size/position/opacity ---
		if (Config.Mode == EHitMarkerMode::SingleInstance)
		{
			// 单实例 COD 风格: 固定大小和位置, 仅脉冲 + 末尾淡出
			// SingleInstance COD-style: fixed size & position, pulse only + tail fade
			OutState.Length = Config.SingleInstanceSize * Marker.SizeScale * Scale * OutState.ImpactArmLengthMultiplier;
			OutState.InnerOffset = Config.SingleInstanceOffset * Scale * OutState.ImpactScaleMultiplier;

			OutState.Color = Marker.CurrentColor;
			if (Config.SingleInstanceFadeRatio > KINDA_SMALL_NUMBER)
			{
				const float FadeStart = 1.0f - Config.SingleInstanceFadeRatio;
				if (AnimProgress > FadeStart)
				{
					const float FadeAlpha = 1.0f - (AnimProgress - FadeStart) / Config.SingleInstanceFadeRatio;
					OutState.Color.A *= FadeAlpha;
				}
			}
		}
		else
		{
			// 多实例经典: 动态 StartSize→EndSize + StartOffset→EndOffset + 全程淡出
			// MultiInstance classic: animated size + offset + full fade
			const float AnimOffset = FMath::Lerp(Config.StartOffset, Config.EndOffset, AnimAlpha);
			OutState.InnerOffset = (Config.BaseDistance + AnimOffset) * Scale * OutState.ImpactScaleMultiplier;
			const float BaseSize = FMath::Lerp(Config.StartSize, Config.EndSize, AnimAlpha);
			OutState.Length = BaseSize * Marker.SizeScale * Scale * OutState.ImpactArmLengthMultiplier;

			OutState.Color = Marker.CurrentColor;
			const float CurrentOpacity = FMath::Lerp(1.0f, 0.0f, AnimProgress);
			OutState.Color.A *= CurrentOpacity;
		}
	}

	/**
	 * Draw a hit marker using a texture-based representation.
	 * @param Canvas The canvas being used for this draw call.
	 * @param Config The hit marker configuration, including the texture resource.
	 * @param State The precomputed render state for the hit marker.
	 */
	static void DrawHitMarkerImage(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FHelsincyHitMarkerRenderState& State)
	{
		if (!Canvas || !Config.CustomTexture || !Config.CustomTexture->GetResource())
		{
			return;
		}

		const FVector2D TexSize(State.Length, State.Length);
		for (int32 i = 0; i < 4; i++)
		{
			const FVector2D Dir = HelsincyHitMarkerShakeMath::RotateDirection(HitMarkerDirs[i], State.ImpactRotationDegrees);
			const FVector2D Pos = State.BasePosition + State.CalculatedGlobalOffset + (Dir * State.InnerOffset) - (TexSize * 0.5f);
			FCanvasTileItem TileItem(Pos, Config.CustomTexture->GetResource(), TexSize, State.Color);
			TileItem.Rotation = FRotator(0.0f, 0.0f, State.ImpactRotationDegrees);
			TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
			TileItem.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(TileItem);
		}
	}

	/**
	 * Draw a hit marker using line primitives.
	 * @param Canvas The canvas being used for this draw call.
	 * @param State The precomputed render state for the hit marker.
	 */
	static void DrawHitMarkerLinear(UCanvas* Canvas, const FHelsincyHitMarkerRenderState& State)
	{
		if (!Canvas)
		{
			return;
		}

		for (int32 i = 0; i < 4; i++)
		{
			const FVector2D Dir = HelsincyHitMarkerShakeMath::RotateDirection(HitMarkerDirs[i], State.ImpactRotationDegrees);
			const FVector2D RightDir(Dir.Y, -Dir.X);

			FVector2D CurrentArmCenter = State.BasePosition + State.CalculatedGlobalOffset;
			if (State.CurrentNormalShakeIntensity > 0.1f)
			{
				CurrentArmCenter += RightDir * (State.NormalShakeValues[i] * State.CurrentNormalShakeIntensity);
			}

			const FVector2D Start = CurrentArmCenter + (Dir * State.InnerOffset);
			const FVector2D End = Start + (Dir * State.Length);

			FCanvasLineItem LineItem(Start, End);
			LineItem.SetColor(State.Color);
			LineItem.LineThickness = State.Thickness;
			LineItem.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(LineItem);
		}
	}

	/**
	 * Draw a hit marker using a tapered triangular shape.
	 * @param Canvas The canvas being used for this draw call.
	 * @param Config The hit marker configuration for tapered rendering.
	 * @param State The precomputed render state for the hit marker.
	 * @param SmoothTexture Optional smooth line texture; falls back to the white texture when null.
	 */
	static void DrawHitMarkerTapered(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FHelsincyHitMarkerRenderState& State, FTexture* SmoothTexture)
	{
		if (!Canvas)
		{
			return;
		}

		FTexture* TexToUse = SmoothTexture ? SmoothTexture : GWhiteTexture;

		auto DrawPass = [&](float WidthScale, float AlphaScale, ESimpleElementBlendMode BlendMode)
		{
			FLinearColor FinalCol = State.Color;
			FinalCol.A *= AlphaScale;

			for (int32 i = 0; i < 4; i++)
			{
				const FVector2D Dir = HelsincyHitMarkerShakeMath::RotateDirection(HitMarkerDirs[i], State.ImpactRotationDegrees);
				const FVector2D RightDir(Dir.Y, -Dir.X);

				FVector2D CurrentArmCenter = State.BasePosition + State.CalculatedGlobalOffset;
				if (State.CurrentNormalShakeIntensity > 0.1f)
				{
					CurrentArmCenter += RightDir * (State.NormalShakeValues[i] * State.CurrentNormalShakeIntensity);
				}

				const FVector2D InnerBase = CurrentArmCenter + (Dir * State.InnerOffset);
				const FVector2D OuterTip = InnerBase + (Dir * State.Length);
				const float HalfWidth = (State.Thickness * WidthScale) * 0.5f;
				const FVector2D BaseLeft = InnerBase + (RightDir * HalfWidth);
				const FVector2D BaseRight = InnerBase - (RightDir * HalfWidth);

				FCanvasTriangleItem TriItem(OuterTip, BaseLeft, BaseRight, TexToUse);
				TriItem.TriangleList[0].V0_UV = FVector2D(1.0f, 0.5f);
				TriItem.TriangleList[0].V1_UV = FVector2D(0.0f, 0.0f);
				TriItem.TriangleList[0].V2_UV = FVector2D(0.0f, 1.0f);
				TriItem.TriangleList[0].V0_Color = FinalCol;
				TriItem.TriangleList[0].V1_Color = FinalCol;
				TriItem.TriangleList[0].V2_Color = FinalCol;
				TriItem.SetColor(FinalCol);
				TriItem.BlendMode = BlendMode;
				Canvas->DrawItem(TriItem);
			}
		};

		DrawPass(Config.TaperedShapeGlowWidthScale, Config.TaperedShapeGlowAlphaScale, SE_BLEND_Additive);
		DrawPass(1.0f, 1.0f, SE_BLEND_Translucent);
	}

	/**
	 * Resolve the smooth line texture used by hit marker rendering.
	 * @param Config The hit marker configuration.
	 * @param Subsystem Crosshair manager subsystem used to resolve optional resources.
	 * @return The smooth texture resource when available, otherwise the white texture.
	 */
	static FTexture* ResolveSmoothHitMarkerTexture(const FHelsincy_HitMarkerProfile& Config, UHelsincyCrosshairManagerSubsystem* Subsystem)
	{
		if (!Config.bUseTaperedShape || !Subsystem)
		{
			return GWhiteTexture;
		}

		if (UTexture2D* Tex = Subsystem->GetSmoothLineTexture())
		{
			return Tex->GetResource();
		}

		return GWhiteTexture;
	}

	/**
	 * Draw a hit marker after render-state resolution has already completed.
	 * @param Canvas The canvas being used for this draw call.
	 * @param Config The hit marker configuration.
	 * @param State The precomputed render state.
	 * @param SmoothTexResource Optional smooth line texture resource for tapered mode.
	 */
	static void DrawResolvedHitMarker(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FHelsincyHitMarkerRenderState& State, FTexture* SmoothTexResource)
	{
		if (Config.CustomTexture)
		{
			DrawHitMarkerImage(Canvas, Config, State);
		}
		else if (Config.bUseTaperedShape)
		{
			DrawHitMarkerTapered(Canvas, Config, State, SmoothTexResource);
		}
		else
		{
			DrawHitMarkerLinear(Canvas, State);
		}
	}

	/**
	 * Draw all active hit markers for the current frame.
	 * @param Canvas The canvas being used for this draw call.
	 * @param Profile The active crosshair profile.
	 * @param Markers All currently active hit marker instances.
	 * @param Center The base render center for hit markers.
	 * @param Scale DPI or resolution scaling factor.
	 * @param Subsystem Crosshair manager subsystem used to resolve optional resources.
	 */
	static void DrawHitMarkers(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, const TArray<FHelsincy_ActiveHitMarker>& Markers, FVector2D Center, float Scale, UHelsincyCrosshairManagerSubsystem* Subsystem)
	{
		const auto& Config = Profile.HitMarkerConfig;
		if (!Canvas || !Config.bEnabled || Markers.Num() == 0)
		{
			return;
		}

		FTexture* SmoothTexResource = ResolveSmoothHitMarkerTexture(Config, Subsystem);
		FHelsincyHitMarkerRenderState RenderState;
		for (const auto& Marker : Markers)
		{
			CalculateHitMarkerState(Config, Marker, Center, Scale, RenderState);
			DrawResolvedHitMarker(Canvas, Config, RenderState, SmoothTexResource);
		}
	}

}

bool UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(AHUD* HUD)
{
	if (!HUD)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] DrawCrosshairForHUD failed: HUD is NULL."));
		return false;
	}

	APlayerController* PC = HUD->GetOwningPlayerController();
	if (!PC)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] DrawCrosshairForHUD failed: PlayerController is NULL for HUD '%s'."),
			*GetNameSafe(HUD));
		return false;
	}

	UCanvas* Canvas = HelsincyCrosshairBridge::FHelsincyHUDCanvasAccessor::GetCanvas(HUD);
	if (!Canvas)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] DrawCrosshairForHUD failed: Canvas is NULL. Are you calling outside the draw phase?"));
		return false;
	}

	return DrawCrosshairForController(PC, Canvas);
}

bool UHelsincyCrosshairRenderLibrary::DrawCrosshairForController(APlayerController* PlayerController, UCanvas* Canvas)
{
	SCOPE_CYCLE_COUNTER(STAT_HC_BridgeDraw);

	if (!PlayerController || !Canvas)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] DrawCrosshairForController failed: PC=%s Canvas=%s"),
			PlayerController ? TEXT("Valid") : TEXT("NULL"),
			Canvas ? TEXT("Valid") : TEXT("NULL"));
		return false;
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] DrawCrosshairForController failed: Pawn is NULL for PC '%s'."),
			*GetNameSafe(PlayerController));
		return false;
	}

	UHelsincyCrosshairComponent* Comp = Pawn->FindComponentByClass<UHelsincyCrosshairComponent>();
	if (!Comp)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Warning,
			TEXT("[HC][Bridge] Component not found on Pawn '%s'. Did you add UHelsincyCrosshairComponent?"),
			*GetNameSafe(Pawn));
		return false;
	}
	if (!Comp->RefreshLocalPlayerGuardForRendering())
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] Crosshair local-player guard is not ready for Pawn '%s'."),
			*GetNameSafe(Pawn));
		return false;
	}
	if (!Comp->IsEnabledCrosshair())
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] Crosshair is disabled on Pawn '%s'."),
			*GetNameSafe(Pawn));
		return false;
	}

	UHelsincyCrosshairManagerSubsystem* Subsystem = GEngine ? GEngine->GetEngineSubsystem<UHelsincyCrosshairManagerSubsystem>() : nullptr;
	if (!Subsystem)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Warning,
			TEXT("[HC][Bridge] CrosshairManagerSubsystem not found. Is GEngine valid?"));
		return false;
	}

	const FHelsincyCrosshairProfile& Profile = Comp->GetCurrentProfile();
	UHelsincyShapeRenderer* Renderer = Subsystem->GetRenderer(Profile.ShapeTag);
	if (!Renderer)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][Bridge] Renderer not found for Tag '%s', trying fallback Cross."),
			*Profile.ShapeTag.ToString());
		Renderer = Subsystem->GetRenderer(FHelsincyCrosshair_Tags::Shape_Cross);
	}

	if (!Renderer)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Warning,
			TEXT("[HC][Bridge] Fallback Cross renderer also not found. No crosshair will be drawn."));
		return false;
	}

	const FHelsincy_CrosshairPresentationState& PresentationState = Comp->GetCrosshairPresentationState_CPP();
	const FVector2D Spread = Comp->GetFinalSpread();
	const float Scale = HelsincyCrosshairBridge::GetDPIScaling(Canvas);
	const FHelsincy_SingleHitMarkerState& SingleHitMarkerState = Comp->GetSingleHitMarkerState();
	const bool bAnyHitMarkerVisible = Profile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance
		? SingleHitMarkerState.bVisible
		: Comp->GetActiveHitMarkers().Num() > 0;
	const HelsincySingleHitMarkerRenderCore::FHitMarkerCrosshairVisibilityResult CrosshairVisibility =
		HelsincySingleHitMarkerRenderCore::ResolveCrosshairVisibilityWhileHitMarkerActive(Profile.HitMarkerConfig, bAnyHitMarkerVisible);
	FLinearColor CrosshairColor = PresentationState.CurrentColor;
	const FHelsincyCrosshairProfile CrosshairDrawProfile =
		HelsincySingleHitMarkerRenderCore::MakeBaseCrosshairDrawProfile(Profile, CrosshairVisibility);

	const FVector2D ScreenCenter(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
	const FVector2D CrosshairCenter = ScreenCenter + Profile.VisualsConfig.GlobalCrosshairOffset * Scale;
	const FVector2D PresentedCrosshairCenter = CrosshairCenter + (PresentationState.CurrentJitterOffset * Scale);
	const FVector2D PresentedSpread = Spread * PresentationState.CurrentScale;
	const float PresentedScale = Scale * PresentationState.CurrentScale;

	UWorld* World = PlayerController->GetWorld();
	const float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.f;
	if (CrosshairVisibility.ShouldDrawBaseCrosshair())
	{
		SCOPE_CYCLE_COUNTER(STAT_HC_RendererDraw);
		Renderer->Draw(Canvas, CrosshairDrawProfile, PresentedSpread, PresentedCrosshairCenter, CrosshairColor, DeltaSeconds, PresentedScale);
	}

	const FVector2D DotCenter = Profile.CenterDotConfig.bAlwaysStayCentered ? ScreenCenter : PresentedCrosshairCenter;
	if (CrosshairVisibility.ShouldDrawCenterDot())
	{
		if (FMath::IsNearlyEqual(CrosshairVisibility.CenterDotAlphaScale, 1.0f))
		{
			HelsincyCrosshairBridge::DrawCenterDot(Canvas, Profile, DotCenter, PresentedScale);
		}
		else
		{
			FHelsincyCrosshairProfile CenterDotProfile = Profile;
			CenterDotProfile.CenterDotConfig.Opacity *= CrosshairVisibility.CenterDotAlphaScale;
			if (CenterDotProfile.CenterDotConfig.Opacity > KINDA_SMALL_NUMBER)
			{
				HelsincyCrosshairBridge::DrawCenterDot(Canvas, CenterDotProfile, DotCenter, PresentedScale);
			}
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_HC_HitMarkerDraw);
		if (Profile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance)
		{
			const bool bSkipSingleHitMarkerDraw = SingleHitMarkerState.bVisible
				&& HelsincyCrosshairDebug::ShouldSkipSingleHitMarkerDraw(PlayerController, EHelsincyHitMarkerDrawPath::Bridge);
			if (!bSkipSingleHitMarkerDraw)
			{
				HelsincySingleHitMarkerRenderCore::DrawSingleHitMarker(
					Canvas,
					Profile,
					SingleHitMarkerState,
					CrosshairCenter,
					Scale,
					Subsystem
				);
			}
		}
		else
		{
			HelsincySingleHitMarkerRenderCore::DrawHitMarkers(Canvas, Profile, Comp->GetActiveHitMarkers(), CrosshairCenter, Scale, Subsystem);
		}
	}

	return true;
}

bool UHelsincyCrosshairRenderLibrary::IsCrosshairDebugEnabled()
{
	return HelsincyCrosshairDebug::IsEnabled();
}
