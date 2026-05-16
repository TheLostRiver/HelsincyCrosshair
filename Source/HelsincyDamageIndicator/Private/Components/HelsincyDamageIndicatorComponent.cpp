// Copyright , Helsincy Games. All Rights Reserved.


#include "Components/HelsincyDamageIndicatorComponent.h"
#include "HelsincyDamageIndicator.h"
#include "Debug/HelsincyDamageIndicatorDebug.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "DataAssets/HelsincyDamageIndicatorDataAsset.h"
#include "SaveGame/HelsincyDamageIndicatorSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"


UHelsincyDamageIndicatorComponent::UHelsincyDamageIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 默认使用箭头样式 | Default to arrow style
	IndicatorProfile.IndicatorStyleTag = FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow;
}

void UHelsincyDamageIndicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
			TEXT("[DI][Comp] OwningPawn is NULL. Owner '%s' is not a Pawn. Deactivating component."),
			*GetNameSafe(GetOwner()));
		SetComponentTickEnabled(false);
		Deactivate();
		return;
	}

	// 快速路径: Controller 已就位 | Fast path: Controller already assigned
	if (OwningPawn->GetController())
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
			SetComponentTickEnabled(false);
			Deactivate();
		}
	}
	else
	{
		// Controller 未就位, 延迟到 Tick 判定 | Controller not assigned, defer to Tick
		bPendingOwnerCheck = true;
		PendingCheckFrameCount = 0;
		ScheduleLocalPlayerGuardRetry();
	}
}

void UHelsincyDamageIndicatorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearLocalPlayerGuardRetry();

	if (bAutoSaveOnEndPlay && bOwnerCheckPassed && OwningPawn)
	{
		SavePresetsToDisk(AutoSaveSlotName);
	}

	Super::EndPlay(EndPlayReason);
}

void UHelsincyDamageIndicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SCOPE_CYCLE_COUNTER(STAT_HDI_ComponentTick);

	if (TickPendingLocalPlayerGuard())
	{
		return;
	}

	UpdateIndicators(DeltaTime);
}

void UHelsincyDamageIndicatorComponent::RegisterDamageEvent(AActor* DamageCauser, FVector LocationIfNoActor)
{
	if (!IndicatorProfile.bEnabled) return;

	FVector TargetLoc = DamageCauser ? DamageCauser->GetActorLocation() : LocationIfNoActor;
	float StartAngle = CalculateTargetAngle(TargetLoc);

	// 检查是否已经存在来自该 Actor 的指示器？如果是，重置时间
	// Check if indicator from this Actor already exists; if so, reset its timer
	if (DamageCauser)
	{
		for (auto& Indicator : ActiveIndicators)
		{
			if (Indicator.bUseActor && Indicator.SourceActor.Get() == DamageCauser)
			{
				Indicator.TimeRemaining = IndicatorProfile.Duration;
				Indicator.InitialDuration = IndicatorProfile.Duration; // 同步更新，避免淡入计算异常 | Sync update to prevent fade-in calculation errors
				// 更新位置逻辑在 Tick 会自动处理
				// Position update logic is handled automatically in Tick
				return;
			}
		}
	}

	// 添加新的 | Add new indicator
	ActiveIndicators.Add(FHelsincy_ActiveDamageIndicator(
		DamageCauser,
		TargetLoc,
		IndicatorProfile.Duration,
		StartAngle
	));
}

#if !UE_BUILD_SHIPPING
void UHelsincyDamageIndicatorComponent::Debug_ForceEnableForValidation()
{
	ClearLocalPlayerGuardRetry();
	OwningPawn = Cast<APawn>(GetOwner());
	bPendingOwnerCheck = false;
	PendingCheckFrameCount = 0;
	bOwnerCheckPassed = OwningPawn != nullptr;

	IndicatorProfile.bEnabled = true;
	if (!IndicatorProfile.IndicatorStyleTag.IsValid())
	{
		IndicatorProfile.IndicatorStyleTag = FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow;
	}

	if (bOwnerCheckPassed)
	{
		PerformFullInitialization();
	}

	SetComponentTickEnabled(true);
	Activate(true);
}

