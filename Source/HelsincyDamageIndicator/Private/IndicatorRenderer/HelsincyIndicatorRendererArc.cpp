// Copyright , Helsincy Games. All Rights Reserved.

#include "IndicatorRenderer/HelsincyIndicatorRendererArc.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"

namespace
{
	void DrawRotatedArcTile(
		UCanvas* Canvas,
		UTexture2D* Texture,
		const FVector2D& Center,
		const FVector2D& Size,
		const FLinearColor& Color,
		float RotationAngle)
	{
		if (!Canvas || !Texture || !Texture->GetResource() || Size.X <= 0.0f || Size.Y <= 0.0f)
		{
			return;
		}

		FCanvasTileItem Tile(Center - Size * 0.5f, Texture->GetResource(), Size, Color);
		Tile.BlendMode = SE_BLEND_Translucent;
		Tile.PivotPoint = FVector2D(0.5f, 0.5f);
		Tile.Rotation = FRotator(0.0f, RotationAngle, 0.0f);
		Canvas->DrawItem(Tile);
	}

	void DrawCenterNibCue(UCanvas* Canvas, const FVector2D& Center, const FVector2D& DirOut, const FVector2D& DirRight, const FVector2D& CueSize, const FLinearColor& Color)
	{
		// 中文：短楔形从弧光中心向屏幕内侧刺入，给玩家一个明确的受击方向锚点。
		// English: The short wedge points inward from the arc center, giving players a clear incoming-hit anchor.
		const FVector2D Tip = Center - DirOut * CueSize.Y;
		const FVector2D Base = Center + DirOut * (CueSize.Y * 0.25f);

		FCanvasTriangleItem Tri(Tip, Base + DirRight * (CueSize.X * 0.5f), Base - DirRight * (CueSize.X * 0.5f), GWhiteTexture);
		Tri.SetColor(Color);
		Tri.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Tri);
	}

	void DrawCenterChevronCue(UCanvas* Canvas, const FVector2D& Center, const FVector2D& DirOut, const FVector2D& DirRight, const FVector2D& CueSize, const FLinearColor& Color)
	{
		// 中文：双线 V 形比楔形更轻，适合希望弧光主体更干净的配置。
		// English: The two-line chevron is lighter than the wedge for profiles that should keep the arc body cleaner.
		const FVector2D Inner = Center - DirOut * CueSize.Y;
		const FVector2D Left = Center + DirRight * (CueSize.X * 0.5f);
		const FVector2D Right = Center - DirRight * (CueSize.X * 0.5f);
		const float Thickness = FMath::Max(1.0f, CueSize.X * 0.12f);

		FCanvasLineItem LineA(Left, Inner);
		LineA.SetColor(Color);
		LineA.LineThickness = Thickness;
		LineA.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(LineA);

		FCanvasLineItem LineB(Right, Inner);
		LineB.SetColor(Color);
		LineB.LineThickness = Thickness;
		LineB.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(LineB);
	}

	void DrawAsymmetricTaperCue(UCanvas* Canvas, const FVector2D& Center, const FVector2D& DirOut, const FVector2D& DirRight, const FVector2D& CueSize, const FLinearColor& Color)
	{
		// 中文：偏心锥形让弧光一端更锐利，在不额外依赖贴图的情况下提供方向感。
		// English: The off-center taper sharpens one side of the arc, adding directionality without another texture.
		const FVector2D Tip = Center - DirOut * CueSize.Y + DirRight * (CueSize.X * 0.35f);
		const FVector2D Base = Center + DirOut * (CueSize.Y * 0.2f) - DirRight * (CueSize.X * 0.2f);

		FCanvasTriangleItem Tri(Tip, Base + DirRight * CueSize.X, Base - DirRight * (CueSize.X * 0.3f), GWhiteTexture);
		Tri.SetColor(Color);
		Tri.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Tri);
	}
}

UHelsincyIndicatorRendererArc::UHelsincyIndicatorRendererArc()
{
	// 中文：构造阶段只绑定 tag，不做纹理分配；默认 mask 会在后续绘制路径中按需生成。
	// English: Bind the tag in the constructor only; default mask allocation is deferred to the draw path.
	AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.Arc"));
}

