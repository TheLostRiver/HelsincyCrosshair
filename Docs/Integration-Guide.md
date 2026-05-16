# HelsincyCrosshair 使用与集成指南

## 1. 适用场景

本指南面向以下需求：

- 你希望在 FPS 或 TPS 项目中接入动态准心与伤害方向指示器
- 你已经有自己的 Character、Pawn、GameMode 和 HUD
- 你想用最少改动接入插件，再逐步扩展

## 2. 接入前提

接入前请确认：

1. 项目已经启用 GameplayTags 模块能力。
2. 本地角色由 Pawn 或 Character 驱动。
3. 游戏存在一个可配置 HUD Class 的 GameMode（方式 A），或你已有自定义 HUD 且能在 DrawHUD 中添加一到两行 Bridge 调用（方式 B）。
4. 需要目标识别时，目标对象能够实现阵营接口或交互接口。

## 3. 双模块概览

插件包含两个独立运行时模块：

| 模块 | 功能 | 核心组件 | 渲染入口 |
|------|------|----------|----------|
| **HelsincyCrosshair** | 准心渲染、动态扩散、目标变色、命中标记 | `UHelsincyCrosshairComponent` | `UHelsincyCrosshairRenderLibrary` |
| **HelsincyDamageIndicator** | 伤害方向指示器 | `UHelsincyDamageIndicatorComponent` | `UHelsincyDamageIndicatorRenderLibrary` |

两个模块在正式运行路径上可以独立使用，也可以同时使用。非 Shipping 调试路径中，准星 HUD 会读取伤害指示器状态用于 `showdebug` 面板，但这不影响正式功能的独立接入。

## 4. 最小可运行接入步骤

### 步骤 1：启用插件

在编辑器的 Plugins 窗口启用 HelsincyCrosshair，或在 .uproject 中启用对应插件条目。

### 步骤 2：选择 HUD 集成方式

插件提供两种 HUD 集成路径，根据项目情况选择：

#### 方式 A：继承 AHelsincyHUD（简单项目 / 快速验证）

将 GameMode 的 HUD Class 设为 `AHelsincyHUD` 或其蓝图子类 `BP_HelsincySuperHUD`。这条路径适合快速验证准心主体、中心点、命中反馈、伤害方向指示器和调试面板。

内置 `AHelsincyHUD` 会在 `DrawHUD()` 中自动调用 `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD`。如果蓝图子类覆盖了绘制事件，请确认没有跳过父类绘制；已有项目仍推荐使用下面的 Bridge API 路径显式接入准心和伤害指示器绘制。

适用于新项目或尚未有自定义 HUD 的场景。

#### 方式 B：Bridge API 无侵入集成（推荐 / 已有项目）

如果项目已经有自己的 HUD 类且不想修改继承链，只需在你的 HUD 的 `DrawHUD()` 中添加 Bridge 调用：

```cpp
void AMyGameHUD::DrawHUD()
{
    Super::DrawHUD();

    // 绘制准心 | Draw crosshair
    UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(this);

    // 绘制伤害指示器 | Draw damage indicators (optional)
    UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(this);
}
```

蓝图中同样可用，在 HUD 的 `ReceiveDrawHUD` 事件中调用 `DrawCrosshairForHUD`；如果启用伤害指示器，再调用 `DrawDamageIndicatorsForHUD`。

> **Bridge API 是推荐的首选方案**，它不要求修改 HUD 继承链，对宿主项目零侵入。详细说明见 [BridgeUsageGuide_CN.md](BridgeUsageGuide_CN.md)。

### 步骤 3：给玩家角色添加组件

在玩家 Pawn 或 Character 上添加：

- `UHelsincyCrosshairComponent` — 准心（必须）
- `UHelsincyDamageIndicatorComponent` — 伤害指示器（可选）

准心组件是运行时核心，没有它 HUD / Bridge 不会拿到任何状态数据。

### 步骤 4：配置默认 DataAsset

准心组件上有两个关键字段：

| 字段 | 说明 |
|------|------|
| `DefaultCrosshairAsset` | 默认准心数据资产（可在编辑器中选） |
| `bUseDefaultCrosshairAssetInit` | 为 true 时 BeginPlay 自动加载该资产 |

