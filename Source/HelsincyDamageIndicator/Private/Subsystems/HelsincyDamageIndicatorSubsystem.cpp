// Copyright , Helsincy Games. All Rights Reserved.


#include "Subsystems/HelsincyDamageIndicatorSubsystem.h"
#include "HelsincyDamageIndicator.h"
#include "Debug/HelsincyDamageIndicatorDebug.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "IndicatorRenderer/HelsincyIndicatorRenderer.h"
#include "IndicatorRenderer/HelsincyIndicatorRendererArc.h"
#include "IndicatorRenderer/HelsincyIndicatorRendererArrow.h"
#include "IndicatorRenderer/HelsincyIndicatorRendererImage.h"
#include "Engine/AssetManager.h"
#include "Settings/HelsincyDamageIndicatorSettings.h"


void UHelsincyDamageIndicatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 注册内置指示器渲染器 | Register built-in indicator renderers
	RegisterDamageIndicatorRenderer(FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow, UHelsincyIndicatorRendererArrow::StaticClass());
	RegisterDamageIndicatorRenderer(FHelsincyDamageIndicator_Tags::Indicator_Style_Image, UHelsincyIndicatorRendererImage::StaticClass());
	RegisterDamageIndicatorRenderer(FHelsincyDamageIndicator_Tags::Indicator_Style_Arc, UHelsincyIndicatorRendererArc::StaticClass());

	// 异步加载蓝图类 | Async load blueprint classes
	StartIndicatorRenderAsyncLoadBlueprintClasses();

	// 生成抗锯齿纹理 | Generate anti-aliased texture
	GenerateSmoothTexture();
}

void UHelsincyDamageIndicatorSubsystem::Deinitialize()
{
	Super::Deinitialize();

	IndicatorRenderers.Empty();
	if (SmoothLineTexture)
	{
		SmoothLineTexture->RemoveFromRoot();
		SmoothLineTexture = nullptr;
	}

	if (IndicatorAsyncLoadHandle.IsValid())
	{
		if (IndicatorAsyncLoadHandle->IsLoadingInProgress())
		{
			IndicatorAsyncLoadHandle->CancelHandle();
		}
	}
	IndicatorAsyncLoadHandle.Reset();
}

UHelsincyIndicatorRenderer* UHelsincyDamageIndicatorSubsystem::GetIndicatorRenderer(FGameplayTag StyleTag)
{
	if (UHelsincyIndicatorRenderer** Found = IndicatorRenderers.Find(StyleTag))
	{
		return *Found;
	}
	return nullptr;
}

TArray<FGameplayTag> UHelsincyDamageIndicatorSubsystem::GetRegisteredTags() const
{
	TArray<FGameplayTag> Tags;
	IndicatorRenderers.GetKeys(Tags);
	return Tags;
}

bool UHelsincyDamageIndicatorSubsystem::RegisterDamageIndicatorRenderer(FGameplayTag Tag, TSubclassOf<UHelsincyIndicatorRenderer> RendererClass)
{
	if (!Tag.IsValid()) return false;

	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style"), false);
	if (Tag == RootTag) return false;
	if (RootTag.IsValid() && !Tag.MatchesTag(RootTag))
	{
		UE_LOG(LogHelsincyDamageIndicator, Warning, TEXT("[DI][Sub] Failed to register Renderer for Tag %s: not a child of Indicator.Style!"), *Tag.ToString());
		return false;
	}

	if (IndicatorRenderers.Contains(Tag))
	{
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
			TEXT("[DI][Sub] Skipping duplicate registration for Tag '%s'. Existing renderer preserved."),
			*Tag.ToString());
		return false;
	}

	if (*RendererClass)
	{
		UHelsincyIndicatorRenderer* RendererInstance = NewObject<UHelsincyIndicatorRenderer>(this, RendererClass);
		IndicatorRenderers.Add(Tag, RendererInstance);
		UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Log,
			TEXT("[DI][Sub] Registered indicator renderer '%s' for Tag '%s'."),
			*RendererClass->GetName(), *Tag.ToString());
		return true;
	}

	UE_CLOG(HelsincyDamageIndicatorDebug::IsVerboseLogEnabled(), LogHelsincyDamageIndicator, Warning,
		TEXT("[DI][Sub] RendererClass is invalid for Tag '%s'."),
		*Tag.ToString());
	return false;
}

