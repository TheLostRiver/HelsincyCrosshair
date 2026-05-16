// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "Engine/Texture2D.h"
#include "UObject/UObjectGlobals.h"

namespace HelsincySingleHitMarkerSpriteSupport
{
	enum class EResolvedSingleHitMarkerSpriteMode : uint8
	{
		LegacyGeometry,
		CoreOnly,
		SpriteDualLayer
	};

	struct FResolvedSingleHitMarkerSpriteAssets
	{
		EResolvedSingleHitMarkerSpriteMode Mode = EResolvedSingleHitMarkerSpriteMode::LegacyGeometry;
		UTexture2D* CoreTexture = nullptr;
		UTexture2D* GlowTexture = nullptr;
	};

	inline const TCHAR* GetDefaultDataAssetPath()
	{
		return TEXT("/HelsincyCrosshair/DA_Crosshair_Default.DA_Crosshair_Default");
	}

	inline const TCHAR* GetDefaultDataAssetFolderPath()
	{
		return TEXT("/HelsincyCrosshair/DataAsset/DA_Crosshair_Default.DA_Crosshair_Default");
	}

	inline const TCHAR* GetDefaultCoreTexturePath()
	{
		return TEXT("/HelsincyCrosshair/HitmarkerImage/T_HC_HitMarker_Core.T_HC_HitMarker_Core");
	}

	inline const TCHAR* GetDefaultGlowTexturePath()
	{
		return TEXT("/HelsincyCrosshair/HitmarkerImage/T_HC_HitMarker_Glow.T_HC_HitMarker_Glow");
	}

	inline float GetDefaultCoreScale()
	{
		return 0.80f;
	}

	inline float GetDefaultGlowScale()
	{
		return 0.86f;
	}

	inline float GetDefaultGlowOpacityScale()
	{
		return 0.26f;
	}

	inline float GetDefaultCrosshairAlphaScale()
	{
		return 0.10f;
	}

	inline bool IsPluginDefaultCrosshairAsset(const UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		const FString AssetPath = Asset->GetPathName();
		return AssetPath == GetDefaultDataAssetPath()
			|| AssetPath == GetDefaultDataAssetFolderPath();
	}

	inline UTexture2D* LoadSpriteTexture(const TCHAR* ObjectPath)
	{
		return ObjectPath ? LoadObject<UTexture2D>(nullptr, ObjectPath) : nullptr;
	}

	inline void ApplyDefaultSpriteBindings(FHelsincy_HitMarkerProfile& Config, const UObject* SourceAsset)
	{
		if (!Config.bEnabled || Config.Mode != EHitMarkerMode::SingleInstance)
		{
			return;
		}

		if (!IsPluginDefaultCrosshairAsset(SourceAsset))
		{
			return;
		}

		// Preserve the asset's selected backend. The default plugin asset path only
		// receives Sprite resources and tuning defaults here; it must not silently
		// override the user's render-mode choice at runtime.
		Config.SingleInstanceCoreScale = GetDefaultCoreScale();
		Config.SingleInstanceGlowScale = GetDefaultGlowScale();
		Config.SingleInstanceGlowOpacityScale = GetDefaultGlowOpacityScale();
		Config.SingleInstanceCrosshairAlphaScale = GetDefaultCrosshairAlphaScale();
		if (Config.CrosshairVisibilityWhileActive == EHelsincyHitMarkerCrosshairVisibilityPolicy::ScaleAlpha)
		{
			Config.CrosshairAlphaScaleWhileHitMarkerActive = GetDefaultCrosshairAlphaScale();
		}

		if (!Config.SingleInstanceCoreTexture)
		{
			Config.SingleInstanceCoreTexture = LoadSpriteTexture(GetDefaultCoreTexturePath());
		}

		if (!Config.SingleInstanceGlowTexture)
		{
			Config.SingleInstanceGlowTexture = LoadSpriteTexture(GetDefaultGlowTexturePath());
		}
	}

	inline FResolvedSingleHitMarkerSpriteAssets ResolveSpriteAssets(const FHelsincy_HitMarkerProfile& Config)
	{
		FResolvedSingleHitMarkerSpriteAssets Result;
		if (!Config.bEnabled
			|| Config.Mode != EHitMarkerMode::SingleInstance
			|| Config.SingleInstanceRenderMode != EHelsincySingleHitMarkerRenderMode::SpriteDualLayer)
		{
			return Result;
		}

		Result.CoreTexture = Config.SingleInstanceCoreTexture;
		Result.GlowTexture = Config.SingleInstanceGlowTexture;

		if (!Result.CoreTexture)
		{
			Result.Mode = EResolvedSingleHitMarkerSpriteMode::LegacyGeometry;
			return Result;
		}

		Result.Mode = Result.GlowTexture
			? EResolvedSingleHitMarkerSpriteMode::SpriteDualLayer
			: EResolvedSingleHitMarkerSpriteMode::CoreOnly;
		return Result;
	}
}
