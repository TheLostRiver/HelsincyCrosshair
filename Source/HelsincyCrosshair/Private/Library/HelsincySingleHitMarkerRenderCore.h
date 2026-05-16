#pragma once

#include "CoreMinimal.h"
#include "CanvasItem.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Library/HelsincyHitMarkerShakeMath.h"
#include "Library/HelsincySingleHitMarkerSpriteSupport.h"
#include "Subsystems/HelsincyCrosshairManagerSubsystem.h"

namespace HelsincySingleHitMarkerRenderCore
{
	struct FSharedHitMarkerRenderState
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

	struct FHitMarkerCrosshairVisibilityResult
	{
		bool bHitMarkerAffectsCrosshair = false;
		bool bShouldDrawBaseCrosshair = true;
		bool bShouldDrawCenterDot = true;
		float BaseCrosshairAlphaScale = 1.0f;
		float CenterDotAlphaScale = 1.0f;

		FORCEINLINE bool ShouldDrawBaseCrosshair() const
		{
			return bShouldDrawBaseCrosshair && BaseCrosshairAlphaScale > KINDA_SMALL_NUMBER;
		}

		FORCEINLINE bool ShouldDrawCenterDot() const
		{
			return bShouldDrawCenterDot && CenterDotAlphaScale > KINDA_SMALL_NUMBER;
		}
	};

	inline FVector2D ResolveImpactDirection(const FVector2D& Direction, const FSharedHitMarkerRenderState& State)
	{
		return HelsincyHitMarkerShakeMath::RotateDirection(Direction, State.ImpactRotationDegrees);
	}

	inline const FVector2D* GetHitMarkerDirections()
	{
		static const float Diag = 0.70710678f;
		static const FVector2D Directions[4] =
		{
			FVector2D(-Diag, -Diag), FVector2D(Diag, -Diag),
			FVector2D(-Diag, Diag), FVector2D(Diag, Diag)
		};
		return Directions;
	}

	inline FTexture* ResolveSmoothHitMarkerTexture(const FHelsincy_HitMarkerProfile& Config, UHelsincyCrosshairManagerSubsystem* Subsystem)
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

	inline bool IsHitMarkerCrosshairVisibilityPolicyActive(const FHelsincy_HitMarkerProfile& Config, bool bAnyHitMarkerVisible)
	{
		return Config.bEnabled && bAnyHitMarkerVisible;
	}

	inline bool AreNewCrosshairVisibilityFieldsAtMigrationDefaults(const FHelsincy_HitMarkerProfile& Config)
	{
		return Config.CrosshairVisibilityWhileActive == EHelsincyHitMarkerCrosshairVisibilityPolicy::Hide
			&& FMath::IsNearlyEqual(Config.CrosshairAlphaScaleWhileHitMarkerActive, 0.10f)
			&& Config.bApplyHitMarkerVisibilityPolicyToCenterDot;
	}

	inline bool HasEditedLegacySingleInstanceVisibilityFields(const FHelsincy_HitMarkerProfile& Config)
	{
		if (!Config.bUseSingleInstanceVisualSeparation)
		{
			return true;
		}

		if (!FMath::IsNearlyEqual(Config.SingleInstanceCrosshairAlphaScale, 0.10f))
		{
			return true;
		}

		if (!Config.bHideCenterDotWhenSingleInstanceActive)
		{
			return true;
		}

		return false;
	}

