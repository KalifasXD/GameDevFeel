# The recipe library and browser {#ch09}

FeelKit includes a library of 38 ready-made recipes, grouped by the feeling they produce. They are finished recipes that can be used as they are, and starting points for a project's own. FeelKit Lite includes a selection of them; FeelKit Pro includes all 38, together with the Recipe Browser, which shows and plays every recipe in one window.

## The Recipe Browser {#ch09_browser}
[edition: Pro]

Open the browser with **Tools** > **FeelKit Recipe Browser**, or right-click a folder in the Content Browser and choose **Browse Recipes...**. It lists the library recipes and the project's own recipes side by side, and plays each one in a small preview.

[shot: S09-01 | The Recipe Browser with FR_Impact_HeavyHit selected: filters on the left, the recipe list, and the preview with the recipe's details]

The window has three areas:

| Area | Contents |
|---|---|
| Filters (left) | **Source**, **Feeling**, **Genre** and **Affects**, described in [see: ch09_filters]. **Clear Filters** shows every recipe again. |
| Recipe list (center) | One tile per recipe, with a search box above it and the number of recipes shown below it. |
| Preview (right) | The selected recipe playing in a preview, its details, a slider for each of its parameters, and the buttons **Use**, **Open** and **Show in Content Browser**. |

The details list the recipe's **Name**, **Description**, **Feeling**, **Genres**, what it **Affects** (camera, screen, audio and so on), its number of **Tracks**, its **Length** and its **Source**: the FeelKit library (read-only) or this project.

## Filters, search and live preview {#ch09_filters}
[edition: Pro]

**Filters.** Each filter narrows the list:

| Filter | Shows | With two or more picked |
|---|---|---|
| **Source** | **All**, **Library** (recipes that ship with FeelKit) or **Project** (the project's own recipes). | One choice at a time. |
| **Feeling** | Recipes of the picked feelings. The number next to each feeling counts its recipes. | Recipes with any of them. |
| **Genre** | Recipes suited to the picked genres. | Recipes with any of them. |
| **Affects** | Recipes that affect the picked channels, such as camera shake or controller vibration. | Only recipes that affect all of them, for example shake and rumble. |

**Search.** The search box matches recipe names, descriptions and parameter names, so a search for `Damage` finds the recipes that take a damage value.

**Live preview.** Resting the pointer on a tile for a quarter of a second plays that recipe once in the preview. Selecting a tile plays it in a loop. Parameter sliders under the preview play the recipe with other values, as in the recipe editor. Two controls of the browser change how the preview plays: **Mute** silences the recipe's sounds, and the comfort menu plays it as a player with reduced motion, reduced flashing or no haptics would experience it ([Ref: ch08]).

## Copying a library recipe into your project {#ch09_copy}

A project uses its own copy of a library recipe. Copies can be made in three ways:

- **In the Recipe Browser** (Pro): select the recipe and click **Use**, or double-click it.
- **In the recipe editor:** open the library recipe and click **Copy to Project** in the banner above the timeline ([Ref: ch04_pick]).
- **From a template** (Pro): right-click a folder in the Content Browser and choose **Recipe from Template...**. A window titled **Create a recipe from a template** shows the library in the same layout as the Recipe Browser; select a recipe and click **Create**.

A save dialog opens, with the name shortened to the part after the feeling: `FR_Impact_HeavyHit` becomes `HeavyHit`. The copy opens in the recipe editor and can be changed freely. In Pro, its **Based On** field records the library recipe it came from.

Some library recipes use sounds and materials from **FeelKit Content** > **Samples**. The copy refers to the same assets; duplicating them into the project is only necessary when they are to be changed.

## Why library recipes are read-only {#ch09_readonly}

Fab installs FeelKit in the engine folder, which every project using that engine version shares, and a FeelKit update replaces the library. A change to a library recipe would therefore affect every project and would be lost at the next update. For this reason the library cannot be edited: its recipes open read-only, and the Content Browser does not save changes to them.

To author the library itself, for example in a studio that maintains its own version of FeelKit, turn on **Edit** > **Editor Preferences** > **Plugins** > **FeelKit** > **Allow Library Editing**. The setting is personal to each user and is off by default.

## Content Browser tiles and tooltips {#ch09_tiles}

In the Content Browser, a recipe's tile shows a small picture of its timeline: one colored bar per track, in the channel colors of the recipe editor. Recipes can therefore be told apart at a glance, as [Ref: ch04_pick] shows for the Impact folder.

Resting the pointer on a tile shows a tooltip with the recipe's **Description**, **Feeling**, **Genres**, **Channels**, number of **Tracks**, **Length** and whether it is **Sustained**. These values are also asset registry tags, so the Content Browser's own filters and searches can use them.

## The 38 library recipes at a glance {#ch09_recipes}

The descriptions are those shown in the Recipe Browser. Recipes marked **PRO** are in FeelKit Pro only; the other eleven are in both editions. Recipes marked sustained keep playing until they are released ([Ref: ch07_sustain]); parameters are listed where a recipe takes one.

{widths: 34,52,14}
| Recipe | Description | Parameter |
|---|---|---|
| >> Impact | | |
| `FR_Impact_BulletImpact` | **Pro.** A shot landing on a surface: a small punch, a mark on the surface and a short rumble. | |
| `FR_Impact_CriticalHit` | **Pro.** A critical hit: a longer freeze, a slow-motion beat, a bright flash and a color push. | |
| `FR_Impact_HeavyHit` | A heavy blow: a longer freeze, a deep camera punch, a flash and a strong rumble. | |
| `FR_Impact_LightHit` | A quick, light hit: a short shake, a small punch and a brief freeze. | |
| `FR_Impact_ScalableHit` | **Pro.** One hit that covers light to heavy: Damage drives how hard everything lands. | `Damage` |
| >> Weight | | |
| `FR_Weight_HeavyFootstep` | Each step of something big: a small ground shake felt more than seen. | |
| `FR_Weight_Land` | **Pro.** Landing on the ground, scaled by how fast the fall was. | `FallSpeed` |
| `FR_Weight_Slam` | **Pro.** Something heavy hitting the ground nearby: a hard shake and a mark left behind. | |
| `FR_Weight_Stomp` | A deliberate, heavy stomp: a freeze, a drop of the camera and a long rumble. | |
| >> Power | | |
| `FR_Power_AbilityCast` | **Pro.** Casting an ability: a short wind-up, a colored wash and a firm rumble. | |
| `FR_Power_ChargeUp` | **Pro.** Holding a charge: a rising hum, a tightening view and a growing rumble. At full charge it releases itself and plays FR_Power_ChargedRelease; let go early and the hum fades out. Sustained. | `Charge` |
| `FR_Power_ChargedRelease` | **Pro.** The charge let go: a flash, a wide camera kick and a heavy rumble. | |
| `FR_Power_Explosion` | **Pro.** A blast going off nearby: the closer it is, the harder it hits. | `Distance` |
| >> Speed | | |
| `FR_Speed_Boost` | **Pro.** Sustained speed: the view widens and the edges blur while the boost lasts. Sustained. | |
| `FR_Speed_Dash` | **Pro.** A burst of speed in the direction of travel: a camera push, a widening view and a whoosh. | |
| `FR_Speed_SprintStart` | Breaking into a sprint: a short lean forward, a widening view and a brief shake. | |
| `FR_Speed_Whoosh` | **Pro.** Something fast passing close by: a light camera turn and a sweep of sound. | |
| >> Reward | | |
| `FR_Reward_ComboStep` | **Pro.** Each step of a streak hits harder than the last: the pop, the camera bump and the rumble grow with the combo, and the count pops up on screen. | `Combo` |
| `FR_Reward_KillConfirm` | Confirmation that something went down: a crisp double tick and a short freeze. | |
| `FR_Reward_LevelUp` | **Pro.** A milestone reached: a warm flash, a lift of the camera and a triumphant sound. | |
| `FR_Reward_Pickup` | Collecting something: a bright, short pop with a rising sound. | |
| >> Danger | | |
| `FR_Danger_Alarm` | **Pro.** An alarm going off: a repeating red wash and a warning tone. Sustained. | |
| `FR_Danger_DamageTaken` | Being hurt: a red wash at the edges, a jolt and a low rumble. | |
| `FR_Danger_DirectionalDamage` | **Pro.** Being hurt from a direction: the camera is pushed away from where it came from. | |
| `FR_Danger_LowHealth` | **Pro.** Running low: a slow pulse at the edges and a heartbeat that rises as health falls. Sustained. | `Health` |
| >> Dread | | |
| `FR_Dread_FailingLight` | **Pro.** A light about to die: irregular flicker with a dip in color. Sustained. | |
| `FR_Dread_Heartbeat` | **Pro.** A heartbeat that follows fear: the beat, the pulse at the edges, the rumble and a slight zoom grow stronger the closer the threat. Sustained. | `Fear` |
| `FR_Dread_JumpScare` | A sudden scare: one hard flash and a stab of sound, with a safe substitute when flashes are turned down. | |
| `FR_Dread_Unease` | **Pro.** Creeping unease: color drains, the edges close in and the world sounds muffled. Sustained. | |
| >> Denial | | |
| `FR_Denial_Blocked` | An action that will not happen: a short stop and a dull thud. | |
| `FR_Denial_Locked` | Something that will not open: a heavy rattle that goes nowhere. | |
| `FR_Denial_OutOfAmmo` | **Pro.** Nothing left to fire: a dry click and a small shake of refusal. | |
| `FR_Denial_WrongInput` | **Pro.** The wrong button: a red shake on the element that refused it. | |
| >> Interface | | |
| `FR_Interface_ButtonHover` | **Pro.** The cursor arrives on a button: a small lift and a soft tick. | |
| `FR_Interface_ButtonPress` | **Pro.** A button taking the press: a squash inward, then a bounce back. | |
| `FR_Interface_Notification` | **Pro.** Something arrives on screen: it slides in, settles and chimes. | |
| `FR_Interface_ScoreTick` | **Pro.** A score counting up: the points pop up, the counter punches and a short click plays. | `Combo` |
| `FR_Interface_ScreenTransition` | **Pro.** Moving between screens: a quick fade out and back in. | |

**Shared parameter names.** Library recipes use the same names for the same kind of value, so one game value can drive several recipes: `Damage`, `FallSpeed`, `Health`, `Fear`, `Charge` and `Distance`. `Distance` is filled in automatically with the distance from the play to the nearest local camera ([Ref: ch07_distance]).

**The Combo accumulator.** `FR_Reward_ComboStep` and `FR_Interface_ScoreTick` read the accumulator `Combo` when the game passes no value. A project that uses them adds `Combo` under **Project Settings** > **Plugins** > **FeelKit** > **Accumulators** and adds to it on each step of a streak ([Ref: ch07_accumulators]). Until then, the recipes play with their default value and saving them reports that the accumulator is not defined.

**Interface recipes.** Most of them move, scale or tint a UMG widget, such as a button or a score counter, so they are played with a target from **Make Feel Target from Widget** ([Ref: ch06_play]). `FR_Interface_ScreenTransition` fades the whole screen and needs no widget.

## Tagging your own recipes {#ch09_tagging}
[edition: Pro]

A project's own recipes appear in the Recipe Browser beside the library, and its filters work for them when their library information is filled in. In the recipe editor, with no track selected, the **Library** category of **Details** holds:

- **Feeling**: one tag under `Feel.Feeling`, such as `Feel.Feeling.Impact`.
- **Genres**: any number of tags under `Feel.Genre`, such as `Feel.Genre.Shooter`.
- **Description**: one or two sentences, shown in the browser and in the Content Browser tooltip.

A project can add its own feelings and genres as gameplay tags under `Feel.Feeling` and `Feel.Genre` in **Project Settings** > **Gameplay Tags**; they then appear as filters in the browser. This information is used only by the editor and is not included in packaged games.
