// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "HUD/HelsincyHUD.h"
#include "HelsincyCrosshair.h"
#include "Debug/HelsincyCrosshairDebug.h"
#include "Library/HelsincySingleHitMarkerRenderCore.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Render/HelsincyShapeRenderer.h"
#include "GameFramework/PlayerController.h"
#include "Components/HelsincyCrosshairComponent.h"
#include "Library/HelsincyDamageIndicatorRenderLibrary.h"
#include "Subsystems/HelsincyCrosshairManagerSubsystem.h"
#if !UE_BUILD_SHIPPING
#include "Components/HelsincyDamageIndicatorComponent.h"
#include "IndicatorRenderer/HelsincyIndicatorRenderer.h"
#include "Library/HelsincyDamageIndicatorPlacementResolver.h"
#include "Subsystems/HelsincyDamageIndicatorSubsystem.h"
#include "Debug/HelsincyDamageIndicatorDebug.h"
#endif
#include "DrawDebugHelpers.h"


// 45度角方向常量 | 45-degree direction constants
const FVector2D AHelsincyHUD::HitMarkerDirs[4] = {
	FVector2D(-Diag, -Diag), FVector2D(Diag, -Diag),
	FVector2D(-Diag, Diag),  FVector2D(Diag, Diag)
};

// 几何可视化调试颜色 | Geometry visualization debug colors
const FLinearColor AHelsincyHUD::DebugColor_CenterRef    = FLinearColor(0.0f, 1.0f, 1.0f, 0.3f);
const FLinearColor AHelsincyHUD::DebugColor_SpreadBox    = FLinearColor(1.0f, 1.0f, 0.0f, 0.5f);
const FLinearColor AHelsincyHUD::DebugColor_DIDirectionRay = FLinearColor(1.0f, 0.5f, 0.0f, 0.6f);

void AHelsincyHUD::DrawHUD()
{
	Super::DrawHUD();
	SCOPE_CYCLE_COUNTER(STAT_HC_HUDDraw);

	const auto Comp {GetCrosshairComponent()};
	FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
	float Scale {GetDPIScaling()};
	FVector2D FinalSpread {FVector2D::ZeroVector};
	FVector2D CrosshairCenter {Center};

	if (!Comp)
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
			TEXT("[HC][HUD] DrawHUD: CrosshairComponent not found on Pawn '%s'."),
			*GetNameSafe(GetOwningPawn()));
	}
	else
	{
		const bool bLocalPlayerReady = Comp->RefreshLocalPlayerGuardForRendering();
		const auto& Profile {Comp->GetCurrentProfile()};
		const FHelsincy_CrosshairPresentationState& PresentationState = Comp->GetCrosshairPresentationState_CPP();
		FinalSpread = Comp->GetFinalSpread();
		CrosshairCenter = Center + Profile.VisualsConfig.GlobalCrosshairOffset * Scale;
		const FVector2D PresentedSpread = FinalSpread * PresentationState.CurrentScale;
		const float PresentedScale = Scale * PresentationState.CurrentScale;
		const FVector2D PresentedCrosshairCenter = CrosshairCenter + (PresentationState.CurrentJitterOffset * Scale);

		if (bLocalPlayerReady && Comp->IsEnabledCrosshair())
		{
			FLinearColor FinalColor {PresentationState.CurrentColor};
			const FHelsincy_SingleHitMarkerState& SingleHitMarkerState = Comp->GetSingleHitMarkerState();
			const bool bAnyHitMarkerVisible = Profile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance
				? SingleHitMarkerState.bVisible
				: Comp->GetActiveHitMarkers().Num() > 0;
			const HelsincySingleHitMarkerRenderCore::FHitMarkerCrosshairVisibilityResult CrosshairVisibility =
				HelsincySingleHitMarkerRenderCore::ResolveCrosshairVisibilityWhileHitMarkerActive(
					Profile.HitMarkerConfig,
					bAnyHitMarkerVisible);
			FLinearColor CrosshairColor = FinalColor;
			const FHelsincyCrosshairProfile CrosshairDrawProfile =
				HelsincySingleHitMarkerRenderCore::MakeBaseCrosshairDrawProfile(Profile, CrosshairVisibility);

			// 更新准心 | Update crosshair renderer
			if (CrosshairVisibility.ShouldDrawBaseCrosshair())
			{
				UpdateCrosshairRenderer(CrosshairDrawProfile, PresentedSpread, PresentedCrosshairCenter, CrosshairColor, PresentedScale);
			}

			// 绘制中心点 | Draw center dot
			if (Profile.CenterDotConfig.bEnabled && CrosshairVisibility.ShouldDrawCenterDot())
			{
				const FVector2D DotCenter = Profile.CenterDotConfig.bAlwaysStayCentered ? Center : PresentedCrosshairCenter;
				if (FMath::IsNearlyEqual(CrosshairVisibility.CenterDotAlphaScale, 1.0f))
				{
					DrawCenterDot(Profile, DotCenter, PresentedScale);
				}
				else
				{
					FHelsincyCrosshairProfile CenterDotProfile = Profile;
					CenterDotProfile.CenterDotConfig.Opacity *= CrosshairVisibility.CenterDotAlphaScale;
					if (CenterDotProfile.CenterDotConfig.Opacity > KINDA_SMALL_NUMBER)
					{
						DrawCenterDot(CenterDotProfile, DotCenter, PresentedScale);
					}
				}
			}

			// 绘制命中反馈 | Draw hit markers
			if (Profile.HitMarkerConfig.bEnabled)
			{
				if (Profile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance)
				{
					// 单实例模式: 从 SingleHitMarkerState 构造临时 Marker 复用渲染管线
					// Single-instance mode: use the shared single-instance render core
					const bool bSkipSingleHitMarkerDraw = SingleHitMarkerState.bVisible
						&& HelsincyCrosshairDebug::ShouldSkipSingleHitMarkerDraw(GetOwningPlayerController(), EHelsincyHitMarkerDrawPath::HUD);
					if (!bSkipSingleHitMarkerDraw)
					{
						HelsincySingleHitMarkerRenderCore::DrawSingleHitMarker(
							Canvas,
							Profile,
							SingleHitMarkerState,
							CrosshairCenter,
							Scale,
							GetCrosshairManagerSubsystem()
						);
					}
				}
				else
				{
					DrawHitMarkers_Refactor(Profile, Comp->GetActiveHitMarkers(), CrosshairCenter, Scale);
				}
			}
		}
	}

	UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(this);

	// === Phase C: Geometry Visualization (C1-C5) ===
