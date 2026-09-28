# FeelKit: Product

What FeelKit is, what each edition contains, how it works, what is built, the library, the demos, and the full
requirements. Updated 2026-09-24.

**The five project documents** (all in `Docs/`):

| Document | What it holds |
|---|---|
| **FeelKit_1_Product.md** (this one) | What FeelKit is and does; editions; design; demos; requirements |
| **FeelKit_2_Launch.md** | Everything left before going live, the Fab listing, price, Fab rules, engine versions, marketing |
| **FeelKit_3_Manual_Plan.md** | The buyer manual: contents, screenshot list, video list, writing rules |
| **FeelKit_4_Log.md** | History: every change, decision (D-numbers), issue (I-numbers) and test result, with dates |
| **FeelKit_5_Research.md** | Competitors, related systems in other games and engines, the GAS research |

Older documents that these replaced are in `Docs/Archive/`. When an entry in the log names one of them, look there.
Decisions in the log override anything written here; this document is kept in line with them.

---

## 1. What FeelKit is

**One line:** gameplay says what happened (a tag and some context), FeelKit decides how it feels.

FeelKit is an Unreal Engine code plugin sold on Fab. A **recipe** is a short timeline of feedback effects: a hitstop, a
camera kick, a flash, a sound, a controller rumble, a UI punch, and so on. The game plays a recipe with one node, or
sends an event that a Feel Map turns into the right recipe. Every effect passes through the player's **comfort
settings**, so a player who turns camera shake off never gets it.

Three pillars:

| Pillar | Meaning |
|---|---|
| Authoring | A timeline editor with a live preview: build and tune a recipe without pressing Play, scrub it forwards and backwards |
| Runtime | A small C++ player with no dependencies outside the engine; nothing runs when nothing plays |
| Comfort | Per-player settings for every kind of effect, applied to FeelKit's own effects and to the engine's camera shakes and rumble |

What FeelKit is not: a combat system, an animation system, a VFX or sound library, a camera rig, a full settings-menu
framework, or an event bus. It is the layer between "something happened" and "the player feels it".

**Who it is for:** Unreal developers, Blueprint or C++, who want hits, jumps, pickups and menus to feel good without
wiring effects by hand in every Blueprint; and studios that need comfort options for accessibility reviews.

**What sets it apart** (the competitor research is in the Research document):
- The only timeline editor for game feel in Unreal, with a preview that is identical to the game (same code runs both).
- Comfort built in, per player, with presets, a ready menu and an audit, where competitors offer one global slider.
- Recent plays: hit something in the game, then open that exact moment in the editor and scrub it.
- One event, many answers: a Feel Map picks the variant by context (heavy hit, critical, surface) with no Blueprint
  branching.

## 2. Editions and price

Two Fab listings (D-081, split D-087, price D-090).

| | **FeelKit Lite** (free) | **FeelKit Pro** |
|---|---|---|
| Price | Free | $34.99 Personal, $64.99 Professional (early price; $39.99 Personal possible after reviews) |
| Recipe editor | Timeline, preview, curves, scrubbing | Same |
| Effects | 12: Procedural Shake, Camera Punch, FOV Kick, Global Hitstop, Slow-mo Ramp, Screen Flash, Vignette Pulse, Scale Punch, Squash and Stretch, Play Sound, Force Feedback Curve, Blueprint Event | All 37 |
| Playing recipes | Play Feel from Blueprint and C++, handles, stop | Same, plus Play Feel And Wait, parameters, accumulators, sustained recipes |
| Comfort | The full comfort layer: presets, saving, flash limiter, comfort menu | Same, plus the Comfort Audit |
| Feel Switch | Yes | Yes |
| Library | 11 recipes built from the Lite effects, with the 7 sample sounds they use | All 38, with the Recipe Browser and Recipe from Template |
| Events and triggers | No | Feel Maps and events, anim notifies, Feel Trigger component, Enhanced Input |
| Integrations | No | GAS add-on, multiplayer (Feel Replication) |
| Tools | No | Debugger, recent plays replay, GIF capture, sound waveforms and tracks from sound, JSON import and export |