	inline bool TryResolveLegacySingleInstanceVisibility(
		const FHelsincy_HitMarkerProfile& Config,
		FHitMarkerCrosshairVisibilityResult& OutResult)
	{
		if (Config.Mode != EHitMarkerMode::SingleInstance
			|| !AreNewCrosshairVisibilityFieldsAtMigrationDefaults(Config)
			|| !HasEditedLegacySingleInstanceVisibilityFields(Config))
		{
			return false;
		}

		if (!Config.bUseSingleInstanceVisualSeparation)
		{
			OutResult = FHitMarkerCrosshairVisibilityResult();
			return true;
		}

		OutResult.bHitMarkerAffectsCrosshair = true;
		OutResult.BaseCrosshairAlphaScale = FMath::Clamp(Config.SingleInstanceCrosshairAlphaScale, 0.0f, 1.0f);
		OutResult.bShouldDrawBaseCrosshair = OutResult.BaseCrosshairAlphaScale > KINDA_SMALL_NUMBER;

		if (Config.bHideCenterDotWhenSingleInstanceActive)
		{
			OutResult.bShouldDrawCenterDot = false;
			OutResult.CenterDotAlphaScale = 0.0f;
		}
		else
		{
			OutResult.CenterDotAlphaScale = FMath::Clamp(Config.SingleInstanceCenterDotAlphaScale, 0.0f, 1.0f);
			OutResult.bShouldDrawCenterDot = OutResult.CenterDotAlphaScale > KINDA_SMALL_NUMBER;
		}

		return true;
	}

	inline FHitMarkerCrosshairVisibilityResult ResolveCrosshairVisibilityWhileHitMarkerActive(
		const FHelsincy_HitMarkerProfile& Config,
		bool bAnyHitMarkerVisible)
	{
		FHitMarkerCrosshairVisibilityResult Result;
		if (!IsHitMarkerCrosshairVisibilityPolicyActive(Config, bAnyHitMarkerVisible))
		{
			return Result;
		}

		if (TryResolveLegacySingleInstanceVisibility(Config, Result))
		{
			return Result;
		}

		switch (Config.CrosshairVisibilityWhileActive)
		{
		case EHelsincyHitMarkerCrosshairVisibilityPolicy::KeepVisible:
			return Result;

		case EHelsincyHitMarkerCrosshairVisibilityPolicy::ScaleAlpha:
		{
			Result.bHitMarkerAffectsCrosshair = true;
			Result.BaseCrosshairAlphaScale = FMath::Clamp(Config.CrosshairAlphaScaleWhileHitMarkerActive, 0.0f, 1.0f);
			if (Config.bApplyHitMarkerVisibilityPolicyToCenterDot)
			{
				Result.CenterDotAlphaScale = Result.BaseCrosshairAlphaScale;
			}
			Result.bShouldDrawBaseCrosshair = Result.BaseCrosshairAlphaScale > KINDA_SMALL_NUMBER;
			Result.bShouldDrawCenterDot = Result.CenterDotAlphaScale > KINDA_SMALL_NUMBER;
			return Result;
		}

		case EHelsincyHitMarkerCrosshairVisibilityPolicy::Hide:
		default:
			Result.bHitMarkerAffectsCrosshair = true;
			Result.bShouldDrawBaseCrosshair = false;
			Result.BaseCrosshairAlphaScale = 0.0f;
			if (Config.bApplyHitMarkerVisibilityPolicyToCenterDot)
			{
				Result.bShouldDrawCenterDot = false;
				Result.CenterDotAlphaScale = 0.0f;
			}
			return Result;
		}
	}

	inline FHelsincyCrosshairProfile MakeBaseCrosshairDrawProfile(
		const FHelsincyCrosshairProfile& Profile,
		const FHitMarkerCrosshairVisibilityResult& Visibility)
	{
		FHelsincyCrosshairProfile DrawProfile = Profile;
		DrawProfile.VisualsConfig.Opacity = FMath::Clamp(
			DrawProfile.VisualsConfig.Opacity * Visibility.BaseCrosshairAlphaScale,
			0.0f,
			1.0f);
		return DrawProfile;
	}

	inline void DrawHitMarkerImage(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FSharedHitMarkerRenderState& State)
	{
		if (!Canvas || !Config.CustomTexture || !Config.CustomTexture->GetResource())
		{
			return;
		}

		const FVector2D* Directions = GetHitMarkerDirections();
		const FVector2D TexSize(State.Length, State.Length);
		for (int32 i = 0; i < 4; i++)
		{
			const FVector2D Dir = ResolveImpactDirection(Directions[i], State);
			const FVector2D Pos = State.BasePosition + State.CalculatedGlobalOffset + (Dir * State.InnerOffset) - (TexSize * 0.5f);
			FCanvasTileItem TileItem(Pos, Config.CustomTexture->GetResource(), TexSize, State.Color);
			TileItem.Rotation = FRotator(0.0f, 0.0f, State.ImpactRotationDegrees);
			TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
			TileItem.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(TileItem);
		}
	}

