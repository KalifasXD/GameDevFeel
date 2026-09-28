# CLAUDE.md: FeelKit

## Project

FeelKit is an Unreal Engine code plugin for Fab: a game feel framework (Recipes of timed feedback Tracks) with a player Comfort layer and a timeline authoring editor.

## Project layout and documents (user rule, 2026-09-24)

The user could not find anything in a folder of 30 documents and 20 build folders. Keep it small:

```
B:/NewUE5Project/
  GameFeelDev/   main project, holds the plugin (Plugins/FeelKit)
  FeelDemoFP/    Shooter and Horror demo project (plugin linked)
  Docs/          the five documents, Manual/ (src, out, Screenshots), Archive/
  Tools/         Run/ (PowerShell runners), Unreal/ (editor Python), Recipes/ (JSON generators), Manual/ (manual build)
  Build/         Logs/ and the current packages only; everything else is temporary and deleted after use
  Backups/       copies taken before changes
  Publish/       the Fab package the user asked for (2026-09-25): PUBLISH-README.md, Lite/ and Pro/ (01-Images,
                 02-Fab-Listing.md, 03-Upload/UE5.x zips). Rebuilt by script, never edited by hand except the listings
```

**The five documents** (`Docs/`), and nothing else:
1. `FeelKit_1_Product.md`: what FeelKit is, editions, design, what is built, library, demos, requirements (section 8, with the requirement IDs).
2. `FeelKit_2_Launch.md`: work left before going live (developer and user), listing, Fab rules, price, engine versions, demo handout, marketing brief. Checklists for the user go here (or in the chat), not in new files.
3. `FeelKit_3_Manual_Plan.md`: the buyer manual plan with the outline, screenshot and video tables the build reads.
4. `FeelKit_4_Log.md`: the log. **Read it first when resuming.** Record every change, decision, test result, issue, assumption and research finding as it happens, with dates and requirement IDs.
5. `FeelKit_5_Research.md`: research reports (add new research as a new part).

**Never create another document.** Add to one of the five. `Docs/Archive/` holds the replaced documents: read-only history. Screenshots for the manual go in `Docs/Manual/Screenshots/<ShotID>.png`; tell the user to put theirs there. Temporary project copies made in `Build/` are deleted when the check is done. The folder is synced by Synology Drive: ask the user to pause it before moving or deleting many files (a moved folder came back on 2026-09-24).

## Hard rules

1. **Engine floor is UE 5.6.** Do not use APIs introduced after 5.6 unless wrapped in `#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7`.
2. **Create or change `.uasset` / `.umap` files only through Unreal's own editor scripting** (Python or editor commandlets run by the engine), never by writing the files directly, and **copy every existing file to `B:/NewUE5Project/Backups/<date-time>/` before touching it** (no git). The editor must be closed while scripts run. The user plays, judges the feel and picks sounds by ear (D-056, 2026-09-18; before that the user created every asset from step lists).
3. **FeelCore depends only on stock modules:** Core, CoreUObject, Engine, InputCore, Slate, SlateCore, UMG, GameplayTags, DeveloperSettings. No GAS, no Enhanced Input, no Niagara in FeelCore.
4. **FeelEditor is `Type: Editor`** and must never be referenced by runtime modules.
5. **No engine source changes.**
6. **Stay inside the current phase.** Do not implement features from later phases, even if they seem easy.
7. **Zero compiler warnings.**
8. **All public types use the `Feel` prefix** (`UFeelRecipe`, `FFeelTrack`, `UFeelStep`, `UFeelSubsystem`).
9. **No AI trace anywhere** (user rule, 2026-09-22, non-negotiable). Nothing in the project may read or look as if it was generated: code, comments, names, documentation, UI text, recipe descriptions, credits, marketing copy, screenshots, icons, metadata. Plain, specific wording; no stock phrases (seamless, robust, leverage, comprehensive, delve, "whether you're"), no em dashes, no emoji, no tutorial-style comments on obvious code. Sweep the shipped plugin and anything user-facing before calling a piece of work finished.
10. **American spelling in everything buyers read** (D-097, 2026-09-25): plugin UI and tooltips, recipe descriptions, credits, the manual, the listing. Unreal names shown in the editor keep their own spelling (for example the pin On Cancelled).