Rules: Lite is a real, usable product with nothing crippled. Recipes made in Lite open in Pro unchanged. Pro-only code is
left out of the Lite download, and the two never clash when a project moves from Lite to Pro. Both are the plugin
FeelKit (FriendlyName "FeelKit Lite" for Lite), so a buyer installs one per engine (D-104). Lite is cut from the one
development source by `Tools/Run/make_edition.py` (D-100); a checked move from Lite to Pro keeps every recipe.

## 3. How it works

### 3.1 Recipes, tracks and steps
- A **recipe** (`UFeelRecipe`, a data asset) holds tracks, a cooldown, a limit on how many copies play at once, a default
  intensity, parameters, and library information (feeling, genres, description).
- A **track** (`FFeelTrack`) places one **step** on the timeline: start, length, channel, intensity curve, conditions
  (chance, distance, local player only), random ranges, and comfort options (essential, substitute step, floor).
- A **step** (`UFeelStep`) is one effect. Steps compute; they never touch the camera, screen or actors themselves.

### 3.2 Shared evaluation, separate output
Effects look identical in the game and in the editor preview because the same code computes both.
- `FFeelEvaluator` turns (recipe, time, comfort) into contributions: camera offset, rotation and field of view,
  post-process weights, actor transforms and material values, sounds.
- **Output sinks** apply them: in the game a camera modifier added automatically to each local player's camera manager,
  plus actor and audio delivery; in the editor the preview scene's camera, mesh and post process.
- **Pure evaluation:** a step's continuous output depends only on (time, alpha, intensity, seed), so scrubbing in any
  direction gives the same frame. Steps with side effects use start and stop calls; steps that cannot scrub say so.
- Timing uses real time, so a hitstop never slows FeelKit's own effects.

### 3.3 When effects overlap
- **Camera:** StrongestWins by default, or AdditiveCapped with caps (project setting).
- **Time:** one owner per scope (global or per actor), highest priority wins, the previous time is always restored.
- **Screen and haptics:** strongest wins per channel and per controller.
- Tracks inside one play add together; separate plays compete through these rules.

### 3.4 Intensity and comfort
- Final intensity = call intensity x curve x comfort group scale x Master.
- Comfort groups: Master, Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion, Haptics.
  Advanced: camera roll on or off, a zoom speed limit, the flash limiter.
- On Camera Shake and Camera Motion a player's 0 always means 0 (D-012).
- Essential tracks: a substitute step plays when the player's scale is 0 (for example a sound instead of a flash).
- Presets: Default, Reduced Motion, Reduced Flashing, No Haptics, plus custom preset assets.
- Settings are saved per local player (one save slot each) or through the game's own save system
  (`IFeelComfortStorage`).
- Comfort also scales the engine's own camera shakes and controller rumble, so existing effects respect it.
- The **comfort menu** (`WBP_FeelComfortMenu`, D-092 to D-095) is ready to use: Master and the six groups, presets,
  advanced options, Try, Reset, keyboard, mouse and gamepad. Buyers copy it into their project to restyle it.

### 3.5 Playing recipes
- Nodes: Play Feel, Stop Feel, Stop All Feel, Play Feel And Wait, Set Feel Parameter, accumulators, Send Feel Event.
- **Parameters** (such as Damage 0 to 100) map to track intensity through curves; the preview has sliders for them.
- **Accumulators** are named values that build up and decay (combo streaks, sustained fire).
- **Sustained recipes** hold a middle section until released (charging, sprinting, low health).
- **Feel Maps** turn an event tag plus context tags into a recipe; the most specific row wins.
- Triggers without code: anim notifies (Play Feel, Play Feel Window, Set Feel Value) and the Feel Trigger component
  (landed, jumped, air jumped, launched, damaged, hit, overlaps).
- Multiplayer: effects are cosmetic and local; the Feel Replication component sends plays to other machines
  (Multicast, Owner Only, Skip Owner, with relevancy). Hitstop in multiplayer slows only the target on that machine.

### 3.6 Modules

