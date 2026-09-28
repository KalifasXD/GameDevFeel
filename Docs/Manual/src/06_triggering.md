# Playing recipes in your game {#ch06}

A recipe plays when something in the game asks for it. The simplest way is the **Play Feel** node. For larger games, FeelKit adds ways that keep recipe choices out of gameplay code: events that a Feel Map turns into recipes, markers in animations, and components that react to damage, landings, collisions or input without any Blueprint wiring. They can be combined freely in one project.

[video: V3]

| Method | Suits | Edition |
|---|---|---|
| **Play Feel** and its variants ([see: ch06_play]) | Any moment the game already handles in Blueprint or C++. | Lite and Pro |
| **Send Feel Event** with a Feel Map ([see: ch06_events]) | Games with many kinds of hits or actions, where designers choose the recipes. | Pro |
| Animation notifies ([see: ch06_notifies]) | Feedback tied to a frame of an animation, such as a sword swing or a footstep. | Pro |
| **Feel Trigger** component ([see: ch06_trigger]) | Damage, landings, jumps, collisions and overlaps, with no wiring. | Pro |
| **Feel Input** component ([see: ch06_input]) | Feedback on button presses, such as firing or charging. | Pro |
| Gameplay Cue notifies ([see: ch06_gas]) | Projects that use the Gameplay Ability System. | Pro |

## Play Feel, and choosing a target {#ch06_play}

**Play Feel** plays a recipe on a target, with an intensity that multiplies every track of that play (1 by default). It returns a handle that identifies this play ([see: ch06_handles]).

The target decides where each kind of effect goes ([see: ch02_targets]). Five nodes make one:

| Node | Plays on | Typical use |
|---|---|---|
| **Make Feel Target from Actor** | An actor. On a character, actor effects move and scale its visible mesh, never its collision capsule. | The player's character, an enemy, a door. |
| **Make Feel Target from Component** | One scene component, such as a weapon mesh. | Effects on part of an actor. |
| **Make Feel Target at Location** | A point in the world. | An explosion; sounds and distance checks use the point. |
| **Make Feel Target from Local Player Camera** | A local player's camera, by player index. | Camera and screen feedback that belongs to no actor. |
| **Make Feel Target from Widget** | A UMG widget. Widget steps apply to it. | Interface feedback such as a score counter punch. |

[shot: S06-01 | The five target nodes. Make Feel Target from Actor, given Self, feeds a Play Feel node]

Camera, screen and controller effects always go to the local player who owns the target: the player controlling the character, or the owner of the widget. A target that belongs to a player on another machine receives none of them on this machine; that player's own machine shows them. With no target at all, they go to the first local player.

**When a play does not start.** Play Feel returns an invalid handle, and nothing plays, when:

- no recipe is connected;
- the recipe's **Cooldown** has not passed, or its **Max Concurrent** limit is reached on this target;
- FeelKit is turned off (the Feel Switch, **Set Feel Enabled** or `feel.Enabled 0`);
- the game runs as a dedicated server, which has no players to show feedback to;
- the target actor or component no longer exists (this case also logs a warning).

## Passing context with Play Feel with Context {#ch06_context}
[edition: Pro]

**Play Feel with Context** plays a recipe like Play Feel, with extra information about this play in a **Feel Play Context**. Build the context with **Make FeelPlayContext**; every field is optional, and an empty context plays exactly like Play Feel.

| Field | Used for |
|---|---|
| **Parameters** | Values for the recipe's parameters, by name, for example `Damage` 50. Tracks with parameter mappings scale by them ([Ref: ch07_parameters]). |
| **Instigator** | A second actor, such as the attacker. Tracks set to **Applies To** Instigator play their actor effects on it ([Ref: ch07_instigator]). |
| **Direction**, **Location**, **Normal** | Details of the event, such as the direction of a hit. Steps such as Camera Punch can take their direction from it ([Ref: ch07_directions]). |
| **Context Tags** | Gameplay tags that describe the circumstances. Feel Maps use them to choose a recipe ([see: ch06_events]). |

[shot: S06-02 | Play Feel with Context. Make FeelPlayContext passes the parameter Damage 50 (from a Make Map node) and the actor itself as Instigator]

The library recipe `FR_Impact_ScalableHit` is built for this: its tracks are mapped to a `Damage` parameter from 0 to 100, so one recipe covers light and heavy hits.

## Handles: stop, release, change a parameter {#ch06_handles}

