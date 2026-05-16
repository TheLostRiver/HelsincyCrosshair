// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "HelsincyDamageIndicatorSubsystem.generated.h"

class UHelsincyIndicatorRenderer;
class UTexture2D;

/**
 * 负责管理所有伤害指示器渲染器的单例
 * Singleton responsible for managing all damage indicator renderers
 */
UCLASS()
class HELSINCYDAMAGEINDICATOR_API UHelsincyDamageIndicatorSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 获取全局通用的抗锯齿笔刷 | Get global anti-aliased brush
	UTexture2D* GetSmoothLineTexture() const { return SmoothLineTexture; }

	/**
	 * 获取指示器渲染器 | Get indicator renderer
	 * @param StyleTag 渲染器对应的样式tag | Style tag corresponding to the renderer
	 * @return 渲染器 | Renderer instance
	 */
	UHelsincyIndicatorRenderer* GetIndicatorRenderer(FGameplayTag StyleTag);

	// 注册新的指示器渲染器 (支持动态扩展)
	// Register new indicator renderer (supports dynamic extension)
	bool RegisterDamageIndicatorRenderer(FGameplayTag Tag, TSubclassOf<UHelsincyIndicatorRenderer> RendererClass);

	// 获取已注册的 Tag 列表 (供调试面板用) | Get registered Tag list (for debug panel)
	TArray<FGameplayTag> GetRegisteredTags() const;

	// 异步加载是否进行中 | Is async loading in progress
	bool IsAsyncLoading() const { return IndicatorAsyncLoadHandle.IsValid() && IndicatorAsyncLoadHandle->IsLoadingInProgress(); }

private:

	// 缓存异步加载句柄，防止被 GC 或提前释放
	// Cache async load handle to prevent GC or premature release
	TSharedPtr<FStreamableHandle> IndicatorAsyncLoadHandle;

	UPROPERTY()
	TMap<FGameplayTag, UHelsincyIndicatorRenderer*> IndicatorRenderers;

	UPROPERTY()
	UTexture2D* SmoothLineTexture;

	// 内部生成函数 | Internal generation function
	void GenerateSmoothTexture();

	void StartIndicatorRenderAsyncLoadBlueprintClasses();
	void OnIndicatorRenderBlueprintsLoaded();
};
