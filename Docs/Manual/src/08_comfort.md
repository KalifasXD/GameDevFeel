# Comfort and accessibility {#ch08}

Strong feedback suits many players and troubles some. Camera shake and motion can cause motion sickness, flashes can be uncomfortable or, for people with photosensitive epilepsy, dangerous, and vibration is unwelcome to others. FeelKit lets every player turn each kind of effect down, applies the choice to every recipe the game plays, and saves it. The complete comfort layer, including the ready-made comfort menu, is part of both editions.

[video: V4]

## Comfort groups, and how the scales apply {#ch08_groups}

Every effect belongs to a comfort group through its channel. Each player has a scale from 0 (off) to 1 (full strength) for each group, and a **Master** scale that multiplies all of them.

| Group | Channels under the default mapping | Typical effects |
|---|---|---|
| **Camera Shake** | `Feel.Camera.Shake` | Procedural Shake |
| **Camera Motion** | `Feel.Camera.Motion` | Camera Punch, FOV Kick, Camera Roll, Camera Zoom |
| **Flashes** | `Feel.Screen.Flash`, `Feel.Actor.Light` | Screen Flash, Light Flash |
| **Hitstop and Slow-mo** | `Feel.Time.Hitstop`, `Feel.Time.SlowMo` | Global Hitstop, Actor Hitstop, Slow-mo Ramp |
| **Screen Distortion** | `Feel.Screen.Distortion`, `Feel.Screen.Color` | Vignette Pulse, Chromatic Aberration, Desaturate, Color Tint |
| **Haptics** | `Feel.Haptics` | Force Feedback Curve, Haptic Pattern |

Channels that no group lists, such as sounds, actor motion and UI effects, are scaled by **Master** alone. The mapping is set in **Project Settings** > **Plugins** > **FeelKit** > **Channel Comfort Groups** ([Ref: ch18]); when a channel matches several rows, the most specific tag wins, so a project can place `Feel.Camera.Shake.Explosion` in a group of its own.

A track's final intensity is the intensity of the play, times its intensity curve, times the player's scale for the track's group, times **Master** ([Ref: ch02_intensity]). A track whose scale is 0 is skipped entirely, unless it is essential ([see: ch08_essential]).

**Which player.** Each local player has settings of their own, and a play uses the settings of the local player its target belongs to. A play that belongs to no local player uses the project's default scales.

## Presets {#ch08_presets}

Presets set every scale and option at once. Players choose one as a starting point and can adjust it afterwards.

| Preset | Values (defaults, editable in the project settings) |
|---|---|
| **Default** | The project's **Default Comfort Scales**: every scale 1, the flash limiter on at 3 flashes per second. New players start with it. |
| **Reduced Motion** | Camera Shake 0.25, Camera Motion 0.25, Screen Distortion 0.5, camera roll off, zoom speed limited to 40° per second. |
| **Reduced Flashing** | Flashes 0.2, flash limiter at 1 flash per second in **Suppress** mode. |
| **No Haptics** | Haptics 0. |

The values of the built-in presets are set under **Project Settings** > **Plugins** > **FeelKit** > **Comfort** > **Presets**. A project can add presets of its own as **Feel Comfort Preset** assets (right-click in the Content Browser, **Miscellaneous** > **Data Asset**, class **Feel Comfort Preset**), each with a **Display Name** and a set of scales, and apply them with **Apply Custom Comfort Preset**.

## Reading and changing a player's settings {#ch08_settings}

The comfort settings of a player are held by the player's **Feel Comfort** subsystem. **Get Feel Comfort** returns it from a player controller; the other nodes are called on it:

| Node | What it does |
|---|---|
| **Get Comfort Scales**, **Set Comfort Scales** | Reads or replaces all scales and options at once. |
| **Set Master Comfort Scale** | Sets **Master**. |
| **Set Comfort Group Scale** | Sets one group, for example Camera Shake to 0.5. |
| **Apply Comfort Preset**, **Apply Custom Comfort Preset** | Applies a built-in preset or a preset asset. |
| **Save Comfort Settings**, **Load Comfort Settings** | Saves or reloads the settings now ([see: ch08_storage]). |
| **Get Effective Force Feedback Scale** | How much of the game's own controller vibration reaches the player ([see: ch08_engine]). |

[shot: S08-01 | Get Feel Comfort from the player controller, then Apply Comfort Preset (Reduced Motion) and Set Comfort Group Scale (Camera Shake 0.5)]

Changes apply at once to every recipe, including recipes already playing. With **Auto Save Comfort** on, the default, each change is also saved.

Besides the scales, each player's settings hold four options:

| Option | Default | Effect |
|---|---|---|
| **Limit Flashes**, **Max Flashes Per Second**, **Flash Limit Mode**, **Softened Flash Scale** | On, 3, Soften, 0.3 | The flash limiter ([see: ch08_flash_limiter]). |
| **Allow Camera Roll** | On | Off removes camera roll from every recipe. |
| **Max Field Of View Change Per Second** | 0 (no limit) | The fastest zoom FeelKit may cause, in degrees per second ([see: ch08_motion]). |

