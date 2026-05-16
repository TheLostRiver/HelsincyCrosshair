# HelsincyCrosshair

**Language / 语言**: [简体中文](README.md) | English

**`HelsincyCrosshair` is a production-ready dynamic crosshair and standalone damage direction indicator HUD plugin for Unreal Engine shooter projects.**

It is not a small sample that only draws a cross at screen center. It is a runtime HUD presentation system designed for real FPS / TPS projects: low-latency `Canvas` / `FCanvas` rendering, 10 built-in crosshair renderers, 3 damage direction indicator styles, COD-style hit feedback, dynamic spread, recoil recovery, target-detection color feedback, DataAsset presets, and built-in `USaveGame` persistence.

The current core Profile structures expose **170+ editable configuration fields**, covering crosshair visuals, dynamic spread, shape parameters, center dot, hit feedback, incoming-damage direction cues, window-edge placement, arc damage indicators, image styles, color mapping, and runtime presets. It works out of the box, but it is also built for deep tuning, renderer extension, and per-weapon / per-character / per-state HUD switching.

The current implementation is split into two Runtime modules:

- `HelsincyCrosshair`: crosshair, center dot, dynamic spread, recoil spread recovery, hit markers, and target-detection color feedback.
- `HelsincyDamageIndicator`: incoming-damage direction indicators, window-edge/radial placement modes, and Arrow/Image/Arc indicator renderers.

For a capability-first overview, read the [Feature overview](Docs/Feature-Overview_EN.md). To understand the overall architecture, why this design was chosen, and how it keeps the plugin extensible, easy to integrate, and performant, read the [Architecture document](Docs/Architecture.md).

To start using the plugin, read the [English user manual](Docs/UserManual_EN.md). For project integration steps, read the [Integration guide](Docs/Integration-Guide.md).

## Highlights

- **A marketplace-grade crosshair system, not a demo widget**: 10 registered built-in crosshair renderers covering Cross, Circle, TStyle, Triangle, Rectangle, Chevron, Polygon, Wings, Image, and DotOnly for competitive FPS, TPS, ability shooters, hip-fire HUDs, interaction cues, and specialized weapon reticles.
- **170+ editable configuration fields**: the core Profiles are fully data-driven, covering color, outline, opacity, global offset, dynamic spread, recoil recovery, shape-specific settings, center dot, target detection, interactable color feedback, hit feedback animation, and damage indicator styles.
- **Standalone damage direction indicator module**: `HelsincyDamageIndicator` is split into its own Runtime module, with Arrow, Image, and Arc styles plus `RadialCircle` and `WindowEdge` placement modes.
- **COD-style hit feedback**: supports body hit, headshot, kill, single-instance refresh, multi-instance stacking, twist, scale punch, tapered lines, normal shake, damage-strength response, and base-crosshair visibility policies.
- **Integration paths that respect real projects**: use `AHelsincyHUD` / `BP_HelsincySuperHUD` directly, or integrate through the Bridge API without changing your existing HUD inheritance chain.
- **Runtime tuning, saving, and restoration**: DataAssets provide defaults, components own runtime Profiles, and built-in `USaveGame` persistence keeps presets across level transitions and restarts.
- **GameplayTag-driven extension**: crosshair shapes, damage indicator styles, and custom renderers are selected by GameplayTag, with both Blueprint and C++ extension paths.
- **Designed for performance-sensitive HUD paths**: avoids the main UMG path, renders through `Canvas` / `FCanvas`, and exposes `stat HelsincyCrosshair` / `stat HelsincyDamageIndicator` profiling hooks.

## Integration Modes

### Legacy HUD

Set the GameMode HUD Class to `AHelsincyHUD`, or use the built-in Blueprint subclass `BP_HelsincySuperHUD`. This path is useful for quick validation and legacy project compatibility.

### Bridge API

Call the render library functions from any C++ or Blueprint HUD without inheriting from `AHelsincyHUD`:

- `UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(AHUD* HUD)`
- `UHelsincyCrosshairRenderLibrary::DrawCrosshairForController(APlayerController*, UCanvas*)`
- `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(AHUD* HUD)`
- `UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForController(APlayerController*, UCanvas*)`

The Bridge path currently covers the main crosshair, Center Dot, Hit Marker, and Damage Indicator drawing.

## Quick Start

1. Enable the `HelsincyCrosshair` plugin in your project.
2. Set the GameMode HUD Class to `AHelsincyHUD` / `BP_HelsincySuperHUD`, or integrate the Bridge API in your custom HUD.
3. Add `UHelsincyCrosshairComponent` to the local player Pawn or Character.
4. Point `DefaultCrosshairAsset` to `Content/DataAsset/DA_Crosshair_Default.uasset`, or call `LoadFromDataAsset` manually after BeginPlay.
5. Call `AddRecoil` when firing, and call `TriggerHitMarker`, `TriggerHeadshotMarker`, or `TriggerHitMarkerAdvanced` when a hit is confirmed.
6. If you need damage direction indicators, also add `UHelsincyDamageIndicatorComponent`.
7. Point `DefaultDataAsset` to `Content/DataAsset/DA_DamageIndicator.uasset` and keep `bUseDefaultDataAssetInit = true`.
8. In character damage events such as `TakeDamage` or `AnyDamage`, call `UHelsincyDamageIndicatorComponent::RegisterDamageEvent`.
9. For friend-or-foe detection, implement `GenericTeamAgentInterface` on targets. For interactable-object detection, implement `IHelsincyTargetInterface`.

For detailed setup instructions, see [Docs/Integration-Guide.md](Docs/Integration-Guide.md).

## Modules And Core Types

