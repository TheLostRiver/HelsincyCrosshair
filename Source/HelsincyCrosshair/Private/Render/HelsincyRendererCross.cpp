// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererCross.h"
#include "Engine/Canvas.h"


UHelsincyRendererCross::UHelsincyRendererCross()
{
    // 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
    // Using RequestGameplayTag because static variable initialization order may be undefined; Name is safest
    // 或者确信 FACS_Tags 已经初始化
    // Or ensure FACS_Tags has already been initialized
    AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Cross"));
}

void UHelsincyRendererCross::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
    // 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
    
	if (!Canvas) return;

    // 1. 引用子结构体 | 1. Reference sub-structs
    const auto& Vis = Profile.VisualsConfig;
    const auto& Cfg = Profile.CrosshairConfig;

    // --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    float Length = Cfg.Length * Scale;
    float Gap = Cfg.Gap * Scale;

    // Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
    // Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
    FVector2D ScaledSpread = Spread * Scale;
    
    // 旋转矩阵 | Rotation matrix
    float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
    bool bRot = !FMath::IsNearlyZero(RadAngle);
    float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
    float S = bRot ? FMath::Sin(RadAngle) : 0.0f;

    const FArm Arms[] = {
        {FVector2D(0, -1), ScaledSpread.Y},
        {FVector2D(0, 1), ScaledSpread.Y},
        {FVector2D(-1, 0), ScaledSpread.X},
        {FVector2D(1, 0), ScaledSpread.X}
    };

    // 我们预先算出4条线的 Start 和 End 点，存入数组
    // Pre-calculate 4 lines' Start and End points into an array
    // 这样在描边和填充两次 Pass 中，就不需要重复计算旋转和坐标了
    // This avoids redundant rotation and coordinate recalculations across outline and fill passes
    FLineSeg Lines[4];

    for (int32 i = 0; i < 4; i++)
    {
        float Off = Gap + Arms[i].Spread;
        FVector2D LocalS = Arms[i].Dir * Off;
        FVector2D LocalE = Arms[i].Dir * (Off + Length);

        if (bRot)
        {
            float TX = LocalS.X; LocalS.X = TX * C - LocalS.Y * S; LocalS.Y = TX * S + LocalS.Y * C;
            float EX = LocalE.X; LocalE.X = EX * C - LocalE.Y * S; LocalE.Y = EX * S + LocalE.Y * C;
        }
        Lines[i] = { Center + LocalS, Center + LocalE };
    }

    // --- Phase 2: 渲染回调 | Rendering Callback ---
    RenderWithOutline(Vis, CurrentColor, Scale, [&](float Thickness, FLinearColor Color)
    {
        for (const auto& L : Lines)
        {
            FCanvasLineItem Line(L.P1, L.P2);
            Line.SetColor(Color);
            Line.LineThickness = Thickness;
            Line.BlendMode = SE_BLEND_Translucent;
            Canvas->DrawItem(Line);
        }
    });
}
