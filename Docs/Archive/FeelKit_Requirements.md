# FeelKit: Requirements Document

| Field | Value |
|---|---|
| Product | FeelKit (working title, check Fab for name conflicts) |
| Type | Unreal Engine code plugin (engine plugin, prebuilt binaries + source) |
| Channel | Fab |
| Version of this document | 0.1 (draft) |
| Owner | Bill |

---

## 1. Purpose

FeelKit lets developers author game feel once as reusable **Recipes**, trigger them with one Blueprint node, and have every effect automatically respect the player's **Comfort settings**.

### 1.1 Product pillars

| ID | Pillar | Meaning |
|---|---|---|
| P1 | Runtime | Fast, dependency-free C++ feedback player |
| P2 | Authoring | Timeline editor with instant preview (main differentiator) |
| P3 | Comfort | Per-player intensity and safety layer applied to every effect |

### 1.2 Priority legend

| Tag | Meaning |
|---|---|
| **M** | MVP (required for first internal milestone) |
| **V1** | Required for public launch |
| **P** | Premium differentiator (launch or post-launch update) |
| **L** | Included in free Lite version |

---

## 2. Scope

### 2.1 In scope
- Runtime feedback engine (camera, time, screen, actor, audio, haptics, UI, spawn)
- Recipe data assets and timeline authoring editor
- Comfort layer with presets, substitution, flash limiter
- Engine-native trigger component
- Sample recipes, demo map, sample comfort widget
- Documentation, trailer material, free Lite version

### 2.2 Out of scope
- VFX or material content libraries (one sample per step only)
- Combat, damage, ability or GAS logic
- Full settings menu framework
- Camera rigs or camera modes
- Movement features (coyote time, input buffering)
- Any claim of accessibility or photosensitivity certification

---

## 3. Platform and Engine Constraints

| ID | Requirement | Priority |
|---|---|---|
| C-001 | Support UE 5.6, 5.7 and 5.8 at launch. Primary development and all content authoring happen in 5.6 (the oldest supported version) | V1 |
| C-001c | All plugin content (.uasset, .umap) is created and saved only in 5.6. Never resave plugin content in 5.7 or 5.8; discard such changes in version control | M |
| C-001d | Newer-engine APIs in C++ are allowed only behind version macros | M |
| C-001a | Every milestone must compile and pass automation tests on 5.6, 5.7 and 5.8 (BuildPlugin per version) | M |
| C-001b | Keep 5.8 support for the full UE5 lifetime; evaluate UE6 only after its Early Access | V1 |
| C-002 | Core runtime depends only on stock modules: Core, CoreUObject, Engine, InputCore, Slate, SlateCore, UMG, GameplayTags, DeveloperSettings | M |
| C-003 | Niagara and Enhanced Input support live in optional modules that compile only when those plugins are enabled | V1 |
| C-004 | Target platforms: Win64 (required), Linux and Mac (V1 if testable), consoles documented as "untested, source provided" | V1 |
| C-005 | Distributed through Fab as an engine plugin with prebuilt binaries and full source | V1 |
| C-006 | Must load and function in a Blueprint-only project installed from the launcher engine | M |
| C-007 | Documentation must state: packaging a Blueprint-only project with a code plugin requires Visual Studio with the C++ workload | V1 |
| C-008 | No engine source modifications | M |
| C-009 | Editor module must be excluded from shipping builds (`Type: Editor`) | M |

---

## 4. Core Concepts and Data Model

### 4.1 Recipe (`UFeelRecipe : UPrimaryDataAsset`)

| Field | Type | Notes |
|---|---|---|
| Tracks | `TArray<FFeelTrack>` | Ordered by start time |
| Duration | float (derived) | Max of track end times |
| Cooldown | float | Seconds, per target |
| MaxConcurrent | int32 | 0 = unlimited |
| DefaultIntensity | float | Multiplier |
| Category | GameplayTag | For browser filtering (e.g. `FeelRecipe.Genre.Platformer`) |
| Description | FText | Shown in browser |

### 4.2 Track (`FFeelTrack`)

