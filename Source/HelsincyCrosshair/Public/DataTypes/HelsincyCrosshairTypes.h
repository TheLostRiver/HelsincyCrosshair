// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "HelsincyCrosshairTypes.generated.h"


// 定义基础臂 (方向 + 对应的扩散轴)
// Basic arm definition (direction + corresponding spread axis)
struct FArm
{
	FVector2D Dir;
	float Spread;
};

// 缓存线段数据的结构
// Cached line segment data
struct FLineSeg 
{ 
	FVector2D P1; 
	FVector2D P2; 
};

// HitMarker 模式枚举
// HitMarker mode enum
UENUM(BlueprintType)
enum class EHitMarkerMode : uint8
{
	/**
	 * 多实例模式 (经典行为): 每次命中可能创建新的 HitMarker 实例
	 * Multi-instance mode (classic behavior): each hit may create a new marker instance
	 */
	MultiInstance    UMETA(DisplayName = "Multi-Instance (Classic)"),

	/**
	 * 单实例模式 (COD 风格): 始终只有一个 HitMarker，命中时刷新状态
	 * Single-instance mode (COD-style): always one marker, hits refresh state
	 */
	SingleInstance   UMETA(DisplayName = "Single-Instance (COD-Style)")
};

// HitMarker 激活期间基础准星显示策略
// Base crosshair visibility policy while HitMarker is active
UENUM(BlueprintType)
enum class EHelsincyHitMarkerCrosshairVisibilityPolicy : uint8
{
	// 命中反馈期间基础准星照常显示 | Keep the base crosshair visible during HitMarker feedback
	KeepVisible UMETA(DisplayName = "Keep Visible"),

	// 命中反馈期间按透明度缩放基础准星 | Scale base crosshair alpha during HitMarker feedback
	ScaleAlpha  UMETA(DisplayName = "Scale Alpha"),

	// 命中反馈期间完全隐藏基础准星 | Hide the base crosshair during HitMarker feedback
	Hide        UMETA(DisplayName = "Hide")
};

/**
 * Single-instance hitmarker render backend.
 * LegacyGeometry preserves the current procedural implementation.
 * SpriteDualLayer upgrades the final draw step to Core + Glow sprites.
 */
UENUM(BlueprintType)
enum class EHelsincySingleHitMarkerRenderMode : uint8
{
	LegacyGeometry UMETA(DisplayName = "Legacy Geometry"),
	SpriteDualLayer UMETA(DisplayName = "Sprite Dual-Layer")
};

// 优先级枚举
// Priority enum
UENUM(BlueprintType)
enum class EHitMarkerPriority : uint8
{
	Low_Priority_Body        = 0 UMETA(DisplayName = "Body Shot"),
	Medium_Priority_Head     = 1 UMETA(DisplayName = "Head Shot"),
	High_Priority_Kill       = 2 UMETA(DisplayName = "Kill")
};

/**
 * 单实例命中准星运行相位
 * Runtime phase for the single-instance hit marker
 */
UENUM(BlueprintType)
enum class EHelsincySingleHitMarkerPhase : uint8
{
	/** 不可见状态 | Fully hidden */
	Hidden        UMETA(DisplayName = "Hidden"),

	/** 激活保持阶段 | Active hold phase */
	ActiveHold    UMETA(DisplayName = "Active Hold"),

	/** 尾部淡出阶段 | Tail fade phase */
	TailFade      UMETA(DisplayName = "Tail Fade")
};

// 定义原生 Tag 便于 C++ 引用
// Native Tag definitions for C++ reference
struct FHelsincyCrosshair_Tags
{
	////////////////////////////////////////////////////
	///      准星 | Crosshair Shapes
	///////////////////////////////////////////////////
	static const FGameplayTag Shape_Cross;
	static const FGameplayTag Shape_Circle;
	static const FGameplayTag Shape_DotOnly;
	static const FGameplayTag Shape_Image;
	static const FGameplayTag Shape_TStyle;

	static const FGameplayTag Shape_Triangle; // 等腰三角形 | Isosceles triangle
	static const FGameplayTag Shape_Rectangle; // 矩形/方框 | Rectangle / box
	static const FGameplayTag Shape_Chevron; // V形/角形 | V-shaped / chevron
	static const FGameplayTag Shape_Polygon; // 多边形 (用于六边形) | Polygon (for hexagon, etc.)
	static const FGameplayTag Shape_Wings; // 双翼/梯形准星 | Wings / ladder crosshair
};


