// Copyright , Helsincy Games. All Rights Reserved.


#include "IndicatorRenderer/HelsincyIndicatorRendererImage.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"


UHelsincyIndicatorRendererImage::UHelsincyIndicatorRendererImage()
{
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.Image"));
}

FVector2D UHelsincyIndicatorRendererImage::GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& Profile, float Scale) const
{
	const FVector2D ScaledSize(
		FMath::Max(0.0f, Profile.ImageConfig.Size.X * Scale),
		FMath::Max(0.0f, Profile.ImageConfig.Size.Y * Scale));
	if (Profile.PlacementMode == EHelsincyDamageIndicatorPlacementMode::RadialCircle)
	{
		return ScaledSize;
	}

	const float BoundsSize = FMath::Max(ScaledSize.X, ScaledSize.Y) * 1.41421356f;
	return FVector2D(BoundsSize, BoundsSize);
}

void UHelsincyIndicatorRendererImage::DrawPointerResolved(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, const FHelsincy_ResolvedDamageIndicatorPlacement& Placement, float Alpha, float Scale)
{
	if (!Canvas || !Placement.bShouldDraw)
	{
		return;
	}

	const auto& Cfg = Profile.ImageConfig;
	if (!Cfg.Texture || !Cfg.Texture->GetResource()) return;

	FLinearColor Color = Cfg.Tint;
	Color.A = Cfg.Tint.A * Alpha * Profile.PointerMaxOpacity;

	const FVector2D DrawSize(
		FMath::Max(0.0f, Cfg.Size.X * Scale),
		FMath::Max(0.0f, Cfg.Size.Y * Scale));
	const FVector2D DrawPos = Placement.Position - DrawSize * 0.5f;

	FCanvasTileItem Tile(DrawPos, Cfg.Texture->GetResource(), DrawSize, Color);
	Tile.BlendMode = SE_BLEND_Translucent;

	if (Cfg.bRotateImage)
	{
		Tile.PivotPoint = FVector2D(0.5f, 0.5f);
		Tile.Rotation = FRotator(0.0f, Placement.RotationAngle + Cfg.RotationOffset, 0.0f);
	}

	Canvas->DrawItem(Tile);
}

void UHelsincyIndicatorRendererImage::DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale)
{
	// 不要调用 Super::DrawPointer(...) ! | Do NOT call Super::DrawPointer(...)!
	
	const auto& Cfg = Profile.ImageConfig;
	if (!Cfg.Texture || !Cfg.Texture->GetResource()) return;

	// 计算最终颜色 | Calculate final color
	FLinearColor Color = Cfg.Tint;
	Color.A = Cfg.Tint.A * Alpha * Profile.PointerMaxOpacity;

	// 计算位置半径 (基于图片的最大边长的一半作为 Offset)
	// Calculate position radius (use half of image's max dimension as offset)
	float MaxDim = FMath::Max(Cfg.Size.X * Scale, Cfg.Size.Y * Scale);
	float Radius = Profile.Radius * Scale;
	float Offset = (MaxDim * 0.5f) + Profile.PointerDistanceOffset * Scale;
    
	Radius = Profile.bPointerOutsideCircle ? (Radius + Offset) : (Radius - Offset);

	float RadAngle = FMath::DegreesToRadians(Angle);
	float ScreenX = Center.X + FMath::Sin(RadAngle) * Radius;
	float ScreenY = Center.Y - FMath::Cos(RadAngle) * Radius;

	// 绘制图片 | Draw image
	FVector2D DrawPos = FVector2D(ScreenX, ScreenY) - (Cfg.Size * Scale * 0.5f);

	FCanvasTileItem Tile(DrawPos, Cfg.Texture->GetResource(), Cfg.Size * Scale, Color);
	Tile.BlendMode = SE_BLEND_Translucent;
    
	// 设置旋转 | Set rotation
	if (Cfg.bRotateImage)
	{
		Tile.PivotPoint = FVector2D(0.5f, 0.5f);
		Tile.Rotation = FRotator(0.0f, Angle + Cfg.RotationOffset, 0.0f);
	}

	Canvas->DrawItem(Tile);
}
