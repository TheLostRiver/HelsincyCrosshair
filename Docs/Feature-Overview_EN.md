# HelsincyCrosshair Feature Overview

`HelsincyCrosshair` is a dynamic crosshair and standalone damage direction indicator plugin for Unreal Engine shooter projects. Its goal is not just to draw a reticle, but to provide a tunable, extensible, persistent HUD presentation system that can be integrated into real project pipelines.

The plugin currently contains two Runtime modules:

- `HelsincyCrosshair`: crosshair rendering, center dot, dynamic spread, recoil recovery, target-detection color feedback, hit feedback, and crosshair presets.
- `HelsincyDamageIndicator`: standalone incoming-damage direction indicators, damage source tracking, window-edge/radial placement, Arc/Image/Arrow styles, and damage indicator presets.

## Configuration Scale

The core Profile structures expose **170+ editable configuration fields**. This figure comes from the `EditAnywhere` fields in `FHelsincyCrosshairProfile` and its nested structs, plus `FHelsincy_DamageIndicatorProfile` and its nested structs.

Those fields are organized around practical HUD tuning areas:

| Area | Coverage |
|------|----------|
| Crosshair visuals | Primary color, outline, opacity, thickness, global offset |
| Target detection | Enemy, friendly, neutral, and interactable GameplayTag color mapping |
| Dynamic spread | Movement spread, airborne penalty, landing recovery, recoil spread and recovery |
| Shape settings | Line, circle, polygon, rectangle, triangle, Chevron, Wings, Image |
| Center dot | Enablement, size, opacity, color, forced-center behavior |
| Hit feedback | Body hit, headshot, kill, single/multi instance, shake, scale, twist, sprite layers |
| Damage indicator | Style, duration, fade, circle, window edge, arrow, image, Arc damage cue |
| Persistence | Memory presets, disk save, auto-load, auto-save |

## Crosshair System

The plugin includes 10 registered built-in crosshair renderers:

- Cross
- Circle
- TStyle
- Triangle
- Rectangle
- Chevron
- Polygon
- Wings
- Image
- DotOnly

These shapes are runtime renderers driven by a unified Profile, not just static image swaps. You can switch DataAssets per weapon, character, stance, equipment state, or gameplay mode, and you can update the active Profile directly at runtime.

Typical use cases include:

- Dynamic spread reticles for competitive FPS games
- Offset reticles for third-person shooters
- Specialized reticles for shotguns, snipers, launchers, and ability weapons
- Interaction highlights for loot, terminals, devices, and world objects
- Combat HUDs that change color based on enemy/friendly/neutral targets

## Dynamic Spread And Recoil

`FHelsincy_DynamicsSettings` controls movement, jumping, landing, and firing recoil behavior. The crosshair can expand based on character speed, airborne state, and fire events, then smoothly recover when movement or recoil settles.

Supported behavior includes:

- Expansion while moving
- Airborne spread penalty
- Smooth landing recovery
- Recoil spread injection when firing
- Separate X/Y spread application
- Maximum recoil limits to prevent visual runaway

## Hit Feedback

The hit feedback system supports body hits, headshots, kills, and advanced hit events. The default path uses COD-style single-instance refresh behavior, while the classic multi-instance stacking mode remains available.

Tunable behavior includes:

- Body, headshot, and kill colors
- Duration and merge threshold
- Clearing older feedback on kill
- Tapered lines and glow
- Whole-marker shake and per-arm normal shake
- Shake frequency, damping, and decay
- Extra headshot/kill shake energy
- Impact twist, scale punch, and arm-length punch
- Damage-strength response
- Base-crosshair visibility policies while hit feedback is active
- Optional image HitMarkers and Core/Glow dual-layer sprites

## Damage Direction Indicator

`HelsincyDamageIndicator` is an independent Runtime module, not a small side feature inside the crosshair module. It can be mounted, configured, and persisted separately.

Built-in styles:

- `Indicator.Style.Arrow`
- `Indicator.Style.Image`
- `Indicator.Style.Arc`

Placement modes:

- `RadialCircle`: indicators move along a circular track around screen center.
- `WindowEdge`: indicators are placed against the current game Canvas / window safe bounds, suitable for both windowed and fullscreen modes.

The Arc style supports arc textures, glow layers, impact scale punch, and direction cue modes such as CenterChevron, AsymmetricTaper, and CenterNib. For modern FPS incoming-damage feedback, it gives projects a more finished visual language than a plain arrow alone.

## Data-Driven Presets

The plugin is built around DataAssets and runtime Profiles:

- Defaults are stored in `UHelsincyCrosshairDataAsset` / `UHelsincyDamageIndicatorDataAsset`.
- Runtime components own the active Profile and can update it directly or load from DataAssets.
- Named in-memory presets are supported.
- Disk persistence is built on `USaveGame`.
- Auto-load and auto-save are available.

This lets designers author multiple crosshair and damage indicator assets in the editor while gameplay code switches them during weapon changes, stance changes, aiming modes, or character state transitions.

## Integration And Extension

Two integration paths are available:

- Legacy HUD: use `AHelsincyHUD` or `BP_HelsincySuperHUD`.
- Bridge API: call render functions from your existing HUD without changing the HUD inheritance chain.

Extension paths are also built in:

- Built-in and custom renderers are selected by GameplayTag.
- Blueprint renderers can implement custom crosshairs or damage indicators.
- C++ renderers can be registered directly.
- `IHelsincyTargetInterface` supports interactable detection and custom color feedback.

## Performance And Debugging

The plugin renders through `Canvas` / `FCanvas` and avoids the main UMG path. Debugging and profiling entry points include:

- `showdebug Crosshair`
- `showdebug DamageIndicator`
- `stat HelsincyCrosshair`
- `stat HelsincyDamageIndicator`
- dedicated debug CVars for crosshair and damage indicator systems

These tools make the plugin practical not only for visual tuning, but also for integration, profiling, and maintenance inside a real game project.

## Next Reading

- [English user manual](UserManual_EN.md): full Blueprint/C++ usage from setup to advanced extension.
- [Integration guide](Integration-Guide.md): project-level setup steps.
- [Configuration reference](Configuration-Reference.md): field-level parameter notes.
- [Extension guide](Extension-Guide.md): custom crosshair and damage indicator renderers.
- [Architecture document](Architecture.md): module boundaries, data flow, and rendering path.