| Field | Type | Notes |
|---|---|---|
| Step | `UFeelStep*` (Instanced) | The effect |
| StartTime | float | Seconds from recipe start |
| Duration | float | Seconds; 0 = instant step |
| Channel | GameplayTag | e.g. `Feel.Camera.Shake`; drives comfort scaling |
| IntensityCurve | `FRuntimeFloatCurve` | Evaluated over normalized track time |
| bEssential | bool | Gameplay-critical information |
| SubstituteStep | `UFeelStep*` (Instanced) | Used when comfort disables the channel and bEssential is true |
| EssentialFloor | float | Minimum scale when bEssential and no substitute |
| Conditions | `FFeelConditions` | Local player only, max distance, chance, platform filter |
| bEnabled | bool | Editor mute |

### 4.3 Step (`UFeelStep : UObject`, `EditInlineNew`, `Blueprintable`)

Lifecycle, all overridable in C++ and Blueprint:

| Function | Called |
|---|---|
| `OnStart(Context)` | Track begins |
| `OnUpdate(Context, Alpha, Intensity)` | Every frame while active |
| `OnStop(Context, bInterrupted)` | Track ends or is cancelled |
| `GetDefaultChannel()` | Used when a track is created in the editor |
| `SupportsPreview()` | Whether editor preview can simulate it |

`FFeelContext` provides: world, target, owning player controller, instance handle, recipe, real-time elapsed.

### 4.4 Handle (`FFeelHandle`)
- Opaque ID, Blueprint-copyable, safe to hold after instance ends.
- `IsValid`, `IsPlaying` queries.

---

## 5. Functional Requirements: Runtime

### 5.1 Subsystem

| ID | Requirement | Priority |
|---|---|---|
| RT-001 | `UFeelSubsystem` (`UTickableWorldSubsystem`) owns all active instances; no actor placement required | M, L |
| RT-002 | Subsystem ticks only when at least one instance is active | M, L |
| RT-003 | All instance timing uses unscaled real time (unaffected by global or actor time dilation) | M, L |
| RT-004 | Instances are pooled; no per-play UObject allocation for the instance itself | V1 |
| RT-005 | Cooldown and MaxConcurrent are enforced per recipe and per target | M |
| RT-006 | Instances stop safely when the target is destroyed or the world tears down; no dangling state | M, L |
| RT-007 | Global kill switch: console variable `feel.Enabled 0` disables all playback | M, L |
| RT-008 | Global intensity console variable `feel.GlobalScale` | V1 |

### 5.2 Public API

| ID | Requirement | Priority |
|---|---|---|
| API-001 | `PlayFeel(WorldContext, Recipe, Target, Intensity) -> FFeelHandle` (Blueprint and C++) | M, L |
| API-002 | `StopFeel(Handle, bBlendOut)` | M, L |
| API-003 | `StopAllFeel(Target optional)` | M, L |
| API-004 | Async Blueprint node `Play Feel And Wait` with `OnFinished` and `OnCancelled` pins | V1 |
| API-005 | `FFeelTarget` supports: Actor, SceneComponent, World Location, Local Player Camera | M, L |
| API-006 | Anim notify `Play Feel` (targets owning actor) | V1 |
| API-007 | Native C++ delegate `OnFeelStarted` / `OnFeelFinished` on the subsystem | V1 |
| API-008 | All Blueprint functions in category `Feel`, with tooltips on every pin | M |

### 5.3 Channel arbiters

| ID | Requirement | Priority |
|---|---|---|
| ARB-001 | Camera arbiter: per-project mode `StrongestWins` (default) or `AdditiveCapped` with a max value | M |
| ARB-002 | Time arbiter: single active owner per mode (global / per-actor); highest priority wins; previous dilation always restored on stop, cancel, or target destruction | M |
| ARB-003 | Screen arbiter: one weighted post-process contribution per channel; weights combine by strongest-wins | M |
| ARB-004 | Haptics arbiter: strongest-wins per controller | V1 |
| ARB-005 | Arbiter decisions are exposed to the debugger (capped, suppressed, substituted) | V1 |

### 5.4 Camera and screen delivery