void UHelsincyDamageIndicatorComponent::Debug_ClearActiveIndicators()
{
	ActiveIndicators.Empty();
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
void UHelsincyDamageIndicatorComponent::Debug_SetPendingLocalPlayerGuardForAutomation(const int32 InitialFrameCount)
{
	bPendingOwnerCheck = true;
	PendingCheckFrameCount = InitialFrameCount;
}

void UHelsincyDamageIndicatorComponent::Debug_TickPendingLocalPlayerGuardForAutomation()
{
	TickPendingLocalPlayerGuard();
}
#endif

void UHelsincyDamageIndicatorComponent::UpdateIndicators(const float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_HDI_UpdateIndicators);

	// --- 更新伤害指示器 | Update damage indicators ---
	if (ActiveIndicators.Num() > 0)
	{
		// 倒序遍历以便安全移除 | Iterate in reverse for safe removal
		for (int32 i = ActiveIndicators.Num() - 1; i >= 0; i--)
		{
			auto& Indicator = ActiveIndicators[i];

			// 1. 计时 | 1. Timer
			Indicator.TimeRemaining -= DeltaTime;
			if (Indicator.TimeRemaining <= 0.0f)
			{
				ActiveIndicators.RemoveAt(i);
				continue;
			}

			// 2. 获取目标当前位置 | 2. Get target current position
			FVector TargetPos = Indicator.SourceLocation;
			if (Indicator.bUseActor && Indicator.SourceActor.IsValid())
			{
				TargetPos = Indicator.SourceActor->GetActorLocation();
				Indicator.SourceLocation = TargetPos; // 缓存最后已知位置 | Cache last known position
			}

			// 3. 计算目标角度 | 3. Calculate target angle
			float TargetAngle = CalculateTargetAngle(TargetPos);

			// 4. 平滑插值 (使用 RInterpTo 处理 -180/180 环绕问题)
			// 4. Smooth interpolation (use RInterpTo to handle -180/180 wrapping)
			float InterpSpeed = IndicatorProfile.RotationInterpSpeed;

			// 因为 RInterpTo 需要 FRotator，我们构造临时的
			// RInterpTo requires FRotator, so we construct temporary ones
			FRotator CurrentRot(0, Indicator.CurrentSmoothAngle, 0);
			FRotator TargetRot(0, TargetAngle, 0);

			FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, InterpSpeed);
			Indicator.CurrentSmoothAngle = NewRot.Yaw;
		}
	}
}

void UHelsincyDamageIndicatorComponent::ScheduleLocalPlayerGuardRetry()
{
	if (UWorld* World = GetWorld())
	{
		if (!LocalPlayerGuardRetryTimerHandle.IsValid())
		{
			World->GetTimerManager().SetTimer(
				LocalPlayerGuardRetryTimerHandle,
				this,
				&UHelsincyDamageIndicatorComponent::RetryLocalPlayerGuard,
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

void UHelsincyDamageIndicatorComponent::ClearLocalPlayerGuardRetry()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LocalPlayerGuardRetryTimerHandle);
	}
	LocalPlayerGuardRetryTimerHandle.Invalidate();
}

void UHelsincyDamageIndicatorComponent::RetryLocalPlayerGuard()
{
	TickPendingLocalPlayerGuard();
}

bool UHelsincyDamageIndicatorComponent::RefreshLocalPlayerGuardForRendering()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return false;
	}

	OwningPawn = Pawn;

	if (!Pawn->GetController())
	{
		if (!bPendingOwnerCheck)
		{
			PendingCheckFrameCount = 0;
		}
		bPendingOwnerCheck = true;
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

bool UHelsincyDamageIndicatorComponent::TickPendingLocalPlayerGuard()
{
	// --- 延迟判定路径: 等待 Controller 就位 | Deferred check: waiting for Controller ---
	if (!bPendingOwnerCheck)
	{
		return false;
	}

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
			SetComponentTickEnabled(false);
			Deactivate();
		}
	}
	else
	{
		PendingCheckFrameCount++;
		if (!bLocalPlayerGuardTimeoutLogged && PendingCheckFrameCount >= MaxPendingCheckFrames)
		{
			UE_LOG(LogHelsincyDamageIndicator, Warning,
				TEXT("[DI][Comp] Controller not assigned after %d checks on '%s'. Keeping local-player guard pending for late possession."),
				MaxPendingCheckFrames, *GetNameSafe(GetOwner()));
			bLocalPlayerGuardTimeoutLogged = true;
		}
		ScheduleLocalPlayerGuardRetry();
	}

	return true; // 等待期间不执行正常 Tick | Skip normal tick while waiting
}

