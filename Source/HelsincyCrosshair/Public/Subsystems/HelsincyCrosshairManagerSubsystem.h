// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "HelsincyCrosshairManagerSubsystem.generated.h"

class UHelsincyShapeRenderer;

/**
 * 负责管理所有准星渲染器的单例
 * Singleton responsible for managing all crosshair renderers
 */
UCLASS()
class HELSINCYCROSSHAIR_API UHelsincyCrosshairManagerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 获取全局通用的抗锯齿笔刷 | Get global anti-aliased brush
	UTexture2D* GetSmoothLineTexture() const { return SmoothLineTexture; }
	
	/**
	 * 获取渲染器 | Get renderer
	 * @param ShapeTag 渲染器对应的图形tag | Shape tag corresponding to the renderer
	 * @return 渲染器 | Renderer instance
	 */
	UHelsincyShapeRenderer* GetRenderer(FGameplayTag ShapeTag);

	// 注册新的渲染器 (支持动态扩展) | Register new renderer (supports dynamic extension)
	bool RegisterRenderer(FGameplayTag Tag, TSubclassOf<UHelsincyShapeRenderer> RendererClass);

	// 获取已注册的 Tag 列表 (供调试面板用) | Get registered Tag list (for debug panel)
	TArray<FGameplayTag> GetRegisteredTags() const;

	// 异步加载是否进行中 | Is async loading in progress
	bool IsAsyncLoading() const { return AsyncLoadHandle.IsValid() && AsyncLoadHandle->IsLoadingInProgress(); }

private:

	// 缓存异步加载句柄，防止被 GC 或提前释放
	// Cache async load handle to prevent GC or premature release
	TSharedPtr<FStreamableHandle> AsyncLoadHandle;
	
	UPROPERTY()
	TMap<FGameplayTag, UHelsincyShapeRenderer*> RendererMap;

	UPROPERTY()
	UTexture2D* SmoothLineTexture;

	// 内部生成函数 | Internal generation function
	void GenerateSmoothTexture();

	void StartCrosshairRenderAsyncLoadBlueprintClasses();
	void OnCrosshairRenderBlueprintsLoaded();
};