FVector2D UHelsincyIndicatorRendererArc::GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& Profile, float Scale) const
{
	const FVector2D ScaledSize(
		FMath::Max(0.0f, Profile.ArcConfig.Size.X * Scale),
		FMath::Max(0.0f, Profile.ArcConfig.Size.Y * Scale));

	if (Profile.PlacementMode == EHelsincyDamageIndicatorPlacementMode::RadialCircle)
	{
		return ScaledSize;
	}

	const float BoundsSize = FMath::Max(ScaledSize.X, ScaledSize.Y) * 1.41421356f;
	return FVector2D(BoundsSize, BoundsSize);
}

void UHelsincyIndicatorRendererArc::DrawPointerResolved(
	UCanvas* Canvas,
	const FHelsincy_DamageIndicatorProfile& Profile,
	const FHelsincy_ResolvedDamageIndicatorPlacement& Placement,
	float Alpha,
	float Scale)
{
	if (!Canvas || !Placement.bShouldDraw)
	{
		return;
	}

	// 中文：弧光主体始终按贴图路径绘制；用户贴图为空时 ResolveArcMaskTexture 会返回默认生成的 mask。
	// English: The arc body always uses the texture path; ResolveArcMaskTexture returns the generated default mask when the user texture is null.
	UTexture2D* ArcTexture = ResolveArcMaskTexture(Profile);
	if (!ArcTexture)
	{
		return;
	}

	const FHelsincy_Ind_ArcConfig& Cfg = Profile.ArcConfig;
	const float FinalAlpha = Alpha * Profile.PointerMaxOpacity;
	const FVector2D DrawSize(
		FMath::Max(0.0f, Cfg.Size.X * Scale),
		FMath::Max(0.0f, Cfg.Size.Y * Scale));

	FLinearColor GlowColor = Cfg.Color;
	GlowColor.A *= FinalAlpha * Cfg.GlowOpacity;
	const FVector2D GlowSize = DrawSize * FMath::Max(1.0f, Cfg.GlowScale);
	DrawRotatedArcTile(Canvas, ArcTexture, Placement.Position, GlowSize, GlowColor, Placement.RotationAngle);

	FLinearColor CoreColor = Cfg.Color;
	CoreColor.A *= FinalAlpha;
	DrawRotatedArcTile(Canvas, ArcTexture, Placement.Position, DrawSize, CoreColor, Placement.RotationAngle);

	DrawDirectionCue(Canvas, Profile, Placement, FinalAlpha, Scale);
}

void UHelsincyIndicatorRendererArc::DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale)
{
	if (!Canvas)
	{
		return;
	}

	// 中文：旧 API 没有 resolved placement，这里只复刻 RadialCircle 语义以保持自定义调用兼容。
	// English: The legacy API has no resolved placement, so this path mirrors RadialCircle semantics for compatibility.
	const FVector2D ScaledSize(
		FMath::Max(0.0f, Profile.ArcConfig.Size.X * Scale),
		FMath::Max(0.0f, Profile.ArcConfig.Size.Y * Scale));
	const float PointerExtent = FMath::Max(ScaledSize.X, ScaledSize.Y) * 0.5f;
	float Radius = Profile.Radius * Scale + Profile.PointerDistanceOffset * Scale + PointerExtent;
	if (!Profile.bPointerOutsideCircle)
	{
		Radius = Profile.Radius * Scale - Profile.PointerDistanceOffset * Scale - PointerExtent;
	}

	const float RadAngle = FMath::DegreesToRadians(Angle);
	FVector2D Direction(FMath::Sin(RadAngle), -FMath::Cos(RadAngle));
	if (Direction.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		Direction = FVector2D(0.0f, -1.0f);
	}
	else
	{
		Direction.Normalize();
	}
	const FVector2D Position = Center + Direction * Radius;

	FHelsincy_ResolvedDamageIndicatorPlacement Placement;
	Placement.Origin = Center;
	Placement.Position = Position;
	Placement.Direction = Direction;
	Placement.RotationAngle = Angle;
	Placement.bShouldDraw = true;
	DrawPointerResolved(Canvas, Profile, Placement, Alpha, Scale);
}