float UHelsincyDamageIndicatorComponent::CalculateTargetAngle(FVector TargetLocation)
{
	AActor* Owner = OwningPawn ? OwningPawn : GetOwner();
	if (!Owner)
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
			TEXT("[DI][Comp] CalculateTargetAngle: Owner is NULL, returning 0."));
		return 0.0f;
	}

	FVector PlayerLoc = Owner->GetActorLocation();
	FRotator PlayerRot = Owner->GetActorRotation();

	// 1. 计算世界方向向量 | 1. Calculate world direction vector
	FVector DirToEnemy = (TargetLocation - PlayerLoc).GetSafeNormal();

	// 2. 将世界向量转换为玩家的本地空间 (Unrotate)
	// 2. Convert world vector to player's local space (Unrotate)
	// 这样 X轴=前方, Y轴=右方 | So X-axis=forward, Y-axis=right
	FVector LocalDir = PlayerRot.UnrotateVector(DirToEnemy);

	// 3. 计算角度 (Atan2 返回弧度，需转度数)
	// 3. Calculate angle (Atan2 returns radians, convert to degrees)
	// UE坐标系: Y是右，X是前。Atan2(Y, X) 返回与X轴的夹角。
	// UE coordinate system: Y=right, X=forward. Atan2(Y, X) returns angle from X-axis.
	// 结果: 前=0, 右=90, 后=180/-180, 左=-90
	// Result: front=0, right=90, back=180/-180, left=-90
	float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Y, LocalDir.X));

	// 我们希望屏幕上方对应 0度
	// We want screen top to correspond to 0 degrees
	// 在 Canvas 数学中，通常我们需要把这个角度转换成屏幕时钟方向
	// In Canvas math, we typically need to convert this angle to screen clock direction
	// 这里我们直接返回相对于"正前方"的偏航
	// Here we directly return the yaw relative to "forward"
	return AngleDeg;
}

// ============ DataAsset API ============

void UHelsincyDamageIndicatorComponent::LoadFromDataAsset(UHelsincyDamageIndicatorDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Comp] LoadFromDataAsset: DataAsset is NULL."));
		return;
	}
	IndicatorProfile = DataAsset->Profile;
}

// ============ 预设系统 | Preset System ============

void UHelsincyDamageIndicatorComponent::SavePreset(FName PresetName)
{
	if (PresetName.IsNone()) return;
	PresetLibrary.Add(PresetName, IndicatorProfile);
	ActivePresetName = PresetName;
}

bool UHelsincyDamageIndicatorComponent::LoadPreset(FName PresetName)
{
	if (const auto* Found = PresetLibrary.Find(PresetName))
	{
		IndicatorProfile = *Found;
		ActivePresetName = PresetName;
		return true;
	}
	return false;
}

TArray<FName> UHelsincyDamageIndicatorComponent::GetAllPresetNames() const
{
	TArray<FName> Names;
	PresetLibrary.GetKeys(Names);
	return Names;
}

// ============ 持久化 API | Persistence API ============

