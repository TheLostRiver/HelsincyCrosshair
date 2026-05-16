# HelsincyCrosshair v4.0.0 Release Notes / 发布说明

## 中文

**v4.0.0** 是 `HelsincyCrosshair` 的首次公开 Release，也是当前架构成熟后的首个正式版本。虽然此前没有公开发布版本，但项目已经经历多轮迭代和几次大规模重构：伤害方向指示器被拆分为独立 Runtime 模块，准心系统完成 10 个内置渲染器覆盖，命中反馈、数据驱动 Profile、持久化和 Bridge API 都已经形成稳定使用路径。

### 版本定位

- 面向 Unreal Engine 4.27 射击项目的高完成度动态准心插件。
- 附带独立 `HelsincyDamageIndicator` Runtime 模块。
- 适用于 FPS、TPS、竞技射击、技能射击、交互 HUD 和武器专属准心系统。

### 核心能力

- 10 个内置准心渲染器：Cross、Circle、TStyle、Triangle、Rectangle、Chevron、Polygon、Wings、Image、DotOnly。
- 3 个内置伤害方向指示器样式：Arrow、Image、Arc。
- 170+ 可编辑配置字段，覆盖准心视觉、动态扩散、后坐力恢复、命中反馈、目标识别、交互变色、受击提示和预设持久化。
- COD 风格 HitMarker，支持普通命中、爆头、击杀、单实例刷新、多实例叠加、扭转、缩放、震动和伤害强度响应。
- DataAsset 默认配置和运行时 Profile 更新。
- `USaveGame` 预设持久化，支持跨关卡和重启保留。
- Legacy HUD 与 Bridge API 两种集成方式。
- GameplayTag 驱动的准心形状、伤害指示器样式和扩展渲染器选择。
- `stat HelsincyCrosshair` 与 `stat HelsincyDamageIndicator` 性能统计入口。

### 发布包内容

发布包包含源码、内容资产、配置、文档、图片资源和 README。发布包明确排除项目过程文件和 Unreal 缓存目录，包括 `progress.md`、`task_plan.md`、`findings.md`、`Binaries/`、`Intermediate/`、`Saved/`、`DerivedDataCache/` 等。

## English

**v4.0.0** is the first public Release of `HelsincyCrosshair`, and the first formal version after the current architecture matured. Even though there were no earlier public releases, the project has already gone through multiple iterations and several large-scale refactors: the damage direction indicator was split into its own Runtime module, the crosshair system now covers 10 built-in renderers, and hit feedback, data-driven Profiles, persistence, and Bridge API integration all have stable usage paths.

### Version Positioning

- A production-ready dynamic crosshair plugin for Unreal Engine 4.27 shooter projects.
- Includes the standalone `HelsincyDamageIndicator` Runtime module.
- Suitable for FPS, TPS, competitive shooters, ability shooters, interaction HUDs, and weapon-specific reticle systems.

### Core Capabilities

- 10 built-in crosshair renderers: Cross, Circle, TStyle, Triangle, Rectangle, Chevron, Polygon, Wings, Image, and DotOnly.
- 3 built-in damage direction indicator styles: Arrow, Image, and Arc.
- 170+ editable configuration fields covering crosshair visuals, dynamic spread, recoil recovery, hit feedback, target detection, interaction color feedback, incoming-damage cues, and preset persistence.
- COD-style HitMarker behavior with body hits, headshots, kills, single-instance refresh, multi-instance stacking, twist, scale punch, shake, and damage-strength response.
- DataAsset defaults and runtime Profile updates.
- `USaveGame` preset persistence across level transitions and restarts.
- Legacy HUD and Bridge API integration paths.
- GameplayTag-driven selection for crosshair shapes, damage indicator styles, and extension renderers.
- Profiling hooks through `stat HelsincyCrosshair` and `stat HelsincyDamageIndicator`.

### Release Package Contents

The release package includes source code, content assets, configuration, documentation, image resources, and README files. It explicitly excludes process notes and Unreal cache directories, including `progress.md`, `task_plan.md`, `findings.md`, `Binaries/`, `Intermediate/`, `Saved/`, and `DerivedDataCache/`.
