// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Components/HelsincyCrosshairComponent.h"

#include "HelsincyCrosshair.h"
#include "Debug/HelsincyCrosshairDebug.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "DataAssets/HelsincyCrosshairDataAsset.h"
#include "GenericTeamAgentInterface.h"
#include "Interfaces/HelsincyTargetInterface.h"
#include "Library/HelsincyHitMarkerShakeMath.h"
#include "Library/HelsincySingleHitMarkerSpriteSupport.h"
#include "SaveGame/HelsincyCrosshairSaveGame.h"
#include "Kismet/GameplayStatics.h"


namespace
{
	int32 AllocateHitMarkerShakeSeed(int32& SeedCounter)
	{
		++SeedCounter;
		if (SeedCounter <= 0)
		{
			SeedCounter = 1;
		}
		return SeedCounter;
	}

	float ResolveImpactMotionEnergy(const FHelsincy_HitMarkerProfile& Config, EHitMarkerPriority Priority, float DamageNormalized)
	{
		const float PriorityMultiplier = HelsincyHitMarkerShakeMath::GetPriorityShakeMultiplier(Config, Priority);
		const float Damage = FMath::Clamp(DamageNormalized, 0.0f, 1.0f);
		const float DamageEnergy = 0.65f + (Damage * Config.DamageToImpactScale);
		return FMath::Clamp(PriorityMultiplier * DamageEnergy, 0.0f, FMath::Max(Config.MaxImpactMotionEnergy, 0.25f));
	}

	void ConfigureActiveHitMarkerShake(FHelsincy_ActiveHitMarker& Marker, const FHelsincy_HitMarkerProfile& Config, EHitMarkerPriority Priority, int32 Seed, float DamageNormalized)
	{
		Marker.ShakeSeed = Seed;
		Marker.ShakePhase = HelsincyHitMarkerShakeMath::PhaseFromSeed(Seed);
		Marker.ShakeDirection = HelsincyHitMarkerShakeMath::DirectionFromSeed(Seed);
		Marker.ShakeEnergy = FMath::Clamp(
			HelsincyHitMarkerShakeMath::GetPriorityShakeMultiplier(Config, Priority),
			0.0f,
			2.5f
		);
		Marker.ImpactDamageScale = FMath::Clamp(DamageNormalized, 0.0f, 1.0f);
		Marker.ImpactMotionEnergy = ResolveImpactMotionEnergy(Config, Priority, DamageNormalized);
		Marker.ImpactMotionSign = HelsincyHitMarkerShakeMath::SignFromSeed(Seed, 42);
	}

	void RefreshActiveHitMarkerShakeEnergy(FHelsincy_ActiveHitMarker& Marker, const FHelsincy_HitMarkerProfile& Config, EHitMarkerPriority Priority, float DamageNormalized)
	{
		Marker.ShakeEnergy = FMath::Clamp(
			FMath::Max(Marker.ShakeEnergy, HelsincyHitMarkerShakeMath::GetPriorityShakeMultiplier(Config, Priority)),
			0.0f,
			2.5f
		);
		Marker.ImpactDamageScale = FMath::Max(Marker.ImpactDamageScale, FMath::Clamp(DamageNormalized, 0.0f, 1.0f));
		Marker.ImpactMotionEnergy = FMath::Clamp(
			FMath::Max(Marker.ImpactMotionEnergy, ResolveImpactMotionEnergy(Config, Priority, DamageNormalized)),
			0.0f,
			FMath::Max(Config.MaxImpactMotionEnergy, 0.25f)
		);
	}

	void InjectSingleHitMarkerShake(FHelsincy_SingleHitMarkerState& State, const FHelsincy_HitMarkerProfile& Config, EHitMarkerPriority Priority, float PulseScale, int32 Seed, float DamageNormalized)
	{
		const float PriorityMultiplier = HelsincyHitMarkerShakeMath::GetPriorityShakeMultiplier(Config, Priority);
		const float PulseEnergy = FMath::Max(0.35f, FMath::Max(0.0f, PulseScale - 1.0f) * 2.0f);
		const float MaxShakeEnergy = FMath::Max(Config.SingleInstanceMaxImpactEnergy, 1.0f) * 1.5f;

		State.ShakeSeed = Seed;
		State.ShakeAge = 0.0f;
		State.ShakePhase = HelsincyHitMarkerShakeMath::PhaseFromSeed(Seed);
		State.ShakeDirection = HelsincyHitMarkerShakeMath::DirectionFromSeed(Seed);
		State.ShakeEnergy = FMath::Clamp(
			FMath::Max(State.ShakeEnergy, PulseEnergy * PriorityMultiplier),
			0.0f,
			MaxShakeEnergy
		);
		State.ImpactDamageScale = FMath::Clamp(DamageNormalized, 0.0f, 1.0f);
		State.ImpactMotionEnergy = FMath::Clamp(
			FMath::Max(State.ImpactMotionEnergy, ResolveImpactMotionEnergy(Config, Priority, DamageNormalized)),
			0.0f,
			FMath::Max(Config.MaxImpactMotionEnergy, 0.25f)
		);
		State.ImpactMotionSign = HelsincyHitMarkerShakeMath::SignFromSeed(Seed, 42);
	}
}


UHelsincyCrosshairComponent::UHelsincyCrosshairComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	
	// 默认配置 | Default configuration
	CurrentProfile.ShapeTag = FHelsincyCrosshair_Tags::Shape_Cross;

	RecoilSpread = FVector2D::ZeroVector;
	StateSpread = FVector2D::ZeroVector;
}

void UHelsincyCrosshairComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (!HasAnyFlags(RF_ClassDefaultObject) && GetOwner() && !GetOwner()->IsTemplate())
	{
		CurrentVisualPrimaryColorCache = CurrentProfile.VisualsConfig.PrimaryColor;
	}

	ResetCrosshairPresentationState();
}

void UHelsincyCrosshairComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		ResetCrosshairPresentationState();
		SetComponentTickEnabled(false);
		Deactivate();
		return;
	}

	// 临时缓存 Pawn 供 ShouldActivateForOwner 使用 | Temp cache Pawn for ShouldActivateForOwner
	OwningPawn = Pawn;
	OwnerCharacter = Cast<ACharacter>(Pawn);

	// 快速路径: Controller 已就位 | Fast path: Controller already assigned
	if (Pawn->GetController())
	{
		if (ShouldActivateForOwner())
		{
			bOwnerCheckPassed = true;
			SetComponentTickEnabled(true);
			if (!IsActive()) Activate();
			PerformFullInitialization();
			bLocalPlayerInitializationComplete = true;
		}
		else
		{
			// 非本地真人玩家, 禁用正常 HUD 逻辑 | Not a local human player, disable normal HUD logic
			OwningPawn = nullptr;
			OwnerCharacter = nullptr;
			ResetCrosshairPresentationState();
			SetComponentTickEnabled(false);
			Deactivate();
		}
	}
	else
	{
		// Controller 未就位, 延迟到 Tick 判定 | Controller not assigned, defer to Tick
		bPendingOwnerCheck = true;
		PendingCheckFrameCount = 0;
		ResetCrosshairPresentationState();
		ScheduleLocalPlayerGuardRetry();
	}
}

void UHelsincyCrosshairComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SCOPE_CYCLE_COUNTER(STAT_HC_CrosshairComponentTick);

	if (TickPendingLocalPlayerGuard())
	{
		return;
	}

	if (!OwningPawn || !bEnabledCrosshair)
	{
		ResetCrosshairPresentationState();

		// 禁用时仍需消耗已存在的 HitMarker 计时器，避免冻结在屏幕上
		// Still tick existing HitMarker timers when disabled, to prevent them freezing on screen
		UpdateHitMarker(DeltaTime);
		return;
	}

	// 检查开关：如果禁用了动态扩散
	// Check toggle: if dynamic spread is disabled
	if (!CurrentProfile.DynamicConfig.bEnableDynamicSpread)
	{
		// 强制将扩散归零，不再进行后续扩散计算
		// Force spread to zero, skip further spread calculations
		StateSpread = FVector2D::ZeroVector;
		RecoilSpread = FVector2D::ZeroVector;
		// 仍然需要更新命中反馈和目标检测
		// Still need to update hit markers and target detection
		UpdateHitMarker(DeltaTime);
		TargetDetectionTimer += DeltaTime;
		if (TargetDetectionTimer >= TargetDetectionInterval)
		{
			TargetDetectionTimer = 0.0f;
			RequestAsyncTargetDetection();
		}
		RefreshCurrentPrimaryColor();
		UpdateCrosshairPresentationState(DeltaTime);
		return;
	}

	FVector2D TargetStateSpread = FVector2D::ZeroVector;
	
	// 1. 计算状态扩散 (每帧直接覆盖，不需要 Interp，因为速度本身是平滑的)
	// 1. Calculate state spread (overwrite each frame; no Interp needed since velocity is already smooth)
	CalculateStateSpread(TargetStateSpread);
	
	// 执行平滑插值
	// Perform smooth interpolation
	// 核心逻辑：Current 追赶 Target
	// Core logic: Current chases Target
	
	const auto& Config = CurrentProfile.DynamicConfig;
    
	// [高级技巧]：根据是"变大"还是"变小"选择不同的速度
	// [Advanced]: choose different interp speeds for expanding vs. contracting
	// 如果 当前值 < 目标值，说明正在扩散 (起跳/加速)，用较快的 JumpExpandSpeed
	// If current < target, spread is expanding (jump/accelerate) -> use faster JumpExpandSpeed
	// 如果 当前值 > 目标值，说明正在恢复 (落地/急停)，用较慢的 LandRecoverySpeed
	// If current > target, spread is recovering (landing/stopping) -> use slower LandRecoverySpeed
    
	// 对 X 轴插值 | Interpolate X axis
	float InterpSpeedX = (StateSpread.X < TargetStateSpread.X) ? Config.JumpExpandSpeed : Config.LandRecoverySpeed;
	StateSpread.X = FMath::FInterpTo(StateSpread.X, TargetStateSpread.X, DeltaTime, InterpSpeedX);

	// 对 Y 轴插值 | Interpolate Y axis
	float InterpSpeedY = (StateSpread.Y < TargetStateSpread.Y) ? Config.JumpExpandSpeed : Config.LandRecoverySpeed;
	StateSpread.Y = FMath::FInterpTo(StateSpread.Y, TargetStateSpread.Y, DeltaTime, InterpSpeedY);


	// 2. 处理后坐力恢复 (Interp Recoil -> 0)
	// 2. Handle recoil recovery (Interp Recoil -> 0)
	UpdateRecoilRecovery(DeltaTime);

	UpdateHitMarker(DeltaTime);
	
	// 目标检测轮询：降低频率，减少不必要的每帧 Trace
	// Target detection polling: reduce frequency to avoid per-frame traces
	TargetDetectionTimer += DeltaTime;
	if (TargetDetectionTimer >= TargetDetectionInterval)
	{
		TargetDetectionTimer = 0.0f;
		RequestAsyncTargetDetection();
	}

	RefreshCurrentPrimaryColor();
	UpdateCrosshairPresentationState(DeltaTime);
}

FVector2D UHelsincyCrosshairComponent::GetFinalSpread() const
{
	// 最终输出 = 状态扩散 + 后坐力扩散
	// Final output = State spread + Recoil spread
	return StateSpread + RecoilSpread;
}

void UHelsincyCrosshairComponent::AddRecoil(float HorizontalKick, float VerticalKick)
{
	if (!bEnabledCrosshair) return;

	if (CurrentProfile.PresentationConfig.bEnableAdaptivePresentation
		&& CurrentProfile.PresentationConfig.bEnableAutoScale
		&& (!FMath::IsNearlyZero(HorizontalKick) || !FMath::IsNearlyZero(VerticalKick)))
	{
		CrosshairPresentationState.FirePulseStrength = 1.0f;
	}

	if (CurrentProfile.PresentationConfig.bEnableAdaptivePresentation
		&& CurrentProfile.PresentationConfig.bEnableAutoJitter
		&& CurrentProfile.PresentationConfig.FireJitterAmplitude > 0.0f
		&& (!FMath::IsNearlyZero(HorizontalKick) || !FMath::IsNearlyZero(VerticalKick)))
	{
		FVector2D JitterDirection {HorizontalKick, -VerticalKick};
		if (JitterDirection.IsNearlyZero())
		{
			JitterDirection = FVector2D(1.0f, 0.0f);
		}
		else
		{
			JitterDirection.Normalize();
		}

		const float JitterAmplitude = CurrentProfile.PresentationConfig.FireJitterAmplitude;
		const float JitterMaxOffset = JitterAmplitude * 1.5f;
		FVector2D JitterOffset = CrosshairPresentationState.CurrentJitterOffset + (JitterDirection * JitterAmplitude);
		const float JitterOffsetSizeSq = JitterOffset.SizeSquared();
		if (JitterOffsetSizeSq > FMath::Square(JitterMaxOffset) && JitterOffsetSizeSq > KINDA_SMALL_NUMBER)
		{
			JitterOffset *= JitterMaxOffset / FMath::Sqrt(JitterOffsetSizeSq);
		}
		CrosshairPresentationState.CurrentJitterOffset = JitterOffset;
		CrosshairPresentationState.CurrentJitterStrength = 1.0f;
	}

	// 如果禁用扩散，开火也不产生后坐力扩散
	// If dynamic spread is disabled, firing will not produce recoil spread
	if (!CurrentProfile.DynamicConfig.bEnableDynamicSpread) { return; }
	
	float Max = CurrentProfile.DynamicConfig.MaxRecoilSpread;

	// 累加后坐力并 Clamp | Accumulate recoil and clamp
	RecoilSpread.X = FMath::Clamp(RecoilSpread.X + FMath::Abs(HorizontalKick), 0.0f, Max);
	RecoilSpread.Y = FMath::Clamp(RecoilSpread.Y + VerticalKick, 0.0f, Max);
}

void UHelsincyCrosshairComponent::DisableCrosshair()
{
	bEnabledCrosshair = false;
	ResetCrosshairPresentationState();
}

void UHelsincyCrosshairComponent::EnableCrosshair()
{
	bEnabledCrosshair = true;
	RefreshCurrentPrimaryColor();
	ResetCrosshairPresentationState();
}

void UHelsincyCrosshairComponent::DisableCenterDot()
{
	CurrentProfile.CenterDotConfig.bEnabled = false;
}

void UHelsincyCrosshairComponent::EnableCenterDot()
{
	CurrentProfile.CenterDotConfig.bEnabled = true;
}

void UHelsincyCrosshairComponent::DisableTargetDetection()
{
	CurrentProfile.VisualsConfig.bEnableTargetSwitching = false;
	ClearTarget();
	RefreshCurrentPrimaryColor();
	ResetCrosshairPresentationState();
}

void UHelsincyCrosshairComponent::EnableTargetDetection()
{
	CurrentProfile.VisualsConfig.bEnableTargetSwitching = true;
	RefreshCurrentPrimaryColor();
	ResetCrosshairPresentationState();
}

void UHelsincyCrosshairComponent::TriggerHitMarker()
{
	if (!CurrentProfile.HitMarkerConfig.bEnabled) return;

	// 添加一个新的 HitMarker 实例到列表
	// Add a new HitMarker instance to the list
	Internal_AddOrUpdateHitMarker(CurrentProfile.HitMarkerConfig.Color, EHitMarkerPriority::Low_Priority_Body, 1.0f, 0.5f);
}

void UHelsincyCrosshairComponent::TriggerHitMarkerColor(FLinearColor CustomColor)
{
	if (!CurrentProfile.HitMarkerConfig.bEnabled) return;

	Internal_AddOrUpdateHitMarker(CustomColor, EHitMarkerPriority::Low_Priority_Body, 1.0f, 0.5f);
}

void UHelsincyCrosshairComponent::TriggerHeadshotMarker()
{
	if (!CurrentProfile.HitMarkerConfig.bEnabled) return;
	
	Internal_AddOrUpdateHitMarker(CurrentProfile.HitMarkerConfig.HeadshotColor, EHitMarkerPriority::Medium_Priority_Head, CurrentProfile.HitMarkerConfig.HeadShotScale, 0.85f);
}

void UHelsincyCrosshairComponent::TriggerKillMarker()
{
	if (!CurrentProfile.HitMarkerConfig.bEnabled) return;
	
	Internal_AddOrUpdateHitMarker(CurrentProfile.HitMarkerConfig.KillColor, EHitMarkerPriority::High_Priority_Kill, CurrentProfile.HitMarkerConfig.KillScale, 1.0f);
}

void UHelsincyCrosshairComponent::TriggerHitMarkerAdvanced(EHitMarkerPriority Priority, FLinearColor CustomColor, float DamageNormalized)
{
	if (!CurrentProfile.HitMarkerConfig.bEnabled) return;

	const float ScaleSize =
		Priority == EHitMarkerPriority::High_Priority_Kill ? CurrentProfile.HitMarkerConfig.KillScale :
		Priority == EHitMarkerPriority::Medium_Priority_Head ? CurrentProfile.HitMarkerConfig.HeadShotScale :
		1.0f;
	Internal_AddOrUpdateHitMarker(CustomColor, Priority, ScaleSize, DamageNormalized);
}

