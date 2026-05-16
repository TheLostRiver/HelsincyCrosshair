// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererChevron.h"
#include "Engine/Canvas.h"


UHelsincyRendererChevron::UHelsincyRendererChevron()
{
    // 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
    // Using RequestGameplayTag because static variable init order may be undefined; Name is safest
    // 或者确信 FACS_Tags 已经初始化
    // Or ensure FACS_Tags has already been initialized
    AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Chevron"));
}

void UHelsincyRendererChevron::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
    // 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
    
	if (!Canvas) return;

    // 1. 获取配置引用 | 1. Get config references
    // Chevron 使用 CrosshairConfig 中的 Length, Gap, Rotation 以及专属的 ChevronOpeningAngle
    // Chevron uses Length, Gap, Rotation from CrosshairConfig plus its own ChevronOpeningAngle
    const auto& Vis = Profile.VisualsConfig;
    const auto& Cfg = Profile.CrosshairConfig;

    // Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
    // Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
    FVector2D ScaledSpread = Spread * Scale;

    // --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    
    float LegLen = Cfg.Length * Scale;
    float Gap = Cfg.Gap * Scale;
    
    // 全局旋转 | Global Rotation
    float GRad = FMath::DegreesToRadians(Cfg.Rotation);
    bool bGRot = !FMath::IsNearlyZero(GRad);
    float GC = bGRot ? FMath::Cos(GRad) : 1.0f;
    float GS = bGRot ? FMath::Sin(GRad) : 0.0f;

    // 局部旋转：计算 V 形的开口角度的一半
    // Local rotation: calculate half of V-shape opening angle
    // 用于生成 V 的两条腿相对于中心轴的偏转
    // Used to generate the deflection of V's two legs relative to the central axis
    float LRad = FMath::DegreesToRadians(Cfg.ChevronOpeningAngle * 0.5f);
    float LC = FMath::Cos(LRad);
    float LS = FMath::Sin(LRad);
    
    const FArm Arms[4] = {
        {FVector2D(0, -1), ScaledSpread.Y}, // 上 | Top
        {FVector2D(0, 1),  ScaledSpread.Y}, // 下 | Bottom
        {FVector2D(-1, 0), ScaledSpread.X}, // 左 | Left
        {FVector2D(1, 0),  ScaledSpread.X}  // 右 | Right
    };

    // 缓存所有线段 (4个V * 2条腿 = 8条线) | Cache all segments (4 V-shapes * 2 legs = 8 lines)
    TArray<FLineSeg, TFixedAllocator<16>> Lines;

    for (const auto& Arm : Arms)
    {
        // 1. 计算 V 的顶点位置 | 1. Calculate V vertex position
        // 顶点沿着臂的方向延伸 Gap + Spread
        // Vertex extends along arm direction by Gap + Spread
        FVector2D LocalVert = Arm.Dir * (Gap + Arm.Spread);

        // 2. 计算两条腿的方向 | 2. Calculate leg directions (Local Rotation)
        // 基于 Arm.Dir 分别向左和向右旋转 HalfAngle
        // Rotate Arm.Dir left and right by HalfAngle respectively
        // 旋转公式 | Rotation formula: 
        // X' = X cos - Y sin
        // Y' = X sin + Y cos
        
        // Leg 1: +Angle
        FVector2D LegDir1(
            Arm.Dir.X * LC - Arm.Dir.Y * LS,
            Arm.Dir.X * LS + Arm.Dir.Y * LC
        );

        // Leg 2: -Angle (Sin 取反)
        FVector2D LegDir2(
            Arm.Dir.X * LC - Arm.Dir.Y * (-LS),
            Arm.Dir.X * (-LS) + Arm.Dir.Y * LC
        );

        // 3. 计算两条腿的终点 | 3. Calculate leg endpoints
        FVector2D LocalEnd1 = LocalVert + LegDir1 * LegLen;
        FVector2D LocalEnd2 = LocalVert + LegDir2 * LegLen;

        // 4. 应用全局旋转并转换到屏幕空间 | 4. Apply global rotation and convert to screen space
        auto Transform = [&](FVector2D P) -> FVector2D
        {
            if (bGRot) {
                float TX = P.X;
                P.X = TX * GC - P.Y * GS;
                P.Y = TX * GS + P.Y * GC;
            }
            return Center + P;
        };

        FVector2D ScreenVert = Transform(LocalVert);
        
        // 添加第一条腿 | Add first leg
        FLineSeg Line1;
        Line1.P1 = ScreenVert;
        Line1.P2 = Transform(LocalEnd1);
        Lines.Add(Line1);

        // 添加第二条腿 | Add second leg
        FLineSeg Line2;
        Line2.P1 = ScreenVert;
        Line2.P2 = Transform(LocalEnd2);
        Lines.Add(Line2);
    }

    // --- Phase 2: 渲染回调 | Rendering Callback ---
    // 使用基类的 RenderWithOutline，传入 Visuals
    // Use base class RenderWithOutline, passing in Visuals
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
