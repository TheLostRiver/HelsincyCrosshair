# HelsincyCrosshair

**Language / 语言**: [简体中文](README.md) | English

`HelsincyCrosshair` is a runtime dynamic crosshair and damage direction indicator plugin for Unreal Engine shooter projects. It uses the `Canvas` / `FCanvas` rendering path and aims to provide low-latency, data-driven, extensible HUD presentation without relying on the main UMG path.

The current implementation is split into two Runtime modules:

- `HelsincyCrosshair`: crosshair, center dot, dynamic spread, recoil spread recovery, hit markers, and target-detection color feedback.
- `HelsincyDamageIndicator`: incoming-damage direction indicators, window-edge/radial placement modes, and Arrow/Image/Arc indicator renderers.

To understand the overall architecture, why this design was chosen, and how it keeps the plugin extensible, easy to integrate, and performant, go directly to the [Architecture document](Docs/Architecture.md).

To start using the plugin, read the [English user manual](Docs/UserManual_EN.md). For project integration steps, read the [Integration guide](Docs/Integration-Guide.md).

## Current Features

- 10 registered built-in crosshair renderers: Cross, Circle, TStyle, Triangle, Rectangle, Chevron, Polygon, Wings, Image, and DotOnly.
- Hit marker feedback supports multiple drawing styles, including COD-style twist, scale, and damage-strength response.
- 3 registered built-in damage direction indicator styles: Arrow, Image, and Arc.
- Damage direction indicators support both `RadialCircle` and `WindowEdge` placement modes; `WindowEdge` is based on the current game Canvas / window edge, not the physical monitor edge.
- GameplayTag-driven selection for crosshair shapes, damage indicator styles, and extension renderers.
- DataAsset-based default configuration loading, plus runtime in-memory preset save/load.
- Built-in `USaveGame` disk persistence so presets can survive level transitions and restarts, with optional auto-save/auto-load.
- Supports both Legacy HUD and Bridge API integration modes.
- Supports UE4 `STAT` profiling via `stat HelsincyCrosshair` / `stat HelsincyDamageIndicator` for plugin hot-path costs.

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
| Documentation plan | [Docs/Documentation-Plan.md](Docs/Documentation-Plan.md) | Project owners, maintainers | Documentation structure, coverage, and maintenance guidance |
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
- `HelsincyDamageIndicator` has been split out of the crosshair module into an independent Runtime module. To show damage direction indicators, you must add `UHelsincyDamageIndicatorComponent`.
- Whether the default character class has the damage indicator component depends on project-side configuration. The plugin does not force-inject the component into every Pawn.
- `WindowEdge` uses the current game Canvas / window bounds and supports windowed mode. It is not based on the desktop monitor edge.
- [Docs/Archive/DesignDocument_Legacy_ACS.md](Docs/Archive/DesignDocument_Legacy_ACS.md) is an archived legacy ACS design document. It is useful only for historical comparison and must not be treated as the current project standard.