bool UHelsincyDamageIndicatorComponent::SavePresetsToDisk(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("DamageIndicatorPresets")) : SlotName;

	UHelsincyDamageIndicatorSaveGame* SaveGameObj = NewObject<UHelsincyDamageIndicatorSaveGame>();
	SaveGameObj->SavedPresets = PresetLibrary;
	SaveGameObj->LastActivePresetName = ActivePresetName;
	SaveGameObj->SaveVersion = UHelsincyDamageIndicatorSaveGame::CurrentSaveVersion;

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObj, FinalSlot, UserIndex);

	UE_CLOG(!bSuccess, LogHelsincyDamageIndicator, Warning,
		TEXT("[DI][Persistence] Failed to save presets to slot '%s' (UserIndex=%d)."),
		*FinalSlot, UserIndex);

	UE_CLOG(bSuccess && HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
		TEXT("[DI][Persistence] Saved %d preset(s) to slot '%s'. ActivePreset='%s'."),
		PresetLibrary.Num(), *FinalSlot, *ActivePresetName.ToString());

	return bSuccess;
}

bool UHelsincyDamageIndicatorComponent::LoadPresetsFromDisk(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("DamageIndicatorPresets")) : SlotName;

	if (!UGameplayStatics::DoesSaveGameExist(FinalSlot, UserIndex))
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
			TEXT("[DI][Persistence] No save found in slot '%s' (UserIndex=%d)."),
			*FinalSlot, UserIndex);
		return false;
	}

	USaveGame* LoadedRaw = UGameplayStatics::LoadGameFromSlot(FinalSlot, UserIndex);
	UHelsincyDamageIndicatorSaveGame* SaveGameObj = Cast<UHelsincyDamageIndicatorSaveGame>(LoadedRaw);

	if (!SaveGameObj)
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Persistence] Failed to load or cast save from slot '%s'."),
			*FinalSlot);
		return false;
	}

	// 数据迁移 | Data migration
	if (SaveGameObj->SaveVersion < UHelsincyDamageIndicatorSaveGame::CurrentSaveVersion)
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

	UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
		TEXT("[DI][Persistence] Loaded %d preset(s) from slot '%s'. ActivePreset='%s'."),
		PresetLibrary.Num(), *FinalSlot, *ActivePresetName.ToString());

	return true;
}

bool UHelsincyDamageIndicatorComponent::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("DamageIndicatorPresets")) : SlotName;
	return UGameplayStatics::DoesSaveGameExist(FinalSlot, UserIndex);
}

bool UHelsincyDamageIndicatorComponent::DeleteSaveFromDisk(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? FString(TEXT("DamageIndicatorPresets")) : SlotName;
	return UGameplayStatics::DeleteGameInSlot(FinalSlot, UserIndex);
}

// ============ 本地玩家守卫 | Local Player Guard ============

bool UHelsincyDamageIndicatorComponent::ShouldActivateForOwner() const
{
	if (!OwningPawn) return false;

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC) return false; // 排除 AI (AAIController) + 无 Controller | Exclude AI + no Controller

	return PC->IsLocalController(); // 排除远端真人 + 模拟代理 | Exclude remote players + simulated proxies
}

void UHelsincyDamageIndicatorComponent::PerformFullInitialization()
{
	// DataAsset 初始化 | DataAsset initialization
	if (bUseDefaultDataAssetInit && DefaultDataAsset)
	{
		LoadFromDataAsset(DefaultDataAsset);
	}

	// 自动加载持久化 | Auto-load persistence
	if (bAutoLoadOnBeginPlay)
	{
		LoadPresetsFromDisk(AutoSaveSlotName);
	}

	// 如果忘记设置指示器样式就使用默认的箭头
	// If indicator style was not set, fall back to default arrow
	if (!IndicatorProfile.IndicatorStyleTag.IsValid())
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
			TEXT("[DI][Comp] IndicatorStyleTag is invalid on Owner '%s', falling back to Arrow."),
			*GetNameSafe(GetOwner()));
		IndicatorProfile.IndicatorStyleTag = FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow;
	}
}