// 定义运行时 HitMarker 实例
// Runtime hit marker instance
struct FHelsincy_ActiveHitMarker
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	// 线条/圆环 粗细 | Line / ring thickness
	float Thickness = 2.0f;

	// 存储该次命中的特定颜色 | Color for this specific hit
	FLinearColor CurrentColor {FLinearColor::White};

	// [新增] 记录优先级 | Priority level
	EHitMarkerPriority Priority {EHitMarkerPriority::Low_Priority_Body};

	// 缩放比例 | Size scale
	float SizeScale = 1.0f;

	// 稳定震动种子 | Stable shake seed
	int32 ShakeSeed = 0;

	// 震动相位 | Shake phase
	float ShakePhase = 0.0f;

	// 屏幕空间震动方向 | Screen-space shake direction
	FVector2D ShakeDirection = FVector2D(1.0f, 0.0f);

	// 本次命中的震动能量 | Shake energy for this hit
	float ShakeEnergy = 1.0f;

	// COD 风格冲击运动能量 | COD-style impact motion energy
	float ImpactMotionEnergy = 1.0f;

	// 归一化伤害强度 | Normalized damage scale
	float ImpactDamageScale = 0.5f;

	// 冲击旋转方向 (-1/1) | Impact rotation direction (-1/1)
	float ImpactMotionSign = 1.0f;

	FHelsincy_ActiveHitMarker(float Duration)
		: TimeRemaining(Duration), TotalDuration(Duration)
	{}

	FHelsincy_ActiveHitMarker(float InTimeRemaining, float InTotalDuration, float InThickness, const FLinearColor& InColor, EHitMarkerPriority InPriority = EHitMarkerPriority::Low_Priority_Body, float InSizeScale = 1.0f)
		: TimeRemaining(InTimeRemaining),
		  TotalDuration(InTotalDuration),
		  Thickness(InThickness),
		  CurrentColor(InColor),
		  Priority(InPriority),
		  SizeScale(InSizeScale)
	{
	}
};

/**
 * 命中反馈配置
 * Hit marker configuration
 */
USTRUCT(BlueprintType)
struct FHelsincy_HitMarkerProfile
{
	GENERATED_BODY()

	// 是否启用命中反馈 | Enable hit marker feedback
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	bool bEnabled = true;

	// HitMarker 模式选择 | HitMarker mode selection
	// SingleInstance (COD 风格): 始终只有一个 HitMarker, 命中刷新状态
	// MultiInstance (经典): 每次命中可创建新实例 (向后兼容)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (EditCondition = "bEnabled"))
	EHitMarkerMode Mode = EHitMarkerMode::SingleInstance;
	
