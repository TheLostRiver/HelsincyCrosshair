# HelsincyCrosshair

**语言 / Language**: 简体中文 | [English](README_EN.md)

`HelsincyCrosshair` 是面向 Unreal Engine 射击项目的运行时动态准心与伤害方向提示插件。插件采用 `Canvas` / `FCanvas` 绘制路径，核心目标是在不依赖 UMG 主路径的前提下，提供低延迟、数据驱动、可扩展、便于蓝图和 C++ 集成的 HUD 表现系统。

当前实现由两个 Runtime 模块组成：

- `HelsincyCrosshair`：准心、中心点、动态扩散、后坐力扩散恢复、命中反馈、目标识别变色。
- `HelsincyDamageIndicator`：受击方向提示、窗口边缘/圆形放置模式、Arrow/Image/Arc 指示器渲染。

如果你想先了解插件采用的整体架构、为什么这样设计，以及它如何做到易扩展、易集成和高性能，请直接阅读 [项目架构文档](Docs/Architecture.md)。

如果你想直接开始使用，请阅读 [中文用户手册](Docs/UserManual_CN.md)；需要按工程步骤接入时，请阅读 [集成指南](Docs/Integration-Guide.md)。

## 当前能力

- 10 个已注册的内置准心渲染器：Cross、Circle、TStyle、Triangle、Rectangle、Chevron、Polygon、Wings、Image、DotOnly。
- 命中反馈支持多样式绘制，并包含 COD 风格的扭转、缩放和伤害强度响应。
- 3 个已注册的内置伤害方向指示器样式：Arrow、Image、Arc。
- 伤害方向指示器支持 `RadialCircle` 和 `WindowEdge` 两种放置模式；`WindowEdge` 基于当前游戏 Canvas / 窗口边缘，而不是物理显示器边缘。
- 基于 GameplayTag 的准心形状、伤害指示器样式和扩展渲染器选择。
- 基于 DataAsset 的默认配置加载，以及运行时内存预设保存/读取。
- 内建 `USaveGame` 磁盘持久化，预设可跨关卡/重启保留，支持自动保存/加载。
- 支持 Legacy HUD 和 Bridge API 两种集成方式。
- 支持 UE4 `STAT` 性能统计，可用 `stat HelsincyCrosshair` / `stat HelsincyDamageIndicator` 查看插件热路径开销。

## 集成方式

### Legacy HUD

将 GameMode 的 HUD Class 设置为 `AHelsincyHUD`，或者使用插件内置蓝图子类 `BP_HelsincySuperHUD`。该路径适合快速验证和兼容旧工程。

### Bridge API

在任意 C++ / 蓝图 HUD 内调用渲染库函数，不要求继承 `AHelsincyHUD`：

- `UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(AHUD* HUD)`
- `UHelsincyCrosshairRenderLibrary::DrawCrosshairForController(APlayerController*, UCanvas*)`
- `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(AHUD* HUD)`
- `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForController(APlayerController*, UCanvas*)`

当前 Bridge 路径覆盖准心主体、Center Dot、Hit Marker 和 Damage Indicator。

## 快速接入

1. 在项目中启用 `HelsincyCrosshair` 插件。
2. 将 GameMode 的 HUD Class 设置为 `AHelsincyHUD` / `BP_HelsincySuperHUD`，或在自定义 HUD 中接入 Bridge API。
3. 在本地玩家 Pawn 或 Character 上添加 `UHelsincyCrosshairComponent`。
4. 将 `DefaultCrosshairAsset` 指向 `Content/DataAsset/DA_Crosshair_Default.uasset`，或在 BeginPlay 之后手动调用 `LoadFromDataAsset`。
5. 在开火时调用 `AddRecoil`，在命中时调用 `TriggerHitMarker`、`TriggerHeadshotMarker` 或 `TriggerHitMarkerAdvanced`。
6. 如果需要伤害方向提示，额外添加 `UHelsincyDamageIndicatorComponent`。
7. 将 `DefaultDataAsset` 指向 `Content/DataAsset/DA_DamageIndicator.uasset`，并保持 `bUseDefaultDataAssetInit = true`。
8. 在角色受伤事件、`TakeDamage` 或 `AnyDamage` 中调用 `UHelsincyDamageIndicatorComponent::RegisterDamageEvent`。
9. 如果需要敌友识别，让目标实现 `GenericTeamAgentInterface`；如果需要交互物识别，让目标实现 `IHelsincyTargetInterface`。

详细步骤见 [Docs/Integration-Guide.md](Docs/Integration-Guide.md)。

## 模块与核心类型

| 模块 | 职责 | 关键组件 | Bridge / 渲染入口 |
|------|------|----------|-------------------|
| `HelsincyCrosshair` | 准心状态、目标识别、命中反馈 | `UHelsincyCrosshairComponent` | `UHelsincyCrosshairRenderLibrary` |
| `HelsincyDamageIndicator` | 伤害方向状态、DataAsset、预设持久化 | `UHelsincyDamageIndicatorComponent` | `UHelsincyDamageIndicatorRenderLibrary` |

`UHelsincyCrosshairComponent` 负责当前准心 Profile、运行时颜色缓存、动态扩散、后坐力恢复、命中反馈实例和异步目标检测。

