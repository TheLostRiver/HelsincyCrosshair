# HelsincyCrosshair 已知问题与限制

本文件记录当前仓库源码中可以确认的实现边界、集成注意事项和已解决历史，目的是避免集成人员基于旧设计稿或错误预期使用插件。

## 当前限制与注意事项

### 1. 自定义 HUD 仍需要明确接入伤害指示器绘制入口

#### 现象

`UHelsincyDamageIndicatorComponent` 只负责保存伤害事件、生命周期、角度和平滑状态；真正绘制需要 HUD 或 Bridge 在有效 Canvas 绘制阶段调用：

- `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(AHUD* HUD)`
- `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForController(APlayerController*, UCanvas*)`

内置 `AHelsincyHUD` 已在 `DrawHUD()` 中调用 `DrawDamageIndicatorsForHUD(this)`，因此使用插件内置 HUD 时会自动尝试绘制运行时伤害指示器。自定义 HUD 或项目已有 HUD 仍需要显式调用 Bridge 绘制入口。

#### 影响

如果角色上已经挂载 `UHelsincyDamageIndicatorComponent`，并且受伤时也调用了 `RegisterDamageEvent()`，但自定义 HUD 没有调用 Damage Indicator Bridge 绘制入口，屏幕上仍然不会出现指示器。

#### 建议

正式项目优先使用 Bridge API，在已有 HUD 的 `DrawHUD()` 中显式调用：

```cpp
UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(this);
UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(this);
```

如果使用插件内置 `AHelsincyHUD`，无需额外手动调用；如果使用蓝图子类或自定义 HUD，请确认对应路径没有覆盖掉父类绘制，或自行调用 `DrawDamageIndicatorsForHUD`。

### 2. Blueprint 自定义伤害指示器 Renderer 对 WindowEdge 仅保持兼容 fallback

#### 现象

伤害指示器的 `WindowEdge` 放置模式依赖 renderer 上报尺寸并消费解析后的屏幕位置。内置 `Arrow`、`Image`、`Arc` renderer 已覆盖 `GetDesiredPointerSize()` 和 `DrawPointerResolved()`。

为了兼容旧扩展，Blueprint `ReceiveDrawPointer` 和旧 C++ `DrawPointer()` 仍然保留，但它们保持 radial fallback 行为，不会天然获得完整的 `WindowEdge` 尺寸解析能力。

#### 影响

自定义 Blueprint 指示器 renderer 在 `RadialCircle` 下通常可以正常工作；但在 `WindowEdge` 下，边缘贴合、角落避让和真实外接尺寸可能与内置 renderer 不完全一致。

#### 建议

如果自定义伤害指示器需要完整支持 `WindowEdge`：

1. 使用 C++ 继承 `UHelsincyIndicatorRenderer`。
2. 覆盖 `GetDesiredPointerSize()`，返回指示器真实绘制尺寸。
3. 覆盖 `DrawPointerResolved()`，使用解析后的 `FHelsincy_ResolvedDamageIndicatorPlacement` 绘制。
4. 通过 `RegisterDamageIndicatorRenderer(Tag, RendererClass)` 手动注册该 C++ renderer。

Blueprint renderer 更适合快速试验、RadialCircle 样式或不严格依赖窗口边缘尺寸的场景。

### 3. IHelsincyCrosshairInterface 当前不是内置 HUD 的强依赖

#### 现象

`AHelsincyHUD` 当前通过 `FindComponentByClass<UHelsincyCrosshairComponent>()` 获取组件，而不是通过 `IHelsincyCrosshairInterface`。

#### 影响

即使角色没有实现该接口，内置 HUD 仍可能正常工作；反过来，仅实现接口但未挂组件，也不会让 HUD 正常绘制。

#### 建议

把这个接口理解为项目扩展接口，而不是内置 HUD 的唯一接入条件。正式接入时仍应在本地玩家 Pawn 或 Character 上添加 `UHelsincyCrosshairComponent`。

### 4. 目标识别效果强依赖碰撞通道设置

#### 现象

目标识别通过 `AsyncLineTraceByChannel` 执行，命中的前提是：

- Trace 起点正确
- 检测通道正确
- 目标碰撞设置允许被该通道命中

#### 影响

很多“敌人不变色”“交互物不变色”的问题，根因并不在颜色逻辑，而在碰撞配置。

#### 建议

优先检查 `VisualsConfig.SwitchingChannel` 与目标碰撞响应，而不是先怀疑颜色映射本身。调试时可结合 `showdebug Crosshair` 查看目标识别状态。

### 5. 当前插件以 UE4.27 为主要验证环境

#### 现象

插件描述文件 `HelsincyCrosshair.uplugin` 声明的 `EngineVersion` 为 `4.27.0`，当前源码实现也保持 UE4.27 兼容写法。

#### 影响

迁移到 UE5 系列通常可行，但不应在没有验证的前提下直接承诺完全兼容。尤其是 Canvas、UHT 生成代码、模块加载阶段、SaveGame 行为和编辑器资产设置，仍需在目标 UE5 版本中回归验证。

#### 建议

如果要正式用于 UE5 项目，建议至少完成一次：

1. 插件编译验证。
2. Editor Development / Game Development / Game Shipping 构建验证。
3. Legacy HUD 与 Bridge API 的运行时绘制验证。
4. DataAsset、SaveGame、WindowEdge、Arc 指示器的视觉回归测试。

## 已解决历史

### 1. DotOnly Tag 已定义，但曾经没有内置渲染器

**已修复** — 已新增 `UHelsincyRendererDotOnly` 渲染器并在 Subsystem 中注册。当前 10 个内置准心渲染器已全部实现并注册。

### 2. 蓝图伤害指示器自动注册链路曾存在实现错误

**已修复** — 在模块拆分重构中，伤害指示器系统已迁移到独立的 `HelsincyDamageIndicator` 模块。Blueprint 指示器渲染器的配置入口已迁移到 `UHelsincyDamageIndicatorSettings::BlueprintIndicatorRenderers`，Subsystem 会异步加载并自动注册配置中的 Blueprint renderer。

### 3. README 与历史设计文档中的“10 种准心”说法曾不完全对应当前实现

**已修复** — 10 个内置准心渲染器已全部实现并注册（含 DotOnly），README 已同步更新。旧版 ACS 设计文档不再作为当前项目标准。

### 4. 运行时预设系统曾仅在内存中生效

**已修复** — 准星组件和伤害指示器组件均已提供 `SavePresetsToDisk()` / `LoadPresetsFromDisk()` 方法（基于 `USaveGame`），以及 `bAutoLoadOnBeginPlay` / `bAutoSaveOnEndPlay` 自动持久化属性。预设现在可以跨关卡和游戏重启保留。

### 5. DamageIndicatorComponent 曾缺少本地真人玩家守卫

**已修复** — `UHelsincyCrosshairComponent` 和 `UHelsincyDamageIndicatorComponent` 均已实现本地玩家守卫。Controller 延迟就位时不会再永久失活，而是保持 Pending 并低频重试；最终通过 `Cast<APlayerController>` + `IsLocalController()` 精确判定本地真人玩家，排除 AI Controller、远端玩家和模拟代理。