推荐：将 `DefaultCrosshairAsset` 指向插件内置的 `Content/DataAsset/DA_Crosshair_Default.uasset`（或你自己的 DataAsset），保持 `bUseDefaultCrosshairAssetInit` 为 true。

如果启用伤害方向指示器，还需要在 `UHelsincyDamageIndicatorComponent` 上配置：

| 字段 | 说明 |
|------|------|
| `DefaultDataAsset` | 默认伤害指示器数据资产（可在编辑器中选） |
| `bUseDefaultDataAssetInit` | 为 true 时 BeginPlay 自动加载该资产 |

推荐：将 `DefaultDataAsset` 指向插件内置的 `Content/DataAsset/DA_DamageIndicator.uasset`（或你自己的 DataAsset），保持 `bUseDefaultDataAssetInit` 为 true。

### 步骤 5：确认本地控制角色生效

组件内部只对本地真人玩家控制的 Pawn 完整激活，不需要在所有远端角色或 AI 上运行 HUD 表现逻辑。系统天然偏向本地 HUD 表现层。

当前版本中，`UHelsincyCrosshairComponent` 和 `UHelsincyDamageIndicatorComponent` 都包含本地玩家守卫。Controller 尚未就位时组件会保持轻量 Pending 状态并低频重试；一旦确认 `APlayerController + IsLocalController()`，本地真人玩家会完成初始化并显示 HUD。AI Controller、远端玩家和模拟代理会被禁用，避免多人或 AI 场景下错误显示本地 HUD。

## 5. 选择准心形状（ShapeTag）

插件通过 `FGameplayTag` 决定使用哪个渲染器。在 DataAsset 或 `CurrentProfile.ShapeTag` 中设置对应 Tag 即可切换形状。

| Tag | 形状 | 说明 |
|-----|------|------|
| `Crosshair.Shape.Cross` | 十字 | 经典 FPS 准心 |
| `Crosshair.Shape.Circle` | 圆形 | 散布圈 |
| `Crosshair.Shape.DotOnly` | 仅圆点 | 最小化准心 |
| `Crosshair.Shape.TStyle` | T 形 | 三方向准心 |
| `Crosshair.Shape.Triangle` | 三角形 | 箭头样式 |
| `Crosshair.Shape.Rectangle` | 矩形 | 矩形准心 |
| `Crosshair.Shape.Chevron` | V 字形 | 人字型 |
| `Crosshair.Shape.Polygon` | 多边形 | 六边形等 |
| `Crosshair.Shape.Wings` | 双翼梯形 | 两侧展翅 |
| `Crosshair.Shape.Image` | 自定义图片 | 需配合 ImageConfig |

如果需要在项目设置中注册额外的蓝图渲染器，可在 **Project Settings → HelsincyCrosshairSystem → BlueprintRenderers** 中添加。

## 6. 开火、命中、受伤时如何调用

### 6.1 开火时

在武器开火逻辑中调用：

```cpp
CrosshairComponent->AddRecoil(HorizontalKick, VerticalKick);
```

参考值建议：

| 武器类型 | 水平 | 垂直 |
|----------|------|------|
| 步枪 | 1–3 | 3–6 |
| 冲锋枪 | 1–2 | 2–4 |
| 霰弹枪 | 3–6 | 4–8 |

具体数值需结合 `DynamicConfig.MaxRecoilSpread` 与 `DynamicConfig.RecoilRecoverySpeed` 调整。

### 6.2 命中时

根据命中类型调用：

| 函数 | 用途 |
|------|------|
| `TriggerHitMarker()` | 普通命中 |
| `TriggerHitMarkerColor(FLinearColor)` | 自定义颜色命中 |
| `TriggerHeadshotMarker()` | 爆头命中（使用 `HitMarkerConfig.HeadshotColor`） |
| `TriggerKillMarker()` | 击杀命中（使用 `HitMarkerConfig.KillColor`） |
| `TriggerHitMarkerAdvanced(Priority, CustomColor, DamageNormalized)` | 高级命中反馈，支持优先级、自定义颜色和伤害强度缩放 |

推荐把这些调用放在武器命中确认或伤害结算成功之后，而不是单纯射线命中就触发。

### 6.3 玩家受伤时（HelsincyDamageIndicator 模块）

在角色的 TakeDamage、AnyDamage 或自定义伤害响应中调用：