#if !UE_BUILD_SHIPPING
	// C1 + C2: Crosshair geometry debug
	if (Comp && HelsincyCrosshairDebug::IsGeometryEnabled())
	{
		const auto& Profile {Comp->GetCurrentProfile()};

		// C1: Center reference crosshair (thin cyan lines across entire screen)
		{
			FCanvasLineItem HLine(FVector2D(0, Center.Y), FVector2D(Canvas->ClipX, Center.Y));
			HLine.SetColor(DebugColor_CenterRef);
			HLine.LineThickness = 1.0f;
			Canvas->DrawItem(HLine);

			FCanvasLineItem VLine(FVector2D(Center.X, 0), FVector2D(Center.X, Canvas->ClipY));
			VLine.SetColor(DebugColor_CenterRef);
			VLine.LineThickness = 1.0f;
			Canvas->DrawItem(VLine);
		}

		// C1: Spread range box (yellow rectangle around CrosshairCenter)
		{
			float SpX = FinalSpread.X * Scale;
			float SpY = FinalSpread.Y * Scale;
			if (SpX > 0.1f || SpY > 0.1f)
			{
				FVector2D TL = CrosshairCenter + FVector2D(-SpX, -SpY);
				FVector2D TR = CrosshairCenter + FVector2D( SpX, -SpY);
				FVector2D BL = CrosshairCenter + FVector2D(-SpX,  SpY);
				FVector2D BR = CrosshairCenter + FVector2D( SpX,  SpY);

				auto DrawBoxEdge = [&](FVector2D A, FVector2D B)
				{
					FCanvasLineItem L(A, B);
					L.SetColor(DebugColor_SpreadBox);
					L.LineThickness = 1.0f;
					Canvas->DrawItem(L);
				};
				DrawBoxEdge(TL, TR);
				DrawBoxEdge(TR, BR);
				DrawBoxEdge(BR, BL);
				DrawBoxEdge(BL, TL);
			}
		}

		// C2: Target detection ray (world space DrawDebugLine)
		if (Profile.VisualsConfig.bEnableTargetSwitching)
		{
			if (APlayerController* PC = GetOwningPlayerController())
			{
				if (APlayerCameraManager* CamMgr = PC->PlayerCameraManager)
				{
					if (UWorld* DebugWorld = GetWorld())
					{
						FVector CamLoc = CamMgr->GetCameraLocation();
						FVector CamFwd = CamMgr->GetCameraRotation().Vector();
						float MaxDist = Profile.VisualsConfig.MaxSwitchingDistance;
						float LifeTime = DebugWorld->GetDeltaSeconds() * 2.0f;
						DrawDebugLine(DebugWorld, CamLoc, CamLoc + CamFwd * MaxDist,
							FColor::Green, false, LifeTime);
					}
				}
			}
		}
	}

	// C3 + C4: Damage Indicator geometry debug
	if (HelsincyDamageIndicatorDebug::IsGeometryEnabled())
	{
		UHelsincyDamageIndicatorComponent* DIComp = nullptr;
		if (APawn* Pawn = GetOwningPawn())
		{
			DIComp = Pawn->FindComponentByClass<UHelsincyDamageIndicatorComponent>();
		}
		if (DIComp && DIComp->IsEnabled())
		{
			if (UWorld* DebugWorld = GetWorld())
			{
				const TArray<FHelsincy_ActiveDamageIndicator>& Indicators = DIComp->GetActiveIndicators();
				float LifeTime = DebugWorld->GetDeltaSeconds() * 2.0f;

				for (const FHelsincy_ActiveDamageIndicator& Ind : Indicators)
				{
					// C3: Direction ray on Canvas (screen center → indicator angle)
					float AngleRad = FMath::DegreesToRadians(Ind.CurrentSmoothAngle);
					FVector2D Dir(FMath::Sin(AngleRad), -FMath::Cos(AngleRad));
					float RayLen = 200.0f * Scale;

					FCanvasLineItem DirLine(Center, Center + Dir * RayLen);
					DirLine.SetColor(DebugColor_DIDirectionRay);
					DirLine.LineThickness = 2.0f;
					Canvas->DrawItem(DirLine);

					// C4: Source position sphere (world space)
					FVector SourceLoc = Ind.SourceActor.IsValid()
						? Ind.SourceActor->GetActorLocation()
						: Ind.SourceLocation;
					DrawDebugSphere(DebugWorld, SourceLoc, 30.0f, 12,
						FColor::Red, false, LifeTime);
				}
			}
		}
	}
#endif
}

void AHelsincyHUD::ShowDebugInfo(float& YL, float& YPos)
{
	Super::ShowDebugInfo(YL, YPos);

#if !UE_BUILD_SHIPPING
	if (!Canvas) return;

	if (ShouldDisplayDebug(FName(TEXT("Crosshair"))) && HelsincyCrosshairDebug::IsTextEnabled())
	{
		ShowDebugCrosshairPanel(YL, YPos);
	}

	if (ShouldDisplayDebug(FName(TEXT("DamageIndicator"))) && HelsincyDamageIndicatorDebug::IsTextEnabled())
	{
		ShowDebugDamageIndicatorPanel(YL, YPos);
	}
#endif
}

UHelsincyCrosshairComponent* AHelsincyHUD::GetCrosshairComponent()
{
	APawn* Pawn = GetOwningPawn();

	// Pawn 变更时重新查找 | Re-lookup when Pawn changes
	if (Pawn != CachedPawn.Get())
	{
		CachedPawn = Pawn;
		CrosshairComponent = Pawn ? Pawn->FindComponentByClass<UHelsincyCrosshairComponent>() : nullptr;
	}

	return CrosshairComponent.Get();
}

UHelsincyCrosshairManagerSubsystem* AHelsincyHUD::GetCrosshairManagerSubsystem()
{
	if (HCSubsystem) return HCSubsystem;
	
	if (GEngine)
	{
		HCSubsystem = GEngine->GetEngineSubsystem<UHelsincyCrosshairManagerSubsystem>();
	}
	return HCSubsystem;
}

#if !UE_BUILD_SHIPPING