`UHelsincyDamageIndicatorComponent` 负责伤害方向配置、活跃指示器生命周期、角度更新、DataAsset 初始化和伤害指示器预设持久化。

HUD 或 Bridge 渲染库负责从组件读取状态，按 GameplayTag 查找对应 renderer，并绘制到当前 `Canvas`。

## 主要资源

- `Content/HUD/BP_HelsincySuperHUD.uasset`：HUD 蓝图资源，可作为默认 HUD 的直接起点。
- `Content/DataAsset/DA_Crosshair_Default.uasset`：默认准心数据资产。
- `Content/DataAsset/DA_DamageIndicator.uasset`：默认伤害指示器数据资产。
- `Content/ArcImage/T_HDI_Arc_Red_01.uasset`：Arc 风格伤害指示器弧光贴图。
- `Content/HitmarkerImage/T_HC_HitMarker_Core.uasset`：命中准星核心贴图。
- `Content/HitmarkerImage/T_HC_HitMarker_Glow.uasset`：命中准星辉光贴图。
- `Config/DefaultHelsincyCrosshair.ini`：旧类名、旧字段名与模块拆分相关重定向配置。

## 代码结构

```text
HelsincyCrosshair/
├── Config/
├── Content/
│   ├── ArcImage/
│   ├── DataAsset/
│   ├── HitmarkerImage/
│   └── HUD/
├── Docs/
├── Resources/
└── Source/
    ├── HelsincyCrosshair/
    │   ├── Public/
    │   └── Private/
    └── HelsincyDamageIndicator/
        ├── Public/
        └── Private/
```

## 数据驱动配置

`FHelsincyCrosshairProfile` 聚合准心视觉、动态扩散、形状参数、中心点和命中反馈配置，可通过 `UHelsincyCrosshairDataAsset` 加载。

`FHelsincy_DamageIndicatorProfile` 聚合伤害指示器开关、生命周期、放置模式、圆环、Arrow/Image/Arc 样式参数和持久化预设数据，可通过 `UHelsincyDamageIndicatorDataAsset` 加载。

伤害指示器样式通过 `IndicatorStyleTag` 选择，当前内置 GameplayTag 包括：

- `Indicator.Style.Arrow`
- `Indicator.Style.Image`
- `Indicator.Style.Arc`

伤害指示器放置模式包括：

- `RadialCircle`：围绕玩家屏幕中心的圆形轨道显示。
- `WindowEdge`：沿当前游戏窗口 / Canvas 安全边界显示，适合窗口化和全屏模式。

## 文档导航

| 文档 | 路径 | 适合谁看 | 说明 |
|------|------|----------|------|
| 项目文档规划 | [Docs/Documentation-Plan.md](Docs/Documentation-Plan.md) | 项目负责人、维护者 | 文档体系设计方案、覆盖范围、维护建议 |
| 项目架构文档 | [Docs/Architecture.md](Docs/Architecture.md) | 程序、技术策划 | 模块边界、类职责、数据流、渲染链路 |
| 使用与集成指南 | [Docs/Integration-Guide.md](Docs/Integration-Guide.md) | 游戏逻辑程序、蓝图开发者 | 从启用插件到角色接入的完整步骤 |
| 配置参考手册 | [Docs/Configuration-Reference.md](Docs/Configuration-Reference.md) | 调参与系统集成人员 | Profile 结构体与配置项说明 |
| 扩展开发指南 | [Docs/Extension-Guide.md](Docs/Extension-Guide.md) | 引擎程序、插件维护者 | 自定义渲染器、目标接口、扩展策略 |
| 完整 API 参考 | [Docs/API-Reference.md](Docs/API-Reference.md) | 程序、蓝图开发者 | 所有类、方法、属性、结构体、枚举、GameplayTag 的双语参考 |
| 用户手册（中文） | [Docs/UserManual_CN.md](Docs/UserManual_CN.md) | 所有用户 | 完整的中文用户手册，覆盖快速集成到高级扩展 |
| 用户手册（英文） | [Docs/UserManual_EN.md](Docs/UserManual_EN.md) | All users | Full English user manual |
| 已知问题与限制 | [Docs/Known-Issues.md](Docs/Known-Issues.md) | 所有人 | 当前版本的实现边界、注意事项和规避建议 |

## 当前版本注意事项

- 这份 README 以当前仓库源码和插件 Content 资源为准，不沿用早期设计文档中的理想状态。
- `HelsincyDamageIndicator` 已从准心模块中拆出为独立 Runtime 模块；需要伤害方向提示时必须添加 `UHelsincyDamageIndicatorComponent`。
- 默认人物类是否挂载伤害指示器组件取决于项目侧配置；插件不会强行给所有 Pawn 注入该组件。
- `WindowEdge` 使用当前游戏 Canvas / 窗口边界计算位置，适配窗口化运行；它不是桌面显示器边界定位。
- [Docs/Archive/DesignDocument_Legacy_ACS.md](Docs/Archive/DesignDocument_Legacy_ACS.md) 是已归档的旧版 ACS 设计文档，仅适合做历史对照，不应作为当前项目标准。