void UHelsincyCrosshairComponent::SavePreset(FName PresetName)
{
	if (!PresetName.IsNone())
	{
		PresetLibrary.Add(PresetName, CurrentProfile);
		ActivePresetName = PresetName;
	}
}

bool UHelsincyCrosshairComponent::LoadPreset(FName PresetName)
{
	if (PresetLibrary.Contains(PresetName))
	{
		CurrentProfile = PresetLibrary[PresetName];
		ActivePresetName = PresetName;
		ClearRecoilAndSpread();
		RefreshCurrentPrimaryColor();
		ResetCrosshairPresentationState();
		ClearPendingSingleHitMarkerDowngrade();
		RefreshSingleHitMarkerStateView();
		return true;
	}
	return false;
}

// ============ 持久化 API | Persistence API ============

void UHelsincyCrosshairComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearLocalPlayerGuardRetry();

	// 清理异步 Trace 生命周期，防止回调在组件销毁后触发
	// Clean up async trace lifecycle to prevent callbacks after component destruction
	TraceDelegate.Unbind();
	AsyncTraceHandle = FTraceHandle();
	AsyncTraceWaitFrames = 0;

	if (bAutoSaveOnEndPlay && bOwnerCheckPassed && OwningPawn)
	{
		SavePresetsToDisk(AutoSaveSlotName);
	}

	ResetCrosshairPresentationState();

	Super::EndPlay(EndPlayReason);
}

bool UHelsincyCrosshairComponent::SavePresetsToDisk(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("CrosshairPresets")) : SlotName;

	UHelsincyCrosshairSaveGame* SaveGameObj = NewObject<UHelsincyCrosshairSaveGame>();
	SaveGameObj->SavedPresets = PresetLibrary;
	SaveGameObj->LastActivePresetName = ActivePresetName;
	SaveGameObj->SaveVersion = UHelsincyCrosshairSaveGame::CurrentSaveVersion;
	
	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObj, FinalSlot, UserIndex);

	UE_CLOG(!bSuccess, LogHelsincyCrosshair, Warning,
		TEXT("[HC][Persistence] Failed to save presets to slot '%s' (UserIndex=%d)."),
		*FinalSlot, UserIndex);

	UE_CLOG(bSuccess && HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
		TEXT("[HC][Persistence] Saved %d preset(s) to slot '%s'. ActivePreset='%s'."),
		PresetLibrary.Num(), *FinalSlot, *ActivePresetName.ToString());

	return bSuccess;
}

bool UHelsincyCrosshairComponent::LoadPresetsFromDisk(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("CrosshairPresets")) : SlotName;

	if (!UGameplayStatics::DoesSaveGameExist(FinalSlot, UserIndex))
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
			TEXT("[HC][Persistence] No save found in slot '%s' (UserIndex=%d)."),
			*FinalSlot, UserIndex);
		return false;
	}

	USaveGame* LoadedRaw = UGameplayStatics::LoadGameFromSlot(FinalSlot, UserIndex);
	UHelsincyCrosshairSaveGame* SaveGameObj = Cast<UHelsincyCrosshairSaveGame>(LoadedRaw);

	if (!SaveGameObj)
	{
		UE_LOG(LogHelsincyCrosshair, Warning,
			TEXT("[HC][Persistence] Failed to load or cast save from slot '%s'."),
			*FinalSlot);
		return false;
	}

	// 数据迁移 (如果版本落后) | Data migration (if version is outdated)
	if (SaveGameObj->SaveVersion < UHelsincyCrosshairSaveGame::CurrentSaveVersion)
	{
		SaveGameObj->MigrateData();
	}

	// 恢复预设库 | Restore preset library
	PresetLibrary = SaveGameObj->SavedPresets;

	// 自动应用上次活跃预设 | Auto-apply last active preset
	if (!SaveGameObj->LastActivePresetName.IsNone() && PresetLibrary.Contains(SaveGameObj->LastActivePresetName))
	{
		LoadPreset(SaveGameObj->LastActivePresetName);
	}

	UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
		TEXT("[HC][Persistence] Loaded %d preset(s) from slot '%s'. ActivePreset='%s'."),
		PresetLibrary.Num(), *FinalSlot, *ActivePresetName.ToString());

	return true;
}

bool UHelsincyCrosshairComponent::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("CrosshairPresets")) : SlotName;
	return UGameplayStatics::DoesSaveGameExist(FinalSlot, UserIndex);
}

bool UHelsincyCrosshairComponent::DeleteSaveFromDisk(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("CrosshairPresets")) : SlotName;
	return UGameplayStatics::DeleteGameInSlot(FinalSlot, UserIndex);
}

TArray<FName> UHelsincyCrosshairComponent::GetAllPresetNames() const
{
	TArray<FName> Names;
	PresetLibrary.GetKeys(Names);
	return Names;
}

void UHelsincyCrosshairComponent::UpdateProfile(const FHelsincyCrosshairProfile& NewProfile)
{
	CurrentProfile = NewProfile;
	ClearRecoilAndSpread();
	RefreshCurrentPrimaryColor();
	ResetCrosshairPresentationState();
	ClearPendingSingleHitMarkerDowngrade();
	RefreshSingleHitMarkerStateView();
}

void UHelsincyCrosshairComponent::LoadFromDataAsset(UHelsincyCrosshairDataAsset* DataAsset)
{
	if (DataAsset)
	{
		// 深拷贝：将资产里的配置复制给当前的运行时配置
		// Deep copy: copy the asset's config to the runtime config
		// 这样后续对 CurrentProfile 的修改（如动态扩散计算、玩家改色）不会污染资产
		// Later modifications to CurrentProfile (spread calculation, color changes) won't pollute the asset
		this->CurrentProfile = DataAsset->Profile;
		HelsincySingleHitMarkerSpriteSupport::ApplyDefaultSpriteBindings(CurrentProfile.HitMarkerConfig, DataAsset);
        
		// 如果你有特殊的逻辑需要重置（比如立刻重置扩散值），可以在这里做
		// Reset any special state here if needed (e.g. immediately reset spread values)
		ClearRecoilAndSpread();
		RefreshCurrentPrimaryColor();
		ResetCrosshairPresentationState();
		ClearPendingSingleHitMarkerDowngrade();
		RefreshSingleHitMarkerStateView();
	}
}

void UHelsincyCrosshairComponent::RefreshCurrentPrimaryColor()
{
	auto& RenderVisuals {CurrentProfile.VisualsConfig};

	// 计算动态颜色 (仅在栈上操作 FLinearColor，极快)
	// Calculate dynamic color (stack-only FLinearColor operations, very fast)
	// 默认使用配置的基础色 | Default to the configured primary color
	FLinearColor FinalColor = RenderVisuals.PrimaryColor; 

	if (RenderVisuals.bEnableTargetSwitching)
	{
		const auto Team {GetCurrentTargetAttitude()};
		FName ColorReason = NAME_None;

		// --- 颜色覆盖逻辑 | Color Override Logic ---
    
		bool bColorOverridden = false;
		FGameplayTag TargetTag {GetCurrentTargetTag()};

		// 通过阵营ID更新准星颜色 | Update crosshair color by team attitude
		UpdateCrosshairColorByTeam(RenderVisuals, Team, FinalColor, bColorOverridden, &ColorReason);

		// 通过FGameplayTag中的交互Tag来更新准星颜色
		// Update crosshair color by interaction Tag from FGameplayTag
		UpdateCrosshairColorByInteractionTag(RenderVisuals, TargetTag, FinalColor, bColorOverridden, &ColorReason);

		// 通用交互 (如果没被 Map 覆盖，也没被阵营覆盖，但确实是可交互物体)
		// Generic interaction (if not overridden by Map or team, but target is interactable)
		UpdateCrosshairColorByDefaultInteractionColor(RenderVisuals, TargetTag, FinalColor, bColorOverridden, &ColorReason);

		// 更新中立颜色(如果已启用开关)
		// Update neutral color (if toggle is enabled)
		UpdateCrosshairColorByNeutral(RenderVisuals, Team, FinalColor, bColorOverridden, &ColorReason);

		bCurrentVisualColorOverrideCache = bColorOverridden;
		CurrentVisualColorReasonCache = ColorReason;
	}
	else
	{
		bCurrentVisualColorOverrideCache = false;
		CurrentVisualColorReasonCache = NAME_None;
	}
	CurrentVisualPrimaryColorCache = FinalColor;
}

void UHelsincyCrosshairComponent::ResetCrosshairPresentationState()
{
	CrosshairPresentationState = FHelsincy_CrosshairPresentationState{};
	CrosshairPresentationState.TargetColor = CurrentVisualPrimaryColorCache;
	CrosshairPresentationState.CurrentColor = CurrentVisualPrimaryColorCache;
	CrosshairPresentationState.bHasTargetColorOverride = bCurrentVisualColorOverrideCache;
	CrosshairPresentationState.ActiveColorReason = CurrentVisualColorReasonCache;
}