| ID | Requirement | Priority |
|---|---|---|
| DEL-001 | Camera and screen effects are delivered through a `UCameraModifier` added automatically to the local player's camera manager; buyer adds nothing | M |
| DEL-002 | Post-process effects applied through the modifier's post-process hook; no volume placement required | M |
| DEL-003 | Works with any camera setup (first person, third person, spring arm, cine camera) that uses the standard camera manager | M |
| DEL-004 | Split-screen: effects apply only to the intended local player | V1 |

### 5.5 Step catalog

| ID | Step | Channel | Priority |
|---|---|---|---|
| ST-001 | Procedural Shake (Perlin, sine, directional) | Camera.Shake | M, L |
| ST-002 | Camera Punch / Offset | Camera.Motion | M, L |
| ST-003 | FOV Kick | Camera.Motion | M, L |
| ST-004 | Camera Roll | Camera.Motion | V1 |
| ST-005 | Camera Zoom | Camera.Motion | V1 |
| ST-006 | Look-at Nudge | Camera.Motion | V1 |
| ST-007 | Global Hitstop | Time.Hitstop | M, L |
| ST-008 | Actor Hitstop (custom time dilation) | Time.Hitstop | M |
| ST-009 | Slow-mo Ramp | Time.SlowMo | M |
| ST-010 | Screen Flash | Screen.Flash | M, L |
| ST-011 | Vignette Pulse | Screen.Distortion | M, L |
| ST-012 | Chromatic Aberration | Screen.Distortion | M |
| ST-013 | Desaturate | Screen.Color | V1 |
| ST-014 | Radial Blur | Screen.Distortion | V1 |
| ST-015 | Color Tint | Screen.Color | V1 |
| ST-016 | Screen Fade | Screen.Fade | V1 |
| ST-017 | Scale Punch | Actor.Transform | M, L |
| ST-018 | Squash and Stretch | Actor.Transform | M |
| ST-019 | Material Parameter Pulse | Actor.Material | M |
| ST-020 | Mesh Wobble | Actor.Transform | V1 |
| ST-021 | Light Flash | Actor.Light | V1 |
| ST-022 | Play Sound (2D / attached / at location) | Audio | M, L |
| ST-023 | Submix Duck | Audio | V1 |
| ST-024 | Pitch Bend | Audio | V1 |
| ST-025 | Low-pass Sweep | Audio | V1 |
| ST-026 | Force Feedback Curve | Haptics | M |
| ST-027 | Haptic Pattern | Haptics | V1 |
| ST-028 | Adaptive Trigger (DualSense) | Haptics | P |
| ST-029 | Widget Punch | UI | V1 |
| ST-030 | Widget Shake | UI | V1 |
| ST-031 | Widget Flash | UI | V1 |
| ST-032 | Number Pop | UI | V1 |
| ST-033 | Spawn Particle (Niagara module) | Spawn | V1 |
| ST-034 | Spawn Decal | Spawn | V1 |
| ST-035 | Recipe Step (plays a nested recipe) | Meta | V1 |
| ST-036 | Blueprint Event Step (calls a custom event) | Meta | M |
| ST-037 | Random Choice Step (picks one of N sub-steps) | Meta | V1 |

Target counts: MVP about 17 steps, V1 about 35 to 40.

### 5.6 Triggers

| ID | Requirement | Priority |
|---|---|---|
| TRG-001 | `UFeelTriggerComponent` maps events to recipes without Blueprint code | V1 |
| TRG-002 | Supported events: OnTakeAnyDamage, OnLanded (only if owner is ACharacter), Component Hit, Begin/End Overlap | V1 |
| TRG-003 | Enhanced Input action trigger in optional module | V1 |
| TRG-004 | Damage trigger can scale intensity by damage amount through a curve | V1 |

### 5.7 Networking

| ID | Requirement | Priority |
|---|---|---|
| NET-001 | Recipes are cosmetic and run locally; no gameplay state is replicated | M |
| NET-002 | `UFeelReplicationComponent` offers `PlayFeelNetworked` with modes Multicast, OwnerOnly, SkipOwner | V1 |
| NET-003 | Networked play supports a relevancy distance filter | V1 |
| NET-004 | Dedicated server never executes steps (no-op) | M |
| NET-005 | Global hitstop in multiplayer is local-only by default and documented as such | V1 |