	// 是否使用尖头(针状)样式？
	// Use tapered (needle-like) style?
	// True = 类似 Cod 的尖头效果 (内粗外细) | COD-like tapered effect (thick inner, thin outer)
	// False = 传统的等宽线条效果 | Traditional uniform-width line style
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (EditCondition = "bEnabled"))
	bool bUseTaperedShape = true;

	// 针状风格下 线条辉光宽度缩放 | Tapered style line glow width scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (ClampMin = "1.0", ClampMax = "5.0", EditCondition = "bEnabled && bUseTaperedShape"))
	float TaperedShapeGlowWidthScale {2.f};

	// 针状风格下 线条辉光透明度缩放 | Tapered style line glow alpha scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (ClampMin = "0.1", ClampMax = "0.9", EditCondition = "bEnabled && bUseTaperedShape"))
	float TaperedShapeGlowAlphaScale {0.4f};

	// 命中反馈持续时间 (秒) | Hit marker duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "bEnabled"))
	float Duration = 0.25f;

	// 合并命中反馈阈值 (单位s, 一个命中反馈实例剩余存活时间小于这个阈值会被合并)
	// Hit marker merge threshold (seconds). Markers with remaining time below this are merged.
	// 通常来说只合并最近的一个命中反馈实例
	// Typically only the most recent marker is merged.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.1", ClampMax = "0.3", EditCondition = "bEnabled"))
	float MergeThreshold {0.15};

	// 命中反馈颜色 (通常为红色或白色) | Hit marker color (typically red or white)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (EditCondition = "bEnabled"))
	FLinearColor Color = FLinearColor::White;

	// 爆头颜色 | Headshot color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (EditCondition = "bEnabled"))
	FLinearColor HeadshotColor = FLinearColor::Red;

	// 爆头时命中反馈大小缩放 | Headshot hit marker size scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "1.0", ClampMax = "5.0", EditCondition = "bEnabled"))
	float HeadShotScale {1.3};

	// 击杀颜色 | Kill color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (EditCondition = "bEnabled"))
	FLinearColor KillColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	// 击杀时命中反馈大小缩放 | Kill hit marker size scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "1.0", ClampMax = "5.0", EditCondition = "bEnabled"))
	float KillScale = 1.7f;

	// 当产生击杀命中反馈时 是否清除 除了击杀命中反馈之外的所有反馈
	// Clear all non-kill markers when a kill marker is triggered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (EditCondition = "bEnabled"))
	bool bClearAllOldHitMarkerOnKill = true;

	// 线条粗细 | Line thickness
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1.0", ClampMax = "20.0", EditCondition = "bEnabled"))
	float Thickness = 2.0f;

	// 基础间距：决定 HitMarker 离准星中心的距离
	// Base distance: determines how far the hit marker is from the crosshair center.
	// 独立于动画参数，方便适配不同大小的准星
	// Independent from animation parameters for easy adaptation to different crosshair sizes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position", meta = (ClampMin = "1.0", ClampMax = "50.0", EditCondition = "bEnabled"))
	float BaseDistance = 8.0f;

	// --- 动画参数 | Animation Parameters ---

	// 初始大小 (线条长度) | Start size (line length)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "10.0", ClampMax = "100.0", EditCondition = "bEnabled"))
	float StartSize = 16.0f;

	// 结束大小 (线条长度) | End size (line length)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "6.0", ClampMax = "50.0", EditCondition = "bEnabled"))
	float EndSize = 8.0f;

	// 初始中心偏移 (线条距离中心的距离) | Start center offset (distance from center)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "50.0", EditCondition = "bEnabled"))
	float StartOffset = 0.0f;

	// 结束中心偏移 (扩散效果关键: 设置比 StartOffset 大就会向外扩散)
	// End center offset (key for spread effect: set larger than StartOffset for outward expansion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "6.0", ClampMax = "50.0", EditCondition = "bEnabled"))
	float EndOffset = 12.0f;

	// 一体式震动强度 (像素) | Unified shake intensity (pixels)
	// 建议值: 3.0 ~ 8.0, 设为 0 则关闭震动
	// Recommended: 3.0 ~ 8.0, set to 0 to disable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "30.0", EditCondition = "bEnabled"))
	float ShakeIntensity = 5.0f;

	// 独立法线震动强度 (Normal/Arm Shake)
	// Per-arm normal shake intensity (pixels)
	// 准星的每个臂沿着垂直方向的抖动强度 (像素)
	// Each arm jitters perpendicular to its direction, creating a "vibrating" or "electric" effect.
	// 建议值: 5.0 ~ 15.0 | Recommended: 5.0 ~ 15.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "30.0", EditCondition = "bEnabled"))
	float NormalShakeIntensity = 5.0f;

	// 震动是否随时间衰减? (对两种震动都有效)
	// Should shake decay over time? (applies to both shake types)
	// True: 刚出现时震动大，快消失时震动小 (推荐，更有物理感)
	// True: Strong shake at start, weakens near end (recommended, more physical feel)
	// False: 全程保持同样强度的震动
	// False: Constant shake intensity throughout
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (EditCondition = "bEnabled"))
	bool bShakeDecay = true;

	// 震动频率 (弧度/秒) | Shake frequency (radians per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "80.0", EditCondition = "bEnabled"))
	float ShakeFrequency = 34.0f;

	// 阻尼速度，值越大越快停下 | Damping speed; higher stops sooner
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "60.0", EditCondition = "bEnabled"))
	float ShakeDamping = 12.0f;

	// 爆头震动能量倍率 | Headshot shake energy multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "4.0", EditCondition = "bEnabled"))
	float HeadshotShakeMultiplier = 1.25f;

	// 击杀震动能量倍率 | Kill shake energy multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "5.0", EditCondition = "bEnabled"))
	float KillShakeMultiplier = 1.5f;

	// 是否使用 COD 风格冲击运动 | Use COD-style impact motion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (EditCondition = "bEnabled"))
	bool bUseImpactMotion = true;

	// 冲击旋转角度 (度) | Impact rotation angle in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "24.0", EditCondition = "bEnabled && bUseImpactMotion"))
	float ImpactRotationDegrees = 6.0f;

	// 整体缩放冲击量 | Whole-marker scale punch amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "0.6", EditCondition = "bEnabled && bUseImpactMotion"))
	float ImpactScalePunch = 0.18f;

	// 臂长冲击量 | Arm length punch amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "0.6", EditCondition = "bEnabled && bUseImpactMotion"))
	float ImpactArmLengthPunch = 0.14f;

	// 平移震动混合权重 | Secondary translation shake weight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnabled && bUseImpactMotion"))
	float ImpactTranslationWeight = 0.25f;

	// 冲击运动主时长 | Main impact motion duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.05", ClampMax = "0.40", EditCondition = "bEnabled && bUseImpactMotion"))
	float ImpactMotionDuration = 0.18f;

	// 伤害对冲击强度的影响 | Damage influence on impact strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnabled && bUseImpactMotion"))
	float DamageToImpactScale = 0.35f;

	// 最大冲击运动能量 | Max impact motion energy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.25", ClampMax = "4.0", EditCondition = "bEnabled && bUseImpactMotion"))
	float MaxImpactMotionEnergy = 1.8f;

	// 可选：自定义图片 (如果为空，则使用程序化绘制的 'X')
	// Optional: custom texture (if empty, uses procedurally drawn 'X')
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (EditCondition = "bEnabled"))
	UTexture2D* CustomTexture = nullptr;

	// --- 单实例模式专属配置 | Single-Instance Mode Specific ---

	// 命中时的缩放脉冲强度 (1.0 = 无脉冲)
	// Scale pulse intensity on hit (1.0 = no pulse)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "1.0", ClampMax = "2.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float HitPulseScale = 1.2f;

	// 脉冲恢复速度 (值越大回弹越快) | Pulse recovery speed (higher = faster snap-back)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "5.0", ClampMax = "30.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float HitPulseRecoverySpeed = 15.0f;

	// 击杀时的脉冲缩放 (比普通命中更大的脉冲)
	// Kill pulse scale (larger pulse than normal hit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "1.0", ClampMax = "3.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float KillPulseScale = 1.5f;

	// 单实例模式: 固定线条大小 (不做 StartSize→EndSize 动画)
	// Single-instance: fixed line size (no StartSize→EndSize animation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "4.0", ClampMax = "40.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float SingleInstanceSize = 10.0f;

	// 单实例模式: 固定中心距离 (不做 StartOffset→EndOffset 动画)
	// Single-instance: fixed center distance (no offset animation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "2.0", ClampMax = "30.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float SingleInstanceOffset = 8.0f;

	// Single-instance render backend selection.
	// Keep LegacyGeometry as the default until the Sprite path is fully wired.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Sprite",
		meta = (EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	EHelsincySingleHitMarkerRenderMode SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::LegacyGeometry;

	// Optional Core sprite for the SpriteDualLayer backend.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Sprite",
		meta = (EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && SingleInstanceRenderMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer"))
	UTexture2D* SingleInstanceCoreTexture = nullptr;

	// Optional Glow sprite for the SpriteDualLayer backend.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Sprite",
		meta = (EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && SingleInstanceRenderMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer"))
	UTexture2D* SingleInstanceGlowTexture = nullptr;

	// Multiplier applied to the shared sprite size derived from SingleInstanceSize and SingleInstanceOffset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Sprite",
		meta = (ClampMin = "0.25", ClampMax = "3.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && SingleInstanceRenderMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer"))
	float SingleInstanceCoreScale = 0.80f;

	// Additional size multiplier for the Glow layer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Sprite",
		meta = (ClampMin = "0.25", ClampMax = "3.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && SingleInstanceRenderMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer"))
	float SingleInstanceGlowScale = 0.86f;

	// Extra opacity scale applied to the Glow layer on top of the runtime state opacity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Sprite",
		meta = (ClampMin = "0.0", ClampMax = "1.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && SingleInstanceRenderMode == EHelsincySingleHitMarkerRenderMode::SpriteDualLayer"))
	float SingleInstanceGlowOpacityScale = 0.26f;

	// 单实例模式: 淡出阶段占总时长的比例 (0.3 = 最后 30% 时间淡出)
	// Single-instance: fade-out portion of total duration (0.3 = last 30% fades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "0.1", ClampMax = "0.8",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float SingleInstanceFadeRatio = 0.3f;

	// 单实例模式：命中能量上限，供后续状态机和调试可视化使用
	// Single-instance: max impact energy for future state machine / debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "0.1", ClampMax = "4.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float SingleInstanceMaxImpactEnergy = 1.0f;

	// 单实例模式：命中能量衰减速度，供后续状态机使用
	// Single-instance: impact energy decay speed for future state machine use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "0.1", ClampMax = "30.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float SingleInstanceImpactDecaySpeed = 8.0f;

	// 单实例模式：命中强调段持续时间
	// Single-instance: accent duration for hit emphasis
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance",
		meta = (ClampMin = "0.01", ClampMax = "0.3",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	float SingleInstanceAccentDuration = 0.08f;

	// HitMarker 可见期间基础准星的显示策略
	// Base crosshair visibility policy while any HitMarker is visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair Visibility",
		meta = (EditCondition = "bEnabled"))
	EHelsincyHitMarkerCrosshairVisibilityPolicy CrosshairVisibilityWhileActive =
		EHelsincyHitMarkerCrosshairVisibilityPolicy::Hide;

	// 当策略为 ScaleAlpha 时，对基础准星透明度应用的缩放
	// Alpha scale applied to the base crosshair when policy is ScaleAlpha
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair Visibility",
		meta = (ClampMin = "0.0", ClampMax = "1.0",
				EditCondition = "bEnabled && CrosshairVisibilityWhileActive == EHelsincyHitMarkerCrosshairVisibilityPolicy::ScaleAlpha"))
	float CrosshairAlphaScaleWhileHitMarkerActive = 0.10f;

	// 是否让中心点跟随基础准星策略一起隐藏或弱化
	// Whether the center dot follows the base crosshair visibility policy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair Visibility",
		meta = (EditCondition = "bEnabled"))
	bool bApplyHitMarkerVisibilityPolicyToCenterDot = true;

	// 单实例模式：是否启用视觉隔离策略，用于把命中 X 与基础准星层级拉开
	// Single-instance: enable visual separation so the hitmarker reads above the base crosshair
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Separation",
		meta = (EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance"))
	bool bUseSingleInstanceVisualSeparation = true;

	// 单实例模式：安全间距缩放，>1 时会把命中 X 向外推一点，避免和中心十字糊成一团
	// Single-instance: safe spacing scale. Values above 1 push the hitmarker outward for readability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Separation",
		meta = (ClampMin = "1.0", ClampMax = "2.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && bUseSingleInstanceVisualSeparation"))
	float SingleInstanceSafeSpacingScale = 1.18f;

	// 单实例模式：激活期间基础准星透明度缩放，越低越强调命中 X
	// Single-instance: alpha scale applied to the base crosshair while the hitmarker is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Separation",
		meta = (ClampMin = "0.0", ClampMax = "1.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && bUseSingleInstanceVisualSeparation"))
	float SingleInstanceCrosshairAlphaScale = 0.10f;

	// 单实例模式：激活期间是否直接隐藏中心点
	// Single-instance: whether the center dot should be hidden while the hitmarker is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Separation",
		meta = (EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && bUseSingleInstanceVisualSeparation"))
	bool bHideCenterDotWhenSingleInstanceActive = true;

	// 单实例模式：若不直接隐藏中心点，则对中心点额外乘上的透明度缩放
	// Single-instance: alpha scale for the center dot when it is weakened instead of fully hidden
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SingleInstance|Separation",
		meta = (ClampMin = "0.0", ClampMax = "1.0",
				EditCondition = "bEnabled && Mode == EHitMarkerMode::SingleInstance && bUseSingleInstanceVisualSeparation && !bHideCenterDotWhenSingleInstanceActive"))
	float SingleInstanceCenterDotAlphaScale = 0.25f;
};

