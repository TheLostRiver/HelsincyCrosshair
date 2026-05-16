// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "UObject/Object.h"
#include "HelsincyDamageIndicatorTypes.generated.h"

class UTexture2D;

// 定义伤害指示器原生 Tag | Define damage indicator native Tags
struct FHelsincyDamageIndicator_Tags
{
	static const FGameplayTag Indicator_Style_Arrow; // 箭头样式 | Arrow style
	static const FGameplayTag Indicator_Style_Image; // 图片样式 | Image style
	static const FGameplayTag Indicator_Style_Arc; // 弧形样式 | Arc style

	static void InitializeNativeTags();
};

UENUM(BlueprintType)
enum class EHelsincyDamageIndicatorPlacementMode : uint8
{
	RadialCircle UMETA(DisplayName = "Radial Circle"),
	WindowEdge UMETA(DisplayName = "Window Edge")
};

/**
 * @brief Arc 风格的方向提示模式。
 *        Direction cue mode used by the Arc damage indicator style.
 */
UENUM(BlueprintType)
enum class EHelsincyDamageIndicatorArcDirectionCueMode : uint8
{
	/** 不绘制方向提示，仅保留弧光。 | Draw only the arc without an extra direction cue. */
	None UMETA(DisplayName = "None"),

	/** 在弧线中心绘制 V 形提示。 | Draw a small V-shaped cue at the arc center. */
	CenterChevron UMETA(DisplayName = "Center Chevron"),

	/** 使用非对称尖端暗示方向。 | Use an asymmetric tapered tip to imply direction. */
	AsymmetricTaper UMETA(DisplayName = "Asymmetric Taper"),

	/** 在弧线中心绘制短楔形指针。 | Draw a short center nib as the direction anchor. */
	CenterNib UMETA(DisplayName = "Center Nib")
};

// 箭头专用配置 | Arrow-specific configuration
USTRUCT(BlueprintType)
struct FHelsincy_Ind_ArrowConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow", meta = (ClampMin = "6.0", ClampMax = "96.0"))
	float Size = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	FLinearColor Color = FLinearColor::Red;
};

// 图片专用配置 | Image-specific configuration
USTRUCT(BlueprintType)
struct FHelsincy_Ind_ImageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image")
	UTexture2D* Texture = nullptr;

	// 限制宽高 (防止图片过大) | Limit width/height (prevent oversized images)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (ClampMin = "24.0", ClampMax = "96.0", EditCondition = "Texture != nullptr"))
	FVector2D Size = FVector2D(32.0f, 32.0f);

	// 图片颜色/染色 (白色表示原图) | Image color/tint (white = original)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (EditCondition = "Texture != nullptr"))
	FLinearColor Tint = FLinearColor::White;

	// 图片是否跟随方向旋转？ | Does image rotate to follow direction?
	// True: 图片会旋转指向敌人 | True: image rotates to point at enemy
	// False: 图片始终保持正向 (适合圆形图标) | False: image stays upright (good for circular icons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (EditCondition = "Texture != nullptr"))
	bool bRotateImage = true;

	// 旋转偏移 (度) | Rotation offset (degrees)
	// 如果你的贴图默认是箭头指向上，这里设为 0
	// If your texture arrow points up by default, set this to 0
	// 如果贴图箭头指向右，这里设为 -90 以校准
	// If texture arrow points right, set to -90 to calibrate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", meta = (ClampMin = "-180.0", ClampMax = "180.0", EditCondition = "Texture != nullptr"))
	float RotationOffset = 0.0f;
};

/**
 * @brief Arc 风格伤害指示器配置。
 *        Configuration for the Arc damage indicator style.
 */
USTRUCT(BlueprintType)
struct FHelsincy_Ind_ArcConfig
{
	GENERATED_BODY()

	/** 弧光贴图；为空时使用运行时生成的默认 alpha mask。 | Arc mask texture; uses the generated default alpha mask when null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc")
	UTexture2D* ArcMaskTexture = nullptr;

	/** 弧光绘制尺寸。 | Draw size of the arc layer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc", meta = (ClampMin = "32.0", ClampMax = "512.0"))
	FVector2D Size = FVector2D(220.0f, 80.0f);

	/** 弧光颜色。 | Tint color for the arc layer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc")
	FLinearColor Color = FLinearColor(1.0f, 0.04f, 0.08f, 1.0f);

	/** 柔光层缩放倍率。 | Scale multiplier for the optional glow pass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float GlowScale = 1.18f;

	/** 柔光层透明度倍率。 | Opacity multiplier for the optional glow pass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GlowOpacity = 0.35f;

	/** 命中瞬间缩放冲击强度。 | Scale punch strength applied at impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float ImpactScalePunch = 0.10f;

	/** 方向提示模式。 | Direction cue mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc|Direction Cue")
	EHelsincyDamageIndicatorArcDirectionCueMode DirectionCueMode = EHelsincyDamageIndicatorArcDirectionCueMode::CenterNib;

	/** 方向提示强度。 | Direction cue strength. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc|Direction Cue", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float DirectionCueStrength = 1.0f;

	/** 方向提示基础尺寸。 | Base size of the procedural direction cue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arc|Direction Cue", meta = (ClampMin = "4.0", ClampMax = "96.0"))
	FVector2D CueSize = FVector2D(18.0f, 14.0f);
};

/**
 * 伤害指示器的视觉配置
 * Visual configuration for damage indicators
 */
USTRUCT(BlueprintType)
struct FHelsincy_DamageIndicatorProfile
{
    GENERATED_BODY()