---

## 6. Functional Requirements: Comfort Layer

### 6.1 Settings

| ID | Requirement | Priority |
|---|---|---|
| CMF-001 | Per local player channel scales (0 to 1): Master, Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion, Haptics | M, L |
| CMF-002 | Final track intensity = CallIntensity x Curve(t) x ChannelScale x MasterScale | M, L |
| CMF-003 | Presets: Default, Reduced Motion, Reduced Flashing, No Haptics | M, L |
| CMF-004 | Developers can define custom presets as data assets | V1 |
| CMF-005 | Project-level default scales in Project Settings (`UFeelSettings : UDeveloperSettings`) | M, L |
| CMF-006 | Channel mapping is data driven: new channels can be added via GameplayTags and assigned to a comfort group | V1 |

### 6.2 Persistence

| ID | Requirement | Priority |
|---|---|---|
| CMF-010 | Default persistence to a per-player config or local player save file | M, L |
| CMF-011 | `IFeelComfortStorage` interface lets buyers route load/save to their own save system | V1 |
| CMF-012 | Settings load before first recipe plays in a session | M, L |

### 6.3 Essential tracks and substitution

| ID | Requirement | Priority |
|---|---|---|
| CMF-020 | When a channel scale is 0 and the track is Essential with a SubstituteStep, the substitute plays instead | V1 |
| CMF-021 | When Essential without substitute, intensity is clamped to EssentialFloor | V1 |
| CMF-022 | Non-essential tracks at scale 0 are skipped entirely (zero cost) | M, L |
| CMF-023 | Validation warns on Essential tracks with no substitute and floor 0 | V1 |

### 6.4 Flash limiter

| ID | Requirement | Priority |
|---|---|---|
| CMF-030 | Counts flash-channel events per rolling second per player | V1 |
| CMF-031 | Above a configurable rate (default 3 per second) additional flashes are softened or suppressed (configurable) | V1 |
| CMF-032 | Reduced Flashing preset enables the limiter with stricter defaults | V1 |
| CMF-033 | Documentation states it is a mitigation helper, not a compliance tool, and references external analysis tools | V1 |

### 6.5 Motion comfort

| ID | Requirement | Priority |
|---|---|---|
| CMF-040 | Reduced Motion caps FOV change rate (degrees per second, configurable) | V1 |
| CMF-041 | Reduced Motion disables camera roll | V1 |
| CMF-042 | Reduced Motion converts directional shake to low-amplitude shake | P |

### 6.6 Sample UI

| ID | Requirement | Priority |
|---|---|---|
| CMF-050 | Sample UMG widget with all sliders, preset dropdown, and live preview button | V1, L (basic) |
| CMF-051 | Widget is gamepad navigable | V1 |
| CMF-052 | Widget uses no third-party UI framework | M |

---

## 7. Functional Requirements: Editor Tooling

### 7.1 Recipe editor

| ID | Requirement | Priority |
|---|---|---|
| ED-001 | Custom asset editor opens on double-click of a Recipe | M |
| ED-002 | Timeline panel: one row per track, color coded by channel | M |
| ED-003 | Drag to move, drag edges to resize, snap to frames (configurable FPS) | M |
| ED-004 | Add track via menu listing all step classes, including Blueprint steps | M |
| ED-005 | Mute, solo, duplicate, delete, reorder tracks | M |
| ED-006 | Details panel shows the selected track and its step properties | M |
| ED-007 | Inline intensity curve editing for the selected track | V1 |
| ED-008 | Full undo/redo for every timeline operation | M |
| ED-009 | Copy/paste tracks between recipes | V1 |
| ED-010 | Timeline zoom and scroll | M |

### 7.2 Preview

| ID | Requirement | Priority |
|---|---|---|
| PV-001 | Preview viewport inside the recipe editor with camera, sample mesh, and mock HUD | M |
| PV-002 | Play, pause, loop, and scrub without starting PIE | M |
| PV-003 | Preview simulates camera, screen, actor, and UI steps; audio plays in editor | M |
| PV-004 | Steps that cannot preview (haptics, time) show a visual indicator on the timeline | M |
| PV-005 | "Play in PIE" button fires the recipe on the local player of the running session | V1 |
| PV-006 | Comfort preset dropdown changes preview output live | V1 |
| PV-007 | Intensity graph showing combined per-channel intensity over time | V1 |
| PV-008 | Replaceable preview mesh | V1 |
| PV-009 | Recipe edits during PIE apply to the next play without restarting | V1 |

