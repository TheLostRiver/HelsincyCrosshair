// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "HelsincyCrosshairDataAsset.generated.h"

/**
 * 储存准星数据的资产
 * Data asset for storing crosshair configuration
 */
UCLASS()
class HELSINCYCROSSHAIR_API UHelsincyCrosshairDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// 可选: 准星名称 | Optional: crosshair display name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CrosshairAsset")
	FText DisplayName;

	// 可选: 准星预览图标 | Optional: crosshair preview icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CrosshairAsset")
	UTexture2D* PreviewIcon {nullptr};
	
	// 这里直接包含我们定义的终极结构体 | Directly contains our ultimate crosshair profile struct
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CrosshairAsset")
	FHelsincyCrosshairProfile Profile;

};
