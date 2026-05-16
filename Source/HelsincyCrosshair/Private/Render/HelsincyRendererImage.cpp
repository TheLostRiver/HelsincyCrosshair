// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Render/HelsincyRendererImage.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"


UHelsincyRendererImage::UHelsincyRendererImage()
{
	// 这里使用 RequestGameplayTag 是因为静态变量初始化顺序可能不确定，用 Name 最稳妥
	// Using RequestGameplayTag because static variable init order may be undefined; Name is safest
	// 或者确信 FACS_Tags 已经初始化
	// Or ensure FACS_Tags has already been initialized
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.Image"));
}

void UHelsincyRendererImage::Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
{
	// 不要调用 Super::Draw(...)! | Do NOT call Super::Draw(...)!
	
	if (!Canvas) return;

	// 1. 获取配置 | 1. Get config
	const auto& Vis = Profile.VisualsConfig;
	const auto& Cfg = Profile.ImageConfig;

	// 必须有贴图资源 | Must have texture resource
	if (!Cfg.Texture || !Cfg.Texture->GetResource()) return;

	// 2. 计算最终大小 | 2. Calculate final size
	// 逻辑：基础大小 + 扩散值
	// Logic: base size + spread value
	// 这样当角色移动时，图片会变大，提供动态反馈
	// This way the image grows when the character moves, providing dynamic feedback
	// 如果不想要动态大小，可以在 Profile 里把 bEnableDynamicSpread 关掉，Spread 就会是 0
	// To disable dynamic sizing, turn off bEnableDynamicSpread in Profile, then Spread will be 0
	FVector2D FinalSize = (Cfg.Size * Scale) + (Spread * Scale);

	// 3. 计算绘制位置 (居中) | 3. Calculate draw position (centered)
	// Canvas 绘制是以左上角为锚点的，所以需要减去半个大小
	// Canvas draws from top-left anchor, so subtract half the size
	FVector2D DrawPos = Center - (FinalSize * 0.5f);

	// 4. 计算最终颜色 | 4. Calculate final color
	FLinearColor FinalColor = Cfg.Tint * CurrentColor;
	FinalColor.A *= Vis.Opacity; // 叠加全局透明度 | Multiply by global opacity

	// 5. 绘制 TileItem | 5. Draw TileItem
	FCanvasTileItem TileItem(DrawPos, Cfg.Texture->GetResource(), FinalSize, FinalColor);
    
	// 图片通常带有 Alpha 通道，必须使用半透明混合
	// Images usually have alpha channels, must use translucent blending
	TileItem.BlendMode = SE_BLEND_Translucent;
    
	// 如果你想支持旋转，可以使用 TileItem.Rotation 和 PivotPoint
	// To support rotation, use TileItem.Rotation and PivotPoint
	// 但目前的 FACS_ImageConfig 没有定义 Rotation 属性。
	// However FACS_ImageConfig currently has no Rotation property.
	// 如果未来需要，可以这样写：
	// If needed in the future, you can write:
	// TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
	// TileItem.Rotation = FRotator(0.0f, SomeRotationValue, 0.0f);

	Canvas->DrawItem(TileItem);

	// 注意：图片渲染器通常不支持程序化描边 (Outline)。
	// Note: Image renderer typically doesn't support procedural outlines.
	// 因为 FCanvas 无法知道图片的非透明区域边缘在哪里。
	// Because FCanvas cannot determine where the non-transparent region edges are.
	// 建议直接使用带描边的 PNG 图片。
	// Recommend using PNG images with pre-baked outlines.
}
