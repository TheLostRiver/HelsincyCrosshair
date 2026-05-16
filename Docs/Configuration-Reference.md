# HelsincyCrosshair 配置参考手册

## 1. 配置总览

插件的核心配置对象是 FHelsincyCrosshairProfile。它可以存在于：

- UHelsincyCrosshairDataAsset 中，作为编辑器资产
- UHelsincyCrosshairComponent::CurrentProfile 中，作为运行时配置

Profile 由多个子结构组成，下面按用途说明。

## 2. 核心字段

### 2.1 ShapeTag

类型：FGameplayTag

作用：指定当前准心主形状。

当前源码已内置注册的形状：

- Crosshair.Shape.Cross
- Crosshair.Shape.Circle
- Crosshair.Shape.TStyle
- Crosshair.Shape.Triangle
- Crosshair.Shape.Rectangle
- Crosshair.Shape.Chevron
- Crosshair.Shape.Polygon
- Crosshair.Shape.Wings
- Crosshair.Shape.Image
- Crosshair.Shape.DotOnly

## 3. FHelsincy_VisualSettings

这部分控制准心的基础视觉表现与目标识别变色。

| 字段 | 说明 | 建议 |
|------|------|------|
| PrimaryColor | 基础颜色 | 常规白色或浅绿色 |
| GlobalCrosshairOffset | 全局屏幕偏移 | TPS 可适当上移 |
| Thickness | 主线条厚度 | 2 到 4 最常用 |
| Opacity | 整体透明度 | 0.8 到 1.0 |
| bEnableOutline | 是否启用描边 | 明亮背景建议开启 |
| OutlineColor | 描边颜色 | 通常黑色 |
| OutlineThickness | 描边额外宽度 | 1 到 2 |
| bEnableTargetSwitching | 是否启用目标识别 | 敌友识别项目建议开启 |
| MaxSwitchingDistance | 最大识别距离 | 8000 到 15000 |
| SwitchingChannel | 检测通道 | 结合项目碰撞通道设置 |
| EnemyColor | 敌对颜色 | 红色 |
| FriendlyColor | 友军颜色 | 绿色 |
| bUseNeutralColor | 是否启用中立色 | 有中立目标时开启 |
| NeutralColor | 中立颜色 | 黄色或蓝色 |
| InteractionColorMap | 交互 Tag 到颜色的映射 | 适合拾取物、终端、可互动机关 |
| bUseDefaultInteractionColor | Tag 未命中映射时的默认交互色开关 | 建议开启 |
| DefaultInteractionColor | 默认交互色 | 黄色较醒目 |

## 4. FHelsincy_DynamicsSettings

这部分控制动态扩散与后坐力恢复。

| 字段 | 说明 | 建议 |
|------|------|------|
| bEnableDynamicSpread | 是否启用动态扩散 | 射击游戏通常开启 |
| VelocityMultiplier | 移动速度乘数 | 0.03 到 0.08 |
| AirbornePenalty | 滞空惩罚 | 30 到 60 |
| JumpExpandSpeed | 扩散增大速度 | 15 到 25 |
| LandRecoverySpeed | 落地恢复速度 | 8 到 15 |
| bApplyMovementToX | 是否把移动扩散作用到 X | 开启 |
| bApplyMovementToY | 是否把移动扩散作用到 Y | 开启 |
| RecoilRecoverySpeed | 后坐力恢复速度 | 8 到 15 |
| MaxRecoilSpread | 后坐力最大扩散 | 40 到 100 |

## 5. 形状配置

### 5.1 FHelsincy_CrosshairConfig

适用于 Cross、TStyle、Chevron 一类线段形状。

| 字段 | 说明 |
|------|------|
| Length | 臂长 |
| Gap | 中心间距 |
| Rotation | 整体旋转 |
| ChevronOpeningAngle | V 形开口角，仅 Chevron 使用 |

### 5.2 FHelsincy_RadialConfig

适用于 Circle 与 Polygon。

| 字段 | 说明 |
|------|------|
| Radius | 半径 |
| Rotation | 旋转偏移 |
| CircleSegments | 圆形分段数 |
| PolygonSides | 多边形边数 |

### 5.3 FHelsincy_BoxConfig

适用于 Rectangle 和 Triangle 一类简单封闭形状。

| 字段 | 说明 |
|------|------|
| Size | 宽高 |
| Rotation | 旋转角度 |

### 5.4 FHelsincy_WingsConfig

适用于双翼或梯形准心。