The handle that Play Feel returns refers to one play of a recipe. Store it in a variable when the play needs to be controlled later.

| Node | What it does | Edition |
|---|---|---|
| **Stop Feel** | Stops the play. With **Blend Out** on (the default), its effects fade over the project's **Blend Out Time** (0.2 s). | Lite and Pro |
| **Stop All Feel** | Stops every play, or only the plays on one actor. | Lite and Pro |
| **Release Feel** | Ends the sustain loop of a sustained recipe, so that it plays its ending ([Ref: ch07_sustain]). | **Pro.** |
| **Set Feel Parameter** | Changes a parameter of the play while it runs, for example a charge that grows while a button is held. | **Pro.** |
| **Is Feel Playing** | Whether the play is still running, including while it fades out. | Lite and Pro |
| **Is Valid (Feel Handle)** | Whether the handle came from a play that started. The play may have finished since. | Lite and Pro |

The **Parameter Name** pin of **Set Feel Parameter** is a dropdown. When its handle comes from a Play Feel node in the same graph, it lists the parameters of that node's recipe; otherwise it lists every recipe parameter in the project.

[shot: S06-03 | One handle from Play Feel used by Set Feel Parameter (the dropdown shows the recipe's Charge parameter) and by Release Feel]

A handle stays safe to use after its play ends: the nodes then do nothing, and **Is Feel Playing** returns false.

## Play Feel and Wait {#ch06_wait}
[edition: Pro]

**Play Feel and Wait** plays a recipe and continues when it ends, which suits sequences such as a door that opens only once its feedback has played. It has two output execution pins:

- **On Finished** fires when the recipe plays to its end.
- **On Cancelled** fires when the play is stopped early, its target is destroyed, or it could not start.

[shot: S06-04 | Play Feel and Wait with its On Finished and On Cancelled pins]

A sustained recipe finishes only after it is released, so pair it with **Release Feel** or use a recipe without sustain.

## Events and Feel Maps {#ch06_events}
[edition: Pro]

With **Send Feel Event**, gameplay code sends what happened, as a gameplay tag, and a **Feel Map** decides which recipe plays. Recipes can then be changed, and variants added, without touching gameplay code. [Ref: ch02_events] introduces the idea.

**Creating a Feel Map.**

1. In the Content Browser, right-click and choose **FeelKit** > **Feel Map**. Name it and open it.
2. Under **Entries**, add one row per event. Each row has an **Event**, the **Recipe** to play, and optionally **Required Tags**, an **Intensity Scale** (1) and a **Priority** (0).
3. Open **Edit** > **Project Settings** > **Plugins** > **FeelKit**, and under **Events** add the map to **Feel Maps**.

[shot: S06-05 | The Feel Map of the Action/RPG demo, with one row per event]

[shot: S06-06 | Project Settings > Plugins > FeelKit > Events, with the Feel Map in Feel Maps]

**Sending an event.** Call **Send Feel Event** with the event tag, a target and, optionally, a context. FeelKit defines four starter events, which a project can use or extend with its own tags:

| Event | Meaning |
|---|---|
| `Feel.Event.Hit.Landed` | An attack connected. Usually sent on the attacker, with the hit location. |
| `Feel.Event.Hit.Received` | Something was hit. Usually sent on what was hit, with the attacker as instigator and the hit location and direction. |
| `Feel.Event.Hurt` | The player was hurt. Usually sent on the player, with the direction the damage came from. |
| `Feel.Event.Death` | Something died or was destroyed. Usually sent on what died. |

**How a row is chosen.** A row answers its event and the event's children: a row for `Feel.Event.Hit` also answers `Feel.Event.Hit.Landed` when no row is more specific. The context must contain all of the row's **Required Tags**. Among the rows that match, a more specific event wins, then the row with more required tags, then the higher **Priority**, then the earlier row. The row's **Intensity Scale** multiplies the intensity of the play.

For example, a map with a row for `Feel.Event.Hit.Landed` that plays a light hit, and a second row for the same event with the required tag `Hit.Heavy` that plays a heavy hit, plays the heavy hit only when the context carries `Hit.Heavy`. Tags such as `Hit.Heavy` are the project's own: add them under **Project Settings** > **Gameplay Tags**.

**Maps on an actor.** A **Feel Trigger** component can hold Feel Maps of its own ([see: ch06_trigger]). When an event targets its actor, those maps are checked first, so that one enemy type can sound and feel different from another.

When no row matches, nothing plays, and a warning in the Output Log names the event, once per event.

## Animation notifies {#ch06_notifies}
[edition: Pro]

Animation notifies play feedback on an exact frame of an animation, such as the moment a sword connects or a foot lands. To add one, open the animation or montage, right-click the **Notifies** track, and choose **Add Notify** or **Add Notify State**, then the FeelKit notify:

| Notify | What it does |
|---|---|
| **Play Feel** | Plays a recipe on that frame. |
| **Send Feel Event** | Sends a Feel Event on that frame; the Feel Maps choose the recipe. |
| **Set Feel Value** | Sets or adds to accumulators on that frame, for example how heavy the current swing is ([Ref: ch07_accumulators]). |
| **Play Feel (Window)** | A notify state: plays a recipe for the length of the window. At its end a sustained recipe is released, and any other recipe is stopped unless **Let Non Sustained Recipe Finish** is on. |

The notifies share **Target** (the skeletal mesh by default, or the owning actor), **Intensity**, **Parameters** and **Context Tags**.

[shot: S06-07 | The Notifies track of the Action/RPG combo montage: Set Feel Value markers ("Feel Value: Swing 0.35") and Play Feel markers ("Feel: FR_ARPG_Swing")]

FeelKit's notifies also play in the animation editor's preview, so actor effects such as a scale punch can be tuned while scrubbing the animation. Camera, time and controller effects need a running game.

## The Feel Trigger component {#ch06_trigger}
[edition: Pro]

The **Feel Trigger** component plays recipes when common things happen to its owner, with no Blueprint wiring. Add it to an actor, and add one entry to **Triggers** for each reaction.

| Event | Fires when | Event value |
|---|---|---|
| **Take Any Damage** | The owner receives damage. | The damage amount. |
| **Landed** | The owner, a Character, lands after falling. | The downward speed at landing, in cm/s. |
| **Component Hit** | The owner's root component reports a blocking hit. | The strength of the hit's impulse. |
| **Begin Overlap**, **End Overlap** | Another actor starts or stops overlapping the owner. | 0 |
| **Jumped** | The owner, a Character, jumps from the ground. | 1 |
| **Air Jumped** | The owner jumps again in the air. | The jump number, 2 or more. |
| **Launched** | The owner is launched, for example by a jump pad or a wall jump. | The launch speed, in cm/s. |

Each entry plays a **Recipe**, or sends a **Feel Event** when no recipe is set. **Value Parameter** passes the event value to a recipe parameter, so a harder landing plays a stronger recipe. The other fields choose where the recipe plays: **Target Skeletal Mesh** (on) plays on the owner's mesh rather than its capsule; **Play On** chooses the owner or the other actor of the event; **Other Actor as Instigator** (on) passes the other actor as the instigator; **Only for Players** (off) ignores events caused by anything but a player.

[shot: S06-08 | The Feel Trigger of the Platformer demo's character: jump, air jump, launch and landing]

The Platformer demo uses four entries: **Jumped** plays `FR_PLAT_Jump`, **Air Jumped** plays `FR_PLAT_AirJump` with the jump number as `JumpNumber`, **Launched** plays `FR_PLAT_WallJump` with the launch speed as `LaunchSpeed`, and **Landed** plays `FR_PLAT_Land` with the landing speed as `LandSpeed`.

**Fire Event** fires the entries of an event from Blueprint, for events that the component cannot detect itself. The component ticks only when an entry uses **Jumped**, **Air Jumped** or **Launched**; the other events are bound once, when play begins.

## Enhanced Input: the Feel Input component {#ch06_input}
[edition: Pro]

The **Feel Input** component plays recipes from Enhanced Input actions. Add it to a pawn or a player controller, and add one entry to **Bindings** for each action:

| Field | Purpose |
|---|---|
| **Action** | The input action. It must be in an input mapping context that the player has added. |
| **Play On** | **Started**, **Triggered**, **Completed** or **Canceled**, as in Enhanced Input. |
| **Recipe** or **Event** | What to play. |
| **Intensity**, **Scale Intensity by Value** | The intensity, optionally multiplied by how far the input is pressed. |
| **Value Parameter** | A recipe parameter that receives the input's value. |
| **End when Input Ends** | Ends the play when the input is released: a sustained recipe plays its ending, others fade out. Suits a charge that lasts while a button is held. |
| **Once Per Press** | Plays at most once per press, even when the action fires every frame. |

[shot: S06-09 | A Feel Input binding: the charged attack action plays FR_Power_ChargeUp while it is held, once per press]

The recipes play on the component's owner. **Handle Input** simulates an input for testing.

## Gameplay Ability System cues {#ch06_gas}
[edition: Pro]

With the GAS add-on installed ([Ref: ch03_gas]), Gameplay Cues can play recipes. Create a Blueprint class with one of these parents and set its **Gameplay Cue Tag** as for any cue notify:

- **Feel Gameplay Cue Notify** plays once, when the cue is executed or added. It suits hits, pickups and other one-shot moments.
- **Feel Gameplay Cue Notify (Actor)** plays while the cue is active: it starts when the cue is added and ends when it is removed. With a sustained recipe, the feedback loops for as long as the gameplay effect lasts. **Stop on Remove** stops the play at once instead of letting it play its ending.

Both play a **Recipe**, or send an **Event**; with neither set, they send the cue's own tag as the event, so Feel Maps can answer cues directly. **Scale Intensity by Magnitude** scales the play by the cue's normalized magnitude, and the two magnitude parameters pass the raw and normalized magnitudes to recipe parameters. **Effect Causer as Instigator** uses the effect causer, such as a projectile, as the instigator.

When a cue is executed from Blueprint, connect a **Make GameplayCueParameters** node to its **Parameters** pin. The node cannot compile with that pin left empty.

## Using FeelKit from C++ {#ch06_cpp}

Add `FeelCore` to the dependencies of the game module in its `.Build.cs` file:

```
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "FeelCore" });
```

Plays go through the world's Feel Subsystem:

```
#include "FeelSubsystem.h"

if (UFeelSubsystem* Feel = UFeelSubsystem::Get(this))
{
    HitHandle = Feel->PlayFeel(HitRecipe, FFeelTarget::FromActor(this), 1.0f);
}
```

Here `HitRecipe` is a `UFeelRecipe*` property set in the editor and `HitHandle` an `FFeelHandle` member, kept to stop or change the play later. `FFeelTarget::FromActor`, `FromComponent`, `AtLocation`, `FromLocalPlayerCamera` and `FromWidget` match the Make Feel Target nodes. Every Blueprint node is also a static function of `UFeelBlueprintLibrary`. In Pro, the subsystem also broadcasts `OnFeelStarted` and `OnFeelFinished` for code that reacts to plays. The Feel Input component needs the module `FeelEnhancedInput`, and the GAS cue notifies need `FeelGAS`. [Ref: ch17] lists the subsystem's functions.

## The Feel Switch: feel off and on while playing {#ch06_switch}

The **Feel Switch** is an actor that lets anyone playing the game turn FeelKit off and on, to see and feel what it adds. [Ref: ch04_switch] shows it in use. Place one in a level; it needs no input assets and no Blueprint changes.

| Setting | Default | Purpose |
|---|---|---|
| **Switch Keys** | Tab, controller View / Share | Keys that turn FeelKit off and on. |
| **Start Enabled** | On | Whether FeelKit is on when the level starts. The choice is never saved. |
| **Show Built in Display** | On | The start card and the corner badge. Turn off to show your own display from the **On Feel Switched** event. |
| **Show Start Card**, **Start Card Title**, **Start Card Text**, **Start Card Seconds** | On, "FeelKit", a short explanation, 8 s | The card that explains the switch when the level starts. |
| **Controls** | empty | The level's controls, listed in a small panel under the badge. |
| **Comfort Menu Keys** | O, controller Menu / Options | Keys that open and close the players' comfort menu ([Ref: ch08]). They are added to the controls panel. |
| **Comfort Menu Class**, **Pause in Comfort Menu** | Project setting, On | Which menu opens, and whether the game pauses while it is open. |

[shot: S06-10 | The Feel Switch's settings in Details, at their defaults]

Turning FeelKit off stops every playing recipe at once, returns slowed time to normal, and blocks new plays until it is turned on again. The game's own camera shakes, sounds and effects are unaffected. The switch applies to the whole game on the machine, so every Play In Editor window switches together. A second Feel Switch in the same level logs a warning and does nothing.

The same switch is available as nodes, with or without a Feel Switch in the level: **Set Feel Enabled**, **Toggle Feel** and **Is Feel Enabled**. They change the same value as the console variable `feel.Enabled` ([Ref: ch19]).
