// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "WorldCollision.h"
#include "GenericTeamAgentInterface.h"
#include "Components/ActorComponent.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "HelsincyCrosshairComponent.generated.h"

class ACharacter;
class APlayerCameraManager;
class UHelsincyCrosshairDataAsset;

/**
 * 准星组件
 * Crosshair Component
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HELSINCYCROSSHAIR_API UHelsincyCrosshairComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHelsincyCrosshairComponent();
	
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	
	// 获取当前用于渲染的配置（包含动态扩散影响）
	// Get current rendering profile (includes dynamic spread effects)
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	const FHelsincyCrosshairProfile& GetCurrentProfile() const { return CurrentProfile; }

	// 获取当前准星的主颜色 | Get current crosshair primary color
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FLinearColor GetCurrentVisualPrimaryColor() const { return CurrentVisualPrimaryColorCache; }

	// 获取当前准星的主颜色 CPP版引用传递性能更高
	// Get current crosshair primary color (C++ version, pass by const reference for better performance)
	const FLinearColor& GetCurrentVisualPrimaryColor_CPP() const { return CurrentVisualPrimaryColorCache; }
	/**
	 * 鑾峰彇鍩虹鍑嗘槦鑷€傚簲琛ㄧ幇褰撳墠鐨勮繍琛屾椂鐘舵€?
	 * @return 鍩虹鍑嗘槦鑷€傚簲琛ㄧ幇鐘舵€佺殑鍓湰锛屼緵 Blueprint 鏌ヨ
	 */
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FHelsincy_CrosshairPresentationState GetCrosshairPresentationState() const { return CrosshairPresentationState; }

	/**
	 * 鑾峰彇鍩虹鍑嗘槦鑷€傚簲琛ㄧ幇褰撳墠鐨勮繍琛屾椂鐘舵€?CPP 鐗堝父閲忓紩鐢?
	 * @return 鍩虹鍑嗘槦鑷€傚簲琛ㄧ幇鐘舵€佺殑甯搁噺寮曠敤
	 */
	const FHelsincy_CrosshairPresentationState& GetCrosshairPresentationState_CPP() const { return CrosshairPresentationState; }

	// 是否启用了准星 | Is crosshair enabled
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FORCEINLINE bool IsEnabledCrosshair() const { return bEnabledCrosshair; }

	bool RefreshLocalPlayerGuardForRendering();

#if WITH_DEV_AUTOMATION_TESTS
	bool IsWaitingForLocalPlayerController() const { return bPendingOwnerCheck; }
	int32 GetPendingLocalPlayerGuardFrameCount() const { return PendingCheckFrameCount; }
	void Debug_SetPendingLocalPlayerGuardForAutomation(int32 InitialFrameCount);
	void Debug_TickPendingLocalPlayerGuardForAutomation();
