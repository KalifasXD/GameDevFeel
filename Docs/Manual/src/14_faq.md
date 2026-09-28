# Troubleshooting and questions {#ch14}

The questions below are the ones that come up most often, grouped by what the problem looks like. Most answers start in the same two places: the Output Log, filtered by `LogFeel` ([Ref: ch10_log]), and, in FeelKit Pro, the FeelKit Debugger ([Ref: ch10_debugger]).

## Nothing plays {#ch14_nothing}

**A recipe plays in the recipe editor but not in the game.** Work through these in order:

1. FeelKit is switched on. The console variable `feel.Enabled` at 0 stops every play and refuses new ones ([Ref: ch19]); a Feel Switch showing **FEEL: OFF** does the same while the game runs ([Ref: ch06_switch]).
2. The call is reached. Put a **Print String** next to **Play Feel**, or look for the recipe under **Playing** in the Debugger.
3. The target is valid. A destroyed or empty target makes **Play Feel** refuse the play, and the log says so.
4. The recipe is not held back by its own limits: **Cooldown** refuses a second play on the same target within that many seconds, and **Max Concurrent** refuses a play once that many copies play on the target (0 means no limit).
5. The player's comfort does not remove the tracks. A group at 0 skips its tracks; the Debugger's **Player N comfort** rows show the scales.

**Some tracks play in the editor but not in the game.** The recipe editor's preview passes the **Local Player Only** and **Max Distance** conditions, since it has no real distance or player. In the game they apply. Open the play from **Recent plays** in the Debugger: the replay labels each skipped track with the reason ([Ref: ch10_labels]).

**Some tracks play in the game but not in the editor.** Tracks marked **No preview**, such as hitstops, slow motion, controller vibration and Blueprint events, act only in a running game ([Ref: ch05_tracks]).

**Send Feel Event plays nothing.** No Feel Map row matches the event and its context tags, or the matching row has no recipe. The log names the event and the number of maps searched, once per event ([Ref: ch06_events]).

**A parameter or accumulator stays at its default.** The accumulator's name is not defined under **Project Settings** > **Plugins** > **FeelKit** > **Accumulators**, or the parameter's name in the recipe differs from the name the game passes. Names are compared exactly ([Ref: ch07_accumulators]).

## Controllers and vibration {#ch14_vibration}

**A PlayStation, Switch or other controller does not vibrate on Windows.** On Windows, Unreal sends vibration only to Xbox-style controllers (XInput). Other controllers vibrate when the game runs through Steam with Steam Input on, which translates them into Xbox-style controllers; this also applies to a packaged game started from Steam. FeelKit uses Unreal's standard force feedback, so it vibrates every controller that Unreal itself drives.

**An Xbox controller does not vibrate either.** Check, in this order:

- The player's **Haptics** or **Master** comfort is 0. The log says so once, with the scales, and the Debugger's **Controller vibration** row shows a warning ([Ref: ch08_engine]).
- The game has set the player controller's own **Force Feedback Scale** to 0. The same Debugger row shows it as the controller scale.
- The console command `showdebug forcefeedback` lists every running force feedback effect with its values, before the controller's scale is applied. FeelKit's effects appear there while they play.

## Camera and screen effects {#ch14_camera}

**Camera effects of a recipe played on an enemy show up on my screen. Is that intended?** Yes. Camera, screen and controller effects go to the local player of the play's target. A target that belongs to no player, such as an enemy or a crate, sends them to the first local player, so the player sees the hit they caused. To keep them off the player's screen, give those tracks a **Local Player Only** condition, or split the recipe into one for the enemy and one for the player.

**In multiplayer, another player's hit does not shake my camera.** A target that belongs to a remote player sends no camera, screen or controller effects to this machine: that player's own machine shows them ([Ref: ch11]).

**Camera Shake is at 0 in the comfort settings, but a recipe still moves the camera.** A player's 0 on **Camera Shake** or **Camera Motion** always silences those groups, even for essential tracks. A track that still moves the camera is on a channel that is not mapped to either group: check its **Channel**, and the channel mapping in the project settings ([Ref: ch08_groups]).

**Two recipes that move the camera make the view jump.** With the default **Strongest Wins** arbitration, the stronger of two camera motions wins each frame. When two plays push the camera in different directions and their strengths cross, the view switches from one to the other within one frame. Push in one direction in both recipes, or switch **Camera Arbitration** to **Additive Capped** ([Ref: ch02_overlap_between]).

