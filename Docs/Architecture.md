# HelsincyCrosshair 项目架构文档

## 1. 架构目标

HelsincyCrosshair 的架构目标可以概括为五点：

1. 用最小运行时开销完成准心和伤害指示器绘制。
2. 将状态计算与绘制职责分开。
3. 用 GameplayTag 和抽象渲染器支持扩展。
4. 用结构体与 DataAsset 实现数据驱动。
5. 通过组件、HUD/Bridge API 和独立模块边界降低集成成本。

它并不是一个 UMG 准心 Widget 系统，而是一个偏底层、偏运行时性能导向的 HUD Canvas 插件。核心架构追求三件事：**易于扩展**（Tag + Renderer 策略模式）、**易于集成**（ActorComponent + HUD/Bridge 入口）、**高性能**（Canvas 原语、短绘制链路、运行时状态与静态资产分离）。

### 1.1 当前项目采用的架构

当前项目采用的是一种 **模块化 Runtime 插件架构 + Component 驱动架构 + Canvas 即时绘制架构 + GameplayTag Renderer 策略架构 + DataAsset 数据驱动架构** 的组合：

- **模块化 Runtime 插件架构**：准星和伤害指示器拆成两个 Runtime 模块，分别拥有自己的组件、Subsystem、Renderer、Settings、DataAsset 和 SaveGame。
- **Component 驱动架构**：运行时状态挂在 Actor/Pawn 的组件上，宿主项目只需要添加组件并调用事件/API，不需要继承插件提供的角色基类。
- **Canvas 即时绘制架构**：每帧通过 HUD/Bridge 直接绘制 Canvas 原语和贴图，不把准星拆成 UMG 控件树。
- **GameplayTag Renderer 策略架构**：Profile 只保存样式 Tag，Subsystem 通过 `Tag -> RendererClass` 注册表选择具体 Renderer。
- **DataAsset 数据驱动架构**：静态配置放入 DataAsset/Profile，运行时预设通过 SaveGame 持久化，代码只负责加载和执行配置。

采用这种架构的原因是：准星、命中反馈和伤害方向提示都属于高频 HUD 叠加层，要求响应快、绘制链路短、接入项目成本低，同时又需要允许项目方不断添加新样式。UMG 更适合复杂交互 UI，但对这种每帧高频、几何变化明确的 HUD 元素来说会引入额外布局与控件管理成本；纯硬编码 Canvas 又会让每新增一个样式都修改核心逻辑。因此本项目把“状态计算”“配置来源”“样式选择”“屏幕绘制”拆开，用 C++ 保持热路径性能，用 Blueprint/Settings 保留低门槛扩展入口。

这种架构的好处是：

- **性能稳定**：Canvas 绘制链路短，Renderer 查找由 Subsystem 注册表完成，DataAsset/SaveGame 不参与每帧热路径。
- **扩展成本低**：新增样式通常只需要新增 GameplayTag、Renderer 和可选配置结构，不需要改动组件核心状态机。
- **集成成本低**：宿主项目可以通过 ActorComponent、内置 HUD 或 Bridge API 接入，不需要重写角色继承链。
- **职责清晰**：组件负责状态，Subsystem 负责注册和资源，Renderer 负责绘制，DataAsset/SaveGame 负责配置与持久化。
- **C++ 与 Blueprint 兼容**：C++ Renderer 适合高性能和精确控制，手动注册保证依赖可控；Blueprint Renderer 适合快速试验和项目自定义，配置后由 Subsystem 自动加载注册。

## 2. 总体结构

### 2.1 模块层级

插件包含两个 Runtime 模块，职责边界独立：

- **HelsincyCrosshair** — 准星核心（形状渲染、动态扩散、命中反馈、目标检测）
- **HelsincyDamageIndicator** — 伤害方向指示器（方向指示器渲染、伤害事件管理）