## Key design decision: shared evaluation, separate output

Effects must look identical in-game and in the editor preview. Do not write preview-only effect code.

- **Steps compute, sinks apply.** A `UFeelStep` computes its contribution for a given time and intensity. It does not touch the camera, post process, or actors directly.
- **`IFeelOutputSink`** receives contributions: camera offset/rotation/FOV, post-process weights and parameters, actor transform and material parameters, sound playback.
- **Runtime sink:** a `UCameraModifier` added automatically to the local player's camera manager, plus actor and audio application (DEL-001, DEL-002).
- **Preview sink:** applies the same contributions to the editor preview scene camera, preview mesh and preview post process.
- **`FFeelEvaluator`** is shared: given a recipe, a time and comfort scales, it produces contributions. Both the runtime subsystem and the editor preview use it.
- Instance timing uses **real (unscaled) time** (RT-003).
- **Pure evaluation (decided 2026-09-15).** A step's continuous output is a pure function of (local time, normalized alpha, intensity, seed): `Evaluate(...) const`. No state accumulated between frames, so scrubbing in any direction gives identical frames. `OnStart`/`OnStop` exist only for side effects. Steps that truly need state return `false` from `SupportsScrub()`. Evaluator tests include a determinism test.

## Completed: Phase 1

Recipe data, shared evaluator, Procedural Shake / Screen Flash / Scale Punch, recipe editor with timeline and preview. All done criteria met; K-002 passed on 2026-09-15 (user: timeline clearly better than typing numbers and replaying PIE).

## Product decisions (2026-09-16, user-approved)

Source: the accepted Strategy v2 (`Docs/Archive/FeelKit_Strategy_v2_Proposal.md`) and log decisions D-012 to D-019; current summary in `Docs/FeelKit_1_Product.md`.
- **Position:** FeelKit is the response layer of an Unreal game: gameplay says what happened (tag plus context), FeelKit decides how it feels, comfort-safe, previewable and scrubbable.
- **Roadmap:** 5B Inputs → 5C Senses → 5D Proof → 5E Reach → Phase 6 Content and launch.
- **Name:** keep FeelKit and the `Feel` prefix (OD-001 closed). "hitstop", "screen shake", "game feel" in the Fab title and tags; **no "juice" in the title or the tags** (D-082).
- **Editions:** free **Lite** that includes the full comfort layer, and **Pro** with everything. No studio tier. Two Fab listings; Lite is a usable taste with nothing crippled (D-081, split being agreed).
- **Comfort rule:** on Camera Shake and Camera Motion a player's setting always wins; the essential floor never applies there (changes CMF-021).
- **Demos:** genre showcase demos (five planned, four since D-064 moved UI feel into the Shooter and Horror HUDs); only **one** documented how-I-made-it tutorial (video or doc with screenshots), for education. Action/RPG sword hit is the 5B acceptance scenario.
- **Demo audio:** free sounds are fine when their license allows redistribution inside a product (CC0 preferred; CC-BY with credits). No NC/ND licenses, no "free but no redistribution" libraries.

## Current phase: Phase 6 (Content and launch), scope being agreed