void AHelsincyHUD::ShowDebugCrosshairPanel(float& YL, float& YPos)
{
	UFont* Font = GEngine->GetSmallFont();
	const float XPos = 4.0f;
	const float XIndent = 20.0f;

	auto DrawHeader = [&](const FString& Text)
	{
		Canvas->SetDrawColor(FColor::Yellow);
		Canvas->DrawText(Font, Text, XPos, YPos);
		YPos += YL;
	};

	auto DrawLine = [&](const FString& Text, const FColor& Color = FColor::White)
	{
		Canvas->SetDrawColor(Color);
		Canvas->DrawText(Font, Text, XIndent, YPos);
		YPos += YL;
	};

	DrawHeader(TEXT("=== HelsincyCrosshair Debug ==="));

	UHelsincyCrosshairComponent* Comp = GetCrosshairComponent();
	UHelsincyCrosshairManagerSubsystem* Sub = GetCrosshairManagerSubsystem();

	if (!Comp)
	{
		DrawLine(TEXT("CrosshairComponent: NOT FOUND"), FColor::Red);
		YPos += YL;
	}
	else
	{
		const FHelsincyCrosshairProfile& Profile = Comp->GetCurrentProfile();

		// --- [Profile] ---
		DrawHeader(TEXT("[Profile]"));
		{
			FString TagStr = Profile.ShapeTag.IsValid() ? Profile.ShapeTag.ToString() : TEXT("<Empty>");
			DrawLine(FString::Printf(TEXT("ShapeTag:     %s"), *TagStr),
				Profile.ShapeTag.IsValid() ? FColor::White : FColor::Red);

			if (CurrentCrosshairRenderer)
			{
				FString Status;
				FColor StatusColor = FColor::White;
				if (CurrentCrosshairRendererTag == Profile.ShapeTag)
				{
					Status = TEXT("REGISTERED");
				}
				else
				{
					Status = TEXT("FALLBACK");
					StatusColor = FColor(255, 165, 0);
				}
				DrawLine(FString::Printf(TEXT("Renderer:     %s (%s)"),
					*CurrentCrosshairRenderer->GetClass()->GetName(), *Status), StatusColor);
			}
			else
			{
				DrawLine(TEXT("Renderer:     MISSING"), FColor::Red);
			}

			DrawLine(FString::Printf(TEXT("DPI Scale:    %.3f"), GetDPIScaling()));

			DrawLine(FString::Printf(TEXT("Enabled:      %s"),
				Comp->IsEnabledCrosshair() ? TEXT("true") : TEXT("false")),
				Comp->IsEnabledCrosshair() ? FColor::White : FColor::Red);
		}

		// --- [Spread] ---
		DrawHeader(TEXT("[Spread]"));
		{
			FVector2D StateSpr = Comp->GetStateSpread();
			FVector2D RecoilSpr = Comp->GetRecoilSpread();
			FVector2D FinalSpr = Comp->GetFinalSpread();
			DrawLine(FString::Printf(TEXT("State:        (%.1f, %.1f)"), StateSpr.X, StateSpr.Y));
			DrawLine(FString::Printf(TEXT("Recoil:       (%.1f, %.1f)"), RecoilSpr.X, RecoilSpr.Y));
			DrawLine(FString::Printf(TEXT("Final:        (%.1f, %.1f)"), FinalSpr.X, FinalSpr.Y));
		}

		// --- [Color] ---
		DrawHeader(TEXT("[Color]"));
		{
			const FLinearColor& PrimaryCol = Profile.VisualsConfig.PrimaryColor;
			const FLinearColor& FinalCol = Comp->GetCurrentVisualPrimaryColor_CPP();
			DrawLine(FString::Printf(TEXT("Primary:      (%.2f, %.2f, %.2f, %.2f)"),
				PrimaryCol.R, PrimaryCol.G, PrimaryCol.B, PrimaryCol.A));
			DrawLine(FString::Printf(TEXT("Final:        (%.2f, %.2f, %.2f, %.2f)"),
				FinalCol.R, FinalCol.G, FinalCol.B, FinalCol.A));
		}

		// --- [Presentation] ---
		DrawHeader(TEXT("[Presentation]"));
		{
			const FHelsincy_CrosshairPresentationState& PresentationState = Comp->GetCrosshairPresentationState_CPP();
			DrawLine(FString::Printf(TEXT("Scale T/C:    %.3f / %.3f"),
				PresentationState.TargetScale,
				PresentationState.CurrentScale));
			DrawLine(FString::Printf(TEXT("Scale M/A:    %.3f / %.3f"),
				PresentationState.MovementContribution,
				PresentationState.AirborneContribution));
			DrawLine(FString::Printf(TEXT("Scale R/F:    %.3f / %.3f"),
				PresentationState.RecoilContribution,
				PresentationState.FirePulseStrength));
			DrawLine(FString::Printf(TEXT("TargetColor:  (%.2f, %.2f, %.2f, %.2f)"),
				PresentationState.TargetColor.R,
				PresentationState.TargetColor.G,
				PresentationState.TargetColor.B,
				PresentationState.TargetColor.A));
			DrawLine(FString::Printf(TEXT("CurrentColor: (%.2f, %.2f, %.2f, %.2f)"),
				PresentationState.CurrentColor.R,
				PresentationState.CurrentColor.G,
				PresentationState.CurrentColor.B,
				PresentationState.CurrentColor.A));
			DrawLine(FString::Printf(TEXT("ColorReason:  %s / Override=%s"),
				*PresentationState.ActiveColorReason.ToString(),
				PresentationState.bHasTargetColorOverride ? TEXT("true") : TEXT("false")));
			DrawLine(FString::Printf(TEXT("Jitter Off:   (%.2f, %.2f)"),
				PresentationState.CurrentJitterOffset.X,
				PresentationState.CurrentJitterOffset.Y));
			DrawLine(FString::Printf(TEXT("Jitter Str:   %.3f"),
				PresentationState.CurrentJitterStrength));
			DrawLine(TEXT("Shared Read:  HUD=true Bridge=true"), FColor::White);
		}

		// --- [Target Detection] ---
		DrawHeader(TEXT("[Target Detection]"));
		{
			ETeamAttitude::Type Att = Comp->GetCurrentTargetAttitude();
			const TCHAR* AttStr =
				(Att == ETeamAttitude::Hostile)  ? TEXT("Hostile") :
				(Att == ETeamAttitude::Friendly) ? TEXT("Friendly") :
				                                   TEXT("Neutral");
			DrawLine(FString::Printf(TEXT("Attitude:     %s"), AttStr));

			FGameplayTag TargetTag = Comp->GetCurrentTargetTag();
			DrawLine(FString::Printf(TEXT("TargetTag:    %s"),
				TargetTag.IsValid() ? *TargetTag.ToString() : TEXT("<None>")));
		}

		// --- [HitMarkers] ---
		DrawHeader(TEXT("[HitMarkers]"));
		{
			const FHelsincy_HitMarkerProfile& HitMarkerConfig = Profile.HitMarkerConfig;
			const TArray<FHelsincy_ActiveHitMarker>& Markers = Comp->GetActiveHitMarkers();
			const FHelsincy_SingleHitMarkerState& SingleState = Comp->GetSingleHitMarkerState();
			const float DebugScale = GetDPIScaling();
			const TCHAR* ModeStr = HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance ? TEXT("Single") : TEXT("Multi");
			const TCHAR* PhaseStr =
				(SingleState.Phase == EHelsincySingleHitMarkerPhase::ActiveHold) ? TEXT("ActiveHold") :
				(SingleState.Phase == EHelsincySingleHitMarkerPhase::TailFade) ? TEXT("TailFade") :
				                                                               TEXT("Hidden");
			const TCHAR* SinglePriStr =
				(SingleState.Priority == EHitMarkerPriority::High_Priority_Kill)   ? TEXT("Kill") :
				(SingleState.Priority == EHitMarkerPriority::Medium_Priority_Head) ? TEXT("Head") :
				                                                                     TEXT("Body");
			const auto ConfigModeToString = [](EHelsincySingleHitMarkerRenderMode InMode) -> const TCHAR*
			{
				return InMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer
					? TEXT("SpriteDualLayer")
					: TEXT("LegacyGeometry");
			};
			const auto ResolvedModeToString = [](HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode InMode) -> const TCHAR*
			{
				switch (InMode)
				{
				case HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::SpriteDualLayer:
					return TEXT("SpriteDualLayer");
				case HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::CoreOnly:
					return TEXT("CoreOnly");
				default:
					return TEXT("LegacyGeometry");
				}
			};
			const auto TextureStateToString = [](const UTexture2D* Texture) -> const TCHAR*
			{
				return (Texture && Texture->GetResource()) ? TEXT("VALID") : TEXT("MISSING");
			};

			DrawLine(FString::Printf(TEXT("Mode:         %s"), ModeStr));
			DrawLine(FString::Printf(TEXT("Single:       Visible=%s Phase=%s Priority=%s"),
				SingleState.bVisible ? TEXT("true") : TEXT("false"),
				PhaseStr,
				SinglePriStr));
			DrawLine(FString::Printf(TEXT("Single O/E/A: %.2f / %.2f / %.2f"),
				SingleState.Opacity,
				SingleState.ImpactEnergy,
				SingleState.AccentStrength));
			DrawLine(FString::Printf(TEXT("Shake S/E/A:  %d / %.2f / %.2f"),
				SingleState.ShakeSeed,
				SingleState.ShakeEnergy,
				SingleState.ShakeAge));
			DrawLine(FString::Printf(TEXT("Shake Dir:    (%.2f, %.2f)"),
				SingleState.ShakeDirection.X,
				SingleState.ShakeDirection.Y));
			DrawLine(FString::Printf(TEXT("Impact:       Dmg=%.2f Energy=%.2f Sign=%.0f"),
				SingleState.ImpactDamageScale,
				SingleState.ImpactMotionEnergy,
				SingleState.ImpactMotionSign));
			if (Profile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance)
			{
				DrawLine(FString::Printf(TEXT("Diag/Guard:   %s / %s"),
					HelsincyCrosshairDebug::IsHitMarkerDiagnosticsEnabled() ? TEXT("On") : TEXT("Off"),
					HelsincyCrosshairDebug::IsHitMarkerSoftGuardEnabled() ? TEXT("On") : TEXT("Off")));
				DrawLine(FString::Printf(TEXT("Draw Guard:   %s"),
					*HelsincyCrosshairDebug::GetSingleHitMarkerDrawGuardSummary(GetOwningPlayerController())));

				const HelsincySingleHitMarkerSpriteSupport::FResolvedSingleHitMarkerSpriteAssets ResolvedSpriteAssets =
					HelsincySingleHitMarkerSpriteSupport::ResolveSpriteAssets(HitMarkerConfig);
				const bool bSpriteRequested =
					HitMarkerConfig.SingleInstanceRenderMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
				const bool bSpriteFallbackToGeometry =
					bSpriteRequested
					&& ResolvedSpriteAssets.Mode == HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::LegacyGeometry;
				const bool bGlowMissingCoreOnly =
					bSpriteRequested
					&& ResolvedSpriteAssets.Mode == HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::CoreOnly;
				const float SafeSpacingScale = HitMarkerConfig.bUseSingleInstanceVisualSeparation
					? FMath::Max(HitMarkerConfig.SingleInstanceSafeSpacingScale, 1.0f)
					: 1.0f;
				const float InnerOffset = HitMarkerConfig.SingleInstanceOffset * SafeSpacingScale * DebugScale;
				const float Length = HitMarkerConfig.SingleInstanceSize * SingleState.GetEffectiveScale() * DebugScale;
				const float BaseSpriteDiameter = (InnerOffset + Length) * 2.0f;
				const float CoreSize = FMath::Max(BaseSpriteDiameter * HitMarkerConfig.SingleInstanceCoreScale, 1.0f);
				const float GlowSize = FMath::Max(BaseSpriteDiameter * HitMarkerConfig.SingleInstanceGlowScale, 1.0f);
				const float CoreAlpha = SingleState.CurrentColor.A * SingleState.Opacity;
				const float GlowAlpha = CoreAlpha * HitMarkerConfig.SingleInstanceGlowOpacityScale;
				const FColor BackendColor = bSpriteFallbackToGeometry
					? FColor(255, 165, 0)
					: FColor::White;
				const FColor TextureColor = !HitMarkerConfig.SingleInstanceCoreTexture
					? FColor::Red
					: (bGlowMissingCoreOnly ? FColor(255, 165, 0) : FColor::White);

				DrawLine(FString::Printf(TEXT("Render Cfg:   %s -> %s"),
					ConfigModeToString(HitMarkerConfig.SingleInstanceRenderMode),
					ResolvedModeToString(ResolvedSpriteAssets.Mode)),
					BackendColor);
				DrawLine(FString::Printf(TEXT("Sprite Tex:   Core=%s (%s)"),
					*GetNameSafe(HitMarkerConfig.SingleInstanceCoreTexture),
					TextureStateToString(HitMarkerConfig.SingleInstanceCoreTexture)),
					TextureColor);
				DrawLine(FString::Printf(TEXT("Sprite Tex2:  Glow=%s (%s)"),
					*GetNameSafe(HitMarkerConfig.SingleInstanceGlowTexture),
					TextureStateToString(HitMarkerConfig.SingleInstanceGlowTexture)),
					bGlowMissingCoreOnly ? FColor(255, 165, 0) : FColor::White);
				DrawLine(FString::Printf(TEXT("Sprite Tune:  CoreScale=%.2f GlowScale=%.2f GlowAlpha=%.2f"),
					HitMarkerConfig.SingleInstanceCoreScale,
					HitMarkerConfig.SingleInstanceGlowScale,
					HitMarkerConfig.SingleInstanceGlowOpacityScale));
				DrawLine(FString::Printf(TEXT("Sprite Real:  CoreSize=%.1f GlowSize=%.1f"),
					CoreSize,
					GlowSize));
				DrawLine(FString::Printf(TEXT("Sprite Alpha: Core=%.2f Glow=%.2f"),
					CoreAlpha,
					GlowAlpha));
			}

			DrawLine(FString::Printf(TEXT("Active Count: %d"), Markers.Num()));
			if (Markers.Num() > 0)
			{
				const FHelsincy_ActiveHitMarker& FirstMarker = Markers[0];
				DrawLine(FString::Printf(TEXT("ClassicShake: Seed=%d Energy=%.2f Dir=(%.2f, %.2f)"),
					FirstMarker.ShakeSeed,
					FirstMarker.ShakeEnergy,
					FirstMarker.ShakeDirection.X,
					FirstMarker.ShakeDirection.Y));
				DrawLine(FString::Printf(TEXT("ClassicImpact: Dmg=%.2f Energy=%.2f Sign=%.0f"),
					FirstMarker.ImpactDamageScale,
					FirstMarker.ImpactMotionEnergy,
					FirstMarker.ImpactMotionSign));
			}
			for (int32 i = 0; i < Markers.Num(); ++i)
			{
				const FHelsincy_ActiveHitMarker& M = Markers[i];
				const TCHAR* PriStr =
					(M.Priority == EHitMarkerPriority::High_Priority_Kill)   ? TEXT("Kill") :
					(M.Priority == EHitMarkerPriority::Medium_Priority_Head) ? TEXT("Head") :
					                                                            TEXT("Body");
				DrawLine(FString::Printf(
					TEXT("  [%d] Priority=%s  TimeLeft=%.2fs  Color=(%.1f,%.1f,%.1f,%.1f)"),
					i, PriStr, M.TimeRemaining,
					M.CurrentColor.R, M.CurrentColor.G, M.CurrentColor.B, M.CurrentColor.A));
			}
		}
	}

	// --- [Subsystem] ---
	DrawHeader(TEXT("[Subsystem]"));
	if (Sub)
	{
		TArray<FGameplayTag> RegisteredTags = Sub->GetRegisteredTags();
		DrawLine(FString::Printf(TEXT("Registered Renderers: %d"), RegisteredTags.Num()));
		FString TagList;
		for (const FGameplayTag& T : RegisteredTags)
		{
			if (!TagList.IsEmpty()) TagList += TEXT(", ");
			TagList += T.ToString();
		}
		if (!TagList.IsEmpty())
		{
			DrawLine(FString::Printf(TEXT("  %s"), *TagList));
		}
		DrawLine(FString::Printf(TEXT("SmoothTexture: %s"),
			Sub->GetSmoothLineTexture() ? TEXT("VALID") : TEXT("MISSING")),
			Sub->GetSmoothLineTexture() ? FColor::White : FColor::Red);
		DrawLine(FString::Printf(TEXT("Async Load:    %s"),
			Sub->IsAsyncLoading() ? TEXT("LOADING") : TEXT("IDLE")));
	}
	else
	{
		DrawLine(TEXT("Subsystem: NOT FOUND"), FColor::Red);
	}

	// --- [DamageIndicator Summary] (B8) ---
	DrawHeader(TEXT("[DamageIndicator Summary]"));
	{
		UHelsincyDamageIndicatorComponent* DIComp = nullptr;
		if (APawn* Pawn = GetOwningPawn())
		{
			DIComp = Pawn->FindComponentByClass<UHelsincyDamageIndicatorComponent>();
		}
		if (DIComp)
		{
			DrawLine(FString::Printf(TEXT("Active Count: %d"), DIComp->GetActiveIndicators().Num()));
		}
		else
		{
			DrawLine(TEXT("Component: NOT FOUND (optional)"), FColor(128, 128, 128));
		}
	}

	YPos += YL;
}