/**
 * 单实例 HitMarker 的运行时状态 (COD 风格)
 * Runtime state for single-instance HitMarker (COD-style)
 */
struct FHelsincy_SingleHitMarkerState
{
	// 是否激活 | Is active
	bool bActive = false;

	// 是否可见 | Is currently visible
	bool bVisible = false;

	// 当前运行相位 | Current runtime phase
	EHelsincySingleHitMarkerPhase Phase = EHelsincySingleHitMarkerPhase::Hidden;

	// 当前剩余时间 | Remaining display time
	float TimeRemaining = 0.0f;

	// 每次命中时重置的基准时长 | Base duration to reset to on each hit
	float TotalDuration = 0.0f;

	// Hold 阶段剩余时间 | Remaining time for the hold phase
	float HoldTimeRemaining = 0.0f;

	// TailFade 阶段剩余时间 | Remaining time for the tail fade phase
	float TailFadeTimeRemaining = 0.0f;

	// 当前显示的颜色 | Current display color
	FLinearColor CurrentColor = FLinearColor::White;

	// 当前优先级 | Current priority
	EHitMarkerPriority Priority = EHitMarkerPriority::Low_Priority_Body;

	// 基础缩放 | Base size scale
	float BaseSizeScale = 1.0f;

	// 当前脉冲缩放 (每次命中跳到 PulseScale, 然后 Interp 回 1.0)
	// Current pulse scale (jumps to PulseScale on hit, interps back to 1.0)
	float CurrentPulseScale = 1.0f;

