// Copyright , Helsincy Games. All Rights Reserved.


#include "IndicatorRenderer/HelsincyIndicatorRendererArrow.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"


UHelsincyIndicatorRendererArrow::UHelsincyIndicatorRendererArrow()
{
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.Arrow"));
}

FVector2D UHelsincyIndicatorRendererArrow::GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& Profile, float Scale) const
{
	const float Size = FMath::Max(0.0f, Profile.ArrowConfig.Size * Scale);
	if (Profile.PlacementMode == EHelsincyDamageIndicatorPlacementMode::RadialCircle)
	{
		return FVector2D(Size, Size);
	}

	const float BoundsSize = Size * 1.41421356f;
	return FVector2D(BoundsSize, BoundsSize);
}

void UHelsincyIndicatorRendererArrow::DrawPointerResolved(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, const FHelsincy_ResolvedDamageIndicatorPlacement& Placement, float Alpha, float Scale)
{
	if (!Canvas || !Placement.bShouldDraw)
	{
		return;
	}

	const auto& Cfg = Profile.ArrowConfig;
	FLinearColor Color = Cfg.Color;
	Color.A = Cfg.Color.A * Alpha * Profile.PointerMaxOpacity;

	FVector2D DirOut = Placement.Direction;
	if (DirOut.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		DirOut = FVector2D(0.0f, -1.0f);
	}
	else
	{
		DirOut.Normalize();
	}
	const FVector2D DirRight(DirOut.Y, -DirOut.X);

	const float HalfSize = Cfg.Size * Scale * 0.5f;
	const FVector2D V1 = Placement.Position + DirOut * HalfSize;
	const FVector2D Base = Placement.Position - DirOut * HalfSize;
	const FVector2D V2 = Base + DirRight * HalfSize;
	const FVector2D V3 = Base - DirRight * HalfSize;

	FCanvasTriangleItem Tri(V1, V2, V3, GWhiteTexture);
	Tri.SetColor(Color);
	Tri.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tri);
}

void UHelsincyIndicatorRendererArrow::DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale)
{
	// 不要调用 Super::DrawPointer(...) ! | Do NOT call Super::DrawPointer(...)!
	
	const auto& Cfg = Profile.ArrowConfig;
    
	// 计算最终颜色 | Calculate final color
	FLinearColor Color = Cfg.Color;
	Color.A = Cfg.Color.A * Alpha * Profile.PointerMaxOpacity;

	// 计算位置半径 | Calculate position radius
	float Radius = Profile.Radius * Scale;
	float Offset = (Cfg.Size * Scale * 0.5f) + Profile.PointerDistanceOffset * Scale;
	Radius = Profile.bPointerOutsideCircle ? (Radius + Offset) : (Radius - Offset);

	float RadAngle = FMath::DegreesToRadians(Angle);
	float ScreenX = Center.X + FMath::Sin(RadAngle) * Radius;
	float ScreenY = Center.Y - FMath::Cos(RadAngle) * Radius;

	// 构造三角形几何 | Construct triangle geometry
	FVector2D ArrowCenter(ScreenX, ScreenY);
	FVector2D DirOut(FMath::Sin(RadAngle), -FMath::Cos(RadAngle));
	FVector2D DirRight(DirOut.Y, -DirOut.X);

	float HalfSize = Cfg.Size * Scale * 0.5f;
	FVector2D V1 = ArrowCenter + DirOut * HalfSize;
	FVector2D Base = ArrowCenter - DirOut * HalfSize;
	FVector2D V2 = Base + DirRight * HalfSize;
	FVector2D V3 = Base - DirRight * HalfSize;

	FCanvasTriangleItem Tri(V1, V2, V3, GWhiteTexture);
	Tri.SetColor(Color);
	Tri.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tri);
}