void AHelsincyHUD::ShowDebugDamageIndicatorPanel(float& YL, float& YPos)
{
	UFont* Font = GEngine->GetSmallFont();
	const float XPos = 4.0f;
	const float XIndent = 20.0f;

	auto DrawHeader = [&](const FString& Text)
	{
		Canvas->SetDrawColor(FColor::Yellow);
		Canvas->DrawText(Font, Text, XPos, YPos);
		YPos += YL;
	};

	auto DrawLine = [&](const FString& Text, const FColor& Color = FColor::White)
	{
		Canvas->SetDrawColor(Color);
		Canvas->DrawText(Font, Text, XIndent, YPos);
		YPos += YL;
	};

	DrawHeader(TEXT("=== HelsincyDamageIndicator Debug ==="));

	UHelsincyDamageIndicatorComponent* DIComp = nullptr;
	if (APawn* Pawn = GetOwningPawn())
	{
		DIComp = Pawn->FindComponentByClass<UHelsincyDamageIndicatorComponent>();
	}
	UHelsincyDamageIndicatorSubsystem* DISub = nullptr;
	if (GEngine)
	{
		DISub = GEngine->GetEngineSubsystem<UHelsincyDamageIndicatorSubsystem>();
	}

	if (!DIComp)
	{
		DrawLine(TEXT("DamageIndicatorComponent: NOT FOUND"), FColor::Red);
	}
	else
	{
		const FHelsincy_DamageIndicatorProfile& DIProfile = DIComp->GetIndicatorProfile();
		const float DamageIndicatorScale = (!Canvas || Canvas->ClipY <= 0.0f) ? 1.0f : Canvas->ClipY / 1080.0f;

		// --- [Profile] ---
		DrawHeader(TEXT("[Profile]"));
		{
			DrawLine(FString::Printf(TEXT("Enabled:      %s"),
				DIProfile.bEnabled ? TEXT("true") : TEXT("false")),
				DIProfile.bEnabled ? FColor::White : FColor::Red);

			FString StyleStr = DIProfile.IndicatorStyleTag.IsValid()
				? DIProfile.IndicatorStyleTag.ToString() : TEXT("<Empty>");
			DrawLine(FString::Printf(TEXT("StyleTag:     %s"), *StyleStr),
				DIProfile.IndicatorStyleTag.IsValid() ? FColor::White : FColor::Red);

			if (DISub)
			{
				bool bHasRenderer = DISub->GetIndicatorRenderer(DIProfile.IndicatorStyleTag) != nullptr;
				DrawLine(FString::Printf(TEXT("Renderer:     %s"),
					bHasRenderer ? TEXT("REGISTERED") : TEXT("MISSING")),
					bHasRenderer ? FColor::White : FColor::Red);
			}

			DrawLine(FString::Printf(TEXT("Duration:     %.1fs"), DIProfile.Duration));
			DrawLine(FString::Printf(TEXT("DPI Scale:    %.3f"), DamageIndicatorScale));
			const TCHAR* PlacementModeText =
				DIProfile.PlacementMode == EHelsincyDamageIndicatorPlacementMode::WindowEdge
					? TEXT("WindowEdge")
					: TEXT("RadialCircle");
			DrawLine(FString::Printf(TEXT("Placement:    %s"), PlacementModeText));
			DrawLine(FString::Printf(TEXT("CanvasSize:   %.0fx%.0f"), Canvas->ClipX, Canvas->ClipY));
			DrawLine(FString::Printf(TEXT("EdgeMargin:   %.1f"), DIProfile.EdgeMargin));
		}

		// --- [Active Indicators] ---
		DrawHeader(TEXT("[Active Indicators]"));
		{
			const TArray<FHelsincy_ActiveDamageIndicator>& Indicators = DIComp->GetActiveIndicators();
			DrawLine(FString::Printf(TEXT("Count:        %d"), Indicators.Num()));
			for (int32 i = 0; i < Indicators.Num(); ++i)
			{
				const FHelsincy_ActiveDamageIndicator& Ind = Indicators[i];
				FString SourceStr = Ind.SourceActor.IsValid()
					? GetNameSafe(Ind.SourceActor.Get())
					: TEXT("<Destroyed>");

				float Alpha = 1.0f;
				if (Ind.TimeRemaining < DIProfile.FadeOutTime && DIProfile.FadeOutTime > 0.0f)
				{
					Alpha = Ind.TimeRemaining / DIProfile.FadeOutTime;
				}
				else if ((Ind.InitialDuration - Ind.TimeRemaining) < DIProfile.FadeInTime && DIProfile.FadeInTime > 0.0f)
				{
					Alpha = (Ind.InitialDuration - Ind.TimeRemaining) / DIProfile.FadeInTime;
				}

				FString FadingStr = (Ind.TimeRemaining < DIProfile.FadeOutTime) ? TEXT(" (fading)") : TEXT("");
				DrawLine(FString::Printf(
					TEXT("  [%d] Source=%s  Angle=%.1fdeg  TimeLeft=%.1fs  Alpha=%.2f%s"),
					i, *SourceStr, Ind.CurrentSmoothAngle,
					Ind.TimeRemaining, Alpha, *FadingStr));
				if (i == 0 && DISub)
				{
					UHelsincyIndicatorRenderer* DebugRenderer = DISub->GetIndicatorRenderer(DIProfile.IndicatorStyleTag);
					if (!DebugRenderer)
					{
						const FGameplayTag ArrowStyleTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.Arrow"), false);
						DebugRenderer = DISub->GetIndicatorRenderer(ArrowStyleTag);
					}

					const FVector2D PointerSize = DebugRenderer
						? DebugRenderer->GetDesiredPointerSize(DIProfile, DamageIndicatorScale)
						: FVector2D::ZeroVector;
					const FHelsincy_ResolvedDamageIndicatorPlacement Placement =
						HelsincyDamageIndicatorPlacement::ResolveDamageIndicatorPlacement(
							DIProfile,
							FVector2D(Canvas->ClipX, Canvas->ClipY),
							FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f),
							Ind.CurrentSmoothAngle,
							DamageIndicatorScale,
							PointerSize
						);

					DrawLine(FString::Printf(
						TEXT("      Resolved=(%.1f, %.1f) Draw=%s"),
						Placement.Position.X,
						Placement.Position.Y,
						Placement.bShouldDraw ? TEXT("true") : TEXT("false")));
				}
			}
		}

		// --- [Circle] ---
		DrawHeader(TEXT("[Circle]"));
		{
			DrawLine(FString::Printf(TEXT("Show:         %s"),
				DIProfile.bShowCircle ? TEXT("true") : TEXT("false")));
			DrawLine(FString::Printf(TEXT("Radius:       %.1f"), DIProfile.Radius));
			DrawLine(FString::Printf(TEXT("Segments:     %d"), DIProfile.CircleSegments));
		}
	}

	// --- [Subsystem] ---
	DrawHeader(TEXT("[Subsystem]"));
	if (DISub)
	{
		TArray<FGameplayTag> RegisteredTags = DISub->GetRegisteredTags();
		DrawLine(FString::Printf(TEXT("Registered Renderers: %d"), RegisteredTags.Num()));
		FString TagList;
		for (const FGameplayTag& T : RegisteredTags)
		{
			if (!TagList.IsEmpty()) TagList += TEXT(", ");
			TagList += T.ToString();
		}
		if (!TagList.IsEmpty())
		{
			DrawLine(FString::Printf(TEXT("  %s"), *TagList));
		}
		DrawLine(FString::Printf(TEXT("SmoothTexture: %s"),
			DISub->GetSmoothLineTexture() ? TEXT("VALID") : TEXT("MISSING")),
			DISub->GetSmoothLineTexture() ? FColor::White : FColor::Red);
		DrawLine(FString::Printf(TEXT("Async Load:    %s"),
			DISub->IsAsyncLoading() ? TEXT("LOADING") : TEXT("IDLE")));
	}
	else
	{
		DrawLine(TEXT("Subsystem: NOT FOUND"), FColor::Red);
	}

	YPos += YL;
}

