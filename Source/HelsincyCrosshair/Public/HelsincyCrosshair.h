// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Stats/Stats.h"

HELSINCYCROSSHAIR_API DECLARE_LOG_CATEGORY_EXTERN(LogHelsincyCrosshair, Log, All);

DECLARE_STATS_GROUP(TEXT("HelsincyCrosshair"), STATGROUP_HelsincyCrosshair, STATCAT_Advanced);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Crosshair Component Tick"), STAT_HC_CrosshairComponentTick, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Crosshair HUD Draw"), STAT_HC_HUDDraw, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Crosshair Bridge Draw"), STAT_HC_BridgeDraw, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Crosshair Renderer Draw"), STAT_HC_RendererDraw, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("HitMarker Draw"), STAT_HC_HitMarkerDraw, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Target Detection Request"), STAT_HC_TargetDetectionRequest, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Target Detection Callback"), STAT_HC_TargetDetectionCallback, STATGROUP_HelsincyCrosshair, HELSINCYCROSSHAIR_API);

class FHelsincyCrosshairModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