## Essential tracks and substitutes {#ch08_essential}

Some feedback carries information the player needs, such as the flash that shows the player was hit. Turning that effect off must not remove the information. A track marked **Essential**, in the **Comfort** category of its settings, is treated differently when its group is turned down:

- With a **Substitute Step** and a scale of 0, the substitute plays instead. Choose a substitute from another group, for example a vignette or a sound instead of a flash, so the information still arrives in a form the player accepts. The substitute plays at the scale of its own group.
- Without a substitute, the scale never falls below the track's **Essential Floor**: at a floor of 0.3, a player who turns flashes off still sees the flash at 30 percent.
- On **Camera Shake** and **Camera Motion**, a player's choice always wins: at 0 these tracks stop completely, whatever the floor, because a little motion is still too much for a player who is made unwell by it. Give such tracks a substitute on another channel.

Tracks that are not essential stop playing when their group reaches 0.

[shot: S08-02 | The Comfort category of the flash track of FR_Dread_JumpScare: Essential, a Vignette Pulse substitute and a floor]

The library recipes `FR_Dread_JumpScare`, `FR_Danger_DamageTaken` and `FR_Danger_DirectionalDamage` each have an essential Screen Flash with a Vignette Pulse substitute. Saving a recipe reports an essential track that comfort settings could remove completely ([Ref: ch05_validation]).

In the recipe editor, the **Comfort** menu of the preview plays a recipe with a preset, and tracks show what comfort does to them ([Ref: ch05_comfort]). The substitute plays only when a group's scale is exactly 0. Of the built-in presets only **No Haptics** sets a scale of 0, so check a flash or motion substitute in the game, with that group set to 0.

## The flash limiter {#ch08_flash_limiter}

The flash limiter counts how many flashes start within a second: every track on a channel mapped to the **Flashes** group, including the tracks of a sustained recipe's release recipe ([Ref: ch07_sustain]). When more start than **Max Flashes Per Second** allows (3 by default), the extra flashes are softened to **Softened Flash Scale** (0.3) or, in **Suppress** mode, not played. The limiter is on by default and is part of each player's settings, so a player can make it stricter.

In the recipe editor, flash tracks that the limiter changes say **Flash softened by the flash limiter** or **Flash suppressed by the flash limiter**.

The flash limiter reduces risk; it does not certify a game as safe. Test the finished game with a photosensitivity analysis tool.

## Motion comfort: camera roll and field of view speed {#ch08_motion}

Two options address motion sickness beyond the Camera Shake and Camera Motion scales:

- **Allow Camera Roll**, when off, removes every sideways tilt of the camera that FeelKit causes, wherever it comes from.
- **Max Field Of View Change Per Second** limits how fast FeelKit may zoom the view. A field of view kick that would change faster is slowed to the limit. 0 means no limit.

## Engine camera shakes and controller vibration {#ch08_engine}

A game usually has camera shakes and controller vibration of its own, played without FeelKit. FeelKit can apply the players' settings to them as well, so that one menu controls everything:

- **Apply Comfort to Engine Camera Shakes** (on) scales the game's own Unreal camera shakes by the player's Camera Shake and Master scales.
- **Apply Comfort to Engine Force Feedback** (on) scales the game's own controller vibration by the player's Haptics and Master scales. FeelKit multiplies the game's own force feedback scale rather than replacing it, and restores it when the setting changes.

When a player's settings silence the controller completely, the Output Log says so once, with the scales responsible. **Get Effective Force Feedback Scale** returns the combined scale, and in Pro the FeelKit Debugger shows it in the **Controller vibration** row ([Ref: ch10]). On Windows, Unreal sends vibration only to Xbox-style controllers; [Ref: ch14] explains how PlayStation controllers are handled.

## Saving settings, or using your own save system {#ch08_storage}

With **Auto Save Comfort** on (the default), FeelKit saves each player's settings to a save game slot of its own, named after **Comfort Save Slot Prefix** and the player's index: `FeelComfort_0` for the first player. The settings are loaded when the local player is created, before any recipe plays, so they apply from the first frame.

A game with its own save system can store the settings there instead:

1. Create a class that implements the **Feel Comfort Storage** interface, with its two functions **Load Comfort Scales** and **Save Comfort Scales**. A Blueprint class works; so does a C++ class that implements `IFeelComfortStorage`.
2. Set **Comfort Storage Class** in **Project Settings** > **Plugins** > **FeelKit** > **Comfort** > **Storage** to that class, or call **Set Comfort Storage** on a player's Feel Comfort subsystem.

FeelKit then calls the class whenever settings are loaded or saved, and the save slot is not used.

## The comfort menu for players {#ch08_menu}

FeelKit includes a complete comfort menu, `WBP_FeelComfortMenu`, in **FeelKit Content** > **UI**. It contains:

| Section | Rows |
|---|---|
| **Master** | Scales every effect at once. |
| **EFFECTS** | **Camera shake**, **Camera motion**, **Flashes**, **Hitstop and slow motion**, **Screen distortion** and **Controller vibration**: the six group scales, shown from 0 to 100 percent. |
| **PRESETS** | **Default**, **Reduced motion**, **Reduced flashing** and **No vibration**: the four built-in presets. |
| **ADVANCED** | **Camera roll** (Allow Camera Roll), **Zoom speed** (Max Field Of View Change Per Second; the far right means **No limit**) and **Flash limiter** (Limit Flashes). |
| Buttons | **Try it** plays a sample of the effects with the current settings; **Reset** returns to the project's defaults; **Close** closes the menu. |

A line under the rows explains the selected setting in plain words, and moving a slider plays a short matching effect, so players feel the difference while they choose. Changes apply at once and are saved shortly after the player stops changing them. The menu works with the mouse, the keyboard and a controller; Esc, the controller's B / Circle button or its Menu button closes it.

[shot: S08-03 | The comfort menu during play, opened with Show Feel Comfort Menu, at the project's default settings]

**Opening the menu.** Call **Show Feel Comfort Menu** with the player controller. **Menu Class** chooses the menu, and when left empty uses **Comfort Menu Class** from the project settings, which is `WBP_FeelComfortMenu` unless changed. **Pause Game** pauses the game while the menu is open; the previews still play. The Feel Switch opens the same menu with O or the controller's Menu / Options button ([Ref: ch06_switch]).

## Restyling the comfort menu {#ch08_restyle}

The comfort menu can be given the look of the game: its layout, fonts, colors and texts are all in the Widget Blueprint. Restyle a copy of it in the project, not FeelKit's own menu.

**Why a copy.** Fab installs FeelKit in the engine folder, not in the project. FeelKit's own `WBP_FeelComfortMenu` is therefore shared by every project that uses the same engine version, and the next FeelKit update replaces it with the original without warning. A change made to it would appear in every other project and then disappear at the next update.

**Making the copy.**

1. In the Content Browser, open **FeelKit Content** > **UI**, right-click `WBP_FeelComfortMenu` and choose **Copy to Project...**.
2. Choose a folder and a name in the save dialog.
3. When asked whether to use the copy as the project's comfort menu, click **Yes**. This sets **Comfort Menu Class** in the project settings, so **Show Feel Comfort Menu** and the Feel Switch open the copy. The choice can be changed later under **Project Settings** > **Plugins** > **FeelKit** > **Comfort**.

The copy opens in the UI designer and can be changed freely.

**What must stay.** The menu's logic finds its controls by name. A control that the copy removes is skipped, and the rest keeps working; a control that is renamed is treated as removed. The names are:

| Kind | Names |
|---|---|
| Sliders | `MasterSlider`, `CameraShakeSlider`, `CameraMotionSlider`, `FlashesSlider`, `HitstopSlider`, `ScreenDistortionSlider`, `HapticsSlider`, `ZoomSpeedSlider` |
| Value texts | `MasterValue`, `CameraShakeValue`, `CameraMotionValue`, `FlashesValue`, `HitstopValue`, `ScreenDistortionValue`, `HapticsValue`, `ZoomSpeedValue` |
| Check boxes | `CameraRollCheckBox`, `FlashLimiterCheckBox` |
| Rows (for highlighting the selected row) | `MasterRow`, `CameraShakeRow`, `CameraMotionRow`, `FlashesRow`, `HitstopRow`, `ScreenDistortionRow`, `HapticsRow`, `CameraRollRow`, `ZoomSpeedRow`, `FlashLimiterRow` |
| Buttons | `DefaultPresetButton`, `ReducedMotionPresetButton`, `ReducedFlashingPresetButton`, `NoHapticsPresetButton`, `TryButton`, `ResetButton`, `CloseButton` |
| Texts | `DescriptionText` (the explanation line), `SaveStatusText`, `CloseHintText` |

The menu's **Class Defaults** hold its remaining settings: the explanation of each row, the recipes played by **Try it** and by each slider, the keys that close it, the row colors and the texts for "No limit", "Saved" and "Saving...".

Editing FeelKit's own menu also works, but only until the next FeelKit update.

## The Comfort Audit {#ch08_audit}
[edition: Pro]

**Tools** > **FeelKit Comfort Audit** checks every recipe and the comfort settings for likely comfort problems and lists them on the **FeelKit Comfort Audit** page of the Message Log. It warns about:

- more than three flashes starting within one second, in a recipe or in a sustain loop that repeats;
- saturated red flashes at an opacity above 0.5, the flashes most likely to cause photosensitive reactions;
- a track on a channel outside its step's comfort group, which players who turn that group down would still get;
- an essential flash with a floor and no substitute, which players who turn flashes off would still see;
- default settings without the flash limiter, and presets that do not reduce what their name promises.

It also notes when engine camera shakes or force feedback are left outside the comfort settings.

[shot: S08-04 | The FeelKit Comfort Audit page of the Message Log, here run on a project with test recipes]

The audit is a readiness helper that catches common problems early. It is not a photosensitivity certification; test the finished game with an analysis tool.
