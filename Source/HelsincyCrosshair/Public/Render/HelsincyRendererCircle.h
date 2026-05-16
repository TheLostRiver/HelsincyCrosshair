// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HelsincyShapeRenderer.h"
#include "HelsincyRendererCircle.generated.h"

class UHelsincyCrosshairManagerSubsystem;
/**
 * 
 */
UCLASS()
class HELSINCYCROSSHAIR_API UHelsincyRendererCircle : public UHelsincyShapeRenderer
{
	GENERATED_BODY()

public:
	UHelsincyRendererCircle();
	
	virtual void Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale) override;

private:

	UPROPERTY()
	UHelsincyCrosshairManagerSubsystem* HCSubsystem {nullptr};
	
	UHelsincyCrosshairManagerSubsystem* GetCrosshairManagerSubsystem();
	
};