#endif

	// 关闭准星 | Disable crosshair
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void DisableCrosshair();

	// 启用准星 | Enable crosshair
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void EnableCrosshair();

	// 关闭中心点 | Disable center dot
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void DisableCenterDot();

	// 启用中心点 | Enable center dot
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void EnableCenterDot();

	// 关闭目标识别 | Disable target detection
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void DisableTargetDetection();

	// 启用目标识别 | Enable target detection
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void EnableTargetDetection();

	// 修改 TriggerHitMarker，不再只是记录时间，而是添加实例
	// TriggerHitMarker now adds an instance instead of just recording time
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void TriggerHitMarker();

	// 触发自定义颜色的命中 | Trigger hit marker with custom color
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void TriggerHitMarkerColor(FLinearColor CustomColor);

	// 触发爆头命中 (自动读取 Profile.HeadshotColor)
	// Trigger headshot marker (auto-reads Profile.HeadshotColor)
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void TriggerHeadshotMarker();

	// 触发击杀命中 (自动读取 Profile.KillColor)
	// Trigger kill marker (auto-reads Profile.KillColor)
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void TriggerKillMarker();

	// 触发带伤害强度的命中反馈 | Trigger hit marker with damage-scaled impact feedback
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void TriggerHitMarkerAdvanced(EHitMarkerPriority Priority, FLinearColor CustomColor, float DamageNormalized = 1.0f);

	// 获取列表供 HUD 绘制 | Get list for HUD rendering
	const TArray<FHelsincy_ActiveHitMarker>& GetActiveHitMarkers() const { return ActiveHitMarkers; }

	// 获取单实例 HitMarker 状态 (COD 风格) | Get single-instance HitMarker state (COD-style)
	const FHelsincy_SingleHitMarkerState& GetSingleHitMarkerState() const { return SingleHitMarkerState; }

	/**
	 * 判断单实例命中准星当前是否可见
	 * @return 是否处于可见显示状态
	 */
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	bool IsSingleHitMarkerVisible() const { return SingleHitMarkerState.bVisible; }

	/**
	 * 获取单实例命中准星当前相位
	 * @return 当前单实例命中准星运行相位
	 */
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	EHelsincySingleHitMarkerPhase GetSingleHitMarkerPhase() const { return SingleHitMarkerState.Phase; }

	/**
	 * 获取单实例命中准星当前透明度缓存
	 * @return 当前单实例命中准星透明度
	 */
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	float GetSingleHitMarkerOpacity() const { return SingleHitMarkerState.Opacity; }

	/**
	 * 获取单实例命中准星当前命中能量
	 * @return 当前单实例命中准星命中能量缓存
	 */
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	float GetSingleHitMarkerImpactEnergy() const { return SingleHitMarkerState.ImpactEnergy; }

	// 获取总扩散 (State + Recoil) | Get final spread (State + Recoil)
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FVector2D GetFinalSpread() const;

	// 获取状态扩散 (不含后坐力) | Get state spread (without recoil)
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FVector2D GetStateSpread() const { return StateSpread; }

	// 获取后坐力扩散 | Get recoil spread
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FVector2D GetRecoilSpread() const { return RecoilSpread; }
	
	// --- Actions ---

	// 增加后坐力扩散 (开火时调用)
	// Add recoil spread (call when firing)
	// VerticalKick: 垂直上抬扩散 | Vertical kick spread
	// HorizontalKick: 水平随机扩散 | Horizontal random spread
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void AddRecoil(float HorizontalKick, float VerticalKick);

	// --- 预设系统 | Preset System ---

	// 保存当前配置为预设 | Save current config as preset
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair|Presets")
	void SavePreset(FName PresetName);

	// 加载预设 | Load preset
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair|Presets")
	bool LoadPreset(FName PresetName);

	// 修改当前配置 (UI绑定用) | Update current profile (for UI binding)
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair|Edit")
	void UpdateProfile(const FHelsincyCrosshairProfile& NewProfile);
	
	// 从资产加载配置 | Load config from data asset
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void LoadFromDataAsset(UHelsincyCrosshairDataAsset* DataAsset);

	// 获取当前瞄准的目标态度 | Get current aimed target attitude
	// Hostile, Friendly, Neutral
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FORCEINLINE TEnumAsByte<ETeamAttitude::Type> GetCurrentTargetAttitude() const { return CurrentTargetAttitude; }

	// 获取当前目标的 Tag | Get current target's Tag
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair")
	FORCEINLINE FGameplayTag GetCurrentTargetTag() const { return CurrentTargetTag; }

	// 清空准星预设库 | Clear crosshair preset library
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void ClearPresetLibrary() { PresetLibrary.Empty(); }

	// ============ 持久化 API | Persistence API ============

	// 将当前内存中的预设库 + 当前活跃预设名 保存到磁盘
	// Save the in-memory preset library + active preset name to disk
	// @param SlotName  存档槽位名 (默认 "CrosshairPresets") | Save slot name (default "CrosshairPresets")
	// @param UserIndex 本地玩家索引 (默认 0, 分屏/多账户时使用) | Local player index (default 0, for split-screen/multi-account)
	// @return 是否保存成功 | Whether save succeeded
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair|Presets")
	bool SavePresetsToDisk(const FString& SlotName = TEXT("CrosshairPresets"), int32 UserIndex = 0);

	// 从磁盘加载预设库 | Load preset library from disk
	// 成功后 PresetLibrary 被替换, 并自动应用 LastActivePreset (如果存在)
	// On success, PresetLibrary is replaced and LastActivePreset is auto-applied (if exists)
	// @return 是否加载成功 | Whether load succeeded
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair|Presets")
	bool LoadPresetsFromDisk(const FString& SlotName = TEXT("CrosshairPresets"), int32 UserIndex = 0);

	// 检查磁盘上是否存在存档 | Check if save exists on disk
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair|Presets")
	bool DoesSaveExist(const FString& SlotName = TEXT("CrosshairPresets"), int32 UserIndex = 0) const;

	// 删除磁盘上的存档 | Delete save from disk
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair|Presets")
	bool DeleteSaveFromDisk(const FString& SlotName = TEXT("CrosshairPresets"), int32 UserIndex = 0);

	// 获取当前预设库中所有预设名称 (方便 UI 构建列表)
	// Get all preset names in current library (for UI list building)
	UFUNCTION(BlueprintPure, Category = "HelsincyCrosshair|Presets")
	TArray<FName> GetAllPresetNames() const;

