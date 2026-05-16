// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class APlayerController;

enum class EHelsincyHitMarkerDrawPath : uint8
{
	None,
	HUD,
	Bridge
};

namespace HelsincyCrosshairDebug
{
#if !UE_BUILD_SHIPPING
	bool IsEnabled();
	bool IsTextEnabled();
	bool IsGeometryEnabled();
	bool IsVerboseLogEnabled();
	bool IsHitMarkerDiagnosticsEnabled();
	bool IsOnlyLocal();
#else
	FORCEINLINE bool IsEnabled()          { return false; }
	FORCEINLINE bool IsTextEnabled()      { return false; }
	FORCEINLINE bool IsGeometryEnabled()  { return false; }
	FORCEINLINE bool IsVerboseLogEnabled(){ return false; }
	FORCEINLINE bool IsHitMarkerDiagnosticsEnabled() { return false; }
	FORCEINLINE bool IsOnlyLocal()        { return true;  }
#endif

	bool IsHitMarkerSoftGuardEnabled();
	bool ShouldSkipSingleHitMarkerDraw(const APlayerController* PlayerController, EHelsincyHitMarkerDrawPath Path);
	FString GetSingleHitMarkerDrawGuardSummary(const APlayerController* PlayerController);
}
