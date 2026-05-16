// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HelsincyShapeRenderer.h"
#include "HelsincyRendererImage.generated.h"

/**
 * 
 */
UCLASS()
class HELSINCYCROSSHAIR_API UHelsincyRendererImage : public UHelsincyShapeRenderer
{
	GENERATED_BODY()

public:
	UHelsincyRendererImage();

	virtual void Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale) override;
};
