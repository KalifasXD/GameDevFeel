# Lite and Pro {#ch15}

FeelKit comes in two editions on Fab. **FeelKit Lite** is free and complete in itself: the recipe editor, twelve effects, the whole comfort layer and a small library. **FeelKit Pro** adds the rest: every effect, the full library and its browser, the ways of hooking recipes to gameplay without Blueprint wiring, multiplayer, GAS, and the debugging tools. Both editions are the same plugin with the same modules and class names, so a project can start on Lite and move to Pro without changing anything.

This manual describes both. A section that applies only to Pro has a pale green band on its heading with **PRO** at the right, and a table row that applies only to Pro starts with **PRO**. Anything without the band or the label applies to both editions.

## What each edition contains {#ch15_contents}

| Area | FeelKit Lite | FeelKit Pro |
|---|---|---|
| Recipe editor | Timeline, preview, intensity curves, scrubbing, mute and solo, copy and paste, comfort preview, Play in PIE, validation on save | The same, plus preview parameter sliders, sustain preview, Recent Plays, Capture GIF, sound waveforms and tracks from sound |
| Effects | 12 | All 37 |
| Playing recipes | **Play Feel** on an actor, component, widget, location or camera; handles, **Stop Feel**, **Stop All Feel**, **Is Feel Playing**; Cooldown and Max Concurrent | The same, plus **Play Feel with Context**, **Play Feel and Wait**, parameters, accumulators and sustained recipes |
| Hooking up gameplay | Blueprint and C++ calls | The same, plus Feel Maps and events, animation notifies, the Feel Trigger and Feel Input components, and a Gameplay Ability System add-on |
| Multiplayer | Effects play on the machine that calls **Play Feel** | The Feel Replication component sends plays to other machines |
| Comfort | Every comfort setting, the presets, the flash limiter, engine camera shakes and force feedback, saving, custom storage, essential tracks, the comfort menu | The same, plus the Comfort Audit |
| Feel Switch | Yes | Yes |
| Library | 11 recipes | 38 recipes, the Recipe Browser and Recipe from Template |
| Tools | Content Browser recipe tiles and tooltips | The same, plus the FeelKit Debugger, `showdebug feel`, `stat Feel`, `feel.GlobalScale`, and JSON import and export |
| Demo recipes | None | The recipes of the Shooter, Horror and Platformer demo levels |
| Engine plugins it enables | None | Niagara and Enhanced Input |

## What Lite leaves out {#ch15_lite}

Everything below is part of Pro only. Lite does not contain it at all, so it never appears in a Lite project's menus, pickers or node search.

**Effects.** Lite has Procedural Shake, Camera Punch, FOV Kick, Global Hitstop, Slow-mo Ramp, Screen Flash, Vignette Pulse, Scale Punch, Squash and Stretch, Play Sound, Force Feedback Curve and Blueprint Event. Pro adds:

| Group | Effects |
|---|---|
| Camera | Camera Roll, Camera Zoom, Look-at Nudge |
| Time | Actor Hitstop |
| Screen | Chromatic Aberration, Desaturate, Color Tint, Screen Fade, Post Process Material Pulse |
| Actor | Material Parameter Pulse, Hit Flash, Mesh Wobble, Light Flash |
| Audio | Sound Class Duck, Pitch Bend, Low-pass Sweep |
| Controller | Haptic Pattern |
| UI | Widget Punch, Widget Shake, Widget Flash, Number Pop |
| Spawn | Spawn Decal, Spawn Particle |
| Recipes | Play Recipe, Random Choice |

**Nodes and components.** Play Feel with Context, Play Feel and Wait, Release Feel, Set Feel Parameter, Add to Feel Accumulator, Set Feel Accumulator, Get Feel Accumulator, Send Feel Event; the Feel Map asset; the Play Feel, Send Feel Event, Set Feel Value and Play Feel (Window) animation notifies; the Feel Trigger, Feel Input and Feel Replication components; the Gameplay Cue notifies of the GAS add-on; the editor scripting functions for JSON and project settings ([Ref: ch17]).

**Recipe and track fields.** Parameters, Sustain, the library fields (Feeling, Genres, Description, Based On), Applies To, Parameter Mappings, Randomness and Conditions.

**Project settings.** Accumulators, Allow Global Time Dilation in Multiplayer, and the Feel Maps list ([Ref: ch18]).

**Editor tools.** The Recipe Browser, Recipe from Template, the FeelKit Debugger, Recent Plays, Capture GIF, sound waveforms and tracks from sound, JSON import and export, and the Comfort Audit.

**Console.** `feel.GlobalScale`, `stat Feel` and `showdebug feel`. `feel.Enabled` is part of both ([Ref: ch19]).

## The Lite library {#ch15_recipes}

FeelKit Lite ships 11 of the 38 library recipes: the ones built only from Lite's effects. They are listed with their descriptions in [Ref: ch09_recipes].

| Feeling | Recipes |
|---|---|
| Impact | `FR_Impact_LightHit`, `FR_Impact_HeavyHit` |
| Weight | `FR_Weight_Stomp`, `FR_Weight_HeavyFootstep` |
| Speed | `FR_Speed_SprintStart` |
| Reward | `FR_Reward_Pickup`, `FR_Reward_KillConfirm` |
| Danger | `FR_Danger_DamageTaken` |
| Dread | `FR_Dread_JumpScare` |
| Denial | `FR_Denial_Blocked`, `FR_Denial_Locked` |

Lite also ships the seven sample sounds these recipes use, the comfort menu and its preview recipes. Lite has no Recipe Browser: the recipes are in the Content Browser under **Plugins** > **FeelKit Lite Content** > **Library**, where their tiles show their tracks. Open one and click **Copy to Project** in the banner above the timeline to get an editable copy ([Ref: ch09_copy]).

## Recipes and data in Lite {#ch15_data}

Lite and Pro save recipes in the same format. A recipe keeps all of its fields in both editions: Lite hides the Pro fields from the editor but does not remove their values, so parameters, random ranges and conditions set in Pro survive a trip through Lite.

Recipes made in Lite open in Pro unchanged. The other way round has one limit: a recipe that uses an effect Lite does not have loses that track's step when it is opened in Lite, and saving it in Lite makes the loss permanent. Open such recipes in Pro only.

The **Play Recipe** and **Random Choice** steps are part of the evaluation core, so a Lite project still plays Pro recipes that contain them, as long as the steps they hold are Lite steps. Lite does not offer them when adding a track.

## Moving from Lite to Pro {#ch15_upgrade}

Remove FeelKit Lite from the engine and install FeelKit Pro to the same engine version. Every recipe, Blueprint node and setting carries over; the Pro features appear in the editor, and the Pro fields of existing recipes start at their defaults. Both editions are the same plugin to Unreal, so only one can be installed per engine version. The steps are in [Ref: ch03_upgrade].

## Licenses {#ch15_license}

Both editions are sold under Fab's Standard License. FeelKit Lite is free, has no time limit and no watermark, and games made with it can be sold. FeelKit Pro is paid, with Personal and Professional licenses as Fab offers them. The terms themselves are Fab's; the listing on Fab links to them.