| Module | Type | Contents |
|---|---|---|
| FeelCore | Runtime | Everything that runs in a game. Depends only on stock engine modules |
| FeelEditor | Editor | Recipe editor, browser, debugger, validation, capture, thumbnails. Never referenced by runtime code |
| FeelNiagara | Runtime | Spawn Particle step |
| FeelEnhancedInput | Runtime | Input action triggers |
| FeelGAS (add-on) | Runtime | Gameplay Cue that plays a recipe. Ships in `Extras/FeelKitGAS` as its own plugin that buyers copy into their project, so GAS is only enabled for buyers who want it (D-076) |

## 4. What is built

Everything below is built and tested (117 automated tests, 7 editor window tests, strict Fab builds with 0 warnings on
UE 5.6, 5.7 and 5.8).

| Area | Features |
|---|---|
| Recipe editor | Timeline with tracks colored by channel, drag, resize, snap, zoom; inline intensity curves; mute, solo, copy and paste; undo; preview viewport with replaceable mesh; play, loop, scrub; comfort preset dropdown; parameter sliders; intensity graph; Play in PIE; edits apply to the next play; validation on save |
| Effects | 37 steps: camera (shake, punch, FOV kick, roll, zoom, look-at nudge), time (global and actor hitstop, slow-mo ramp), screen (flash, vignette, chromatic aberration, desaturate, tint, fade, post-process material pulse), actor (scale punch, squash and stretch, material pulse, hit flash, mesh wobble, light flash), audio (play sound, sound class duck, pitch bend, low-pass sweep), haptics (force feedback curve, haptic pattern), UI (widget punch, shake, flash, number pop), spawn (decal, particle), meta (play recipe, random choice, Blueprint event) |
| Inputs | Parameters, play context (instigator, target, location, direction, context tags), conditions, random ranges, sustained recipes, accumulators, Feel Maps and events, anim notifies, Feel Trigger, Enhanced Input, GAS add-on, multiplayer |
| Comfort | Everything in 3.4, the comfort menu, the Comfort Audit, flash limiter |
| Tools | Recipe Browser with hover preview and filters; Recipe from Template; read-only library recipes; Content Browser tiles drawn as mini timelines; FeelKit Debugger; `showdebug feel`; `stat Feel`; recent plays replay; GIF capture (Off and On); sound waveforms with snapping and tracks from sound; JSON import and export |
| Feel Switch | An actor that turns FeelKit off and on while playing (Tab or controller View), shows a start card, a FEEL ON/OFF badge and the level's controls, and opens the comfort menu (O or controller Menu) |

## 5. Library and sample content

38 library recipes, organized by feeling, with genre tags. Written as JSON (`Plugins/FeelKit/Library`, generated by
`Tools/Recipes/gen_library.py`) and imported by script. Library recipes are read-only; buyers copy them into their
project to change them.

| Feeling | Recipes (`FR_<Feeling>_...`) |
|---|---|
| Impact | LightHit, HeavyHit, CriticalHit, ScalableHit (Damage 0 to 100), BulletImpact |
| Weight | Land (FallSpeed), Stomp, HeavyFootstep, Slam |
| Power | ChargeUp (sustained), ChargedRelease, Explosion (Distance), AbilityCast |
| Speed | Dash, Boost (sustained), Whoosh, SprintStart |
| Reward | Pickup, ComboStep (Combo accumulator), LevelUp, KillConfirm |
| Danger | DamageTaken, DirectionalDamage, LowHealth (sustained, Health), Alarm |
| Dread | Heartbeat (sustained, Fear), Unease (sustained), JumpScare (essential flash with a substitute), FailingLight |
| Denial | Blocked, OutOfAmmo, Locked, WrongInput |
| Interface | ButtonHover, ButtonPress, Notification, ScoreTick, ScreenTransition |

Sample content in the plugin: CC0 sounds (Kenney) and three FeelKit materials (scorch decal, hit flash, post-process
pulse), every file listed with its source and license in `Plugins/FeelKit/Credits.md`. The comfort menu and its eight
preview recipes are in `/FeelKit/UI`.

## 6. Demos

Four demo levels, each on one of Unreal's own templates, each with a Feel Switch so players can compare the feel off and
on. They are our showcase and our acceptance tests.