UTexture2D* UHelsincyIndicatorRendererArc::ResolveArcMaskTexture(const FHelsincy_DamageIndicatorProfile& Profile)
{
	if (Profile.ArcConfig.ArcMaskTexture)
	{
		return Profile.ArcConfig.ArcMaskTexture;
	}

	if (!DefaultArcMaskTexture)
	{
		DefaultArcMaskTexture = GenerateDefaultArcMask();
	}

	return DefaultArcMaskTexture;
}

void UHelsincyIndicatorRendererArc::DrawDirectionCue(
	UCanvas* Canvas,
	const FHelsincy_DamageIndicatorProfile& Profile,
	const FHelsincy_ResolvedDamageIndicatorPlacement& Placement,
	float Alpha,
	float Scale) const
{
	if (!Canvas || Alpha <= 0.0f || Profile.ArcConfig.DirectionCueStrength <= 0.0f)
	{
		return;
	}

	const FHelsincy_Ind_ArcConfig& Cfg = Profile.ArcConfig;
	if (Cfg.DirectionCueMode == EHelsincyDamageIndicatorArcDirectionCueMode::None)
	{
		return;
	}

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

	const float Strength = FMath::Max(0.0f, Cfg.DirectionCueStrength);
	const FVector2D CueSize = Cfg.CueSize * Scale * Strength;
	FLinearColor CueColor = Cfg.Color;
	CueColor.A *= Alpha;

	// 中文：cue mode 只控制 Arc renderer 内部图形，不改变 GameplayTag 级 renderer 选择。
	// English: Cue mode controls only the internal Arc geometry; renderer selection remains GameplayTag-based.
	switch (Cfg.DirectionCueMode)
	{
	case EHelsincyDamageIndicatorArcDirectionCueMode::None:
		return;
	case EHelsincyDamageIndicatorArcDirectionCueMode::CenterChevron:
		DrawCenterChevronCue(Canvas, Placement.Position, DirOut, DirRight, CueSize, CueColor);
		return;
	case EHelsincyDamageIndicatorArcDirectionCueMode::AsymmetricTaper:
		DrawAsymmetricTaperCue(Canvas, Placement.Position, DirOut, DirRight, CueSize, CueColor);
		return;
	case EHelsincyDamageIndicatorArcDirectionCueMode::CenterNib:
	default:
		DrawCenterNibCue(Canvas, Placement.Position, DirOut, DirRight, CueSize, CueColor);
		return;
	}
}

UTexture2D* UHelsincyIndicatorRendererArc::GenerateDefaultArcMask()
{
	constexpr int32 Width = 512;
	constexpr int32 Height = 128;

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture || !Texture->PlatformData || Texture->PlatformData->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	Texture->SRGB = 0;
	Texture->Filter = TextureFilter::TF_Bilinear;

	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!Data)
	{
		return nullptr;
	}

	FColor* Pixels = static_cast<FColor*>(Data);
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			// 中文：默认 mask 使用一条抛物线弧心线，叠加厚度衰减、两端渐隐和中心亮度增强。
			// English: The default mask uses a parabolic arc centerline with thickness falloff, tip fade, and center boost.
			const float U = (static_cast<float>(X) / static_cast<float>(Width - 1)) * 2.0f - 1.0f;
			const float V = static_cast<float>(Y) / static_cast<float>(Height - 1);
			const float ArcY = 0.66f - 0.24f * (1.0f - U * U);
			const float Distance = FMath::Abs(V - ArcY);
			const float CoreAlpha = FMath::Clamp(1.0f - Distance / 0.055f, 0.0f, 1.0f);
			const float SmoothedCore = CoreAlpha * CoreAlpha * (3.0f - 2.0f * CoreAlpha);
			const float TipAlpha = FMath::Clamp((1.0f - FMath::Abs(U)) / 0.18f, 0.0f, 1.0f);
			const float CenterBoost = FMath::Lerp(0.82f, 1.0f, FMath::Clamp(1.0f - FMath::Abs(U), 0.0f, 1.0f));
			const uint8 AlphaValue = static_cast<uint8>(FMath::Clamp(SmoothedCore * TipAlpha * CenterBoost, 0.0f, 1.0f) * 255.0f);
			Pixels[Y * Width + X] = FColor(255, 255, 255, AlphaValue);
		}
	}

	Mip.BulkData.Unlock();
	Texture->UpdateResource();
	return Texture;
}
