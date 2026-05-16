// Copyright 2024-2025, Helsincy Games. All Rights Reserved.


#include "Subsystems/HelsincyCrosshairManagerSubsystem.h"
#include "HelsincyCrosshair.h"
#include "Debug/HelsincyCrosshairDebug.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "Render/HelsincyRendererCircle.h"
#include "Render/HelsincyRendererCross.h"
#include "Render/HelsincyRendererTStyle.h"
#include "Render/HelsincyRendererTriangle.h"
#include "Render/HelsincyRendererRectangle.h"
#include "Render/HelsincyRendererChevron.h"
#include "Render/HelsincyRendererPolygon.h"
#include "Render/HelsincyRendererWings.h"
#include "Engine/AssetManager.h"
#include "Render/HelsincyRendererImage.h"
#include "Render/HelsincyRendererDotOnly.h"
#include "Settings/HelsincyCrosshairSettings.h"


void UHelsincyCrosshairManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// 注册内置渲染器 C++采用手动注册
	// Register built-in renderers, C++ uses manual registration
	// 这种方式把 Tag 和 类 的绑定解耦了
	// This approach decouples the binding between Tags and Classes
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Cross,  UHelsincyRendererCross::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Circle, UHelsincyRendererCircle::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_TStyle, UHelsincyRendererTStyle::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Triangle, UHelsincyRendererTriangle::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Rectangle, UHelsincyRendererRectangle::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Chevron, UHelsincyRendererChevron::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Polygon, UHelsincyRendererPolygon::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Wings, UHelsincyRendererWings::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_Image, UHelsincyRendererImage::StaticClass());
	RegisterRenderer(FHelsincyCrosshair_Tags::Shape_DotOnly, UHelsincyRendererDotOnly::StaticClass());

	// 异步加载蓝图类  蓝图使用自动注册
	// Async load blueprint classes; blueprints use auto-registration
	StartCrosshairRenderAsyncLoadBlueprintClasses();

	// 生成抗锯齿纹理 | Generate anti-aliased texture
	GenerateSmoothTexture();
}

void UHelsincyCrosshairManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	RendererMap.Empty();
	if (SmoothLineTexture)
	{
		SmoothLineTexture->RemoveFromRoot();
		SmoothLineTexture = nullptr;
	}
	
	if (AsyncLoadHandle.IsValid())
	{
		if (AsyncLoadHandle->IsLoadingInProgress())
		{
			AsyncLoadHandle->CancelHandle();
		}
	}
	AsyncLoadHandle.Reset();
}

UHelsincyShapeRenderer* UHelsincyCrosshairManagerSubsystem::GetRenderer(FGameplayTag ShapeTag)
{
	// TMap::Find 是极快的哈希查找 | TMap::Find is extremely fast hash lookup
	if (UHelsincyShapeRenderer** Found = RendererMap.Find(ShapeTag))
	{
		return *Found;
	}
	return nullptr;
}

TArray<FGameplayTag> UHelsincyCrosshairManagerSubsystem::GetRegisteredTags() const
{
	TArray<FGameplayTag> Tags;
	RendererMap.GetKeys(Tags);
	return Tags;
}

bool UHelsincyCrosshairManagerSubsystem::RegisterRenderer(FGameplayTag Tag, TSubclassOf<UHelsincyShapeRenderer> RendererClass)
{
	// [安全检查 1] Tag 是否有效 | [Safety Check 1] Is Tag valid
	if (!Tag.IsValid()) return false;

	// [安全检查 2] Tag 是否属于 Crosshair.Shape 层级
	// [Safety Check 2] Does Tag belong to Crosshair.Shape hierarchy
	// RequestGameplayTag 确保我们拿到的是系统里的那个根节点
	// RequestGameplayTag ensures we get the root node from the system
	// 如果 Tag 是 "Ability.Fire"，这里 MatchesTag 就会返回 false
	// If Tag is "Ability.Fire", MatchesTag will return false here
	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape"), false); // false = 不报错，只查找 | false = don't error, just lookup
	if (Tag == RootTag) return false;
	if (RootTag.IsValid() && !Tag.MatchesTag(RootTag)) 
	{
		UE_LOG(LogHelsincyCrosshair, Warning, TEXT("[HC][Sub] Failed to register Renderer for Tag %s: not a child of Crosshair.Shape!"), *Tag.ToString());
		return false;
	}

	// 检查重复 | Check for duplicates
	if (RendererMap.Contains(Tag))
	{
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
			TEXT("[HC][Sub] Skipping duplicate registration for Tag '%s'. Existing renderer preserved."),
			*Tag.ToString());
		return false; 
	}

	if (*RendererClass)
	{
		UHelsincyShapeRenderer* RendererInstance = NewObject<UHelsincyShapeRenderer>(this, RendererClass);
		RendererMap.Add(Tag, RendererInstance);
		UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Log,
			TEXT("[HC][Sub] Registered renderer '%s' for Tag '%s'."),
			*RendererClass->GetName(), *Tag.ToString());
		return true;
	}

	UE_CLOG(HelsincyCrosshairDebug::IsVerboseLogEnabled(), LogHelsincyCrosshair, Warning,
		TEXT("[HC][Sub] RendererClass is invalid for Tag '%s'."),
		*Tag.ToString());
	return false;
}