| Module | Responsibility | Key Component | Bridge / Render Entry |
|--------|----------------|---------------|-----------------------|
| `HelsincyCrosshair` | Crosshair state, target detection, hit feedback | `UHelsincyCrosshairComponent` | `UHelsincyCrosshairRenderLibrary` |
| `HelsincyDamageIndicator` | Damage direction state, DataAsset loading, preset persistence | `UHelsincyDamageIndicatorComponent` | `UHelsincyDamageIndicatorRenderLibrary` |

`UHelsincyCrosshairComponent` owns the active crosshair Profile, runtime color cache, dynamic spread, recoil recovery, hit marker instances, and async target detection.

`UHelsincyDamageIndicatorComponent` owns damage direction configuration, active indicator lifetimes, angle updates, DataAsset initialization, and damage indicator preset persistence.

The HUD or Bridge render library reads state from the components, resolves the matching renderer by GameplayTag, and draws to the current `Canvas`.

## Main Assets

- `Content/HUD/BP_HelsincySuperHUD.uasset`: HUD Blueprint asset that can be used as a default HUD starting point.
- `Content/DataAsset/DA_Crosshair_Default.uasset`: default crosshair data asset.
- `Content/DataAsset/DA_DamageIndicator.uasset`: default damage indicator data asset.
- `Content/ArcImage/T_HDI_Arc_Red_01.uasset`: arc texture for the Arc-style damage indicator.
- `Content/HitmarkerImage/T_HC_HitMarker_Core.uasset`: core hit marker texture.
- `Content/HitmarkerImage/T_HC_HitMarker_Glow.uasset`: hit marker glow texture.
- `Config/DefaultHelsincyCrosshair.ini`: redirects for old class names, old field names, and module split compatibility.

## Code Structure

```text
HelsincyCrosshair/
+-- Config/
+-- Content/
|   +-- ArcImage/
|   +-- DataAsset/
|   +-- HitmarkerImage/
|   +-- HUD/
+-- Docs/
+-- Resources/
+-- Source/
    +-- HelsincyCrosshair/
    |   +-- Public/
    |   +-- Private/
    +-- HelsincyDamageIndicator/
        +-- Public/
        +-- Private/
```

## Data-Driven Configuration

`FHelsincyCrosshairProfile` groups crosshair visuals, dynamic spread, shape parameters, center dot, and hit marker configuration. It can be loaded through `UHelsincyCrosshairDataAsset`.

`FHelsincy_DamageIndicatorProfile` groups damage indicator enablement, lifetime, placement mode, circle, Arrow/Image/Arc style parameters, and persistent preset data. It can be loaded through `UHelsincyDamageIndicatorDataAsset`.

Damage indicator style selection is driven by `IndicatorStyleTag`. Built-in GameplayTags include:

- `Indicator.Style.Arrow`
- `Indicator.Style.Image`
- `Indicator.Style.Arc`

Damage indicator placement modes include:

- `RadialCircle`: displays indicators on a circular track around the player screen center.
- `WindowEdge`: displays indicators along the current game window / Canvas safe frame, suitable for both windowed and fullscreen modes.

## Documentation Map

| Document | Path | Audience | Notes |
|----------|------|----------|-------|
| Feature overview | [Docs/Feature-Overview_EN.md](Docs/Feature-Overview_EN.md) | All users, technical artists, designers, programmers | Capability overview, configuration scale, crosshair and damage indicator highlights |
| v4.0.0 release notes | [Docs/Release-Notes-v4.0.0.md](Docs/Release-Notes-v4.0.0.md) | All users | First public Release positioning, core capabilities, and package contents |
| Release packaging | [Docs/Release-Packaging.md](Docs/Release-Packaging.md) | Maintainers, release owners | Bilingual release package contents, exclusion rules, and packaging command |
| Architecture | [Docs/Architecture.md](Docs/Architecture.md) | Programmers, technical designers | Module boundaries, class responsibilities, data flow, and rendering path |
| Integration guide | [Docs/Integration-Guide.md](Docs/Integration-Guide.md) | Gameplay programmers, Blueprint users | Full setup path from plugin enablement to character integration |
| Configuration reference | [Docs/Configuration-Reference.md](Docs/Configuration-Reference.md) | Tuning and integration users | Profile structs and config field reference |
| Extension guide | [Docs/Extension-Guide.md](Docs/Extension-Guide.md) | Engine programmers, plugin maintainers | Custom renderers, target interfaces, and extension strategy |
| API reference | [Docs/API-Reference.md](Docs/API-Reference.md) | Programmers, Blueprint developers | Bilingual reference for classes, methods, properties, structs, enums, and GameplayTags |
| User manual (Chinese) | [Docs/UserManual_CN.md](Docs/UserManual_CN.md) | All users | Full Chinese user manual from quick setup to advanced extension |
| User manual (English) | [Docs/UserManual_EN.md](Docs/UserManual_EN.md) | All users | Full English user manual |
| Known issues and limitations | [Docs/Known-Issues.md](Docs/Known-Issues.md) | Everyone | Current implementation boundaries, notes, and workarounds |

## Current Version Notes

- This README is based on the current source code and plugin Content assets, not on early design-document assumptions.
- The first public release is versioned as `v4.0.0`; this is a mature initial release label and does not imply that public v1/v2/v3 releases existed.
- `HelsincyDamageIndicator` has been split out of the crosshair module into an independent Runtime module. To show damage direction indicators, you must add `UHelsincyDamageIndicatorComponent`.
- Whether the default character class has the damage indicator component depends on project-side configuration. The plugin does not force-inject the component into every Pawn.
- `WindowEdge` uses the current game Canvas / window bounds and supports windowed mode. It is not based on the desktop monitor edge.