| Demo | Level | Project | Hooked through |
|---|---|---|---|
| Action/RPG "Weight Class" | Lvl_Combat | GameFeelDev | Anim notifies on the combo and charged attack, the template's damage events, a guarding enemy, block, parry and guard break (D-069). Camera 3.5 m. Hit keeps the approved kick and rattle (D-072) |
| Platformer "Bounce Feel" | Lvl_Platforming | GameFeelDev | Feel Trigger on the character (jumped, air jumped, launched, landed), a marker on the dash. Landing view only goes down (D-067) |
| Shooter "Every Bullet Has an Opinion" | Lvl_Shooter | FeelDemoFP | Recipe slots added to the template's weapon, projectile, enemy and player code (marked `// FeelKit`), jump pads, HUD reactions |
| Horror "Heartbeat" | Lvl_Horror | FeelDemoFP | Sustained sprint and out-of-breath recipes, lamps and doorways with Feel Triggers, scare spots, sprint meter reactions |

- **Projects:** `GameFeelDev` (C++ Third Person template) holds the plugin and the first two levels; `FeelDemoFP`
  (C++ First Person template) holds the other two and loads the same plugin through a folder link.
- **Where the demo content lives:** the Action/RPG kit is in `/Game/FeelKitDemos/ActionRPG` of GameFeelDev because it
  depends on Epic's template content (D-089); the Platformer, Shooter and Horror recipes are in `/FeelKit/Demos`.
  Recipe sources are generated by `Tools/Recipes/gen_demo_*.py`; the levels are built by `Tools/Unreal/build_*_kit.py`.
- **For buyers:** a playable download per demo (the user publishes the links). Owners of FeelKit can ask for the UE 5.6
  source projects on Discord or by email (D-086). A script will make the handout copies (D-091).
- **Engine versions:** the demo levels use Epic's 5.6 template content, which crashes in 5.7 and 5.8 even without
  FeelKit, so the demo source is offered for 5.6 only.
- Demo sounds are CC0 or CC-BY with credits (D-017); no non-commercial or no-redistribution sources.

## 7. Engine versions and platforms

- UE 5.6 is the floor; everything is authored and saved in 5.6. Newer engine APIs only behind version checks.
- Built and tested on 5.6, 5.7 and 5.8 (Fab requires the newest version at the first submission).
- Offered for every platform Unreal supports; the listing and manual say plainly that only Windows was built and tested
  (D-073).
- PlayStation and other non-Xbox controllers rumble on Windows only through Steam Input (D-083).

---

## 8. Requirements

The original requirements (version 0.1, 2026-09-15). The IDs (C-001, ST-012, CMF-021 and so on) are used in the code,
the log and the manual plan. Where a decision changed a requirement, the table below says so; the rest still applies.

### 8.0 Changed since the requirements were written

| Requirement | Now |
|---|---|
| Product name (OD-001) | FeelKit, with the `Feel` prefix (D-014) |
| CMF-021 Essential floor | Does not apply to Camera Shake and Camera Motion: a player's 0 is always 0 (D-012) |
| C-001c "discard in version control" | There is no version control (D-001); content is saved in 5.6 only and backups are copies in `Backups/` |
| C-003 optional modules | Niagara and Enhanced Input modules are inside the plugin; GAS is a separate add-on folder (D-076) |
| CMF-050 sample widget | Built as the full comfort menu (D-085, D-092) |
| CT-002 demo map with 4 rooms | Four separate demo levels on Unreal's templates (D-052, D-064) |
| CT-005 sounds | CC0 preferred, CC-BY allowed with credits (D-017) |
| DOC-* documentation | Required at launch (D-077), one Word manual plus PDF (D-080) |
| Release tiers and price (section 13 of the original) | Replaced by section 2 of this document |
| Milestones and kill criteria (section 14 of the original) | History; phases 1 to 5 are complete, see the log |
| ST-004 to ST-006, ST-013 to ST-016, ST-020 to ST-025, ST-027, ST-029 to ST-035, ST-037 | Built in Phase 5 (some under different names: Sound Class Duck for Submix Duck, Hit Flash added, Post Process Material Pulse added, Radial Blur not built) |
| ST-028 Adaptive Trigger (DualSense), FeelHaptics module | Not built; possible later premium module (OD-005) |
| NF-005 Unreal Insights trace | Deferred |
| NF-002 performance number | Not measured yet (Launch document) |
| DEL-004 split-screen, TS-008 split-screen | Not tested yet (Launch document) |