    // 总开关 | Master switch
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    bool bEnabled = true;

	// 选择渲染样式 | Select rendering style
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (Categories = "Indicator.Style", EditCondition = "bEnabled"))
	FGameplayTag IndicatorStyleTag;

    // 指示器存在时间 (秒) | Indicator duration (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1.0", ClampMax = "30.0", EditCondition = "bEnabled"))
    float Duration = 3.0f;

	// 指示器放置模式 | Indicator placement mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (EditCondition = "bEnabled"))
	EHelsincyDamageIndicatorPlacementMode PlacementMode = EHelsincyDamageIndicatorPlacementMode::RadialCircle;

	// 窗口边缘模式的安全边距 | Safe margin for window-edge placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "256.0", EditCondition = "bEnabled && PlacementMode == EHelsincyDamageIndicatorPlacementMode::WindowEdge"))
	float EdgeMargin = 48.0f;

	// 窗口边缘模式的角落避让距离 | Corner padding for window-edge placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "128.0", EditCondition = "bEnabled && PlacementMode == EHelsincyDamageIndicatorPlacementMode::WindowEdge"))
	float EdgeCornerPadding = 24.0f;

	// WindowEdge 模式默认隐藏圆环 | Hide the radial circle by default in WindowEdge mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (EditCondition = "bEnabled && PlacementMode == EHelsincyDamageIndicatorPlacementMode::WindowEdge"))
	bool bHideCircleInWindowEdgeMode = true;

	// 圆环的最大不透明度 (0.0 - 1.0) | Circle max opacity (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnabled"))
	float CircleMaxOpacity = 0.25f;

	// 箭头的最大不透明度 (0.0 - 1.0) | Pointer max opacity (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnabled"))
	float PointerMaxOpacity = 1.0f;

	// 淡入时间 (秒) | Fade-in time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "bEnabled"))
	float FadeInTime = 0.2f;

	// 淡出时间 (秒) | Fade-out time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "bEnabled"))
	float FadeOutTime = 0.5f;

    // --- 圆环设置 | Circle Settings ---
    
    // 是否显示背景圆环 | Whether to show background circle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle", meta = (EditCondition = "bEnabled"))
    bool bShowCircle = true;

	// 圆形分段数 | Circle segment count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle", meta = (ClampMin = "64", ClampMax = "360", EditCondition = "bEnabled"))
	int32 CircleSegments = 64;

    // 圆环半径 | Circle radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle", meta = (ClampMin = "50.0", ClampMax = "300.0", EditCondition = "bEnabled"))
    float Radius = 150.0f;

    // 圆环颜色 | Circle color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle", meta = (EditCondition = "bEnabled"))
    FLinearColor CircleColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.2f);

	// 圆环线条粗细 | Circle line thickness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle", meta = (ClampMin = "2.0", ClampMax = "30.0", EditCondition = "bEnabled"))
    float CircleThickness = 2.0f;

    // --- 箭头设置 | Arrow Settings ---

    // 箭头位置偏移 | Arrow position offset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow", meta = (EditCondition = "bEnabled"))
    bool bPointerOutsideCircle = true;
	
	// 距离圆环的间距 | Distance offset from circle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "5.0", ClampMax = "30.0", EditCondition = "bEnabled"))
	float PointerDistanceOffset = 5.0f;

    // 箭头的平滑移动速度 | Arrow smooth movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow", meta = (ClampMin = "5.0", ClampMax = "30.0", EditCondition = "bEnabled"))
    float RotationInterpSpeed = 10.0f;

	// 具体的样式配置 | Specific style configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Arrow", meta = (EditCondition = "bEnabled"))
	FHelsincy_Ind_ArrowConfig ArrowConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Image", meta = (EditCondition = "bEnabled"))
	FHelsincy_Ind_ImageConfig ImageConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Arc", meta = (EditCondition = "bEnabled"))
	FHelsincy_Ind_ArcConfig ArcConfig;

    FHelsincy_DamageIndicatorProfile()
    {
    }
};

/**
 * 运行时：记录单个伤害来源的状态
 * Runtime: tracks state of a single damage source
 */
struct FHelsincy_ActiveDamageIndicator
{
    // 伤害来源的世界位置 (或者是 Actor 指针，如果是 Actor 则可以追踪移动)
    // Damage source world location (or Actor pointer if actor-based, enabling movement tracking)
    TWeakObjectPtr<AActor> SourceActor;
    FVector SourceLocation; // 如果 Actor 销毁了，记录最后位置 | Last known location if Actor is destroyed
    bool bUseActor;

    // 剩余时间 | Time remaining
    float TimeRemaining;

	// 记录初始总时长，用于计算淡入 | Record initial total duration for fade-in calculation
	float InitialDuration;
    
    // 当前显示的屏幕角度 (用于插值平滑) | Current displayed screen angle (for smooth interpolation)
    float CurrentSmoothAngle; 

    // 初始化 | Initialization
    FHelsincy_ActiveDamageIndicator(AActor* InActor, FVector InLoc, float InDuration, float StartAngle)
        : SourceActor(InActor), SourceLocation(InLoc), bUseActor(InActor != nullptr), 
          TimeRemaining(InDuration), InitialDuration(InDuration), CurrentSmoothAngle(StartAngle)
    {}
};