void UHelsincyCrosshairComponent::UpdateCrosshairPresentationState(float DeltaTime)
{
	const FHelsincy_CrosshairPresentationState PreviousState = CrosshairPresentationState;
	const FHelsincy_CrosshairPresentationSettings& PresentationConfig = CurrentProfile.PresentationConfig;

	FHelsincy_CrosshairPresentationState NewState{};
	NewState.TargetColor = CurrentVisualPrimaryColorCache;
	NewState.bHasTargetColorOverride = bCurrentVisualColorOverrideCache;
	NewState.ActiveColorReason = CurrentVisualColorReasonCache;

	if (!PresentationConfig.bEnableAdaptivePresentation)
	{
		NewState.CurrentColor = NewState.TargetColor;
		CrosshairPresentationState = NewState;
		return;
	}

	if (PresentationConfig.bEnableAutoColorBlend)
	{
		if (PresentationConfig.ColorInterpSpeed <= 0.0f)
		{
			NewState.CurrentColor = NewState.TargetColor;
		}
		else
		{
			NewState.CurrentColor = FMath::CInterpTo(PreviousState.CurrentColor, NewState.TargetColor, DeltaTime, PresentationConfig.ColorInterpSpeed);
		}
	}
	else
	{
		NewState.CurrentColor = NewState.TargetColor;
	}

	if (PresentationConfig.bEnableAutoScale)
	{
		float MaxReferenceSpeed = 0.0f;
		float CurrentPlanarSpeed = 0.0f;
		bool bIsAirborne = false;

		if (OwnerCharacter)
		{
			CurrentPlanarSpeed = OwnerCharacter->GetVelocity().Size2D();

			if (UCharacterMovementComponent* CharacterMovement = OwnerCharacter->GetCharacterMovement())
			{
				MaxReferenceSpeed = CharacterMovement->GetMaxSpeed();
				bIsAirborne = CharacterMovement->IsFalling();
			}
		}
		else if (OwningPawn)
		{
			CurrentPlanarSpeed = OwningPawn->GetVelocity().Size2D();

			if (UPawnMovementComponent* PawnMovement = OwningPawn->GetMovementComponent())
			{
				MaxReferenceSpeed = PawnMovement->GetMaxSpeed();
				bIsAirborne = PawnMovement->IsFalling();
			}
		}

		const float MovementAlpha = MaxReferenceSpeed > KINDA_SMALL_NUMBER
			? FMath::Clamp(CurrentPlanarSpeed / MaxReferenceSpeed, 0.0f, 1.0f)
			: 0.0f;
		const float MoveScaleDelta = PresentationConfig.MoveScaleMax - PresentationConfig.IdleScale;
		const float AirborneScaleDelta = PresentationConfig.AirborneScaleMax - PresentationConfig.IdleScale;
		const float MaxRecoilSpread = CurrentProfile.DynamicConfig.MaxRecoilSpread;
		const float RecoilAlpha = MaxRecoilSpread > KINDA_SMALL_NUMBER
			? FMath::Clamp(RecoilSpread.GetAbsMax() / MaxRecoilSpread, 0.0f, 1.0f)
			: 0.0f;
		const float FirePulseScaleDelta = PresentationConfig.FireScalePulse - PresentationConfig.IdleScale;
		const float InterpSpeed = PresentationConfig.ScaleInterpSpeed;
		const float FirePulseRecoverySpeed = FMath::Max(InterpSpeed * 2.0f, 1.0f);

		NewState.MovementContribution = MoveScaleDelta * MovementAlpha;
		NewState.AirborneContribution = bIsAirborne ? AirborneScaleDelta : 0.0f;
		NewState.RecoilContribution = PresentationConfig.RecoilScaleContribution * RecoilAlpha;
		NewState.FirePulseStrength = FMath::FInterpTo(PreviousState.FirePulseStrength, 0.0f, DeltaTime, FirePulseRecoverySpeed);
		NewState.TargetScale = PresentationConfig.IdleScale
			+ NewState.MovementContribution
			+ NewState.AirborneContribution
			+ NewState.RecoilContribution;

		if (InterpSpeed <= 0.0f)
		{
			NewState.CurrentScale = NewState.TargetScale;
		}
		else
		{
			const float PreviousScale = PreviousState.CurrentScale > 0.0f
				? PreviousState.CurrentScale
				: PresentationConfig.IdleScale;
			NewState.CurrentScale = FMath::FInterpTo(PreviousScale, NewState.TargetScale, DeltaTime, InterpSpeed);
		}

		NewState.CurrentScale += FirePulseScaleDelta * NewState.FirePulseStrength;
	}

	if (PresentationConfig.bEnableAutoJitter)
	{
		const float JitterRecoverySpeed = FMath::Max(PresentationConfig.JitterRecoverySpeed, 1.0f);
		const float MaxRecoilSpread = CurrentProfile.DynamicConfig.MaxRecoilSpread;
		const float RecoilAlpha = MaxRecoilSpread > KINDA_SMALL_NUMBER
			? FMath::Clamp(RecoilSpread.GetAbsMax() / MaxRecoilSpread, 0.0f, 1.0f)
			: 0.0f;

		FVector2D TargetJitterOffset = FVector2D::ZeroVector;
		if (!RecoilSpread.IsNearlyZero())
		{
			FVector2D RecoilDirection {RecoilSpread.X, -RecoilSpread.Y};
			if (!RecoilDirection.IsNearlyZero())
			{
				RecoilDirection.Normalize();
				TargetJitterOffset = RecoilDirection * (PresentationConfig.FireJitterAmplitude * 0.25f * RecoilAlpha);
			}
		}

		NewState.CurrentJitterOffset.X = FMath::FInterpTo(PreviousState.CurrentJitterOffset.X, TargetJitterOffset.X, DeltaTime, JitterRecoverySpeed);
		NewState.CurrentJitterOffset.Y = FMath::FInterpTo(PreviousState.CurrentJitterOffset.Y, TargetJitterOffset.Y, DeltaTime, JitterRecoverySpeed);

		const float DecayedJitterStrength = FMath::FInterpTo(PreviousState.CurrentJitterStrength, 0.0f, DeltaTime, JitterRecoverySpeed);
		NewState.CurrentJitterStrength = FMath::Max(DecayedJitterStrength, RecoilAlpha * 0.35f);
	}

	CrosshairPresentationState = NewState;
}

void UHelsincyCrosshairComponent::UpdateCrosshairColorByInteractionTag(const FHelsincy_VisualSettings& Vis, const FGameplayTag& Tag, FLinearColor& Color, bool& bOverride, FName* OutReason)
{
	// 检查交互 Tag 映射 | Check interaction Tag mapping
	if (Tag.IsValid() && !bOverride)
	{
		// [优化算法] 向父级回溯查找 (Parent Traversal)
		// [Optimization] Parent traversal lookup
		// 复杂度: O(Depth)，通常 Depth < 5。远优于 O(N) 遍历。
		// Complexity: O(Depth), typically Depth < 5. Much better than O(N) traversal.
		FGameplayTag TempTag = Tag;

		// 循环向上查找父级 | Loop upward to find parent
		while (TempTag.IsValid())
		{
			// TMap::Find 是哈希查找，极快 O(1) | TMap::Find is hash lookup, O(1)
			if (const FLinearColor* TempMappedColor = Vis.InteractionColorMap.Find(TempTag))
			{
				Color = *TempMappedColor;
				bOverride = true;
				if (OutReason)
				{
					*OutReason = TempTag.GetTagName();
				}
				break; // 找到了最近的父级配置，停止 | Found nearest parent config, stop
			}
			// 没找到，尝试获取直接父级 (例如 Loot.Gold -> Loot)
			// Not found, try getting direct parent (e.g. Loot.Gold -> Loot)
			TempTag = TempTag.RequestDirectParent();
		}
	}
}

void UHelsincyCrosshairComponent::UpdateCrosshairColorByTeam(const FHelsincy_VisualSettings& Vis, const ETeamAttitude::Type TeamAttitude, FLinearColor& Color, bool& bOverride, FName* OutReason)
{
	// 检查阵营 (如果没被交互覆盖) | Check team attitude (if not overridden by interaction)
	if (!bOverride)
	{
		switch (TeamAttitude)
		{
		case ETeamAttitude::Hostile:
			Color = Vis.EnemyColor;
			bOverride = true;
			if (OutReason)
			{
				*OutReason = TEXT("TeamHostile");
			}
			break;
		case ETeamAttitude::Friendly:
			Color = Vis.FriendlyColor;
			bOverride = true;
			if (OutReason)
			{
				*OutReason = TEXT("TeamFriendly");
			}
			break;
		default:
			break;
		}
	}
}