private:
	FLinearColor CurrentVisualPrimaryColorCache {FLinearColor::White};
	bool bCurrentVisualColorOverrideCache {false};
	FName CurrentVisualColorReasonCache {NAME_None};
	void RefreshCurrentPrimaryColor();
	void UpdateCrosshairColorByInteractionTag(const FHelsincy_VisualSettings& Vis, const FGameplayTag& Tag, FLinearColor& Color, bool& bOverride, FName* OutReason = nullptr);
	void UpdateCrosshairColorByTeam(const FHelsincy_VisualSettings& Vis, const ETeamAttitude::Type TeamAttitude, FLinearColor& Color, bool& bOverride, FName* OutReason = nullptr);
	void UpdateCrosshairColorByDefaultInteractionColor(const FHelsincy_VisualSettings& Vis, const FGameplayTag& Tag, FLinearColor& Color, bool& bOverride, FName* OutReason = nullptr);
	void UpdateCrosshairColorByNeutral(const FHelsincy_VisualSettings& Vis, const ETeamAttitude::Type TeamAttitude, FLinearColor& Color, bool& bOverride, FName* OutReason = nullptr);
	
private:
	
	// 通用处理函数 | Common processing function
	void Internal_AddOrUpdateHitMarker(FLinearColor Color, EHitMarkerPriority Priority, float ScaleSize = 1.f, float DamageNormalized = 0.5f);

	/**
	 * 清空单实例命中准星的待降级视觉请求
	 */
	void ClearPendingSingleHitMarkerDowngrade();

	/**
	 * 记录单实例命中准星的延后降级请求
	 * @param Config 命中准星配置
	 * @param Color 待降级阶段使用的颜色
	 * @param Priority 待降级阶段使用的优先级
	 * @param ScaleSize 待降级阶段使用的缩放
	 */
	void QueuePendingSingleHitMarkerDowngrade(const FHelsincy_HitMarkerProfile& Config, const FLinearColor& Color, EHitMarkerPriority Priority, float ScaleSize);

	/**
	 * 应用单实例命中准星的待降级视觉请求
	 */
	void ApplyPendingSingleHitMarkerDowngrade();

	/**
	 * 获取单实例命中准星的延后降级延迟
	 * @param Config 命中准星配置
	 * @return 延后降级的等待时间
	 */
	float GetSingleHitMarkerDowngradeDelay(const FHelsincy_HitMarkerProfile& Config) const;

	/**
	 * 更新单实例命中准星状态机
	 * @param DeltaTime 帧间隔
	 */
	void UpdateSingleHitMarkerStateMachine(float DeltaTime);

	/**
	 * 灏嗗熀纭€鍑嗘槦鑷€傚簲琛ㄧ幇鐘舵€佸綊浣嶅埌涓€т綅
	 */
	void ResetCrosshairPresentationState();

	/**
	 * 鏇存柊鍩虹鍑嗘槦鑷€傚簲琛ㄧ幇鐘舵€?
	 * @param DeltaTime 甯ч棿闅?
	 */
	void UpdateCrosshairPresentationState(float DeltaTime);