### 8.1 Scope

**In scope:** runtime feedback engine (camera, time, screen, actor, audio, haptics, UI, spawn); recipe data assets and
timeline authoring editor; comfort layer with presets, substitution, flash limiter; engine-native trigger component;
sample recipes, demos, comfort menu; documentation, trailer material, free Lite version.

**Out of scope:** VFX or material content libraries; combat, damage, ability or GAS logic; a full settings menu
framework; camera rigs or camera modes; movement features (coyote time, input buffering); any claim of accessibility or
photosensitivity certification.

**Priority tags used below:** M = MVP, V1 = required for launch, P = premium or later, L = included in Lite (original
plan; the current Lite contents are in section 2).

### 8.2 Platform and Engine Constraints

| ID | Requirement | Priority |
|---|---|---|
| C-001 | Support UE 5.6, 5.7 and 5.8 at launch. Primary development and all content authoring happen in 5.6 (the oldest supported version) | V1 |
| C-001c | All plugin content (.uasset, .umap) is created and saved only in 5.6. Never resave plugin content in 5.7 or 5.8; do not keep such changes | M |
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

### 8.3 Core Concepts and Data Model

#### 8.3.1 Recipe (`UFeelRecipe : UPrimaryDataAsset`)

| Field | Type | Notes |
|---|---|---|
| Tracks | `TArray<FFeelTrack>` | Ordered by start time |
| Duration | float (derived) | Max of track end times |
| Cooldown | float | Seconds, per target |
| MaxConcurrent | int32 | 0 = unlimited |
| DefaultIntensity | float | Multiplier |
| Category | GameplayTag | For browser filtering (e.g. `FeelRecipe.Genre.Platformer`) |
| Description | FText | Shown in browser |

#### 8.3.2 Track (`FFeelTrack`)

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

#### 8.3.3 Step (`UFeelStep : UObject`, `EditInlineNew`, `Blueprintable`)

Lifecycle, all overridable in C++ and Blueprint:

| Function | Called |
|---|---|
| `OnStart(Context)` | Track begins |
| `OnUpdate(Context, Alpha, Intensity)` | Every frame while active |
| `OnStop(Context, bInterrupted)` | Track ends or is cancelled |
| `GetDefaultChannel()` | Used when a track is created in the editor |
| `SupportsPreview()` | Whether editor preview can simulate it |

`FFeelContext` provides: world, target, owning player controller, instance handle, recipe, real-time elapsed.

#### 8.3.4 Handle (`FFeelHandle`)
- Opaque ID, Blueprint-copyable, safe to hold after instance ends.
- `IsValid`, `IsPlaying` queries.

### 8.4 Functional Requirements: Runtime

#### 8.4.1 Subsystem

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

#### 8.4.2 Public API

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

#### 8.4.3 Channel arbiters

| ID | Requirement | Priority |
|---|---|---|
| ARB-001 | Camera arbiter: per-project mode `StrongestWins` (default) or `AdditiveCapped` with a max value | M |
| ARB-002 | Time arbiter: single active owner per mode (global / per-actor); highest priority wins; previous dilation always restored on stop, cancel, or target destruction | M |
| ARB-003 | Screen arbiter: one weighted post-process contribution per channel; weights combine by strongest-wins | M |
| ARB-004 | Haptics arbiter: strongest-wins per controller | V1 |
| ARB-005 | Arbiter decisions are exposed to the debugger (capped, suppressed, substituted) | V1 |

#### 8.4.4 Camera and screen delivery

| ID | Requirement | Priority |
|---|---|---|
| DEL-001 | Camera and screen effects are delivered through a `UCameraModifier` added automatically to the local player's camera manager; buyer adds nothing | M |
| DEL-002 | Post-process effects applied through the modifier's post-process hook; no volume placement required | M |
| DEL-003 | Works with any camera setup (first person, third person, spring arm, cine camera) that uses the standard camera manager | M |
| DEL-004 | Split-screen: effects apply only to the intended local player | V1 |

#### 8.4.5 Step catalog

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

#### 8.4.6 Triggers

