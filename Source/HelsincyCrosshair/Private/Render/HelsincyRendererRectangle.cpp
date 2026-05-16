// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererRectangle.h"
#include "Engine/Canvas.h"


UHelsincyRendererRectangle::UHelsincyRendererRectangle()
{
	// 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
	// Using RequestGameplayTag because static variable init order may be undefined; Name is safest
	// 或者确信 FACS_Tags 已经初始化
	// Or ensure FACS_Tags has already been initialized
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Rectangle"));
}

void UHelsincyRendererRectangle::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
	// 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
	
	if (!Canvas) return;

	// 1. 获取配置引用 | 1. Get config references
	// BoxConfig 包含了 Size 和 Rotation
	// BoxConfig contains Size and Rotation
	const auto& Vis = Profile.VisualsConfig;
	const auto& Cfg = Profile.BoxConfig;

	FVector2D ScaledSize = FVector2D(FMath::Clamp(Cfg.Size.X, 6.0f, 100.0f), FMath::Clamp(Cfg.Size.Y, 6.0f, 100.0f)) * Scale;
	// Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
	// Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
	FVector2D ScaledSpread = Spread * Scale;

	// --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    
	// 计算半宽和半高 | Calculate Half Width/Height
	// Spread.X 增加宽度，Spread.Y 增加高度 -> 完美的动态反馈
	// Spread.X increases width, Spread.Y increases height -> perfect dynamic feedback
	float HW = (ScaledSize.X + ScaledSpread.X) * 0.5f;
	float HH = (ScaledSize.Y + ScaledSpread.Y) * 0.5f;

	// 预计算旋转矩阵 | Pre-calculate rotation matrix
	float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
	bool bRot = !FMath::IsNearlyZero(RadAngle);
	float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
	float S = bRot ? FMath::Sin(RadAngle) : 0.0f;

	// 定义4个角 (未旋转的本地坐标)
	// Define 4 corners (unrotated local coordinates)
	// 顺序: 左上 -> 右上 -> 右下 -> 左下 (顺时针)
	// Order: Top Left -> Top Right -> Bottom Right -> Bottom Left (clockwise)
	FVector2D RawPoints[4] = {
		{-HW, -HH}, // Top Left
		{ HW, -HH}, // Top Right
		{ HW,  HH}, // Bottom Right
		{-HW,  HH}  // Bottom Left
	};
    
	// 计算旋转并平移后的屏幕坐标 | Calculate screen coordinates after rotation and translation
	FVector2D ScreenPoints[4];
	for(int32 i = 0; i < 4; i++)
	{
		FVector2D P = RawPoints[i];
        
		// 应用旋转 | Apply rotation
		if (bRot)
		{
			float TX = P.X;
			P.X = TX * C - P.Y * S;
			P.Y = TX * S + P.Y * C;
		}
        
		// 加上中心点 | Add center offset
		ScreenPoints[i] = Center + P;
	}

	// --- Phase 2: 渲染回调 | Rendering Callback ---
	// 调用基类的 RenderWithOutline，传入 Visuals 设置
	// Call base class RenderWithOutline, passing in Visuals settings
	RenderWithOutline(Vis, CurrentColor, Scale, [&](float Thickness, FLinearColor Color)
	{
		// 绘制四条边 | Draw four edges
		for (int32 i = 0; i < 4; i++)
		{
			// 连接当前点和下一个点 (模4以回到起点)
			// Connect current point to next (mod 4 to wrap back to start)
			// 0-1, 1-2, 2-3, 3-0
			FCanvasLineItem Line(ScreenPoints[i], ScreenPoints[(i + 1) % 4]);
            
			Line.SetColor(Color);
			Line.LineThickness = Thickness;
			Line.BlendMode = SE_BLEND_Translucent;

			Canvas->DrawItem(Line);
		}
	});
}
