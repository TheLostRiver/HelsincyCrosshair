// Copyright , Helsincy Games. All Rights Reserved.

#include "HelsincyDamageIndicator.h"

DEFINE_LOG_CATEGORY(LogHelsincyDamageIndicator);

DEFINE_STAT(STAT_HDI_ComponentTick);
DEFINE_STAT(STAT_HDI_UpdateIndicators);
DEFINE_STAT(STAT_HDI_BridgeDraw);
DEFINE_STAT(STAT_HDI_DrawIndicators);

#define LOCTEXT_NAMESPACE "FHelsincyDamageIndicatorModule"

void FHelsincyDamageIndicatorModule::StartupModule()
{
}

void FHelsincyDamageIndicatorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHelsincyDamageIndicatorModule, HelsincyDamageIndicator)
