#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HelsincyCrosshairRenderLibrary.generated.h"

class AHUD;
class UCanvas;
class APlayerController;

UCLASS()
class HELSINCYCROSSHAIR_API UHelsincyCrosshairRenderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 在 HUD 上下文中绘制准星
	 * Draw crosshair within HUD context
	 * @param HUD 当前负责绘制的 HUD | The HUD currently responsible for drawing
	 * @return 若成功获取到 PlayerController、Canvas 并完成桥接绘制则返回 true，否则返回 false
	 *         Returns true if PlayerController and Canvas were obtained and bridge drawing completed, false otherwise
	 * @note 该函数应在 HUD 的 DrawHUD/PostRender 绘制生命周期中调用，否则 Canvas 可能为空
	 *       This function should be called during HUD's DrawHUD/PostRender lifecycle; Canvas may be null otherwise
	 */
	UFUNCTION(BlueprintCallable, Category="HelsincyCrosshair|Integration")
	static bool DrawCrosshairForHUD(AHUD* HUD);

	/**
	 * 在指定控制器和 Canvas 上绘制整套准星
	 * Draw the full crosshair suite on a specific controller and Canvas
	 * @param PlayerController 当前玩家控制器，用于定位 Pawn 与 World | Current player controller for locating Pawn and World
	 * @param Canvas 当前帧绘制所使用的 Canvas | The Canvas used for current frame drawing
	 * @return 若成功完成准星主体、中心点、命中反馈与受伤指示器的整帧绘制则返回 true，否则返回 false
	 *         Returns true if crosshair body, center dot, hit feedback, and damage indicators were all drawn, false otherwise
	 * @note 该函数是 Bridge 层真正的渲染入口，要求外部已经处于有效的 HUD/Canvas 绘制阶段
	 *       This is the actual render entry point for the Bridge layer; requires being in a valid HUD/Canvas draw phase
	 */
	UFUNCTION(BlueprintCallable, Category="HelsincyCrosshair|Integration")
	static bool DrawCrosshairForController(APlayerController* PlayerController, UCanvas* Canvas);

	/**
	 * 查询准星调试是否已启用 (hc.Debug.Enable)
	 * Query whether crosshair debug is enabled (hc.Debug.Enable)
	 * @return 当前是否启用调试输出 | Whether debug output is currently enabled
	 */
	UFUNCTION(BlueprintPure, Category="HelsincyCrosshair|Debug")
	static bool IsCrosshairDebugEnabled();
};