在正式运行路径上，两个模块可以分别启用：用户可以只使用准星、只使用伤害指示器、或同时使用两者。需要注意的是，非 Shipping 调试路径中 `HelsincyCrosshair` 会为了 ShowDebug/验证信息读取伤害指示器状态，因此存在一条调试用途的单向依赖。

两个模块均在 PreLoadingScreen 阶段加载，这意味着它们可以较早完成原生 Tag 和渲染器体系初始化。

### 2.2 HelsincyCrosshair 目录职责

| 目录 | 作用 |
|------|------|
| Public/Components | 对外暴露的准心核心组件 |
| Public/HUD | HUD 入口类，负责调度绘制 |
| Public/Subsystems | 全局准心渲染器管理器 |
| Public/DataTypes | 准心配置结构、运行时状态结构、Tag 常量 |
| Public/DataAssets | 准心配置资产容器 |
| Public/Render | 准心形状渲染器抽象与实现 |
| Public/Interfaces | 与宿主项目交互的接口 |
| Public/Settings | 准心开发者设置，挂接蓝图扩展入口 |
| Public/Library | Bridge 层 — 准心绘制入口 |

### 2.3 HelsincyDamageIndicator 目录职责

| 目录 | 作用 |
|------|------|
| Public/Components | 伤害指示器组件（事件注册、状态管理） |
| Public/Subsystems | 全局指示器渲染器管理器 |
| Public/DataTypes | 指示器配置结构、运行时状态结构、Tag 常量 |
| Public/DataAssets | 伤害指示器配置资产容器 |
| Public/SaveGame | 伤害指示器运行时预设持久化 |
| Public/IndicatorRenderer | 伤害方向指示器渲染器抽象与实现 |
| Public/Settings | 指示器开发者设置，挂接蓝图扩展入口 |
| Public/Library | Bridge 层与放置解析器 — 指示器绘制入口、RadialCircle/WindowEdge 坐标解析 |

## 3. 核心类职责

### 3.1 HelsincyCrosshair 模块

| 类/结构 | 角色 | 职责 |
|---------|------|------|
| UHelsincyCrosshairComponent | 运行时核心 | 保存当前 Profile，计算扩散，维护命中反馈，执行目标检测 |
| AHelsincyHUD | 绘制入口 | 每帧从组件读取状态，选择渲染器并绘制 Canvas |
| UHelsincyCrosshairManagerSubsystem | 注册中心 | 管理准心渲染器和共享平滑纹理 |
| UHelsincyShapeRenderer | 准心渲染抽象基类 | 定义准心形状 Draw 接口和通用描边辅助 |
| UHelsincyCrosshairDataAsset | 数据容器 | 以资产形式持有一份完整 Profile |
| FHelsincyCrosshairProfile | 核心配置体 | 汇总视觉、动态扩散、形状、中心点和命中反馈配置 |
| IHelsincyTargetInterface | 目标接口 | 为目标对象提供交互 Tag |
| UHelsincyCrosshairSettings | 扩展配置入口 | 允许在配置中声明蓝图准心渲染器类 |
| UHelsincyCrosshairRenderLibrary | Bridge | 提供蓝图可调用的准星绘制入口 |

### 3.2 HelsincyDamageIndicator 模块

| 类/结构 | 角色 | 职责 |
|---------|------|------|
| UHelsincyDamageIndicatorComponent | 运行时核心 | 管理伤害事件、维护活跃指示器列表、计算角度插值 |
| UHelsincyDamageIndicatorSubsystem | 注册中心 | 管理指示器渲染器和共享平滑纹理 |
| UHelsincyIndicatorRenderer | 指示器渲染抽象基类 | 定义伤害方向指示器 DrawPointer 接口 |
| UHelsincyDamageIndicatorDataAsset | 数据容器 | 以资产形式持有一份完整伤害指示器 Profile |
| UHelsincyDamageIndicatorSaveGame | 持久化容器 | 保存运行时预设库和当前预设名 |
| FHelsincy_DamageIndicatorProfile | 核心配置体 | 汇总伤害指示器的放置、视觉、淡入淡出、圆环和样式专属配置 |
| FHelsincy_ActiveDamageIndicator | 运行时状态 | 记录单个伤害来源的状态 |
| UHelsincyDamageIndicatorSettings | 扩展配置入口 | 允许在配置中声明蓝图指示器渲染器类 |
| UHelsincyDamageIndicatorRenderLibrary | Bridge | 提供蓝图可调用的指示器绘制入口 |

