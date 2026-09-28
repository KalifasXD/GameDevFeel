# Core ideas {#ch02}

## Recipes, tracks and steps {#ch02_recipes}

A **recipe** is an asset that defines the feedback for a single moment of play: a hit landing, a heavy landing after a fall, a pickup, a menu button being pressed. A recipe consists of **tracks** arranged on a timeline, in the same manner that a Level Sequence arranges tracks in Sequencer. Each track plays one **step**, which is the effect itself: a camera shake, a screen flash, a sound, a controller vibration or a brief freeze of time. FeelKit Pro has 37 steps and FeelKit Lite has 12 of them; [ref: ch16] describes each step and names its edition.

[shot: S02-01 | The recipe editor on a copy of FR_Impact_HeavyHit. 1 the steps, one per track; 2 a track on the timeline; 3 its channel, by name and color; 4 the playhead]

When a recipe plays, each track begins at its **Start Time** and runs its step for its **Duration**, both measured in seconds. A new track lasts 0.5 s. A track with a length of 0 is an instant track: steps that act once, such as Play Sound, Spawn Decal or Blueprint Event, can sit on one. The play ends when its last track ends.

A few settings belong to the recipe as a whole:

- **Cooldown** (default 0 s): how long the same recipe must wait before it can play again on the same target.
- **Max Concurrent** (default 0, which means no limit): how many plays of the recipe can run at once on one target. When the limit is reached, a new play does not start and the running ones continue.
- **Default Intensity** (default 1): a multiplier on every play of the recipe.

The full list of recipe and track fields is in [ref: ref_recipe_fields] and [ref: ref_track_fields].

A step computes its output for a given moment from three inputs only: the time within its track, its intensity at that time, and a seed. It retains no state from earlier frames. This property allows the recipe editor to scrub the timeline in either direction and show exactly the frame the game would show at that moment, and it guarantees that the same seed always produces the same shake. Steps that act once, such as playing a sound or calling a Blueprint event, do so when their track starts, and some act again when it stops.

Recipes are timed in real time, which Unreal refers to as unscaled time. Consequently, a hitstop that slows the world does not slow the recipe that caused it: a shake continues to move during the freeze, and the recipe ends when its timeline specifies. FeelKit's runtime performs work only while a recipe plays or while a change it made still has to be restored; when nothing is playing, it does not tick at all.

## Channels, and what happens when effects overlap {#ch02_channels}

Every track belongs to a **channel**, a gameplay tag under `Feel`. A new track receives its step's default channel, for example `Feel.Camera.Motion` for Camera Punch, and you can change it on the track. The channel colors the track on the timeline, groups the recipe editor's **Intensity** tab, and decides which comfort group scales the track ([see: ch02_comfort]).

| Channel | Used by | Comfort group |
|---|---|---|
| `Feel.Camera.Shake` | Procedural Shake | Camera Shake |
| `Feel.Camera.Motion` | Camera Punch, FOV Kick, Camera Roll, Camera Zoom, Look-at Nudge | Camera Motion |
| `Feel.Time.Hitstop` | Global Hitstop, Actor Hitstop | Hitstop and Slow-mo |
| `Feel.Time.SlowMo` | Slow-mo Ramp | Hitstop and Slow-mo |
| `Feel.Screen.Flash` | Screen Flash | Flashes |
| `Feel.Screen.Distortion` | Vignette Pulse, Chromatic Aberration, Post Process Material Pulse | Screen Distortion |
| `Feel.Screen.Color` | Desaturate, Color Tint | Screen Distortion |
| `Feel.Screen.Fade` | Screen Fade | none (Master only) |
| `Feel.Actor.Transform` | Scale Punch, Squash and Stretch, Mesh Wobble | none (Master only) |
| `Feel.Actor.Material` | Material Parameter Pulse, Hit Flash | none (Master only) |
| `Feel.Actor.Light` | Light Flash | Flashes |
| `Feel.Audio` | Play Sound | none (Master only) |
| `Feel.Audio.Mix` | Sound Class Duck, Pitch Bend, Low-pass Sweep | none (Master only) |
| `Feel.Haptics` | Force Feedback Curve, Haptic Pattern | Haptics |
| `Feel.UI` | Widget Punch, Widget Shake, Widget Flash, Number Pop | none (Master only) |
| `Feel.Spawn` | Spawn Decal, Spawn Particle | none (Master only) |
| `Feel.Meta.Event` | Blueprint Event | none (Master only) |
| `Feel.Meta.Recipe` | Play Recipe, Random Choice | none (Master only) |