void UHelsincyCrosshairManagerSubsystem::GenerateSmoothTexture()
{
	// NOTE: 此函数与 HelsincyDamageIndicatorSubsystem::GenerateSmoothTexture() 算法对称。
	// NOTE: This function mirrors HelsincyDamageIndicatorSubsystem::GenerateSmoothTexture().
	// 两个模块设计上独立，共享工具函数会引入耆合，因此保留重复。
	// The two modules are designed independently; sharing utility functions would introduce coupling, so duplication is kept.
	// 若修改纹理生成算法，请同步更新两处。
	// If you modify the texture generation algorithm, please update both locations.

	// 创建一个 32x32 的临时纹理 | Create a 32x32 transient texture
	constexpr int32 Width = 32;
	constexpr int32 Height = 32;
    
	// 创建临时纹理 | Create transient texture
	SmoothLineTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    
	if (!SmoothLineTexture)
	{
		UE_LOG(LogHelsincyCrosshair, Error, TEXT("[HC][Sub] SmoothTexture creation failed: CreateTransient returned null."));
		return;
	}

	SmoothLineTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	SmoothLineTexture->SRGB = 0;
	SmoothLineTexture->Filter = TextureFilter::TF_Bilinear; // 确保双线性过滤开启 | Ensure bilinear filtering is enabled
	SmoothLineTexture->AddToRoot();

	if (!SmoothLineTexture->PlatformData || SmoothLineTexture->PlatformData->Mips.Num() == 0)
	{
		UE_LOG(LogHelsincyCrosshair, Error, TEXT("[HC][Sub] SmoothTexture creation failed: PlatformData or Mips not available."));
		SmoothLineTexture->RemoveFromRoot();
		SmoothLineTexture = nullptr;
		return;
	}

	{
		FTexture2DMipMap& Mip = SmoothLineTexture->PlatformData->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		if (!Data)
		{
			UE_LOG(LogHelsincyCrosshair, Error, TEXT("[HC][Sub] SmoothTexture creation failed: Mip BulkData lock returned null."));
			Mip.BulkData.Unlock();
			SmoothLineTexture->RemoveFromRoot();
			SmoothLineTexture = nullptr;
			return;
		}
		FColor* FormattedData = static_cast<FColor*>(Data);

		// [核心修改] 我们不再计算到圆心的距离，而是只计算到"中心线"的垂直距离
		// [Core] Instead of calculating distance to circle center, only calculate vertical distance to center line
		float CenterY = (Height - 1) * 0.5f;
		float MaxDistY = Height * 0.5f;

		for (int32 Y = 0; Y < Height; Y++)
		{
			// 计算当前行距离中心线的垂直距离 (0.0 ~ 1.0)
			// Calculate vertical distance of current row from center line (0.0 ~ 1.0)
			float DistY = FMath::Abs(static_cast<float>(Y) - CenterY);
            
			// 归一化距离: 中心=0.0, 边缘=1.0
			// Normalized distance: center=0.0, edge=1.0
			float NormalizedDist = FMath::Clamp(DistY / MaxDistY, 0.0f, 1.0f);

			// 反转: 中心=1.0 (不透明), 边缘=0.0 (透明)
			// Invert: center=1.0 (opaque), edge=0.0 (transparent)
			float AlphaRatio = 1.0f - NormalizedDist;

			// 使用 SmoothStep 让边缘羽化更柔和，而不是生硬的线性渐变
			// Use SmoothStep for softer edge feathering instead of harsh linear gradient
			// 公式 | Formula: x * x * (3 - 2 * x)
			AlphaRatio = AlphaRatio * AlphaRatio * (3.0f - 2.0f * AlphaRatio);
            
			// [微调] 稍微增强一点中心强度，防止线条看起来太细
			// [Fine-tune] Slightly boost center intensity to prevent lines from looking too thin
			AlphaRatio = FMath::Pow(AlphaRatio, 0.8f);

			uint8 AlphaVal = static_cast<uint8>(AlphaRatio * 255.0f);

			for (int32 X = 0; X < Width; X++)
			{
				// [关键点] | [Key Point]
				// X轴 (长度方向) 保持颜色一致，不要渐变！
				// X-axis (length direction) keeps color uniform, no gradient!
				// 只有 Y轴 (宽度方向) 有 Alpha 渐变
				// Only Y-axis (width direction) has Alpha gradient
                
				// 为了避免线段连接处有极微小的硬切，我们只在最两端(0和Width-1)做极其微小的淡出(可选)，
				// To avoid tiny hard cuts at segment joins, optionally do minimal fade at ends (0 and Width-1),
				// 但通常完全实心连接效果最好。这里我们保持 X 轴全实心。
				// but fully solid connections usually look best. We keep X-axis fully solid here.
                
				FormattedData[Y * Width + X] = FColor(255, 255, 255, AlphaVal);
			}
		}

		Mip.BulkData.Unlock();
		SmoothLineTexture->UpdateResource();
	}
}