## 4. 运行时数据流

### 4.1 初始化阶段

1. 模块加载。
2. Engine Subsystem 初始化。
3. `UHelsincyCrosshairManagerSubsystem` 注册内置 C++ 准星渲染器。
4. `UHelsincyDamageIndicatorSubsystem` 注册内置 C++ 指示器渲染器。
5. 两个 Subsystem 根据各自 DeveloperSettings 异步加载 Blueprint 渲染器类。
6. 准星组件 BeginPlay 时决定是否从 `DefaultCrosshairAsset` 加载默认配置。
7. 伤害指示器组件 BeginPlay 时决定是否从 `DefaultDataAsset` 加载默认配置，并按本地玩家归属决定是否激活。
8. HUD 或 Bridge 绘制入口在每帧绘制时查找组件与 Subsystem。

### 4.2 每帧更新阶段

**UHelsincyCrosshairComponent** Tick 流程：

1. 计算动态状态扩散。
2. 恢复后坐力扩散。
3. 更新命中反馈实例寿命。
4. 发起异步目标检测。
5. 根据当前命中目标刷新准心主颜色缓存。

**UHelsincyDamageIndicatorComponent** Tick 流程：

1. 更新伤害指示器存活时间。
2. 更新指示器角度的平滑插值。
3. 移除已过期的指示器。

### 4.3 每帧绘制阶段

HUD/Bridge 中准星绘制流程：

1. 获取本地玩家的 UHelsincyCrosshairComponent。
2. 检查是否启用准心。
3. 获取当前 Profile、最终颜色、最终扩散和 DPI 缩放。
4. 根据 ShapeTag 向 Subsystem 请求对应的 UHelsincyShapeRenderer。
5. 绘制主准心。
6. 根据配置绘制中心点。
7. 根据 ActiveHitMarkers 绘制命中反馈。

伤害指示器绘制流程（独立模块，需单独调用 HUD 或 Bridge 入口）：

1. 获取 UHelsincyDamageIndicatorComponent。
2. 检查是否启用指示器。
3. 获取配置 Profile 和活跃指示器列表。
4. 通过 DamageIndicatorSubsystem 获取指示器渲染器。
5. 根据 `PlacementMode` 解析 RadialCircle 或游戏窗口边缘坐标。
6. 绘制背景圆环（可选）和方向指示器。

## 5. 渲染架构

### 5.1 为什么选择 Canvas

当前插件选择在 AHUD::DrawHUD 中直接使用 Canvas 原语绘制，而不是将准心拆成一组 UMG 控件。这样做的直接收益是：

- 绘制链路更短
- 更容易做像素级和几何级控制
- 不依赖 Slate 布局系统
- 更适合高频、低复杂度的 HUD 叠加元素

### 5.2 渲染器策略模式

准心形状不是写死在 HUD 里的。HUD 只负责拿到 Tag，然后转给具体渲染器：

- UHelsincyRendererCross
- UHelsincyRendererCircle
- UHelsincyRendererTStyle
- UHelsincyRendererTriangle
- UHelsincyRendererRectangle
- UHelsincyRendererChevron
- UHelsincyRendererPolygon
- UHelsincyRendererWings
- UHelsincyRendererImage
- UHelsincyRendererDotOnly

伤害方向提示同理，当前内置：

- UHelsincyIndicatorRendererArrow
- UHelsincyIndicatorRendererImage
- UHelsincyIndicatorRendererArc

### 5.3 描边机制

UHelsincyShapeRenderer 提供 RenderWithOutline 辅助方法。具体渲染器只需描述“如何画本体”，描边由基类控制是否启用、描边颜色和额外厚度。