void UHelsincyCrosshairComponent::UpdateCrosshairColorByDefaultInteractionColor(const FHelsincy_VisualSettings& Vis, const FGameplayTag& Tag, FLinearColor& Color, bool& bOverride, FName* OutReason)
{
	if (!bOverride && Tag.IsValid() && Vis.bUseDefaultInteractionColor)
	{
		Color = Vis.DefaultInteractionColor;
		bOverride = true;
		if (OutReason)
		{
			*OutReason = Tag.GetTagName();
		}
	}
}

void UHelsincyCrosshairComponent::UpdateCrosshairColorByNeutral(const FHelsincy_VisualSettings& Vis, const ETeamAttitude::Type TeamAttitude, FLinearColor& Color, bool& bOverride, FName* OutReason)
{
	if (bOverride || !Vis.bUseNeutralColor) return;
	
	switch (TeamAttitude)
	{
	case ETeamAttitude::Neutral:
		Color = Vis.NeutralColor;
		bOverride = true;
		if (OutReason)
		{
			*OutReason = TEXT("TeamNeutral");
		}
		break;
	default:
		break;
	}
}

void UHelsincyCrosshairComponent::Internal_AddOrUpdateHitMarker(FLinearColor Color, EHitMarkerPriority Priority, float ScaleSize, float DamageNormalized)
{
	if (!CurrentProfile.HitMarkerConfig.bEnabled || !bEnabledCrosshair) return;

	// ===== 单实例模式 (COD 风格) | Single-Instance Mode (COD-Style) =====
	if (CurrentProfile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance)
	{
		const FHelsincy_HitMarkerProfile& HitConfig = CurrentProfile.HitMarkerConfig;
		const bool bWasActive = SingleHitMarkerState.bActive;
		const EHitMarkerPriority PreviousPriority = SingleHitMarkerState.Priority;
		const float PreviousImpactEnergy = SingleHitMarkerState.ImpactEnergy;
		float PulseScale = HitConfig.HitPulseScale;
		if (Priority == EHitMarkerPriority::High_Priority_Kill)
		{
			PulseScale = HitConfig.KillPulseScale;
		}
		const int32 ShakeSeed = AllocateHitMarkerShakeSeed(HitMarkerShakeSeedCounter);

		SingleHitMarkerState.ApplyHit(
			HitConfig.Duration,
			HitConfig.Thickness,
			Color, Priority, ScaleSize, PulseScale
		);

		SingleHitMarkerState.ImpactEnergy = FMath::Clamp(
			PreviousImpactEnergy + FMath::Max(0.0f, PulseScale - 1.0f),
			0.0f,
			HitConfig.SingleInstanceMaxImpactEnergy
		);
		SingleHitMarkerState.AccentStrength = 1.0f;
		InjectSingleHitMarkerShake(SingleHitMarkerState, HitConfig, Priority, PulseScale, ShakeSeed, DamageNormalized);

		if (!bWasActive || Priority >= PreviousPriority)
		{
			ClearPendingSingleHitMarkerDowngrade();
		}
		else
		{
			QueuePendingSingleHitMarkerDowngrade(HitConfig, Color, Priority, ScaleSize);
		}

		RefreshSingleHitMarkerStateView();
		return;
	}

	// ===== 多实例模式 (经典) | Multi-Instance Mode (Classic) =====

	if (Priority == EHitMarkerPriority::High_Priority_Kill && CurrentProfile.HitMarkerConfig.bClearAllOldHitMarkerOnKill)
	{
		// 击杀强制移除旧的，添加新的，确保动画从头播放 (0 -> 1 放大过程)
		// Kill: force remove old markers, add new one, ensure animation plays from start (0 -> 1 scale-up)
		// 这样给玩家强烈的"完成感"
		// This gives the player a strong sense of "finishing off"
		ActiveHitMarkers.Empty(); // 清空之前的干扰 | Clear previous interference
		FHelsincy_ActiveHitMarker NewMarker(
			CurrentProfile.HitMarkerConfig.Duration,
			CurrentProfile.HitMarkerConfig.Duration,
			CurrentProfile.HitMarkerConfig.Thickness,
			Color,
			Priority,
			ScaleSize
		);
		ConfigureActiveHitMarkerShake(
			NewMarker,
			CurrentProfile.HitMarkerConfig,
			Priority,
			AllocateHitMarkerShakeSeed(HitMarkerShakeSeedCounter),
			DamageNormalized
		);
		ActiveHitMarkers.Add(NewMarker);
		return;
	}

	// --- 核心优化：合并/升级逻辑 | Core Optimization: Merge/Upgrade Logic ---
    
	// 我们只检查最近的一个 Marker (数组末尾)
	// We only check the most recent Marker (array tail)
	// 如果你想支持多路 HitMarker (比如散弹枪多弹丸)，可以遍历整个数组，
	// For multi-pellet HitMarkers (e.g. shotguns), you could iterate the whole array,
	// 但通常中心 HitMarker 只需要关注最近的一个。
	// but center HitMarker typically only needs the most recent one.
	if (ActiveHitMarkers.Num() > 0)
	{
		FHelsincy_ActiveHitMarker& LastMarker = ActiveHitMarkers.Last();

		// 定义一个"合并窗口期" (Merge Window)
		// Define a "Merge Window"
		// 只有当上一个 Marker 刚生成不久 (比如已经播放了不到 20% 的时间)，我们才合并。
		// Only merge if the previous marker was just created (e.g. less than 20% of its duration has elapsed).
		// 如果上一个 Marker 已经快消失了，新的命中应该生成一个新的，而不是把快消失的那个突然变色。
		// If the previous marker is about to fade, the new hit should create a new one rather than recoloring the fading one.
		float TimeElapsed = LastMarker.TotalDuration - LastMarker.TimeRemaining;
		// 阈值内的连续命中视为同一次反馈流
		// Consecutive hits within threshold are treated as the same feedback stream
		// Clamp 防止 MergeThreshold > TotalDuration 导致永久合并
		// Clamp to prevent MergeThreshold > TotalDuration causing permanent merging
		float MergeWindow = FMath::Min(CurrentProfile.HitMarkerConfig.MergeThreshold, LastMarker.TotalDuration * 0.5f);

		if (TimeElapsed <= MergeWindow)
		{
			// 情况 A: 新的优先级更高 (例如: 白 -> 红)
			// Case A: New priority is higher (e.g. white -> red)
			// 动作: 升级颜色，升级优先级，重置时间 (让动画重新开始或延续)
			// Action: Upgrade color & priority, reset timer (restart or extend animation)
			if (Priority > LastMarker.Priority)
			{
				LastMarker.CurrentColor = Color;
				LastMarker.Priority = Priority;
				LastMarker.TimeRemaining = LastMarker.TotalDuration; // 重置时间，让它多显示一会儿 | Reset timer for longer display
				RefreshActiveHitMarkerShakeEnergy(LastMarker, CurrentProfile.HitMarkerConfig, Priority, DamageNormalized);
				// LastMarker.TotalDuration = ...; // 如果不同类型时长不同，这里也要更新 | Update if different types have different durations
				return; // 已处理，退出 | Handled, exit
			}
            
			// 情况 B: 优先级相同 (例如: 白 -> 白，或者 红 -> 红)
			// Case B: Same priority (e.g. white -> white, or red -> red)
			// 动作: 仅重置时间 (Refill)，保持反馈持续显示，不需要添加新实例
			// Action: Only reset timer (refill), keep feedback displaying, no new instance needed
			else if (Priority == LastMarker.Priority)
			{
				LastMarker.TimeRemaining = LastMarker.TotalDuration;
				RefreshActiveHitMarkerShakeEnergy(LastMarker, CurrentProfile.HitMarkerConfig, Priority, DamageNormalized);
				return; // 已处理，退出 | Handled, exit
			}

			// 情况 C: 新的优先级更低 (例如: 红 -> 白)
			// Case C: New priority is lower (e.g. red -> white)
			// 动作: 忽略这次"降级"请求，或者仅重置时间但保持红色。
			// Action: Ignore this "downgrade" request, or only reset timer while keeping the red color.
			// 通常 FPS 中，只要打中头，即使同时擦伤脚，也应该显示爆头反馈。
			// In typical FPS, a headshot should display headshot feedback even if a leg is grazed simultaneously.
			else 
			{
				// 也可以选择 LastMarker.TimeRemaining = LastMarker.TotalDuration; 来延长显示
				// Alternatively: LastMarker.TimeRemaining = LastMarker.TotalDuration; to extend display
				return; // 忽略低级反馈，退出 | Ignore lower-priority feedback, exit
			}
		}
	}

	// 如果没有可合并的 Marker，或者超出了合并阈值，则添加新的
	// If no mergeable Marker exists, or merge threshold exceeded, add a new one
	// 防御性上限：避免极端场景下内存持续增长
	// Defensive cap: prevent unbounded memory growth in extreme scenarios
	if (ActiveHitMarkers.Num() >= 32)
	{
		ActiveHitMarkers.RemoveAt(0);
	}
	FHelsincy_ActiveHitMarker NewMarker(
		CurrentProfile.HitMarkerConfig.Duration,
		CurrentProfile.HitMarkerConfig.Duration,
		CurrentProfile.HitMarkerConfig.Thickness,
		Color,
		Priority,
		ScaleSize
	);
	ConfigureActiveHitMarkerShake(
		NewMarker,
		CurrentProfile.HitMarkerConfig,
		Priority,
		AllocateHitMarkerShakeSeed(HitMarkerShakeSeedCounter),
		DamageNormalized
	);
	ActiveHitMarkers.Add(NewMarker);
}