### 7.3 Browser, debugger, validation

| ID | Requirement | Priority |
|---|---|---|
| TL-001 | Recipe browser tab with genre and channel filters | V1 |
| TL-002 | Live debugger panel in PIE: active instances, time remaining, arbiter decisions, comfort scales | V1 |
| TL-003 | Gameplay Debugger category with the same data in development builds | V1 |
| TL-004 | Data validation on save: invalid curves, missing assets, zero-length tracks, essential tracks without fallback | V1 |
| TL-005 | Asset thumbnails for recipes | P |
| TL-006 | Content Browser asset action: "Create Recipe" from template list | V1 |

---

## 8. Non-functional Requirements

### 8.1 Performance

| ID | Requirement | Priority |
|---|---|---|
| NF-001 | Zero game thread cost when no instances are active (no tick registered) | M |
| NF-002 | 50 concurrent instances under 0.2 ms game thread on a reference mid-range PC (measure and publish) | V1 |
| NF-003 | No per-frame heap allocation during steady-state playback | V1 |
| NF-004 | `stat Feel` stat group with instance count and tick time | V1 |
| NF-005 | Unreal Insights trace channel for play/stop events | P |

### 8.2 Reliability

| ID | Requirement | Priority |
|---|---|---|
| NF-010 | No crash when target, controller, or world is destroyed mid-playback | M |
| NF-011 | Time dilation always restored, including on PIE stop and level travel | M |
| NF-012 | Null or missing assets in a recipe log a warning and skip the track | M |

### 8.3 Usability

| ID | Requirement | Priority |
|---|---|---|
| NF-020 | First working result in 5 minutes following the Quick Start | V1 |
| NF-021 | Works without modifying buyer GameMode, PlayerController, Character, or GameInstance | M |
| NF-022 | Every Blueprint node and property has a tooltip | V1 |

### 8.4 Code quality

| ID | Requirement | Priority |
|---|---|---|
| NF-030 | Passes Fab code plugin technical requirements | V1 |
| NF-031 | Compiles without warnings on all supported engine versions | V1 |
| NF-032 | Engine version differences isolated behind version macros | V1 |
| NF-033 | Public headers documented; source follows Epic coding standard | V1 |

---

## 9. Module Layout

| Module | Type | Contents | Tier |
|---|---|---|---|
| FeelCore | Runtime | Subsystem, recipe, track, steps, arbiters, comfort, settings, API | Lite subset + Full |
| FeelEditor | Editor | Asset editor, timeline, preview, browser, validation, debugger UI | Full |
| FeelHaptics | Runtime | Advanced haptics, DualSense triggers | Full (P) |
| FeelNiagara | Runtime (optional) | Spawn Particle step | Full |
| FeelEnhancedInput | Runtime (optional) | Input action trigger | Full |
| Content | Assets | Recipes, demo map, sample widget, sample materials | Full; Lite gets a small subset |

---

## 10. Content Deliverables

| ID | Deliverable | Priority |
|---|---|---|
| CT-001 | 10 recipes (MVP), 30 to 40 recipes (V1) across Platformer, Shooter, Action/RPG, Horror, Vehicle, UI | M / V1 |
| CT-002 | Demo map with 4 rooms (Platformer, Shooter, Action, Horror), each with Feel On/Off toggle | V1 |
| CT-003 | In-world comfort panel in demo map | V1 |
| CT-004 | Minimal sample materials for screen and actor steps | M |
| CT-005 | Sample sounds (licensed for redistribution: CC0 preferred, CC-BY with credits; license documented, see D-017) | V1 |
| CT-006 | Playable packaged demo (Windows) | V1 |

---

## 11. Documentation