### 5.4 平滑纹理

UHelsincyCrosshairManagerSubsystem 和 UHelsincyDamageIndicatorSubsystem 各自在初始化阶段独立生成一张 32x32 的 SmoothLineTexture，作为线条边缘平滑和圆形 AA 的共享资源。两个模块独立生成各自的纹理，确保完全解耦。这避免了插件必须附带一张固定贴图资源。

## 6. 配置架构

### 6.1 Profile 聚合设计

**准星模块**：FHelsincyCrosshairProfile 是准星系统的配置中枢，内部聚合：

- FHelsincy_VisualSettings
- FHelsincy_DynamicsSettings
- FHelsincy_CrosshairConfig
- FHelsincy_RadialConfig
- FHelsincy_BoxConfig
- FHelsincy_WingsConfig
- FHelsincy_ImageConfig
- FHelsincy_CenterDotConfig
- FHelsincy_HitMarkerProfile

**伤害指示器模块**：FHelsincy_DamageIndicatorProfile 是指示器系统的独立配置，包含：

- 总开关、持续时间、渲染样式 Tag
- 放置模式配置：`RadialCircle` 或 `WindowEdge`
- 游戏窗口边缘配置：`EdgeMargin`、`EdgeCornerPadding`、`bHideCircleInWindowEdgeMode`
- 淡入淡出配置
- 圆环配置（显示/颜色/粗细/半径/分段）
- 箭头/图片/弧光渲染器的专属配置（`FHelsincy_Ind_ArrowConfig` / `FHelsincy_Ind_ImageConfig` / `FHelsincy_Ind_ArcConfig`）

两个 Profile 完全独立，可分别存储和管理。

### 6.2 运行时状态与静态配置分离

**准星组件**运行时状态：

- 配置：CurrentProfile
- 运行时颜色缓存：CurrentVisualPrimaryColorCache
- 状态扩散：StateSpread
- 后坐力扩散：RecoilSpread
- 命中反馈实例：ActiveHitMarkers

**伤害指示器组件**运行时状态：

- 配置：IndicatorProfile
- 伤害方向实例：ActiveIndicators
- 运行时预设库：PresetLibrary
- 当前预设名：ActivePresetName

这样做的价值是：动态值不会污染资产本身，两套功能的状态互不干扰。

## 7. 目标识别与颜色系统

### 7.1 目标检测方式

组件每帧发起 AsyncLineTraceByChannel，不阻塞游戏线程。检测起点优先来自 PlayerCameraManager，否则回退到 Owner 的视点。

### 7.2 目标分类来源

目标颜色变化的判断来源有两类：

1. GenericTeamAgentInterface：用于敌友中立阵营判断。
2. IHelsincyTargetInterface：用于返回交互 GameplayTag。

### 7.3 颜色覆盖顺序

当前逻辑大致遵循以下顺序：

1. 基础色 PrimaryColor
2. 阵营色覆盖
3. 交互 Tag 映射表覆盖
4. 默认交互色覆盖
5. 中立色覆盖

这意味着配置时需要明确颜色优先级预期，避免多种系统同时作用时产生误判。

## 8. 预设与资产设计

插件同时支持两类配置来源：

### 8.1 DataAsset 持久配置

`UHelsincyCrosshairDataAsset` 用于在编辑器中配置一份准心样式，并作为默认值或模板使用。

`UHelsincyDamageIndicatorDataAsset` 用于在编辑器中配置一份伤害指示器样式。角色或 Pawn 上的 `UHelsincyDamageIndicatorComponent` 可通过 `DefaultDataAsset` + `bUseDefaultDataAssetInit` 在 BeginPlay 自动加载，也可以在运行时调用 `LoadFromDataAsset()` 切换配置。

### 8.2 运行时内存预设

