# HelsincyCrosshair 插件用户手册

> **版本**: 1.0  
> **引擎**: Unreal Engine 4.27  
> **作者**: Helsincy Games  
> **适用对象**: 蓝图用户 / C++ 用户 / 策划

---

## 目录

1. [插件简介](#1-插件简介)
2. [快速开始（5 分钟集成）](#2-快速开始5-分钟集成)
3. [集成方式详解](#3-集成方式详解)
4. [准星模块 — 完整用法](#4-准星模块--完整用法)
5. [伤害指示器模块 — 完整用法](#5-伤害指示器模块--完整用法)
6. [准星形状一览](#6-准星形状一览)
7. [配置参数详解](#7-配置参数详解)
8. [扩展指南 — 自定义准心形状](#8-扩展指南--自定义准心形状)
9. [扩展指南 — 自定义伤害指示器样式](#9-扩展指南--自定义伤害指示器样式)
10. [调试模式](#10-调试模式)
11. [常见问题 (FAQ)](#11-常见问题-faq)
12. [附录 — Blueprint API 速查表](#12-附录--blueprint-api-速查表)

---

## 1. 插件简介

**HelsincyCrosshair** 是一款面向 FPS/TPS 射击游戏的高性能动态准心与伤害方向指示器插件。

### 核心特点

- **高性能 Canvas 渲染** — 不使用 UMG，直接在 HUD Canvas 上绘制，性能开销极低
- **10 种内置准心形状** — 十字、圆形、T形、三角形、矩形、V形、多边形、双翼、图片、纯圆点
- **动态扩散系统** — 移动、跳跃、开火自动扩大准心，停下后平滑恢复
- **命中反馈标记** — 普通命中 / 爆头 / 击杀，支持自定义颜色、震动、尖头样式
- **目标识别变色** — 自动识别敌人（红色）、友军（绿色）、可交互物体（自定义颜色）
- **伤害方向指示器** — 受击时显示伤害来源方向，支持箭头、自定义图片和弧形三种样式
- **数据驱动** — 所有配置通过 DataAsset + 结构体管理，策划可直接调参
- **零侵入 Bridge API** — 无需修改你的 HUD 继承链，一行代码即可集成
- **蓝图完全可用** — 所有功能均可在蓝图中使用，无需编写任何 C++ 代码
- **可扩展** — 支持蓝图或 C++ 创建自定义准心形状和指示器样式

### 模块结构

| 模块 | 功能 |
|------|------|
| `HelsincyCrosshair` | 准心渲染、动态扩散、命中反馈、目标检测 |
| `HelsincyDamageIndicator` | 伤害方向指示器 |

两个模块完全独立，可以单独使用或同时使用。

---

## 2. 快速开始（5 分钟集成）

只需 4 步即可在你的项目中看到动态准心。伤害方向指示器是独立组件，如果也需要受击提示，按第 2 步和第 3 步中的可选项一起接入。

### 第 1 步：启用插件

打开编辑器 → **Edit → Plugins** → 搜索 "HelsincyCrosshair" → 勾选启用 → 重启编辑器。

### 第 2 步：在角色蓝图上添加组件

1. 打开你的**玩家角色蓝图**（Character 或 Pawn）。
2. 点击 **Add Component** → 搜索 `HelsincyCrosshairComponent` → 添加。
3. 在组件的 Details 面板中：
   - 将 `Default Crosshair Asset` 设为 `DA_Crosshair_Default`（插件自带的默认资产）。
   - 确保 `Use Default Crosshair Asset Init` 为 **true**。

> 如果你也想要伤害方向指示器，再添加一个 `HelsincyDamageIndicatorComponent`。
> 同时将它的 `Default Data Asset` 设为 `DA_DamageIndicator`，并确保 `Use Default Data Asset Init` 为 **true**。随后在角色的 **Any Damage** 或 **Take Damage** 事件中调用 `Register Damage Event`。

### 第 3 步：设置 HUD

**最简单的方式**（推荐新手）：

1. 打开你的 **GameMode** 蓝图。
2. 将 `HUD Class` 设为 `BP_HelsincySuperHUD`（插件自带）。

**如果你已有自己的 HUD 蓝图**：

1. 打开你的 HUD 蓝图。
2. 在 **Event ReceiveDrawHUD** 事件中：
3. 添加节点 `Draw Crosshair For HUD`（来自 HelsincyCrosshairRenderLibrary）。
4. 如果接入了伤害方向指示器，再添加节点 `Draw Damage Indicators For HUD`（来自 HelsincyDamageIndicatorRenderLibrary）。
5. 将 `self` 分别连接到这些节点的 `HUD` 输入引脚。

> 这是 **Bridge API** 方式。你不需要改变 HUD 的父类；准心和伤害指示器分别有独立的绘制入口。

### 第 4 步：运行测试

按 **Play** 运行游戏，你应该能在屏幕中心看到准心。

---

## 3. 集成方式详解

### 方式 A：使用插件自带的 HUD（最简单）

将 GameMode 的 `HUD Class` 设为 `BP_HelsincySuperHUD` 或 `AHelsincyHUD`。HUD 会自动处理所有绘制逻辑：准心、中心点、命中标记、伤害指示器。

**适合**：新项目、快速原型验证。

### 方式 B：Bridge API 无侵入集成（推荐）

在你自己的 HUD 蓝图或 C++ HUD 中，按需要调用对应的 Bridge 绘制入口：

**蓝图**：在 `Event ReceiveDrawHUD` 中使用 `Draw Crosshair For HUD` 节点；如果使用伤害方向指示器，再调用 `Draw Damage Indicators For HUD` 节点。

**C++**：
```cpp
void AMyHUD::DrawHUD()
{
    Super::DrawHUD();
    UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(this);
    UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(this);
}
```

**适合**：已有自定义 HUD 的项目。不修改继承链，零侵入。

---

## 4. 准星模块 — 完整用法

### 4.1 开火时 — 增加后坐力扩散

在武器开火逻辑中，调用准心组件的 **Add Recoil** 节点：

- `Horizontal Kick`：水平随机扩散幅度
- `Vertical Kick`：垂直上抬扩散幅度

**蓝图操作**：
1. 获取角色上的 `HelsincyCrosshairComponent`。
2. 调用 `Add Recoil`，传入水平和垂直数值。

**推荐数值**：

| 武器类型 | 水平 | 垂直 |
|----------|------|------|
| 步枪 | 1–3 | 3–6 |
| 冲锋枪 | 1–2 | 2–4 |
| 霰弹枪 | 3–6 | 4–8 |
| 手枪 | 1–2 | 2–4 |
| 狙击枪 | 5–10 | 8–15 |

> 数值需结合 `Dynamic Config → Max Recoil Spread` 和 `Recoil Recovery Speed` 来调整手感。

### 4.2 命中时 — 触发命中标记

在伤害结算成功后（不是射线命中时），调用对应节点：

| 节点 | 用途 | 颜色来源 |
|------|------|----------|
| `Trigger Hit Marker` | 普通命中 | `Hit Marker Config → Color` |
| `Trigger Hit Marker Color` | 自定义颜色命中 | 你传入的颜色参数 |
| `Trigger Headshot Marker` | 爆头命中 | `Hit Marker Config → Headshot Color` |
| `Trigger Kill Marker` | 击杀命中 | `Hit Marker Config → Kill Color` |

**蓝图操作**：
1. 在武器命中确认事件中，获取 `HelsincyCrosshairComponent`。
2. 根据命中类型调用对应的节点。

> 默认情况下，只要任意 HitMarker 可见，基础准心和中心圆点会临时隐藏，并在反馈结束后自动恢复。如需旧版 SingleInstance 透明度弱化效果，将 `CrosshairVisibilityWhileActive` 设为 `ScaleAlpha`；如需保持基础准心显示，设为 `KeepVisible`。

### 4.3 切换武器时 — 加载不同准心

每把武器可以配置不同的准心样式：

1. 为每种武器创建一个 `HelsincyCrosshairDataAsset`（右键 Content Browser → Miscellaneous → Data Asset → 选择 `HelsincyCrosshairDataAsset`）。
2. 在资产中配置该武器的准心参数。
3. 在切换武器时调用 `Load From Data Asset` 节点。

### 4.4 运行时开关控制

| 节点 | 功能 |
|------|------|
| `Enable Crosshair` / `Disable Crosshair` | 启用 / 关闭准心（例如：瞄准镜模式下关闭） |
| `Is Enabled Crosshair` | 查询当前准心是否启用 |
| `Enable Center Dot` / `Disable Center Dot` | 单独开关中心圆点 |
| `Enable Target Detection` / `Disable Target Detection` | 单独开关目标识别变色 |

### 4.5 目标识别与变色

插件会自动检测准心前方的目标并改变颜色。支持两种识别方式：

**方式一：阵营识别（GenericTeamAgentInterface）**

如果目标实现了 Unreal 引擎的 GenericTeamAgentInterface，准心会自动变色：
- 敌人（Hostile）→ 使用 `Visuals Config → Enemy Color`（默认红色）
- 友军（Friendly）→ 使用 `Visuals Config → Friendly Color`（默认绿色）
- 中立（Neutral）→ 可选使用 `Neutral Color`

**方式二：交互对象识别（IHelsincyTargetInterface）**

如果目标实现了 `IHelsincyTargetInterface` 接口并返回一个 GameplayTag：
- 检查 `Visuals Config → Interaction Color Map` 中是否有匹配项
- 如果有 → 使用映射的颜色
- 如果没有但启用了 `Use Default Interaction Color` → 使用默认交互色

**适合**：拾取物、可交互物体、终端、机关等。

### 4.6 预设系统

你可以在运行时保存和加载准心配置：

| 节点 | 功能 |
|------|------|
| `Save Preset` | 以一个名字保存当前配置到内存 |
| `Load Preset` | 根据名字加载已保存的配置（返回是否成功） |
| `Clear Preset Library` | 清空所有已保存的预设 |
| `Get All Preset Names` | 获取预设库中所有预设名称（方便构建 UI 列表） |

#### 磁盘持久化

插件内建了基于 USaveGame 的持久化方案，预设可以保存到磁盘，跨关卡和游戏重启后仍然保留：

| 节点 | 功能 |
|------|------|
| `Save Presets To Disk` | 将内存中的预设库 + 当前活跃预设名 保存到磁盘（默认槽位 `CrosshairPresets`） |
| `Load Presets From Disk` | 从磁盘恢复预设库，并自动应用上次活跃预设 |
| `Does Save Exist` | 检查磁盘上是否存在存档 |
| `Delete Save From Disk` | 删除磁盘上的存档 |

#### 自动持久化（零代码模式）

在组件 Details 面板中勾选以下属性，即可实现零代码自动保存/加载：

| 属性 | 默认值 | 说明 |
|------|--------|------|
| `Auto Load On Begin Play` | false | 开启后 BeginPlay 时自动从磁盘加载预设 |
| `Auto Save On End Play` | false | 开启后 EndPlay 时自动将预设保存到磁盘 |
| `Auto Save Slot Name` | `CrosshairPresets` | 自动保存/加载使用的槽位名称 |

> **提示**：存档文件保存在 `{ProjectDir}/Saved/SaveGames/CrosshairPresets.sav`。

### 4.7 查询当前状态

| 节点 | 返回值 |
|------|--------|
| `Get Current Profile` | 当前完整准心配置 |
| `Get Current Visual Primary Color` | 当前主颜色（已包含目标变色结果） |
| `Get Final Spread` | 当前总扩散 (X, Y) |
| `Get Current Target Attitude` | 当前目标阵营态度 (Hostile / Friendly / Neutral) |
| `Get Current Target Tag` | 当前目标的交互 GameplayTag |

---

## 5. 伤害指示器模块 — 完整用法

### 5.1 基础集成

1. 在角色蓝图上添加 `HelsincyDamageIndicatorComponent`。
2. 在组件的 Details 面板中，将 `Default Data Asset` 设为插件自带的 `DA_DamageIndicator`，并保持 `Use Default Data Asset Init = true`。
3. 在角色的 **Any Damage** 或 **Take Damage** 事件中，调用组件的 `Register Damage Event` 节点。
   - `Damage Causer`：伤害来源 Actor（如果有）
   - `Location If No Actor`：如果没有 Actor，传入伤害来源的世界坐标
4. 确保 HUD 有伤害指示器绘制：
   - **方式 A**（使用 `BP_HelsincySuperHUD`）：自动绘制，无需额外操作。
   - **方式 B**（Bridge）：在 HUD 中额外调用 `Draw Damage Indicators For HUD` 节点。

### 5.2 配置指示器外观

在组件的 Details 面板 → `Indicator Profile` 中配置：

- `Indicator Style Tag`：选择 `Indicator.Style.Arrow`（箭头）、`Indicator.Style.Image`（自定义图片）或 `Indicator.Style.Arc`（弧形）
- `Placement Mode`：选择 `Radial Circle` 或 `Window Edge`。
- `Edge Margin`：Window Edge 模式下距离游戏窗口 Canvas 边缘的安全边距。
- `Edge Corner Padding`：Window Edge 模式下靠近角落时的避让距离。
- `Hide Circle In Window Edge Mode`：Window Edge 模式下默认隐藏圆环。
- `Duration`：指示器显示时长
- `Radius`：圆环半径
- `Style|Arrow → Arrow Config → Size / Color`：箭头大小和颜色
- `Fade In Time` / `Fade Out Time`：淡入淡出时间
- `bShow Circle`：是否显示背景圆环

> `Window Edge` 使用的是当前游戏窗口/Viewport 的 Canvas 边界，也就是 `Canvas->ClipX / ClipY`，不是显示器物理屏幕边缘。窗口化运行时，指示器会跟随游戏窗口大小变化。

如果选择 `Indicator.Style.Image`，还需配置：
- `Style|Image → Image Config → Texture`：自定义图片
- `Style|Image → Image Config → Size`：图片大小
- `Style|Image → Image Config → bRotate Image`：是否跟随方向旋转

### 弧形伤害指示器

`Indicator.Style.Arc` 用于绘制类似 COD 的红色弧形受击提示。你可以提供自己的 `ArcMaskTexture`，也可以留空使用插件运行时生成的默认弧光遮罩。

推荐默认值：

- `Style|Arc → Arc Config → DirectionCueMode = CenterNib`
- `Style|Arc → Arc Config → DirectionCueStrength = 1.0`
- `PlacementMode = WindowEdge`

`CenterNib` 会在弧形中心绘制短小方向指针，避免玩家只能看到受击弧但无法判断敌人方向。

### 5.3 运行时控制

| 节点 | 功能 |
|------|------|
| `Is Enabled` | 查询指示器是否启用 |
| `Get Indicator Profile` | 获取当前配置 |
| `Set Indicator Profile` | 运行时修改配置 |
| `Load From Data Asset` | 从 `UHelsincyDamageIndicatorDataAsset` 加载整套配置 |
| `Save Preset` / `Load Preset` | 保存或加载内存预设 |
| `Save Presets To Disk` / `Load Presets From Disk` | 将伤害指示器预设持久化到磁盘，默认槽位为 `DamageIndicatorPresets` |

---

## 6. 准星形状一览

插件内置 10 种准心形状，通过 `ShapeTag` 字段选择：

| 形状 | ShapeTag | 关联配置 | 说明 |
|------|----------|----------|------|
| 十字 | `Crosshair.Shape.Cross` | CrosshairConfig | 经典 FPS 准心。4 方向臂 |
| 圆形 | `Crosshair.Shape.Circle` | RadialConfig | 散布圈。可调分段数和半径 |
| 纯圆点 | `Crosshair.Shape.DotOnly` | （使用 CenterDotConfig） | 最小化准心，仅显示中心点 |
| T 形 | `Crosshair.Shape.TStyle` | CrosshairConfig | 3 方向准心（无上臂） |
| 三角形 | `Crosshair.Shape.Triangle` | BoxConfig | 等腰三角形 |
| 矩形 | `Crosshair.Shape.Rectangle` | BoxConfig | 矩形框 |
| V 形 | `Crosshair.Shape.Chevron` | CrosshairConfig | V 字 / 人字型。可调张开角度 |
| 多边形 | `Crosshair.Shape.Polygon` | RadialConfig | 五边形、六边形等。可调边数 |
| 双翼 | `Crosshair.Shape.Wings` | WingsConfig | 两侧多段横线。适合滑翔 / 下落参考 |
| 自定义图片 | `Crosshair.Shape.Image` | ImageConfig | 使用你自己的贴图 |

### 如何选择形状

在 DataAsset 或组件的 `Current Profile → Shape Tag` 下拉菜单中，选择 `Crosshair.Shape.*` 下的对应 Tag 即可。

---

## 7. 配置参数详解

所有配置都在 `FHelsincyCrosshairProfile` 结构体下，可在 DataAsset 或组件的 Details 面板中编辑。

### 7.1 视觉设置 (Visuals Config)

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| Primary Color | 颜色 | 白色 | 准心基础颜色 |
| Global Crosshair Offset | 向量2D | (0, 0) | 准心相对屏幕中心的偏移。Y=-50 可让准心稍微靠上（适合 TPP） |
| Thickness | 浮点 | 2.0 | 线条粗细（像素）。范围 1–30 |
| Opacity | 浮点 | 1.0 | 全局不透明度。范围 0–1 |
| Enable Outline | 布尔 | true | 启用准心描边 |
| Outline Color | 颜色 | 黑色 | 描边颜色 |
| Outline Thickness | 浮点 | 1.0 | 描边宽度。范围 1–20 |
| Enable Target Switching | 布尔 | true | 启用目标检测变色 |
| Max Switching Distance | 浮点 | 10000 | 目标检测最大距离（厘米）。范围 5000–30000 |
| Switching Channel | 碰撞通道 | ECC_Pawn | 用于目标检测的碰撞通道 |
| Enemy Color | 颜色 | 红色 | 瞄准敌人时的颜色 |
| Friendly Color | 颜色 | 绿色 | 瞄准友军时的颜色 |
| Interaction Color Map | Tag→颜色映射表 | 空 | 自定义交互物体的 Tag→颜色对应关系 |

### 7.2 动态扩散设置 (Dynamic Config)

| 参数 | 默认值 | 说明 |
|------|--------|------|
| Enable Dynamic Spread | true | 总开关。关闭后准心不会随移动/射击扩散 |
| Velocity Multiplier | 0.05 | 移动速度→扩散的系数。值越大移动时扩散越大 |
| Airborne Penalty | 50.0 | 跳跃/滞空时的固定扩散惩罚 |
| State Interp Speed | 15.0 | 状态扩散的平滑速度。越大响应越快 |
| Recoil Recovery Speed | 10.0 | 后坐力恢复速度。越大恢复越快 |
| Max Recoil Spread | 100.0 | 最大后坐力限制（防止扩散出屏幕） |

### 7.3 命中标记设置 (Hit Marker Config)

| 参数 | 默认值 | 说明 |
|------|--------|------|
| Enabled | true | 启用命中标记 |
| `CrosshairVisibilityWhileActive` | Hide | HitMarker 可见期间的基础准心策略：`Hide`、`KeepVisible` 或 `ScaleAlpha` |
| `CrosshairAlphaScaleWhileHitMarkerActive` | 0.10 | 仅 `ScaleAlpha` 使用的透明度倍率，可保留旧版 SingleInstance 弱化效果 |
| `bApplyHitMarkerVisibilityPolicyToCenterDot` | true | 将同样的隐藏或透明度策略应用到中心圆点 |
| Use Tapered Shape | true | 使用尖头风格（类 CoD）。false 则为等宽线条 |
| Duration | 0.25 秒 | 命中标记持续时间 |
| Color | 白色 | 普通命中颜色 |
| Headshot Color | 红色 | 爆头颜色 |
| Head Shot Scale | 1.3 | 爆头时大小倍率 |
| Kill Color | 红色 | 击杀颜色 |
| Kill Scale | 1.7 | 击杀时大小倍率 |
| Shake Intensity | 5.0 | 全局震动强度（像素） |
| Normal Shake Intensity | 5.0 | 单臂法线震动强度（让标记看起来在"颤抖"） |
| Shake Frequency | 34.0 | 确定性震动频率。值越高回弹越快 |
| Shake Damping | 12.0 | 确定性震动阻尼。值越高收敛越快 |
| Headshot Shake Multiplier | 1.25 | 爆头命中注入的额外震动能量倍率 |
| Kill Shake Multiplier | 1.5 | 击杀命中注入的额外震动能量倍率 |
| Use Impact Motion | true | 启用 COD 风格扭转/缩放冲击运动 |
| Impact Rotation Degrees | 6.0 | 命中瞬间的基础扭转角度 |
| Impact Scale Punch | 0.18 | 命中瞬间整体放大的冲击量 |
| Impact Arm Length Punch | 0.14 | 命中瞬间四臂向外扩张的冲击量 |
| Impact Translation Weight | 0.25 | 辅助平移震动权重；主反馈仍是扭转和缩放 |
| Impact Motion Duration | 0.18 秒 | 主冲击运动时长，越短越硬 |
| Damage To Impact Scale | 0.35 | 伤害强度对冲击感的额外影响 |
| Max Impact Motion Energy | 1.8 | 高频命中时的最大冲击能量上限 |
| Shake Decay | true | 震动随时间衰减 |
| Custom Texture | 空 | 可选的自定义命中标记图片 |

### 7.4 中心点设置 (Center Dot Config)

| 参数 | 默认值 | 说明 |
|------|--------|------|
| Enabled | true | 显示中心圆点 |
| Always Stay Centered | true | 圆点始终在屏幕正中心（不受 GlobalCrosshairOffset 影响） |
| Size | 2.0 | 圆点半径（像素） |
| Opacity | 1.0 | 不透明度 |
| Color | 白色 | 圆点颜色 |

---

## 8. 扩展指南 — 自定义准心形状

你可以通过**蓝图**或 **C++** 创建全新的准心形状。

### 8.1 蓝图方式（无需 C++ 知识）

#### 第 1 步：创建渲染器蓝图

1. 在 Content Browser 中右键 → **Blueprint Class**。
2. 在 "All Classes" 中搜索 `HelsincyShapeRenderer` → 选择它作为父类。
3. 命名为例如 `BP_MyCustomCrosshair`。

#### 第 2 步：设置 Tag

1. 打开蓝图 → 在 Class Defaults 中找到 `Associated Tag`。
2. 设置一个自定义的 GameplayTag，例如 `Crosshair.Shape.MyCustom`。
   - 需要先在 **Project Settings → Gameplay Tags** 中创建这个 Tag。

#### 第 3 步：实现绘制逻辑

1. 在蓝图的 Event Graph 中找到 **Event On Draw Crosshair**（`ReceiveDraw`）。
2. 使用传入的参数进行绘制：
   - `Canvas`：用于绘制的画布
   - `Profile`：完整的准心配置（你可以读取颜色、粗细等）
   - `Spread`：当前扩散值 (X 水平, Y 垂直)
   - `Center`：屏幕中心坐标
   - `Final Color`：已经计算好的最终颜色（包含目标变色）
   - `Delta Time`：帧间隔时间
   - `Scale`：分辨率自适应缩放值

3. 在 Canvas 上调用 `Draw Line`、`Draw Texture` 等节点来实现你的自定义形状。

#### 第 4 步：注册渲染器

打开 **Edit → Project Settings → Engine → HelsincyCrosshairSystem**：

在 `Blueprint Renderers` 数组中添加你的蓝图类 `BP_MyCustomCrosshair`。

#### 第 5 步：使用

在 DataAsset 或组件的 `Shape Tag` 中选择 `Crosshair.Shape.MyCustom`，你的自定义准心就会生效。

### 8.2 C++ 方式

```cpp
UCLASS()
class UMyCustomRenderer : public UHelsincyShapeRenderer
{
    GENERATED_BODY()
public:
    UMyCustomRenderer()
    {
        AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.MyCustom"));
    }

    virtual void Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile,
        FVector2D Spread, FVector2D Center, FLinearColor Color,
        float DeltaTime, float Scale) override
    {
        // 你的自定义绘制逻辑
    }
};
```

C++ 渲染器不会仅因为类存在就自动注册。内置 C++ 渲染器由插件在 Subsystem 初始化时手动注册；项目侧自定义 C++ 渲染器也需要在合适的初始化时机调用注册函数，例如：

```cpp
if (GEngine)
{
    if (UHelsincyCrosshairManagerSubsystem* Manager =
        GEngine->GetEngineSubsystem<UHelsincyCrosshairManagerSubsystem>())
    {
        Manager->RegisterRenderer(
            FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.MyCustom")),
            UMyCustomRenderer::StaticClass());
    }
}
```

如果你希望完全通过编辑器配置，也可以创建继承 `UHelsincyShapeRenderer` 的蓝图类，并把该蓝图类加入 `HelsincyCrosshairSystem → Blueprint Renderers`。

---

## 9. 扩展指南 — 自定义伤害指示器样式

### 9.1 蓝图方式

#### 第 1 步：创建指示器渲染器蓝图

1. 右键 → Blueprint Class → 选择 `HelsincyIndicatorRenderer` 作为父类。
2. 命名为例如 `BP_MyDamageIndicator`。

#### 第 2 步：设置 Tag

在 Class Defaults 中设置 `Associated Tag` 为自定义的 Tag，例如 `Indicator.Style.MyCustom`。

#### 第 3 步：实现绘制逻辑

在 **Event On Draw Pointer**（`ReceiveDrawPointer`）中使用传入参数绘制：

- `Canvas`：画布
- `Profile`：伤害指示器配置
- `Center`：当前 Canvas/游戏窗口中的旧 radial 中心
- `Angle`：方向角度（0=上, 90=右, 180=下, 270=左）
- `Alpha`：当前透明度（已计算淡入淡出）
- `Scale`：分辨率缩放

> 兼容说明：旧自定义 renderer 覆盖的 `DrawPointer` / 蓝图 `ReceiveDrawPointer` 仍会编译并保持 radial fallback 行为。若 C++ renderer 需要真实 `Window Edge` 放置，应覆盖 `GetDesiredPointerSize` / `DrawPointerResolved`。

#### 第 4 步：注册

在 **Project Settings → HelsincyDamageIndicatorSystem** 中将蓝图类添加到 `Blueprint Indicator Renderers` / `BlueprintIndicatorRenderers`。

#### 第 5 步：使用

在 `DamageIndicatorComponent → Indicator Profile → Indicator Style Tag` 中选择你的 Tag。

### 9.2 C++ 方式

```cpp
UCLASS()
class UMyDamageIndicator : public UHelsincyIndicatorRenderer
{
    GENERATED_BODY()
public:
    UMyDamageIndicator()
    {
        AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.MyCustom"));
    }

    virtual void DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile,
        FVector2D Center, float Angle, float Alpha, float Scale) override
    {
        // 你的自定义绘制逻辑
    }
};
```

C++ 指示器渲染器同样需要手动注册，或者改用蓝图 renderer 并加入 `HelsincyDamageIndicatorSystem → Blueprint Indicator Renderers`。

```cpp
if (GEngine)
{
    if (UHelsincyDamageIndicatorSubsystem* Manager =
        GEngine->GetEngineSubsystem<UHelsincyDamageIndicatorSubsystem>())
    {
        Manager->RegisterDamageIndicatorRenderer(
            FGameplayTag::RequestGameplayTag(FName("Indicator.Style.MyCustom")),
            UMyDamageIndicator::StaticClass());
    }
}
```

如果你的 C++ 指示器需要真正支持 `Window Edge` 放置模式，建议额外覆盖 `GetDesiredPointerSize` 和 `DrawPointerResolved`；只覆盖 `DrawPointer` 会保持旧 radial fallback 语义。

---

## 10. 调试模式

插件内置了完整的调试系统，帮助你在开发阶段快速排查问题。

### 10.1 ShowDebug 面板

在游戏运行时，打开控制台（按 `~` 键）输入：

| 命令 | 功能 |
|------|------|
| `showdebug Crosshair` | 显示准心调试面板。包含：当前 Profile、扩散值、颜色、目标检测、命中标记、子系统状态 |
| `showdebug DamageIndicator` | 显示伤害指示器调试面板。包含：配置、活跃指示器列表、圆环参数、子系统状态 |

### 10.2 CVar 控制台变量

#### 准星调试

| 变量 | 默认 | 说明 |
|------|------|------|
| `hc.Debug.Enable` | 0 | 主开关。设为 1 开启准星调试系统 |
| `hc.Debug.Text` | 1 | 开启屏幕文字调试输出 |
| `hc.Debug.Geometry` | 0 | 开启几何可视化（在屏幕上绘制调试线条和框） |
| `hc.Debug.VerboseLog` | 0 | 开启详细日志输出到 Output Log |
| `hc.Debug.OnlyLocal` | 1 | 仅对本地控制角色输出调试信息 |

#### 伤害指示器调试

| 变量 | 默认 | 说明 |
|------|------|------|
| `di.Debug.Enable` | 0 | 主开关。设为 1 开启指示器调试系统 |
| `di.Debug.Text` | 1 | 开启屏幕文字调试输出 |
| `di.Debug.Geometry` | 0 | 开启几何可视化 |
| `di.Debug.VerboseLog` | 0 | 开启详细日志输出 |

### 10.3 性能统计

插件接入了 UE4 自带的 `STAT` 性能统计系统。运行游戏时打开控制台，输入：

| 命令 | 功能 |
|------|------|
| `stat HelsincyCrosshair` | 显示准星模块的 Tick、HUD 绘制、Bridge 绘制、Renderer 绘制、HitMarker 绘制和目标检测开销 |
| `stat HelsincyDamageIndicator` | 显示伤害指示器模块的 Tick、指示器更新、Bridge 绘制和指示器绘制开销 |

常见统计项含义：

| 统计项 | 含义 |
|--------|------|
| `Crosshair Component Tick` | 准星组件每帧状态更新，包括扩散、命中反馈状态和目标检测调度 |
| `Crosshair HUD Draw` | `AHelsincyHUD::DrawHUD` 中准星绘制路径的整体开销 |
| `Crosshair Bridge Draw` | Bridge API 准星绘制入口的整体开销 |
| `Crosshair Renderer Draw` | 当前准星形状 renderer 的绘制开销 |
| `HitMarker Draw` | 命中反馈绘制开销 |
| `Target Detection Request` / `Target Detection Callback` | 异步目标检测请求和回调处理开销 |
| `DamageIndicator Component Tick` | 伤害指示器组件每帧状态更新 |
| `DamageIndicator Update Indicators` | 活跃伤害方向实例的生命周期、角度和平滑更新 |
| `DamageIndicator Bridge Draw` | Bridge API 伤害指示器绘制入口的整体开销 |
| `DamageIndicator Draw Indicators` | 圆环、WindowEdge/RadialCircle 指示器和具体 renderer 绘制开销 |

这些统计项适合在开发版或测试版中排查 HUD 开销。需要关闭显示时，再次输入对应的 `stat` 命令即可切换显示状态。

### 10.4 使用示例

```
~ 打开控制台
hc.Debug.Enable 1       -- 开启准星调试
hc.Debug.Geometry 1     -- 显示扩散框和中心参考线
showdebug Crosshair     -- 显示完整调试面板
stat HelsincyCrosshair  -- 显示准星性能统计
```

> 调试功能在 Shipping 构建中不可用（零运行时开销）。

---

## 11. 常见问题 (FAQ)

### Q1：看不到准心

**排查清单**：
1. ✅ HUD 是否设为 `BP_HelsincySuperHUD`，或者你的 HUD 是否调用了 `Draw Crosshair For HUD`？
2. ✅ 角色蓝图上是否添加了 `HelsincyCrosshairComponent`？
3. ✅ 该角色是否为本地控制的 Pawn？（多人游戏中只有本地玩家才显示准心）
4. ✅ 组件的 `Enabled Crosshair` 是否为 true？
5. ✅ `Current Profile → Shape Tag` 是否选择了有效的 Tag（如 `Crosshair.Shape.Cross`）？

### Q2：准心不随移动 / 开火扩散

**排查清单**：
1. ✅ `Dynamic Config → Enable Dynamic Spread` 是否为 true？
2. ✅ 开火时是否调用了 `Add Recoil` 节点？
3. ✅ 角色是否有 `CharacterMovementComponent`？（需要它来计算速度和跳跃状态）

### Q3：看向目标不变色

**排查清单**：
1. ✅ `Visuals Config → Enable Target Switching` 是否为 true？
2. ✅ `Switching Channel` 碰撞通道是否能命中目标？（默认为 ECC_Pawn）
3. ✅ 目标是否实现了 `GenericTeamAgentInterface` 或 `IHelsincyTargetInterface`？
4. ✅ 准心与目标之间是否有障碍物遮挡？

### Q4：伤害指示器不出现

**排查清单**：
1. ✅ 角色蓝图上是否添加了 `HelsincyDamageIndicatorComponent`？
2. ✅ `Default Data Asset` 是否指向 `DA_DamageIndicator`，或 `Indicator Profile → Enabled` 是否为 true？
3. ✅ `Indicator Style Tag` 是否为有效样式（例如 `Indicator.Style.Arc` / `Indicator.Style.Arrow`）？
4. ✅ 受伤时是否调用了 `Register Damage Event` 节点？
5. ✅ HUD 中是否有伤害指示器绘制调用？（方式 A 自动处理，方式 B 需手动添加 `Draw Damage Indicators For HUD`）

### Q5：命中标记不出现

**排查清单**：
1. ✅ `Hit Marker Config → Enabled` 是否为 true？
2. ✅ 命中确认后是否调用了 `Trigger Hit Marker` 节点？
3. ✅ 确认是在伤害结算成功后调用的，而不是仅仅是射线命中。

### Q6：如何让不同武器用不同准心？

1. 为每种武器创建一个 `HelsincyCrosshairDataAsset`。
2. 在武器 Actor 上用一个变量引用对应的 DataAsset。
3. 在装备/切换武器时，调用准心组件的 `Load From Data Asset` 节点。

### Q7：准心在多人游戏中所有玩家都能看到吗？

不会。准心组件只对**本地控制的角色**启用 Tick。远端玩家的准心组件不会工作，也不会产生任何性能开销。

### Q8：可以在蓝图中创建自定义准心形状吗？

可以！请参阅 [第 8 章](#8-扩展指南--自定义准心形状) 的蓝图扩展指南。只需 5 步即可注册一个全新的准心形状。

### Q9：支持 UMG Widget 准心吗？

不支持。本插件专注于 Canvas 高性能渲染路径，这是 FPS 游戏中准心的最佳实践。UMG Widget 的额外开销对高帧率 FPS 游戏不理想。

### Q10：Shipping 构建中调试功能会有性能开销吗？

不会。所有调试代码都包裹在 `#if !UE_BUILD_SHIPPING` 中，Shipping 构建中完全不包含调试代码，零开销。

---

## 12. 附录 — Blueprint API 速查表

### 准星组件 (UHelsincyCrosshairComponent)

| 节点名 | 类型 | 说明 |
|--------|------|------|
| Enable Crosshair | Callable | 启用准心 |
| Disable Crosshair | Callable | 关闭准心 |
| Is Enabled Crosshair | Pure | 查询是否启用 |
| Enable Center Dot | Callable | 启用中心点 |
| Disable Center Dot | Callable | 关闭中心点 |
| Enable Target Detection | Callable | 启用目标检测 |
| Disable Target Detection | Callable | 关闭目标检测 |
| Add Recoil | Callable | 增加后坐力扩散 |
| Trigger Hit Marker | Callable | 触发普通命中标记 |
| Trigger Hit Marker Color | Callable | 触发自定义颜色命中 |
| Trigger Headshot Marker | Callable | 触发爆头命中标记 |
| Trigger Kill Marker | Callable | 触发击杀命中标记 |
| Trigger Hit Marker Advanced | Callable | 触发带优先级、颜色和伤害强度缩放的命中反馈 |
| Load From Data Asset | Callable | 从 DataAsset 加载配置 |
| Update Profile | Callable | 直接覆盖当前配置 |
| Save Preset | Callable | 保存预设到内存 |
| Load Preset | Callable | 从内存加载预设 |
| Clear Preset Library | Callable | 清空预设库 |
| Get All Preset Names | Pure | 获取所有预设名称 |
| Save Presets To Disk | Callable | 将预设库保存到磁盘 |
| Load Presets From Disk | Callable | 从磁盘恢复预设库 |
| Does Save Exist | Pure | 检查磁盘存档是否存在 |
| Delete Save From Disk | Callable | 删除磁盘存档 |
| Get Current Profile | Pure | 获取当前配置 |
| Get Current Visual Primary Color | Pure | 获取当前颜色 |
| Get Crosshair Presentation State | Pure | 获取基础准星自适应表现状态 |
| Get Final Spread | Pure | 获取总扩散 |
| Get State Spread | Pure | 获取状态扩散 |
| Get Recoil Spread | Pure | 获取后坐力扩散 |
| Get Current Target Attitude | Pure | 获取目标阵营态度 |
| Get Current Target Tag | Pure | 获取目标交互 Tag |
| Is Single Hit Marker Visible | Pure | 查询单实例命中准星是否可见 |
| Get Single Hit Marker Phase | Pure | 获取单实例命中准星当前相位 |
| Get Single Hit Marker Opacity | Pure | 获取单实例命中准星当前透明度 |
| Get Single Hit Marker Impact Energy | Pure | 获取单实例命中准星当前命中能量 |

### 伤害指示器组件 (UHelsincyDamageIndicatorComponent)

| 节点名 | 类型 | 说明 |
|--------|------|------|
| Register Damage Event | Callable | 注册受伤事件 |
| Set Indicator Profile | Callable | 修改配置 |
| Get Indicator Profile | Pure | 获取配置 |
| Is Enabled | Pure | 查询是否启用 |
| Load From Data Asset | Callable | 从伤害指示器 DataAsset 加载配置 |
| Save Preset | Callable | 保存伤害指示器预设到内存 |
| Load Preset | Callable | 从内存加载伤害指示器预设 |
| Clear Preset Library | Callable | 清空伤害指示器预设库 |
| Get All Preset Names | Pure | 获取所有伤害指示器预设名称 |
| Save Presets To Disk | Callable | 将伤害指示器预设库保存到磁盘 |
| Load Presets From Disk | Callable | 从磁盘恢复伤害指示器预设库 |
| Does Save Exist | Pure | 检查伤害指示器磁盘存档是否存在 |
| Delete Save From Disk | Callable | 删除伤害指示器磁盘存档 |

### Bridge API（蓝图函数库）

| 节点名 | 所属库 | 说明 |
|--------|--------|------|
| Draw Crosshair For HUD | CrosshairRenderLibrary | 在 HUD 中绘制准心 |
| Draw Crosshair For Controller | CrosshairRenderLibrary | 底层渲染入口 |
| Draw Damage Indicators For HUD | DamageIndicatorRenderLibrary | 在 HUD 中绘制伤害指示器 |
| Draw Damage Indicators For Controller | DamageIndicatorRenderLibrary | 底层渲染入口 |
