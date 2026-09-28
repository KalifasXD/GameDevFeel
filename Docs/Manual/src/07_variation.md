# Parameters, context and variation {#ch07}
[edition: Pro]

A fixed recipe plays the same way every time. Parameters, accumulators and the play context let one recipe respond to what happened: how hard the hit was, how far away the explosion is, how long the combo has lasted, which way the blow came from. Random ranges, conditions and Random Choice add controlled variation, so that repeated plays do not look mechanical. All of these features are part of FeelKit Pro.

[video: V2]

## Parameters and track mappings {#ch07_parameters}

A **parameter** is a named number that the game passes each time a recipe plays, such as the damage of a hit. Tracks read it through **parameter mappings**, which turn the parameter into a multiplier for the track's intensity.

**Declaring a parameter.** In the recipe editor, with no track selected, add an entry to **Parameters** in **Details**:

| Field | Default | Purpose |
|---|---|---|
| **Name** | None | The name the game uses, such as `Damage`. |
| **Default Value** | 0 | The value used when the game passes none. |
| **Min Value**, **Max Value** | 0, 1 | The meaningful range, in the game's own units. Values outside it are clamped. |
| **Accumulator** | None | An accumulator to read when the game passes no value ([see: ch07_accumulators]). |
| **Description** | empty | What the value means, shown as the tooltip of the preview slider. |

**Mapping a track to it.** Select a track and add an entry to **Parameter Mappings**: choose the **Parameter** and, optionally, a **Curve**. The curve's horizontal axis is the parameter from its minimum (0) to its maximum (1); its vertical axis is the multiplier. The default straight line from 0 to 1 makes the track grow in proportion to the parameter. A curve that starts at 0.3, for example, keeps a light hit at 30 percent strength instead of silencing it. Several mappings on one track multiply together, and they combine with the track's intensity curve.

