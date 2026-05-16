// Copyright , Helsincy Games. All Rights Reserved.

#include "Debug/HelsincyDamageIndicatorDebug.h"

#include "HelsincyDamageIndicator.h"
#include "Components/HelsincyDamageIndicatorComponent.h"
#include "DataAssets/HelsincyDamageIndicatorDataAsset.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Library/HelsincyDamageIndicatorRenderLibrary.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "UnrealClient.h"

#if !UE_BUILD_SHIPPING

static TAutoConsoleVariable<int32> CVarDIDebugEnable(
	TEXT("di.Debug.Enable"),
	0,
	TEXT("Master switch for HelsincyDamageIndicator debug output. 0=Off, 1=On"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarDIDebugText(
	TEXT("di.Debug.Text"),
	1,
	TEXT("Enable on-screen text debug (ShowDebug). Only works when di.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarDIDebugGeometry(
	TEXT("di.Debug.Geometry"),
	0,
	TEXT("Enable geometry visualization. Only works when di.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarDIDebugVerboseLog(
	TEXT("di.Debug.VerboseLog"),
	0,
	TEXT("Enable verbose log output. Only works when di.Debug.Enable=1"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarDITestDrawInViewport(
	TEXT("di.Test.DrawInViewport"),
	0,
	TEXT("Validation-only viewport debug draw bridge for HelsincyDamageIndicator. 0=Off, 1=On"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarDITestSourceDistance(
	TEXT("di.Test.SourceDistance"),
	1200.0f,
	TEXT("World-space source distance used by di.Test.Spawn validation events."),
	ECVF_Default);

static TWeakObjectPtr<APlayerController> DITestValidationPlayerController;
static FDelegateHandle DITestDebugDrawHandle;
static const TCHAR* DITestDamageIndicatorDataAssetPath = TEXT("/HelsincyCrosshair/DataAsset/DA_DamageIndicator.DA_DamageIndicator");
static const TCHAR* DITestArcTextureObjectPath = TEXT("/HelsincyCrosshair/ArcImage/T_HDI_Arc_Red_01.T_HDI_Arc_Red_01");

static void SetConsoleInt(const TCHAR* Name, int32 Value)
{
	if (IConsoleVariable* Var = IConsoleManager::Get().FindConsoleVariable(Name))
	{
		Var->Set(Value, ECVF_SetByConsole);
	}
}

static UTexture2D* LoadValidationArcTexture()
{
	// 中文：仅用于非 Shipping 验证入口；正式运行时仍由用户配置的 ArcConfig.ArcMaskTexture 决定。
	// English: Validation-only helper; runtime profiles still use the user-configured ArcConfig.ArcMaskTexture.
	UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, DITestArcTextureObjectPath);
	UE_CLOG(!Texture, LogHelsincyDamageIndicator, Warning,
		TEXT("[DI][Test] Arc validation texture not found at %s; falling back to generated arc mask."),
		DITestArcTextureObjectPath);
	return Texture;
}

static bool TryLoadValidationDataAssetProfile(FHelsincy_DamageIndicatorProfile& InOutProfile)
{
	// 中文：验证入口优先复用插件 Content 中已配置好的 DataAsset，避免截图验收偏离真实默认资源。
	// English: Prefer the configured plugin Content DataAsset so screenshot validation matches the real default asset.
	const UHelsincyDamageIndicatorDataAsset* DataAsset = LoadObject<UHelsincyDamageIndicatorDataAsset>(nullptr, DITestDamageIndicatorDataAssetPath);
	if (!DataAsset)
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Test] DamageIndicator validation DataAsset not found at %s; using component profile fallback."),
			DITestDamageIndicatorDataAssetPath);
		return false;
	}

	InOutProfile = DataAsset->Profile;
	return true;
}

static void DrawValidationDamageIndicators(UCanvas* Canvas, APlayerController* PlayerController)
{
	if (!Canvas || CVarDITestDrawInViewport.GetValueOnGameThread() == 0)
	{
		return;
	}

	APlayerController* ResolvedPC = PlayerController ? PlayerController : DITestValidationPlayerController.Get();
	if (!ResolvedPC)
	{
		return;
	}

	UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForController(ResolvedPC, Canvas);
}

static void EnsureValidationDebugDrawRegistered()
{
	if (DITestDebugDrawHandle.IsValid())
	{
		return;
	}

	DITestDebugDrawHandle = UDebugDrawService::Register(
		TEXT("Game"),
		FDebugDrawDelegate::CreateStatic(&DrawValidationDamageIndicators));
}

static void UnregisterValidationDebugDraw()
{
	if (DITestDebugDrawHandle.IsValid())
	{
		UDebugDrawService::Unregister(DITestDebugDrawHandle);
		DITestDebugDrawHandle.Reset();
	}
}

static APlayerController* ResolveValidationPlayerController(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->IsLocalPlayerController())
		{
			return PC;
		}
	}

	return World->GetFirstPlayerController();
}

static UHelsincyDamageIndicatorComponent* FindOrCreateValidationComponent(APawn* Pawn)
{
	if (!Pawn)
	{
		return nullptr;
	}

	if (UHelsincyDamageIndicatorComponent* Existing = Pawn->FindComponentByClass<UHelsincyDamageIndicatorComponent>())
	{
		return Existing;
	}

	const FName ComponentName = MakeUniqueObjectName(
		Pawn,
		UHelsincyDamageIndicatorComponent::StaticClass(),
		TEXT("DI_ValidationComponent"));

	UHelsincyDamageIndicatorComponent* Created = NewObject<UHelsincyDamageIndicatorComponent>(Pawn, ComponentName);
	if (!Created)
	{
		return nullptr;
	}

	Created->CreationMethod = EComponentCreationMethod::Instance;
	Pawn->AddInstanceComponent(Created);
	Created->RegisterComponent();
	return Created;
}

static bool ParsePlacementMode(const TArray<FString>& Args, EHelsincyDamageIndicatorPlacementMode& OutMode)
{
	if (Args.Num() == 0 || Args[0].Equals(TEXT("WindowEdge"), ESearchCase::IgnoreCase))
	{
		OutMode = EHelsincyDamageIndicatorPlacementMode::WindowEdge;
		return true;
	}

	if (Args[0].Equals(TEXT("RadialCircle"), ESearchCase::IgnoreCase)
		|| Args[0].Equals(TEXT("Radial"), ESearchCase::IgnoreCase))
	{
		OutMode = EHelsincyDamageIndicatorPlacementMode::RadialCircle;
		return true;
	}

	return false;
}

static int32 ParseDirectionCount(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		return 8;
	}

	return FCString::Atoi(*Args[1]) <= 4 ? 4 : 8;
}

static FGameplayTag ParseStyleTag(const TArray<FString>& Args)
{
	if (Args.Num() >= 3 && Args[2].Equals(TEXT("Arc"), ESearchCase::IgnoreCase))
	{
		return FHelsincyDamageIndicator_Tags::Indicator_Style_Arc;
	}

	if (Args.Num() >= 3 && Args[2].Equals(TEXT("Image"), ESearchCase::IgnoreCase))
	{
		return FHelsincyDamageIndicator_Tags::Indicator_Style_Image;
	}

	return FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow;
}

static bool ShouldRequestValidationScreenshot(const TArray<FString>& Args)
{
	for (const FString& Arg : Args)
	{
		if (Arg.Equals(TEXT("Screenshot"), ESearchCase::IgnoreCase)
			|| Arg.Equals(TEXT("Shot"), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

static void RequestValidationScreenshot(
	EHelsincyDamageIndicatorPlacementMode PlacementMode,
	FGameplayTag StyleTag,
	int32 DirectionCount)
{
	const FString ScreenshotDir = FPaths::ProjectSavedDir() / TEXT("Screenshots/DamageIndicatorValidation");
	IFileManager::Get().MakeDirectory(*ScreenshotDir, true);

	const FString PlacementName = PlacementMode == EHelsincyDamageIndicatorPlacementMode::WindowEdge
		? TEXT("WindowEdge")
		: TEXT("RadialCircle");
	const FString StyleName = StyleTag == FHelsincyDamageIndicator_Tags::Indicator_Style_Arc
		? TEXT("Arc")
		: StyleTag == FHelsincyDamageIndicator_Tags::Indicator_Style_Image
			? TEXT("Image")
			: TEXT("Arrow");
	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString Filename = FString::Printf(
		TEXT("%s/DI_%s_%d_%s_%s.png"),
		*ScreenshotDir,
		*PlacementName,
		DirectionCount,
		*StyleName,
		*Timestamp);

	FScreenshotRequest::RequestScreenshot(Filename, true, false);
	UE_LOG(LogHelsincyDamageIndicator, Log, TEXT("[DI][Test] Requested validation screenshot: %s"), *Filename);
}

static void ApplyValidationProfile(
	UHelsincyDamageIndicatorComponent* Component,
	EHelsincyDamageIndicatorPlacementMode PlacementMode,
	FGameplayTag StyleTag)
{
	if (!Component)
	{
		return;
	}

	FHelsincy_DamageIndicatorProfile Profile = Component->GetIndicatorProfile();
	TryLoadValidationDataAssetProfile(Profile);
	Profile.bEnabled = true;
	Profile.IndicatorStyleTag = StyleTag;
	Profile.Duration = 30.0f;
	Profile.FadeInTime = FMath::Min(Profile.FadeInTime, 0.1f);
	Profile.FadeOutTime = FMath::Min(Profile.FadeOutTime, 0.35f);
	Profile.PointerMaxOpacity = 1.0f;
	Profile.PlacementMode = PlacementMode;
	Profile.bShowCircle = true;
	Profile.bHideCircleInWindowEdgeMode = true;
	Profile.EdgeMargin = FMath::Max(Profile.EdgeMargin, 48.0f);
	Profile.EdgeCornerPadding = FMath::Max(Profile.EdgeCornerPadding, 24.0f);
	Profile.ArrowConfig.Size = FMath::Max(Profile.ArrowConfig.Size, 48.0f);
	Profile.ArrowConfig.Color = FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
	if (StyleTag == FHelsincyDamageIndicator_Tags::Indicator_Style_Image)
	{
		if (!Profile.ImageConfig.Texture)
		{
			Profile.ImageConfig.Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/AICON-Green.AICON-Green"));
		}
		Profile.ImageConfig.Size = FVector2D(64.0f, 64.0f);
		Profile.ImageConfig.Tint = FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
		Profile.ImageConfig.bRotateImage = true;
	}
	if (StyleTag == FHelsincyDamageIndicator_Tags::Indicator_Style_Arc)
	{
		// 中文：DataAsset 若已配置 Arc 贴图与尺寸则优先保留；缺失时才补验证默认值。
		// English: Preserve DataAsset arc texture/size when configured; only fill validation defaults when missing.
		if (!Profile.ArcConfig.ArcMaskTexture)
		{
			Profile.ArcConfig.ArcMaskTexture = LoadValidationArcTexture();
		}
		if (Profile.ArcConfig.Size.X < 32.0f || Profile.ArcConfig.Size.Y < 32.0f)
		{
			// 中文：当前 1024x512 贴图带有透明安全边距，验证尺寸稍放大以保证截图中弧光可读。
			// English: The current 1024x512 texture contains transparent padding, so validation uses a larger draw size for readability.
			Profile.ArcConfig.Size = FVector2D(420.0f, 210.0f);
		}
		if (Profile.ArcConfig.Color.A <= 0.0f)
		{
			Profile.ArcConfig.Color = FLinearColor(1.0f, 0.04f, 0.08f, 1.0f);
		}
		Profile.ArcConfig.DirectionCueMode = EHelsincyDamageIndicatorArcDirectionCueMode::CenterNib;
		Profile.ArcConfig.DirectionCueStrength = 1.0f;
		if (Profile.ArcConfig.CueSize.X < 4.0f || Profile.ArcConfig.CueSize.Y < 4.0f)
		{
			Profile.ArcConfig.CueSize = FVector2D(22.0f, 16.0f);
		}
	}
	Profile.CircleColor = FLinearColor(1.0f, 0.85f, 0.0f, 0.25f);
	Component->SetIndicatorProfile(Profile);
}

static void SpawnValidationDirections(UHelsincyDamageIndicatorComponent* Component, APawn* Pawn, int32 DirectionCount)
{
	if (!Component || !Pawn)
	{
		return;
	}

	static const float FourWayAngles[] = { 0.0f, 90.0f, 180.0f, -90.0f };
	static const float EightWayAngles[] = { 0.0f, 45.0f, 90.0f, 135.0f, 180.0f, -135.0f, -90.0f, -45.0f };

	const float* Angles = DirectionCount <= 4 ? FourWayAngles : EightWayAngles;
	const int32 AngleCount = DirectionCount <= 4 ? UE_ARRAY_COUNT(FourWayAngles) : UE_ARRAY_COUNT(EightWayAngles);
	const float SourceDistance = FMath::Max(100.0f, CVarDITestSourceDistance.GetValueOnGameThread());
	const FVector PawnLocation = Pawn->GetActorLocation();
	const FRotator PawnRotation = Pawn->GetActorRotation();

	for (int32 Index = 0; Index < AngleCount; ++Index)
	{
		const float AngleRad = FMath::DegreesToRadians(Angles[Index]);
		const FVector LocalDirection(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
		const FVector WorldDirection = PawnRotation.RotateVector(LocalDirection);
		Component->RegisterDamageEvent(nullptr, PawnLocation + WorldDirection * SourceDistance);
	}
}

static void HandleDITestSpawn(const TArray<FString>& Args, UWorld* World)
{
	EHelsincyDamageIndicatorPlacementMode PlacementMode = EHelsincyDamageIndicatorPlacementMode::WindowEdge;
	if (!ParsePlacementMode(Args, PlacementMode))
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Test] Unknown placement mode '%s'. Usage: di.Test.Spawn [WindowEdge|RadialCircle] [4|8] [Arrow|Image|Arc]"),
			Args.Num() > 0 ? *Args[0] : TEXT("<None>"));
		return;
	}

	APlayerController* PC = ResolveValidationPlayerController(World);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Test] Cannot spawn validation indicators: local Pawn not found."));
		return;
	}

	DITestValidationPlayerController = PC;
	UHelsincyDamageIndicatorComponent* Component = FindOrCreateValidationComponent(Pawn);
	if (!Component)
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning,
			TEXT("[DI][Test] Cannot spawn validation indicators: failed to create component on Pawn '%s'."),
			*GetNameSafe(Pawn));
		return;
	}

	Component->Debug_ForceEnableForValidation();
	const FGameplayTag StyleTag = ParseStyleTag(Args);
	const int32 DirectionCount = ParseDirectionCount(Args);
	ApplyValidationProfile(Component, PlacementMode, StyleTag);
	Component->Debug_ClearActiveIndicators();
	SpawnValidationDirections(Component, Pawn, DirectionCount);

	SetConsoleInt(TEXT("di.Test.DrawInViewport"), 1);
	SetConsoleInt(TEXT("di.Debug.Enable"), 1);
	SetConsoleInt(TEXT("di.Debug.Text"), 1);
	EnsureValidationDebugDrawRegistered();

	UE_LOG(LogHelsincyDamageIndicator, Log,
		TEXT("[DI][Test] Spawned %d validation indicators on Pawn '%s' with mode=%s style=%s."),
		DirectionCount,
		*GetNameSafe(Pawn),
		PlacementMode == EHelsincyDamageIndicatorPlacementMode::WindowEdge ? TEXT("WindowEdge") : TEXT("RadialCircle"),
		*StyleTag.ToString());

	if (ShouldRequestValidationScreenshot(Args))
	{
		RequestValidationScreenshot(PlacementMode, StyleTag, DirectionCount);
	}
}