#endif

float AHelsincyHUD::GetDPIScaling() const
{
	if (!Canvas || Canvas->ClipY <= 0.0f) return 1.0f;
	return FMath::Clamp(Canvas->ClipY / DPIScalingReferenceHeight, 0.5f, DPIScalingMax);
}

void AHelsincyHUD::UpdateCrosshairRenderer(const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D CrosshairCenter, FLinearColor FinalColor, float Scale)
{
	SCOPE_CYCLE_COUNTER(STAT_HC_RendererDraw);

	UWorld* World = GetWorld();
	if (!World) return;
	const float DeltaSeconds = World->GetDeltaSeconds();

	if (CurrentCrosshairRendererTag == Profile.ShapeTag)
	{
		if (CurrentCrosshairRenderer)
		{
			CurrentCrosshairRenderer->Draw(Canvas, Profile, Spread, CrosshairCenter, FinalColor, DeltaSeconds, Scale);
		}
		return;
	}
	
	if (auto* Subsystem = GetCrosshairManagerSubsystem())
	{
		if (auto* Renderer = Subsystem->GetRenderer(Profile.ShapeTag))
		{
			CurrentCrosshairRendererTag = Profile.ShapeTag;
			CurrentCrosshairRenderer = Renderer;
			Renderer->Draw(Canvas, Profile, Spread, CrosshairCenter, FinalColor, DeltaSeconds, Scale);
		}
		else
		{
			// 只要 Subsystem 注册了 C++ 的 Cross 渲染器
			// As long as Subsystem has registered the C++ Cross renderer
			// 我们可以硬编码调用一下 Cross 渲染器作为 fallback
			// We can hardcode a call to the Cross renderer as fallback
			UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Verbose,
				TEXT("[HC][HUD] Renderer not found for Tag '%s', trying fallback Cross."),
				*Profile.ShapeTag.ToString());
			if (auto* DefaultRenderer = Subsystem->GetRenderer(FHelsincyCrosshair_Tags::Shape_Cross))
			{
				CurrentCrosshairRendererTag = FHelsincyCrosshair_Tags::Shape_Cross;
				CurrentCrosshairRenderer = DefaultRenderer;
				// 暂时用默认准星顶替一下 | Temporarily substitute with default crosshair
				DefaultRenderer->Draw(Canvas, Profile, Spread, CrosshairCenter, FinalColor, DeltaSeconds, Scale);
			}
		}
	}
}

