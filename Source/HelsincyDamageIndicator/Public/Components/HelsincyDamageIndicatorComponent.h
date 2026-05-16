// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "HelsincyDamageIndicatorComponent.generated.h"

class APlayerController;
class UHelsincyDamageIndicatorDataAsset;

/**
 * 伤害指示器组件
 * Damage Indicator Component
 * 独立于准星组件工作，负责管理伤害方向指示器的生命周期和状态更新
 * Works independently from crosshair component; manages damage direction indicator lifecycle and state updates
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HELSINCYDAMAGEINDICATOR_API UHelsincyDamageIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHelsincyDamageIndicatorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	// 接收伤害通知 | Receive damage notification
	// 在 Character 的 TakeDamage 或 AnyDamage 事件中调用
	// Call this in Character's TakeDamage or AnyDamage events
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator")
	void RegisterDamageEvent(AActor* DamageCauser, FVector LocationIfNoActor);

	// 获取当前的活跃指示器列表 (供渲染) | Get active indicator list (for rendering)
	const TArray<FHelsincy_ActiveDamageIndicator>& GetActiveIndicators() const { return ActiveIndicators; }

	// 获取当前用于渲染的配置 | Get current rendering configuration
	UFUNCTION(BlueprintPure, Category = "HelsincyDamageIndicator")
	const FHelsincy_DamageIndicatorProfile& GetIndicatorProfile() const { return IndicatorProfile; }

	// 修改配置 | Modify configuration
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator")
	void SetIndicatorProfile(const FHelsincy_DamageIndicatorProfile& NewProfile) { IndicatorProfile = NewProfile; }

	// 是否启用 | Is enabled
	UFUNCTION(BlueprintPure, Category = "HelsincyDamageIndicator")
	bool IsEnabled() const { return IndicatorProfile.bEnabled; }

	bool RefreshLocalPlayerGuardForRendering();

#if WITH_DEV_AUTOMATION_TESTS
	bool IsWaitingForLocalPlayerController() const { return bPendingOwnerCheck; }
	int32 GetPendingLocalPlayerGuardFrameCount() const { return PendingCheckFrameCount; }
	void Debug_SetPendingLocalPlayerGuardForAutomation(int32 InitialFrameCount);
	void Debug_TickPendingLocalPlayerGuardForAutomation();
#endif

#if !UE_BUILD_SHIPPING
	// 调试/验收专用：运行时创建组件后强制完成本地初始化 | Debug/validation only: force local initialization after runtime component creation
	void Debug_ForceEnableForValidation();

	// 调试/验收专用：清空当前活动指示器 | Debug/validation only: clear active indicators
	void Debug_ClearActiveIndicators();
#endif

	// ============ DataAsset API ============

	// 从数据资产加载配置 | Load config from data asset
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator")
	void LoadFromDataAsset(UHelsincyDamageIndicatorDataAsset* DataAsset);

	// ============ 预设系统 | Preset System ============

	// 保存当前配置为预设 | Save current config as preset
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator|Presets")
	void SavePreset(FName PresetName);

	// 加载预设 | Load preset
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator|Presets")
	bool LoadPreset(FName PresetName);

	// 获取所有预设名称 | Get all preset names
	UFUNCTION(BlueprintPure, Category = "HelsincyDamageIndicator|Presets")
	TArray<FName> GetAllPresetNames() const;

	// 清空预设库 | Clear preset library
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator|Presets")
	void ClearPresetLibrary() { PresetLibrary.Empty(); }

	// ============ 持久化 API | Persistence API ============

	// 保存预设到磁盘 | Save presets to disk
	// @param SlotName  存档槽位名 (默认 "DamageIndicatorPresets") | Save slot name
	// @param UserIndex 本地玩家索引 | Local player index
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator|Presets")
	bool SavePresetsToDisk(const FString& SlotName = TEXT("DamageIndicatorPresets"), int32 UserIndex = 0);

	// 从磁盘加载预设 | Load presets from disk
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator|Presets")
	bool LoadPresetsFromDisk(const FString& SlotName = TEXT("DamageIndicatorPresets"), int32 UserIndex = 0);

	// 检查存档是否存在 | Check if save exists
	UFUNCTION(BlueprintPure, Category = "HelsincyDamageIndicator|Presets")
	bool DoesSaveExist(const FString& SlotName = TEXT("DamageIndicatorPresets"), int32 UserIndex = 0) const;

	// 删除存档 | Delete save
	UFUNCTION(BlueprintCallable, Category = "HelsincyDamageIndicator|Presets")
	bool DeleteSaveFromDisk(const FString& SlotName = TEXT("DamageIndicatorPresets"), int32 UserIndex = 0);

private:

	/**
	 * 判断组件是否应该为当前 Owner 激活
	 * 仅当 Owner 是本地真人玩家控制的 Pawn 时返回 true
	 * Check whether this component should activate for its owner.
	 * Returns true only when the owning Pawn is controlled by a local human player.
	 */
	bool ShouldActivateForOwner() const;

	/** 执行完整初始化 (DataAsset + 持久化 + StyleTag fallback) | Perform full initialization */
	void PerformFullInitialization();

	// 是否等待 Controller 就位后再做本地判定 | Waiting for Controller before activation check
	bool bPendingOwnerCheck = false;

	// 等待帧计数器 | Frame counter while waiting for Controller
	int32 PendingCheckFrameCount = 0;

	// 最大等待帧数, 超过则默认禁用 | Max frames to wait before defaulting to disabled
	static constexpr int32 MaxPendingCheckFrames = 60;

	// 组件是否已通过本地判定并确认激活 | Whether the component passed the local-player check
	bool bOwnerCheckPassed = false;

	// 本地真人玩家完整初始化是否已经执行 | Whether local-human initialization has already run
	bool bLocalPlayerInitializationComplete = false;

	FTimerHandle LocalPlayerGuardRetryTimerHandle;
	bool bLocalPlayerGuardTimeoutLogged = false;
	static constexpr float LocalPlayerGuardRetryIntervalSeconds = 0.10f;

	void ScheduleLocalPlayerGuardRetry();
	void ClearLocalPlayerGuardRetry();
	void RetryLocalPlayerGuard();
	bool TickPendingLocalPlayerGuard();

	// 伤害指示器配置 | Damage indicator configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyDamageIndicator", meta = (AllowPrivateAccess))
	FHelsincy_DamageIndicatorProfile IndicatorProfile;

	// 存储当前所有活跃的伤害指示器 | Store all currently active damage indicators
	TArray<FHelsincy_ActiveDamageIndicator> ActiveIndicators;

	void UpdateIndicators(const float DeltaTime);

	// 内部函数：计算目标相对于玩家的角度
	// Internal: calculate target angle relative to player
	float CalculateTargetAngle(FVector TargetLocation);

	// 缓存owner指针 | Cache owner pointer
	UPROPERTY()
	APawn* OwningPawn;

	// ============ DataAsset 默认配置 | DataAsset Defaults ============

	// 默认数据资产 (编辑器配置) | Default data asset (editor config)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyDamageIndicator|Defaults", meta = (AllowPrivateAccess))
	UHelsincyDamageIndicatorDataAsset* DefaultDataAsset = nullptr;

	// 是否使用默认数据资产初始化 | Use default data asset for initialization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyDamageIndicator|Defaults", meta = (AllowPrivateAccess))
	bool bUseDefaultDataAssetInit = true;

	// ============ 预设系统 | Preset System ============

	// 内存预设库 | In-memory preset library
	UPROPERTY()
	TMap<FName, FHelsincy_DamageIndicatorProfile> PresetLibrary;

	// 当前活跃预设名 | Current active preset name
	UPROPERTY()
	FName ActivePresetName;

	// ============ 自动持久化 | Auto-Persistence ============

	// BeginPlay 时自动加载 | Auto-load on BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyDamageIndicator|Persistence", meta = (AllowPrivateAccess))
	bool bAutoLoadOnBeginPlay = false;

	// EndPlay 时自动保存 | Auto-save on EndPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyDamageIndicator|Persistence", meta = (AllowPrivateAccess))
	bool bAutoSaveOnEndPlay = false;

	// 自动保存/加载槽位名 | Auto save/load slot name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyDamageIndicator|Persistence",
		meta = (AllowPrivateAccess, EditCondition = "bAutoLoadOnBeginPlay || bAutoSaveOnEndPlay"))
	FString AutoSaveSlotName = TEXT("DamageIndicatorPresets");
};