void UHelsincyCrosshairComponent::RefreshTickState()
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		OwningPawn = Pawn;
		OwnerCharacter = Cast<ACharacter>(Pawn);

		if (!Pawn->GetController())
		{
			if (!bPendingOwnerCheck)
			{
				PendingCheckFrameCount = 0;
			}
			bPendingOwnerCheck = true;
			ResetCrosshairPresentationState();
			ScheduleLocalPlayerGuardRetry();
			return;
		}

		ClearLocalPlayerGuardRetry();
		bPendingOwnerCheck = false;
		PendingCheckFrameCount = 0;
		bLocalPlayerGuardTimeoutLogged = false;

		bool bShouldTick = ShouldActivateForOwner();
		SetComponentTickEnabled(bShouldTick);
		
		if (bShouldTick && !IsActive())
		{
			Activate(); // 重新激活 | Reactivate
		}
		else if (!bShouldTick && IsActive())
		{
			Deactivate(); // 休眠 | Deactivate
		}
		if (bShouldTick)
		{
			bOwnerCheckPassed = true;
		}
		else
		{
			OwningPawn = nullptr;
			OwnerCharacter = nullptr;
			ResetCrosshairPresentationState();
		}

		RefreshCurrentPrimaryColor();
		ResetCrosshairPresentationState();
	}
	else
	{
		ResetCrosshairPresentationState();
	}
}

#if WITH_DEV_AUTOMATION_TESTS
void UHelsincyCrosshairComponent::Debug_SetPendingLocalPlayerGuardForAutomation(const int32 InitialFrameCount)
{
	bPendingOwnerCheck = true;
	PendingCheckFrameCount = InitialFrameCount;
}

void UHelsincyCrosshairComponent::Debug_TickPendingLocalPlayerGuardForAutomation()
{
	TickPendingLocalPlayerGuard();
}
#endif

void UHelsincyCrosshairComponent::ScheduleLocalPlayerGuardRetry()
{
	if (UWorld* World = GetWorld())
	{
		if (!LocalPlayerGuardRetryTimerHandle.IsValid())
		{
			World->GetTimerManager().SetTimer(
				LocalPlayerGuardRetryTimerHandle,
				this,
				&UHelsincyCrosshairComponent::RetryLocalPlayerGuard,
				LocalPlayerGuardRetryIntervalSeconds,
				true);
		}
		if (IsRegistered())
		{
			SetComponentTickEnabled(false);
		}
	}
	else
	{
		if (IsRegistered())
		{
			SetComponentTickEnabled(true);
		}
	}
}

void UHelsincyCrosshairComponent::ClearLocalPlayerGuardRetry()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LocalPlayerGuardRetryTimerHandle);
	}
	LocalPlayerGuardRetryTimerHandle.Invalidate();
}

void UHelsincyCrosshairComponent::RetryLocalPlayerGuard()
{
	TickPendingLocalPlayerGuard();
}

bool UHelsincyCrosshairComponent::RefreshLocalPlayerGuardForRendering()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return false;
	}

	OwningPawn = Pawn;
	OwnerCharacter = Cast<ACharacter>(Pawn);

	if (!Pawn->GetController())
	{
		if (!bPendingOwnerCheck)
		{
			PendingCheckFrameCount = 0;
		}
		bPendingOwnerCheck = true;
		ResetCrosshairPresentationState();
		ScheduleLocalPlayerGuardRetry();
		return false;
	}

	if (bPendingOwnerCheck)
	{
		TickPendingLocalPlayerGuard();
	}

	if (!ShouldActivateForOwner())
	{
		bPendingOwnerCheck = false;
		ClearLocalPlayerGuardRetry();
		OwningPawn = nullptr;
		OwnerCharacter = nullptr;
		ResetCrosshairPresentationState();
		SetComponentTickEnabled(false);
		Deactivate();
		return false;
	}

	if (!bOwnerCheckPassed)
	{
		bOwnerCheckPassed = true;
		SetComponentTickEnabled(true);
		if (!IsActive()) Activate();
		if (!bLocalPlayerInitializationComplete)
		{
			PerformFullInitialization();
			bLocalPlayerInitializationComplete = true;
		}
	}

	return true;
}

bool UHelsincyCrosshairComponent::TickPendingLocalPlayerGuard()
{
	// --- 延迟判定路径: 等待 Controller 就位 | Deferred check: waiting for Controller ---
	if (!bPendingOwnerCheck)
	{
		return false;
	}

	ResetCrosshairPresentationState();
	if (OwningPawn && OwningPawn->GetController())
	{
		// Controller 到位, 做最终判定 | Controller assigned, perform final check
		bPendingOwnerCheck = false;
		PendingCheckFrameCount = 0;
		bLocalPlayerGuardTimeoutLogged = false;
		ClearLocalPlayerGuardRetry();
		if (ShouldActivateForOwner())
		{
			bOwnerCheckPassed = true;
			SetComponentTickEnabled(true);
			if (!IsActive()) Activate();
			if (!bLocalPlayerInitializationComplete)
			{
				PerformFullInitialization();
				bLocalPlayerInitializationComplete = true;
			}
		}
		else
		{
			OwningPawn = nullptr;
			OwnerCharacter = nullptr;
			ResetCrosshairPresentationState();
			SetComponentTickEnabled(false);
			Deactivate();
		}
	}
	else
	{
		PendingCheckFrameCount++;
		if (!bLocalPlayerGuardTimeoutLogged && PendingCheckFrameCount >= MaxPendingCheckFrames)
		{
			UE_LOG(LogHelsincyCrosshair, Warning,
				TEXT("[HC][Comp] Controller not assigned after %d checks on '%s'. Keeping local-player guard pending for late possession."),
				MaxPendingCheckFrames, *GetNameSafe(GetOwner()));
			bLocalPlayerGuardTimeoutLogged = true;
		}
		ScheduleLocalPlayerGuardRetry();
	}

	return true; // 等待期间不执行正常 Tick | Skip normal tick while waiting
}

bool UHelsincyCrosshairComponent::ShouldActivateForOwner() const
{
	if (!OwningPawn) return false;

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC) return false; // 排除 AI (AAIController) + 无 Controller | Exclude AI + no Controller

	return PC->IsLocalController(); // 排除远端真人 + 模拟代理 | Exclude remote players + simulated proxies
}

void UHelsincyCrosshairComponent::PerformFullInitialization()
{
	if (bUseDefaultCrosshairAssetInit)
	{
		if (!DefaultCrosshairAsset)
		{
			UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
				TEXT("[HC][Comp] DefaultCrosshairAsset is NULL on Pawn '%s'. LoadFromDataAsset will be skipped."),
				*GetNameSafe(OwningPawn));
		}
		LoadFromDataAsset(DefaultCrosshairAsset);
	}

	// 如果忘记设置准星类型就使用默认的十字准星
	// If ShapeTag was not set, fall back to the default Cross crosshair
	if (!CurrentProfile.ShapeTag.IsValid())
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
			TEXT("[HC][Comp] ShapeTag is invalid on Pawn '%s', falling back to Cross."),
			*GetNameSafe(OwningPawn));
		CurrentProfile.ShapeTag = FHelsincyCrosshair_Tags::Shape_Cross;
	}

	SetupPlayerCameraManager();

	// 绑定回调函数
	// Bind callback delegate
	TraceDelegate.BindUObject(this, &ThisClass::OnTraceCompleted);

	// 自动加载持久化预设 | Auto-load persisted presets
	if (bAutoLoadOnBeginPlay)
	{
		LoadPresetsFromDisk(AutoSaveSlotName);
	}

	ResetCrosshairPresentationState();
}

