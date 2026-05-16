// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererWings.h"
#include "Engine/Canvas.h"


UHelsincyRendererWings::UHelsincyRendererWings()
{
    // 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
    // Using RequestGameplayTag because static variable init order may be undefined; Name is safest
    // 或者确信 FACS_Tags 已经初始化
    // Or ensure FACS_Tags has already been initialized
    AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Wings"));
}

void UHelsincyRendererWings::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
    // 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
    
	if (!Canvas) return;

    // 1. 获取配置引用 | 1. Get config references
    const auto& Vis = Profile.VisualsConfig;
    const auto& Cfg = Profile.WingsConfig;

    // Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
    // Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions

    // --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    
    int32 Count = FMath::Max(2, Cfg.WingsLineCount);
    float Spacing = Cfg.WingsVerticalSpacing * Scale;
    
    // Spread.X 控制左右开合 | Spread.X controls left/right gap
    float HalfGap = ((Cfg.Gap * Scale) + (Spread.X * Scale)) * 0.5f;

    // 预计算旋转 | Pre-calculate rotation
    float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
    bool bRot = !FMath::IsNearlyZero(RadAngle);
    float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
    float S = bRot ? FMath::Sin(RadAngle) : 0.0f;
    
    // 缓存数组 (Count条横线*2侧 + 2条竖线, 最多 20*2+2=42)
    // Cache array (Count horizontal lines * 2 sides + 2 vertical lines, max 20*2+2=42)
    TArray<FLineSeg, TInlineAllocator<48>> Lines;
    Lines.Reserve((Count * 2) + 2);

    // 辅助 Lambda: 添加并旋转线段 | Helper Lambda: add and rotate line segment
    auto AddLine = [&](FVector2D P1, FVector2D P2)
    {
        if (bRot) {
            float TX1 = P1.X; P1.X = TX1 * C - P1.Y * S; P1.Y = TX1 * S + P1.Y * C;
            float TX2 = P2.X; P2.X = TX2 * C - P2.Y * S; P2.Y = TX2 * S + P2.Y * C;
        }
        // 使用显式构造防止编译器推导歧义 | Explicit construction to avoid compiler deduction ambiguity
        FLineSeg NewLine;
        NewLine.P1 = Center + P1;
        NewLine.P2 = Center + P2;
        Lines.Add(NewLine);
    };

    // --- 计算垂直位置逻辑 | Vertical position calculation logic ---

    // 1. 计算准星总高度 | 1. Calculate total crosshair height
    float TotalHeight = static_cast<float>(Count - 1) * Spacing;
    
    // 2. 决定起始 Y 坐标 (第一条线的位置)
    // 2. Determine starting Y coordinate (position of the first line)
    float StartY = 0.0f;

    if (Cfg.bWingsAlignTop)
    {
        // 顶部对齐模式：第一条线就在 0 (屏幕中心)
        // Top-align mode: first line at 0 (screen center)
        StartY = 0.0f;
    }
    else
    {
        // 中心对齐模式：第一条线在 -TotalH / 2 (向上偏移一半高度)
        // Center-align mode: first line at -TotalH / 2 (offset half height upward)
        StartY = -TotalHeight * 0.5f;
    }

    // 3. 加上手动微调偏移 (Vertical Offset) + 垂直扩散
    // 3. Add manual offset (Vertical Offset) + vertical spread
    StartY += Cfg.WingsVerticalOffset * Scale + Spread.Y * Scale;

    // --- 生成几何体 | Generate geometry ---

    // A. 生成竖线 (垂直主干) | A. Generate vertical lines (main stems)
    if (Cfg.bWingsDrawVerticalLines)
    {
        float EndY = StartY + TotalHeight;
        AddLine(FVector2D(-HalfGap, StartY), FVector2D(-HalfGap, EndY)); // 左主干 | Left stem
        AddLine(FVector2D(HalfGap, StartY), FVector2D(HalfGap, EndY));   // 右主干 | Right stem
    }

    // B. 生成横线 (阶梯) | B. Generate horizontal lines (steps)
    for (int32 i = 0; i < Count; i++)
    {
        float Y = StartY + (i * Spacing);
        
        // 计算插值比例 (0.0 ~ 1.0) | Calculate interpolation alpha (0.0 ~ 1.0)
        float Alpha = (Count > 1) ? static_cast<float>(i) / static_cast<float>(Count - 1) : 0.0f;
        
        // 根据比例计算当前横线的长度 (从 TopLength 渐变到 BottomLength)
        // Calculate current line length based on alpha (lerp from TopLength to BottomLength)
        float Len = FMath::Lerp(Cfg.WingsTopLength * Scale, Cfg.WingsBottomLength * Scale, Alpha);

        // 左翼横线 (向左延伸) | Left wing line (extends left)
        AddLine(FVector2D(-HalfGap, Y), FVector2D(-HalfGap - Len, Y)); 
        
        // 右翼横线 (向右延伸) | Right wing line (extends right)
        AddLine(FVector2D(HalfGap, Y), FVector2D(HalfGap + Len, Y));   
    }

    // --- Phase 2: 渲染回调 | Rendering Callback ---
    // 调用基类的 RenderWithOutline，传入 Visuals 设置
    // Call base class RenderWithOutline, passing in Visuals settings
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