准星组件内部维护一个 `TMap<FName, FHelsincyCrosshairProfile>` 预设库，可在运行时 `SavePreset` 和 `LoadPreset`。通过 `SavePresetsToDisk()` / `LoadPresetsFromDisk()` 可将预设库序列化到磁盘（基于 `USaveGame`），实现跨关卡和游戏重启后的持久化。也可通过组件属性 `bAutoLoadOnBeginPlay` / `bAutoSaveOnEndPlay` 实现自动保存/加载。

伤害指示器组件同样维护一个 `TMap<FName, FHelsincy_DamageIndicatorProfile>` 预设库，使用 `UHelsincyDamageIndicatorSaveGame` 保存到默认槽位 `DamageIndicatorPresets`。这使玩家可以在运行时切换 RadialCircle/WindowEdge、Arrow/Image/Arc 等不同指示器方案，并将结果持久化。

### 8.3 游戏窗口边缘放置

伤害指示器支持两种数据驱动放置模式：

- `RadialCircle`：围绕屏幕中心或准心附近的圆形轨道放置，适合传统圆环式伤害提示。
- `WindowEdge`：根据当前游戏 Canvas 的 `ClipX / ClipY` 将指示器放到游戏窗口边缘，而不是物理显示器边缘。这样在窗口化、无边框窗口和分辨率缩放场景下都能保持正确位置。

`WindowEdge` 的角落平滑由放置解析器处理，避免指示器经过四个角时发生明显跳变。渲染器收到的是解析后的中心点、朝向和尺寸，因此 Arrow/Image/Arc 可以共享同一套边缘放置逻辑。

## 9. 扩展架构

### 9.1 准心扩展

新增准心渲染器的核心要求：

- 继承 UHelsincyShapeRenderer
- 设置 AssociatedTag
- 实现 Draw 或蓝图事件 ReceiveDraw
- 让 Subsystem 能注册该类

这里需要区分 C++ 和 Blueprint 两条扩展路径：

- **C++ 渲染器：手动注册。** 创建 `UHelsincyShapeRenderer` 子类，设置 `AssociatedTag`，实现 `Draw()`。仅创建 C++ 类不会自动注册，必须在模块初始化、项目初始化逻辑或自定义 Subsystem 初始化中显式调用 `RegisterRenderer(Tag, RendererClass)`。
- **Blueprint 渲染器：配置后自动注册。** 创建 Blueprint 子类，设置 `AssociatedTag`，实现 `ReceiveDraw`，并把 Blueprint 类加入 `UHelsincyCrosshairSettings::BlueprintRenderers`。Subsystem 会在初始化后异步加载并自动注册这些 Blueprint 类。

准心扩展之所以容易，是因为核心组件和 HUD/Bridge 不关心具体形状类。它们只输出当前状态、Profile、颜色、扩散值和 `ShapeTag`，随后由 Subsystem 的 `Tag -> RendererClass` 注册表找到对应 Renderer。新增形状通常只影响新的 Renderer 类和一个 GameplayTag，不需要改写准星组件的状态计算，也不需要在 HUD 中增加大量分支。

Blueprint 能自动注册，是因为项目设置里保存的是明确声明过的 Blueprint Renderer 类。Subsystem 初始化后会异步加载这些类，读取类默认对象（CDO）上的 `AssociatedTag`，再调用内部注册逻辑加入注册表。C++ Renderer 采用手动注册，是为了避免运行时扫描所有 C++ 类带来的加载成本、模块耦合和不可控副作用；插件只注册内置 C++ Renderer，项目自定义 C++ Renderer 由项目侧显式接入。

### 9.2 指示器扩展

新增伤害指示器渲染器的核心要求（在 HelsincyDamageIndicator 模块中）：

- 继承 UHelsincyIndicatorRenderer
- 设置 AssociatedTag
- 实现 DrawPointer 或蓝图事件 ReceiveDrawPointer
- 让 UHelsincyDamageIndicatorSubsystem 能注册该类

同样需要区分 C++ 和 Blueprint：