| ID | Requirement | Priority |
|---|---|---|
| TRG-001 | `UFeelTriggerComponent` maps events to recipes without Blueprint code | V1 |
| TRG-002 | Supported events: OnTakeAnyDamage, OnLanded (only if owner is ACharacter), Component Hit, Begin/End Overlap | V1 |
| TRG-003 | Enhanced Input action trigger in optional module | V1 |
| TRG-004 | Damage trigger can scale intensity by damage amount through a curve | V1 |

#### 8.4.7 Networking

| ID | Requirement | Priority |
|---|---|---|
| NET-001 | Recipes are cosmetic and run locally; no gameplay state is replicated | M |
| NET-002 | `UFeelReplicationComponent` offers `PlayFeelNetworked` with modes Multicast, OwnerOnly, SkipOwner | V1 |
| NET-003 | Networked play supports a relevancy distance filter | V1 |
| NET-004 | Dedicated server never executes steps (no-op) | M |
| NET-005 | Global hitstop in multiplayer is local-only by default and documented as such | V1 |

### 8.5 Functional Requirements: Comfort Layer

#### 8.5.1 Settings

| ID | Requirement | Priority |
|---|---|---|
| CMF-001 | Per local player channel scales (0 to 1): Master, Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion, Haptics | M, L |
| CMF-002 | Final track intensity = CallIntensity x Curve(t) x ChannelScale x MasterScale | M, L |
| CMF-003 | Presets: Default, Reduced Motion, Reduced Flashing, No Haptics | M, L |
| CMF-004 | Developers can define custom presets as data assets | V1 |
| CMF-005 | Project-level default scales in Project Settings (`UFeelSettings : UDeveloperSettings`) | M, L |
| CMF-006 | Channel mapping is data driven: new channels can be added via GameplayTags and assigned to a comfort group | V1 |

#### 8.5.2 Persistence

| ID | Requirement | Priority |
|---|---|---|
| CMF-010 | Default persistence to a per-player config or local player save file | M, L |
| CMF-011 | `IFeelComfortStorage` interface lets buyers route load/save to their own save system | V1 |
| CMF-012 | Settings load before first recipe plays in a session | M, L |

#### 8.5.3 Essential tracks and substitution

| ID | Requirement | Priority |
|---|---|---|
| CMF-020 | When a channel scale is 0 and the track is Essential with a SubstituteStep, the substitute plays instead | V1 |
| CMF-021 | When Essential without substitute, intensity is clamped to EssentialFloor | V1 |
| CMF-022 | Non-essential tracks at scale 0 are skipped entirely (zero cost) | M, L |
| CMF-023 | Validation warns on Essential tracks with no substitute and floor 0 | V1 |

#### 8.5.4 Flash limiter

| ID | Requirement | Priority |
|---|---|---|
| CMF-030 | Counts flash-channel events per rolling second per player | V1 |
| CMF-031 | Above a configurable rate (default 3 per second) additional flashes are softened or suppressed (configurable) | V1 |
| CMF-032 | Reduced Flashing preset enables the limiter with stricter defaults | V1 |
| CMF-033 | Documentation states it is a mitigation helper, not a compliance tool, and references external analysis tools | V1 |

#### 8.5.5 Motion comfort

| ID | Requirement | Priority |
|---|---|---|
| CMF-040 | Reduced Motion caps FOV change rate (degrees per second, configurable) | V1 |
| CMF-041 | Reduced Motion disables camera roll | V1 |
| CMF-042 | Reduced Motion converts directional shake to low-amplitude shake | P |

#### 8.5.6 Sample UI

| ID | Requirement | Priority |
|---|---|---|
| CMF-050 | Sample UMG widget with all sliders, preset dropdown, and live preview button | V1, L (basic) |
| CMF-051 | Widget is gamepad navigable | V1 |
| CMF-052 | Widget uses no third-party UI framework | M |

### 8.6 Functional Requirements: Editor Tooling

#### 8.6.1 Recipe editor

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

#### 8.6.2 Preview

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

#### 8.6.3 Browser, debugger, validation