	// 线条粗细 | Thickness
	float Thickness = 2.0f;

	// 当前透明度缓存 | Cached opacity for the current phase
	float Opacity = 0.0f;

	// 命中能量缓存 | Cached impact energy
	float ImpactEnergy = 0.0f;

	// 命中强调强度缓存 | Cached accent strength
	float AccentStrength = 0.0f;

	// 命中震动能量缓存 | Cached shake energy
	float ShakeEnergy = 0.0f;

	// COD 风格冲击运动能量 | COD-style impact motion energy
	float ImpactMotionEnergy = 0.0f;

	// 归一化伤害强度 | Normalized damage scale
	float ImpactDamageScale = 0.0f;

	// 冲击旋转方向 (-1/1) | Impact rotation direction (-1/1)
	float ImpactMotionSign = 1.0f;

	// 稳定震动种子 | Stable shake seed
	int32 ShakeSeed = 0;

	// 当前震动年龄 | Current shake age
	float ShakeAge = 0.0f;

	// 震动相位 | Shake phase
	float ShakePhase = 0.0f;

	// 屏幕空间震动方向 | Screen-space shake direction
	FVector2D ShakeDirection = FVector2D(1.0f, 0.0f);

	/** 处理一次命中事件 | Process a new hit event */
	void ApplyHit(float Duration, float InThickness, const FLinearColor& Color,
				  EHitMarkerPriority InPriority, float InSizeScale, float PulseScale)
	{
		bActive = true;
		TimeRemaining = Duration;
		TotalDuration = Duration;
		Thickness = InThickness;

		// 优先级升级或相同 → 刷新颜色和大小 | Upgrade or same priority → refresh color and size
		if (InPriority >= Priority)
		{
			CurrentColor = Color;
			Priority = InPriority;
			BaseSizeScale = InSizeScale;
		}

		// 触发脉冲 (无论优先级) | Trigger pulse (regardless of priority)
		CurrentPulseScale = PulseScale;
		ImpactEnergy = FMath::Max(0.0f, PulseScale - 1.0f);
		AccentStrength = 1.0f;
		ShakeAge = 0.0f;
	}

	/** 每帧更新 | Per-frame update */
	void Tick(float DeltaTime, float PulseRecoverySpeed)
	{
		if (!bActive) return;

		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			bActive = false;
			TimeRemaining = 0.0f;
			Priority = EHitMarkerPriority::Low_Priority_Body;
			CurrentPulseScale = 1.0f;
			ImpactEnergy = 0.0f;
			AccentStrength = 0.0f;
			ShakeEnergy = 0.0f;
			ShakeAge = 0.0f;
			ImpactMotionEnergy = 0.0f;
			ImpactDamageScale = 0.0f;
			ImpactMotionSign = 1.0f;
			return;
		}

		ShakeAge += DeltaTime;

