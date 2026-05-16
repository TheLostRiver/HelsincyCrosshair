// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererTriangle.h"
#include "Engine/Canvas.h"


UHelsincyRendererTriangle::UHelsincyRendererTriangle()
{
	// 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
	// Using RequestGameplayTag because static variable init order may be undefined; Name is safest
	// 或者确信 FACS_Tags 已经初始化
	// Or ensure FACS_Tags has already been initialized
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Triangle"));
}

void UHelsincyRendererTriangle::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
	// 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
	
	if (!Canvas) return;

	// 1. 获取配置引用 | 1. Get config references
	// Triangle 和 Rectangle 共用 BoxConfig (Size.X=宽, Size.Y=高)
	// Triangle and Rectangle share BoxConfig (Size.X=width, Size.Y=height)
	const auto& Vis = Profile.VisualsConfig;
	const auto& Cfg = Profile.BoxConfig;

	FVector2D TriangleSize = FVector2D(FMath::Clamp(Cfg.Size.X, 6.0f, 100.0f), FMath::Clamp(Cfg.Size.Y, 6.0f, 100.0f)) * Scale;
	// Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
	// Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
	FVector2D ScaledSpread = Spread * Scale;
	
	// --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    
	// 计算半宽和半高 | Calculate half-width and half-height
	// Spread.X 撑开底边宽度, Spread.Y 撑开高度
	// Spread.X stretches base width, Spread.Y stretches height
	float HW = (TriangleSize.X + ScaledSpread.X) * 0.5f;
	float HH = (TriangleSize.Y + ScaledSpread.Y) * 0.5f;

	// 预计算旋转 | Pre-calculate rotation
	float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
	bool bRot = !FMath::IsNearlyZero(RadAngle);
	float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
	float S = bRot ? FMath::Sin(RadAngle) : 0.0f;

	// 定义3个顶点 (本地坐标) | Define 3 vertices (local coordinates)
	// 假设中心点(0,0)是几何中心 | Assume center (0,0) is geometric center
	FVector2D RawPoints[3] = {
		{0.0f, -HH}, // 上顶点 | Top vertex
		{ HW,  HH},  // 右下 | Bottom Right
		{-HW,  HH}   // 左下 | Bottom Left
	};

	// 计算旋转和平移后的屏幕坐标 | Calculate screen coordinates after rotation and translation
	FVector2D ScreenPoints[3];
	for(int32 i = 0; i < 3; i++)
	{
		FVector2D P = RawPoints[i];
		if (bRot)
		{
			float TX = P.X;
			P.X = TX * C - P.Y * S;
			P.Y = TX * S + P.Y * C;
		}
		ScreenPoints[i] = Center + P;
	}

	// --- Phase 2: 渲染回调 | Rendering Callback ---
	RenderWithOutline(Vis, CurrentColor, Scale, [&](float Thickness, FLinearColor Color)
	{
		// 循环绘制 3 条边 | Draw 3 edges in a loop
		for (int32 i = 0; i < 3; i++)
		{
			// (i+1)%3 确保 0-1, 1-2, 2-0 闭合 | (i+1)%3 ensures 0-1, 1-2, 2-0 closure
			FCanvasLineItem Line(ScreenPoints[i], ScreenPoints[(i+1)%3]);
			Line.SetColor(Color);
			Line.LineThickness = Thickness;
			Line.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(Line);
		}
	});
}