void UHelsincyDamageIndicatorSubsystem::GenerateSmoothTexture()
{
	// NOTE: 此函数与 HelsincyCrosshairManagerSubsystem::GenerateSmoothTexture() 算法对称。
	// NOTE: This function mirrors HelsincyCrosshairManagerSubsystem::GenerateSmoothTexture().
	// 两个模块设计上独立，共享工具函数会引入耦合，因此保留重复。
	// The two modules are designed independently; sharing utility functions would introduce coupling, so duplication is kept.
	// 若修改纹理生成算法，请同步更新两处。
	// If you modify the texture generation algorithm, please update both locations.

	constexpr int32 Width = 32;
	constexpr int32 Height = 32;

	SmoothLineTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);

	if (!SmoothLineTexture)
	{
		UE_LOG(LogHelsincyDamageIndicator, Error, TEXT("[DI][Sub] SmoothTexture creation failed: CreateTransient returned null."));
		return;
	}

	SmoothLineTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	SmoothLineTexture->SRGB = 0;
	SmoothLineTexture->Filter = TextureFilter::TF_Bilinear;
	SmoothLineTexture->AddToRoot();

	if (!SmoothLineTexture->PlatformData || SmoothLineTexture->PlatformData->Mips.Num() == 0)
	{
		UE_LOG(LogHelsincyDamageIndicator, Error, TEXT("[DI][Sub] SmoothTexture creation failed: PlatformData or Mips not available."));
		SmoothLineTexture->RemoveFromRoot();
		SmoothLineTexture = nullptr;
		return;
	}

	FTexture2DMipMap& Mip = SmoothLineTexture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!Data)
	{
		UE_LOG(LogHelsincyDamageIndicator, Error, TEXT("[DI][Sub] SmoothTexture creation failed: Mip BulkData lock returned null."));
		SmoothLineTexture->RemoveFromRoot();
		SmoothLineTexture = nullptr;
		return;
	}
	FColor* FormattedData = static_cast<FColor*>(Data);

	float CenterY = (Height - 1) * 0.5f;
	float MaxDistY = Height * 0.5f;

	for (int32 Y = 0; Y < Height; Y++)
	{
		float DistY = FMath::Abs(static_cast<float>(Y) - CenterY);
		float NormalizedDist = FMath::Clamp(DistY / MaxDistY, 0.0f, 1.0f);
		float AlphaRatio = 1.0f - NormalizedDist;
		AlphaRatio = AlphaRatio * AlphaRatio * (3.0f - 2.0f * AlphaRatio);
		AlphaRatio = FMath::Pow(AlphaRatio, 0.8f);

		uint8 AlphaVal = static_cast<uint8>(AlphaRatio * 255.0f);

		for (int32 X = 0; X < Width; X++)
		{
			FormattedData[Y * Width + X] = FColor(255, 255, 255, AlphaVal);
		}
	}

	Mip.BulkData.Unlock();
	SmoothLineTexture->UpdateResource();
}

void UHelsincyDamageIndicatorSubsystem::StartIndicatorRenderAsyncLoadBlueprintClasses()
{
	const UHelsincyDamageIndicatorSettings* Settings = GetDefault<UHelsincyDamageIndicatorSettings>();
	if (!Settings || Settings->BlueprintIndicatorRenderers.Num() == 0) return;

	TArray<FSoftObjectPath> AssetsToLoad;
	for (const auto& SoftPtr : Settings->BlueprintIndicatorRenderers)
	{
		AssetsToLoad.Add(SoftPtr.ToSoftObjectPath());
	}

	if (AssetsToLoad.Num() == 0) return;

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

	IndicatorAsyncLoadHandle = Streamable.RequestAsyncLoad(
		AssetsToLoad,
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnIndicatorRenderBlueprintsLoaded)
	);
}

void UHelsincyDamageIndicatorSubsystem::OnIndicatorRenderBlueprintsLoaded()
{
	const UHelsincyDamageIndicatorSettings* Settings = GetDefault<UHelsincyDamageIndicatorSettings>();
	if (!Settings) return;

	for (const auto& SoftPtr : Settings->BlueprintIndicatorRenderers)
	{
		if (UClass* LoadedClass = SoftPtr.Get())
		{
			if (auto* CDO = Cast<UHelsincyIndicatorRenderer>(LoadedClass->GetDefaultObject()))
			{
				RegisterDamageIndicatorRenderer(CDO->AssociatedTag, LoadedClass);
			}
			else
			{
				UE_LOG(LogHelsincyDamageIndicator, Warning,
					TEXT("[DI][Sub] CDO Cast failed for loaded class '%s'. Is it derived from UHelsincyIndicatorRenderer?"),
					*GetNameSafe(LoadedClass));
			}
		}
		else
		{
			UE_LOG(LogHelsincyDamageIndicator, Warning,
				TEXT("[DI][Sub] Failed to load Blueprint IndicatorRenderer: '%s'. Asset may be missing or corrupted."),
				*SoftPtr.ToSoftObjectPath().ToString());
		}
	}

	IndicatorAsyncLoadHandle.Reset();

	UE_LOG(LogHelsincyDamageIndicator, Log, TEXT("[DI][Sub] Async loading of Indicator Blueprint Renderers complete. %d blueprint classes processed."), Settings->BlueprintIndicatorRenderers.Num());
}