```cpp
DamageIndicatorComponent->RegisterDamageEvent(DamageCauserActor, HitLocation);
```

伤害指示器组件还提供以下控制 API：

| 函数 | 用途 |
|------|------|
| `IsEnabled()` | 查询当前是否启用 |
| `GetIndicatorProfile()` | 获取当前配置 |
| `SetIndicatorProfile(NewProfile)` | 运行时修改配置 |
| `LoadFromDataAsset(DataAsset)` | 从 DataAsset 加载配置 |
| `SavePreset(PresetName)` | 保存当前配置为命名预设（内存级） |
| `LoadPreset(PresetName)` | 加载指定预设 |
| `GetAllPresetNames()` | 获取所有预设名称 |
| `SavePresetsToDisk(SlotName, UserIndex)` | 保存预设库到磁盘 |
| `LoadPresetsFromDisk(SlotName, UserIndex)` | 从磁盘恢复预设库 |

> 可通过组件属性 `bAutoLoadOnBeginPlay` / `bAutoSaveOnEndPlay` 实现零代码自动保存/加载。默认槽位: `"DamageIndicatorPresets"`。
>
> 编辑器中可设置 `DefaultDataAsset` 指向 `UHelsincyDamageIndicatorDataAsset`，配合 `bUseDefaultDataAssetInit = true` 实现零代码初始化。

### 6.4 选择伤害指示器样式与放置模式

伤害指示器通过 `IndicatorProfile.IndicatorStyleTag` 选择渲染器：

| Tag | 样式 | 说明 |
|-----|------|------|
| `Indicator.Style.Arrow` | 箭头 | 轻量、方向明确，适合默认方案 |
| `Indicator.Style.Image` | 图片 | 使用自定义贴图作为方向提示 |
| `Indicator.Style.Arc` | 弧光 | COD 风格红色弧形提示，可配合中心指针增强方向感 |

放置模式由 `IndicatorProfile.PlacementMode` 控制：

| 模式 | 说明 |
|------|------|
| `RadialCircle` | 围绕屏幕中心的圆形轨道显示，适合传统伤害圆环 |
| `WindowEdge` | 沿当前游戏窗口 / Canvas 安全边界显示，适合窗口化和全屏模式 |

`WindowEdge` 使用当前 `Canvas->ClipX / ClipY` 计算位置，不使用物理显示器边缘。Arc 样式可通过 `ArcConfig.ArcMaskTexture` 指定弧光贴图；如果为空，内置 renderer 会生成默认弧光遮罩。

## 7. 运行时控制 API

`UHelsincyCrosshairComponent` 提供以下 BlueprintCallable / BlueprintPure 函数，用于运行时动态控制：

### 7.1 开关控制

| 函数 | 说明 |
|------|------|
| `EnableCrosshair()` / `DisableCrosshair()` | 运行时启用 / 关闭准心 |
| `IsEnabledCrosshair()` | 查询准心是否启用 |
| `EnableCenterDot()` / `DisableCenterDot()` | 单独开关中心圆点 |
| `EnableTargetDetection()` / `DisableTargetDetection()` | 单独开关目标识别 |

### 7.2 状态查询

| 函数 | 返回值 |
|------|--------|
| `GetCurrentProfile()` | 当前完整准心配置 `FHelsincyCrosshairProfile` |
| `GetCurrentVisualPrimaryColor()` | 当前主颜色（含目标变色后的最终值） |
| `GetFinalSpread()` | 总扩散 = 状态扩散 + 后坐力扩散 |
| `GetCurrentTargetAttitude()` | 当前瞄准目标的阵营态度 (Hostile/Friendly/Neutral) |
| `GetCurrentTargetTag()` | 当前目标的交互 GameplayTag |

### 7.3 配置与预设

| 函数 | 说明 |
|------|------|
| `LoadFromDataAsset(DataAsset)` | 从 DataAsset 加载完整配置（武器切换时推荐） |
| `UpdateProfile(NewProfile)` | 直接修改当前配置（UI 绑定用） |
| `SavePreset(PresetName)` | 将当前配置保存为命名预设（内存级） |
| `LoadPreset(PresetName)` | 加载指定预设（返回 bool 表示是否成功） |
| `ClearPresetLibrary()` | 清空内存中的预设库 |
| `GetAllPresetNames()` | 获取预设库中所有名称 |
| `SavePresetsToDisk(SlotName, UserIndex)` | 将预设库保存到磁盘（USaveGame） |
| `LoadPresetsFromDisk(SlotName, UserIndex)` | 从磁盘恢复预设库，自动应用上次活跃预设 |
| `DoesSaveExist(SlotName, UserIndex)` | 检查磁盘存档是否存在 |
| `DeleteSaveFromDisk(SlotName, UserIndex)` | 删除磁盘存档 |