The comfort groups in this table are the project defaults. You can change the mapping in the project settings ([see: set_comfort]).

### Inside one play {#ch02_overlap_inside}

The tracks of a single play are combined before any output reaches the screen. Movement adds up: camera location and rotation offsets, field of view changes, and the scale and motion of the target are summed across the play's tracks, so two camera tracks placed on top of each other move the camera by both. Everything else takes the strongest track: the strongest flash, fade and tint, the strongest weight for each post-process value, and the strongest value for each controller motor.

### Between separate plays {#ch02_overlap_between}

When several plays run concurrently, for example two hits a few frames apart, FeelKit resolves their results separately for each kind of output:

| Output | Rule |
|---|---|
| Camera location, rotation and field of view of one player | **Strongest Wins** by default: for each of the three, the strongest play wins. With **Additive Capped** the plays add up and the sum is limited by caps (30 cm, 8° and 15° by default). Set in the project settings ([see: set_camera]). |
| Time dilation of the world or of one actor | One request wins per clock: the highest **Priority**, and among equal priorities the slowest dilation. When no request remains, the clock gets back the dilation it had before FeelKit changed it. |
| Flash, fade, tint and each post-process value | The strongest wins. |
| Controller vibration of one player | The strongest value wins for each motor. |
| Scale and motion of one component | Changes add up. |
| Material parameters | Scalar changes add up; for colors the strongest wins. |
| Sound classes | The lowest volume, the pitch farthest from normal and the lowest filter cutoff win. |
| Widgets | Movement, scale and rotation add up; for the tint the strongest wins. |

Under Strongest Wins, two plays that push the camera in different directions do not blend: whichever is stronger in a given frame determines that frame. If two camera recipes often overlap and aim different ways, either give one of them less camera motion or switch to Additive Capped.

## Intensity: how strong each effect plays {#ch02_intensity}

At every moment, each track has an intensity, and its step scales the effect by that value. A Camera Punch with a **Location Punch** of 10 cm moves the camera 10 cm at intensity 1 and 5 cm at intensity 0.5. Intensity above 1 is allowed and makes most effects larger; values that cannot exceed a limit, such as a flash's opacity or a blend weight, are clamped.

A track's intensity is the product of the following factors:

| Part | Where it comes from | Default |
|---|---|---|
| Call intensity | The **Intensity** pin of Play Feel and the other play nodes. A Feel Map row multiplies it by its **Intensity Scale**. | 1 |
| Default intensity | The recipe's **Default Intensity**. | 1 |
| Intensity curve | The track's **Intensity Curve**, read over the track from 0 at its start to 1 at its end. | A new track fades from 1 to 0; tracks of steps that shape their own motion start flat at 1. An empty curve counts as 1. |
| Parameter mappings | One multiplier for each of the track's parameter mappings ([see: ch02_parameters]). | none |
| Random intensity | A value between the track's **Random Intensity** Min and Max, picked once per play. | 1 |
| Comfort | The player's scale for the track's comfort group, times their Master scale ([see: ch02_comfort]). | 1 |
| Global scale | The console variable `feel.GlobalScale` ([see: con_feel_globalscale]). | 1 |

Two more factors apply only at times: during a blend out after **Stop Feel** the intensity fades to 0 over **Blend Out Time**, and a flash above the rate the flash limiter allows is softened or suppressed.

[shot: S02-02 | The Intensity tab of FR_Impact_ScalableHit, one line per channel]

The recipe editor's **Intensity** tab draws the combined intensity of each channel across the recipe, with the preview's comfort setting applied.

## Targets: what a recipe plays on {#ch02_targets}

Every play has a **target**, created with one of the Make Feel Target nodes ([see: ref_nodes_targets]):

| Target | Made with | What it is |
|---|---|---|
| Actor | Make Feel Target from Actor | An actor in the level, such as a character, an enemy or a prop. |
| Scene Component | Make Feel Target from Component | One component, such as a weapon mesh. |
| World Location | Make Feel Target at Location | A point in the world, for effects with no actor, such as an explosion. |
| Local Player Camera | Make Feel Target from Local Player Camera | The camera of a local player, by player index. |
| Widget | Make Feel Target from Widget | A UMG widget, for UI steps. |
| None | the default Feel Target | Nothing in particular. |

The target determines where each kind of effect is applied:

- **Camera, screen and controller effects** go to the local player the target belongs to: that player's own pawn, controller, or something they own. A target that belongs to no player, such as an enemy or a prop, sends them to the first local player. A target that belongs to a player on another machine in a networked game sends them nowhere on this machine; that player's own machine shows them.
- **Actor effects** (scale, motion, materials, lights, actor time, attached sounds and Blueprint events) apply to the target actor or component. On a character, whose root is a collision capsule, motion and scale go to the visible mesh instead, so collision and movement never change.
- **Widget steps** apply only to a widget target.
- **Spawned effects** appear at the target's location, or at a location passed with the play.

In FeelKit Pro, a play can also carry a second actor, the **instigator**, for example the player whose attack caused the hit. Tracks whose **Applies To** is set to Instigator play their actor effects on that actor, and send their camera, screen and controller effects to that actor's player. One recipe can then shake the enemy that was hit and kick the camera of the player who hit it. Tracks set to Instigator are skipped when the play has no instigator. The instigator is passed in the play context ([see: ref_play_context]).

## Parameters, accumulators and sustained recipes, in brief {#ch02_parameters}

[edition: Pro]

These three features allow a single recipe to serve many situations. [Ref: ch07] explains them in full.

**Parameters** are named numbers a recipe accepts each time it plays, such as Damage or Speed. The recipe declares each one with a **Default Value**, a **Min Value** and a **Max Value**; the game passes values with **Play Feel with Context**. A track follows a parameter through a **parameter mapping**: a curve that turns the parameter, read from 0 at its minimum to 1 at its maximum, into an intensity multiplier. The library recipe `FR_Impact_ScalableHit` declares Damage from 0 to 100 (default 50). Most of its tracks play at 30 to 40% of their strength at Damage 0 and at full strength at Damage 100, while its flash stays faint below Damage 60. One recipe then covers light and heavy hits. A parameter named **Distance** is special: when the game does not pass it, FeelKit fills in the distance in centimeters from the target to the nearest local player camera. **Set Feel Parameter** changes a parameter while the recipe plays.

**Accumulators** are named values that build up each time the game adds to them and fall back toward 0 over time, defined in the project settings ([see: set_playback]) with a **Max Value**, a **Decay Per Second** and a **Decay Delay**. A recipe parameter can read an accumulator whenever the game does not pass the parameter itself: FeelKit looks for the value stored for the play's target, then for its instigator, then for the global value. Repeated plays then grow stronger while they come quickly and calm down when they stop. The library recipe `FR_Reward_ComboStep` has a Combo parameter from 0 to 10 that reads an accumulator named Combo. FeelKit defines no accumulators, so add Combo to the project settings before using that recipe.

A **sustained** recipe keeps playing while a condition lasts. It has a looping region between **Sustain Start** and **Sustain End**: time loops there until the play is released, then the rest of the recipe plays as its ending. The library recipe `FR_Power_ChargeUp` loops between 0.35 s and 0.95 s while a charge is held. **Release Feel** releases a play, and so do the end of a **Play Feel (Window)** notify and a Feel Input binding set to end with its input. **Stop Feel** stops a play whether it loops or not.

## Events and Feel Maps, in brief {#ch02_events}

[edition: Pro]

Gameplay code need not refer to recipes directly. It can instead send a **Feel Event**: a gameplay tag that says what happened, such as `Feel.Event.Hit.Landed`, with a target and a play context. A **Feel Map** asset then decides which recipe plays for that event. Changing how a hit feels therefore becomes a change to a single Feel Map row, while the gameplay code remains unchanged.