		// 脉冲恢复: CurrentPulseScale → 1.0 | Pulse recovery: interp back to 1.0
		CurrentPulseScale = FMath::FInterpTo(CurrentPulseScale, 1.0f, DeltaTime, PulseRecoverySpeed);
	}

	/** 同步派生显示状态 | Synchronize derived display state */
	void RefreshDerivedState(float FadeRatio, float MaxImpactEnergy)
	{
		if (!bActive || TimeRemaining <= 0.0f || TotalDuration <= 0.0f)
		{
			bVisible = false;
			Phase = EHelsincySingleHitMarkerPhase::Hidden;
			HoldTimeRemaining = 0.0f;
			TailFadeTimeRemaining = 0.0f;
			Opacity = 0.0f;
			ImpactEnergy = 0.0f;
			AccentStrength = 0.0f;
			ShakeEnergy = 0.0f;
			ShakeAge = 0.0f;
			ImpactMotionEnergy = 0.0f;
			ImpactDamageScale = 0.0f;
			ImpactMotionSign = 1.0f;
			return;
		}

		const float SafeFadeRatio = FMath::Clamp(FadeRatio, 0.0f, 1.0f);
		const float FadeDuration = TotalDuration * SafeFadeRatio;

		bVisible = true;

		if (FadeDuration > KINDA_SMALL_NUMBER && TimeRemaining <= FadeDuration)
		{
			Phase = EHelsincySingleHitMarkerPhase::TailFade;
			HoldTimeRemaining = 0.0f;
			TailFadeTimeRemaining = TimeRemaining;
			Opacity = FMath::Clamp(TimeRemaining / FadeDuration, 0.0f, 1.0f);
		}
		else
		{
			Phase = EHelsincySingleHitMarkerPhase::ActiveHold;
			HoldTimeRemaining = FMath::Max(0.0f, TimeRemaining - FadeDuration);
			TailFadeTimeRemaining = FadeDuration;
			Opacity = 1.0f;
		}

		const float SafeMaxImpactEnergy = FMath::Max(MaxImpactEnergy, KINDA_SMALL_NUMBER);
		const float RawImpactEnergy = FMath::Max(0.0f, CurrentPulseScale - 1.0f);
		ImpactEnergy = FMath::Clamp(RawImpactEnergy, 0.0f, SafeMaxImpactEnergy);
		AccentStrength = FMath::Clamp(RawImpactEnergy / SafeMaxImpactEnergy, 0.0f, 1.0f);
	}

	/** 计算当前实际缩放 | Calculate effective scale */
	float GetEffectiveScale() const
	{
		return BaseSizeScale * CurrentPulseScale;
	}
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ==========================================
//   通用视觉设置 (所有准星共用)
//   Common Visual Settings (shared by all crosshairs)
// ==========================================
USTRUCT(BlueprintType)
struct FHelsincy_VisualSettings
{
    GENERATED_BODY()

	// 基础颜色 | Primary color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor PrimaryColor = FLinearColor::White;

	// --- 全局位置调整 | Global Position Adjustment ---
	// 准星相对于屏幕中心的偏移量
	// Crosshair offset from screen center
	// X > 0: 向右 | Right, X < 0: 向左 | Left
	// Y > 0: 向下 | Down, Y < 0: 向上 | Up
	// (例如: Y = -50 可以让准星稍微靠上，适合 TPP 视角)
	// (e.g. Y = -50 shifts crosshair slightly upward, suitable for TPP)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "-300.0", ClampMax = "300.0"))
	FVector2D GlobalCrosshairOffset = FVector2D::ZeroVector;

	// 线条/圆环 粗细 | Line / ring thickness
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float Thickness = 2.0f;

	// 整体不透明度 | Overall opacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Opacity = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Outline")
	bool bEnableOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (EditCondition = "bEnableOutline"))
	FLinearColor OutlineColor = FLinearColor::Black;

	// 描边额外宽度 | Outline extra width
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (ClampMin = "1.0", ClampMax = "20.0", EditCondition = "bEnableOutline"))
	float OutlineThickness = 1.0f;

	// 目标识别开关 | Target detection toggle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	bool bEnableTargetSwitching = true;

	// 最大识别距离(cm) | Max detection distance (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (ClampMin = "5000.0", ClampMax = "30000.0", EditCondition = "bEnableTargetSwitching"))
	float MaxSwitchingDistance = 10000.f;

	// 用于识别与准心交互的检测通道 | Collision channel for crosshair interaction detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (EditCondition = "bEnableTargetSwitching"))
	TEnumAsByte<ECollisionChannel> SwitchingChannel = ECC_Pawn;

	// 敌军颜色 | Enemy color (Hostile)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (EditCondition = "bEnableTargetSwitching"))
	FLinearColor EnemyColor = FLinearColor::Red;

	// 友军颜色 | Friendly color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (EditCondition = "bEnableTargetSwitching"))
	FLinearColor FriendlyColor = FLinearColor::Green;

	// 中立单位颜色 | Neutral unit color
	// 如果不设置，默认保持 PrimaryColor | If disabled, defaults to PrimaryColor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (EditCondition = "bEnableTargetSwitching"))
	bool bUseNeutralColor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (EditCondition = "bUseNeutralColor"))
	FLinearColor NeutralColor = FLinearColor::Yellow;

	// 交互颜色映射表 | Interaction color map
	// Key = 目标返回的 Tag (如 Target.Loot) | Tag returned by target (e.g. Target.Loot)
	// Value = 准星要变的颜色 (如 金色) | Crosshair color to switch to (e.g. gold)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TMap<FGameplayTag, FLinearColor> InteractionColorMap;

	// [新增] 默认交互颜色 (可选) | Default interaction color (optional)
	// 如果目标实现了接口但 Tag 不在 Map 里，是否使用一个通用高亮色？
	// If target implements the interface but Tag is not in the map, use a generic highlight color?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bUseDefaultInteractionColor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bUseDefaultInteractionColor"))
	FLinearColor DefaultInteractionColor = FLinearColor::Yellow;
};

// ==========================================
//    动态扩散设置 (Spread & Recoil)
//    Dynamic Spread Settings (Spread & Recoil)
//    定义移动、跳跃和后坐力恢复的具体数值
//    Defines movement, jump, and recoil recovery values
// ==========================================
USTRUCT(BlueprintType)
struct FHelsincy_DynamicsSettings
{
    GENERATED_BODY()

	// --- 动态状态扩散 | State Spread ---

