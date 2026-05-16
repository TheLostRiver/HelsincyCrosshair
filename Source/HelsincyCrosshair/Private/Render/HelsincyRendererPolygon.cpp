// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererPolygon.h"
#include "Engine/Canvas.h"


UHelsincyRendererPolygon::UHelsincyRendererPolygon()
{
    // 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
    // Using RequestGameplayTag because static variable init order may be undefined; Name is safest
    // 或者确信 FACS_Tags 已经初始化
    // Or ensure FACS_Tags has already been initialized
    AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Polygon"));
}

void UHelsincyRendererPolygon::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
    // 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
    
	if (!Canvas) return;

    const auto& Vis = Profile.VisualsConfig;
    const auto& Cfg = Profile.RadialConfig; // 使用 RadialConfig | Using RadialConfig

    // Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
    // Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
    FVector2D ScaledSpread = Spread * Scale;
    float ScaledRadius = Cfg.Radius * Scale;

    // --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    int32 Sides = FMath::Clamp(Cfg.PolygonSides, 3, 30);
    float RX = ScaledRadius + ScaledSpread.X;
    float RY = ScaledRadius + ScaledSpread.Y;

    float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
    bool bRot = !FMath::IsNearlyZero(RadAngle);
    float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
    float S = bRot ? FMath::Sin(RadAngle) : 0.0f;

    // 预计算所有屏幕空间顶点 | Pre-calculate all screen-space vertices
    TArray<FVector2D, TInlineAllocator<40>> Vertices;
    Vertices.SetNumUninitialized(Sides + 1); // +1 为了闭合 | +1 for closure

    float Step = PI * 2.0f / static_cast<float>(Sides);
    for (int32 i = 0; i <= Sides; i++)
    {
        float A = i * Step;
        // 计算本地坐标 | Calculate local coordinates
        float LX = RX * FMath::Cos(A);
        float LY = RY * FMath::Sin(A);

        // 应用旋转并加上中心 | Apply rotation and add center offset
        if (bRot)
        {
            float TX = LX;
            LX = TX * C - LY * S;
            LY = TX * S + LY * C;
        }
        Vertices[i] = Center + FVector2D(LX, LY);
    }

    // --- Phase 2: 渲染回调 | Rendering Callback ---
    RenderWithOutline(Vis, CurrentColor, Scale, [&](float Thickness, FLinearColor Color)
    {
        for (int32 i = 1; i < Vertices.Num(); i++)
        {
            FCanvasLineItem Line(Vertices[i-1], Vertices[i]);
            Line.SetColor(Color);
            Line.LineThickness = Thickness;
            Line.BlendMode = SE_BLEND_Translucent;
            Canvas->DrawItem(Line);
        }
    });
}