	inline void DrawHitMarkerLinear(UCanvas* Canvas, const FSharedHitMarkerRenderState& State)
	{
		if (!Canvas)
		{
			return;
		}

		const FVector2D* Directions = GetHitMarkerDirections();
		for (int32 i = 0; i < 4; i++)
		{
			const FVector2D Dir = ResolveImpactDirection(Directions[i], State);
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

	inline void DrawHitMarkerTapered(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FSharedHitMarkerRenderState& State, FTexture* SmoothTexture)
	{
		if (!Canvas)
		{
			return;
		}

		FTexture* TexToUse = SmoothTexture ? SmoothTexture : GWhiteTexture;
		const FVector2D* Directions = GetHitMarkerDirections();

		auto DrawPass = [&](float WidthScale, float AlphaScale, ESimpleElementBlendMode BlendMode)
		{
			FLinearColor FinalColor = State.Color;
			FinalColor.A *= AlphaScale;

			for (int32 i = 0; i < 4; i++)
			{
				const FVector2D Dir = ResolveImpactDirection(Directions[i], State);
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
				TriItem.TriangleList[0].V0_Color = FinalColor;
				TriItem.TriangleList[0].V1_Color = FinalColor;
				TriItem.TriangleList[0].V2_Color = FinalColor;
				TriItem.SetColor(FinalColor);
				TriItem.BlendMode = BlendMode;
				Canvas->DrawItem(TriItem);
			}
		};

		DrawPass(Config.TaperedShapeGlowWidthScale, Config.TaperedShapeGlowAlphaScale, SE_BLEND_Additive);
		DrawPass(1.0f, 1.0f, SE_BLEND_Translucent);
	}

	inline void DrawResolvedHitMarker(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FSharedHitMarkerRenderState& State, FTexture* SmoothTexResource)
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

	inline void CalculateClassicHitMarkerState(const FHelsincy_HitMarkerProfile& Config, const FHelsincy_ActiveHitMarker& Marker, FVector2D Center, float Scale, FSharedHitMarkerRenderState& OutState)
	{
		const float LifeTimeRatio = Marker.TotalDuration > KINDA_SMALL_NUMBER ? Marker.TimeRemaining / Marker.TotalDuration : 0.0f;
		const float Age = FMath::Max(Marker.TotalDuration - Marker.TimeRemaining, 0.0f);
		const float AnimProgress = 1.0f - LifeTimeRatio;
		const float AnimAlpha = FMath::InterpEaseOut(0.0f, 1.0f, AnimProgress, 2.0f);

		const float DecayFactor = Config.bShakeDecay
			? LifeTimeRatio * LifeTimeRatio
			: 1.0f;
		const float ShakeEnergy = FMath::Max(Marker.ShakeEnergy, 0.0f);

		OutState.BasePosition = Center;
		OutState.Scale = Scale;
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
				Config.ShakeIntensity * Scale * DecayFactor * ShakeEnergy,
				Config.ShakeFrequency,
				Config.ShakeDamping,
				Marker.ShakePhase);
			OutState.ImpactRotationDegrees = 0.0f;
			OutState.ImpactScaleMultiplier = 1.0f;
			OutState.ImpactArmLengthMultiplier = 1.0f;
			OutState.ImpactMotionAlpha = 0.0f;
		}
		const float NormalShakeWeight = Config.bUseImpactMotion ? 0.35f : 1.0f;
		OutState.CurrentNormalShakeIntensity = Config.NormalShakeIntensity * Scale * DecayFactor * ShakeEnergy * NormalShakeWeight;
		HelsincyHitMarkerShakeMath::ResolveNormalShakeValues(
			Marker.ShakeSeed,
			Age,
			Config.ShakeFrequency,
			Config.ShakeDamping,
			OutState.NormalShakeValues);

		OutState.Thickness = Config.Thickness * Scale * FMath::Lerp(1.0f, 1.12f, OutState.ImpactMotionAlpha);
		OutState.InnerOffset = (Config.BaseDistance + FMath::Lerp(Config.StartOffset, Config.EndOffset, AnimAlpha)) * Scale * OutState.ImpactScaleMultiplier;
		OutState.Length = FMath::Lerp(Config.StartSize, Config.EndSize, AnimAlpha) * Marker.SizeScale * Scale * OutState.ImpactArmLengthMultiplier;
		OutState.Color = Marker.CurrentColor;
		OutState.Color.A *= FMath::Lerp(1.0f, 0.0f, AnimProgress);
	}

