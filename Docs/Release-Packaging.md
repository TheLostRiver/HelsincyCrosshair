# HelsincyCrosshair Release Packaging / 发布打包说明

## 中文

当前首次公开发布版本定为 **v4.0.0**。虽然这是项目第一次正式发布 Release，但插件已经经历多轮迭代和大规模重构，当前能力已经覆盖动态准心、独立伤害方向指示器、命中反馈、数据驱动 Profile、预设持久化、Bridge API 和扩展渲染器体系，因此使用 v4.0.0 作为成熟版首发版本。

### 发布包必须包含

- `HelsincyCrosshair.uplugin`
- `Source/`
- `Content/`
- `Config/`
- `Docs/`
- `Resources/`
- `Image/`
- `README.md`
- `README_EN.md`

### 发布包必须排除

- `progress.md`
- `task_plan.md`
- `findings.md`
- `.git/`
- `.vs/`
- `.vscode/`
- `.idea/`
- `.codex/`
- `.superpowers/`
- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `BuildPluginPackage/`
- `Package/`
- `Dist/`

### 推荐打包命令

在插件根目录执行：

```powershell
.\Scripts\Package-Release.ps1 -Version 4.0.0
```

脚本会生成：

```text
Dist/HelsincyCrosshair-v4.0.0.zip
```

脚本会在压缩前检查 `Content/` 和 `Docs/` 是否存在，并阻止过程文件与 Unreal 缓存目录进入发布包。

## English

The first public release is versioned as **v4.0.0**. Although this is the first formal GitHub Release, the plugin has already gone through multiple iterations and large-scale refactors. The current feature set covers dynamic crosshairs, a standalone damage direction indicator module, hit feedback, data-driven Profiles, preset persistence, Bridge API integration, and renderer extension, so v4.0.0 is used as a mature initial release.

### Release Package Must Include

- `HelsincyCrosshair.uplugin`
- `Source/`
- `Content/`
- `Config/`
- `Docs/`
- `Resources/`
- `Image/`
- `README.md`
- `README_EN.md`

### Release Package Must Exclude

- `progress.md`
- `task_plan.md`
- `findings.md`
- `.git/`
- `.vs/`
- `.vscode/`
- `.idea/`
- `.codex/`
- `.superpowers/`
- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `BuildPluginPackage/`
- `Package/`
- `Dist/`

### Recommended Packaging Command

Run this from the plugin root:

```powershell
.\Scripts\Package-Release.ps1 -Version 4.0.0
```

The script creates:

```text
Dist/HelsincyCrosshair-v4.0.0.zip
```

Before compressing the package, the script verifies that `Content/` and `Docs/` are present and blocks process notes or Unreal cache directories from entering the release archive.