- **C++ 渲染器：手动注册。** 创建 `UHelsincyIndicatorRenderer` 子类，设置 `AssociatedTag`，实现 `DrawPointer()` 或更推荐的新接口 `DrawPointerResolved()`。如果渲染器需要完整支持 `WindowEdge`，还应覆盖 `GetDesiredPointerSize()`，这样放置解析器才能正确处理边缘和角落。C++ 类需要显式调用 `RegisterDamageIndicatorRenderer(Tag, RendererClass)` 才会进入注册表。
- **Blueprint 渲染器：配置后自动注册。** 创建 Blueprint 子类，设置 `AssociatedTag`，实现 `ReceiveDrawPointer`，并把 Blueprint 类加入 `UHelsincyDamageIndicatorSettings::BlueprintIndicatorRenderers`。Subsystem 会在初始化后异步加载并自动注册这些 Blueprint 类。现有 Blueprint renderer 会保留径向 fallback 行为；若需要完全一致的 WindowEdge 尺寸控制，推荐用 C++ renderer。

伤害指示器扩展也遵循同一套模式：组件只负责维护伤害来源、生命周期、角度和平滑状态；放置解析器负责把方向转换成 RadialCircle 或 WindowEdge 的最终屏幕位置；Renderer 只负责把“已解析的位置和朝向”画出来。因此新增 Arrow/Image/Arc 之外的样式时，不需要改动伤害事件系统或窗口边缘算法，只需要新增 Renderer、Tag 和对应配置。

Blueprint 自动注册的原因同样来自 DeveloperSettings：用户把 Blueprint Renderer 类加入设置后，Subsystem 会异步加载、读取 CDO 的 `AssociatedTag` 并注册。C++ 自定义 Renderer 必须手动注册，原因是 C++ 类不会天然出现在配置数组里，也不应依赖全局类扫描来寻找扩展点；显式注册可以让模块加载顺序、依赖关系和性能开销都保持可控。

### 9.3 接口扩展

插件没有强依赖宿主项目具体角色类，而是通过接口和引擎标准接口与项目交互，这种做法降低了耦合度，便于迁移到不同项目。

## 10. 架构上的已知边界

### 10.1 ~~DotOnly 仅有 Tag，暂无内置渲染器~~ (已实现)

`Crosshair.Shape.DotOnly` 已拥有对应的内置渲染器 `UHelsincyRendererDotOnly`，注册在 Subsystem 中。当前 10 个 ShapeTag 均有完整的渲染器实现。

### 10.2 ~~蓝图伤害指示器自动注册存在实现错误~~ (已修复)

在模块拆分重构中已修复。`UHelsincyDamageIndicatorSubsystem::OnIndicatorRenderBlueprintsLoaded` 现在正确调用 `RegisterDamageIndicatorRenderer`。

### 10.3 HUD 并未直接使用 IHelsincyCrosshairInterface 获取组件

当前 HUD 通过 FindComponentByClass 获取组件，IHelsincyCrosshairInterface 更适合给上层逻辑或你自己的角色体系使用，而不是内置 HUD 的硬依赖。

## 11. 架构总结

HelsincyCrosshair 的核心价值不在于形状数量本身，而在于它把“准心状态计算”“伤害方向状态计算”“屏幕绘制”“样式选择”“数据配置”“目标识别”拆成了可以独立演进的部分。

这种架构带来三点直接收益：

- **易于扩展**：新增样式时只需要添加 Renderer 与 GameplayTag，不需要改写核心组件或 HUD 绘制流程。
- **易于集成**：项目侧可以选择内置 HUD、Bridge API、ActorComponent、DataAsset、SaveGame 预设等不同接入深度。
- **高性能**：核心绘制基于 Canvas，运行时只维护必要状态，DataAsset 与 SaveGame 不参与每帧热路径，Renderer 查找通过 Subsystem 注册表完成。

因此它适合用于中大型射击项目中的基础 HUD 层，也为后续命中反馈、伤害指示器风格、武器/角色驱动配置扩展留出了足够空间。
