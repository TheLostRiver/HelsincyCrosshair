// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "Library/HelsincyDamageIndicatorPlacementResolver.h"
#include "UObject/Object.h"
#include "HelsincyIndicatorRenderer.generated.h"

/**
 * 伤害指示器渲染策略基类
 * Base class for damage indicator rendering strategy
 */
UCLASS(Abstract, Blueprintable) // [关键] 允许蓝图继承 | [Key] Allow blueprint inheritance
class HELSINCYDAMAGEINDICATOR_API UHelsincyIndicatorRenderer : public UObject
{
	GENERATED_BODY()

public:
	// 用于自动注册的 Tag | Tag used for auto-registration
	UPROPERTY(EditDefaultsOnly, Category = "Config", meta = (Categories = "Indicator.Style"))
	FGameplayTag AssociatedTag;

	/**
	 * @param Canvas 画布 | Canvas
	 * @param Profile 配置数据 | Configuration data
	 * @param Center 屏幕物理中心 | Screen physical center
	 * @param Angle 指示器在屏幕上的角度 (0=上, 90=右) | Indicator angle on screen (0=top, 90=right)
	 * @param Alpha 当前透明度 (已计算好淡入淡出) | Current opacity (fade in/out already calculated)
	 * @param Scale 自适应分辨率缩放 | Adaptive resolution scaling
	 */
	virtual void DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale);

	/**
	 * Return the visible pointer size used by placement resolution.
	 * Base returns zero so legacy renderers keep their old radial behavior.
	 */
	virtual FVector2D GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& Profile, float Scale) const;

	/**
	 * V2 draw path. Built-in renderers draw at Placement.Position.
	 * Base fallback calls the legacy DrawPointer() with Placement.Origin.
	 */
	virtual void DrawPointerResolved(
		UCanvas* Canvas,
		const FHelsincy_DamageIndicatorProfile& Profile,
		const FHelsincy_ResolvedDamageIndicatorPlacement& Placement,
		float Alpha,
		float Scale);

protected:
	/**
	 * 蓝图入口函数 (Hook) | Blueprint entry function (Hook)
	 * 只有蓝图子类需要实现此事件。
	 * Only blueprint subclasses need to implement this event.
	 * @param Canvas 画布 | Canvas
	 * @param Profile 配置数据 | Configuration data
	 * @param Center 屏幕物理中心 | Screen physical center
	 * @param Angle 指示器在屏幕上的角度 (0=上, 90=右) | Indicator angle on screen (0=top, 90=right)
	 * @param Alpha 当前透明度 (已计算好淡入淡出) | Current opacity (fade in/out already calculated)
	 * @param Scale 自适应分辨率缩放 | Adaptive resolution scaling
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ACS", meta = (DisplayName = "OnDrawPointer"))
	void ReceiveDrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale);
	
};