Phase 5 finished on 2026-09-17 (record below). Phase 6 per the Strategy v2 roadmap: demos (`Docs/FeelKit_1_Product.md` section 6), recipe library, TL-001 browser, TL-005 thumbnails, TL-006 templates, docs, Lite/Pro packaging, trailer. Split into parts (D-038). **Part A, content tools, recipe library and editor look: complete on 2026-09-18** (user: "Everything is perfect"). **Current: part B, demos** (D-052 kits for Unreal templates, D-053 all demos before release, four since D-064, D-054 Action/RPG first, D-055 videos in 6D). Action/RPG kit approved by the user on 2026-09-19 ("crazy good"); its ragdoll PIE warnings fixed (I-037); design `Docs/FeelKit_1_Product.md` section 6 (details in `Docs/Archive/FeelKit_Phase6B_Demos_Design.md`). Feel Switch (D-057, feel off/on while playing, Tab / controller View) built 2026-09-22 and placed in the Action/RPG level; every demo gets one. Platformer demo "Bounce Feel" built end to end 2026-09-22 (Feel Trigger Jumped / Air Jumped / Launched, D-058); Feel Switch and Platformer approved 2026-09-22. Shooter demo (second host project `B:/NewUE5Project/FeelDemoFP`, C++ First Person template, D-059) approved 2026-09-22, jump pads added; Horror demo built 2026-09-22 (D-060), reworked and approved 2026-09-23. The UI menu demo (D-062) was dropped on 2026-09-23 (D-064, changes D-053: four demo levels): UI feel lives in the Shooter HUD and the Horror sprint meter (template UI classes hooked, `// FeelKit`); the menu is removed (copy in `Backups/2026-09-23_170544_ui_demo_removed`). No "juice" wording in demo titles (competitor Game Juice Pro). Action/RPG additions 2026-09-23: camera 3.5 m by measurement (`DiagFeel.ARPGCameraStudy`), softer hit turn (D-068, fast camera turns smear under motion blur), guards (D-069: `CombatGuardComponent` energy shield, guard enemy `CombatGuardEnemy`, block Left Shift / left trigger, parry, guard break). Platformer landing: the view only goes down (D-067). All four demo levels approved 2026-09-23; the Action/RPG hit keeps the camera kick and rattle approved on 2026-09-19 (D-072), with no full-screen flash on heavy strikes. Also 2026-09-23 night: the Action/RPG punch was retuned by measured on-screen knock (D-070: 8.0% of the screen width against the first version's 8.2%, one-way Kick, no rebound) and every level lists its controls under the Feel Switch badge (D-071, Feel Switch `Controls`). Launch work and status: `Docs/FeelKit_2_Launch.md`. D-026 closed (GIF only); D-027 closed by D-076: GAS support is an add-on folder inside FeelKit (Extras/FeelKitGAS) that buyers copy into their project; proven with a real package (gas_proof.ps1). Launch metadata: version 1.0.0, publisher Billo, every platform with Windows the only tested one (D-073), no tests in the shipped package (D-075). Camera motion rule from I-038 and D-067: movement cameras (not hits, D-072) use Kick or Smooth shapes, not fast springs or shakes that turn the view both ways. 116/116 tests, 6/6 UI tests. Documentation is required at launch (Fab 4.3.8, D-077); the developer writes it as a Word manual plus PDF (D-080), hosted on Google Drive. Editor look decisions (D-049 editor look, D-050 counterparts: recipe editor = Sequencer, Recipe Browser = Content Browser, tiles = Unreal data thumbnails, Debugger = Outliner). Before any editor-facing handover, render the window with the DiagFeel picture diagnostics (RecipeEditorShot, BrowserShot, DebuggerShot, ThumbnailSheet, WindowShot) and compare it with what was agreed.

- Built: library metadata on recipes (feeling, genres, description, based on) with asset registry tags, recipe browser tab with live hover preview and filters, Recipe from Template, read-only library recipes with the Allow Library Editing preference, Content Browser recipe tiles and tooltips, JSON import that tolerates missing asset references, and 38 library recipes as JSON under `Plugins/FeelKit/Library` with `Credits.md` (CC0 Kenney sounds).
- Controller rumble (I-029) root cause found and fixed 2026-09-17: comfort was zeroing the player controller's global force feedback scale (D-046); it now multiplies and restores the game's value, and FeelKit says when vibration is silenced.
- Verified (2026-09-18): **107/107 tests**, 5/5 UI tests in a rendering session, editor + Game Development + Game Shipping builds, strict BuildPlugin, all 0 warnings.
- Design and plan in `Docs/Archive/` (FeelKit_Phase6A_*), decisions D-038 to D-045.
- Library recipes are written as JSON by the developer and imported through editor scripting (hard rule 2, D-056). Later parts: B demos, C documentation, D launch readiness. I-029 (PlayStation rumble) closed 2026-09-24 as a platform note: Steam Input (D-083).

## Completed: Phase 5 remainder (5B, 5C, 5D, 5E in one go)

User instruction (2026-09-16): implement the rest of Phase 5 in one go, then one full user test pass (project log D-024, A-006). Plan and per-chunk records: `Docs/FeelKit_4_Log.md` section 1a.

### Status (2026-09-17)
- **Built:** chunk 1 Inputs (sustain, live parameters, accumulators, Feel Map + events, anim notifies, trigger component, Play Feel And Wait, delegates, `feel.GlobalScale`, `stat Feel`); chunk 2 Senses (camera/screen/actor/audio/widget/spawn steps, Play Recipe, Random Choice, FeelNiagara); chunk 3 Proof (flash limiter, motion comfort, engine shake and force feedback comfort, moment capture + Recent Plays replay, track decision labels, FeelKit Debugger tab, `showdebug feel`, Comfort Audit, Capture GIF, waveform + onset snapping + tracks from sound); chunk 4 Reach (Feel Replication component, NET-005 hitstop policy, FeelGAS, FeelEnhancedInput, recipe JSON import/export).
- **User test pass 1 (2026-09-17):** most checks passed; reported problems fixed as I-009 to I-020 (Enter/Tab focus, network routing, decal, post-process weight, GIF palette, cue waveforms, debugger, captures, name pickers). **96/96 tests** plus the UI test `FeelKit.Editor.DetailsKeepFocusAfterEdit` (needs a rendering session, skips under -nullrhi). Network modes verified with two processes via the host project harness `feeltest.net`.
- **User retests 2-3 (2026-09-17 night):** B6 passed; packaging fix I-027 (host harness editor-only); D1 root cause I-026 fixed (D-037: actor motion on a character moves its mesh, never the collision capsule), **97/97 tests**, network modes verified in PIE with the real K key; rumble chain verified up to Windows XInput, device layer checked with the host probe `feeltest.rumble` (I-024). Verification before handoff includes the Game target (Development and Shipping).
- **User retest 4 (2026-09-17 late night):** D1, D2, D4, D5 passed. Open: D3 (checklist lacked Make GameplayCueParameters, I-031), D2 optional global time in multiplayer (measured working with `DiagFeel.PIETimeDilation`, waiting for the user's description, I-030). PlayStation controllers rumble on Windows only through Steam Input; closed as a platform note (D-083, 2026-09-24).
- **Strict BuildPlugin:** package to a short path (`B:/NewUE5Project/Build/FKPkg4`, last run 2026-09-17 evening successful, 0 warnings); the session scratch folder exceeds the 260-character path limit (I-008).
- **Complete (2026-09-17):** user confirmed every check (D3 and the D2 optional check last). Never hand over checks before the fixes are built and verified.
- **Closed 2026-09-23:** D-027 (GAS only for buyers who want it: add-on folder, D-076) and D-026 (GIF only).
- **UE 5.7.4 and 5.8.3 installed (2026-09-24)** at `D:/EpicGames/UE_5.7` and `D:/EpicGames/UE_5.8`: FeelKit and the GAS add-on are a go on both (strict packages 0 warnings, 116/116 tests, 6/6 UI tests; `Docs/FeelKit_2_Launch.md` section 5). Demo levels must be rebuilt on each engine's own template (Epic's 5.6 template levels crash in 5.7/5.8 PIE without FeelKit). Checks run on copies via `verify_engine.ps1`; the 5.6 projects stay on 5.6. Rewind Debugger track (NF-005) deferred.

### Decisions made during the one-go implementation (details in the project log)
- D-022 surface as a context tag (no PhysicsCore). D-025 comfort audit thresholds (3 flashes/s; saturated red >= 80% red share above opacity 0.5), always worded as a readiness helper. D-028 networked worlds localize global time dilation to the play target.


## Completed: Phase 5A (editor power features)

Status: complete on 2026-09-16. User confirmed all 8 checks (check 6 after fixing I-005, stale Asset Check messages reopening the Message Log). Editor build 0 warnings, 50/50 tests, strict BuildPlugin successful. The record below stays for reference; its decisions still apply.

Goal: widen the lead of the authoring workflow, the product's main differentiator. Phase 5 is split into parts; the user chose editor power features first (2026-09-15).

### In scope
- ED-007 inline intensity curve editing for the selected track, directly on its timeline row: add, move and delete keys, all undoable
- ED-009 copy and paste tracks between recipes: the selected track or all tracks; paste lands at the playhead and keeps relative timing
- PV-005 Play in PIE button: plays the recipe on the running session's local player pawn
- PV-006 comfort preset dropdown in the preview (Neutral, project defaults, built-in presets); the preview reflects it live
- PV-007 intensity graph: combined intensity per channel across the recipe
- PV-008 replaceable preview mesh (static or skeletal), saved per recipe as editor-only data
- PV-009 recipe edits during PIE apply to the next play without restarting (verify, fix if needed)
- TL-004 data validation on save: invalid curves, missing assets, zero-length tracks whose step needs a length, essential tracks without a fallback (also covers CMF-023)
- Automation tests for validation rules, track clipboard, per-channel intensity sampling and preview comfort

### Out of scope for Phase 5A
The rest of Phase 5 (V1 steps, triggers, networking, debugger, flash limiter, motion comfort), recipe browser (TL-001), thumbnails (TL-005), templates (TL-006).

### Decisions (2026-09-15)
- **Preview mesh** is stored per recipe as editor-only data (a sword recipe previews on a sword) and is stripped from cooked builds.
- **Track clipboard** is in-process: copied tracks and their steps are duplicated, so paste works across recipe editors in the same editor session.
- **Validation** uses `IsDataValid` on recipes and steps, so it runs through Unreal's standard Data Validation (on save and on demand).

### Done criteria (all must pass)
1. Plugin builds for Win64 with zero warnings on UE 5.6 (editor target and `BuildPlugin -StrictIncludes`), and test logs show no LogPython or FeelKit warnings.
2. Automation tests in `FeelKit` pass, including `FeelKit.Runtime.ForceFeedbackDelivery` from Phase 4 and new tests for validation, clipboard, intensity sampling and preview comfort.
3. The user confirms each feature in the editor, using a checklist that states every setup step.

Criterion 3 can only be checked by the user. When 1 and 2 pass, stop and ask the user to verify 3.

## Completed: Phase 4

Goal: every remaining MVP step, each clearly better than what competitors ship.

Status: complete on 2026-09-15. User confirmed every editor preview item and every PIE check (force feedback confirmed with Steam Input, see Platform notes). Strict BuildPlugin 0 warnings, 43/43 tests. `FeelKit.Runtime.ForceFeedbackDelivery` was written afterwards; it runs with the first Phase 5A build. The Phase 4 record below stays for reference; its decisions still apply.

The user chose to skip the validation posts and assume demand, so kill check K-003 is waived (2026-09-15). The user's bar: go above and beyond, no minimal implementations.

### In scope
- ST-002 Camera Punch and ST-003 FOV Kick: shaped motion (spring with frequency and damping, kick, smooth), seeded direction variation
- ST-018 Squash and Stretch: volume preserving, shaped motion
- ST-009 Slow-mo Ramp: eased ramp in and out through the time arbiter
- ST-011 Vignette Pulse and ST-012 Chromatic Aberration: post-process contributions, strongest wins per parameter (ARB-003); one shared builder feeds the runtime camera manager blends and the preview's view override
- ST-019 Material Parameter Pulse: scalar and color parameters through dynamic material parameters or custom primitive data, original values restored
- ST-022 Play Sound: 2D, attached or at location, seeded volume and pitch variation, optional fade out at track end; audible in the editor preview (PV-003)
- ST-026 Force Feedback Curve: per-motor strengths shaped by the track curve, strongest wins per player
- ST-036 Blueprint Event Step: calls a named custom event on the target at track start and end, passing intensity when the event takes a float
- One track lifecycle (OnStart / OnStop) shared by the runtime and the editor preview
- Automation tests for every new step and delivery path

### Out of scope for Phase 4
Validation posts, non-MVP steps, triggers, networking, sample widget, flash limiter, recipe content (CT-001), later editor features (ED-007 inline curve, PV-005 PIE button, PV-006 comfort dropdown).

### Decisions (2026-09-15)
- **K-003 waived** by the user.
- **Side effects never happen in Evaluate.** Per-instance side-effect bookkeeping (such as the audio component to fade out) is keyed by instance and track.
- **Steps that shape their own envelope** (motion shapes, hitstops, slow-mo) start new tracks with a flat intensity curve.
- **Actor delivery** (scale, material parameters) and **post-process building** are shared helpers used by both runtime and preview.
- **Comfort:** sounds have no comfort group (Master only); force feedback uses the Haptics group; slow-mo uses Hitstop and Slow-mo; vignette and chromatic aberration use Screen Distortion.

### Done criteria (all must pass)
1. Plugin builds for Win64 with zero warnings on UE 5.6 (editor target and `BuildPlugin -StrictIncludes`), and test logs show no LogPython or FeelKit warnings.
2. Automation tests in `FeelKit` pass, including new tests for every Phase 4 step and for post-process, material, haptics and lifecycle delivery.
3. All earlier tests still pass.
4. The user confirms:
   - Editor preview: Camera Punch, FOV Kick, Squash and Stretch, Vignette Pulse, Chromatic Aberration and Material Parameter Pulse are visible; Play Sound is audible; Slow-mo Ramp, Force Feedback Curve and Blueprint Event show the no-preview hatch
   - PIE: every Phase 4 step works on the Third Person character, including force feedback on a gamepad and a Blueprint event firing

Criterion 4 can only be checked by the user. When 1 to 3 pass, stop and ask the user to verify 4.

## Completed: Phase 3

Goal: every effect respects each player's comfort settings, and those settings persist.

Status: all done criteria met on 2026-09-15. Strict BuildPlugin 0 warnings, 28/28 tests, user confirmed all four PIE checks. The Phase 3 record below stays for reference; its decisions still apply.

### In scope
- Comfort scales per local player (0 to 1): Master, Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion, Haptics (CMF-001)
- Final intensity = Call x Curve x Channel scale x Master (CMF-002), applied inside `FFeelEvaluator` so runtime and preview share it
- Built-in presets Default, Reduced Motion, Reduced Flashing, No Haptics with values editable in project settings (CMF-003); custom presets as `UFeelComfortPreset` data assets (CMF-004)
- Project default scales in `UFeelSettings` (CMF-005); channel tag to comfort group mapping in `UFeelSettings`, most specific tag wins (CMF-006)
- `UFeelComfortSubsystem` (`ULocalPlayerSubsystem`): loads settings when the local player is created, before any recipe plays (CMF-012); Blueprint API to read, set, apply presets and save
- Default persistence: one SaveGame slot per local player (CMF-010); `IFeelComfortStorage` routes load and save to the buyer's own system (CMF-011)
- Essential tracks: the substitute step plays when the channel scale is 0 (CMF-020); otherwise the comfort scale never drops below EssentialFloor (CMF-021); non-essential tracks at scale 0 are skipped without calling the step (CMF-022)
- Automation tests for comfort math, mapping, presets, persistence, substitution and floor (TS-001, TS-003)

### Out of scope for Phase 3
Sample comfort widget (CMF-050 to CMF-052), flash limiter (CMF-030 to CMF-033), motion comfort rules (CMF-040 to CMF-042), essential-track validation warnings (CMF-023), comfort preset dropdown in the editor preview (PV-006), haptics steps, networking, all other steps.

### Decisions (2026-09-15)
- **OD-003 resolved:** default comfort persistence is one SaveGame slot per local player.
- **Substitutes** play at Call x Curve x Master x the comfort scale of the substitute step's own default channel.
- **EssentialFloor** is the minimum comfort scale (Master included) for an essential track without a substitute.
- **Whose comfort:** an instance uses the comfort of the local player it resolves to; with no local player, the project defaults apply.
- **Editor preview** evaluates with neutral comfort (all 1) until PV-006.

### Done criteria (all must pass)
1. Plugin builds for Win64 with zero warnings on UE 5.6 (editor target and `BuildPlugin -StrictIncludes`).
2. Automation tests in `FeelKit` pass, including new tests for: Call x Curve x Channel x Master, non-essential tracks skipped at scale 0, EssentialFloor, substitution, most specific channel mapping, presets, save/load round trip, custom storage routing.
3. All Phase 1 and Phase 2 tests still pass.
4. The user confirms in PIE, without changing any game class:
   - Setting Camera Shake to 0 from Blueprint removes a recipe's shake while its flash still plays
   - Applying the Reduced Motion preset visibly reduces shake
   - An essential flash with a substitute plays the substitute when Flashes is 0
   - Comfort settings survive stopping and restarting PIE

Criterion 4 can only be checked by the user. When 1 to 3 pass, stop and ask the user to verify 4.

## Completed: Phase 2

Runtime subsystem, PlayFeel API, camera / time / screen arbiters, automatic camera modifier, flash scene view extension, Global and Actor Hitstop. All done criteria met on 2026-09-15 (strict BuildPlugin 0 warnings, 20 tests, user PIE checks passed). The Phase 2 record below stays for reference; its decisions still apply.

### Phase 2 scope (done)
- `UFeelSubsystem` (`UTickableWorldSubsystem`): owns instances, ticks only while instances are active, real (unscaled) time, Cooldown and MaxConcurrent per recipe and target, safe stop when the target is destroyed or the world tears down, `feel.Enabled` console variable (RT-001 to RT-003, RT-005 to RT-007)
- Blueprint and C++ API in category `Feel`, tooltips on every pin: `PlayFeel`, `StopFeel`, `StopAllFeel`, `FFeelHandle`, `FFeelTarget` (API-001 to API-003, API-005, API-008)
- Arbiters: camera (StrongestWins default, AdditiveCapped with caps), time (one owner per global / per-actor scope, highest priority wins, previous dilation always restored), screen (strongest-wins per channel) (ARB-001 to ARB-003)
- Delivery: `UCameraModifier` added automatically to each local player's camera manager; screen flash through a plugin-registered scene view extension; scale punch applied to the target's scene component with its base scale restored (DEL-001 to DEL-003)
- Brought forward: Global Hitstop (ST-007) and Actor Hitstop (ST-008), needed for the exit gate
- `UFeelSettings` project settings with only what Phase 2 needs: camera arbitration mode and caps, blend-out time
- Automation tests for arbiter rules (TS-002) and cooldown / max concurrent (TS-004)

### Out of scope for Phase 2
Comfort layer and comfort settings, Play Feel And Wait, anim notify, native delegates, instance pooling (RT-004), `feel.GlobalScale`, track conditions evaluation, networking (including the dedicated-server no-op), triggers, split-screen, all other steps, editor PIE button.

### Decisions (2026-09-15)
- **Screen flash at runtime:** a scene view extension sets `FSceneView::OverlayColor` using the same `FFeelFrameOutput` helper as the preview, layered on top of (not replacing) the game's camera fade. This intentionally differs from DEL-002's wording; the camera modifier's post-process hook stays reserved for later post-process steps.
- **OD-004 resolved:** default camera arbitration is StrongestWins.
- **Combination order:** tracks inside one instance add together (the recipe author's intent); separate instances compete through the arbiters.

### Done criteria (all must pass)
1. Plugin builds for Win64 with zero warnings on UE 5.6 (editor target and BuildPlugin).
2. Automation tests in the `FeelKit` group pass, including new tests for: camera StrongestWins and AdditiveCapped, time arbiter priority and restore (on stop, on cancel, on target destroyed), screen strongest-wins, Cooldown, MaxConcurrent.
3. A test proves the subsystem does not tick while no instances are active (RT-002, NF-001).
4. The user confirms in PIE on the Third Person map, without changing any game class:
   - `PlayFeel` from a Blueprint plays shake, flash and scale punch on the character
   - A Global Hitstop overlapping a shake: the shake keeps moving in real time during the hitstop, and time returns to normal afterwards
   - Stopping PIE during a hitstop leaves no slowed time in the next PIE session
   - `feel.Enabled 0` stops all playback

Criterion 4 can only be checked by the user. When 1 to 3 pass, stop and ask the user to verify 4.

## Platform notes

- **Controller rumble on Windows (confirmed 2026-09-15):** Unreal only sends force feedback to XInput (Xbox-style) pads. Non-XInput controllers (PlayStation, Switch, generic) rumble only when the game runs through Steam with Steam Input translating the controller, which also applies to packaged builds launched via Steam. FeelKit is not at fault: it uses Unreal's standard dynamic force feedback, and `showdebug forcefeedback` shows the motor values. This belongs in the FAQ (DOC-007). Native DualSense support is the premium FeelHaptics module (ST-028, OD-005).

## Build and test

Host project: `B:\NewUE5Project\GameFeelDev` (C++ Third Person template). Plugin: `GameFeelDev\Plugins\FeelKit`. No git.

```
# Build the editor target
"B:/UE_5.6/Engine/Build/BatchFiles/Build.bat" GameFeelDevEditor Win64 Development -Project="B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" -WaitMutex

# Run automation tests headless
"B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" -ExecCmds="Automation RunTests FeelKit; Quit" -unattended -nullrhi -nosplash -log

# Second host project for the First Person demos (Shooter, Horror): FeelDemoFP, plugin linked by a junction
# FeelDemoFP/Plugins/FeelKit -> GameFeelDev/Plugins/FeelKit (never delete FeelDemoFP/Plugins recursively; rmdir the junction)
# GAS add-on (D-076): FeelKit/Extras/FeelKitGAS (own .uplugin, module FeelGAS); GameFeelDev uses it through the junction
# GameFeelDev/Plugins/FeelKitGAS -> FeelKit/Extras/FeelKitGAS (never delete recursively; rmdir the junction)
"B:/UE_5.6/Engine/Build/BatchFiles/Build.bat" FeelDemoFPEditor Win64 Development -Project="B:/NewUE5Project/FeelDemoFP/FeelDemoFP.uproject" -WaitMutex

# Ready-made runners (PowerShell, logs in B:/NewUE5Project/Build/Logs): Tools/Run/
#   run_tests.ps1 [-Filter FeelKit]      all automation tests, headless
#   run_ui_tests.ps1                     the 7 editor window tests (needs a rendering session)
#   run_arpg.ps1 / run_platformer.ps1 / run_platformer_snap.ps1   demo playthroughs in GameFeelDev
#   run_shooter.ps1 / run_horror.ps1     demo playthroughs in FeelDemoFP
#   run_arpg_guard.ps1                   Action/RPG guards: block, guard break, parry, respawn (18 checks)
#   run_arpg_camera.ps1 [-Setups ...] [-FeelOff]   Action/RPG camera study: visibility, sharpness, FeelKit turn per frame
#   run_platformer_landing.ps1           what FeelKit adds to the camera on landings from four heights
#   run_feelswitch_ui.ps1                Feel Switch in Play In Editor, with pictures
#   package_plugin.ps1 [-Edition Lite|Pro] [-FabUrl ...]   the shipping package: copy without tests (D-075),
#                                        edition split (make_edition.py, D-100), strict BuildPlugin, checks
#   package_all.ps1                      both editions on 5.6, 5.7, 5.8 (Build/FKPkg_<Edition>_<tag>)
#   check_packages.ps1 / check_upgrade.ps1   each package loaded on its engine; Lite to Pro move
#   make_uploads.py / make_gallery.py    Publish/ zips and gallery images
#   run_gallery.ps1 / lite_gallery_shots.ps1   demo pictures (FEELKIT_GALLERY) and Lite editor pictures
# Edition markers in the plugin source: // FEELKIT_PRO_BEGIN ... // FEELKIT_PRO_END around Pro-only code,
# // FEELKIT_LITE: <code> for a Lite-only line. New Pro-only code must be marked, or Lite will not build.
#   verify_engine.ps1 -Engine -Version  copies both projects to Build/UE<tag>, builds with that engine, runs tests, UI tests, playthroughs
#   gas_proof.ps1                        two new projects from the package: without the GAS add-on GAS stays off; with it the GAS test passes
# Thumbnails: DiagFeel.RefreshRecipeThumbnails (rendering session) re-renders and saves every recipe thumbnail under /FeelKit
# Demo kit builders, comfort menu builder and sound imports: Tools/Unreal/*.py (run with the editor closed)
# Recipe generators: Tools/Recipes/gen_*.py (library, demo kits, comfort menu previews; write JSON)
# Manual: Tools/Manual/gen_reference.py (reference chapters from the code), build_manual.py [--pdf], lint_manual.py

# Package the plugin (release check)
"B:/UE_5.6/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin -Plugin="B:/NewUE5Project/GameFeelDev/Plugins/FeelKit/FeelKit.uplugin" -Package="<output>" -TargetPlatforms=Win64 -StrictIncludes
```

## Working style

- Work in small steps. Build after each meaningful change.
- When a step needs visual verification, stop and give the user a short checklist.
- When unsure about a design choice not covered here or in the requirements, ask instead of guessing.
