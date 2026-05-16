// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#include "HelsincyCrosshair.h"

DEFINE_LOG_CATEGORY(LogHelsincyCrosshair);

DEFINE_STAT(STAT_HC_CrosshairComponentTick);
DEFINE_STAT(STAT_HC_HUDDraw);
DEFINE_STAT(STAT_HC_BridgeDraw);
DEFINE_STAT(STAT_HC_RendererDraw);
DEFINE_STAT(STAT_HC_HitMarkerDraw);
DEFINE_STAT(STAT_HC_TargetDetectionRequest);
DEFINE_STAT(STAT_HC_TargetDetectionCallback);

#define LOCTEXT_NAMESPACE "FHelsincyCrosshairModule"

void FHelsincyCrosshairModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FHelsincyCrosshairModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHelsincyCrosshairModule, HelsincyCrosshair)