| ID | Deliverable | Priority |
|---|---|---|
| DOC-001 | Quick Start (5-minute path) | V1 |
| DOC-002 | Concepts: recipes, tracks, steps, channels, comfort | V1 |
| DOC-003 | Step reference (every step, every property) | V1 |
| DOC-004 | Creating custom steps in Blueprint and C++ | V1 |
| DOC-005 | Comfort layer guide, including substitution and custom storage | V1 |
| DOC-006 | Multiplayer guide | V1 |
| DOC-007 | FAQ: Blueprint-only projects, packaging requirements, "temp target" log message | V1 |
| DOC-008 | Changelog | V1 |

---

## 12. Testing

| ID | Requirement | Priority |
|---|---|---|
| TS-001 | Automation tests for intensity math (curve x channel x master) | M |
| TS-002 | Automation tests for arbiter rules (strongest-wins, additive cap, time owner restore) | M |
| TS-003 | Automation tests for comfort substitution and floor | V1 |
| TS-004 | Automation tests for cooldown and max concurrent | M |
| TS-005 | Manual test: Blueprint-only project, launcher engine, enable plugin, use in editor | M |
| TS-006 | Manual test: package Blueprint-only project on Windows with Visual Studio installed | V1 |
| TS-007 | Manual test: packaging failure message on machine without Visual Studio, documented | V1 |
| TS-008 | Manual test: listen server and client, split-screen | V1 |
| TS-009 | Performance capture for NF-002 | V1 |
| TS-010 | Test on each supported engine version before every release | V1 |

---

## 13. Release Tiers and Pricing

| Tier | Contents | Price |
|---|---|---|
| Lite (free) | FeelCore subset (steps tagged L), comfort layer, basic sample widget, a few recipes, no editor module | Free |
| Full | All modules, all steps, editor tooling, recipes, demo map | Early access about $39.99, V1 about $69.99 to $79.99 (Personal) |
| Professional license | Same as Full | About 2 to 3x Personal |

---

## 14. Milestones and Gates

| Phase | Focus | Exit gate |
|---|---|---|
| 0 | Competitor inspection (Game Juice Pro, Agentic FeedbackFX, Adrenaline Game Juice): demos, docs | None of them has timeline/preview or a comfort layer |
| 1 | ED-001 to ED-006, ED-008, ED-010, PV-001 to PV-004 with ST-001, ST-010, ST-017 | Timeline workflow clearly better than a details panel; about 3 weeks max |
| 2 | RT-001 to RT-007, API-001 to API-003, API-005, ARB-001 to ARB-003, DEL-001 to DEL-003 | Overlapping hitstop and shake behave correctly |
| 3 | CMF-001 to CMF-022 | Presets work end to end, settings persist |
| 4 | Remaining MVP steps, validation posts (GIF + signup page) | Kill criteria check |
| 5 | V1 steps, triggers, networking, debugger, flash limiter, motion comfort | Feature complete |
| 6 | Recipes, demo map, docs, trailer, Lite, Fab submission | Launch |

### 14.1 Kill criteria

| ID | Condition | Action |
|---|---|---|
| K-001 | A competitor already ships timeline/preview or a comfort layer (Phase 0) | Stop or re-scope |
| K-002 | Phase 1 timeline is not clearly better than a details panel workflow | Stop |
| K-003 | Validation posts collect fewer than about 100 signups in 2 weeks | Stop |
| K-004 | A fourth juice plugin appears on Fab before MVP | Re-evaluate |
| K-005 | Epic announces a native feedback framework | Re-evaluate |
| K-006 | Lite gets fewer than about 1,000 library adds in its first month | Re-evaluate Full pricing and positioning |

---

## 15. Open Decisions

| ID | Question |
|---|---|
| OD-001 | Final product name |
| OD-002 | RESOLVED: 5.6, 5.7, 5.8 (see C-001). If 5.5 support is wanted, decide BEFORE creating any content, because content authoring would have to move to 5.5 |
| OD-003 | Comfort persistence default: config file or local player save game |
| OD-004 | Default camera arbitration mode |
| OD-005 | Whether DualSense support ships at launch or as an update |
| OD-006 | Whether the Lite version is a separate Fab listing or a separate plugin in the same listing |
| OD-007 | Minimum hardware for published performance numbers |