protected:
	
	/** 刷新 Tick 和缓存状态。Possession 变更后应调用此函数。 */
	/** Refresh tick and cached state. Should be called after possession changes. */
	UFUNCTION(BlueprintCallable, Category = "HelsincyCrosshair")
	void RefreshTickState();

	/**
	 * 判断组件是否应该为当前 Owner 激活
	 * 仅当 Owner 是本地真人玩家控制的 Pawn 时返回 true
	 * Check whether this component should activate for its owner.
	 * Returns true only when the owning Pawn is controlled by a local human player.
	 */
	bool ShouldActivateForOwner() const;

	/** 执行完整初始化 (DataAsset + Camera + TraceDelegate + 持久化) | Perform full initialization */
	void PerformFullInitialization();

	TWeakObjectPtr<APlayerCameraManager> PlayerCameraManagerWeak;
	void SetupPlayerCameraManager();
	
	// 当前激活的配置 | Currently active profile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyCrosshair")
	FHelsincyCrosshairProfile CurrentProfile;

	// 内存中的预设库 (如果要持久化，可将此 Map 存入 SaveGame)
	// In-memory preset library (can be persisted by saving this Map to SaveGame)
	UPROPERTY()
	TMap<FName, FHelsincyCrosshairProfile> PresetLibrary;

	// 当前活跃预设名 (用于持久化恢复) | Current active preset name (for persistence restore)
	UPROPERTY()
	FName ActivePresetName;

	// ============ 自动持久化设置 | Auto-Persistence Settings ============

	// 是否在 BeginPlay 时自动从磁盘加载预设
	// Auto-load presets from disk on BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyCrosshair|Persistence")
	bool bAutoLoadOnBeginPlay = false;

	// 是否在 EndPlay 时自动保存到磁盘
	// Auto-save to disk on EndPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyCrosshair|Persistence")
	bool bAutoSaveOnEndPlay = false;

	// 自动保存/加载的槽位名 | Auto save/load slot name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyCrosshair|Persistence",
		meta = (EditCondition = "bAutoLoadOnBeginPlay || bAutoSaveOnEndPlay"))
	FString AutoSaveSlotName = TEXT("CrosshairPresets");