| 字段 | 说明 |
|------|------|
| bWingsAlignTop | 顶端对齐模式 |
| WingsVerticalOffset | 整体垂直偏移 |
| WingsLineCount | 横线数量 |
| WingsVerticalSpacing | 横线间距 |
| WingsTopLength | 顶部长线长度 |
| WingsBottomLength | 底部长线长度 |
| bWingsDrawVerticalLines | 是否绘制竖线 |

### 5.5 FHelsincy_ImageConfig

适用于图片准心。

| 字段 | 说明 |
|------|------|
| Texture | 准心贴图 |
| Size | 图片尺寸 |
| Tint | 图片染色 |

## 6. 中心点配置

FHelsincy_CenterDotConfig 控制中心点。

| 字段 | 说明 | 建议 |
|------|------|------|
| bEnabled | 是否启用中心点 | 红点型准心建议开启 |
| bAlwaysStayCentered | 是否始终固定在屏幕中心 | TPS 可按需求决定 |
| Size | 中心点大小 | 2 到 4 |
| Opacity | 中心点透明度 | 0.8 到 1.0 |
| Color | 中心点颜色 | 通常与主色一致 |

## 7. 命中反馈配置

FHelsincy_HitMarkerProfile 决定命中反馈的视觉与动画。默认情况下，任意 HitMarker 可见时会临时隐藏基础准星和中心点，并在反馈结束后自动恢复。

| 字段 | 说明 |
|------|------|
| bEnabled | 总开关 |
| Mode | HitMarker 模式: `SingleInstance`（COD 风格单实例，默认）或 `MultiInstance`（经典多实例） |
| CrosshairVisibilityWhileActive | HitMarker 可见期间的基础准星策略：`Hide`（默认隐藏）、`KeepVisible`（保持显示）或 `ScaleAlpha`（按透明度缩放） |
| CrosshairAlphaScaleWhileHitMarkerActive | `ScaleAlpha` 策略下的基础准星透明度缩放，默认 `0.10` |
| bApplyHitMarkerVisibilityPolicyToCenterDot | 是否将同一策略应用到中心点，默认 true |
| bUseTaperedShape | 是否使用锥形针状样式 |
| TaperedShapeGlowWidthScale | 针状光晕宽度缩放 |
| TaperedShapeGlowAlphaScale | 针状光晕透明度缩放 |
| Duration | 单次命中反馈持续时间 |
| MergeThreshold | 连续命中合并窗口 |
| Color | 普通命中颜色 |
| HeadshotColor | 爆头颜色 |
| HeadShotScale | 爆头缩放 |
| KillColor | 击杀颜色 |
| KillScale | 击杀缩放 |
| bClearAllOldHitMarkerOnKill | 击杀时是否清理旧实例 |
| Thickness | 线宽 |
| BaseDistance | 离中心的基础距离 |
| StartSize / EndSize | 动画起止长度 |
| StartOffset / EndOffset | 动画起止偏移 |
| ShakeIntensity | 整体震动强度 |
| NormalShakeIntensity | 臂法线方向震动强度 |
| bShakeDecay | 震动是否衰减 |
| CustomTexture | 自定义图片 |
| HitPulseScale | 单实例模式：命中时的缩放脉冲强度 (1.0 = 无脉冲)。范围 1.0–2.0 |
| HitPulseRecoverySpeed | 单实例模式：脉冲恢复速度（值越大回弹越快）。范围 5.0–30.0 |
| KillPulseScale | 单实例模式：击杀时的脉冲缩放（比普通命中更大）。范围 1.0–3.0 |

建议：

- 竞技风格：缩短 Duration，降低 ShakeIntensity
- 强反馈风格：提高 KillScale，开启 bClearAllOldHitMarkerOnKill
- COD 风格（推荐）：使用 `SingleInstance` 模式，保持默认 HitPulseScale=1.2 和 KillPulseScale=1.5
- 旧 SingleInstance 弱化风格：将 CrosshairVisibilityWhileActive 设为 `ScaleAlpha`
- 经典多实例风格：切换到 `MultiInstance` 模式，调整 MergeThreshold 控制合并窗口

## 8. 伤害方向指示器配置

FHelsincy_DamageIndicatorProfile 控制受伤方向提示。

