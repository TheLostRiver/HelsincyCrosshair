# HelsincyCrosshair v4.0.2 Release Notes / 发布说明

## 中文

**v4.0.2** 是 `HelsincyCrosshair` 的命中反馈维护版本。本版本聚焦 `SingleInstance` + `SpriteDualLayer` HitMarker 的可读性、持续时间和调试稳定性。

### 修复内容

- **修复 SpriteDualLayer HitMarker 一闪而过**：当旧资产或已保存资产中的 `SingleInstanceSpriteMinDisplayDuration` 为无效短值时，运行时会回退到 0.35 秒的可感知显示时长。
- **补齐 SpriteDualLayer 运动模式**：新增 `SingleInstanceSpriteMotionMode`。默认 `WholeSpriteShake` 使用完整 Core/Glow 贴图；`PerArmQuadrantShake` 使用独立单臂贴图绘制四条手臂。
- **改善默认几何 HitMarker 观感**：默认 `Thickness`、`StartSize` 和 `EndSize` 调轻，减少默认命中反馈的突兀感。
- **加固绘制路径诊断**：HUD 与 Bridge 路径现在会记录同帧单实例 HitMarker 绘制，避免重复绘制造成闪烁判断噪音。

### 验证

- `HelsincyCrosshair.HitMarker.SingleInstance` 自动化测试组通过。
- 使用 UE 4.27 + VS2022 工具链通过 UnrealBuildTool 编译验证。

### 发布包

推荐使用：

```powershell
.\Scripts\Package-Release.ps1 -Version 4.0.2
```

如果本机 PowerShell 执行策略阻止脚本运行，可以使用：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\Package-Release.ps1 -Version 4.0.2
```

生成：

```text
Dist/HelsincyCrosshair-v4.0.2.zip
```

## English

**v4.0.2** is a hit-feedback maintenance release for `HelsincyCrosshair`. It focuses on `SingleInstance` + `SpriteDualLayer` HitMarker readability, display lifetime, and draw-path diagnostics.

### Fixes

- **Fixed SpriteDualLayer hitmarkers flashing out too quickly**: invalid or too-short saved `SingleInstanceSpriteMinDisplayDuration` values now fall back to a perceptible 0.35s runtime duration.
- **Added SpriteDualLayer motion presets**: `SingleInstanceSpriteMotionMode` now selects between the stable `WholeSpriteShake` default using full Core/Glow textures and `PerArmQuadrantShake` using dedicated single-arm sprites.
- **Lightened default geometry hitmarkers**: default `Thickness`, `StartSize`, and `EndSize` were reduced for a less abrupt default hit feedback shape.
- **Hardened draw diagnostics**: HUD and Bridge single-instance HitMarker paths now record same-frame draws so duplicate draw noise is easier to diagnose and guard.

### Verification

- Passed the `HelsincyCrosshair.HitMarker.SingleInstance` automation test group.
- Verified source compilation through UnrealBuildTool with UE 4.27 and the local VS2022 toolchain.

### Release Package

Recommended command:

```powershell
.\Scripts\Package-Release.ps1 -Version 4.0.2
```

If the local PowerShell execution policy blocks script execution, use:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\Package-Release.ps1 -Version 4.0.2
```

Output:

```text
Dist/HelsincyCrosshair-v4.0.2.zip
```
