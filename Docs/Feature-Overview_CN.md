# HelsincyCrosshair 功能总览

`HelsincyCrosshair` 是一套面向 Unreal Engine 射击项目的动态准心与独立伤害方向指示器插件。它的定位不是“画一个准星”，而是提供一整套可调、可扩展、可持久化、可直接接入真实项目 HUD 流程的表现系统。

当前插件由两个 Runtime 模块组成：

- `HelsincyCrosshair`：负责准心、中心点、动态扩散、后坐力恢复、目标识别变色、命中反馈与准心预设。
- `HelsincyDamageIndicator`：负责独立伤害方向提示、受击来源追踪、窗口边缘/圆形放置、Arc/Image/Arrow 样式与伤害指示器预设。

## 参数规模

插件核心 Profile 结构已经暴露 **170+ 个可编辑配置字段**。这个数字来自源码中 `FHelsincyCrosshairProfile` 及其子结构、`FHelsincy_DamageIndicatorProfile` 及其子结构的 `EditAnywhere` 配置项统计。

这些字段并不是堆数量，而是围绕实际 HUD 调参场景拆分：

| 参数区域 | 覆盖内容 |
|----------|----------|
| 准心基础视觉 | 主颜色、描边、透明度、线宽、全局偏移 |
| 目标识别 | 敌人、友军、中立、交互物 GameplayTag 到颜色映射 |
| 动态扩散 | 移动扩散、跳跃惩罚、落地恢复、后坐力扩散与恢复 |
| 形状参数 | 线段、圆形、多边形、矩形、三角形、Chevron、Wings、Image |
| 中心点 | 启用、大小、透明度、颜色、是否强制居中 |
| 命中反馈 | 普通命中、爆头、击杀、单实例/多实例、震动、缩放、扭转、图片层 |
| 伤害指示器 | 样式、持续时间、淡入淡出、圆环、窗口边缘、箭头、图片、Arc 弧光 |
| 持久化 | 内存预设、磁盘保存、自动加载、自动保存 |

## 准心系统

插件内置 10 个已注册的准心渲染器：

- Cross
- Circle
- TStyle
- Triangle
- Rectangle
- Chevron
- Polygon
- Wings
- Image
- DotOnly

这些形状不是静态图片替换，而是由统一 Profile 驱动的运行时渲染器。你可以按武器、角色、姿态、装备状态或玩法模式切换 DataAsset，也可以直接在运行时更新 Profile。

典型用途包括：

- 竞技 FPS 的动态扩散准心
- TPS 的偏移准心
- 霰弹枪、狙击枪、榴弹、技能武器等特殊准心
- 拾取物、机关、终端等交互目标高亮
- 按敌友关系自动变色的战斗 HUD

## 动态扩散与后坐力

`FHelsincy_DynamicsSettings` 提供移动、跳跃、落地和射击后坐力相关配置。准心可以根据角色速度、滞空状态和开火事件动态扩大，并在停止移动或后坐力恢复后平滑收敛。

这让插件可以表达更接近真实射击手感的准星状态，而不是只有固定图案：

- 移动时扩大
- 滞空时增加惩罚
- 落地后平滑恢复
- 开火后注入后坐力扩散
- 分别控制 X/Y 轴扩散
- 限制最大后坐力，避免视觉失控

## 命中反馈

命中反馈系统支持普通命中、爆头、击杀和高级命中事件。默认走 COD 风格的单实例刷新路径，也保留经典多实例叠加模式。

可调能力包括：

- 命中颜色、爆头颜色、击杀颜色
- 命中持续时间和合并阈值
- 击杀时清理旧反馈
- 针状线条和辉光
- 整体震动与单臂法线震动
- 震动频率、阻尼、衰减
- 爆头/击杀额外震动能量
- 命中瞬间扭转、缩放和臂长冲击
- 根据伤害强度改变反馈力度
- 命中反馈期间隐藏、保留或降低基础准心透明度
- 可选图片 HitMarker 与 Core/Glow 双层 Sprite 路径

## 伤害方向指示器

`HelsincyDamageIndicator` 是独立 Runtime 模块，不是准心模块里的附属小功能。它可以单独挂载、单独配置、单独持久化。

内置样式：

- `Indicator.Style.Arrow`
- `Indicator.Style.Image`
- `Indicator.Style.Arc`

放置模式：

- `RadialCircle`：围绕屏幕中心的圆形轨道。
- `WindowEdge`：沿当前游戏 Canvas / 窗口安全边界放置，适合窗口化和全屏模式。

Arc 样式支持弧光贴图、柔光层、冲击缩放，以及 CenterChevron、AsymmetricTaper、CenterNib 等方向提示模式。对于希望做现代 FPS 受击反馈的项目，它比单纯箭头更接近成品游戏的视觉语言。

## 数据驱动与预设

插件围绕 DataAsset 和 Runtime Profile 设计：

- 默认外观由 `UHelsincyCrosshairDataAsset` / `UHelsincyDamageIndicatorDataAsset` 管理。
- 运行时组件持有当前 Profile，可以直接更新或从 DataAsset 加载。
- 支持按名称保存内存预设。
- 内建 `USaveGame` 磁盘持久化。
- 支持自动加载和自动保存。

这意味着策划可以在编辑器里制作多个准心/伤害指示器资产，程序可以在武器切换、角色状态变化、瞄准模式切换时加载不同配置。

## 集成与扩展

插件提供两条集成路径：

- Legacy HUD：直接使用 `AHelsincyHUD` 或 `BP_HelsincySuperHUD`。
- Bridge API：在已有 HUD 中调用绘制函数，不修改 HUD 继承链。

扩展路径同样保留：

- 使用 GameplayTag 选择内置或自定义 Renderer。
- 蓝图可继承 Renderer 类创建自定义准心或伤害指示器。
- C++ 可注册新的渲染器类。
- `IHelsincyTargetInterface` 可用于交互物识别和自定义颜色反馈。

## 性能与调试

插件使用 `Canvas` / `FCanvas` 绘制路径，避免依赖 UMG 主路径。调试和性能入口包括：

- `showdebug Crosshair`
- `showdebug DamageIndicator`
- `stat HelsincyCrosshair`
- `stat HelsincyDamageIndicator`
- 准心与伤害指示器专用 Debug CVar

这些能力让插件不仅能跑起来，也方便在项目接入、调参、性能检查和问题定位时持续维护。

## 下一步阅读

- [中文用户手册](UserManual_CN.md)：从启用插件到完整蓝图/C++ 用法。
- [集成指南](Integration-Guide.md)：按工程步骤接入准心与伤害指示器。
- [配置参考手册](Configuration-Reference.md)：字段级参数说明。
- [扩展开发指南](Extension-Guide.md)：自定义准心和伤害指示器渲染器。
- [项目架构文档](Architecture.md)：模块边界、数据流和渲染链路。
