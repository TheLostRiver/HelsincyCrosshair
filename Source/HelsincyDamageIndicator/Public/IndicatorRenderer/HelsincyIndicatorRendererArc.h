// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IndicatorRenderer/HelsincyIndicatorRenderer.h"
#include "HelsincyIndicatorRendererArc.generated.h"

class UTexture2D;

/**
 * @brief 弧形伤害指示器渲染器。
 *        Renderer for COD-like arc damage indicators.
 */
UCLASS()
class HELSINCYDAMAGEINDICATOR_API UHelsincyIndicatorRendererArc : public UHelsincyIndicatorRenderer
{
	GENERATED_BODY()

public:
	/**
	 * @brief 初始化 Arc renderer 并绑定 `Indicator.Style.Arc`。
	 *        Initializes the Arc renderer and binds `Indicator.Style.Arc`.
	 */
	UHelsincyIndicatorRendererArc();

	/**
	 * @brief 返回 placement resolver 用于边界预留的可见尺寸。
	 *        Returns visible size used by the placement resolver for bounds reservation.
	 */
	virtual FVector2D GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& Profile, float Scale) const override;

	/**
	 * @brief 使用 resolved placement 绘制弧形指示器。
	 *        Draws the arc indicator using resolved placement.
	 */
	virtual void DrawPointerResolved(
		UCanvas* Canvas,
		const FHelsincy_DamageIndicatorProfile& Profile,
		const FHelsincy_ResolvedDamageIndicatorPlacement& Placement,
		float Alpha,
		float Scale) override;

	/**
	 * @brief 兼容旧 renderer API 的径向绘制入口。
	 *        Legacy radial draw entry kept for renderer API compatibility.
	 */
	virtual void DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale) override;

private:
	/**
	 * @brief 解析弧光贴图；用户贴图为空时返回缓存的默认 mask。
	 *        Resolves the arc mask; returns the cached default mask when the user texture is null.
	 */
	UTexture2D* ResolveArcMaskTexture(const FHelsincy_DamageIndicatorProfile& Profile);

	/**
	 * @brief 绘制程序化方向提示。
	 *        Draws the procedural direction cue.
	 */
	void DrawDirectionCue(
		UCanvas* Canvas,
		const FHelsincy_DamageIndicatorProfile& Profile,
		const FHelsincy_ResolvedDamageIndicatorPlacement& Placement,
		float Alpha,
		float Scale) const;

	/**
	 * @brief 生成默认弧光 alpha mask。
	 *        Generates the default arc alpha mask.
	 */
	UTexture2D* GenerateDefaultArcMask();

	/** 运行时生成的默认弧光 mask 缓存。 | Runtime-generated default arc mask cache. */
	UPROPERTY(Transient)
	UTexture2D* DefaultArcMaskTexture = nullptr;
};