**During a hitstop, the shake keeps moving.** That is intended: FeelKit times its effects in real time, so a freeze of the game does not freeze the feedback that sells it ([Ref: ch02_recipes]).

**A hitstop in a network game slows only one character.** In a networked game, a global hitstop or slow motion slows only the play's target on that machine, because slowing the whole world would put one machine out of step with the others. **Allow Global Time Dilation In Multiplayer** in the project settings changes that ([Ref: ch11_time]).

## Materials and decals {#ch14_materials}
[edition: Pro]

**A Post Process Material Pulse shows the material at full strength for the whole track.** Unreal shows an active post-process material at full strength. The step fades it by writing its weight into a scalar parameter of the material, named **Weight** by default: add that parameter and use it as the Alpha of a **Lerp** from the scene (**Scene Texture: PostProcessInput0**) to the effect. The scene texture's color has four channels, so compare it with a **Constant4Vector**, not a Constant3Vector, or the material does not compile.

**A Spawn Decal step shows nothing.** Check three things:

- The material's domain is **Deferred Decal**.
- Its inputs are connected nodes. Values typed on the result node's pins without a connected node are ignored by decal materials, and the decal draws nothing. Connect a **Constant3Vector** to **Base Color**, for example.
- A surface is within reach: with **Find Surface** on, the step looks up to **Surface Search Distance** (500 cm) beyond the target for a surface to project on.

## Blueprints, animations and GAS {#ch14_blueprints}

**The Blueprint Event step does nothing.** The event name must match a custom event or function on the target exactly, and the event must take no inputs or a single float, which receives the intensity. The log names the target and the event when either is wrong ([Ref: ch10_log]).

**A widget reaction keeps restarting while the pointer rests near a button.** A Widget Punch or Widget Shake on a button moves the button itself, and with it the area that takes the pointer: the pointer leaves the moved button, then enters it again, and the hover reaction restarts. Put a panel inside the button, let it draw the button's look, and play the reactions on the panel, not on the button. (Widget Punch and Widget Shake are part of FeelKit Pro.)

**Does a scale punch or squash on a character change its collision?** No. Actor effects on a character move and scale its mesh, never its collision capsule, so movement and hits are unaffected.

**Gameplay Cue nodes fail to compile after adding the GAS add-on.** **Execute Gameplay Cue On Actor**, **Add Gameplay Cue On Actor** and **Remove Gameplay Cue On Actor** have a **Parameters** pin that must be connected. Connect a **Make GameplayCueParameters** node to it ([Ref: ch06_gas]).

## Recipes and the library {#ch14_editor}

**A library recipe cannot be edited.** Library recipes are read-only, so that an update of FeelKit never changes a project's game. Copy one into the project and edit the copy ([Ref: ch09_copy]). **Allow Library Editing** in the editor preferences is for authoring the library itself.

**Changes to a recipe during Play In Editor do not show at once.** They apply from the next play of the recipe, without restarting Play In Editor.

**Does the preview look exactly like the game?** The preview and the game run the same code, so a track that plays in both looks the same. The differences come from the inputs: steps marked **No preview** act only in the game, the preview has its own comfort preset and parameter values, and its camera is not the game's camera. A replay of a recorded play removes the last two differences ([Ref: ch10_replay]).

## Installing, packaging and engine versions {#ch14_install}

**Packaging a Blueprint-only project fails or asks for Visual Studio.** Any code plugin that the engine does not enable by default makes Unreal build a game executable for the project, which needs Visual Studio 2022 with the C++ game development workload ([Ref: ch03_packaging]).

**The editor reports two plugins named FeelKit.** FeelKit Lite and FeelKit Pro are the same plugin to Unreal, so only one of them can be installed per engine version. Remove Lite before installing Pro ([Ref: ch03_upgrade]).

**The GAS add-on asks to be rebuilt when the project opens.** That is expected: the add-on compiles with the project. Click **Yes** ([Ref: ch03_gas]).

**A demo project from FeelKit crashes in Unreal Engine 5.7 or 5.8.** The demo projects are built on Epic's 5.6 template content, whose levels crash in Play In Editor on 5.7 and 5.8 even without FeelKit. Open them with 5.6. FeelKit itself runs on all three versions.

## Getting help {#ch14_help}

When the answer is not here, ask on the FeelKit Discord, https://discord.gg/AtJ6RdwaxA, or write to billoue4@gmail.com. A report is easiest to act on with the engine version, the FeelKit version (**Edit** > **Plugins**), what was expected, what happened, and any `LogFeel` lines from the Output Log.
