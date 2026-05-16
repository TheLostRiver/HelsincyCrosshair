// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HelsincyCrosshairInterface.generated.h"

class UHelsincyCrosshairComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHelsincyCrosshairInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELSINCYCROSSHAIR_API IHelsincyCrosshairInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HelsincyCrosshairInterface")
	UHelsincyCrosshairComponent* GetHelsincyCrosshairComponent();
	
};
