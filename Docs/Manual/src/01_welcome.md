# Welcome {#ch01}

FeelKit is an Unreal Engine plugin for game feel: the short, precise feedback that makes an action read as heavy, fast, dangerous or rewarding. A hit that freezes the game for a few frames, kicks the camera, flashes the screen, punches the character's scale, plays a sound and rumbles the controller is one such piece of feedback. FeelKit lets these effects be designed, previewed and tuned together, and played from the game with a single node.

## What FeelKit does {#ch01_what}

In a game that uses FeelKit, gameplay code states what happened, and FeelKit decides how it feels. The design rests on four ideas, each described in [Ref: ch02]:

- **Recipes.** A recipe is an asset that places effects on a timeline: a camera punch at 0 s, a hitstop at 0.02 s, a sound, a flash, a rumble. The game plays the whole recipe with **Play Feel**, so the timing of the effects is designed once, in one place.
- **A preview that matches the game.** The recipe editor plays the recipe while it is being edited, using the same code as the game, so most of a recipe is tuned without starting the game. Scrubbing the timeline shows any moment exactly.
- **Rules for overlapping effects.** When several recipes play at once, FeelKit decides which camera shake, flash or time scale wins, and always restores the camera and game time afterwards.
- **Comfort for players.** Every effect belongs to a comfort group, such as camera shake, flashes or controller vibration. Players can turn each group down in a ready-made menu, and FeelKit applies their choice to every recipe the game plays. The recipe editor can preview a recipe with the same settings.

[shot: S01-01 | The recipe editor with the HeavyHit recipe: the preview above, the timeline with its seven tracks below, the settings on the right]

## What is in the package {#ch01_package}

| Part | Contents |
|---|---|
| Plugin modules | **FeelCore** (runtime), **FeelEditor** (the recipe editor and editor tools), **FeelNiagara** (the particle step, Pro only) and **FeelEnhancedInput** (the Feel Input component, Pro only). Full C++ source is included. |
| Steps | 37 effects (twelve in Lite) for the camera, the screen, actors, game time, audio, controller vibration, UI and spawned effects ([Ref: ch16]). |
| Recipe library | 38 ready-made recipes (eleven in Lite), grouped by feeling: Impact, Weight, Power, Speed, Reward, Danger, Dread, Denial and Interface ([Ref: ch09]). |
| Demo recipes | **Pro.** The recipes of the Shooter (20), Horror (8) and Platformer (5) demo levels ([Ref: ch13]). |
| Samples | 43 sounds (seven in Lite), one sound attenuation asset and three materials, used by the library and the demos. |
| Comfort menu | A player menu for the comfort settings, ready to use and to restyle ([Ref: ch08]). |
| GAS add-on | **Pro.** A separate plugin in the `Extras` folder that plays recipes from Gameplay Cues ([Ref: ch03_gas]). |

FeelKit is sold in two editions. **FeelKit Lite** is free and contains the recipe editor, twelve steps, the comfort layer with its menu, the Feel Switch and a selection of library recipes. **FeelKit Pro** contains everything described in this manual. [Ref: ch15] compares the two in full.

Playable builds of the four demo levels show FeelKit in finished scenes, with a key that turns FeelKit off and on for comparison:

- [Action/RPG](https://drive.google.com/file/d/1439QD9WBBOfkhdBi9PYWdEE0jxMGH-Ow/view?usp=sharing)
- [Platformer](https://drive.google.com/file/d/1dzFHOMr03-rhNYP136canMOdSchmUExf/view?usp=sharing)
- [Shooter](https://drive.google.com/file/d/136TpVvZeGb1wYiVQnvFN9dw47Ej3ffHW/view?usp=sharing)
- [Horror](https://drive.google.com/file/d/1Akb1gguqHMuv_-cLY3VhWjjfTunTtPkU/view?usp=sharing)

## How to use this manual {#ch01_manual}

The manual has two parts:

- **Part One, the guide** (chapters 1 to 15), explains how to work with FeelKit, task by task, in the order in which a project usually uses it. Chapters 2 to 4 are worth reading in full; the others can be read when their topic comes up.
- **Part Two, the reference** (chapters 16 onwards), lists every step, node, component, setting and console command with its default values. It is meant for looking things up, not for reading from start to end. Its entries are generated from FeelKit's source code, so they always match the version they describe.

**Finding a topic.** The table below points to the section for the most common tasks. In the PDF, the bookmarks panel lists every section, and all references in the text are links.

| To do this | Read |
|---|---|
| Install FeelKit and enable it in a project | [see: ch03_install] |
| Get a first result in five minutes | [see: ch04] |
| Build or change a recipe | [see: ch05] |
| Play a recipe from a Blueprint or from C++ | [Ref: ch06_play] |
| Play recipes from animations, collisions, damage or input without Blueprint wiring | [Ref: ch06] |
| Make one recipe serve light and heavy hits | [Ref: ch07_parameters] |
| Let players reduce shake, flashes or vibration | [Ref: ch08] |
| Find out why a recipe did not play | [Ref: ch10] |
| Use FeelKit in a multiplayer game | [Ref: ch11] |
| Look up what a step, node or setting does | [see: ch16], [see: ch17], [see: ch18] |
| Solve a known problem | [Ref: ch14] |

**Conventions.**

- Names shown in the Unreal editor, such as buttons, menus and settings, are printed in **bold**: **Play Feel**, **Project Settings** > **Plugins** > **FeelKit**.
- Names that are code, assets or gameplay tags are printed in a code font: `FR_Impact_HeavyHit`, `Feel.Camera.Shake`, `UFeelRecipe`.
- A section that only FeelKit Pro contains has a pale green band on its heading, with a green bar on the left and **PRO** at the right. In tables and in the contents, the word **PRO** marks what only Pro contains. Everything without the band or the label is in both editions.
- Times are in seconds and distances in centimeters, as in Unreal.

## Getting help {#ch01_help}

- **Discord:** [discord.gg/AtJ6RdwaxA](https://discord.gg/AtJ6RdwaxA), for questions, problems and suggestions.
- **Email:** billoue4@gmail.com, which is also the support address in the plugin's details in **Edit** > **Plugins**.

A report is easiest to act on when it names the engine version, the FeelKit version (shown in **Edit** > **Plugins**), what was expected and what happened, and any `LogFeel` lines from the Output Log. [Ref: ch14] answers the most common questions, and [Ref: ch10] describes the tools that show what FeelKit is doing while the game runs.