[shot: D02-01 | Diagram: a Feel Event goes to a Feel Map, the map picks a recipe, and the recipe's tracks reach the camera, the screen, the actor, audio, the controller and the UI]

Each row of a Feel Map names an **Event**, optional **Required Tags**, the **Recipe** to play, an **Intensity Scale** and a **Priority**. A row answers its event and every event below it, and applies only when the play context's **Context Tags** contain all its required tags. When several rows match, the row with the more specific event wins, then the one with more required tags, then the higher priority, then the row listed first. Two rows for the same event can then pick a heavy or a light variant from a context tag, with no branching in Blueprint.

FeelKit reads the Feel Maps listed in the project settings ([see: set_events]). A Feel Trigger component on the target can hold more maps, which are checked first. FeelKit defines four starter events: `Feel.Event.Hit.Landed`, `Feel.Event.Hit.Received`, `Feel.Event.Hurt` and `Feel.Event.Death`; projects add their own tags as needed. When no row matches an event, nothing plays and a warning in the log names the event.

[shot: S02-03 | A Feel Map in its editor with four rows for Feel.Event.Hit.Landed and Feel.Event.Hit.Received, two of them with Required Tags]

Events are sent by the **Send Feel Event** node, and can also come from animation notifies, the Feel Trigger component, the Feel Input component and gameplay cues. [Ref: ch06_events] shows each of these.

## Comfort on one page {#ch02_comfort}

Screen shake, flashes and slowed time can cause discomfort or nausea for some players, and some players cannot use controller vibration. FeelKit therefore maintains comfort settings for each local player and applies them to every recipe, both in the game and in the editor preview.

**Scales.** Each player has a Master scale and six group scales, each from 0 (off) to 1 (full strength): **Camera Shake**, **Camera Motion**, **Flashes**, **Hitstop and Slow-mo**, **Screen Distortion** and **Haptics**. A track's comfort scale is its group's scale times Master. The project settings map channels to groups ([see: set_comfort]); channels that are not mapped, such as audio and actor effects, are scaled by Master only. A track whose comfort scale is 0 does not play at all, unless it is essential (below).

**Presets.** FeelKit has three built-in presets besides the project defaults, with values you can change in the project settings:

| Preset | What it changes |
|---|---|
| Reduced Motion | Camera Shake 0.25, Camera Motion 0.25, Screen Distortion 0.5, camera roll off, field of view changes limited to 40° per second |
| Reduced Flashing | Flashes 0.2, at most 1 flash per second, extra flashes suppressed |
| No Haptics | Haptics 0 |

Custom presets are **Feel Comfort Preset** assets.

**Essential tracks.** A track marked **Essential** carries information the player needs, for example a flash that says the player was hit. When the player turns its group off, its **Substitute Step** plays instead, scaled by the substitute's own group; the library recipe `FR_Dread_JumpScare` has an essential Screen Flash with a Vignette Pulse as its substitute. An essential track without a substitute never drops below its **Essential Floor**. The floor never applies on Camera Shake and Camera Motion: there the player's setting always wins, and at 0 those tracks play no motion.

**Flash limiter.** Each player's settings limit how many flash tracks may start per second: 3 by default, with extra flashes softened to 0.3 of their strength. The limiter reduces risk but does not certify a game as safe for photosensitive players; games should also be tested with a dedicated analysis tool.

**Motion comfort.** A player can turn camera roll off for every recipe, and can limit how fast FeelKit changes the field of view, in degrees per second.

**Engine effects.** By default a player's Camera Shake scale (times Master) also applies to camera shakes the game plays through the engine, and their Haptics scale (times Master) to all controller vibration. Two project settings turn this off.

**Saving.** Settings load when a local player is created, before any recipe plays, and save automatically when they change: one save game slot per local player, named `FeelComfort_0`, `FeelComfort_1` and so on. You can route loading and saving to your own save system instead.

Players change their settings through your game's options menu, which calls the comfort nodes ([see: ref_nodes_comfort]). In the recipe editor, the **Comfort** dropdown previews a recipe with a preset applied. [Ref: ch08] covers comfort in full.

## Why the editor preview matches the game {#ch02_preview}

The recipe editor does not imitate the game; both run the same code. The steps compute their output, and a shared evaluator applies the recipe's timing, intensity and comfort rules to it. Only the final stage, which delivers the output, differs. In the game, a camera modifier on each local player's camera manager applies camera output, a view extension draws flashes and fades, and actor, audio and controller output goes to the real objects. In the editor, the same output goes to the preview's camera, preview mesh and post-process settings. Actor effects are applied and put back by the same code in both.

Because steps retain no state between frames, the preview can jump to any point in time and show the frame the game would show there. Scrubbing backwards is therefore as exact as playing forwards.

Some effects need a running game and cannot be shown in the preview: time dilation (Global Hitstop, Actor Hitstop, Slow-mo Ramp), controller vibration (Force Feedback Curve, Haptic Pattern), Blueprint Event, Light Flash, the widget steps and Number Pop. Their tracks carry a **No preview** label; the recipe's other tracks still preview normally. To judge them, start Play In Editor and press **Play in PIE** in the recipe editor, which plays the recipe on the player's pawn. Changes made in the editor apply to the next play without restarting Play In Editor.