| 字段 | 说明 |
|------|------|
| bEnabled | 总开关 |
| IndicatorStyleTag | 指示器样式 Tag |
| Duration | 指示器持续时间 |
| CircleMaxOpacity | 圆环最大透明度 |
| PointerMaxOpacity | 指针最大透明度 |
| FadeInTime | 淡入时间 |
| FadeOutTime | 淡出时间 |
| bShowCircle | 是否显示底圈 |
| CircleSegments | 圆环分段数 |
| Radius | 半径 |
| CircleColor | 圆环颜色 |
| CircleThickness | 圆环线宽 |
| bPointerOutsideCircle | 指针在圆环外还是内 |
| PointerDistanceOffset | 指针离圆环偏移 |
| RotationInterpSpeed | 角度平滑速度 |
| ArrowConfig | 箭头专属配置 |
| ImageConfig | 图片专属配置 |

### 指示器样式 Tag

当前内置：

- Indicator.Style.Arrow
- Indicator.Style.Image

### 数据资产 (DataAsset)

使用 `UHelsincyDamageIndicatorDataAsset` 在编辑器中管理伤害指示器配置:

1. Content Browser → 右键 → Miscellaneous → Data Asset → 选择 `HelsincyDamageIndicatorDataAsset`
2. 编辑 Profile 中各项参数
3. 在 Character BP 的 `UHelsincyDamageIndicatorComponent` 中设置 `DefaultDataAsset` 属性
4. 确保 `bUseDefaultDataAssetInit = true` (默认即为 true)

运行时切换: `DamageIndicatorComp->LoadFromDataAsset(OtherDataAsset);`

### 预设与持久化

DamageIndicator 支持与 Crosshair 相同的预设与持久化系统:

| 功能 | API |
|------|-----|
| 保存预设到内存 | `SavePreset(PresetName)` |
| 加载内存预设 | `LoadPreset(PresetName)` |
| 列出所有预设 | `GetAllPresetNames()` |
| 保存到磁盘 | `SavePresetsToDisk(SlotName, UserIndex)` |
| 从磁盘加载 | `LoadPresetsFromDisk(SlotName, UserIndex)` |
| 自动加载 | 设置 `bAutoLoadOnBeginPlay = true` |
| 自动保存 | 设置 `bAutoSaveOnEndPlay = true` |

默认槽位: `"DamageIndicatorPresets"` (与准星预设的 `"CrosshairPresets"` 独立)

## 9. 资产配置建议

推荐将 FHelsincyCrosshairProfile 按武器或状态拆成多个 DataAsset：

- DA_Crosshair_Rifle
- DA_Crosshair_SMG
- DA_Crosshair_Sniper
- DA_Crosshair_Shotgun
- DA_Crosshair_InteractOnly

这种管理方式比在一个资产里硬编码所有情况更清晰。

## 10. 调参建议模板

### 10.1 突击步枪模板

- ShapeTag：Cross 或 TStyle
- Length：10 到 14
- Gap：5 到 8
- Thickness：2 到 3
- VelocityMultiplier：0.05
- RecoilRecoverySpeed：10 到 12

### 10.2 冲锋枪模板

- ShapeTag：Cross 或 Chevron
- Gap：偏小
- 动态扩散开启
- 恢复速度较快

### 10.3 狙击镜外腰射模板

- ShapeTag：Circle 或 Wings
- Gap：较大
- AirbornePenalty：较高
- 命中反馈时长略短

## 11. 配置层面的注意事项

1. 当前所有 10 个内置 ShapeTag 均有对应渲染器。如果你注册了自定义 Tag 但未提供渲染器，该 Tag 不会产生绘制。
2. 运行时 UpdateProfile 会整体覆盖 CurrentProfile，局部调参时要注意别丢掉其他字段。
3. SavePreset / LoadPreset 操作内存中的预设库；如需写入磁盘，调用 `SavePresetsToDisk()` / `LoadPresetsFromDisk()`。也可勾选 `bAutoLoadOnBeginPlay` / `bAutoSaveOnEndPlay` 实现自动持久化。
4. 目标识别依赖碰撞通道设置，很多“变色失效”其实是 Trace 没打到目标。
## 12. 持久化存档策略说明 (Sync vs Async SaveGame)

本插件的 `SavePresetsToDisk()` / `LoadPresetsFromDisk()` 使用的是 UE4 的 **同步存档** API (`UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot`)。以下详细说明选用同步方案的原因、异步方案的适用场景，以及未来优化方向。

### 12.1 为什么使用同步存档

#### 数据量极小

准星和伤害指示器的预设数据本质上是一组扁平的结构体 (`FHelsincyCrosshairProfile` / `FHelsincy_DamageIndicatorProfile`)，序列化后体积通常 **< 50 KB**，即使玩家保存了大量预设也很难超过 **200 KB**。这个量级的 I/O 对现代 SSD/HDD 来说几乎是瞬时完成的（< 1 ms），不会造成任何可感知的帧率卡顿。