> 插件已内建 USaveGame 持久化。可通过组件属性 `bAutoLoadOnBeginPlay` / `bAutoSaveOnEndPlay` 实现零代码自动保存/加载。

## 8. 目标识别与变色接入

插件支持两种目标识别路径。

### 8.1 阵营识别

如果目标实现 `GenericTeamAgentInterface`，组件会通过 `FGenericTeamId::GetAttitude` 判断：

- Hostile → 敌军色
- Friendly → 友军色
- Neutral → 中立色

颜色在 `VisualsConfig` 中配置。适合 AI 敌友识别、PVP/PVE 阵营判断。

### 8.2 交互对象识别

如果目标实现 `IHelsincyTargetInterface`，则可返回一个 GameplayTag，例如：

- `Target.Interactable`
- `Target.Loot.Gold`
- `Target.Terminal`

插件会查找 `VisualsConfig.InteractionColorMap` 中是否存在匹配或父级匹配项。适合拾取物、可交互物体、终端机关等。

## 9. Blueprint 集成建议

### 9.1 角色蓝图推荐做法

1. 添加 `HelsincyCrosshairComponent`。
2. 暴露一个可切换的准心 DataAsset 变量。
3. 在 BeginPlay 或武器切换时调用 `LoadFromDataAsset`。
4. 在开火事件中调用 `AddRecoil`。
5. 在命中事件中调用 `TriggerHitMarker` / `TriggerHeadshotMarker` / `TriggerKillMarker`。

如果需要伤害方向指示器：

6. 额外添加 `HelsincyDamageIndicatorComponent`。
7. 在受伤事件中调用 `RegisterDamageEvent`。
8. 确认 HUD 中有伤害指示器绘制调用：Bridge 路径需添加 `DrawDamageIndicatorsForHUD`；内置 `AHelsincyHUD` 已自动调用，蓝图子类需避免覆盖掉父类绘制。

### 9.2 武器切换推荐做法

每把武器维护一个对应的 `UHelsincyCrosshairDataAsset`，在装备时：

1. 获取角色上的 `CrosshairComponent`。
2. 调用 `LoadFromDataAsset(WeaponCrosshairAsset)`。

这样比逐项修改 `CurrentProfile` 更稳定，也更容易交给策划调参。

## 10. C++ 集成示例

### 10.1 开火 + 命中

```cpp
// 仅在本地控制 Pawn 上调用有意义
if (UHelsincyCrosshairComponent* Crosshair = FindComponentByClass<UHelsincyCrosshairComponent>())
{
    // 开火时
    Crosshair->AddRecoil(1.5f, 4.0f);

    // 命中确认后
    Crosshair->TriggerHitMarker();
    // 或 Crosshair->TriggerHeadshotMarker();
    // 或 Crosshair->TriggerKillMarker();
}
```

### 10.2 切换准心资产

```cpp
if (UHelsincyCrosshairComponent* Crosshair = FindComponentByClass<UHelsincyCrosshairComponent>())
{
    Crosshair->LoadFromDataAsset(NewWeaponCrosshairAsset);
}
```

### 10.3 伤害指示器

```cpp
if (UHelsincyDamageIndicatorComponent* DI = FindComponentByClass<UHelsincyDamageIndicatorComponent>())
{
    DI->RegisterDamageEvent(DamageCauserActor, HitLocation);
}
```

### 10.4 Bridge 集成（在已有 HUD 中）

```cpp
#include "Library/HelsincyCrosshairRenderLibrary.h"
#include "Library/HelsincyDamageIndicatorRenderLibrary.h"

void AMyProjectHUD::DrawHUD()
{
    Super::DrawHUD();
    UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(this);
    UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(this);
}
```

### 10.5 运行时切换

