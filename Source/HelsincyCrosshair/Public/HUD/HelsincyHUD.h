// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes/HelsincyCrosshairTypes.h"
#include "GameFramework/HUD.h"
#include "HelsincyHUD.generated.h"

// Keep the existing GENERATED_BODY line numbers stable until UHT regenerates this header.

class UHelsincyShapeRenderer;
class UHelsincyCrosshairComponent;
class UHelsincyCrosshairManagerSubsystem;
class APawn;
class FTexture;
#if !UE_BUILD_SHIPPING
class UHelsincyDamageIndicatorComponent;
class UHelsincyDamageIndicatorSubsystem;
#endif

// 用于在绘制函数间传递计算好的状态，避免参数爆炸
// Used to pass pre-calculated state between draw functions, avoiding parameter explosion
struct FHelsincyHitMarkerRenderState
{
	// 基础中心点 (未震动) | Base center point (before shake)
	FVector2D BasePosition; 
    
	// 计算好的全局偏移向量 (每一帧固定一个值，应用到所有臂)
	// Pre-calculated global offset vector (fixed per frame, applied to all arms)
	FVector2D CalculatedGlobalOffset;

	// 计算好的法线震动强度 | Pre-calculated normal shake intensity
	float CurrentNormalShakeIntensity;

	// 稳定法线震动采样值 | Stable per-arm normal shake samples
	float NormalShakeValues[4] = {0.f, 0.f, 0.f, 0.f};
	
	FLinearColor Color;     // 计算好淡出后的颜色 | Calculated color after fade
	float InnerOffset;      // 内圈半径 (像素) | Inner radius (pixels)
	float Length;           // 指针长度 (像素) | Pointer length (pixels)
	float Thickness;        // 粗细 (像素) | Thickness (pixels)
	float Scale;            // DPI 缩放 | DPI scale
	float ImpactRotationDegrees = 0.0f;
	float ImpactScaleMultiplier = 1.0f;
	float ImpactArmLengthMultiplier = 1.0f;
	float ImpactMotionAlpha = 0.0f;
};

/**
 * 
 */
UCLASS()
class HELSINCYCROSSHAIR_API AHelsincyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	virtual void ShowDebugInfo(float& YL, float& YPos) override;

protected:

	UPROPERTY()
	TWeakObjectPtr<UHelsincyCrosshairComponent> CrosshairComponent {nullptr};

	TWeakObjectPtr<APawn> CachedPawn {nullptr};

	UPROPERTY()
	UHelsincyCrosshairManagerSubsystem* HCSubsystem {nullptr};

	UHelsincyCrosshairComponent* GetCrosshairComponent();

	UHelsincyCrosshairManagerSubsystem* GetCrosshairManagerSubsystem();

	float GetDPIScaling() const;
	
	FGameplayTag CurrentCrosshairRendererTag {FGameplayTag::EmptyTag};

	UPROPERTY()
	UHelsincyShapeRenderer* CurrentCrosshairRenderer {nullptr};

private:
	static constexpr float DPIScalingReferenceHeight = 1080.0f;
	static constexpr float DPIScalingMax = 4.0f;

	void UpdateCrosshairRenderer(const FHelsincyCrosshairProfile& Profile, FVector2D Spread, FVector2D CrosshairCenter, FLinearColor FinalColor, float Scale);
	
	void DrawCenterDot(const FHelsincyCrosshairProfile& Profile, FVector2D Center, float Scale);

	// 45度角方向常量 | 45-degree direction constants
	static constexpr float Diag = 0.70710678f;
	// 缓存常用的方向向量 (45度角) | Cache common direction vectors (45 degrees)
	static const FVector2D HitMarkerDirs[4];

	// 命中反馈绘制 | Hit marker drawing
	void DrawHitMarkers_Refactor(const FHelsincyCrosshairProfile& Profile, const TArray<FHelsincy_ActiveHitMarker>& Markers, FVector2D Center, float Scale);
	// 计算单个 Marker 的物理状态 (震动、动画插值、颜色)
	// Calculate a single Marker's physical state (shake, animation interp, color)
	void CalculateHitMarkerState(const FHelsincyCrosshairProfile& Profile, const FHelsincy_ActiveHitMarker& Marker, FVector2D Center, float Scale, FHelsincyHitMarkerRenderState& OutState);

	// 绘制模式实现 | Draw mode implementations
	void DrawHitMarker_Image(const FHelsincy_HitMarkerProfile& Config, const FHelsincyHitMarkerRenderState& State);
	// 尖头/针状模式 (准星呈现尖锐的针状 带有微弱的光晕感 内粗外尖)
	// Tapered/needle mode (sharp needle-like shape with subtle glow, thick inner, thin outer)
	void DrawHitMarker_Tapered(const FHelsincy_HitMarkerProfile& HitMarkerProfile, const FHelsincyHitMarkerRenderState& State, FTexture* SmoothTexture);
	// 传统线条模式 (经典的矩形长条 干净利落 没有渐变粗细)
	// Traditional line mode (classic rectangular bar, clean and sharp, no tapered width)
	void DrawHitMarker_Linear(const FHelsincyHitMarkerRenderState& State);

#if !UE_BUILD_SHIPPING
	// ShowDebug 面板绘制 (从 ShowDebugInfo 中提取以降低单函数复杂度)
	// ShowDebug panel drawing (extracted from ShowDebugInfo to reduce single-function complexity)
	void ShowDebugCrosshairPanel(float& YL, float& YPos);
	void ShowDebugDamageIndicatorPanel(float& YL, float& YPos);
#endif

	// 几何可视化调试颜色常量 | Geometry visualization debug color constants
	static const FLinearColor DebugColor_CenterRef;
	static const FLinearColor DebugColor_SpreadBox;
	static const FLinearColor DebugColor_DIDirectionRay;
	
};

