# HelsincyCrosshair Plugin — User Manual

> **Version**: 1.0  
> **Engine**: Unreal Engine 4.27  
> **Author**: Helsincy Games  
> **Audience**: Blueprint Users / C++ Users / Designers

---

## Table of Contents

1. [Plugin Overview](#1-plugin-overview)
2. [Quick Start (5-Minute Setup)](#2-quick-start-5-minute-setup)
3. [Integration Methods](#3-integration-methods)
4. [Crosshair Module — Full Usage](#4-crosshair-module--full-usage)
5. [Damage Indicator Module — Full Usage](#5-damage-indicator-module--full-usage)
6. [Crosshair Shapes Overview](#6-crosshair-shapes-overview)
7. [Configuration Reference](#7-configuration-reference)
8. [Extension Guide — Custom Crosshair Shapes](#8-extension-guide--custom-crosshair-shapes)
9. [Extension Guide — Custom Damage Indicator Styles](#9-extension-guide--custom-damage-indicator-styles)
10. [Debug Mode](#10-debug-mode)
11. [Frequently Asked Questions (FAQ)](#11-frequently-asked-questions-faq)
12. [Appendix — Blueprint API Quick Reference](#12-appendix--blueprint-api-quick-reference)

---

## 1. Plugin Overview

**HelsincyCrosshair** is a high-performance dynamic crosshair and damage direction indicator plugin for FPS/TPS shooting games.

### Key Features

- **High-Performance Canvas Rendering** — Draws directly on the HUD Canvas (no UMG), extremely low overhead
- **10 Built-in Crosshair Shapes** — Cross, Circle, T-Style, Triangle, Rectangle, Chevron, Polygon, Wings, Image, Dot Only
- **Dynamic Spread System** — Crosshair automatically expands during movement, jumping, and firing, smoothly recovers when idle
- **Hit Marker Feedback** — Normal / headshot / kill markers with custom colors, shake effects, and tapered line styles
- **Target Detection Color Switching** — Automatically detects enemies (red), friendlies (green), and interactable objects (custom colors)
- **Damage Direction Indicators** — Shows the direction of incoming damage with arrow, custom image, or arc styles
- **Data-Driven** — All configuration managed via DataAssets and structs, designers can tweak values directly
- **Zero-Intrusion Bridge API** — No need to modify your HUD inheritance chain; call the crosshair and damage indicator drawing entries as needed
- **Fully Blueprint Compatible** — Every feature is available in Blueprints, no C++ knowledge required
- **Extensible** — Create custom crosshair shapes and indicator styles via Blueprint or C++

### Module Structure

| Module | Purpose |
|--------|---------|
| `HelsincyCrosshair` | Crosshair rendering, dynamic spread, hit markers, target detection |
| `HelsincyDamageIndicator` | Damage direction indicators |

Both modules are fully independent — use one or both as needed.

---

## 2. Quick Start (5-Minute Setup)

Just 4 steps to see a dynamic crosshair in your project. Damage direction indicators are an independent component; if you also need incoming-damage feedback, include the optional setup in Step 2 and Step 3.

### Step 1: Enable the Plugin

Open the editor → **Edit → Plugins** → Search "HelsincyCrosshair" → Check to enable → Restart the editor.

### Step 2: Add Components to Your Character Blueprint

1. Open your **player Character** (or Pawn) Blueprint.
2. Click **Add Component** → Search for `HelsincyCrosshairComponent` → Add it.
3. In the component's Details panel:
   - Set `Default Crosshair Asset` to `DA_Crosshair_Default` (included with the plugin).
   - Make sure `Use Default Crosshair Asset Init` is **true**.

> If you also want damage direction indicators, add a `HelsincyDamageIndicatorComponent` as well.
> Also set its `Default Data Asset` to `DA_DamageIndicator`, keep `Use Default Data Asset Init` set to **true**, and call `Register Damage Event` from the character's **Any Damage** or **Take Damage** event.

### Step 3: Set Up the HUD

**Easiest approach** (recommended for beginners):

1. Open your **GameMode** Blueprint.
2. Set `HUD Class` to `BP_HelsincySuperHUD` (included with the plugin).

**If you already have your own HUD Blueprint**:

1. Open your HUD Blueprint.
2. In the **Event ReceiveDrawHUD** event:
3. Add a `Draw Crosshair For HUD` node (from HelsincyCrosshairRenderLibrary).
4. If you use damage direction indicators, add a `Draw Damage Indicators For HUD` node (from HelsincyDamageIndicatorRenderLibrary).
5. Connect `self` to the `HUD` input pin on each node.

> This is the **Bridge API** approach. You don't need to change your HUD's parent class; crosshair and damage indicators have separate drawing entries.

### Step 4: Test

Press **Play** — you should see a crosshair at the center of the screen.

---

## 3. Integration Methods

### Method A: Use the Plugin's Built-in HUD (Simplest)

Set your GameMode's `HUD Class` to `BP_HelsincySuperHUD` or `AHelsincyHUD`. The HUD automatically handles all rendering: crosshair, center dot, hit markers, and damage indicators.

**Best for**: New projects, rapid prototyping.

### Method B: Bridge API — Zero-Intrusion Integration (Recommended)

In your own HUD Blueprint or C++ HUD, call the Bridge drawing entries you need:

**Blueprint**: Use the `Draw Crosshair For HUD` node in `Event ReceiveDrawHUD`; if you use damage direction indicators, also call `Draw Damage Indicators For HUD`.

**C++**:
```cpp
void AMyHUD::DrawHUD()
{
    Super::DrawHUD();
    UHelsincyCrosshairRenderLibrary::DrawCrosshairForHUD(this);
    UHelsincyDamageIndicatorRenderLibrary::DrawDamageIndicatorsForHUD(this);
}
```

**Best for**: Projects with an existing custom HUD. Zero-intrusion — no inheritance chain changes needed.

---

## 4. Crosshair Module — Full Usage

### 4.1 On Fire — Add Recoil Spread

In your weapon fire logic, call the crosshair component's **Add Recoil** node:

- `Horizontal Kick`: Horizontal random spread amount
- `Vertical Kick`: Vertical upward spread amount

**Blueprint**:
1. Get the `HelsincyCrosshairComponent` from your character.
2. Call `Add Recoil` with horizontal and vertical values.

**Recommended Values**:

| Weapon Type | Horizontal | Vertical |
|-------------|-----------|----------|
| Assault Rifle | 1–3 | 3–6 |
| SMG | 1–2 | 2–4 |
| Shotgun | 3–6 | 4–8 |
| Pistol | 1–2 | 2–4 |
| Sniper | 5–10 | 8–15 |

> Adjust values in combination with `Dynamic Config → Max Recoil Spread` and `Recoil Recovery Speed` to fine-tune the feel.

### 4.2 On Hit — Trigger Hit Markers

After confirming damage (not just a raycast hit), call the appropriate node:

| Node | Purpose | Color Source |
|------|---------|-------------|
| `Trigger Hit Marker` | Normal hit | `Hit Marker Config → Color` |
| `Trigger Hit Marker Color` | Custom color hit | The color you pass in |
| `Trigger Headshot Marker` | Headshot | `Hit Marker Config → Headshot Color` |
| `Trigger Kill Marker` | Kill confirmed | `Hit Marker Config → Kill Color` |

**Blueprint**:
1. In your weapon hit confirmation event, get the `HelsincyCrosshairComponent`.
2. Call the appropriate trigger node based on hit type.

> By default, while any HitMarker is visible, the base crosshair and center dot are hidden, then restored automatically when feedback ends. Set `CrosshairVisibilityWhileActive` to `ScaleAlpha` to keep the old SingleInstance alpha-weakening look, or `KeepVisible` to leave the base crosshair unchanged.

### 4.3 On Weapon Switch — Load Different Crosshair

Each weapon can have its own crosshair style:

1. Create a `HelsincyCrosshairDataAsset` for each weapon (Right-click Content Browser → Miscellaneous → Data Asset → Select `HelsincyCrosshairDataAsset`).
2. Configure the crosshair parameters for that weapon.
3. On weapon equip, call `Load From Data Asset`.

### 4.4 Runtime Toggle Controls

| Node | Function |
|------|----------|
| `Enable Crosshair` / `Disable Crosshair` | Turn crosshair on/off (e.g., when zooming into a scope) |
| `Is Enabled Crosshair` | Query if crosshair is currently enabled |
| `Enable Center Dot` / `Disable Center Dot` | Toggle center dot independently |
| `Enable Target Detection` / `Disable Target Detection` | Toggle target detection color switching |

### 4.5 Target Detection & Color Switching

The plugin automatically detects targets in front of the crosshair and changes color. Two detection methods are supported:

**Method 1: Team Detection (GenericTeamAgentInterface)**

If the target implements Unreal's `GenericTeamAgentInterface`, the crosshair automatically changes color:
- Enemy (Hostile) → Uses `Visuals Config → Enemy Color` (default: red)
- Friendly → Uses `Visuals Config → Friendly Color` (default: green)
- Neutral → Optionally uses `Neutral Color`

**Method 2: Interaction Detection (IHelsincyTargetInterface)**

If the target implements `IHelsincyTargetInterface` and returns a GameplayTag:
- Checks `Visuals Config → Interaction Color Map` for a matching entry
- If matched → Uses the mapped color
- If not matched but `Use Default Interaction Color` is enabled → Uses the default interaction color

**Ideal for**: Pickups, interactable objects, terminals, switches, etc.

### 4.6 Preset System

Save and load crosshair configurations at runtime:

| Node | Function |
|------|----------|
| `Save Preset` | Save current configuration to memory under a name |
| `Load Preset` | Load a saved configuration by name (returns success/fail) |
| `Clear Preset Library` | Clear all saved presets |
| `Get All Preset Names` | Get all preset names in the library (useful for building UI lists) |

#### Disk Persistence

The plugin includes built-in USaveGame persistence. Presets survive across level transitions and game restarts:

| Node | Function |
|------|----------|
| `Save Presets To Disk` | Save the preset library + active preset name to disk (default slot: `CrosshairPresets`) |
| `Load Presets From Disk` | Restore presets from disk and auto-apply the last active preset |
| `Does Save Exist` | Check if a save file exists on disk |
| `Delete Save From Disk` | Delete the save file from disk |

#### Auto Persistence (Zero-Code Mode)

Enable these properties in the component's Details panel for automatic save/load:

| Property | Default | Description |
|----------|---------|-------------|
| `Auto Load On Begin Play` | false | Auto-load presets from disk on BeginPlay |
| `Auto Save On End Play` | false | Auto-save presets to disk on EndPlay |
| `Auto Save Slot Name` | `CrosshairPresets` | Slot name used for auto save/load |

> **Tip**: Save files are stored at `{ProjectDir}/Saved/SaveGames/CrosshairPresets.sav`.

### 4.7 State Queries

| Node | Returns |
|------|---------|
| `Get Current Profile` | Full active crosshair configuration |
| `Get Current Visual Primary Color` | Current primary color (includes target color override) |
| `Get Final Spread` | Total spread (X, Y) = state spread + recoil |
| `Get Current Target Attitude` | Target attitude: Hostile / Friendly / Neutral |
| `Get Current Target Tag` | Target's interaction GameplayTag |

---

## 5. Damage Indicator Module — Full Usage

### 5.1 Basic Integration

1. Add a `HelsincyDamageIndicatorComponent` to your character Blueprint.
2. In the component's Details panel, set `Default Data Asset` to the built-in `DA_DamageIndicator` and keep `Use Default Data Asset Init = true`.
3. In the character's **Any Damage** or **Take Damage** event, call the component's `Register Damage Event` node:
   - `Damage Causer`: The attacking Actor (if available)
   - `Location If No Actor`: If no Actor source, pass the world location of the damage
4. Ensure HUD has indicator drawing:
   - **Method A** (using `BP_HelsincySuperHUD`): Automatic — no extra work needed.
   - **Method B** (Bridge): Add a `Draw Damage Indicators For HUD` node in your HUD.

### 5.2 Configure Indicator Appearance

In the component's Details panel → `Indicator Profile`:

- `Indicator Style Tag`: Choose `Indicator.Style.Arrow` (arrow), `Indicator.Style.Image` (custom image), or `Indicator.Style.Arc` (arc)
- `Placement Mode`: choose `Radial Circle` or `Window Edge`.
- `Edge Margin`: safe margin from the game Canvas edge in Window Edge mode.
- `Edge Corner Padding`: extra corner avoidance for Window Edge placement.
- `Hide Circle In Window Edge Mode`: hide the radial circle by default while using Window Edge.
- `Duration`: How long the indicator stays visible
- `Radius`: Circle ring radius
- `Style|Arrow → Arrow Config → Size / Color`: Arrow size and color
- `Fade In Time` / `Fade Out Time`: Fade timing
- `Show Circle`: Whether to display the background circle ring

> `Window Edge` uses the current game window/viewport Canvas bounds, `Canvas->ClipX / ClipY`. It does not use physical monitor bounds. In windowed mode, indicators follow the game window size.

If using `Indicator.Style.Image`, also configure:
- `Style|Image → Image Config → Texture`: Your custom image
- `Style|Image → Image Config → Size`: Image size
- `Style|Image → Image Config → Rotate Image`: Whether the image rotates to face the damage source

### Arc Damage Indicator

`Indicator.Style.Arc` draws a COD-like red arc damage indicator. You can assign `ArcMaskTexture`, or leave it empty to use the runtime-generated default arc mask.

Recommended defaults:

- `Style|Arc → Arc Config → DirectionCueMode = CenterNib`
- `Style|Arc → Arc Config → DirectionCueStrength = 1.0`
- `PlacementMode = WindowEdge`

`CenterNib` draws a short center cue so players can read the incoming direction instead of guessing from the arc alone.

### 5.3 Runtime Controls

| Node | Function |
|------|----------|
| `Is Enabled` | Check if indicator is active |
| `Get Indicator Profile` | Get current configuration |
| `Set Indicator Profile` | Modify configuration at runtime |
| `Load From Data Asset` | Load the full configuration from `UHelsincyDamageIndicatorDataAsset` |
| `Save Preset` / `Load Preset` | Save or load in-memory presets |
| `Save Presets To Disk` / `Load Presets From Disk` | Persist damage indicator presets to disk; default slot is `DamageIndicatorPresets` |

---

## 6. Crosshair Shapes Overview

The plugin includes 10 built-in crosshair shapes, selected via the `ShapeTag` field:

| Shape | ShapeTag | Config Used | Description |
|-------|----------|-------------|-------------|
| Cross | `Crosshair.Shape.Cross` | CrosshairConfig | Classic FPS crosshair with 4 arms |
| Circle | `Crosshair.Shape.Circle` | RadialConfig | Spread circle. Adjustable segments and radius |
| Dot Only | `Crosshair.Shape.DotOnly` | (CenterDotConfig) | Minimal crosshair — center dot only |
| T-Style | `Crosshair.Shape.TStyle` | CrosshairConfig | Three-arm crosshair (no top arm) |
| Triangle | `Crosshair.Shape.Triangle` | BoxConfig | Isosceles triangle |
| Rectangle | `Crosshair.Shape.Rectangle` | BoxConfig | Rectangular frame |
| Chevron | `Crosshair.Shape.Chevron` | CrosshairConfig | V-shape. Adjustable opening angle |
| Polygon | `Crosshair.Shape.Polygon` | RadialConfig | Pentagon, hexagon, etc. Adjustable side count |
| Wings | `Crosshair.Shape.Wings` | WingsConfig | Multi-segment horizontal lines. Great for flight/glide HUD |
| Image | `Crosshair.Shape.Image` | ImageConfig | Your custom texture |

### How to Select a Shape

In the DataAsset or component's `Current Profile → Shape Tag` dropdown, select any Tag under `Crosshair.Shape.*`.

---

## 7. Configuration Reference

All configuration lives under `FHelsincyCrosshairProfile`, editable in any DataAsset or directly on the component's Details panel.

### 7.1 Visual Settings (Visuals Config)

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Primary Color | Color | White | Base crosshair color |
| Global Crosshair Offset | Vector2D | (0, 0) | Offset from screen center (px). Y=-50 shifts crosshair up (good for TPP) |
| Thickness | Float | 2.0 | Line thickness (px). Range: 1–30 |
| Opacity | Float | 1.0 | Global opacity. Range: 0–1 |
| Enable Outline | Bool | true | Enable outline around crosshair lines |
| Outline Color | Color | Black | Outline color |
| Outline Thickness | Float | 1.0 | Outline width. Range: 1–20 |
| Enable Target Switching | Bool | true | Enable target detection color switching |
| Max Switching Distance | Float | 10000 | Max detection distance (cm). Range: 5000–30000 |
| Switching Channel | Collision Channel | ECC_Pawn | Trace channel for target detection |
| Enemy Color | Color | Red | Color when aiming at hostiles |
| Friendly Color | Color | Green | Color when aiming at friendlies |
| Interaction Color Map | Tag→Color Map | Empty | Custom tag-to-color mapping for interactive objects |

### 7.2 Dynamic Spread Settings (Dynamic Config)

| Parameter | Default | Description |
|-----------|---------|-------------|
| Enable Dynamic Spread | true | Master switch. When off, crosshair remains static |
| Velocity Multiplier | 0.05 | Movement speed → spread factor. Higher = more spread when moving |
| Airborne Penalty | 50.0 | Fixed spread penalty while airborne |
| State Interp Speed | 15.0 | How fast state spread responds. Higher = faster response |
| Recoil Recovery Speed | 10.0 | How fast recoil recovers. Higher = faster recovery |
| Max Recoil Spread | 100.0 | Maximum recoil spread cap (prevents going off-screen) |

### 7.3 Hit Marker Settings (Hit Marker Config)

| Parameter | Default | Description |
|-----------|---------|-------------|
| Enabled | true | Enable hit markers |
| `CrosshairVisibilityWhileActive` | Hide | Base crosshair policy while any HitMarker is visible: `Hide`, `KeepVisible`, or `ScaleAlpha` |
| `CrosshairAlphaScaleWhileHitMarkerActive` | 0.10 | Alpha multiplier used only by `ScaleAlpha`; preserves the old SingleInstance weakening style |
| `bApplyHitMarkerVisibilityPolicyToCenterDot` | true | Applies the same hide or alpha policy to the center dot |
| Use Tapered Shape | true | CoD-style tapered lines (thick-center, thin-ends). False = uniform lines |
| Duration | 0.25s | How long the marker stays visible |
| Color | White | Normal hit color |
| Headshot Color | Red | Headshot indicator color |
| Head Shot Scale | 1.3 | Headshot size multiplier |
| Kill Color | Red | Kill marker color |
| Kill Scale | 1.7 | Kill size multiplier |
| Shake Intensity | 5.0 | Global shake intensity (px) |
| Normal Shake Intensity | 5.0 | Per-arm shake ("electric" trembling effect) |
| Shake Frequency | 34.0 | Deterministic shake frequency. Higher values snap back faster |
| Shake Damping | 12.0 | Deterministic shake damping. Higher values settle faster |
| Headshot Shake Multiplier | 1.25 | Extra shake energy injected by headshots |
| Kill Shake Multiplier | 1.5 | Extra shake energy injected by kills |
| Use Impact Motion | true | Enable COD-style twist/scale impact motion |
| Impact Rotation Degrees | 6.0 | Base twist angle applied on hit |
| Impact Scale Punch | 0.18 | Whole-marker scale punch on hit |
| Impact Arm Length Punch | 0.14 | Arm-length expansion on hit |
| Impact Translation Weight | 0.25 | Secondary translation shake weight; twist and scale remain the primary feedback |
| Impact Motion Duration | 0.18s | Main impact motion duration. Shorter values feel harder |
| Damage To Impact Scale | 0.35 | Additional impact response from normalized damage |
| Max Impact Motion Energy | 1.8 | Maximum impact motion energy under rapid hits |
| Shake Decay | true | Shake diminishes over time |
| Custom Texture | None | Optional custom hit marker image |

### 7.4 Center Dot Settings (Center Dot Config)

| Parameter | Default | Description |
|-----------|---------|-------------|
| Enabled | true | Show center dot |
| Always Stay Centered | true | Dot stays at true screen center (ignores GlobalCrosshairOffset) |
| Size | 2.0 | Dot radius (px) |
| Opacity | 1.0 | Opacity |
| Color | White | Dot color |

---

## 8. Extension Guide — Custom Crosshair Shapes

Create completely new crosshair shapes via **Blueprint** or **C++**.

### 8.1 Blueprint Method (No C++ Required)

#### Step 1: Create the Renderer Blueprint

1. Right-click in Content Browser → **Blueprint Class**.
2. Under "All Classes", search `HelsincyShapeRenderer` → Select it as the parent class.
3. Name it, e.g., `BP_MyCustomCrosshair`.

#### Step 2: Set Up the Tag

1. Open the Blueprint → In Class Defaults, find `Associated Tag`.
2. Set a custom GameplayTag, e.g., `Crosshair.Shape.MyCustom`.
   - You'll need to create this tag in **Project Settings → Gameplay Tags** first.

#### Step 3: Implement Drawing Logic

1. In the Event Graph, find the **Event On Draw Crosshair** (`ReceiveDraw`).
2. Use the provided parameters to draw your custom shape:
   - `Canvas`: The drawing canvas
   - `Profile`: Full crosshair config (read colors, thickness, etc.)
   - `Spread`: Current spread amount (X horizontal, Y vertical)
   - `Center`: Screen center coordinates
   - `Final Color`: Pre-calculated final color (includes target color override)
   - `Delta Time`: Frame delta time
   - `Scale`: Resolution-adaptive scale factor

3. Use `Draw Line`, `Draw Texture`, etc. on the Canvas to create your shape.

#### Step 4: Register the Renderer

Open **Edit → Project Settings → Engine → HelsincyCrosshairSystem**:

In the `Blueprint Renderers` array, add your Blueprint class `BP_MyCustomCrosshair`.

#### Step 5: Use It

Select `Crosshair.Shape.MyCustom` in any DataAsset or component's `Shape Tag` — your custom shape is now active.

### 8.2 C++ Method

```cpp
UCLASS()
class UMyCustomRenderer : public UHelsincyShapeRenderer
{
    GENERATED_BODY()
public:
    UMyCustomRenderer()
    {
        AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.MyCustom"));
    }

    virtual void Draw(UCanvas* Canvas, const FHelsincyCrosshairProfile& Profile,
        FVector2D Spread, FVector2D Center, FLinearColor Color,
        float DeltaTime, float Scale) override
    {
        // Your custom drawing logic here
    }
};
```

C++ renderers are not registered just because the class exists. Built-in C++ renderers are manually registered by the plugin during Subsystem initialization; project-side custom C++ renderers must also call the registration API at an appropriate initialization point, for example:

```cpp
if (GEngine)
{
    if (UHelsincyCrosshairManagerSubsystem* Manager =
        GEngine->GetEngineSubsystem<UHelsincyCrosshairManagerSubsystem>())
    {
        Manager->RegisterRenderer(
            FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.MyCustom")),
            UMyCustomRenderer::StaticClass());
    }
}
```

If you want a fully editor-driven workflow, create a Blueprint class derived from `UHelsincyShapeRenderer` and add it to `HelsincyCrosshairSystem → Blueprint Renderers`.

---

## 9. Extension Guide — Custom Damage Indicator Styles

### 9.1 Blueprint Method

#### Step 1: Create the Indicator Renderer Blueprint

1. Right-click → Blueprint Class → Select `HelsincyIndicatorRenderer` as parent.
2. Name it, e.g., `BP_MyDamageIndicator`.

#### Step 2: Set Up the Tag

In Class Defaults, set `Associated Tag` to a custom tag, e.g., `Indicator.Style.MyCustom`.

#### Step 3: Implement Drawing Logic

In **Event On Draw Pointer** (`ReceiveDrawPointer`), use the provided parameters:

- `Canvas`: Drawing canvas
- `Profile`: Damage indicator configuration
- `Center`: Legacy radial center in the current Canvas/window
- `Angle`: Direction angle in degrees (0=up, 90=right, 180=down, 270=left)
- `Alpha`: Current opacity (0–1, fade-in/out pre-calculated)
- `Scale`: Resolution-adaptive scale factor

> Compatibility note: Existing custom renderers that override `DrawPointer` keep compiling and keep their radial fallback behavior. Blueprint `ReceiveDrawPointer` remains the existing legacy-parameter event. C++ renderers that need true `Window Edge` placement should override `GetDesiredPointerSize` and `DrawPointerResolved`.

#### Step 4: Register

In **Project Settings → HelsincyDamageIndicatorSystem**, add your Blueprint class to the `Blueprint Indicator Renderers` / `BlueprintIndicatorRenderers` array.

#### Step 5: Use It

Select your custom tag in `DamageIndicatorComponent → Indicator Profile → Indicator Style Tag`.

### 9.2 C++ Method

```cpp
UCLASS()
class UMyDamageIndicator : public UHelsincyIndicatorRenderer
{
    GENERATED_BODY()
public:
    UMyDamageIndicator()
    {
        AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.MyCustom"));
    }

    virtual void DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile,
        FVector2D Center, float Angle, float Alpha, float Scale) override
    {
        // Your custom drawing logic here
    }
};
```

C++ damage indicator renderers also need manual registration, or you can use a Blueprint renderer and add it to `HelsincyDamageIndicatorSystem → Blueprint Indicator Renderers`.

```cpp
if (GEngine)
{
    if (UHelsincyDamageIndicatorSubsystem* Manager =
        GEngine->GetEngineSubsystem<UHelsincyDamageIndicatorSubsystem>())
    {
        Manager->RegisterDamageIndicatorRenderer(
            FGameplayTag::RequestGameplayTag(FName("Indicator.Style.MyCustom")),
            UMyDamageIndicator::StaticClass());
    }
}
```

If your C++ indicator needs true `Window Edge` placement, override `GetDesiredPointerSize` and `DrawPointerResolved`; overriding only `DrawPointer` keeps the legacy radial fallback semantics.

---

## 10. Debug Mode

The plugin includes a comprehensive debug system to help troubleshoot issues during development.

### 10.1 ShowDebug Panels

While the game is running, open the console (press `~`) and type:

| Command | Function |
|---------|----------|
| `showdebug Crosshair` | Show crosshair debug panel: Profile, spread, color, target detection, hit markers, subsystem status |
| `showdebug DamageIndicator` | Show damage indicator debug panel: Config, active indicators, circle parameters, subsystem status |

### 10.2 Console Variables (CVars)

#### Crosshair Debug

| Variable | Default | Description |
|----------|---------|-------------|
| `hc.Debug.Enable` | 0 | Master switch. Set to 1 to enable crosshair debug |
| `hc.Debug.Text` | 1 | Enable on-screen text debug output |
| `hc.Debug.Geometry` | 0 | Enable geometry visualization (debug lines and boxes on screen) |
| `hc.Debug.VerboseLog` | 0 | Enable verbose log output to Output Log |
| `hc.Debug.OnlyLocal` | 1 | Only output debug for locally controlled character |

#### Damage Indicator Debug

| Variable | Default | Description |
|----------|---------|-------------|
| `di.Debug.Enable` | 0 | Master switch. Set to 1 to enable indicator debug |
| `di.Debug.Text` | 1 | Enable on-screen text debug output |
| `di.Debug.Geometry` | 0 | Enable geometry visualization |
| `di.Debug.VerboseLog` | 0 | Enable verbose log output |

### 10.3 Performance Stats

The plugin uses UE4's built-in `STAT` profiling system. While the game is running, open the console and type:

| Command | Function |
|---------|----------|
| `stat HelsincyCrosshair` | Show crosshair module cost: component tick, HUD draw, Bridge draw, renderer draw, HitMarker draw, and target detection |
| `stat HelsincyDamageIndicator` | Show damage indicator module cost: component tick, indicator update, Bridge draw, and indicator rendering |

Common stat entries:

| Stat | Meaning |
|------|---------|
| `Crosshair Component Tick` | Per-frame crosshair state update, including spread, hit marker state, and target-detection scheduling |
| `Crosshair HUD Draw` | Overall cost of the crosshair draw path inside `AHelsincyHUD::DrawHUD` |
| `Crosshair Bridge Draw` | Overall cost of the Bridge API crosshair draw entry |
| `Crosshair Renderer Draw` | Draw cost of the active crosshair shape renderer |
| `HitMarker Draw` | Hit marker rendering cost |
| `Target Detection Request` / `Target Detection Callback` | Async target-detection request and callback processing cost |
| `DamageIndicator Component Tick` | Per-frame damage indicator component update |
| `DamageIndicator Update Indicators` | Lifetime, angle, and smoothing updates for active damage indicators |
| `DamageIndicator Bridge Draw` | Overall cost of the Bridge API damage indicator draw entry |
| `DamageIndicator Draw Indicators` | Circle, WindowEdge/RadialCircle placement, and concrete renderer draw cost |

These stats are intended for development and profiling builds. To hide a stat panel, enter the same `stat` command again.

### 10.4 Usage Example

```
~ (open console)
hc.Debug.Enable 1       -- Enable crosshair debug
hc.Debug.Geometry 1     -- Show spread box and center reference lines
showdebug Crosshair     -- Show full debug panel
stat HelsincyCrosshair  -- Show crosshair performance stats
```

> Debug features are stripped from Shipping builds — zero runtime overhead.

---

## 11. Frequently Asked Questions (FAQ)

### Q1: I can't see the crosshair

**Checklist**:
1. ✅ Is the HUD set to `BP_HelsincySuperHUD`, or does your HUD call `Draw Crosshair For HUD`?
2. ✅ Does the character have a `HelsincyCrosshairComponent`?
3. ✅ Is this character locally controlled? (Only local players see the crosshair in multiplayer)
4. ✅ Is `Enabled Crosshair` set to true on the component?
5. ✅ Is `Current Profile → Shape Tag` set to a valid tag (e.g., `Crosshair.Shape.Cross`)?

### Q2: Crosshair doesn't spread when moving / firing

**Checklist**:
1. ✅ Is `Dynamic Config → Enable Dynamic Spread` set to true?
2. ✅ Is `Add Recoil` being called when firing?
3. ✅ Does the character have a `CharacterMovementComponent`? (Required for velocity/airborne calculations)

### Q3: Crosshair doesn't change color when aiming at targets

**Checklist**:
1. ✅ Is `Visuals Config → Enable Target Switching` set to true?
2. ✅ Can the `Switching Channel` trace channel hit the target? (Default: ECC_Pawn)
3. ✅ Does the target implement `GenericTeamAgentInterface` or `IHelsincyTargetInterface`?
4. ✅ Is there anything blocking the line of sight between the crosshair and the target?

### Q4: Damage indicators don't appear

**Checklist**:
1. ✅ Does the character have a `HelsincyDamageIndicatorComponent`?
2. ✅ Does `Default Data Asset` point to `DA_DamageIndicator`, or is `Indicator Profile → Enabled` set to true?
3. ✅ Is `Indicator Style Tag` a valid style (for example `Indicator.Style.Arc` / `Indicator.Style.Arrow`)?
4. ✅ Is `Register Damage Event` being called when damage is taken?
5. ✅ Does the HUD have damage indicator drawing? (Method A handles it automatically; Method B requires `Draw Damage Indicators For HUD`)

### Q5: Hit markers don't appear

**Checklist**:
1. ✅ Is `Hit Marker Config → Enabled` set to true?
2. ✅ Is `Trigger Hit Marker` being called after hit confirmation?
3. ✅ Make sure you call it after damage is confirmed, not just when a ray hits something.

### Q6: How do I use different crosshairs for different weapons?

1. Create a `HelsincyCrosshairDataAsset` for each weapon type.
2. Store a reference to the DataAsset on each weapon Actor.
3. When equipping/switching weapons, call `Load From Data Asset` on the crosshair component.

### Q7: Do all players see the crosshair in multiplayer?

No. The crosshair component only enables Tick for the **locally controlled character**. Remote players' crosshair components don't run and produce zero performance overhead.

### Q8: Can I create custom crosshair shapes in Blueprint?

Absolutely! See [Chapter 8](#8-extension-guide--custom-crosshair-shapes) for the full Blueprint extension guide. It takes just 5 steps to register a brand new shape.

### Q9: Does it support UMG Widget crosshairs?

No. This plugin focuses on the Canvas high-performance rendering path, which is the best practice for crosshairs in FPS games. UMG Widget overhead is undesirable for high-framerate FPS titles.

### Q10: Do debug features add overhead in Shipping builds?

No. All debug code is wrapped in `#if !UE_BUILD_SHIPPING` — Shipping builds contain zero debug code.

---

## 12. Appendix — Blueprint API Quick Reference

### Crosshair Component (UHelsincyCrosshairComponent)

| Node Name | Type | Description |
|-----------|------|-------------|
| Enable Crosshair | Callable | Turn on crosshair |
| Disable Crosshair | Callable | Turn off crosshair |
| Is Enabled Crosshair | Pure | Check if enabled |
| Enable Center Dot | Callable | Show center dot |
| Disable Center Dot | Callable | Hide center dot |
| Enable Target Detection | Callable | Enable target detection |
| Disable Target Detection | Callable | Disable target detection |
| Add Recoil | Callable | Add recoil spread |
| Trigger Hit Marker | Callable | Trigger normal hit marker |
| Trigger Hit Marker Color | Callable | Trigger custom-color hit marker |
| Trigger Headshot Marker | Callable | Trigger headshot marker |
| Trigger Kill Marker | Callable | Trigger kill marker |
| Trigger Hit Marker Advanced | Callable | Trigger hit marker with priority, color, and damage-scaled impact feedback |
| Load From Data Asset | Callable | Load config from DataAsset |
| Update Profile | Callable | Overwrite current profile |
| Save Preset | Callable | Save preset to memory |
| Load Preset | Callable | Load preset from memory |
| Clear Preset Library | Callable | Clear all presets |
| Get All Preset Names | Pure | Get all preset names |
| Save Presets To Disk | Callable | Save preset library to disk |
| Load Presets From Disk | Callable | Restore preset library from disk |
| Does Save Exist | Pure | Check if disk save exists |
| Delete Save From Disk | Callable | Delete disk save |
| Get Current Profile | Pure | Get active config |
| Get Current Visual Primary Color | Pure | Get current color |
| Get Crosshair Presentation State | Pure | Get base crosshair adaptive presentation state |
| Get Final Spread | Pure | Get total spread |
| Get State Spread | Pure | Get state spread |
| Get Recoil Spread | Pure | Get recoil spread |
| Get Current Target Attitude | Pure | Get target team attitude |
| Get Current Target Tag | Pure | Get target interaction tag |
| Is Single Hit Marker Visible | Pure | Check whether the single-instance hit marker is visible |
| Get Single Hit Marker Phase | Pure | Get the current single-instance hit marker phase |
| Get Single Hit Marker Opacity | Pure | Get the current single-instance hit marker opacity |
| Get Single Hit Marker Impact Energy | Pure | Get the current single-instance hit marker impact energy |

### Damage Indicator Component (UHelsincyDamageIndicatorComponent)

| Node Name | Type | Description |
|-----------|------|-------------|
| Register Damage Event | Callable | Register incoming damage |
| Set Indicator Profile | Callable | Update indicator config |
| Get Indicator Profile | Pure | Get indicator config |
| Is Enabled | Pure | Check if enabled |
| Load From Data Asset | Callable | Load config from a damage indicator DataAsset |
| Save Preset | Callable | Save damage indicator preset to memory |
| Load Preset | Callable | Load damage indicator preset from memory |
| Clear Preset Library | Callable | Clear damage indicator preset library |
| Get All Preset Names | Pure | Get all damage indicator preset names |
| Save Presets To Disk | Callable | Save damage indicator preset library to disk |
| Load Presets From Disk | Callable | Restore damage indicator preset library from disk |
| Does Save Exist | Pure | Check if damage indicator disk save exists |
| Delete Save From Disk | Callable | Delete damage indicator disk save |

### Bridge API (Blueprint Function Libraries)

| Node Name | Library | Description |
|-----------|---------|-------------|
| Draw Crosshair For HUD | CrosshairRenderLibrary | Draw crosshair in HUD context |
| Draw Crosshair For Controller | CrosshairRenderLibrary | Low-level render entry |
| Draw Damage Indicators For HUD | DamageIndicatorRenderLibrary | Draw damage indicators in HUD context |
| Draw Damage Indicators For Controller | DamageIndicatorRenderLibrary | Low-level render entry |