	// 是否启用动态扩散？ | Enable dynamic spread?
	// True = 准星随移动/射击扩散 | Crosshair expands with movement/shooting
	// False = 静态准星 (Static Crosshair)，无论怎么动都保持静止 | Static crosshair, no spread regardless of state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread")
	bool bEnableDynamicSpread = true;

	// 移动速度对扩散的影响倍率 | Movement speed spread multiplier (Spread += Speed * Multiplier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread", meta = (ClampMin = "0.01", ClampMax = "0.5", EditCondition = "bEnableDynamicSpread"))
	float VelocityMultiplier = 0.05f;

	// 跳跃/滞空时的固定扩散惩罚 | Fixed spread penalty while airborne (jumping/falling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread", meta = (ClampMin = "5.0", ClampMax = "100.0", EditCondition = "bEnableDynamicSpread"))
	float AirbornePenalty = 50.0f;

	// (高级感) 起跳扩散速度 | Jump expand speed
	// 如果想要"起跳瞬间变大，落地缓慢收缩"的效果，可以将JumpExpandSpeed调大点,LandRecoverySpeed调低点
	// For "instant expand on jump, slow shrink on land": increase JumpExpandSpeed, decrease LandRecoverySpeed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread", meta = (ClampMin = "5.0", ClampMax = "30.0", EditCondition = "bEnableDynamicSpread"))
	float JumpExpandSpeed = 20.0f;

	// 恢复慢 | Landing recovery speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread", meta = (ClampMin = "5.0", ClampMax = "30.0", EditCondition = "bEnableDynamicSpread"))
	float LandRecoverySpeed = 10.0f;

	// 是否将移动扩散应用到 X 轴 (水平) | Apply movement spread to X axis (horizontal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread", meta = (EditCondition = "bEnableDynamicSpread"))
	bool bApplyMovementToX = true;

	// 是否将移动扩散应用到 Y 轴 (垂直) | Apply movement spread to Y axis (vertical)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread", meta = (EditCondition = "bEnableDynamicSpread"))
	bool bApplyMovementToY = true;

	// --- 后坐力扩散 | Recoil Spread ---

	// 后坐力恢复速度 (数值越大恢复越快) | Recoil recovery speed (higher = faster recovery)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "5.0", ClampMax = "30.0", EditCondition = "bEnableDynamicSpread"))
	float RecoilRecoverySpeed = 10.0f;

	// 最大后坐力限制 (防止扩散出屏幕) | Max recoil spread limit (prevents spread from going off-screen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "10.0", ClampMax = "100.0", EditCondition = "bEnableDynamicSpread"))
	float MaxRecoilSpread = 100.0f;
};

// ==========================================
//  形状特定设置 | Shape-Specific Settings
// ==========================================

// A. 十字 / T形 / Chevron | Cross / T-Style / Chevron
USTRUCT(BlueprintType)
struct FHelsincy_CrosshairPresentationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	bool bEnableAdaptivePresentation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Channels", meta = (EditCondition = "bEnableAdaptivePresentation"))
	bool bEnableAutoScale = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Channels", meta = (EditCondition = "bEnableAdaptivePresentation"))
	bool bEnableAutoColorBlend = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Channels", meta = (EditCondition = "bEnableAdaptivePresentation"))
	bool bEnableAutoJitter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.5", ClampMax = "2.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoScale"))
	float IdleScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.5", ClampMax = "2.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoScale"))
	float MoveScaleMax = 1.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.5", ClampMax = "2.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoScale"))
	float AirborneScaleMax = 1.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.5", ClampMax = "2.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoScale"))
	float FireScalePulse = 1.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoScale"))
	float RecoilScaleContribution = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "1.0", ClampMax = "30.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoScale"))
	float ScaleInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "1.0", ClampMax = "30.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoColorBlend"))
	float ColorInterpSpeed = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.0", ClampMax = "20.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoJitter"))
	float FireJitterAmplitude = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "1.0", ClampMax = "30.0", EditCondition = "bEnableAdaptivePresentation && bEnableAutoJitter"))
	float JitterRecoverySpeed = 12.0f;
};

USTRUCT(BlueprintType)
struct FHelsincy_CrosshairPresentationState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scale")
	float TargetScale = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scale")
	float CurrentScale = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
	FLinearColor TargetColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
	FLinearColor CurrentColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion")
	FVector2D CurrentJitterOffset = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion")
	float CurrentJitterStrength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scale")
	float FirePulseStrength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scale")
	float RecoilContribution = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scale")
	float MovementContribution = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scale")
	float AirborneContribution = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
	bool bHasTargetColorOverride = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
	FName ActiveColorReason = NAME_None;
};

USTRUCT(BlueprintType)
struct FHelsincy_CrosshairConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float Length = 12.0f;

	// 中心间隙 (Spread 会基于此增加) | Center gap (spread builds on top of this)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Gap = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float Rotation = 0.0f;

    // 仅用于 Chevron (V形) | Chevron only: V-shape opening angle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair", meta = (ClampMin = "10.0", ClampMax = "350.0"))
    float ChevronOpeningAngle = 90.0f;
};

