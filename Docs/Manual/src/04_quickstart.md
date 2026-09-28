# Quick start: your first recipe {#ch04}

This chapter takes one recipe from the FeelKit library into a project, previews it in the recipe editor and plays it on the player's character. It requires no C++ and takes about five minutes. The steps assume that FeelKit is enabled ([see: ch03_install]) and that the project was created from Unreal's Third Person template; any project with a playable character works in the same way.

[video: V1]

## Pick a recipe from the library {#ch04_pick}

FeelKit includes a library of ready-made recipes, grouped by the feeling they produce: Impact, Weight, Power, Speed, Reward, Danger, Dread, Denial and Interface. Library recipes are read-only, so that an update of FeelKit can replace them without affecting a project; a project always works with its own copy. This chapter uses `FR_Impact_HeavyHit`, a heavy blow that combines a short freeze, a camera shake, a camera punch, a scale punch, a flash, a sound and a controller rumble.

1. In the Content Browser, click **Settings** (the gear icon at the right end of its toolbar) and turn on **Show Plugin Content**. FeelKit's content then appears under **Plugins > FeelKit Content**.
2. Open **FeelKit Content > Library > Impact** and select `FR_Impact_HeavyHit`.

[shot: S04-01 | The Impact folder of the FeelKit library in the Content Browser. 1 the folder; 2 FR_Impact_HeavyHit]

3. Double-click the recipe. The recipe editor opens it with a banner across the timeline: "Library recipe (read-only). Copy it to your project to edit."
4. Click **Copy to Project**. In the save dialog, choose a folder of the project, keep the suggested name `HeavyHit` and click **Save**. The copy opens in the recipe editor and can be changed freely. In Pro, its **Based On** field records the library recipe it came from.

[shot: S04-02 | A library recipe opens read-only. Copy to Project creates an editable copy in the project]

> In FeelKit Pro, **Tools > FeelKit Recipe Browser** presents the same library with filters by feeling, genre and channel, and plays each recipe when the pointer rests on it. [Ref: ch09] describes it.

## Preview it without pressing Play {#ch04_preview}

The recipe editor plays a recipe on its own preview camera and mesh, using the same code as the game ([see: ch02_preview]). Most of a recipe can therefore be judged without starting the game.

1. In the **Timeline** toolbar, click **Play**, or press Space while the timeline has focus. The preview shows the camera shake, the camera punch, the scale punch and the flash, and the sound plays.
2. Click **Loop** to repeat the recipe while observing it.
3. Drag the red playhead along the ruler to step through the recipe. Because steps retain no state between frames, scrubbing backwards shows exactly the same frames as playing forwards.

[shot: S04-03 | The recipe editor's preview above the Timeline toolbar. Play, Stop and Loop on the left; the ruler for scrubbing on the right]

Two tracks of `HeavyHit` carry the label **No preview**: Global Hitstop, which slows the game's time, and Force Feedback Curve, which drives the controller. Both act only in a running game, which the next section starts.

## Play it from a Blueprint {#ch04_play}

A recipe plays when the game calls the **Play Feel** node. The following steps play `HeavyHit` on the player's character whenever the 1 key is pressed.

1. Open the character Blueprint, for example `BP_ThirdPersonCharacter`, and its **Event Graph**.
2. Right-click an empty area of the graph, type **1** and choose **Input > Keyboard Events > 1**.
3. Drag from the **Pressed** pin, type **Play Feel** and choose **Play Feel** from the **Feel** category.
4. On the Play Feel node, set **Recipe** to `HeavyHit`.
5. Drag from the **Target** pin and choose **Make Feel Target from Actor**. Right-click the graph, choose **Get a reference to self**, and connect it to the **Actor** pin.
6. Compile and save the Blueprint.

[shot: S04-04 | The Event Graph after step 5: the 1 key plays HeavyHit, with the character itself as the target]

Start Play In Editor and press 1. The recipe plays on the character: the camera kicks and shakes, the screen flashes, the character's mesh punches in scale, time freezes briefly and a connected controller rumbles.

[shot: S04-05 | The same view before the hit and 0.07 s after pressing 1, when the camera punch and the scale punch are at their strongest. Motion blur was turned off for this picture]

The target decides where each effect is applied ([see: ch02_targets]). Camera, screen and controller effects reach the player who owns the target, here the player controlling the character. Actor effects apply to the character; because a character's root is its collision capsule, FeelKit moves and scales the visible mesh instead, so collision and movement are never affected.

A keyboard event in the character Blueprint is a quick way to try a recipe. In a finished game, recipes are played from the game's own events, such as a hit being registered or a landing; [Ref: ch06] describes the options, including triggers that need no Blueprint at all.

## Compare with the feel off {#ch04_switch}

The **Feel Switch** turns FeelKit off and on while the game runs, so that the difference can be seen and felt directly. It is useful for tuning, for playtests and for showing a game to others.

1. Open **Window > Place Actors**, search for **Feel Switch** and drag it into the level.
2. Start Play In Editor. A start card explains the switch, and a **FEEL: ON** badge appears in the top right corner, with the key for the comfort settings below it.

[shot: S04-06 | The Feel Switch at the start of play: the start card and the FEEL: ON badge]

3. Press 1 to play the recipe, then press Tab (View / Share on a controller). The badge changes to **FEEL: OFF**, and pressing 1 now shows the game without FeelKit. Press Tab again to turn FeelKit back on.

[shot: S04-07 | The badge while FeelKit is turned off]

Turning the feel off stops only FeelKit: effects that the game itself plays are unaffected. Every play session starts with FeelKit on, and the choice is not saved. Pressing O (Menu / Options on a controller) opens the comfort settings menu, which [Ref: ch08] describes. The switch keys, the start card and the controls list are set in the Feel Switch's details.

## Where to go next {#ch04_next}

- [Ref: ch05] explains every part of the recipe editor, including how to build a recipe from an empty timeline.
- [Ref: ch06] shows how to play recipes from gameplay events, animations and triggers.
- [Ref: ch07] covers parameters, context and variation, which let one recipe serve light and heavy hits alike.
- [Ref: ch08] covers the comfort settings and the comfort menu for players.
- [Ref: ch16] lists every step with its settings.