void UHelsincyCrosshairComponent::SetupPlayerCameraManager()
{
	if (!OwningPawn) return;

	auto TempController {Cast<APlayerController>(OwningPawn->GetController())};
	if (TempController)
	{
		PlayerCameraManagerWeak = TempController->PlayerCameraManager;
	}
}

void UHelsincyCrosshairComponent::RequestAsyncTargetDetection()
{
	SCOPE_CYCLE_COUNTER(STAT_HC_TargetDetectionRequest);

	if (!CurrentProfile.VisualsConfig.bEnableTargetSwitching)
	{
		CurrentTargetAttitude = ETeamAttitude::Neutral;
		return;
	};

	UWorld* World = GetWorld();
	if (!World) return;
	if (AsyncTraceHandle.IsValid())
	{
		// 超时保护: 防止回调永不触发导致目标检测永久停止
		// Timeout protection: prevent target detection from permanently stopping if callback never fires
		if (++AsyncTraceWaitFrames > AsyncTraceMaxWaitFrames)
		{
			AsyncTraceHandle = FTraceHandle();
			AsyncTraceWaitFrames = 0;
		}
		else
		{
			return;
		}
	}
	AsyncTraceWaitFrames = 0;
	
	// 准备参数 | Prepare parameters
	FVector Start;
	FRotator Rot;
	AActor* OwnerActor = GetOwner();
	if (PlayerCameraManagerWeak.IsValid())
	{
		APlayerCameraManager* PlayerCameraManager = PlayerCameraManagerWeak.Get();
		Start = PlayerCameraManager->GetCameraLocation();
		Rot = PlayerCameraManager->GetCameraRotation();
	}
	else
	{
		if (!OwnerActor)
		{
			ClearTarget();
			return;
		}
		OwnerActor->GetActorEyesViewPoint(Start, Rot);
	}
	FVector End = Start + (Rot.Vector() * CurrentProfile.VisualsConfig.MaxSwitchingDistance);

	FCollisionQueryParams Params;
	if (AActor* IgnoredActor = OwningPawn ? static_cast<AActor*>(OwningPawn) : OwnerActor)
	{
		Params.AddIgnoredActor(IgnoredActor);
	}
	// 优化：不进行复杂碰撞检测，只检测简单碰撞体
	// Optimization: skip complex collision, only test simple collision bodies
	Params.bTraceComplex = false;

	// 3. 发送异步请求 | 3. Send async trace request
	// ECC_Pawn 或者你自定义的 TraceChannel | ECC_Pawn or your custom TraceChannel
	// 注意：AsyncLineTraceByChannel 返回一个 Handle
	// Note: AsyncLineTraceByChannel returns a Handle
	AsyncTraceHandle = World->AsyncLineTraceByChannel(
		EAsyncTraceType::Single, // 单次检测 | Single trace
		Start,
		End,
		CurrentProfile.VisualsConfig.SwitchingChannel,
		Params,
		FCollisionResponseParams::DefaultResponseParam,
		&TraceDelegate // 传入我们的委托 | Pass in our delegate
	);
}

void UHelsincyCrosshairComponent::OnTraceCompleted(const FTraceHandle& Handle, FTraceDatum& Datum)
{
	SCOPE_CYCLE_COUNTER(STAT_HC_TargetDetectionCallback);

	if (!Handle.IsValid())
	{
		// 重置句柄 | Reset handle
		AsyncTraceHandle = FTraceHandle();
		ClearTarget();
		return;
	}
	
	// 检查是否有命中 | Check if there was a hit
	if (Datum.OutHits.Num() > 0)
	{
		// 处理结果 (AsyncTraceByChannel 虽然叫 Single，但 Datum 依然返回数组，通常取第0个)
		// Process result (AsyncTraceByChannel named Single still returns an array; typically take index 0)
		ProcessHitResult(Datum.OutHits[0]);
	}
	else
	{
		// 没命中 | No hit
		ClearTarget();
	}
	// 重置句柄 | Reset handle
	AsyncTraceHandle = FTraceHandle();
}

void UHelsincyCrosshairComponent::ProcessHitResult(const FHitResult& Hit)
{
	if (!CurrentProfile.VisualsConfig.bEnableTargetSwitching)
	{
		CurrentTargetAttitude = ETeamAttitude::Neutral;
		return;
	}
	
	AActor* Target = Hit.GetActor();
	if (!Target) 
	{
		ClearTarget();
		return;
	}

	UpdateCurrentTargetTag(Target);
}

void UHelsincyCrosshairComponent::ClearTarget()
{
	CurrentTargetTag = FGameplayTag::EmptyTag;
	CurrentTargetAttitude = ETeamAttitude::Neutral;
}

void UHelsincyCrosshairComponent::UpdateCurrentTargetTag(AActor* Actor)
{
	if (!Actor)
	{
		ClearTarget();
		return;
	}

	// 先重置旧状态，避免不同接口之间切换时残留旧值
	// Reset old state first to prevent stale values when switching between interfaces
	ClearTarget();

	if (Actor->Implements<UGenericTeamAgentInterface>())
	{
		CurrentTargetAttitude = FGenericTeamId::GetAttitude(GetOwner(), Actor);
	}
	if (Actor->Implements<UHelsincyTargetInterface>())
	{
		CurrentTargetTag = IHelsincyTargetInterface::Execute_GetCrosshairInteractionTag(Actor);
	}
}

void UHelsincyCrosshairComponent::ClearRecoilAndSpread()
{
	StateSpread = FVector2D::ZeroVector;
	RecoilSpread = FVector2D::ZeroVector;
}

void UHelsincyCrosshairComponent::CalculateStateSpread(FVector2D& TargetStateSpread)
{
	// 如果没有 owner，归零 | If no owner, reset to zero
	if (!OwningPawn) 
	{
		StateSpread = FVector2D::ZeroVector;
		return;
	}

	float Speed = OwningPawn->GetVelocity().Size();
	bool bIsAirborne = false;

	if (OwnerCharacter)
	{
		if (UCharacterMovementComponent* CMC = OwnerCharacter->GetCharacterMovement())
		{
			bIsAirborne = CMC->IsFalling();
		}
	}
	else if (auto* TempMoveComp = OwningPawn->GetMovementComponent())
	{
		bIsAirborne = TempMoveComp->IsFalling();
	}

	// 计算基础数值 | Calculate base values
	const auto& Config = CurrentProfile.DynamicConfig;
    
	float SpreadValue = Speed * Config.VelocityMultiplier;
    
	if (bIsAirborne)
	{
		SpreadValue += Config.AirbornePenalty;
	}

	// 应用到轴 | Apply to axes
	TargetStateSpread.X = Config.bApplyMovementToX ? SpreadValue : 0.0f;
	TargetStateSpread.Y = Config.bApplyMovementToY ? SpreadValue : 0.0f;
}

void UHelsincyCrosshairComponent::UpdateRecoilRecovery(float DeltaTime)
{
	// ==========================================
	//  后坐力恢复 | Recoil Spread Recovery
	// ==========================================
	if (!RecoilSpread.IsNearlyZero())
	{
		float Recovery = CurrentProfile.DynamicConfig.RecoilRecoverySpeed;
		RecoilSpread.X = FMath::FInterpTo(RecoilSpread.X, 0.0f, DeltaTime, Recovery);
		RecoilSpread.Y = FMath::FInterpTo(RecoilSpread.Y, 0.0f, DeltaTime, Recovery);
	}
}

void UHelsincyCrosshairComponent::UpdateHitMarker(const float DeltaTime)
{
	// 单实例模式 | Single-instance mode
	if (CurrentProfile.HitMarkerConfig.Mode == EHitMarkerMode::SingleInstance)
	{
		UpdateSingleHitMarkerStateMachine(DeltaTime);
		return;
	}

	// --- 多实例模式: 更新 HitMarker | Multi-instance mode: Update HitMarker ---
	// 倒序遍历以便删除 | Iterate in reverse for safe removal
	if (ActiveHitMarkers.Num() > 0)
	{
		for (int32 i = ActiveHitMarkers.Num() - 1; i >= 0; i--)
		{
			ActiveHitMarkers[i].TimeRemaining -= DeltaTime;
			if (ActiveHitMarkers[i].TimeRemaining <= 0.0f)
			{
				ActiveHitMarkers.RemoveAtSwap(i);
			}
		}
	}
}

void UHelsincyCrosshairComponent::ClearPendingSingleHitMarkerDowngrade()
{
	bHasPendingSingleHitMarkerDowngrade = false;
	PendingSingleHitMarkerPriority = EHitMarkerPriority::Low_Priority_Body;
	PendingSingleHitMarkerColor = FLinearColor::White;
	PendingSingleHitMarkerScale = 1.0f;
	PendingSingleHitMarkerDelayRemaining = 0.0f;
}