#### EndPlay 安全性

`bAutoSaveOnEndPlay = true` 时，存档发生在 `EndPlay()` 回调中。此时 Actor/Component 正在被销毁，UObject 生命周期已进入尾声。如果使用异步存档：

- 异步任务持有的 `USaveGame` 对象可能在回调执行前被 GC 回收
- 组件自身可能已被销毁，回调中的 `this` 指针悬空
- UE4.27 的 `AsyncSaveGameToSlot` 的完成代理 (Delegate) 无法保证在对象销毁后仍安全触发

同步存档在 `EndPlay()` 中是 **确定性** 的：调用返回时数据已写入磁盘，无需担心生命周期问题。

#### 一致性与确定性

同步调用返回 `true` 即表示数据已落盘。调用者可以立即做出判断（如 UI 提示"保存成功"）。异步方案需要额外的回调/事件机制，增加了代码复杂度，对一个 < 50 KB 的写入来说不值得。

#### 与 Crosshair 模块保持一致

HelsincyCrosshair 和 HelsincyDamageIndicator 两个模块共享相同的持久化架构模式。保持同步方案使两个模块的行为完全可预测，降低集成者的认知负担。

### 12.2 什么情况下应该使用异步存档

以下场景中，异步存档是更好的选择：

| 场景 | 原因 |
|------|------|
| **存档数据量大 (> 1 MB)** | 大量数据的序列化 + 磁盘写入可能占用 10+ ms，造成帧率波动 |
| **高频自动保存** | 如每隔 N 秒自动存档一次，同步 I/O 会周期性地阻塞游戏线程 |
| **开放世界 / 全局存档** | 整个世界状态的存档通常很大且包含复杂对象图 |
| **网络存储 / 云存档** | 网络延迟不可预测，同步调用可能阻塞数百毫秒 |
| **主线程帧率极其敏感** | VR 或竞技场景中即使 1–2 ms 的抖动也不可接受 |

UE4.27 提供了 `UGameplayStatics::AsyncSaveGameToSlot()` 和 `AsyncLoadGameFromSlot()`，使用方式：

```cpp
// 异步存档示例 (仅供参考, 本插件未使用)
UGameplayStatics::AsyncSaveGameToSlot(SaveGameObj, SlotName, UserIndex,
    FAsyncSaveGameToSlotDelegate::CreateLambda([](const FString& Slot, int32 Idx, bool bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Async save to [%s] %s"), *Slot, bSuccess ? TEXT("OK") : TEXT("FAIL"));
    })
);
```

> **注意**: 异步 API 的完成回调在游戏线程触发，但触发时机不确定。如果在 `EndPlay` 中使用异步存档，必须确保 SaveGame 对象不会被提前 GC（例如用 `UPROPERTY()` 强引用或 `AddToRoot()`），并且回调中不访问已销毁的组件。

### 12.3 未来优化考虑

#### 短期 — 无需改动

当前方案对插件目标场景（FPS 准星 + 伤害指示器预设）完全足够。数据量远低于同步 I/O 的性能阈值，无需优化。

#### 中期 — 可选的异步升级路径

如果未来插件扩展到以下场景，可考虑升级为异步：

1. **预设包含大量贴图引用**: 如果 Profile 中嵌入了大型纹理数据（而非资产路径引用），序列化体积可能膨胀
2. **云存档集成**: 与 Steam Cloud / Epic Cloud Save / 自建云服务对接时，必须使用异步以避免网络阻塞
3. **自动保存频率提高**: 如果增加了"每次修改自动落盘"的功能，高频同步写入可能影响帧率

升级路径建议：

```
当前: SaveGameToSlot (同步)
  ↓ 数据量增长 或 云存档需求
过渡: AsyncSaveGameToSlot + 回调通知
  ↓ 更复杂的存档需求
高级: 独立 SaveGame 管理器 + 队列化写入 + 错误重试
```

#### 长期 — 架构预留

当前代码已做好异步升级的基础预留：

- `SavePresetsToDisk()` 返回 `bool`，未来可改为返回委托句柄或异步任务对象
- `USaveGame` 子类独立存在 (`UHelsincyCrosshairSaveGame` / `UHelsincyDamageIndicatorSaveGame`)，可无缝切换底层 I/O 方式
- 存档槽名 (`SlotName`) 参数化，未来可映射到云存档 Key

如需升级为异步，改动范围仅限于 `SavePresetsToDisk()` / `LoadPresetsFromDisk()` 的内部实现，不影响外部 API 签名和使用方式。