| ID | Requirement | Priority |
|---|---|---|
| TL-001 | Recipe browser tab with genre and channel filters | V1 |
| TL-002 | Live debugger panel in PIE: active instances, time remaining, arbiter decisions, comfort scales | V1 |
| TL-003 | Gameplay Debugger category with the same data in development builds | V1 |
| TL-004 | Data validation on save: invalid curves, missing assets, zero-length tracks, essential tracks without fallback | V1 |
| TL-005 | Asset thumbnails for recipes | P |
| TL-006 | Content Browser asset action: "Create Recipe" from template list | V1 |

### 8.7 Non-functional Requirements

#### 8.7.1 Performance

| ID | Requirement | Priority |
|---|---|---|
| NF-001 | Zero game thread cost when no instances are active (no tick registered) | M |
| NF-002 | 50 concurrent instances under 0.2 ms game thread on a reference mid-range PC (measure and publish) | V1 |
| NF-003 | No per-frame heap allocation during steady-state playback | V1 |
| NF-004 | `stat Feel` stat group with instance count and tick time | V1 |
| NF-005 | Unreal Insights trace channel for play/stop events | P |

#### 8.7.2 Reliability

| ID | Requirement | Priority |
|---|---|---|
| NF-010 | No crash when target, controller, or world is destroyed mid-playback | M |
| NF-011 | Time dilation always restored, including on PIE stop and level travel | M |
| NF-012 | Null or missing assets in a recipe log a warning and skip the track | M |

#### 8.7.3 Usability

| ID | Requirement | Priority |
|---|---|---|
| NF-020 | First working result in 5 minutes following the Quick Start | V1 |
| NF-021 | Works without modifying buyer GameMode, PlayerController, Character, or GameInstance | M |
| NF-022 | Every Blueprint node and property has a tooltip | V1 |

#### 8.7.4 Code quality

| ID | Requirement | Priority |
|---|---|---|
| NF-030 | Passes Fab code plugin technical requirements | V1 |
| NF-031 | Compiles without warnings on all supported engine versions | V1 |
| NF-032 | Engine version differences isolated behind version macros | V1 |
| NF-033 | Public headers documented; source follows Epic coding standard | V1 |

### 8.8 Module Layout

| Module | Type | Contents | Tier |
|---|---|---|---|
| FeelCore | Runtime | Subsystem, recipe, track, steps, arbiters, comfort, settings, API | Lite subset + Full |
| FeelEditor | Editor | Asset editor, timeline, preview, browser, validation, debugger UI | Full |
| FeelHaptics | Runtime | Advanced haptics, DualSense triggers | Full (P) |
| FeelNiagara | Runtime (optional) | Spawn Particle step | Full |
| FeelEnhancedInput | Runtime (optional) | Input action trigger | Full |
| Content | Assets | Recipes, demo map, sample widget, sample materials | Full; Lite gets a small subset |

### 8.9 Content Deliverables

| ID | Deliverable | Priority |
|---|---|---|
| CT-001 | 10 recipes (MVP), 30 to 40 recipes (V1) across Platformer, Shooter, Action/RPG, Horror, Vehicle, UI | M / V1 |
| CT-002 | Demo map with 4 rooms (Platformer, Shooter, Action, Horror), each with Feel On/Off toggle | V1 |
| CT-003 | In-world comfort panel in demo map | V1 |
| CT-004 | Minimal sample materials for screen and actor steps | M |
| CT-005 | Sample sounds (licensed for redistribution: CC0 preferred, CC-BY with credits; license documented, see D-017) | V1 |
| CT-006 | Playable packaged demo (Windows) | V1 |

### 8.10 Documentation

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

### 8.11 Testing

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

### 8.12 Open decisions (all resolved)

| ID | Question | Answer |
|---|---|---|
| OD-001 | Product name | FeelKit, `Feel` prefix (D-014) |
| OD-002 | Engine versions | 5.6, 5.7, 5.8 (C-001) |
| OD-003 | Comfort persistence default | One SaveGame slot per local player (D-005) |
| OD-004 | Default camera arbitration | StrongestWins (D-004) |
| OD-005 | DualSense support at launch | Not at launch; possible premium module later |
| OD-006 | Lite as a separate listing | Yes, two Fab listings (D-081) |
| OD-007 | Hardware for performance numbers | Open, with the performance measurement (Launch document) |