void AHelsincyHUD::DrawCenterDot(const FHelsincyCrosshairProfile& Profile, FVector2D Center, float Scale)
{
	const auto& DotCfg = Profile.CenterDotConfig;
	
	// 缩放大小 | Scale size
	float Size = DotCfg.Size * Scale;
	// 限制最小 1 像素，否则 4K 下可能看不见 | Clamp minimum 1px, otherwise invisible on 4K
	Size = FMath::Max(1.0f, Size);
	
	FLinearColor DotColor = Profile.CenterDotConfig.Color;
	DotColor.A = Profile.CenterDotConfig.Opacity;

	// 使用 Box Item 绘制实心点 | Use Box Item to draw solid dot
	// 居中偏移 | Center offset
	FVector2D Pos = Center - FVector2D(Size * 0.5f, Size * 0.5f);
    
	FCanvasTileItem TileItem(Pos, FVector2D(Size, Size), DotColor);
	// 使用白色纹理来着色，如果没有 GWhiteTexture，CanvasTileItem 默认可能不显示，通常 HUD 默认有 WhiteTexture
	// Use white texture for tinting; without GWhiteTexture, CanvasTileItem may not display; HUD usually has WhiteTexture
	TileItem.Texture = GWhiteTexture; 
	TileItem.BlendMode = SE_BLEND_Translucent;
    
	Canvas->DrawItem(TileItem);
}