// B. 径向/多边形类 (Circle, Polygon) | Radial / polygon (Circle, Polygon)
USTRUCT(BlueprintType)
struct FHelsincy_RadialConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial", meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float Radius = 10.0f;

	// 旋转 (用于调整多边形尖头朝向) | Rotation (adjusts polygon tip orientation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float Rotation = 0.0f;

	// 圆形分段数 (数量越多圆形越平滑 视觉效果越好 但会牺牲一定性能)
	// Circle segment count (more = smoother, but slightly more expensive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle", meta = (ClampMin = "24", ClampMax = "360"))
	int32 CircleSegments = 32;

	// 多边形的边数量 仅用于 Polygon (Hexagon等)
	// Polygon side count, used for Polygon only (hexagon, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Polygon", meta = (ClampMin = "5", ClampMax = "30"))
	int32 PolygonSides = 6;
};

// C. 双翼 / 梯形 | Wings / ladder
USTRUCT(BlueprintType)
struct FHelsincy_WingsConfig
{
    GENERATED_BODY()

	// --- Wings (双翼/梯形) 参数 | Wings Parameters ---
	// 是否对齐顶端？ | Align to top?
	// True  = 第一条横线位于屏幕正中心 (适合下坠参考)
	// True  = First horizontal line at screen center (useful for drop reference)
	// False = 整个形状的中心位于屏幕正中心 (默认)
	// False = Entire shape centered on screen (default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings")
	bool bWingsAlignTop = false;

	// 垂直偏移量 (用于在对齐的基础上进行微调)
	// Vertical offset (fine-tune on top of alignment)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "-150.0", ClampMax = "150.0"))
	float WingsVerticalOffset = 0.0f;
    
	// 横线的数量 | Number of horizontal lines
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "2", ClampMax = "20"))
	int32 WingsLineCount = 6;

	// 垂直方向上，每条线的间距 | Vertical spacing between lines
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "2.0", ClampMax = "30"))
	float WingsVerticalSpacing = 6.0f;

	// 最上方横线的长度 | Top line length
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "15.0", ClampMax = "100.0"))
	float WingsTopLength = 20.0f;

	// 最下方横线的长度 (程序会自动在 Top 和 Bottom 之间插值)
	// Bottom line length (auto-interpolated between Top and Bottom)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "5.0", ClampMax = "100.0"))
	float WingsBottomLength = 5.0f;
    
	// 是否绘制垂直的主干线 | Draw vertical trunk lines
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings")
	bool bWingsDrawVerticalLines = true;

	// 双翼形状自身的间距 (不再借用 CrosshairConfig.Gap)
	// Wings own gap (independent from CrosshairConfig.Gap)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "0.0", ClampMax = "150.0"))
	float Gap = 10.0f;

	// 双翼形状自身的旋转角度 | Wings own rotation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float Rotation = 0.0f;
	
};

// 矩形 / 三角形 (简单形状) | Rectangle / Triangle (simple shapes)
USTRUCT(BlueprintType)
struct FHelsincy_BoxConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "6.0", ClampMax = "100.0"))
    FVector2D Size = FVector2D(12.0f, 8.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float Rotation = 0.0f;
};

// 图片类 | Image-based crosshair
USTRUCT(BlueprintType)
struct FHelsincy_ImageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image")
	UTexture2D* Texture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (EditCondition = "Texture != nullptr"))
	FVector2D Size = FVector2D(32.0f, 32.0f);
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (EditCondition = "Texture != nullptr"))
	FLinearColor Tint = FLinearColor::White;
	
};

// 中心点 | Center dot
USTRUCT(BlueprintType)
struct FHelsincy_CenterDotConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CenterDot")
    bool bEnabled = true;

	// 中心点是否总是保持在屏幕正中心? (如果为false 当用户调整了全局偏移时 中心点也会跟着偏移)
	// Should center dot always stay at screen center? (if false, it follows Global Offset)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CenterDot")
	bool bAlwaysStayCentered = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CenterDot", meta = (ClampMin = "1.0", ClampMax = "100.0", EditCondition = "bEnabled"))
    float Size = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CenterDot", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnabled"))
    float Opacity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CenterDot")
    FLinearColor Color = FLinearColor::White;
};

// ==========================================
//    准星配置文件 | Crosshair Profile
//    包含所有视觉参数，支持直接序列化和预设加载
//    Contains all visual parameters, supports serialization and preset loading
// ==========================================
USTRUCT(BlueprintType)
struct FHelsincyCrosshairProfile
{
    GENERATED_BODY()

	// --- 核心选择 | Core Selection ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (Categories = "Crosshair.Shape"))
	FGameplayTag ShapeTag;

	// --- 通用模块 | Common Modules ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modules")
	FHelsincy_VisualSettings VisualsConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modules")
	FHelsincy_DynamicsSettings DynamicConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modules")
	FHelsincy_CrosshairPresentationSettings PresentationConfig;

	// --- 形状配置 | Shape Configs ---
	// 在编辑器中，它们会以折叠的形式出现，非常整洁
	// In the editor, these appear as collapsible sections
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FHelsincy_CrosshairConfig CrosshairConfig;

	// 径向/多边形类 (Circle, Polygon) | Radial / polygon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FHelsincy_RadialConfig RadialConfig;

	// Rect & Triangle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FHelsincy_BoxConfig BoxConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FHelsincy_WingsConfig WingsConfig;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FHelsincy_ImageConfig ImageConfig;

	// --- 额外功能 | Extra Features ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	FHelsincy_CenterDotConfig CenterDotConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	FHelsincy_HitMarkerProfile HitMarkerConfig;

    
};
