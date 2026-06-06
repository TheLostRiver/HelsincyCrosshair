# HelsincyCrosshair Release Packaging / 发布打包说明

## 中文

当前发布版本定为 **v4.0.1**。**v4.0.0** 仍是项目第一次正式公开 Release；v4.0.1 是其后的维护版本，聚焦异步目标检测、本地玩家守卫重新激活和公共模块依赖暴露。

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
- `CHANGELOG.md`
- `Docs/Release-Notes-v4.0.1.md`

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
.\Scripts\Package-Release.ps1 -Version 4.0.1
```

脚本会生成：

```text
Dist/HelsincyCrosshair-v4.0.1.zip
```

脚本会在压缩前检查必要的顶层目录、README、CHANGELOG 和对应版本发布说明是否存在，并阻止过程文件与 Unreal 缓存目录进入发布包。

## English

The current release is versioned as **v4.0.1**. **v4.0.0** remains the first formal public release; v4.0.1 is a maintenance release focused on async target detection, local-player guard reactivation, and public module dependency exposure.

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
- `CHANGELOG.md`
- `Docs/Release-Notes-v4.0.1.md`

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
.\Scripts\Package-Release.ps1 -Version 4.0.1
```

The script creates:

```text
Dist/HelsincyCrosshair-v4.0.1.zip
```

Before compressing the package, the script verifies required top-level directories, README files, CHANGELOG, and the matching version release notes, then blocks process notes or Unreal cache directories from entering the release archive.
