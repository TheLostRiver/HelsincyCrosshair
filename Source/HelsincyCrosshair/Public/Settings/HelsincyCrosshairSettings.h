// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HelsincyCrosshairSettings.generated.h"

class UHelsincyShapeRenderer;

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="HelsincyCrosshairSystem"))
class HELSINCYCROSSHAIR_API UHelsincyCrosshairSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 用户可以在这里手动添加蓝图编写的 Renderer
	// Users can manually add Blueprint-written Renderers here
	UPROPERTY(Config, EditAnywhere, Category = "HelsincyCrosshairSettings")
	TArray<TSoftClassPtr<UHelsincyShapeRenderer>> BlueprintRenderers;
};