static void HandleDITestClear(const TArray<FString>& Args, UWorld* World)
{
	(void)Args;

	APlayerController* PC = ResolveValidationPlayerController(World);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	UHelsincyDamageIndicatorComponent* Component = Pawn ? Pawn->FindComponentByClass<UHelsincyDamageIndicatorComponent>() : nullptr;
	if (Component)
	{
		Component->Debug_ClearActiveIndicators();
	}

	SetConsoleInt(TEXT("di.Test.DrawInViewport"), 0);
	DITestValidationPlayerController.Reset();
	UnregisterValidationDebugDraw();
	UE_LOG(LogHelsincyDamageIndicator, Log, TEXT("[DI][Test] Cleared validation indicators and disabled viewport draw bridge."));
}

static FAutoConsoleCommandWithWorldAndArgs CommandDITestSpawn(
	TEXT("di.Test.Spawn"),
	TEXT("Spawn validation damage indicators. Usage: di.Test.Spawn [WindowEdge|RadialCircle] [4|8] [Arrow|Image|Arc] [Screenshot]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&HandleDITestSpawn));

static FAutoConsoleCommandWithWorldAndArgs CommandDITestClear(
	TEXT("di.Test.Clear"),
	TEXT("Clear validation damage indicators and disable di.Test.DrawInViewport."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&HandleDITestClear));

bool HelsincyDamageIndicatorDebug::IsEnabled()
{
	return CVarDIDebugEnable.GetValueOnGameThread() != 0;
}

bool HelsincyDamageIndicatorDebug::IsTextEnabled()
{
	return IsEnabled() && CVarDIDebugText.GetValueOnGameThread() != 0;
}

bool HelsincyDamageIndicatorDebug::IsGeometryEnabled()
{
	return IsEnabled() && CVarDIDebugGeometry.GetValueOnGameThread() != 0;
}

bool HelsincyDamageIndicatorDebug::IsVerboseLogEnabled()
{
	return IsEnabled() && CVarDIDebugVerboseLog.GetValueOnGameThread() != 0;
}

#endif // !UE_BUILD_SHIPPING