void AHelsincyHUD::DrawHitMarkers_Refactor(const FHelsincyCrosshairProfile& Profile, const TArray<FHelsincy_ActiveHitMarker>& Markers, FVector2D Center, float Scale)
{
	SCOPE_CYCLE_COUNTER(STAT_HC_HitMarkerDraw);

	HelsincySingleHitMarkerRenderCore::DrawHitMarkers(
		Canvas,
		Profile,
		Markers,
		Center,
		Scale,
		GetCrosshairManagerSubsystem()
	);
}

void AHelsincyHUD::CalculateHitMarkerState(const FHelsincyCrosshairProfile& Profile, const FHelsincy_ActiveHitMarker& Marker, FVector2D Center, float Scale, FHelsincyHitMarkerRenderState& OutState)
{
	const auto& Config = Profile.HitMarkerConfig;

	// 生命周期与动画进度 | Lifecycle and animation progress
	float LifeTimeRatio = Marker.TotalDuration > KINDA_SMALL_NUMBER
		? Marker.TimeRemaining / Marker.TotalDuration : 0.0f;
	float AnimProgress = 1.0f - LifeTimeRatio;
	float AnimAlpha = FMath::InterpEaseOut(0.0f, 1.0f, AnimProgress, 2.0f);
    
	// 2. 计算衰减系数 | 2. Calculate decay factor
	float DecayFactor = 1.0f;
	if (Config.bShakeDecay)
	{
		// 使用平方衰减，让震动在开始时很强，然后迅速变弱
		// Use quadratic decay: strong shake at start, weakens rapidly
		DecayFactor = LifeTimeRatio * LifeTimeRatio;
	}
	
	OutState.BasePosition = Center;
	OutState.Scale = Scale;

	OutState.CalculatedGlobalOffset = FVector2D::ZeroVector;
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
			float FadeStart = 1.0f - Config.SingleInstanceFadeRatio;
			if (AnimProgress > FadeStart)
			{
				float FadeAlpha = 1.0f - (AnimProgress - FadeStart) / Config.SingleInstanceFadeRatio;
				OutState.Color.A *= FadeAlpha;
			}
		}
	}
	else
	{
		// 多实例经典: 动态 StartSize→EndSize + StartOffset→EndOffset + 全程淡出
		// MultiInstance classic: animated size + offset + full fade
		float AnimOffset = FMath::Lerp(Config.StartOffset, Config.EndOffset, AnimAlpha);
		OutState.InnerOffset = (Config.BaseDistance + AnimOffset) * Scale * OutState.ImpactScaleMultiplier;

		float BaseSize = FMath::Lerp(Config.StartSize, Config.EndSize, AnimAlpha);
		OutState.Length = BaseSize * Marker.SizeScale * Scale * OutState.ImpactArmLengthMultiplier;

		OutState.Color = Marker.CurrentColor;
		float CurrentOpacity = FMath::Lerp(1.0f, 0.0f, AnimProgress);
		OutState.Color.A *= CurrentOpacity;
	}
}

void AHelsincyHUD::DrawHitMarker_Image(const FHelsincy_HitMarkerProfile& Config, const FHelsincyHitMarkerRenderState& State)
{
	if (!Config.CustomTexture || !Config.CustomTexture->GetResource()) return;
	// --- 图片模式 | Image Mode ---
	// 如果是图片，Offset 控制图片离中心的扩散距离 | For images, Offset controls spread distance from center
	// Size 控制图片本身的大小 | Size controls the image's own size
	FVector2D TexSize(State.Length, State.Length);
            
		// 绘制4个方向的贴图 (类似 CoD 的 X 贴图) | Draw textures in 4 directions (CoD-like X pattern)
	for (int32 i = 0; i < 4; i++)
	{
		const FVector2D Dir = HelsincyHitMarkerShakeMath::RotateDirection(HitMarkerDirs[i], State.ImpactRotationDegrees);
		FVector2D Pos = State.BasePosition + State.CalculatedGlobalOffset + (Dir * State.InnerOffset) - (TexSize * 0.5f);
		FCanvasTileItem TileItem(Pos, Config.CustomTexture->GetResource(), TexSize, State.Color);
		TileItem.Rotation = FRotator(0.0f, 0.0f, State.ImpactRotationDegrees);
		TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem); 
	}
}

