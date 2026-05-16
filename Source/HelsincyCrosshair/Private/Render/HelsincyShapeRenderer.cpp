// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyShapeRenderer.h"



void UHelsincyShapeRenderer::RenderWithOutline(const FHelsincy_VisualSettings& Visuals, FLinearColor CurrentColor, float Scale, TFunctionRef<void(float, FLinearColor)> DrawCallback)
{
	// 缩放粗细 | Scale thickness
	float ScaledThickness = FMath::Max(1.0f, Visuals.Thickness * Scale);
	float ScaledOutlineThick = FMath::Max(1.0f, Visuals.OutlineThickness * Scale);

	// Pass 1: Outline
	if (Visuals.bEnableOutline)
	{
		// 描边总宽度 = 主体 + (描边 * 2)
		// Outline total width = core + (outline * 2)
		float TotalWidth = ScaledThickness + (ScaledOutlineThick * 2.0f);
        
		FLinearColor OutlineCol = Visuals.OutlineColor;
		OutlineCol.A = Visuals.Opacity;
		DrawCallback(TotalWidth, OutlineCol);
	}

	// Pass 2: Core
	FLinearColor CoreCol = CurrentColor;
	CoreCol.A = Visuals.Opacity;
	DrawCallback(ScaledThickness, CoreCol);
	
}
