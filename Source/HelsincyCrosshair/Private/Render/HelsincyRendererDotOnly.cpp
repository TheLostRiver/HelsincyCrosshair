// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererDotOnly.h"


UHelsincyRendererDotOnly::UHelsincyRendererDotOnly()
{
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.DotOnly"));
}

void UHelsincyRendererDotOnly::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
	// 不绘制任何形状。CenterDot 由 HUD 层单独绘制。
	// No shape drawn. CenterDot is drawn separately by the HUD layer.
}