[shot: S07-01 | A copy of FR_Impact_ScalableHit: the Damage slider under the Timeline toolbar, the Damage parameter, and the selected track's Parameter Mappings]

**Passing the value.** The game passes values in the **Parameters** of a **Feel Play Context** ([Ref: ch06_context]), from an animation notify's **Parameters**, or from a Feel Trigger entry's **Value Parameter** ([Ref: ch06_trigger]). **Set Feel Parameter** changes a value while the recipe plays ([Ref: ch06_handles]).

**Previewing.** The recipe editor shows a slider for each parameter under **Preview parameters:**, so the whole range can be tuned without the game ([Ref: ch05_preview]).

**Example.** `FR_Impact_ScalableHit` declares `Damage` from 0 to 100 (default 50), and maps its shake, punch, flash and rumble to it: one recipe covers a graze and a critical blow. `FR_Weight_Land` declares `FallSpeed` from 0 to 1600 cm/s; a Feel Trigger entry for **Landed** passes the landing speed to it.

## The Distance parameter {#ch07_distance}

A parameter named `Distance` is filled in automatically when the game does not pass it: FeelKit sets it to the distance, in centimeters, from the play's target to the nearest local player's camera. Map tracks to it with a curve that falls from 1 to 0 to make an effect weaken with distance.

`FR_Power_Explosion` uses it: its flash, camera punch, shake, chromatic aberration, muffled sound and rumble are strong close to the blast and fade with distance. For a hard cut-off instead, use the **Max Distance** condition of a track ([see: ch07_random]).

## Accumulators: effects that build up with repeated plays {#ch07_accumulators}

An **accumulator** is a named value that builds up as the game adds to it and decays over time, such as a combo counter or heat that rises with each shot. Recipes read it through a parameter, so feedback grows with a streak without any bookkeeping in the game.

**Defining an accumulator.** Add an entry to **Project Settings** > **Plugins** > **FeelKit** > **Playback** > **Accumulators**:

| Field | Default | Purpose |
|---|---|---|
| **Name** | None | The name that nodes and parameters use. |
| **Max Value** | 1 | The value never rises above it. |
| **Decay Per Second** | 1 | How fast it falls back toward 0. 0 means it never decays. |
| **Decay Delay** | 0 s | How long after the last change it starts to decay. |

[shot: S07-02 | Accumulators in Project Settings: Combo, with a maximum of 10, a decay of 1 per second and a delay of 1.5 s]

**Changing it.** The nodes **Add to Feel Accumulator**, **Set Feel Accumulator** and **Get Feel Accumulator** change and read a value. Their **Actor** pin makes the value belong to one actor, such as each enemy's own heat; left empty, the value is global. Their name pins are dropdowns listing the project's accumulators. The **Set Feel Value** animation notify sets or adds to accumulators from an animation ([Ref: ch06_notifies]).

**Reading it.** Set a recipe parameter's **Accumulator** field. While the recipe plays, the parameter reads the target's own value, else the value of the play's instigator, else the global value, every frame. A value the game passes directly in the play context takes precedence.

**Example.** The Action/RPG demo defines `Combo` (maximum 10, decay 1 per second after 1.5 s), and the player's Blueprint adds 1 to the player's own value on every hit. `FR_ARPG_EnemyHurt` plays on the enemy that was hit; its parameter `Streak` reads `Combo` and, since the enemy has no value of its own, finds the player's through the instigator, so a long combo hits harder. The combo animations set the accumulator `Swing` to 0.35, 0.65 and 1.0 on their three attacks with **Set Feel Value** notifies, and the hit recipes read it, so the third attack of a combo lands hardest. The library recipes `FR_Reward_ComboStep` and `FR_Interface_ScoreTick` read `Combo` in the same way ([Ref: ch09_recipes]).

A parameter that names an accumulator the project does not define plays with its default value, and saving the recipe reports it.

## Sustained recipes {#ch07_sustain}

A sustained recipe keeps playing for as long as a condition lasts: a charge held down, an alarm, low health. In **Details**, turn on **Sustain** and set **Sustain Start** and **Sustain End**. When the play reaches **Sustain End**, time returns to **Sustain Start** and the region loops. When the play is released, it continues past the region and plays the rest of the recipe, its ending.

A sustained play is released by:

- **Release Feel** on its handle ([Ref: ch06_handles]);
- the end of a **Play Feel (Window)** notify state ([Ref: ch06_notifies]);
- a Feel Input binding with **End when Input Ends**, when the input is released ([Ref: ch06_input]);
- the removal of its Gameplay Cue, with a **Feel Gameplay Cue Notify (Actor)** ([Ref: ch06_gas]).

**Stop Feel** ends it at once, without the ending. The recipe editor loops the region while previewing and has a **Release** button ([Ref: ch05_preview]).

**Example.** `FR_Power_ChargeUp` loops from 0.35 s to 0.95 s. Combined with **Set Feel Parameter** on its `Charge` parameter, the hum, the tightening view and the rumble grow while the charge is held, and the release plays its ending. Seven library recipes are sustained ([Ref: ch09_recipes]).

## The instigator, and the Applies To setting {#ch07_instigator}

Many moments involve two actors: the attacker and the one hit, the player and the pickup. The play's target is one of them; the **Instigator** in the play context can be the other ([Ref: ch06_context]). A Feel Trigger passes the other actor of the event as the instigator by default.

Each track chooses which of the two its actor effects apply to, with **Applies To** in the **Track** category:

- **Play Target** (the default): actor effects such as a scale punch or a material flash apply to the target.
- **Instigator**: they apply to the instigator instead. Camera, screen and controller effects go to the local player of that actor. A play without an instigator skips the track, and the recipe editor labels it **Skipped: the play had no instigator**.

**Example.** `FR_ARPG_Parry` plays on the parrying player. Its Hit Flash and Actor Hitstop tracks are set to Instigator, so the attacker whose blow was parried flashes and freezes, from the same recipe.

## Directions and locations {#ch07_directions}

The play context can carry a **Direction**, a **Location** and a surface **Normal**. Some steps use them:

| Step | Uses |
|---|---|
| **Camera Punch** | **Direction Source**: **Step Settings** (the punch as set on the step), **Play Direction**, **Away From Play Location** or **Toward Play Location**. The punch keeps its strength and points along that direction, as seen from the camera when the play started. |
| **Spawn Decal** | Appears at the play's **Location**, projects along its **Normal** when given. |
| **Spawn Particle** | Appears at the play's **Location**; **Orient to Normal** aligns the system to the **Normal**. |
| **Number Pop** | Appears at the play's **Location**, or at the target. |
| **Blueprint Event**, steps written in Blueprint | Can read the whole context. |

Each step falls back to its own settings or to the target when the play passes no direction or location. `FR_Danger_DirectionalDamage` uses **Away From Play Location**: passing the attacker's location pushes the camera away from where the damage came from.

## Randomness, conditions and Random Choice {#ch07_random}

**Random ranges.** The **Randomness** category of a track holds **Random Intensity** and **Random Duration Scale**, each a range from **Min** to **Max**. Every play picks a value in each range: 0.8 to 1.2 makes each play between 80 and 120 percent as strong. 1 to 1, the default, means no variation. Steps with noise, such as Procedural Shake, also change pattern with the track's **Seed**, and Camera Punch has a **Direction Jitter**.

**Conditions.** The **Conditions** category decides, once per play, whether the track plays at all:

| Condition | Default | The track plays only when |
|---|---|---|
| **Local Player Only** | Off | The target belongs to a player on this machine. |
| **Max Distance** | 0 (no limit) | The target is within this distance of the local camera. |
| **Chance** | 1 | A random roll succeeds: 0.5 plays about every second time. |
| **Platforms** | empty (all) | The game runs on one of the listed platforms, by Unreal's platform names, such as Windows or PS5. |

The rolls are made when the play starts and kept for its whole length, so scrubbing in the editor shows the same result. A track skipped by a condition says why on its bar, for example **Skipped this play (chance 50%)**.

**Random Choice.** The **Random Choice** step plays one of several steps, picked for each play by weight: an option with weight 2 is picked twice as often as one with weight 1. Use it for variety within one track, such as three different impact sounds or two shake patterns. The pick is fixed for the play; the recipe editor labels the bar with the option picked for the preview.

[shot: S07-03 | A Random Choice track with three Play Sound options and their weights]

## Recipes inside recipes {#ch07_nested}

The **Play Recipe** step plays another recipe inside a track, with the same target, context and parameters. **Intensity Scale** multiplies the inner recipe on top of the track's intensity, and the inner recipe's tracks keep their own channels and comfort settings. Give the track at least the length of the inner recipe, which stops when the track ends.

Nesting lets common pieces be shared: one impact recipe used inside several weapon recipes, changed in one place. Recipes nest up to four deep, and a recipe cannot play itself; saving reports both cases. The recipe editor previews the inner recipe exactly.