```cpp
if (UHelsincyCrosshairComponent* Crosshair = FindComponentByClass<UHelsincyCrosshairComponent>())
{
    // 瞄准时关闭准心
    Crosshair->DisableCrosshair();

    // 退出瞄准时恢复
    Crosshair->EnableCrosshair();

    // 查询当前扩散
    FVector2D Spread = Crosshair->GetFinalSpread();
}
```

## 11. 调试与验证清单

接入完成后，建议按下面顺序验收：

1. 屏幕中心是否出现准心。
2. 开火时准心是否扩散并逐渐恢复。
3. 命中时是否出现 HitMarker。
4. 受击时是否出现方向指示器（需挂载 `DamageIndicatorComponent` + HUD 绘制调用）。
5. 看向敌人和友军时颜色是否变化。
6. 看向可交互物体时颜色是否按 Tag 映射变化。
7. 切换武器时是否能切换为对应准心样式。
8. 调用 `DisableCrosshair()` / `EnableCrosshair()` 是否正常生效。

可使用控制台命令 `showdebug Crosshair` 或 `showdebug DamageIndicator` 查看运行时调试面板。

## 12. 常见接入问题

### 12.1 看不到准心

优先排查：

1. HUD 是否为 `AHelsincyHUD` 子类，或者 DrawHUD 中是否调用了 `DrawCrosshairForHUD`。
2. 角色上是否存在 `UHelsincyCrosshairComponent`。
3. 该角色是否为本地控制 Pawn。
4. `bEnabledCrosshair` 是否被关闭（可调用 `IsEnabledCrosshair()` 检查）。
5. `CurrentProfile.ShapeTag` 是否有效（必须是 `Crosshair.Shape.*` 下的合法 Tag）。

### 12.2 看得到准心，但没有动态扩散

检查：

1. `DynamicConfig.bEnableDynamicSpread` 是否开启。
2. 开火逻辑是否实际调用了 `AddRecoil`。
3. 角色移动组件是否在工作。

### 12.3 看向目标不变色

检查：

1. `VisualsConfig.bEnableTargetSwitching` 是否开启。
2. `VisualsConfig.SwitchingChannel` 是否能命中目标。
3. 目标是否实现 `GenericTeamAgentInterface` 或 `IHelsincyTargetInterface`。
4. 目标是否被 OwningPawn 忽略外的碰撞体遮挡。

### 12.4 伤害指示器不出现

检查：

1. 角色上是否挂载了 `UHelsincyDamageIndicatorComponent`。
2. `IndicatorProfile.bEnabled` 是否为 true。
3. 受伤时是否调用了 `RegisterDamageEvent`。
4. HUD 中是否有 `DrawDamageIndicatorsForHUD` 的调用；Bridge 路径需要手动添加，内置 `AHelsincyHUD` 已自动调用，蓝图子类需避免覆盖掉父类绘制。

## 13. 推荐集成策略

对于正式项目，推荐采用下面的落地方式：

1. **优先使用 Bridge API 集成**，在已有 HUD 的 DrawHUD 中添加 `DrawCrosshairForHUD`；如果启用伤害指示器，再添加 `DrawDamageIndicatorsForHUD`，避免修改继承链。
2. 每种武器维护一个准心 DataAsset，切换时调用 `LoadFromDataAsset`。
3. 角色统一挂 `CrosshairComponent`；如果需要伤害方向提示，再挂 `DamageIndicatorComponent`，不要在武器上重复创建。
4. 阵营识别走 `GenericTeamAgentInterface`，交互识别走 `IHelsincyTargetInterface`。
5. 复杂自定义样式优先做新渲染器（见 [Extension-Guide.md](Extension-Guide.md)），不建议把绘制逻辑堆到 HUD。
6. 需要自定义 Blueprint 渲染器时，准星在 **Project Settings → HelsincyCrosshairSystem → BlueprintRenderers** 中添加，伤害指示器在 **Project Settings → HelsincyDamageIndicatorSystem → BlueprintIndicatorRenderers** 中添加。Blueprint Renderer 会由 Subsystem 异步加载并自动注册；C++ Renderer 需要项目侧显式调用 `RegisterRenderer` 或 `RegisterDamageIndicatorRenderer` 手动注册。

如果你要做进一步定制，请继续阅读 [Configuration-Reference.md](Configuration-Reference.md) 和 [Extension-Guide.md](Extension-Guide.md)。
