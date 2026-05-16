// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererCircle.h"
#include "Engine/Canvas.h"
#include "Subsystems/HelsincyCrosshairManagerSubsystem.h"


UHelsincyRendererCircle::UHelsincyRendererCircle()
{
	// 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
	// Using RequestGameplayTag because static variable init order may be undefined; Name is safest
	// 或者确信 FACS_Tags 已经初始化
	// Or ensure FACS_Tags has already been initialized
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Circle"));
}

void UHelsincyRendererCircle::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
    // 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
    
	if (!Canvas) return;

    // 1. 获取配置引用 | 1. Get config references
    const auto& Vis = Profile.VisualsConfig;
    const auto& Cfg = Profile.RadialConfig;

    // --- Phase 1: 几何预计算 | Geometry Pre-calculation ---
    
    // 限制分段数，防止性能过剩或视觉棱角
    // Clamp segment count to prevent excessive performance cost or visual artifacts
    int32 Segments = FMath::Clamp(Cfg.CircleSegments, 8, 360);

    float ScaledRadius = Cfg.Radius * Scale;
    // Spread 本身是像素单位，也需要缩放，否则高分辨率下准星扩的不够开
    // Spread is in pixels and also needs scaling, otherwise crosshair doesn't expand enough at high resolutions
    FVector2D ScaledSpread = Spread * Scale;
    
    // 各向异性半径计算 | Anisotropic Radius calculation
    // Spread.X 撑开宽度，Spread.Y 撑开高度 | Spread.X stretches width, Spread.Y stretches height
    float RadiusX = ScaledRadius + ScaledSpread.X;
    float RadiusY = ScaledRadius + ScaledSpread.Y;

    // 预计算旋转矩阵 | Pre-calculate rotation matrix
    float RadAngle = FMath::DegreesToRadians(Cfg.Rotation);
    bool bRot = !FMath::IsNearlyZero(RadAngle);
    float C = bRot ? FMath::Cos(RadAngle) : 1.0f;
    float S = bRot ? FMath::Sin(RadAngle) : 0.0f;

    // 预计算所有屏幕空间顶点 | Pre-calculate all screen-space vertices
    // 缓存数组大小为 Segments + 1 (为了闭合圆环，最后一点重合第一点)
    // Cache array size is Segments + 1 (last point overlaps first for ring closure)
    TArray<FVector2D, TInlineAllocator<362>> Vertices;
    Vertices.Reserve(Segments + 1);

    float Step = PI * 2.0f / static_cast<float>(Segments);

    for (int32 i = 0; i <= Segments; i++)
    {
        float Angle = i * Step;

        // A. 计算本地椭圆坐标 (未旋转)
        // A. Calculate local ellipse coordinates (before rotation)
        // X = Rx * Cos(A)
        // Y = Ry * Sin(A)
        float LX = RadiusX * FMath::Cos(Angle);
        float LY = RadiusY * FMath::Sin(Angle);

        // B. 应用旋转矩阵 | B. Apply rotation matrix
        if (bRot)
        {
            float TX = LX;
            LX = TX * C - LY * S;
            LY = TX * S + LY * C;
        }

        // C. 加上中心偏移并存入数组 | C. Add center offset and store in array
        Vertices.Add(Center + FVector2D(LX, LY));
    }

    UTexture2D* SmoothTexture {nullptr};
    if (auto* Subsystem = GetCrosshairManagerSubsystem())
    {
        SmoothTexture = Subsystem->GetSmoothLineTexture();
    }

    // --- Phase 2: 渲染回调 | Rendering Callback ---
    // 调用基类的 RenderWithOutline，传入 Visuals 设置
    // Call base class RenderWithOutline, passing in Visuals settings
    // 这个函数会自动处理描边层(黑色)和主体层(颜色)的两次绘制
    // This function auto-handles two passes: outline (black) and core (color)
    RenderWithOutline(Vis, CurrentColor, Scale, [&](float Thickness, FLinearColor Color)
    {
        // 遍历顶点数组，将相邻点连成线
        // Iterate vertex array, connecting adjacent points as lines
        // 稍微补偿一点粗细，因为软边缘会让视觉变细
        // Slightly compensate thickness, as soft edges make it appear thinner
        float SoftThickness = Thickness + 1.0f;

        // 获取抗锯齿纹理资源 | Get anti-aliased texture resource
        FTexture* TextureResource = nullptr;
        if (SmoothTexture && SmoothTexture->GetResource())
        {
            TextureResource = SmoothTexture->GetResource();
        }
        else
        {
            // 如果没生成成功，回退到默认白图，但还是用 TileItem 画
            // If texture generation failed, fall back to default white texture, still draw with TileItem
            TextureResource = GWhiteTexture;
        }

        for (int32 i = 1; i < Vertices.Num(); i++)
        {
            FVector2D P1 = Vertices[i-1];
            FVector2D P2 = Vertices[i];

            // --- 核心修改：用 TileItem 模拟 LineItem | Core: simulate LineItem with TileItem ---
            
            // 1. 计算向量差 | 1. Calculate vector difference
            FVector2D Delta = P2 - P1;
            float Length = Delta.Size();
            
            if (Length <= 0.0f) continue;

            // 2. 计算角度 (Atan2 返回弧度，需转为角度)
            // 2. Calculate angle (Atan2 returns radians, convert to degrees)
            float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));

            // 3. 创建 TileItem | 3. Create TileItem
            // 初始位置设为 P1 (起点) | Initial position set to P1 (start point)
            // 大小设为 (长度, 粗细) | Size set to (length, thickness)
            FCanvasTileItem Tile(P1, TextureResource, FVector2D(Length, SoftThickness), Color);
            
            // 4. 设置旋转锚点 (Pivot) | 4. Set rotation anchor (Pivot)
            // (0, 0.5) 表示：X轴从左边开始旋转，Y轴在中间
            // (0, 0.5): X-axis rotates from left edge, Y-axis centered
            // 这样 P1 就会是线条的起点中心
            // This makes P1 the center of the line's start point
            Tile.PivotPoint = FVector2D(0.0f, 0.5f);
            
            // 5. 设置旋转 | 5. Set rotation
            Tile.Rotation = FRotator(0.0f, AngleDeg, 0.0f);

            // 6. 混合模式 | 6. Blend mode
            Tile.BlendMode = SE_BLEND_Translucent;

            Canvas->DrawItem(Tile);
        }
    });
}

UHelsincyCrosshairManagerSubsystem* UHelsincyRendererCircle::GetCrosshairManagerSubsystem()
{
    if (HCSubsystem) return HCSubsystem;

    if (GEngine)
    {
        HCSubsystem = GEngine->GetEngineSubsystem<UHelsincyCrosshairManagerSubsystem>();
    }
    return HCSubsystem;
}