private:

	// 是否启用准星 | Whether crosshair is enabled
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HelsincyCrosshair|Defaults", meta = (AllowPrivateAccess))
	bool bEnabledCrosshair {true};

	// 默认的准星数据资产 (在编辑器里配置默认值)
	// Default crosshair data asset (configure defaults in editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyCrosshair|Defaults", meta = (AllowPrivateAccess))
	UHelsincyCrosshairDataAsset* DefaultCrosshairAsset {nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelsincyCrosshair|Defaults", meta = (AllowPrivateAccess))
	bool bUseDefaultCrosshairAssetInit {true};

	FGameplayTag CurrentTargetTag;

	// 缓存当前态度，供 HUD 读取 | Cache current attitude for HUD access
	ETeamAttitude::Type CurrentTargetAttitude = ETeamAttitude::Neutral;

	// --- 异步检测相关 | Async Detection ---

	// 目标检测轮询间隔 (秒)，避免每帧发起 Trace
	// Target detection polling interval (seconds), avoids per-frame trace
	float TargetDetectionTimer = 0.0f;
	static constexpr float TargetDetectionInterval = 0.05f; // 20Hz

	// 追踪当前的请求句柄 | Track current request handle
	FTraceHandle AsyncTraceHandle;

	// 异步 Trace 超时保护: 超过此帧数后强制重置句柄
	// Async trace timeout protection: force reset handle after this many frames
	int32 AsyncTraceWaitFrames = 0;
	static constexpr int32 AsyncTraceMaxWaitFrames = 10;

	// 回调委托 (当物理线程完成检测时调用)
	// Callback delegate (called when physics thread finishes detection)
	FTraceDelegate TraceDelegate;

	// 内部函数：发起请求 | Internal: initiate async trace request
	void RequestAsyncTargetDetection();

	// 内部函数：回调处理 | Internal: callback handler
	void OnTraceCompleted(const FTraceHandle& Handle, FTraceDatum& Datum);

	// 内部函数：处理命中结果 (核心逻辑复用)
	// Internal: process hit result (core logic reuse)
	void ProcessHitResult(const FHitResult& Hit);
	void ClearTarget();
	
	// 内部检测函数 | Internal detection function
	void UpdateCurrentTargetTag(AActor* Actor);

	// 使用数组代替单变量 LastHitTime
	// Use array instead of single LastHitTime variable
	TArray<FHelsincy_ActiveHitMarker> ActiveHitMarkers;

	// 单实例模式运行时状态 (COD 风格) | Single-instance mode runtime state (COD-style)
	FHelsincy_SingleHitMarkerState SingleHitMarkerState;

	// HitMarker 震动种子计数器 | HitMarker shake seed counter
	int32 HitMarkerShakeSeedCounter = 0;

	// 鍩虹鍑嗘槦鑷€傚簲琛ㄧ幇杩愯鏃剁姸鎬?| Runtime state for adaptive base-crosshair presentation
	FHelsincy_CrosshairPresentationState CrosshairPresentationState;

	// 单实例模式待降级视觉请求是否存在 | Whether a delayed downgrade request is pending
	bool bHasPendingSingleHitMarkerDowngrade = false;

	// 待降级优先级 | Pending downgrade priority
	EHitMarkerPriority PendingSingleHitMarkerPriority = EHitMarkerPriority::Low_Priority_Body;

	// 待降级颜色 | Pending downgrade color
	FLinearColor PendingSingleHitMarkerColor = FLinearColor::White;

	// 待降级缩放 | Pending downgrade size scale
	float PendingSingleHitMarkerScale = 1.0f;

	// 待降级剩余延迟 | Remaining delay before applying the pending downgrade
	float PendingSingleHitMarkerDelayRemaining = 0.0f;

	// 1. 后坐力扩散 (随时间衰减) | 1. Recoil spread (decays over time)
	FVector2D RecoilSpread;

	// 2. 状态扩散 (每帧计算) | 2. State spread (calculated each frame)
	FVector2D StateSpread;

	// 缓存 Character 指针以优化 Tick 中的性能
	// Cache Character pointer for Tick performance optimization
	UPROPERTY()
	ACharacter* OwnerCharacter;

	// 缓存owner指针 | Cache owner pawn pointer
	UPROPERTY()
	APawn* OwningPawn;

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

	void ClearRecoilAndSpread();

	// 内部函数：计算状态扩散 | Internal: calculate state spread
	void CalculateStateSpread(FVector2D& TargetStateSpread);
	// 内部函数：处理后坐力恢复 | Internal: handle recoil recovery
	void UpdateRecoilRecovery(float DeltaTime);

	void UpdateHitMarker(const float DeltaTime);

	/** 同步单实例命中准星的派生显示状态缓存 | Synchronize derived single-instance state cache */
	void RefreshSingleHitMarkerStateView();
};
