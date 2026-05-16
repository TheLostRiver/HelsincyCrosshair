// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Stats/Stats.h"

HELSINCYDAMAGEINDICATOR_API DECLARE_LOG_CATEGORY_EXTERN(LogHelsincyDamageIndicator, Log, All);

DECLARE_STATS_GROUP(TEXT("HelsincyDamageIndicator"), STATGROUP_HelsincyDamageIndicator, STATCAT_Advanced);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DamageIndicator Component Tick"), STAT_HDI_ComponentTick, STATGROUP_HelsincyDamageIndicator, HELSINCYDAMAGEINDICATOR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DamageIndicator Update Indicators"), STAT_HDI_UpdateIndicators, STATGROUP_HelsincyDamageIndicator, HELSINCYDAMAGEINDICATOR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DamageIndicator Bridge Draw"), STAT_HDI_BridgeDraw, STATGROUP_HelsincyDamageIndicator, HELSINCYDAMAGEINDICATOR_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DamageIndicator Draw Indicators"), STAT_HDI_DrawIndicators, STATGROUP_HelsincyDamageIndicator, HELSINCYDAMAGEINDICATOR_API);

class FHelsincyDamageIndicatorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
