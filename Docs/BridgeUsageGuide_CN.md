# HelsincyCrosshair Bridge 接入指南（CN）

> 目标：在不继承 `AHelsincyHUD` 的前提下使用准星插件。

## 1. 适用场景

- 项目已有自定义 HUD（C++ 或蓝图）
- 不希望改动现有 HUD 继承体系
- 希望保持 Canvas 渲染性能路径

## 2. 前置条件

1. 角色上已挂载 `UHelsincyCrosshairComponent`
2. 已配置可用的 Crosshair Profile / DataAsset
3. 插件启用且子系统正常注册

## 3. C++ HUD 接入

在你的 HUD 类中（`DrawHUD()`）：

```cpp
void AYourHUD::DrawHUD()
{
    Super::DrawHUD();
    UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(this);
}
```

如果你要更底层地控制（例如多 Canvas 场景）：

```cpp
UHelsincyCrosshairRenderLibrary::DrawCrosshairForController(PlayerController, Canvas);
```

## 4. 蓝图 HUD 接入

在 HUD 蓝图的绘制时机中调用节点：

- `DrawCrosshairForHUD`

建议：

- 确保调用频率为每帧绘制周期
- 避免同一帧重复调用（防止重复绘制）

## 5. 与 Legacy 模式关系

- Legacy：`AHelsincyHUD`（兼容旧工程）
- Bridge：任意 HUD 调用函数（推荐新接入）

迁移建议：

1. 保持旧 HUD 不变
2. 在现有 HUD 增加一行 Bridge 调用
3. 对比视觉一致性后再移除旧依赖

## 6. 常见问题排查

### 6.1 调用后没有准星

检查项：

- HUD/Canvas 是否有效
- PlayerController 是否有效
- Pawn 是否存在
- Pawn 上是否有 `UHelsincyCrosshairComponent`
- 组件是否启用准星
- Subsystem 是否已注册渲染器

### 6.2 有主体准星但无反馈

检查项：

- Profile 中 HitMarker / DamageIndicator 配置是否启用
- 命中事件与伤害事件是否正确写入组件状态

### 6.3 重复绘制

检查项：

- 是否同时使用 Legacy HUD 与 Bridge 调用
- 是否在同一帧调用了多个 Bridge 入口

## 7. 联机验证建议（Listen Server）

请至少验证：

1. Host/Client 主体准星一致
2. 命中反馈触发一致
3. 伤害指示器角度与淡入淡出表现一致

可配合：`/docs/MULTIPLAYER_TEST_MATRIX.md`

## 8. 后续建议

- 将 `AHelsincyHUD` 逐步改为薄封装
- 为蓝图提供最小示例工程片段
- 将 Bridge 验证结果纳入 PR 模板


---

## 9. 调试体系参考

- 统一调试方案：[`CrosshairDebugPlan_CN.md`](CrosshairDebugPlan_CN.md)
