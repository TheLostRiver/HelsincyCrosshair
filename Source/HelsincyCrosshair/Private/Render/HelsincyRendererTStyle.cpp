// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererTStyle.h"
#include "Engine/Canvas.h"


UHelsincyRendererTStyle::UHelsincyRendererTStyle()
{
    // 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
    // Using RequestGameplayTag because static variable init order may be undefined; Name is safest
    // 或者确信 FACS_Tags 已经初始化
    // Or ensure FACS_Tags has already been initialized
    AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.TStyle"));
}

void UHelsincyRendererTStyle::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
    // 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
    
	if (!Canvas) return;

    // 1. 获取配置引用 | 1. Get config references
    const auto& Vis = Profile.VisualsConfig;
    const auto& Cfg = Profile.CrosshairConfig;

    // Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
    // Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
    FVector2D ScaledSpread = Spread * Scale;

    // --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    float Length = Cfg.Length * Scale;
    float Gap = Cfg.Gap * Scale;

    // 旋转 | Rotation
    float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
    bool bRot = !FMath::IsNearlyZero(RadAngle);
    float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
    float S = bRot ? FMath::Sin(RadAngle) : 0.0f;

    // 定义 T 字的三个臂 (下, 左, 右) | Define T-shape's three arms (bottom, left, right)
    const FArm Arms[3] = {
        {FVector2D(0, 1), ScaledSpread.Y},   // 下 | Bottom
        {FVector2D(-1, 0), ScaledSpread.X},  // 左 | Left
        {FVector2D(1, 0), ScaledSpread.X}    // 右 | Right
    };

    // 缓存 3 条线段 | Cache 3 line segments
    TArray<FLineSeg, TFixedAllocator<3>> Lines;

    for (const auto& Arm : Arms)
    {
        float Off = Gap + Arm.Spread;
        
        // 计算本地起止点 | Calculate local start/end points
        FVector2D LocalS = Arm.Dir * Off;
        FVector2D LocalE = Arm.Dir * (Off + Length);

        // 应用旋转 | Apply rotation
        if (bRot)
        {
            float TX = LocalS.X; LocalS.X = TX * C - LocalS.Y * S; LocalS.Y = TX * S + LocalS.Y * C;
            float EX = LocalE.X; LocalE.X = EX * C - LocalE.Y * S; LocalE.Y = EX * S + LocalE.Y * C;
        }

        // 存入缓存 | Store in cache
        FLineSeg Seg;
        Seg.P1 = Center + LocalS;
        Seg.P2 = Center + LocalE;
        Lines.Add(Seg);
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
