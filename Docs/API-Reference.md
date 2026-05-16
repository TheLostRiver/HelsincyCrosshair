# HelsincyCrosshair Plugin — API Reference / API 参考手册

> **Version / 版本**: 1.0  
> **Engine / 引擎**: UE 4.27  
> **Modules / 模块**: `HelsincyCrosshair`, `HelsincyDamageIndicator`

---

## Table of Contents / 目录

1. [Module: HelsincyCrosshair / 准星模块](#1-module-helsincycrosshair--准星模块)
   - 1.1 [UHelsincyCrosshairComponent](#11-uhelsincycrosshaircomponent)
   - 1.2 [UHelsincyCrosshairRenderLibrary (Bridge API)](#12-uhelsincycrosshairrenderlibrary-bridge-api)
   - 1.3 [AHelsincyHUD](#13-ahelsincyhud)
   - 1.4 [UHelsincyCrosshairManagerSubsystem](#14-uhelsincycrosshairmanagersubsystem)
   - 1.5 [UHelsincyCrosshairDataAsset](#15-uhelsincycrosshairdataasset)
   - 1.6 [UHelsincyShapeRenderer](#16-uhelsincyshaperenderer)
   - 1.7 [UHelsincyCrosshairSettings](#17-uhelsincycrosshairsettings)
   - 1.8 [IHelsincyCrosshairInterface](#18-ihelsincycrosshairinterface)
   - 1.9 [IHelsincyTargetInterface](#19-ihelsincytargetinterface)
2. [Module: HelsincyDamageIndicator / 伤害指示器模块](#2-module-helsincydamageindicator--伤害指示器模块)
   - 2.1 [UHelsincyDamageIndicatorComponent](#21-uhelsincydamageindicatorcomponent)
   - 2.2 [UHelsincyDamageIndicatorRenderLibrary (Bridge API)](#22-uhelsincydamageindicatorrenderlibrary-bridge-api)
   - 2.3 [UHelsincyDamageIndicatorSubsystem](#23-uhelsincydamageindicatorsubsystem)
   - 2.4 [UHelsincyIndicatorRenderer](#24-uhelsincyindicatorrenderer)
   - 2.5 [UHelsincyDamageIndicatorDataAsset](#25-uhelsincydamageindicatordataasset)
   - 2.6 [UHelsincyDamageIndicatorSaveGame](#26-uhelsincydamageindicatorsavegame)
   - 2.7 [UHelsincyDamageIndicatorSettings](#27-uhelsincydamageindicatorsettings)
3. [Data Structures / 数据结构](#3-data-structures--数据结构)
   - 3.1 [FHelsincyCrosshairProfile](#31-fhelsincycrosshairprofile)
   - 3.2 [FHelsincy_VisualSettings](#32-fhelsincy_visualsettings)
   - 3.3 [FHelsincy_DynamicsSettings](#33-fhelsincy_dynamicssettings)
   - 3.4 [FHelsincy_CrosshairConfig](#34-fhelsincy_crosshairconfig)
   - 3.5 [FHelsincy_RadialConfig](#35-fhelsincy_radialconfig)
   - 3.6 [FHelsincy_WingsConfig](#36-fhelsincy_wingsconfig)
   - 3.7 [FHelsincy_BoxConfig](#37-fhelsincy_boxconfig)
   - 3.8 [FHelsincy_ImageConfig](#38-fhelsincy_imageconfig)
   - 3.9 [FHelsincy_CenterDotConfig](#39-fhelsincy_centerdotconfig)
   - 3.10 [FHelsincy_HitMarkerProfile](#310-fhelsincy_hitmarkerprofile)
   - 3.11 [FHelsincy_DamageIndicatorProfile](#311-fhelsincy_damageindicatorprofile)
4. [Enums / 枚举](#4-enums--枚举)
5. [GameplayTags / 标签常量](#5-gameplaytags--标签常量)
6. [Performance Stats / 性能统计](#6-performance-stats--性能统计)

---

## 1. Module: HelsincyCrosshair / 准星模块

### 1.1 UHelsincyCrosshairComponent

> **Header**: `Components/HelsincyCrosshairComponent.h`  
> **Parent / 父类**: `UActorComponent`  
> The runtime core of the crosshair system. Attach to the player Pawn/Character.  
> 准星系统的运行时核心。挂载到玩家 Pawn/Character 上。

#### Properties / 属性

| Property | Type | Category | Description (EN) | 说明 (CN) |
|----------|------|----------|-------------------|-----------|
| `CurrentProfile` | `FHelsincyCrosshairProfile` | HelsincyCrosshair | The active crosshair configuration. EditAnywhere, BlueprintReadWrite. | 当前激活的准心完整配置。可编辑、蓝图读写。 |
| `DefaultCrosshairAsset` | `UHelsincyCrosshairDataAsset*` | HelsincyCrosshair\|Defaults | Default DataAsset loaded on BeginPlay. | BeginPlay 时自动加载的默认 DataAsset。 |
| `bUseDefaultCrosshairAssetInit` | `bool` | HelsincyCrosshair\|Defaults | If true, auto-load DefaultCrosshairAsset on BeginPlay. Default: `true`. | 为 true 时 BeginPlay 自动加载默认资产。 |
| `bEnabledCrosshair` | `bool` | HelsincyCrosshair\|Defaults | Master on/off switch. Default: `true`. ReadOnly in BP. | 准星总开关。默认开启。蓝图只读。 |

#### Methods — On/Off Control / 开关控制

| Method | Specifier | Return | Description (EN) | 说明 (CN) |
|--------|-----------|--------|-------------------|-----------|
| `EnableCrosshair()` | BlueprintCallable | `void` | Enable the crosshair at runtime. | 运行时启用准心。 |
| `DisableCrosshair()` | BlueprintCallable | `void` | Disable the crosshair at runtime. | 运行时关闭准心。 |
| `IsEnabledCrosshair()` | BlueprintPure | `bool` | Query whether the crosshair is enabled. | 查询准心是否启用。 |
| `EnableCenterDot()` | BlueprintCallable | `void` | Enable the center dot independently. | 单独启用中心圆点。 |
| `DisableCenterDot()` | BlueprintCallable | `void` | Disable the center dot independently. | 单独关闭中心圆点。 |
| `EnableTargetDetection()` | BlueprintCallable | `void` | Enable target detection (color switching). | 启用目标识别（变色）。 |
| `DisableTargetDetection()` | BlueprintCallable | `void` | Disable target detection. | 关闭目标识别。 |

#### Methods — Recoil & Hit Markers / 后坐力与命中标记

| Method | Specifier | Params | Description (EN) | 说明 (CN) |
|--------|-----------|--------|-------------------|-----------|
| `AddRecoil(float H, float V)` | BlueprintCallable | `HorizontalKick`, `VerticalKick` | Add recoil spread. Call on fire. | 增加后坐力扩散。开火时调用。 |
| `TriggerHitMarker()` | BlueprintCallable | — | Trigger a normal hit marker instance. | 触发普通命中标记。 |
| `TriggerHitMarkerColor(FLinearColor)` | BlueprintCallable | `CustomColor` | Trigger a hit marker with a custom color. | 触发自定义颜色的命中标记。 |
| `TriggerHeadshotMarker()` | BlueprintCallable | — | Trigger a headshot hit marker. Uses `HitMarkerConfig.HeadshotColor`. | 触发爆头命中标记。使用 `HitMarkerConfig.HeadshotColor`。 |
| `TriggerKillMarker()` | BlueprintCallable | — | Trigger a kill hit marker. Uses `HitMarkerConfig.KillColor`. | 触发击杀命中标记。使用 `HitMarkerConfig.KillColor`。 |
| `TriggerHitMarkerAdvanced(Priority, CustomColor, DamageNormalized)` | BlueprintCallable | `Priority`, `CustomColor`, `DamageNormalized` | Trigger hit marker with damage-scaled impact motion. | 触发带伤害缩放冲击运动的命中标记。 |

#### Methods — State Query / 状态查询

| Method | Specifier | Return | Description (EN) | 说明 (CN) |
|--------|-----------|--------|-------------------|-----------|
| `GetCurrentProfile()` | BlueprintPure | `const FHelsincyCrosshairProfile&` | Get the full active profile. | 获取当前完整配置。 |
| `GetCurrentVisualPrimaryColor()` | BlueprintPure | `FLinearColor` | Get the resolved primary color (after target color override). | 获取当前主颜色（已含目标变色结果）。 |
| `GetFinalSpread()` | BlueprintPure | `FVector2D` | Get total spread = state spread + recoil spread. | 获取总扩散 = 状态扩散 + 后坐力扩散。 |
| `GetCurrentTargetAttitude()` | BlueprintPure | `ETeamAttitude::Type` | Current target attitude: Hostile / Friendly / Neutral. | 当前瞄准目标的阵营态度。 |
| `GetCurrentTargetTag()` | BlueprintPure | `FGameplayTag` | Current target's interaction tag (from `IHelsincyTargetInterface`). | 当前目标的交互 Tag。 |
| `GetActiveHitMarkers()` | C++ only | `const TArray<FHelsincy_ActiveHitMarker>&` | Active hit marker instances for HUD rendering. | 活跃命中标记列表，供 HUD 渲染。 |
| `GetStateSpread()` | BlueprintPure | `FVector2D` | Movement/airborne spread only (no recoil). | 仅状态扩散（不含后坐力）。 |
| `GetRecoilSpread()` | BlueprintPure | `FVector2D` | Recoil spread only. | 仅后坐力扩散。 |

#### Methods — Config & Presets / 配置与预设

| Method | Specifier | Params | Description (EN) | 说明 (CN) |
|--------|-----------|--------|-------------------|-----------|
| `LoadFromDataAsset(UHelsincyCrosshairDataAsset*)` | BlueprintCallable | `DataAsset` | Load a full profile from a DataAsset. Recommended for weapon switching. | 从 DataAsset 加载完整配置。推荐武器切换时使用。 |
| `UpdateProfile(const FHelsincyCrosshairProfile&)` | BlueprintCallable | `NewProfile` | Overwrite the current profile entirely. For UI binding. | 整体覆盖当前配置。UI 绑定用。 |
| `SavePreset(FName)` | BlueprintCallable | `PresetName` | Save current profile as a named preset (in-memory). Also sets ActivePresetName. | 将当前配置保存为命名预设（内存级）。同时设置 ActivePresetName。 |
| `LoadPreset(FName)` | BlueprintCallable | `PresetName` → `bool` | Load a named preset. Returns false if not found. | 加载指定预设。未找到返回 false。 |
| `ClearPresetLibrary()` | BlueprintCallable | — | Clear all saved presets from memory. | 清空内存中的全部预设。 |
| `GetAllPresetNames()` | BlueprintPure | — → `TArray<FName>` | Get all preset names in the library. Useful for building UI lists. | 获取预设库中所有名称。方便构建 UI 列表。 |

#### Methods — Disk Persistence / 磁盘持久化

| Method | Specifier | Params | Description (EN) | 说明 (CN) |
|--------|-----------|--------|-------------------|------------|
| `SavePresetsToDisk(FString, int32)` | BlueprintCallable | `SlotName="CrosshairPresets"`, `UserIndex=0` → `bool` | Save the preset library + ActivePresetName to disk via USaveGame. | 将预设库 + 活跃预设名通过 USaveGame 保存到磁盘。 |
| `LoadPresetsFromDisk(FString, int32)` | BlueprintCallable | `SlotName="CrosshairPresets"`, `UserIndex=0` → `bool` | Load presets from disk. Auto-applies LastActivePreset if found. | 从磁盘加载预设库。自动应用上次活跃预设。 |
| `DoesSaveExist(FString, int32)` | BlueprintPure | `SlotName`, `UserIndex` → `bool` | Check if a save file exists on disk. | 检查磁盘上是否存在存档。 |
| `DeleteSaveFromDisk(FString, int32)` | BlueprintCallable | `SlotName`, `UserIndex` → `bool` | Delete the save file from disk. | 删除磁盘上的存档。 |

#### Properties — Persistence / 持久化属性

| Property | Type | Default | Description (EN) | 说明 (CN) |
|----------|------|---------|-------------------|------------|
| `bAutoLoadOnBeginPlay` | `bool` | `false` | Auto-load presets from disk on BeginPlay. | BeginPlay 时自动从磁盘加载预设。 |
| `bAutoSaveOnEndPlay` | `bool` | `false` | Auto-save presets to disk on EndPlay. | EndPlay 时自动保存预设到磁盘。 |
| `AutoSaveSlotName` | `FString` | `"CrosshairPresets"` | Slot name for auto save/load. | 自动保存/加载的槽位名。 |

---

### 1.2 UHelsincyCrosshairRenderLibrary (Bridge API)

> **Header**: `Library/HelsincyCrosshairRenderLibrary.h`  
> **Parent / 父类**: `UBlueprintFunctionLibrary`  
> Static utility for non-intrusive HUD integration. Call from any HUD's DrawHUD.  
> 无侵入 HUD 集成的静态工具库。在任意 HUD 的 DrawHUD 中调用。

| Method | Specifier | Params | Return | Description (EN) | 说明 (CN) |
|--------|-----------|--------|--------|-------------------|-----------|
| `DrawCrosshairForHUD(AHUD*)` | BlueprintCallable | `HUD` | `bool` | Draw crosshair using the HUD's PlayerController and Canvas. **Recommended entry point.** | 使用 HUD 的 Controller 和 Canvas 绘制准心。**推荐入口。** |
| `DrawCrosshairForController(APlayerController*, UCanvas*)` | BlueprintCallable | `PlayerController`, `Canvas` | `bool` | Low-level bridge. Requires valid Canvas context. | 底层桥接入口。需在有效 Canvas 上下文中调用。 |
| `IsCrosshairDebugEnabled()` | BlueprintPure | — | `bool` | Query if crosshair debug output is active. | 查询准心调试输出是否开启。 |

---

### 1.3 AHelsincyHUD

> **Header**: `HUD/HelsincyHUD.h`  
> **Parent / 父类**: `AHUD`  
> Legacy HUD class. Automatically draws crosshair, center dot, hit markers, and runtime damage indicators through the DamageIndicator Bridge.
> 传统 HUD 类。通过 DamageIndicator Bridge 自动绘制准心、中心点、命中标记和运行时伤害指示器。

| Override | Description (EN) | 说明 (CN) |
|----------|-------------------|-----------|
| `DrawHUD()` | Full crosshair rendering pipeline. | 完整准心绘制流水线。 |
| `ShowDebugInfo(float& YL, float& YPos)` | Debug panels via `showdebug Crosshair` / `showdebug DamageIndicator`. | 通过控制台命令 `showdebug Crosshair` / `showdebug DamageIndicator` 显示调试面板。 |

| Debug Constants | Type | Value | Description (EN) | 说明 (CN) |
|-----------------|------|-------|-------------------|-----------|
| `DebugColor_CenterRef` | `static const FLinearColor` | (0, 1, 1, 0.3) | Center reference cross color. | 中心参考十字颜色。 |
| `DebugColor_SpreadBox` | `static const FLinearColor` | (1, 1, 0, 0.5) | Spread visualization box color. | 扩散可视化框颜色。 |
| `DebugColor_DIDirectionRay` | `static const FLinearColor` | (1, 0.5, 0, 0.6) | Damage indicator direction ray color. | 伤害指示器方向射线颜色。 |

---

### 1.4 UHelsincyCrosshairManagerSubsystem

> **Header**: `Subsystems/HelsincyCrosshairManagerSubsystem.h`  
> **Parent / 父类**: `UEngineSubsystem`  
> Singleton that manages renderer registration, lookup, and shared resources.  
> 管理渲染器注册、查询及共享资源的全局单例。

| Method | Return | Description (EN) | 说明 (CN) |
|--------|--------|-------------------|-----------|
| `GetRenderer(FGameplayTag ShapeTag)` | `UHelsincyShapeRenderer*` | Look up a renderer by ShapeTag. Returns nullptr if unregistered. | 根据 ShapeTag 查找渲染器。未注册返回 nullptr。 |
| `RegisterRenderer(FGameplayTag, TSubclassOf<UHelsincyShapeRenderer>)` | `bool` | Register a custom renderer at runtime. C++ custom renderers call this manually. | 运行时注册自定义渲染器。C++ 自定义渲染器需要手动调用。 |
| `GetRegisteredTags()` | `TArray<FGameplayTag>` | List all registered shape tags. Used by debug panel. | 列出所有已注册的形状 Tag。供调试面板使用。 |
| `GetSmoothLineTexture()` | `UTexture2D*` | Get the shared anti-aliased line texture. | 获取共享的抗锯齿线条纹理。 |
| `IsAsyncLoading()` | `bool` | Check if blueprint renderer classes are still loading. | 检查蓝图渲染器类是否仍在异步加载。 |

---

### 1.5 UHelsincyCrosshairDataAsset

> **Header**: `DataAssets/HelsincyCrosshairDataAsset.h`  
> **Parent / 父类**: `UPrimaryDataAsset`  
> A data-driven container for a complete crosshair configuration.  
> 完整准心配置的数据驱动容器。

| Property | Type | Description (EN) | 说明 (CN) |
|----------|------|-------------------|-----------|
| `DisplayName` | `FText` | Optional human-readable name. | 可选的可读名称。 |
| `PreviewIcon` | `UTexture2D*` | Optional preview thumbnail for UI. | 可选的预览缩略图。 |
| `Profile` | `FHelsincyCrosshairProfile` | The full crosshair profile. | 完整准心配置。 |

---

### 1.6 UHelsincyShapeRenderer

> **Header**: `Render/HelsincyShapeRenderer.h`  
> **Parent / 父类**: `UObject` (`Abstract, Blueprintable`)  
> Base class for all crosshair shape renderers. Subclass to create custom shapes.  
> 准心形状渲染器的抽象基类。继承此类可创建自定义形状。

| Property/Method | Description (EN) | 说明 (CN) |
|-----------------|-------------------|-----------|
| `AssociatedTag` | `FGameplayTag` — Tag used during built-in or settings-based registration. Set in constructor/CDO. | 内置或项目设置注册时使用的 Tag。在构造函数/CDO 中设置。 |
| `Draw(UCanvas*, Profile, Spread, Center, Color, DeltaTime, Scale)` | Virtual. Override in C++ subclass. Default calls `ReceiveDraw`. | 虚函数。C++ 子类覆盖。默认调用蓝图事件。 |
| `ReceiveDraw(...)` | BlueprintImplementableEvent. Override in Blueprint subclass. | 蓝图可实现事件。蓝图子类中实现。 |
| `RenderWithOutline(Visuals, Color, Scale, Callback)` | Protected helper. Auto-handles outline rendering based on config. | Protected 辅助。根据配置自动处理描边。 |

**Built-in Renderers / 内置渲染器** (10):

| Class | Tag | Shape (EN) | 形状 (CN) |
|-------|-----|------------|-----------|
| `UHelsincyRendererCross` | `Crosshair.Shape.Cross` | Classic cross | 经典十字 |
| `UHelsincyRendererCircle` | `Crosshair.Shape.Circle` | Circle | 圆形 |
| `UHelsincyRendererDotOnly` | `Crosshair.Shape.DotOnly` | Dot only | 仅圆点 |
| `UHelsincyRendererTStyle` | `Crosshair.Shape.TStyle` | T-style | T 形 |
| `UHelsincyRendererTriangle` | `Crosshair.Shape.Triangle` | Triangle | 三角形 |
| `UHelsincyRendererRectangle` | `Crosshair.Shape.Rectangle` | Rectangle | 矩形 |
| `UHelsincyRendererChevron` | `Crosshair.Shape.Chevron` | Chevron / V-shape | V 字形 |
| `UHelsincyRendererPolygon` | `Crosshair.Shape.Polygon` | Polygon (hexagon etc.) | 多边形 |
| `UHelsincyRendererWings` | `Crosshair.Shape.Wings` | Wings / trapezoid | 双翼梯形 |
| `UHelsincyRendererImage` | `Crosshair.Shape.Image` | Custom image | 自定义图片 |

---

### 1.7 UHelsincyCrosshairSettings

> **Header**: `Settings/HelsincyCrosshairSettings.h`  
> **Parent / 父类**: `UDeveloperSettings`  
> Project Settings entry: **Project Settings → HelsincyCrosshairSystem**  
> 项目设置入口：**项目设置 → HelsincyCrosshairSystem**

| Property | Type | Description (EN) | 说明 (CN) |
|----------|------|-------------------|-----------|
| `BlueprintRenderers` | `TArray<TSoftClassPtr<UHelsincyShapeRenderer>>` | Blueprint renderer classes to register on startup. | 启动时注册的蓝图渲染器类列表。 |

> Registration model / 注册模型：Blueprint renderers listed in `BlueprintRenderers` are async-loaded by the subsystem and registered automatically from their CDO `AssociatedTag`. C++ custom renderers are not globally scanned; register them explicitly with `RegisterRenderer()` from module/project initialization code.
>
> `BlueprintRenderers` 中配置的蓝图渲染器会由 Subsystem 异步加载，并根据 CDO 上的 `AssociatedTag` 自动注册。C++ 自定义渲染器不会全局扫描，需要在模块或项目初始化逻辑中显式调用 `RegisterRenderer()`。

---

### 1.8 IHelsincyCrosshairInterface

> **Header**: `Interfaces/HelsincyCrosshairInterface.h`  
> Optional interface for retrieving the crosshair component from an actor.  
> 可选接口，用于从 Actor 上获取准心组件。

| Method | Return | Description (EN) | 说明 (CN) |
|--------|--------|-------------------|-----------|
| `GetHelsincyCrosshairComponent()` | `UHelsincyCrosshairComponent*` | BlueprintCallable, BlueprintNativeEvent. Implement on your character. | 在角色上实现以暴露组件。蓝图可调用、原生事件。 |

---

### 1.9 IHelsincyTargetInterface

> **Header**: `Interfaces/HelsincyTargetInterface.h`  
> Interface for interactive objects to expose an interaction tag to the crosshair.  
> 可交互对象实现此接口，向准心暴露交互 Tag。

| Method | Return | Description (EN) | 说明 (CN) |
|--------|--------|-------------------|-----------|
| `GetCrosshairInteractionTag()` | `FGameplayTag` | BlueprintCallable, BlueprintNativeEvent, const. Return `EmptyTag` to not change color. | 返回交互 Tag。返回空 Tag 则不改变颜色。 |

---

## 2. Module: HelsincyDamageIndicator / 伤害指示器模块

### 2.1 UHelsincyDamageIndicatorComponent

> **Header**: `Components/HelsincyDamageIndicatorComponent.h`  
> **Parent / 父类**: `UActorComponent`  
> Manages damage indicator lifecycle and state. Attach to player Pawn/Character.  
> 管理伤害指示器的生命周期与状态。挂载到玩家 Pawn/Character 上。

| Property | Type | Category | Description (EN) | 说明 (CN) |
|----------|------|----------|-------------------|-----------|
| `IndicatorProfile` | `FHelsincy_DamageIndicatorProfile` | HelsincyDamageIndicator | The visual configuration for damage indicators. | 伤害指示器视觉配置。 |
| `DefaultDataAsset` | `UHelsincyDamageIndicatorDataAsset*` | HelsincyDamageIndicator\|Defaults | Optional default DataAsset loaded during local initialization. | 本地初始化阶段可自动加载的默认数据资产。 |
| `bUseDefaultDataAssetInit` | `bool` | HelsincyDamageIndicator\|Defaults | If true, auto-load `DefaultDataAsset` when available. Default: `true`. | 为 true 且资产有效时自动加载 `DefaultDataAsset`。默认 `true`。 |
| `bAutoLoadOnBeginPlay` | `bool` | HelsincyDamageIndicator\|Persistence | Auto-load preset library from disk during initialization. Default: `false`. | 初始化时自动从磁盘加载预设库。默认 `false`。 |
| `bAutoSaveOnEndPlay` | `bool` | HelsincyDamageIndicator\|Persistence | Auto-save preset library on EndPlay. Default: `false`. | EndPlay 时自动保存预设库。默认 `false`。 |
| `AutoSaveSlotName` | `FString` | HelsincyDamageIndicator\|Persistence | Save/load slot name. Default: `"DamageIndicatorPresets"`. | 自动保存/加载槽位名。默认 `"DamageIndicatorPresets"`。 |

| Method | Specifier | Params / Return | Description (EN) | 说明 (CN) |
|--------|-----------|-----------------|-------------------|-----------|
| `RegisterDamageEvent(AActor*, FVector)` | BlueprintCallable | `DamageCauser`, `LocationIfNoActor` | Register an incoming damage event. Call from TakeDamage. | 注册受伤事件。在 TakeDamage 中调用。 |
| `GetIndicatorProfile()` | BlueprintPure | → `const FHelsincy_DamageIndicatorProfile&` | Get the current indicator configuration. | 获取当前指示器配置。 |
| `SetIndicatorProfile(...)` | BlueprintCallable | `NewProfile` | Replace the indicator configuration at runtime. | 运行时替换指示器配置。 |
| `IsEnabled()` | BlueprintPure | → `bool` | Check if `IndicatorProfile.bEnabled` is true. | 检查指示器是否启用。 |
| `GetActiveIndicators()` | C++ only | → `const TArray<FHelsincy_ActiveDamageIndicator>&` | Active damage indicator instances for rendering. | 活跃指示器列表，供渲染使用。 |
| `LoadFromDataAsset(DataAsset)` | BlueprintCallable | `UHelsincyDamageIndicatorDataAsset*` | Load configuration from a data asset. | 从数据资产加载配置。 |
| `SavePreset(FName)` | BlueprintCallable | `PresetName` | Save current config as a named preset in memory. | 将当前配置保存为内存预设。 |
| `LoadPreset(FName)` | BlueprintCallable | `PresetName` → `bool` | Load a named preset from the in-memory library. | 从内存预设库加载。 |
| `GetAllPresetNames()` | BlueprintPure | → `TArray<FName>` | List all preset names in the in-memory library. | 列出内存预设库中所有名称。 |
| `ClearPresetLibrary()` | BlueprintCallable | — | Clear all in-memory presets. | 清空内存预设库。 |
| `SavePresetsToDisk(FString, int32)` | BlueprintCallable | `SlotName` (default "DamageIndicatorPresets"), `UserIndex` → `bool` | Persist preset library to disk via SaveGame. | 将预设库持久化到磁盘。 |
| `LoadPresetsFromDisk(FString, int32)` | BlueprintCallable | `SlotName`, `UserIndex` → `bool` | Load preset library from disk. Auto-applies last active preset. | 从磁盘加载预设库。自动应用上次活跃预设。 |
| `DoesSaveExist(FString, int32)` | BlueprintPure | `SlotName`, `UserIndex` → `bool` | Check if a save exists on disk. | 检查磁盘存档是否存在。 |
| `DeleteSaveFromDisk(FString, int32)` | BlueprintCallable | `SlotName`, `UserIndex` → `bool` | Delete a save from disk. | 删除磁盘存档。 |

---

### 2.2 UHelsincyDamageIndicatorRenderLibrary (Bridge API)

> **Header**: `Library/HelsincyDamageIndicatorRenderLibrary.h`  
> **Parent / 父类**: `UBlueprintFunctionLibrary`  
> Static utility for non-intrusive damage indicator integration.  
> 无侵入伤害指示器集成的静态工具库。
> Custom HUDs should call these functions manually from `AHUD::DrawHUD` or an equivalent valid Canvas draw phase; built-in `AHelsincyHUD` already calls `DrawDamageIndicatorsForHUD()` during `DrawHUD()`.
> 自定义 HUD 需要在 `AHUD::DrawHUD` 或等价的有效 Canvas 绘制阶段手动调用；内置 `AHelsincyHUD` 已在 `DrawHUD()` 中调用 `DrawDamageIndicatorsForHUD()`。

| Method | Specifier | Params | Return | Description (EN) | 说明 (CN) |
|--------|-----------|--------|--------|-------------------|-----------|
| `DrawDamageIndicatorsForHUD(AHUD*)` | BlueprintCallable | `HUD` | `bool` | Draw damage indicators using the HUD's context. **Recommended entry point.** | 使用 HUD 上下文绘制伤害指示器。**推荐入口。** |
| `DrawDamageIndicatorsForController(APlayerController*, UCanvas*)` | BlueprintCallable | `PlayerController`, `Canvas` | `bool` | Low-level bridge. | 底层桥接入口。 |
| `IsDamageIndicatorDebugEnabled()` | BlueprintPure | — | `bool` | Query if DI debug output is active. | 查询伤害指示器调试是否开启。 |

---

### 2.3 UHelsincyDamageIndicatorSubsystem

> **Header**: `Subsystems/HelsincyDamageIndicatorSubsystem.h`  
> **Parent / 父类**: `UEngineSubsystem`  
> Singleton that manages indicator renderer registration and shared resources.  
> 管理指示器渲染器注册及共享资源的全局单例。

| Method | Return | Description (EN) | 说明 (CN) |
|--------|--------|-------------------|-----------|
| `GetIndicatorRenderer(FGameplayTag StyleTag)` | `UHelsincyIndicatorRenderer*` | Look up a renderer by style tag. | 根据样式 Tag 查找渲染器。 |
| `RegisterDamageIndicatorRenderer(FGameplayTag, TSubclassOf<UHelsincyIndicatorRenderer>)` | `bool` | Register a custom indicator renderer. C++ custom renderers call this manually. | 注册自定义指示器渲染器。C++ 自定义渲染器需要手动调用。 |
| `GetRegisteredTags()` | `TArray<FGameplayTag>` | List all registered indicator style tags. | 列出所有已注册的指示器样式 Tag。 |
| `GetSmoothLineTexture()` | `UTexture2D*` | Get the shared anti-aliased line texture. | 获取共享抗锯齿线条纹理。 |
| `IsAsyncLoading()` | `bool` | Check if blueprint renderer classes are still loading. | 检查蓝图渲染器类是否仍在异步加载。 |

---

### 2.4 UHelsincyIndicatorRenderer

> **Header**: `IndicatorRenderer/HelsincyIndicatorRenderer.h`  
> **Parent / 父类**: `UObject` (`Abstract, Blueprintable`)  
> Base class for damage indicator pointer renderers.  
> 伤害指示器指针渲染器的抽象基类。

| Property/Method | Description (EN) | 说明 (CN) |
|-----------------|-------------------|-----------|
| `AssociatedTag` | `FGameplayTag` — Tag used during built-in or settings-based registration. Categories: `Indicator.Style`. | 内置或项目设置注册时使用的 Tag。类别：`Indicator.Style`。 |
| `DrawPointer(Canvas, Profile, Center, Angle, Alpha, Scale)` | Virtual. Override in C++ to implement custom rendering. | 虚函数。C++ 子类覆盖。 |
| `ReceiveDrawPointer(...)` | BlueprintImplementableEvent. Override in Blueprint subclass. | 蓝图可实现事件。蓝图子类中实现。 |
| `GetDesiredPointerSize(Profile, Scale)` | C++ V2. Reports pointer bounds for placement resolution. | C++ V2。为放置解析报告指示器外接尺寸。 |
| `DrawPointerResolved(Canvas, Profile, Placement, Alpha, Scale)` | C++ V2 draw path for resolved placement. | C++ V2 resolved placement 绘制路径。 |

> Compatibility / 兼容性：`DrawPointer` and Blueprint `ReceiveDrawPointer` remain available for existing custom renderers and keep their radial fallback behavior. C++ renderers that need true `WindowEdge` placement should override `GetDesiredPointerSize()` and `DrawPointerResolved()`.
>
> `DrawPointer` 与蓝图 `ReceiveDrawPointer` 会继续保留，现有自定义 renderer 仍可编译并保持 radial fallback 行为。需要真实 `WindowEdge` 放置的 C++ renderer 应覆盖 `GetDesiredPointerSize()` 与 `DrawPointerResolved()`。

**Built-in Indicator Renderers / 内置指示器渲染器** (3):

| Class | Tag | Style (EN) | 样式 (CN) |
|-------|-----|------------|-----------|
| `UHelsincyIndicatorRendererArrow` | `Indicator.Style.Arrow` | Triangular arrow | 三角箭头 |
| `UHelsincyIndicatorRendererImage` | `Indicator.Style.Image` | Custom image | 自定义图片 |
| `UHelsincyIndicatorRendererArc` | `Indicator.Style.Arc` | COD-like red arc | 类 COD 红色弧光 |

---

### 2.5 UHelsincyDamageIndicatorDataAsset

> **Header**: `DataAssets/HelsincyDamageIndicatorDataAsset.h`  
> **Parent / 父类**: `UPrimaryDataAsset`  
> A data-driven container for a complete damage indicator configuration. Analogous to `UHelsincyCrosshairDataAsset`.  
> 完整伤害指示器配置的数据驱动容器。与 `UHelsincyCrosshairDataAsset` 对齐。

| Property | Type | Description (EN) | 说明 (CN) |
|----------|------|-------------------|-----------|
| `DisplayName` | `FText` | Optional human-readable name. | 可选的可读名称。 |
| `PreviewIcon` | `UTexture2D*` | Optional preview thumbnail for UI. | 可选的预览缩略图。 |
| `Profile` | `FHelsincy_DamageIndicatorProfile` | The full damage indicator profile. | 完整伤害指示器配置。 |

---

### 2.6 UHelsincyDamageIndicatorSaveGame

> **Header**: `SaveGame/HelsincyDamageIndicatorSaveGame.h`  
> **Parent / 父类**: `USaveGame`  
> Persists the damage indicator preset library to disk. Default slot: `"DamageIndicatorPresets"`.  
> 将伤害指示器预设库持久化到磁盘。默认槽位：`"DamageIndicatorPresets"`。

| Property | Type | Description (EN) | 说明 (CN) |
|----------|------|-------------------|-----------|
| `SaveVersion` | `int32` | Data version for future migration. | 数据版本号，用于未来迁移。 |
| `SavedPresets` | `TMap<FName, FHelsincy_DamageIndicatorProfile>` | Name → full config mapping. | 名称 → 配置映射。 |
| `LastActivePresetName` | `FName` | Last used preset (auto-applied on load). | 上次使用的预设（加载时自动应用）。 |

| Method | Description (EN) | 说明 (CN) |
|--------|-------------------|-----------|
| `MigrateData()` | Incremental version migration. | 增量版本迁移。 |

---

### 2.7 UHelsincyDamageIndicatorSettings

> **Header**: `Settings/HelsincyDamageIndicatorSettings.h`
> **Parent / 父类**: `UDeveloperSettings`
> Project Settings entry: **Project Settings → HelsincyDamageIndicatorSystem**
> 项目设置入口：**Project Settings → HelsincyDamageIndicatorSystem**

| Property | Type | Description (EN) | 说明 (CN) |
|----------|------|-------------------|-----------|
| `BlueprintIndicatorRenderers` | `TArray<TSoftClassPtr<UHelsincyIndicatorRenderer>>` | Blueprint indicator renderer classes to register on startup. | 启动时注册的蓝图指示器渲染器类列表。 |

> Registration model / 注册模型：Blueprint indicator renderers listed here are async-loaded by `UHelsincyDamageIndicatorSubsystem` and registered automatically from their CDO `AssociatedTag`. C++ custom indicator renderers are not globally scanned; register them explicitly with `RegisterDamageIndicatorRenderer()`.
>
> 这里配置的蓝图指示器渲染器会由 `UHelsincyDamageIndicatorSubsystem` 异步加载，并根据 CDO 上的 `AssociatedTag` 自动注册。C++ 自定义指示器渲染器不会全局扫描，需要显式调用 `RegisterDamageIndicatorRenderer()`。

---

## 3. Data Structures / 数据结构

### 3.1 FHelsincyCrosshairProfile

> The root configuration struct. Contains all visual, dynamic, and shape-specific settings.  
> 根配置结构体。包含所有视觉、动态、形状特定设置。

| Field | Type | Description (EN) | 说明 (CN) |
|-------|------|-------------------|-----------|
| `ShapeTag` | `FGameplayTag` | Selects which shape renderer to use. Categories: `Crosshair.Shape`. | 选择使用哪个形状渲染器。 |
| `VisualsConfig` | `FHelsincy_VisualSettings` | Color, outline, targeting settings. | 颜色、描边、目标识别设置。 |
| `DynamicConfig` | `FHelsincy_DynamicsSettings` | Spread, recoil, movement dynamics. | 扩散、后坐力、移动动态设置。 |
| `CrosshairConfig` | `FHelsincy_CrosshairConfig` | Cross / TStyle / Chevron shape params. | 十字 / T形 / V形 参数。 |
| `RadialConfig` | `FHelsincy_RadialConfig` | Circle / Polygon shape params. | 圆形 / 多边形参数。 |
| `BoxConfig` | `FHelsincy_BoxConfig` | Rectangle / Triangle shape params. | 矩形 / 三角形参数。 |
| `WingsConfig` | `FHelsincy_WingsConfig` | Wings/trapezoid shape params. | 双翼梯形参数。 |
| `ImageConfig` | `FHelsincy_ImageConfig` | Custom image params. | 自定义图片参数。 |
| `CenterDotConfig` | `FHelsincy_CenterDotConfig` | Center dot toggle and style. | 中心点开关与样式。 |
| `HitMarkerConfig` | `FHelsincy_HitMarkerProfile` | Hit marker animation, colors, shake. | 命中标记动画、颜色、震动。 |

---

### 3.2 FHelsincy_VisualSettings

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `PrimaryColor` | `FLinearColor` | White | Base crosshair color. | 基础准心颜色。 |
| `GlobalCrosshairOffset` | `FVector2D` | (0, 0) | Offset from screen center (px). X=right, Y=down. | 相对屏幕中心的偏移（像素）。 |
| `Thickness` | `float` | 2.0 | Line thickness (px). Range: 1–30. | 线条粗细。 |
| `Opacity` | `float` | 1.0 | Global opacity. Range: 0–1. | 全局不透明度。 |
| `bEnableOutline` | `bool` | true | Enable outline around crosshair lines. | 启用准心描边。 |
| `OutlineColor` | `FLinearColor` | Black | Outline color. | 描边颜色。 |
| `OutlineThickness` | `float` | 1.0 | Extra outline width (px). Range: 1–20. | 额外描边宽度。 |
| `bEnableTargetSwitching` | `bool` | true | Enable target-based color switching. | 启用目标变色。 |
| `MaxSwitchingDistance` | `float` | 10000 | Max trace distance (cm). Range: 5000–30000. | 最大检测距离 (cm)。 |
| `SwitchingChannel` | `ECollisionChannel` | ECC_Pawn | Trace channel for target detection. | 目标检测碰撞通道。 |
| `EnemyColor` | `FLinearColor` | Red | Color when targeting hostile. | 瞄准敌人时的颜色。 |
| `FriendlyColor` | `FLinearColor` | Green | Color when targeting friendly. | 瞄准友军时的颜色。 |
| `bUseNeutralColor` | `bool` | false | Use a specific neutral color instead of PrimaryColor. | 是否使用特定的中立颜色。 |
| `NeutralColor` | `FLinearColor` | Yellow | Neutral target color. | 中立目标颜色。 |
| `InteractionColorMap` | `TMap<FGameplayTag, FLinearColor>` | Empty | Tag-to-color mapping for interactive objects. | 交互对象的 Tag→颜色映射表。 |
| `bUseDefaultInteractionColor` | `bool` | true | Use a fallback color for unmatched interaction tags. | 对未匹配 Tag 使用默认交互色。 |
| `DefaultInteractionColor` | `FLinearColor` | Yellow | Fallback interaction highlight color. | 默认交互高亮色。 |

---

### 3.3 FHelsincy_DynamicsSettings

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `bEnableDynamicSpread` | `bool` | true | Master switch for spread dynamics. | 动态扩散总开关。 |
| `VelocityMultiplier` | `float` | 0.05 | Movement speed → spread factor. Range: 0.01–0.5. | 移动速度→扩散系数。 |
| `AirbornePenalty` | `float` | 50.0 | Fixed spread penalty while airborne. Range: 5–100. | 滞空时的固定扩散惩罚。 |
| `JumpExpandSpeed` | `float` | 20.0 | Jump-start expansion speed. Range: 5–30. | 起跳扩散速度。 |
| `LandRecoverySpeed` | `float` | 10.0 | Landing recovery speed. Range: 5–30. | 落地回收速度。 |
| `bApplyMovementToX` | `bool` | true | Apply movement spread on horizontal axis. | 水平轴应用移动扩散。 |
| `bApplyMovementToY` | `bool` | true | Apply movement spread on vertical axis. | 垂直轴应用移动扩散。 |
| `RecoilRecoverySpeed` | `float` | 10.0 | Recoil recovery speed. Higher = faster recovery. Range: 5–30. | 后坐力恢复速度。越大恢复越快。 |
| `MaxRecoilSpread` | `float` | 100.0 | Max recoil spread cap (px). Range: 10–100. | 最大后坐力限制（像素）。 |

---

### 3.4 FHelsincy_CrosshairConfig

> Used by: Cross, TStyle, Chevron  
> 适用：十字、T形、V形

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `Length` | `float` | 12.0 | Arm length (px). Range: 10–100. | 臂长（像素）。 |
| `Gap` | `float` | 6.0 | Center gap (px). Spread adds on top. Range: 0–100. | 中心间隙。扩散基于此增加。 |
| `Rotation` | `float` | 0.0 | Rotation (degrees). Range: 0–360. | 旋转（度）。 |
| `ChevronOpeningAngle` | `float` | 90.0 | V-shape opening angle. Chevron only. Range: 10–350. | V 形张开角度。仅 Chevron。 |

---

### 3.5 FHelsincy_RadialConfig

> Used by: Circle, Polygon  
> 适用：圆形、多边形

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `Radius` | `float` | 10.0 | Radius (px). Range: 10–100. | 半径（像素）。 |
| `Rotation` | `float` | 0.0 | Rotation (degrees). Range: 0–360. | 旋转（度）。 |
| `CircleSegments` | `int32` | 32 | Circle segment count. Range: 24–360. | 圆形分段数。 |
| `PolygonSides` | `int32` | 6 | Polygon side count. Range: 5–30. | 多边形边数。 |

---

### 3.6 FHelsincy_WingsConfig

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `bWingsAlignTop` | `bool` | false | Align top line to screen center. | 顶线对齐屏幕中心。 |
| `WingsVerticalOffset` | `float` | 0.0 | Vertical fine-tune offset (px). Range: ±150. | 垂直微调偏移。 |
| `WingsLineCount` | `int32` | 6 | Number of horizontal lines. Range: 2–20. | 横线数量。 |
| `WingsVerticalSpacing` | `float` | 6.0 | Spacing between lines (px). Range: 2–30. | 线间距。 |
| `WingsTopLength` | `float` | 20.0 | Top line half-length (px). Range: 15–100. | 顶线长度。 |
| `WingsBottomLength` | `float` | 5.0 | Bottom line half-length (px). Range: 5–100. | 底线长度。 |
| `bWingsDrawVerticalLines` | `bool` | true | Draw the vertical trunk lines. | 是否绘制垂直主干线。 |

---

### 3.7 FHelsincy_BoxConfig

> Used by: Rectangle, Triangle  
> 适用：矩形、三角形

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `Size` | `FVector2D` | (12, 8) | Width × Height (px). Range: 6–100. | 宽×高（像素）。 |
| `Rotation` | `float` | 0.0 | Rotation (degrees). Range: 0–360. | 旋转（度）。 |

---

### 3.8 FHelsincy_ImageConfig

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `Texture` | `UTexture2D*` | nullptr | The crosshair image texture. | 准心图片纹理。 |
| `Size` | `FVector2D` | (32, 32) | Display size (px). | 显示大小（像素）。 |
| `Tint` | `FLinearColor` | White | Color tint applied to the image. | 叠加色。 |

---

### 3.9 FHelsincy_CenterDotConfig

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `bEnabled` | `bool` | true | Show center dot. | 显示中心圆点。 |
| `bAlwaysStayCentered` | `bool` | true | Dot stays at screen center regardless of GlobalCrosshairOffset. | 圆点始终在屏幕正中心，不受全局偏移影响。 |
| `Size` | `float` | 2.0 | Dot radius (px). Range: 1–100. | 圆点半径。 |
| `Opacity` | `float` | 1.0 | Dot opacity. Range: 0–1. | 不透明度。 |
| `Color` | `FLinearColor` | White | Dot color. | 圆点颜色。 |

---

### 3.10 FHelsincy_HitMarkerProfile

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `bEnabled` | `bool` | true | Enable hit markers. | 启用命中标记。 |
| `Mode` | `EHitMarkerMode` | SingleInstance | HitMarker mode: `SingleInstance` (COD-style, one marker refreshed on each hit) or `MultiInstance` (classic, each hit may spawn a new marker). | HitMarker 模式：`SingleInstance`（COD 风格单实例刷新）或 `MultiInstance`（经典多实例）。 |
| `bUseTaperedShape` | `bool` | true | Use tapered (CoD-style) shape vs uniform lines. | 使用尖头（类 CoD）vs 等宽线条。 |
| `TaperedShapeGlowWidthScale` | `float` | 2.0 | Tapered glow width multiplier. Range: 1–5. | 尖头辉光宽度倍率。 |
| `TaperedShapeGlowAlphaScale` | `float` | 0.4 | Tapered glow alpha. Range: 0.1–0.9. | 尖头辉光透明度。 |
| `Duration` | `float` | 0.25 | Hit marker display time (sec). Range: 0.1–1.0. | 显示持续时间（秒）。 |
| `MergeThreshold` | `float` | 0.15 | Time threshold to merge sequential hits. Range: 0.1–0.3. | 连续命中合并阈值。 |
| `Color` | `FLinearColor` | White | Normal hit color. | 普通命中颜色。 |
| `HeadshotColor` | `FLinearColor` | Red | Headshot hit color. | 爆头命中颜色。 |
| `HeadShotScale` | `float` | 1.3 | Headshot size multiplier. Range: 1–5. | 爆头大小倍率。 |
| `KillColor` | `FLinearColor` | Red | Kill hit color. | 击杀命中颜色。 |
| `KillScale` | `float` | 1.7 | Kill size multiplier. Range: 1–5. | 击杀大小倍率。 |
| `bClearAllOldHitMarkerOnKill` | `bool` | true | Clear all non-kill markers when a kill marker triggers. | 触发击杀时清除所有非击杀标记。 |
| `Thickness` | `float` | 2.0 | Line thickness (px). Range: 1–20. | 线条粗细。 |
| `BaseDistance` | `float` | 8.0 | Distance from crosshair center (px). Range: 1–50. | 距准心中心距离。 |
| `StartSize` | `float` | 16.0 | Initial line length (px). Range: 10–100. | 初始线段长度。 |
| `EndSize` | `float` | 8.0 | Final line length (px). Range: 6–50. | 最终线段长度。 |
| `StartOffset` | `float` | 0.0 | Initial center offset (px). Range: 0–50. | 初始中心偏移。 |
| `EndOffset` | `float` | 12.0 | Final center offset (px). Range: 6–50. | 最终中心偏移。 |
| `ShakeIntensity` | `float` | 5.0 | Global shake intensity (px). Range: 0–30. | 全局震动强度。 |
| `NormalShakeIntensity` | `float` | 5.0 | Per-arm normal shake intensity (px). Range: 0–30. | 单臂法线震动强度。 |
| `ShakeFrequency` | `float` | 34.0 | Deterministic shake oscillation frequency. Range: 0–80. | 确定性震动频率。 |
| `ShakeDamping` | `float` | 12.0 | Deterministic shake damping. Range: 0–60. | 确定性震动阻尼。 |
| `HeadshotShakeMultiplier` | `float` | 1.25 | Extra shake energy multiplier for headshots. Range: 0–4. | 爆头震动能量倍率。 |
| `KillShakeMultiplier` | `float` | 1.5 | Extra shake energy multiplier for kills. Range: 0–5. | 击杀震动能量倍率。 |
| `bUseImpactMotion` | `bool` | true | Use COD-style twist/scale impact motion. | 启用 COD 风格扭转/缩放冲击运动。 |
| `ImpactRotationDegrees` | `float` | 6.0 | Base twist angle in degrees. Range: 0–24. | 基础扭转角度。 |
| `ImpactScalePunch` | `float` | 0.18 | Whole-marker impact scale amount. Range: 0–0.6. | 整体缩放冲击量。 |
| `ImpactArmLengthPunch` | `float` | 0.14 | Arm length expansion amount. Range: 0–0.6. | 臂长扩张冲击量。 |
| `ImpactTranslationWeight` | `float` | 0.25 | Secondary translation shake weight. Range: 0–1. | 辅助平移震动权重。 |
| `ImpactMotionDuration` | `float` | 0.18 | Main impact motion duration. Range: 0.05–0.40. | 主冲击运动时长。 |
| `DamageToImpactScale` | `float` | 0.35 | Damage influence on impact strength. Range: 0–1. | 伤害对冲击强度的影响。 |
| `MaxImpactMotionEnergy` | `float` | 1.8 | Max accumulated impact motion energy. Range: 0.25–4. | 最大冲击运动能量。 |
| `bShakeDecay` | `bool` | true | Shake decays over time. | 震动随时间衰减。 |
| `CustomTexture` | `UTexture2D*` | nullptr | Optional custom image. Null = procedural draw. | 可选自定义图片。空则程序化绘制。 |
| `HitPulseScale` | `float` | 1.2 | SingleInstance mode: scale pulse on hit (1.0 = no pulse). Range: 1–2. | 单实例模式: 命中时缩放脉冲强度。 |
| `HitPulseRecoverySpeed` | `float` | 15.0 | SingleInstance mode: pulse recovery interp speed. Range: 5–30. | 单实例模式: 脉冲恢复插值速度。 |
| `KillPulseScale` | `float` | 1.5 | SingleInstance mode: kill pulse scale (stronger than normal hit). Range: 1–3. | 单实例模式: 击杀脉冲缩放（大于普通命中）。 |
| `SingleInstanceSize` | `float` | 10.0 | SingleInstance fixed line size, replacing StartSize/EndSize animation. Range: 4–40. | 单实例固定线条大小，替代 StartSize/EndSize 动画。 |
| `SingleInstanceOffset` | `float` | 8.0 | SingleInstance fixed center distance, replacing StartOffset/EndOffset animation. Range: 2–30. | 单实例固定中心距离，替代 StartOffset/EndOffset 动画。 |
| `SingleInstanceRenderMode` | `EHelsincySingleHitMarkerRenderMode` | LegacyGeometry | SingleInstance render backend: procedural geometry or dual-layer sprites. | 单实例渲染后端：程序化几何或双层贴图。 |
| `SingleInstanceCoreTexture` | `UTexture2D*` | nullptr | Optional core sprite for `SpriteDualLayer`. | `SpriteDualLayer` 的可选核心贴图。 |
| `SingleInstanceGlowTexture` | `UTexture2D*` | nullptr | Optional glow sprite for `SpriteDualLayer`. | `SpriteDualLayer` 的可选辉光贴图。 |
| `SingleInstanceCoreScale` | `float` | 0.80 | Core sprite size multiplier. Range: 0.25–3.0. | 核心贴图尺寸倍率。 |
| `SingleInstanceGlowScale` | `float` | 0.86 | Glow sprite size multiplier. Range: 0.25–3.0. | 辉光贴图尺寸倍率。 |
| `SingleInstanceGlowOpacityScale` | `float` | 0.26 | Extra glow opacity multiplier. Range: 0–1. | 辉光额外透明度倍率。 |
| `SingleInstanceFadeRatio` | `float` | 0.3 | Fraction of total duration used for tail fade. Range: 0.1–0.8. | 总时长中用于尾部淡出的比例。 |
| `SingleInstanceMaxImpactEnergy` | `float` | 1.0 | Max cached impact energy for derived state. Range: 0.1–4.0. | 派生状态使用的最大命中能量。 |
| `SingleInstanceImpactDecaySpeed` | `float` | 8.0 | Impact energy decay speed. Range: 0.1–30. | 命中能量衰减速度。 |
| `SingleInstanceAccentDuration` | `float` | 0.08 | Duration of the hit accent phase. Range: 0.01–0.3. | 命中强调段持续时间。 |
| `bUseSingleInstanceVisualSeparation` | `bool` | true | Separate hitmarker readability from the base crosshair while active. | 激活时让命中标记和基础准星在视觉上拉开。 |
| `SingleInstanceSafeSpacingScale` | `float` | 1.18 | Push hitmarker outward for readability. Range: 1–2. | 为可读性将命中标记向外推的间距倍率。 |
| `SingleInstanceCrosshairAlphaScale` | `float` | 0.10 | Legacy compatibility alpha scale. When newer Crosshair Visibility fields remain at migration defaults and this legacy field is edited, it maps to runtime base crosshair alpha. Prefer `CrosshairVisibilityWhileActive` + `CrosshairAlphaScaleWhileHitMarkerActive` for new assets. | 旧兼容透明度缩放。新 Crosshair Visibility 字段仍为迁移默认值且此旧字段被编辑时，会映射到运行时基础准星透明度；新资产优先使用 `CrosshairVisibilityWhileActive` + `CrosshairAlphaScaleWhileHitMarkerActive`。 |
| `bHideCenterDotWhenSingleInstanceActive` | `bool` | true | Legacy compatibility center-dot behavior. When newer fields remain at migration defaults and this legacy field is edited, it controls whether the center dot is hidden while SingleInstance hitmarker is active. | 旧兼容中心点行为。新字段仍为迁移默认值且此旧字段被编辑时，控制单实例命中标记激活期间是否隐藏中心点。 |
| `SingleInstanceCenterDotAlphaScale` | `float` | 0.25 | Legacy compatibility center-dot alpha scale used when `bHideCenterDotWhenSingleInstanceActive` is false. Prefer the newer Crosshair Visibility policy for new assets. | 旧兼容中心点透明度缩放，仅在 `bHideCenterDotWhenSingleInstanceActive` 为 false 时使用；新资产优先使用新的 Crosshair Visibility 策略。 |

### 3.10.1 FHelsincy_SingleHitMarkerState

Runtime state struct for single-instance HitMarker (COD-style). Not a USTRUCT—used internally by `UHelsincyCrosshairComponent`.  
单实例 HitMarker 的运行时状态结构体（COD 风格）。非 USTRUCT，由组件内部使用。

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|------------|
| `bActive` | `bool` | false | Whether the marker lifecycle is active. | 当前生命周期是否激活。 |
| `bVisible` | `bool` | false | Whether the current derived phase should draw. | 当前派生相位是否需要绘制。 |
| `Phase` | `EHelsincySingleHitMarkerPhase` | Hidden | Current runtime phase. | 当前运行相位。 |
| `TimeRemaining` | `float` | 0.0 | Remaining display time. | 剩余显示时间。 |
| `TotalDuration` | `float` | 0.0 | Base duration reset on each hit. | 每次命中重置的基准时长。 |
| `HoldTimeRemaining` | `float` | 0.0 | Remaining active-hold time. | ActiveHold 阶段剩余时间。 |
| `TailFadeTimeRemaining` | `float` | 0.0 | Remaining tail-fade time. | TailFade 阶段剩余时间。 |
| `CurrentColor` | `FLinearColor` | White | Current display color. | 当前显示颜色。 |
| `Priority` | `EHitMarkerPriority` | Body | Current priority level. | 当前优先级。 |
| `BaseSizeScale` | `float` | 1.0 | Base size scale. | 基础缩放。 |
| `CurrentPulseScale` | `float` | 1.0 | Pulse scale (jumps on hit, interps back to 1.0). | 脉冲缩放。 |
| `Thickness` | `float` | 2.0 | Line thickness. | 线条粗细。 |
| `Opacity` | `float` | 0.0 | Cached opacity for the current phase. | 当前相位透明度缓存。 |
| `ImpactEnergy` | `float` | 0.0 | Cached impact energy. | 命中能量缓存。 |
| `AccentStrength` | `float` | 0.0 | Cached hit accent strength. | 命中强调强度缓存。 |
| `ShakeEnergy` | `float` | 0.0 | Cached shake energy. | 震动能量缓存。 |
| `ImpactMotionEnergy` | `float` | 0.0 | COD-style twist/scale impact motion energy. | COD 风格扭转/缩放冲击运动能量。 |
| `ImpactDamageScale` | `float` | 0.0 | Normalized damage scale. | 归一化伤害强度。 |
| `ImpactMotionSign` | `float` | 1.0 | Impact rotation direction (-1/1). | 冲击旋转方向（-1/1）。 |
| `ShakeSeed` | `int32` | 0 | Stable shake seed. | 稳定震动种子。 |
| `ShakeAge` | `float` | 0.0 | Current shake age. | 当前震动年龄。 |
| `ShakePhase` | `float` | 0.0 | Shake phase. | 震动相位。 |
| `ShakeDirection` | `FVector2D` | (1, 0) | Screen-space shake direction. | 屏幕空间震动方向。 |

**Methods / 方法:**

| Method | Description (EN) | 说明 (CN) |
|--------|-------------------|------------|
| `ApplyHit(Duration, Thickness, Color, Priority, SizeScale, PulseScale)` | Process a new hit event: refresh timer, upgrade priority if higher, trigger pulse. | 处理一次命中：刷新计时器，升级优先级，触发脉冲。 |
| `Tick(DeltaTime, PulseRecoverySpeed)` | Per-frame update: decay timer, interp pulse back to 1.0, deactivate when expired. | 每帧更新：衰减计时器，脉冲恢复，过期时停用。 |
| `RefreshDerivedState(FadeRatio, MaxImpactEnergy)` | Recompute visibility phase, opacity, and clamped impact/accent energy. | 重新计算可见相位、透明度和限制后的冲击/强调能量。 |
| `GetEffectiveScale()` | Returns `BaseSizeScale * CurrentPulseScale`. | 返回实际缩放 = 基础缩放 × 脉冲缩放。 |

---

### 3.11 FHelsincy_DamageIndicatorProfile

| Field | Type | Default | Description (EN) | 说明 (CN) |
|-------|------|---------|-------------------|-----------|
| `bEnabled` | `bool` | true | Master switch. | 总开关。 |
| `IndicatorStyleTag` | `FGameplayTag` | — | Select renderer style. Categories: `Indicator.Style`. | 选择渲染样式。 |
| `PlacementMode` | `EHelsincyDamageIndicatorPlacementMode` | RadialCircle | Select radial-circle or game-window-edge placement. | 选择圆环或游戏窗口边缘放置模式。 |
| `EdgeMargin` | `float` | 48.0 | WindowEdge safe margin in Canvas pixels. Range: 0-256. | WindowEdge 模式下相对 Canvas 边缘的安全边距。 |
| `EdgeCornerPadding` | `float` | 24.0 | Extra corner avoidance in WindowEdge mode. Range: 0-128. | WindowEdge 模式下的角落避让距离。 |
| `bHideCircleInWindowEdgeMode` | `bool` | true | Hide the radial circle when using WindowEdge placement. | 使用 WindowEdge 时默认隐藏圆环。 |
| `Duration` | `float` | 3.0 | Display duration (sec). Range: 1–30. | 显示持续时间。 |
| `CircleMaxOpacity` | `float` | 0.25 | Circle ring max opacity. Range: 0–1. | 圆环最大不透明度。 |
| `PointerMaxOpacity` | `float` | 1.0 | Pointer max opacity. Range: 0–1. | 指针最大不透明度。 |
| `FadeInTime` | `float` | 0.2 | Fade-in duration (sec). Range: 0.1–1.0. | 淡入时间。 |
| `FadeOutTime` | `float` | 0.5 | Fade-out duration (sec). Range: 0.1–1.0. | 淡出时间。 |
| `bShowCircle` | `bool` | true | Show background circle ring. | 显示背景圆环。 |
| `CircleSegments` | `int32` | 64 | Circle segment count. Range: 64–360. | 圆形分段数。 |
| `Radius` | `float` | 150.0 | Circle radius (px). Range: 50–300. | 圆环半径。 |
| `CircleColor` | `FLinearColor` | (1,1,1, 0.2) | Circle ring color. | 圆环颜色。 |
| `CircleThickness` | `float` | 2.0 | Circle ring thickness (px). Range: 2–30. | 圆环线条粗细。 |
| `bPointerOutsideCircle` | `bool` | true | Place pointer outside the circle ring. | 指针置于圆环外侧。 |
| `PointerDistanceOffset` | `float` | 5.0 | Spacing from circle (px). Range: 5–30. | 距圆环间距。 |
| `RotationInterpSpeed` | `float` | 10.0 | Pointer smooth rotation speed. Range: 5–30. | 指针平滑旋转速度。 |
| `ArrowConfig` | `FHelsincy_Ind_ArrowConfig` | — | Arrow-specific: Size, Color. | 箭头专属配置。 |
| `ImageConfig` | `FHelsincy_Ind_ImageConfig` | — | Image-specific: Texture, Size, Tint, bRotateImage, RotationOffset. | 图片专属配置。 |
| `ArcConfig` | `FHelsincy_Ind_ArcConfig` | — | Arc-specific: ArcMaskTexture, Size, Color, Glow, DirectionCueMode. | 弧形专属配置：弧光贴图、尺寸、颜色、柔光、方向提示模式。 |

---

### 3.11.1 Arc Damage Indicator Style

`Indicator.Style.Arc` draws a COD-like red arc indicator. It uses `ArcConfig.ArcMaskTexture` when provided; otherwise the built-in renderer generates a default alpha mask at runtime. Direction readability is controlled by `ArcConfig.DirectionCueMode`, with `CenterNib` as the recommended default.

Key fields:

| Field | Type | Recommended / Default | Description |
|-------|------|-----------------------|-------------|
| `ArcConfig.ArcMaskTexture` | `UTexture2D*` | `nullptr` | Optional user arc mask. Null uses the generated default mask. |
| `ArcConfig.Size` | `FVector2D` | `(220, 80)` | Draw size of the arc layer. |
| `ArcConfig.Color` | `FLinearColor` | Red | Tint color for the arc and cue. |
| `ArcConfig.DirectionCueMode` | `EHelsincyDamageIndicatorArcDirectionCueMode` | `CenterNib` | Internal Arc cue mode: `None`, `CenterChevron`, `AsymmetricTaper`, or `CenterNib`. |
| `ArcConfig.DirectionCueStrength` | `float` | `1.0` | Multiplier for procedural cue visibility/size. |
| `ArcConfig.CueSize` | `FVector2D` | `(18, 14)` | Base size of the procedural direction cue. |

---

## 4. Enums / 枚举

### EHelsincyDamageIndicatorPlacementMode

| Value | Meaning (EN) | 说明 (CN) |
|---|---|---|
| `RadialCircle` | Keep the existing center/radius placement. | 保持当前中心圆环半径放置方式。 |
| `WindowEdge` | Place pointers against the current game Canvas/window safe frame (`Canvas->ClipX / ClipY`), not physical monitor bounds. | 将指示器放在当前游戏 Canvas/窗口安全边界（`Canvas->ClipX / ClipY`）上，不使用显示器物理边界。 |

### EHitMarkerMode

| Value | Display Name (EN) | 说明 (CN) |
|-------|-------------------|------------|
| `MultiInstance` (0) | Multi-Instance (Classic) | 多实例模式（经典）：每次命中可创建新的 HitMarker 实例。 |
| `SingleInstance` (1) | Single-Instance (COD-Style) | 单实例模式（COD 风格）：始终只有一个 HitMarker，命中时刷新状态。 |

### EHelsincySingleHitMarkerRenderMode

| Value | Display Name (EN) | 说明 (CN) |
|-------|-------------------|------------|
| `LegacyGeometry` | Legacy Geometry | 使用程序化几何绘制单实例 HitMarker。 |
| `SpriteDualLayer` | Sprite Dual-Layer | 使用 Core + Glow 双层贴图绘制单实例 HitMarker。 |

### EHelsincySingleHitMarkerPhase

| Value | Display Name (EN) | 说明 (CN) |
|-------|-------------------|------------|
| `Hidden` | Hidden | 不可见状态。 |
| `ActiveHold` | Active Hold | 激活保持阶段。 |
| `TailFade` | Tail Fade | 尾部淡出阶段。 |

### EHitMarkerPriority

| Value | Display Name (EN) | 说明 (CN) |
|-------|-------------------|-----------|
| `Low_Priority_Body` (0) | Body Shot | 身体命中 |
| `Medium_Priority_Head` (1) | Head Shot | 爆头命中 |
| `High_Priority_Kill` (2) | Kill | 击杀命中 |

---

## 5. GameplayTags / 标签常量

### Crosshair Shape Tags / 准心形状标签

Defined in `FHelsincyCrosshair_Tags`. All under `Crosshair.Shape.*`:  
定义于 `FHelsincyCrosshair_Tags`，全部位于 `Crosshair.Shape.*` 下：

| C++ Constant | Tag String | Renderer |
|-------------|------------|----------|
| `Shape_Cross` | `Crosshair.Shape.Cross` | `UHelsincyRendererCross` |
| `Shape_Circle` | `Crosshair.Shape.Circle` | `UHelsincyRendererCircle` |
| `Shape_DotOnly` | `Crosshair.Shape.DotOnly` | `UHelsincyRendererDotOnly` |
| `Shape_Image` | `Crosshair.Shape.Image` | `UHelsincyRendererImage` |
| `Shape_TStyle` | `Crosshair.Shape.TStyle` | `UHelsincyRendererTStyle` |
| `Shape_Triangle` | `Crosshair.Shape.Triangle` | `UHelsincyRendererTriangle` |
| `Shape_Rectangle` | `Crosshair.Shape.Rectangle` | `UHelsincyRendererRectangle` |
| `Shape_Chevron` | `Crosshair.Shape.Chevron` | `UHelsincyRendererChevron` |
| `Shape_Polygon` | `Crosshair.Shape.Polygon` | `UHelsincyRendererPolygon` |
| `Shape_Wings` | `Crosshair.Shape.Wings` | `UHelsincyRendererWings` |

### Damage Indicator Style Tags / 伤害指示器样式标签

Defined in `FHelsincyDamageIndicator_Tags`. All under `Indicator.Style.*`:  
定义于 `FHelsincyDamageIndicator_Tags`，全部位于 `Indicator.Style.*` 下：

| C++ Constant | Tag String | Renderer |
|-------------|------------|----------|
| `Indicator_Style_Arrow` | `Indicator.Style.Arrow` | `UHelsincyIndicatorRendererArrow` |
| `Indicator_Style_Image` | `Indicator.Style.Image` | `UHelsincyIndicatorRendererImage` |
| `Indicator_Style_Arc` | `Indicator.Style.Arc` | `UHelsincyIndicatorRendererArc` |

---

## 6. Performance Stats / 性能统计

The plugin exposes UE4 `STAT` groups for runtime profiling. Use these console commands in development or test builds; repeat the command to toggle the overlay.

插件接入 UE4 `STAT` 性能统计系统。开发版或测试版运行时可使用以下控制台命令查看开销，再次输入同一命令可切换显示状态。

| Console Command | Stat Group | Description (EN) | 说明 (CN) |
|-----------------|------------|-------------------|-----------|
| `stat HelsincyCrosshair` | `STATGROUP_HelsincyCrosshair` | Crosshair component tick, HUD draw, Bridge draw, renderer draw, HitMarker draw, and target detection cost. | 准星组件 Tick、HUD 绘制、Bridge 绘制、Renderer 绘制、HitMarker 绘制和目标检测开销。 |
| `stat HelsincyDamageIndicator` | `STATGROUP_HelsincyDamageIndicator` | Damage indicator component tick, indicator state update, Bridge draw, and indicator rendering cost. | 伤害指示器组件 Tick、指示器状态更新、Bridge 绘制和指示器渲染开销。 |

### HelsincyCrosshair Stats

| C++ Stat | Display Name | Description (EN) | 说明 (CN) |
|----------|--------------|-------------------|-----------|
| `STAT_HC_CrosshairComponentTick` | Crosshair Component Tick | Per-frame crosshair state update, including spread, hitmarker state, and target detection scheduling. | 准星组件每帧状态更新，包括扩散、命中反馈状态和目标检测调度。 |
| `STAT_HC_HUDDraw` | Crosshair HUD Draw | `AHelsincyHUD::DrawHUD` crosshair path cost. | `AHelsincyHUD::DrawHUD` 准星路径开销。 |
| `STAT_HC_BridgeDraw` | Crosshair Bridge Draw | Bridge API crosshair draw entry cost. | Bridge API 准星绘制入口开销。 |
| `STAT_HC_RendererDraw` | Crosshair Renderer Draw | Current crosshair shape renderer draw cost. | 当前准星形状 Renderer 绘制开销。 |
| `STAT_HC_HitMarkerDraw` | HitMarker Draw | HitMarker draw cost. | 命中反馈绘制开销。 |
| `STAT_HC_TargetDetectionRequest` | Target Detection Request | Async target detection request cost. | 异步目标检测请求开销。 |
| `STAT_HC_TargetDetectionCallback` | Target Detection Callback | Async target detection callback processing cost. | 异步目标检测回调处理开销。 |

### HelsincyDamageIndicator Stats

| C++ Stat | Display Name | Description (EN) | 说明 (CN) |
|----------|--------------|-------------------|-----------|
| `STAT_HDI_ComponentTick` | DamageIndicator Component Tick | Per-frame damage indicator component update. | 伤害指示器组件每帧更新开销。 |
| `STAT_HDI_UpdateIndicators` | DamageIndicator Update Indicators | Active indicator lifetime, angle, and smoothing update cost. | 活跃指示器生命周期、角度和平滑状态更新开销。 |
| `STAT_HDI_BridgeDraw` | DamageIndicator Bridge Draw | Bridge API damage indicator draw entry cost. | Bridge API 伤害指示器绘制入口开销。 |
| `STAT_HDI_DrawIndicators` | DamageIndicator Draw Indicators | Circle, WindowEdge/RadialCircle placement, and concrete renderer draw cost. | 圆环、WindowEdge/RadialCircle 放置和具体 Renderer 绘制开销。 |
