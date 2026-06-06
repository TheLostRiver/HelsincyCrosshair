# HelsincyCrosshair v4.0.1 Release Notes / 发布说明

## 中文

**v4.0.1** 是 `HelsincyCrosshair` 在 v4.0.0 首次公开发布后的维护版本。本版本聚焦运行时可靠性和 C++ 集成稳定性，不改变插件的主要功能边界，也不引入新的资产或配置迁移要求。

### 修复内容

- **异步目标检测过期回调防护**：`UHelsincyCrosshairComponent::OnTraceCompleted` 现在会校验回调 `FTraceHandle` 是否仍是当前活动请求。超过等待帧数后被放弃的旧 trace 即使稍后回调，也不会覆盖当前目标高亮或清空新的目标状态。
- **本地玩家守卫重新激活**：`UHelsincyCrosshairComponent` 和 `UHelsincyDamageIndicatorComponent` 现在会在 owner 变为 AI、远端玩家、无 Controller 或非本地拥有者时清理 owner eligibility 状态；当 Pawn 后续重新变成本地真人玩家时，组件可以重新激活，并保留一次性默认资产 / 预设初始化状态。
- **公共模块依赖暴露**：`HelsincyCrosshair.Build.cs` 和 `HelsincyDamageIndicator.Build.cs` 已将公共头实际暴露的 Unreal 依赖放入 `PublicDependencyModuleNames`，降低外部 C++ 模块包含公共 API 时的编译/链接风险。

### 验证

- `HelsincyCrosshair.LocalPlayerGuard.PendingControllerSurvivesLegacyTimeout`
- `HelsincyCrosshair.LocalPlayerGuard.ReactivatesAfterNonLocalOwner`
- `HelsincyCrosshair.TargetDetection.AsyncTraceIgnoresStaleCallback`
- `HelsincyCrosshair.DamageIndicator.LocalPlayerGuard.PendingControllerSurvivesLegacyTimeout`
- `HelsincyCrosshair.DamageIndicator.LocalPlayerGuard.ReactivatesAfterNonLocalOwner`
- 使用 UE 4.27 + VS2022 工具链通过 UnrealBuildTool 源码编译验证。

### 发布包

推荐使用：

```powershell
.\Scripts\Package-Release.ps1 -Version 4.0.1
```

生成：

```text
Dist/HelsincyCrosshair-v4.0.1.zip
```

## English

**v4.0.1** is a maintenance release after the first public v4.0.0 release of `HelsincyCrosshair`. This release focuses on runtime reliability and C++ integration stability. It does not change the main feature surface, add new assets, or require configuration migration.

### Fixes

- **Async target detection stale callback guard**: `UHelsincyCrosshairComponent::OnTraceCompleted` now verifies that the callback `FTraceHandle` is still the active request. Late callbacks from timed-out traces can no longer overwrite the current target highlight or clear newer target state.
- **Local-player guard reactivation**: `UHelsincyCrosshairComponent` and `UHelsincyDamageIndicatorComponent` now clear owner eligibility when the owner becomes AI-controlled, remote, controllerless, or otherwise non-local. If the same Pawn later becomes a local real player again, the component can reactivate while preserving one-time default asset / preset initialization state.
- **Public module dependency exposure**: `HelsincyCrosshair.Build.cs` and `HelsincyDamageIndicator.Build.cs` now place Unreal modules exposed by public headers in `PublicDependencyModuleNames`, reducing compile/link risk for external C++ modules that include the public API.

### Verification

- `HelsincyCrosshair.LocalPlayerGuard.PendingControllerSurvivesLegacyTimeout`
- `HelsincyCrosshair.LocalPlayerGuard.ReactivatesAfterNonLocalOwner`
- `HelsincyCrosshair.TargetDetection.AsyncTraceIgnoresStaleCallback`
- `HelsincyCrosshair.DamageIndicator.LocalPlayerGuard.PendingControllerSurvivesLegacyTimeout`
- `HelsincyCrosshair.DamageIndicator.LocalPlayerGuard.ReactivatesAfterNonLocalOwner`
- Verified source compilation through UnrealBuildTool with UE 4.27 and the local VS2022 toolchain.

### Release Package

Recommended command:

```powershell
.\Scripts\Package-Release.ps1 -Version 4.0.1
```

Output:

```text
Dist/HelsincyCrosshair-v4.0.1.zip
```