void UHelsincyCrosshairManagerSubsystem::StartCrosshairRenderAsyncLoadBlueprintClasses()
{
	const UHelsincyCrosshairSettings* Settings = GetDefault<UHelsincyCrosshairSettings>();
	if (!Settings || Settings->BlueprintRenderers.Num() == 0) return;

	// 收集所有需要加载的软引用路径
	// Collect all soft reference paths that need loading
	TArray<FSoftObjectPath> AssetsToLoad;
	for (const auto& SoftPtr : Settings->BlueprintRenderers)
	{
		AssetsToLoad.Add(SoftPtr.ToSoftObjectPath());
	}

	if (AssetsToLoad.Num() == 0) return;

	// 获取全局流式加载管理器进行异步加载
	// Get global streaming loader manager for async loading
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    
	// 发起请求，绑定回调 | Initiate request and bind callback
	AsyncLoadHandle = Streamable.RequestAsyncLoad(
		AssetsToLoad, 
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnCrosshairRenderBlueprintsLoaded)
	);

}

void UHelsincyCrosshairManagerSubsystem::OnCrosshairRenderBlueprintsLoaded()
{
	// 回调执行时，所有蓝图类都已经加载进内存，变成了 UClass
	// When callback executes, all blueprint classes are loaded into memory as UClass
	const UHelsincyCrosshairSettings* Settings = GetDefault<UHelsincyCrosshairSettings>();
	if (!Settings) return;

	for (const auto& SoftPtr : Settings->BlueprintRenderers)
	{
		// 此时 Get() 不会阻塞，因为已经 Load 完了
		// Get() won't block at this point since loading is already complete
		if (UClass* LoadedClass = SoftPtr.Get())
		{
			if (auto* CDO = Cast<UHelsincyShapeRenderer>(LoadedClass->GetDefaultObject()))
			{
				RegisterRenderer(CDO->AssociatedTag, LoadedClass);
			}
			else
			{
				UE_LOG(LogHelsincyCrosshair, Warning,
					TEXT("[HC][Sub] CDO Cast failed for loaded class '%s'. Is it derived from UHelsincyShapeRenderer?"),
					*GetNameSafe(LoadedClass));
			}
		}
		else
		{
			UE_LOG(LogHelsincyCrosshair, Warning,
				TEXT("[HC][Sub] Failed to load Blueprint Renderer: '%s'. Asset may be missing or corrupted."),
				*SoftPtr.ToSoftObjectPath().ToString());
		}
	}
    
	// 释放句柄 | Release handle
	AsyncLoadHandle.Reset();
    
	UE_LOG(LogHelsincyCrosshair, Log, TEXT("[HC][Sub] Async loading of Blueprint Renderers complete. %d blueprint classes processed."), Settings->BlueprintRenderers.Num());
}

