# Changelog

All notable maintenance changes for HelsincyCrosshair are recorded here.

## Unreleased

No changes yet.

## v4.0.2 - 2026-07-07

### Fixed

- Fixed single-instance `SpriteDualLayer` hitmarkers reading as a one-frame flash when saved assets carried an invalid or too-short sprite display duration. Runtime resolution now falls back to a perceptible 0.35s sprite minimum.
- Added a sprite motion mode selector for single-instance hitmarkers. `WholeSpriteShake` is the stable default, while `PerArmQuadrantShake` uses dedicated single-arm textures for four independent arms instead of slicing a complete X sprite.
- Reduced default legacy hitmarker geometry thickness and size for a lighter default profile.
- Added draw diagnostics and duplicate draw guarding for HUD and Bridge single-instance hitmarker paths.

### Verification

- Added and ran single-instance hitmarker automation coverage for sprite duration fallback, whole-sprite motion, per-arm sprite selection, custom texture fallback, canvas rotation, and tempered sprite shake.
- Verified the plugin source with UnrealBuildTool on UE 4.27 using the local VS2022 toolchain.
- See [Docs/Release-Notes-v4.0.2.md](Docs/Release-Notes-v4.0.2.md) for the full bilingual release notes.

## v4.0.1 - 2026-06-06

### Fixed

- Hardened async target detection against stale callbacks. `UHelsincyCrosshairComponent::OnTraceCompleted` now ignores late `AsyncLineTraceByChannel` callbacks when their `FTraceHandle` is no longer the active request, so an old trace can no longer overwrite the current target highlight or clear a newer target state.
- Fixed local-player guard reactivation for both `UHelsincyCrosshairComponent` and `UHelsincyDamageIndicatorComponent`. Components now clear owner-eligibility state when the owner becomes non-local, AI-controlled, or controllerless, while preserving one-time initialization state so a local -> non-local/AI -> local transition can reactivate correctly without reloading defaults unnecessarily.
- Corrected public module dependency exposure in `HelsincyCrosshair.Build.cs` and `HelsincyDamageIndicator.Build.cs`. Public headers now export the Unreal modules needed by their public API, including `CoreUObject`, `Engine`, `GameplayTags`, `DeveloperSettings`, and `AIModule` where applicable.

### Verification

- Added and ran local-player guard automation coverage for crosshair and damage indicator pending-controller and reactivation paths.
- Added and ran async target-detection automation coverage for stale trace callback handling.
- Verified the plugin source with UnrealBuildTool on UE 4.27 using the local VS2022 toolchain.
- See [Docs/Release-Notes-v4.0.1.md](Docs/Release-Notes-v4.0.1.md) for the full bilingual release notes.

## v4.0.0

- First public release of the current mature plugin architecture.
- Includes the standalone `HelsincyDamageIndicator` Runtime module, 10 built-in crosshair renderers, 3 built-in damage indicator renderers, data-driven profiles, preset persistence, Bridge API integration, and extension docs.
- See [Docs/Release-Notes-v4.0.0.md](Docs/Release-Notes-v4.0.0.md) for the full bilingual release notes.
