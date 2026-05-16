// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HelsincyDamageIndicatorRenderLibrary.generated.h"

class AHUD;
class UCanvas;
class APlayerController;

UCLASS()
class HELSINCYDAMAGEINDICATOR_API UHelsincyDamageIndicatorRenderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 在 HUD 上下文中绘制伤害指示器
	 * Draw damage indicators within HUD context
	 * @param HUD 当前负责绘制的 HUD | The HUD currently responsible for drawing
	 * @return 若成功获取到 PlayerController、Canvas 并完成绘制则返回 true，否则返回 false
	 *         Returns true if PlayerController and Canvas were obtained and drawing completed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category="HelsincyDamageIndicator|Integration")
	static bool DrawDamageIndicatorsForHUD(AHUD* HUD);

	/**
	 * 在指定控制器和 Canvas 上绘制伤害指示器
	 * Draw damage indicators on specified controller and Canvas
	 * @param PlayerController 当前玩家控制器，用于定位 Pawn | Current player controller for locating Pawn
	 * @param Canvas 当前帧绘制所使用的 Canvas | The Canvas used for current frame drawing
	 * @return 若成功完成伤害指示器整帧绘制则返回 true，否则返回 false
	 *         Returns true if damage indicator frame drawing completed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category="HelsincyDamageIndicator|Integration")
	static bool DrawDamageIndicatorsForController(APlayerController* PlayerController, UCanvas* Canvas);

	/**
	 * 查询伤害指示器调试是否已启用 (di.Debug.Enable)
	 * Query whether damage indicator debug is enabled (di.Debug.Enable)
	 * @return 当前是否启用调试输出 | Whether debug output is currently enabled
	 */
	UFUNCTION(BlueprintPure, Category="HelsincyDamageIndicator|Debug")
	static bool IsDamageIndicatorDebugEnabled();
};
