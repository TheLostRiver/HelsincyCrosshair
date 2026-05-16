// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "HelsincyDamageIndicatorDataAsset.generated.h"

class UTexture2D;

/**
 * 储存伤害指示器数据的资产
 * Data asset for storing damage indicator configuration
 */
UCLASS()
class HELSINCYDAMAGEINDICATOR_API UHelsincyDamageIndicatorDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// 可选: 显示名称 | Optional: display name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	// 可选: 预览图标 | Optional: preview icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	UTexture2D* PreviewIcon = nullptr;

	// 伤害指示器配置 | Damage indicator profile
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FHelsincy_DamageIndicatorProfile Profile;
};
