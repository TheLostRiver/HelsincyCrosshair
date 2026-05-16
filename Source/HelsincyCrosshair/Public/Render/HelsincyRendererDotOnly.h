// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HelsincyShapeRenderer.h"
#include "HelsincyRendererDotOnly.generated.h"

/**
 * DotOnly 渲染器 | DotOnly Renderer
 * 选中此形状时不绘制任何准星线条，仅依赖 HUD 层的 CenterDot 绘制。
 * When this shape is selected, no crosshair lines are drawn; relies solely on HUD layer's CenterDot.
 */
UCLASS()
class HELSINCYCROSSHAIR_API UHelsincyRendererDotOnly : public UHelsincyShapeRenderer
{
	GENERATED_BODY()

public:
	UHelsincyRendererDotOnly();

	virtual void Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale) override;
};