	inline FVector2D CalculateSingleHitMarkerSpriteCenter(const FSharedHitMarkerRenderState& State)
	{
		return State.BasePosition + State.CalculatedGlobalOffset;
	}

	inline float CalculateSingleHitMarkerSpriteSize(const FSharedHitMarkerRenderState& State, float ScaleMultiplier)
	{
		const float BaseDiameter = (State.InnerOffset + State.Length) * 2.0f;
		return FMath::Max(BaseDiameter * ScaleMultiplier, 1.0f);
	}

	inline void DrawSingleHitMarkerSpriteLayer(UCanvas* Canvas, UTexture2D* Texture, FVector2D Center, float Size, const FLinearColor& Color, float RotationDegrees)
	{
		if (!Canvas || !Texture || !Texture->GetResource())
		{
			return;
		}

		const FVector2D SpriteSize(Size, Size);
		const FVector2D Position = Center - (SpriteSize * 0.5f);
		FCanvasTileItem TileItem(Position, Texture->GetResource(), SpriteSize, Color);
		TileItem.Rotation = FRotator(0.0f, 0.0f, RotationDegrees);
		TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}

	inline bool DrawSingleHitMarkerSprite(UCanvas* Canvas, const FHelsincy_HitMarkerProfile& Config, const FSharedHitMarkerRenderState& State)
	{
		const HelsincySingleHitMarkerSpriteSupport::FResolvedSingleHitMarkerSpriteAssets ResolvedAssets =
			HelsincySingleHitMarkerSpriteSupport::ResolveSpriteAssets(Config);

		if (ResolvedAssets.Mode == HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::LegacyGeometry)
		{
			return false;
		}

		const FVector2D SpriteCenter = CalculateSingleHitMarkerSpriteCenter(State);
		const float CoreSize = CalculateSingleHitMarkerSpriteSize(State, Config.SingleInstanceCoreScale);
		const float GlowSize = CalculateSingleHitMarkerSpriteSize(State, Config.SingleInstanceGlowScale);

		if (ResolvedAssets.Mode == HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::SpriteDualLayer)
		{
			FLinearColor GlowColor = State.Color;
			GlowColor.A *= Config.SingleInstanceGlowOpacityScale;
			DrawSingleHitMarkerSpriteLayer(Canvas, ResolvedAssets.GlowTexture, SpriteCenter, GlowSize, GlowColor, State.ImpactRotationDegrees);
		}

		DrawSingleHitMarkerSpriteLayer(Canvas, ResolvedAssets.CoreTexture, SpriteCenter, CoreSize, State.Color, State.ImpactRotationDegrees);
		return true;
	}

