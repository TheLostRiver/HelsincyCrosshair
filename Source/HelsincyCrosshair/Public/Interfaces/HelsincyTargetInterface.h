// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "HelsincyTargetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHelsincyTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELSINCYCROSSHAIR_API IHelsincyTargetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/**
	 * 获取该物体的交互 Tag | Get this object's interaction Tag
	 * 例如: Target.Interactable, Target.Loot.Gold
	 * Example: Target.Interactable, Target.Loot.Gold
	 * 如果返回 Empty Tag，则不改变颜色
	 * If returns Empty Tag, color will not change
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HelsincyTargetInterface")
	FGameplayTag GetCrosshairInteractionTag() const;
	
};