void UHelsincyCrosshairComponent::QueuePendingSingleHitMarkerDowngrade(
	const FHelsincy_HitMarkerProfile& Config,
	const FLinearColor& Color,
	EHitMarkerPriority Priority,
	float ScaleSize)
{
	const float DowngradeDelay = GetSingleHitMarkerDowngradeDelay(Config);

	if (!bHasPendingSingleHitMarkerDowngrade || Priority > PendingSingleHitMarkerPriority)
	{
		PendingSingleHitMarkerPriority = Priority;
		PendingSingleHitMarkerColor = Color;
		PendingSingleHitMarkerScale = ScaleSize;
	}

	PendingSingleHitMarkerDelayRemaining = bHasPendingSingleHitMarkerDowngrade
		? FMath::Min(PendingSingleHitMarkerDelayRemaining, DowngradeDelay)
		: DowngradeDelay;
	bHasPendingSingleHitMarkerDowngrade = true;
}

void UHelsincyCrosshairComponent::ApplyPendingSingleHitMarkerDowngrade()
{
	if (!bHasPendingSingleHitMarkerDowngrade || !SingleHitMarkerState.bActive)
	{
		ClearPendingSingleHitMarkerDowngrade();
		return;
	}

	SingleHitMarkerState.Priority = PendingSingleHitMarkerPriority;
	SingleHitMarkerState.CurrentColor = PendingSingleHitMarkerColor;
	SingleHitMarkerState.BaseSizeScale = PendingSingleHitMarkerScale;
	SingleHitMarkerState.AccentStrength = FMath::Max(SingleHitMarkerState.AccentStrength, 0.35f);
	ClearPendingSingleHitMarkerDowngrade();
}

float UHelsincyCrosshairComponent::GetSingleHitMarkerDowngradeDelay(const FHelsincy_HitMarkerProfile& Config) const
{
	return FMath::Clamp(
		FMath::Max(Config.SingleInstanceAccentDuration, Config.Duration * 0.25f),
		0.03f,
		Config.Duration
	);
}

void UHelsincyCrosshairComponent::UpdateSingleHitMarkerStateMachine(float DeltaTime)
{
	const FHelsincy_HitMarkerProfile& HitConfig = CurrentProfile.HitMarkerConfig;

	SingleHitMarkerState.Tick(DeltaTime, HitConfig.HitPulseRecoverySpeed);
	if (!SingleHitMarkerState.bActive)
	{
		ClearPendingSingleHitMarkerDowngrade();
		RefreshSingleHitMarkerStateView();
		return;
	}

	SingleHitMarkerState.ImpactEnergy = FMath::FInterpTo(
		SingleHitMarkerState.ImpactEnergy,
		0.0f,
		DeltaTime,
		HitConfig.SingleInstanceImpactDecaySpeed
	);
	SingleHitMarkerState.ShakeEnergy = FMath::FInterpTo(
		SingleHitMarkerState.ShakeEnergy,
		0.0f,
		DeltaTime,
		HitConfig.SingleInstanceImpactDecaySpeed
	);
	SingleHitMarkerState.ImpactMotionEnergy = FMath::FInterpTo(
		SingleHitMarkerState.ImpactMotionEnergy,
		0.0f,
		DeltaTime,
		FMath::Max(HitConfig.SingleInstanceImpactDecaySpeed, 18.0f)
	);

	if (HitConfig.SingleInstanceAccentDuration > KINDA_SMALL_NUMBER)
	{
		SingleHitMarkerState.AccentStrength = FMath::Clamp(
			SingleHitMarkerState.AccentStrength - (DeltaTime / HitConfig.SingleInstanceAccentDuration),
			0.0f,
			1.0f
		);
	}
	else
	{
		SingleHitMarkerState.AccentStrength = 0.0f;
	}

	if (bHasPendingSingleHitMarkerDowngrade)
	{
		PendingSingleHitMarkerDelayRemaining = FMath::Max(0.0f, PendingSingleHitMarkerDelayRemaining - DeltaTime);
		if (PendingSingleHitMarkerDelayRemaining <= 0.0f && SingleHitMarkerState.Priority > PendingSingleHitMarkerPriority)
		{
			ApplyPendingSingleHitMarkerDowngrade();
		}
		else if (SingleHitMarkerState.Priority <= PendingSingleHitMarkerPriority)
		{
			ClearPendingSingleHitMarkerDowngrade();
		}
	}

	RefreshSingleHitMarkerStateView();
}

void UHelsincyCrosshairComponent::RefreshSingleHitMarkerStateView()
{
	if (CurrentProfile.HitMarkerConfig.Mode != EHitMarkerMode::SingleInstance)
	{
		SingleHitMarkerState.bActive = false;
		SingleHitMarkerState.TimeRemaining = 0.0f;
		SingleHitMarkerState.TotalDuration = 0.0f;
		SingleHitMarkerState.CurrentPulseScale = 1.0f;
		SingleHitMarkerState.bVisible = false;
		SingleHitMarkerState.Phase = EHelsincySingleHitMarkerPhase::Hidden;
		SingleHitMarkerState.HoldTimeRemaining = 0.0f;
		SingleHitMarkerState.TailFadeTimeRemaining = 0.0f;
		SingleHitMarkerState.Opacity = 0.0f;
		SingleHitMarkerState.ImpactEnergy = 0.0f;
		SingleHitMarkerState.AccentStrength = 0.0f;
		SingleHitMarkerState.ShakeEnergy = 0.0f;
		SingleHitMarkerState.ShakeAge = 0.0f;
		SingleHitMarkerState.ImpactMotionEnergy = 0.0f;
		SingleHitMarkerState.ImpactDamageScale = 0.0f;
		SingleHitMarkerState.ImpactMotionSign = 1.0f;
		ClearPendingSingleHitMarkerDowngrade();
		return;
	}

	const FHelsincy_HitMarkerProfile& HitConfig = CurrentProfile.HitMarkerConfig;
	if (!SingleHitMarkerState.bActive || SingleHitMarkerState.TimeRemaining <= 0.0f || SingleHitMarkerState.TotalDuration <= 0.0f)
	{
		SingleHitMarkerState.bVisible = false;
		SingleHitMarkerState.Phase = EHelsincySingleHitMarkerPhase::Hidden;
		SingleHitMarkerState.HoldTimeRemaining = 0.0f;
		SingleHitMarkerState.TailFadeTimeRemaining = 0.0f;
		SingleHitMarkerState.Opacity = 0.0f;
		SingleHitMarkerState.ImpactEnergy = 0.0f;
		SingleHitMarkerState.AccentStrength = 0.0f;
		SingleHitMarkerState.ShakeEnergy = 0.0f;
		SingleHitMarkerState.ShakeAge = 0.0f;
		SingleHitMarkerState.ImpactMotionEnergy = 0.0f;
		SingleHitMarkerState.ImpactDamageScale = 0.0f;
		SingleHitMarkerState.ImpactMotionSign = 1.0f;
		ClearPendingSingleHitMarkerDowngrade();
		return;
	}

	const float SafeFadeRatio = FMath::Clamp(HitConfig.SingleInstanceFadeRatio, 0.0f, 1.0f);
	const float FadeDuration = SingleHitMarkerState.TotalDuration * SafeFadeRatio;
	SingleHitMarkerState.bVisible = true;

	if (FadeDuration > KINDA_SMALL_NUMBER && SingleHitMarkerState.TimeRemaining <= FadeDuration)
	{
		SingleHitMarkerState.Phase = EHelsincySingleHitMarkerPhase::TailFade;
		SingleHitMarkerState.HoldTimeRemaining = 0.0f;
		SingleHitMarkerState.TailFadeTimeRemaining = SingleHitMarkerState.TimeRemaining;
		SingleHitMarkerState.Opacity = FMath::Clamp(SingleHitMarkerState.TimeRemaining / FadeDuration, 0.0f, 1.0f);
	}
	else
	{
		SingleHitMarkerState.Phase = EHelsincySingleHitMarkerPhase::ActiveHold;
		SingleHitMarkerState.HoldTimeRemaining = FMath::Max(0.0f, SingleHitMarkerState.TimeRemaining - FadeDuration);
		SingleHitMarkerState.TailFadeTimeRemaining = FadeDuration;
		SingleHitMarkerState.Opacity = 1.0f;
	}

	SingleHitMarkerState.ImpactEnergy = FMath::Clamp(
		SingleHitMarkerState.ImpactEnergy,
		0.0f,
		HitConfig.SingleInstanceMaxImpactEnergy
	);
	SingleHitMarkerState.AccentStrength = FMath::Clamp(SingleHitMarkerState.AccentStrength, 0.0f, 1.0f);
}