	inline void CalculateSingleHitMarkerState(const FHelsincy_HitMarkerProfile& Config, const FHelsincy_SingleHitMarkerState& MarkerState, FVector2D Center, float Scale, FSharedHitMarkerRenderState& OutState)
	{
		const float SafeMaxImpactEnergy = FMath::Max(Config.SingleInstanceMaxImpactEnergy, KINDA_SMALL_NUMBER);
		const float ImpactRatio = FMath::Clamp(MarkerState.ImpactEnergy / SafeMaxImpactEnergy, 0.0f, 1.0f);
		const float FeedbackStrength = FMath::Clamp(FMath::Max(MarkerState.AccentStrength, ImpactRatio), 0.0f, 1.0f);
		const float ShakeStrength = Config.bShakeDecay
			? FMath::Max(FeedbackStrength, MarkerState.ShakeEnergy)
			: FMath::Max(1.0f, MarkerState.ShakeEnergy);
		const float SafeSpacingScale = Config.bUseSingleInstanceVisualSeparation
			? FMath::Max(Config.SingleInstanceSafeSpacingScale, 1.0f)
			: 1.0f;

		OutState.BasePosition = Center;
		OutState.Scale = Scale;
		if (Config.bUseImpactMotion)
		{
			const HelsincyHitMarkerShakeMath::FResolvedImpactMotion ImpactMotion =
				HelsincyHitMarkerShakeMath::ResolveImpactMotion(
					Config,
					MarkerState.ShakeDirection,
					MarkerState.ShakeAge,
					MarkerState.ImpactMotionEnergy,
					MarkerState.ImpactDamageScale,
					MarkerState.ImpactMotionSign,
					MarkerState.ShakePhase,
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
				MarkerState.ShakeDirection,
				MarkerState.ShakeAge,
				Config.ShakeIntensity * Scale * ShakeStrength,
				Config.ShakeFrequency,
				Config.ShakeDamping,
				MarkerState.ShakePhase);
			OutState.ImpactRotationDegrees = 0.0f;
			OutState.ImpactScaleMultiplier = 1.0f;
			OutState.ImpactArmLengthMultiplier = 1.0f;
			OutState.ImpactMotionAlpha = 0.0f;
		}
		const float NormalShakeWeight = Config.bUseImpactMotion ? 0.35f : 1.0f;
		OutState.CurrentNormalShakeIntensity = Config.NormalShakeIntensity * Scale * ShakeStrength * NormalShakeWeight;
		HelsincyHitMarkerShakeMath::ResolveNormalShakeValues(
			MarkerState.ShakeSeed,
			MarkerState.ShakeAge,
			Config.ShakeFrequency,
			Config.ShakeDamping,
			OutState.NormalShakeValues);
		OutState.Color = MarkerState.CurrentColor;
		OutState.Color.A *= MarkerState.Opacity;
		OutState.InnerOffset = Config.SingleInstanceOffset * SafeSpacingScale * Scale * OutState.ImpactScaleMultiplier;
		OutState.Length = Config.SingleInstanceSize * MarkerState.GetEffectiveScale() * Scale * OutState.ImpactArmLengthMultiplier;
		OutState.Thickness = MarkerState.Thickness * Scale * FMath::Lerp(1.0f, 1.12f, OutState.ImpactMotionAlpha);
	}

	inline void DrawHitMarkers(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, const TArray<FHelsincy_ActiveHitMarker>& Markers, FVector2D Center, float Scale, UHelsincyCrosshairManagerSubsystem* Subsystem)
	{
		const FHelsincy_HitMarkerProfile& Config = Profile.HitMarkerConfig;
		if (!Canvas || !Config.bEnabled || Markers.Num() == 0)
		{
			return;
		}

		FTexture* SmoothTexResource = ResolveSmoothHitMarkerTexture(Config, Subsystem);
		FSharedHitMarkerRenderState RenderState;
		for (const FHelsincy_ActiveHitMarker& Marker : Markers)
		{
			CalculateClassicHitMarkerState(Config, Marker, Center, Scale, RenderState);
			DrawResolvedHitMarker(Canvas, Config, RenderState, SmoothTexResource);
		}
	}

	inline void DrawSingleHitMarker(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, const FHelsincy_SingleHitMarkerState& MarkerState, FVector2D Center, float Scale, UHelsincyCrosshairManagerSubsystem* Subsystem)
	{
		const FHelsincy_HitMarkerProfile& Config = Profile.HitMarkerConfig;
		if (!Canvas || !Config.bEnabled || !MarkerState.bVisible)
		{
			return;
		}

		FSharedHitMarkerRenderState RenderState;
		CalculateSingleHitMarkerState(Config, MarkerState, Center, Scale, RenderState);
		if (!DrawSingleHitMarkerSprite(Canvas, Config, RenderState))
		{
			DrawResolvedHitMarker(Canvas, Config, RenderState, ResolveSmoothHitMarkerTexture(Config, Subsystem));
		}
	}
}