void AHelsincyHUD::DrawHitMarker_Tapered(const FHelsincy_HitMarkerProfile& HitMarkerProfile, const FHelsincyHitMarkerRenderState& State, FTexture* SmoothTexture)
{
	// 1. 纹理保险：如果有抗锯齿纹理就用，没有就用纯白
    FTexture* TexToUse = (SmoothTexture) ? SmoothTexture : GWhiteTexture;

    auto DrawPass = [&](float WidthScale, float AlphaScale, ESimpleElementBlendMode BlendMode)
    {
        FLinearColor FinalCol = State.Color;
        // 恢复透明度逻辑 (日志显示你的Alpha是正常的)
        // Restore alpha logic (logs show your Alpha values are normal)
        FinalCol.A *= AlphaScale; 

        for (int32 i = 0; i < 4; i++)
        {
        	// 线条朝向 | Line direction
            FVector2D Dir = HelsincyHitMarkerShakeMath::RotateDirection(HitMarkerDirs[i], State.ImpactRotationDegrees);
        	// 【关键】垂直于线条的方向 (法线)
        	// [Key] Direction perpendicular to line (normal)
            FVector2D RightDir(Dir.Y, -Dir.X);

        	// --- [震动叠加逻辑] | [Shake Overlay Logic] ---
            
        	// 基础位置 | Base position
        	FVector2D CurrentArmCenter = State.BasePosition;

        	// 叠加全局震动 (所有臂都一样)
        	// Apply global shake (same for all arms)
        	// 如果 GlobalShake 是 (0,0)，这里加了也没影响
        	// If GlobalShake is (0,0), adding it has no effect
        	CurrentArmCenter += State.CalculatedGlobalOffset;

            // 叠加法线震动 (每个臂使用稳定采样，两个 Pass 共享相同值)
            // Apply normal shake (stable per-arm samples, both passes share same values)
            if (State.CurrentNormalShakeIntensity > 0.1f)
            {
                // 沿着法线方向偏移 | Offset along normal direction
                CurrentArmCenter += RightDir * (State.NormalShakeValues[i] * State.CurrentNormalShakeIntensity);
            }

            // 计算顶点 | Calculate vertices
            FVector2D InnerBase = CurrentArmCenter + (Dir * State.InnerOffset);
            FVector2D OuterTip = InnerBase + (Dir * State.Length);

            float HalfWidth = (State.Thickness * WidthScale) * 0.5f;
            FVector2D BaseLeft = InnerBase + (RightDir * HalfWidth);
            FVector2D BaseRight = InnerBase - (RightDir * HalfWidth);

            // 构造三角形 (顶点顺序: Tip, Left, Right)
            // Construct triangle (vertex order: Tip, Left, Right)
            FCanvasTriangleItem TriItem(OuterTip, BaseLeft, BaseRight, TexToUse);

        	// --- [修正 UV 设置逻辑] | [Corrected UV Setup Logic] ---
            // 由于UHelsincyCrosshairManagerSubsystem生成的程序化抗锯齿纹理有些部分是透明的
            // Since the procedural anti-aliased texture generated by UHelsincyCrosshairManagerSubsystem has some transparent parts
        	// 如果不设置UV直接拿来绘制就会导致画的命中反馈变成完全透明的, 所以这部分UV调整必不可少
            // Without setting UV, the drawn hit markers become fully transparent, so this UV adjustment is essential
        	// TriangleList[0] 代表这一个三角形整体
        	// TriangleList[0] represents this entire triangle
        	// 我们直接修改它的 V0, V1, V2 成员
        	// We directly modify its V0, V1, V2 members
            
        	// 顶点 0 (OuterTip / 尖头) -> 映射到纹理中心 (实心)
        	// Vertex 0 (OuterTip) -> map to texture center (solid)
        	TriItem.TriangleList[0].V0_UV = FVector2D(1.0f, 0.5f);
            
        	// 顶点 1 (BaseLeft / 底座左) -> 映射到纹理左上 (透明)
        	// Vertex 1 (BaseLeft) -> map to texture top-left (transparent)
        	TriItem.TriangleList[0].V1_UV = FVector2D(0.0f, 0.0f);

        	// 顶点 2 (BaseRight / 底座右) -> 映射到纹理左下 (透明)
        	// Vertex 2 (BaseRight) -> map to texture bottom-left (transparent)
        	TriItem.TriangleList[0].V2_UV = FVector2D(0.0f, 1.0f);

        	// [可选] 为了防止颜色没应用上，手动同步顶点颜色 (虽然 TriItem.SetColor 通常够用了)
        	// [Optional] Manually sync vertex colors to prevent color not being applied (though TriItem.SetColor usually suffices)
        	TriItem.TriangleList[0].V0_Color = FinalCol;
        	TriItem.TriangleList[0].V1_Color = FinalCol;
        	TriItem.TriangleList[0].V2_Color = FinalCol;

            // 设置颜色 | Set color
            TriItem.SetColor(FinalCol);
            TriItem.BlendMode = BlendMode;
            
            if (Canvas) Canvas->DrawItem(TriItem);
        }
    };

    // Pass 1: 辉光 (Additive 混合, 产生发光叠加效果)
    // Pass 1: Glow (Additive blend, produces luminous overlay effect)
    DrawPass(HitMarkerProfile.TaperedShapeGlowWidthScale, HitMarkerProfile.TaperedShapeGlowAlphaScale, SE_BLEND_Additive);
    
    // Pass 2: 核心 (Translucent 混合, 保持实心外观)
    // Pass 2: Core (Translucent blend, maintains solid appearance)
    DrawPass(1.0f, 1.0f, SE_BLEND_Translucent);
}

void AHelsincyHUD::DrawHitMarker_Linear(const FHelsincyHitMarkerRenderState& State)
{
	for (int32 i = 0; i < 4; i++)
	{
		FVector2D Dir = HelsincyHitMarkerShakeMath::RotateDirection(HitMarkerDirs[i], State.ImpactRotationDegrees);
		FVector2D RightDir(Dir.Y, -Dir.X); // 计算垂直方向 | Calculate perpendicular direction

		// --- 相同的震动叠加逻辑 | Same shake overlay logic ---
		FVector2D CurrentArmCenter = State.BasePosition + State.CalculatedGlobalOffset;

		if (State.CurrentNormalShakeIntensity > 0.1f)
		{
			CurrentArmCenter += RightDir * (State.NormalShakeValues[i] * State.CurrentNormalShakeIntensity);
		}
        
		// 绘制 | Draw
		FVector2D Start = CurrentArmCenter + (Dir * State.InnerOffset);
		FVector2D End = Start + (Dir * State.Length);

		FCanvasLineItem LineItem(Start, End);
		LineItem.SetColor(State.Color);
		LineItem.LineThickness = State.Thickness;
		LineItem.BlendMode = SE_BLEND_Translucent;
		if (Canvas) Canvas->DrawItem(LineItem);
	}
}
