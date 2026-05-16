// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HelsincyDamageIndicatorSettings.generated.h"

class UHelsincyIndicatorRenderer;

/**
 * 伤害指示器模块的全局设置
 * Global settings for the damage indicator module
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="HelsincyDamageIndicatorSystem"))
class HELSINCYDAMAGEINDICATOR_API UHelsincyDamageIndicatorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 用户可以在这里手动添加蓝图编写的指示器 Renderer
	// Users can manually add Blueprint-written indicator Renderers here
	UPROPERTY(Config, EditAnywhere, Category = "HelsincyDamageIndicatorSettings")
	TArray<TSoftClassPtr<UHelsincyIndicatorRenderer>> BlueprintIndicatorRenderers;
};
