// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#include "Debug/HelsincyCrosshairDebug.h"
#include "HelsincyCrosshair.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"

namespace
{
	struct FHelsincySingleHitMarkerDrawRecord
	{
		uint64 FrameNumber = MAX_uint64;
		EHelsincyHitMarkerDrawPath FirstPathThisFrame = EHelsincyHitMarkerDrawPath::None;
		EHelsincyHitMarkerDrawPath LastPathThisFrame = EHelsincyHitMarkerDrawPath::None;
		bool bDuplicateDetectedThisFrame = false;
		uint64 LastDuplicateFrame = MAX_uint64;
		EHelsincyHitMarkerDrawPath LastDuplicateFirstPath = EHelsincyHitMarkerDrawPath::None;
		EHelsincyHitMarkerDrawPath LastDuplicateSecondPath = EHelsincyHitMarkerDrawPath::None;
		int32 TotalDuplicateEvents = 0;
	};

	TAutoConsoleVariable<int32> CVarHCHitMarkerSoftGuard(
		TEXT("hc.HitMarker.SoftGuard"),
		1,
		TEXT("Prevent duplicate single-instance hitmarker draws across HUD and Bridge in the same frame. 0=Off, 1=On"),
		ECVF_Default);

	TMap<const APlayerController*, FHelsincySingleHitMarkerDrawRecord> GSingleHitMarkerDrawRecords;

	const TCHAR* GetHitMarkerDrawPathName(EHelsincyHitMarkerDrawPath Path)
	{
		switch (Path)
		{
		case EHelsincyHitMarkerDrawPath::HUD:
			return TEXT("HUD");
		case EHelsincyHitMarkerDrawPath::Bridge:
			return TEXT("Bridge");
		default:
			return TEXT("None");
		}
	}
}

#if !UE_BUILD_SHIPPING

static TAutoConsoleVariable<int32> CVarHCDebugEnable(
	TEXT("hc.Debug.Enable"),
	0,
	TEXT("Master switch for HelsincyCrosshair debug output. 0=Off, 1=On"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarHCDebugText(
	TEXT("hc.Debug.Text"),
	1,
	TEXT("Enable on-screen text debug (ShowDebug). Only works when hc.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarHCDebugGeometry(
	TEXT("hc.Debug.Geometry"),
	0,
	TEXT("Enable geometry visualization (debug lines/spheres). Only works when hc.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarHCDebugVerboseLog(
	TEXT("hc.Debug.VerboseLog"),
	0,
	TEXT("Enable verbose log output. Only works when hc.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarHCDebugHitMarkerDiag(
	TEXT("hc.Debug.HitMarkerDiag"),
	0,
	TEXT("Enable single-instance hitmarker diagnostics. Only works when hc.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarHCDebugOnlyLocal(
	TEXT("hc.Debug.OnlyLocal"),
	1,
	TEXT("Only output debug for locally controlled pawn. 0=All, 1=LocalOnly"),
	ECVF_Default);

bool HelsincyCrosshairDebug::IsEnabled()
{
	return CVarHCDebugEnable.GetValueOnGameThread() != 0;
}

bool HelsincyCrosshairDebug::IsTextEnabled()
{
	return IsEnabled() && CVarHCDebugText.GetValueOnGameThread() != 0;
}

bool HelsincyCrosshairDebug::IsGeometryEnabled()
{
	return IsEnabled() && CVarHCDebugGeometry.GetValueOnGameThread() != 0;
}

bool HelsincyCrosshairDebug::IsVerboseLogEnabled()
{
	return IsEnabled() && CVarHCDebugVerboseLog.GetValueOnGameThread() != 0;
}

bool HelsincyCrosshairDebug::IsHitMarkerDiagnosticsEnabled()
{
	return IsEnabled() && CVarHCDebugHitMarkerDiag.GetValueOnGameThread() != 0;
}

bool HelsincyCrosshairDebug::IsOnlyLocal()
{
	return CVarHCDebugOnlyLocal.GetValueOnGameThread() != 0;
}

#endif // !UE_BUILD_SHIPPING

bool HelsincyCrosshairDebug::IsHitMarkerSoftGuardEnabled()
{
	return CVarHCHitMarkerSoftGuard.GetValueOnGameThread() != 0;
}

bool HelsincyCrosshairDebug::ShouldSkipSingleHitMarkerDraw(const APlayerController* PlayerController, EHelsincyHitMarkerDrawPath Path)
{
	if (!PlayerController)
	{
		return false;
	}

	FHelsincySingleHitMarkerDrawRecord& Record = GSingleHitMarkerDrawRecords.FindOrAdd(PlayerController);
	const uint64 CurrentFrame = GFrameCounter;

	if (Record.FrameNumber != CurrentFrame)
	{
		Record.FrameNumber = CurrentFrame;
		Record.FirstPathThisFrame = Path;
		Record.LastPathThisFrame = Path;
		Record.bDuplicateDetectedThisFrame = false;
		return false;
	}

	Record.LastPathThisFrame = Path;
	if (!Record.bDuplicateDetectedThisFrame)
	{
		Record.bDuplicateDetectedThisFrame = true;
		Record.LastDuplicateFrame = CurrentFrame;
		Record.LastDuplicateFirstPath = Record.FirstPathThisFrame;
		Record.LastDuplicateSecondPath = Path;
		++Record.TotalDuplicateEvents;

#if !UE_BUILD_SHIPPING
		UE_CLOG(IsHitMarkerDiagnosticsEnabled() || IsVerboseLogEnabled(), LogHelsincyCrosshair, Warning,
			TEXT("[HC][HitMarker] Duplicate single-instance draw detected. Controller='%s' Frame=%llu FirstPath=%s SecondPath=%s SoftGuard=%s"),
			*GetNameSafe(PlayerController),
			CurrentFrame,
			GetHitMarkerDrawPathName(Record.LastDuplicateFirstPath),
			GetHitMarkerDrawPathName(Record.LastDuplicateSecondPath),
			IsHitMarkerSoftGuardEnabled() ? TEXT("On") : TEXT("Off"));
#endif
	}

	return IsHitMarkerSoftGuardEnabled();
}

FString HelsincyCrosshairDebug::GetSingleHitMarkerDrawGuardSummary(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return TEXT("NoController");
	}

	const FHelsincySingleHitMarkerDrawRecord* Record = GSingleHitMarkerDrawRecords.Find(PlayerController);
	if (!Record)
	{
		return TEXT("NoRecord");
	}

	return FString::Printf(TEXT("Frame=%llu First=%s Last=%s Dup=%s Total=%d"),
		Record->FrameNumber,
		GetHitMarkerDrawPathName(Record->FirstPathThisFrame),
		GetHitMarkerDrawPathName(Record->LastPathThisFrame),
		Record->bDuplicateDetectedThisFrame ? TEXT("true") : TEXT("false"),
		Record->TotalDuplicateEvents);
}
