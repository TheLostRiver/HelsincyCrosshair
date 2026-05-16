// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/Function.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "HelsincyShapeRenderer.generated.h"

class UCanvas;

/**
 * 渲染策略基类
 * Renderer strategy base class
 * 所有具体的准星形状都继承此类
 * All concrete crosshair shapes inherit from this class
 */
UCLASS(Abstract, Blueprintable)
class HELSINCYCROSSHAIR_API UHelsincyShapeRenderer : public UObject
{
	GENERATED_BODY()

public:

	// 子类在构造函数中设置此Tag，用于自动注册
	// Subclass sets this Tag in constructor for automatic registration
	UPROPERTY(EditDefaultsOnly, Category = "HelsincyShapeRenderer", meta = (Categories = "Crosshair.Shape"))
	FGameplayTag AssociatedTag;
	
	// 纯虚函数的变体，供子类实现 | Virtual function variant for subclass implementation
	virtual void Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor CurrentColor, float DeltaTime, float Scale)
	{
		// 默认行为：调用蓝图事件
		// Default behavior: call Blueprint event
		// 如果这是个 C++ 子类（如 Renderer_Cross），它会覆盖这个函数且不调用 Super，
		// If this is a C++ subclass (e.g. Renderer_Cross), it overrides this function without calling Super,
		// 所以这行代码永远不会在高性能 C++ 渲染器中执行。
		// so this line will never execute in high-performance C++ renderers.
		if (Canvas) { ReceiveDraw(Canvas, Profile, Spread, Center, CurrentColor, DeltaTime, Scale); }
	}

protected:
	/**
	 * 蓝图入口函数 (Hook)
	 * Blueprint entry function (Hook)
	 * 只有当你是蓝图子类时，才需要实现这个事件。
	 * Only implement this event when you are a Blueprint subclass.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HelsincyShapeRenderer", meta = (DisplayName = "On Draw Crosshair"))
	void ReceiveDraw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D Center, FLinearColor FinalColor, float DeltaTime, float Scale);

	// --- 高效描边辅助函数 | Efficient Outline Helper ---
	// 这是一个通用算法：它会根据 Profile 配置，自动决定是调用 DrawCallback 一次还是两次
	// Generic algorithm: auto-decides whether to call DrawCallback once or twice based on Profile config
	// Callback 参数: (float CurrentThickness, FLinearColor CurrentColor)
	// Callback params: (float CurrentThickness, FLinearColor CurrentColor)
	void RenderWithOutline(const FHelsincy_VisualSettings& Visuals, FLinearColor CurrentColor, float Scale, TFunctionRef<void(float, FLinearColor)> DrawCallback);
	
};
