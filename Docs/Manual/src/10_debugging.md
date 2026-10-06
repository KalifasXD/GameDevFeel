# Debugging and tuning {#ch10}

When a recipe does not play, plays too weakly or plays twice, the question is always the same: what did FeelKit decide, and why? Five tools answer it: the FeelKit Debugger during Play In Editor, replays of recorded plays in the recipe editor, the labels on track bars, the on-screen `showdebug feel` page, and the log. A sixth, Capture GIF, records a recipe for sharing.

The track labels ([see: ch10_labels]) and the log messages ([see: ch10_log]) are part of both editions. The Debugger, recent plays, `showdebug feel`, `stat Feel` and Capture GIF are part of FeelKit Pro.

## The FeelKit Debugger {#ch10_debugger}
[edition: Pro]

The Debugger lists what FeelKit is doing in every running game world, and keeps a list of plays that have ended. Open it from **Tools** > **FeelKit Debugger**. It can stay open while the game runs; it updates four times a second.

[shot: S10-01 | The FeelKit Debugger during Play In Editor: three recipes playing, the Combo accumulator, the player's comfort with Camera Shake at 0.25, and two recent plays]

Each running world is one row, named after its level, with its role in the **Details** column: **Single player**, **Listen server**, **Client** or **Dedicated server**. A network test in Play In Editor shows one row per world, so the plays of the server and of each client can be compared side by side. Under each world:

| Folder | Row | Columns |
|---|---|---|
| **Playing** | One row per play. | **Target**: the actor it plays on. **Time**: how far the play is, out of its length, such as `0.21 / 0.50 s`. **Value**: the play's intensity. **Details**: **Playing**, **Sustaining** while a sustained recipe waits in its loop, **released by** and the parameter's name when its **Release Parameter** released it, **then** and a recipe's name when a release recipe follows the release, or **Stopping** during a blend out, followed by the play's parameters, such as `Damage 80.00`. |
| **Accumulators** | One row per accumulator value. | **Target**: the actor the value belongs to, or **Global**. **Value**: its current value. |
| **Player N comfort** | One row per group, from **Master** to **Haptics**. | **Value**: the player's scale. Groups at 1 are dimmed, so changed ones stand out. The player row's **Details** show the flash limiter and whether camera roll is allowed. |
| **Controller vibration** | Under each player. | **Value**: the scale that reaches the controller. **Details**: the comfort scale and the controller scale. When nothing reaches the controller, the row shows a warning icon and says so. |

Below the worlds, **Recent plays** lists the plays that have ended, newest first ([see: ch10_replay]). Each row names the recipe, the target, the time the play ended, its intensity, and in **Details** how long it lasted, whether it was released (whether its **Release Parameter** did it, and which release recipe followed) or stopped early, and its parameters.

The search box at the top filters every folder by recipe, target and value. **Clear Recent Plays** empties the list of recent plays. With no game running, the Debugger says **Nothing running** and still shows the recent plays.

**Reading it.** A few common findings:

- A recipe that should play is missing from **Playing**: the play never started. Check the log for a warning ([see: ch10_log]), and that the call reaches **Play Feel** at all.
- A play is listed but nothing is seen: check its **Value** (an intensity of 0 plays silently) and the player's comfort scales; then open the play from **Recent plays** and read the track labels ([see: ch10_labels]).
- A sustained recipe stays at **Sustaining**: nothing releases it ([Ref: ch07_sustain]).
- An accumulator never rises: its name is not defined in the project settings, and the log says so.
- The controller does not vibrate: read the **Controller vibration** row ([Ref: ch14]).

## Replaying a recent play in the editor {#ch10_replay}
[edition: Pro]

Every play that ends during Play In Editor is recorded, with everything needed to show it again: the random seed, the intensity, the parameters, the player's comfort scales, the flash limiter's decisions, the distance from the camera, whether the target was a local player's, whether the play had an instigator, and the view direction. The recipe editor can then replay that exact play in its preview and scrub through it, after play has stopped.

To open a recorded play:

- In the Debugger, double-click its row under **Recent plays**, or click the play button in its **Replay** column. The recipe opens and the replay starts.
- In an open recipe editor, open the **Recent Plays** menu in the Timeline toolbar and choose a play. Each entry shows how long ago the play ended, its intensity and its target.

While a replay is active, the menu reads **Replaying a Recorded Play** and the preview uses the recorded inputs in place of its own settings: the **Comfort** menu and the parameter sliders make no difference. Edits to the recipe apply to the replay at once, which makes it the quickest way to fix a play that felt wrong in the game: open it, change a track, and watch the same moment again. **Back to Normal Preview** in the same menu ends the replay.

A replayed sustained play loops until **Release** is clicked, because the moment the game released it is not recorded. Its ending then follows the recorded outcome, so the release recipe that played in the game, **On Full Release** or **On Early Release**, plays again ([Ref: ch07_sustain]).

[shot: S10-02 | A replayed play of HeavyHit: the shake plays at a quarter because of the player's comfort, the camera punch was skipped by its distance condition, and the flash was softened by the flash limiter]

FeelKit keeps the last 64 plays. The list lives until the editor closes, so plays from earlier Play In Editor sessions stay available. A replay needs its recipe: a play whose recipe asset has since been deleted is shown dimmed and cannot be opened.

## Track labels that explain why a track did not play {#ch10_labels}

A track bar in the recipe editor can carry a short label after its channel name. The label explains what the preview, or the replayed play, decided for that track. Only the first reason that applies is shown, in this order:

| Label | Meaning |
|---|---|
| **Muted** | The track's mute button is on ([Ref: ch05_tracks]). |
| **Silent: another track is soloed** | Another track is soloed and this one is not. |
| **Skipped: not on this platform** | The track's **Platforms** condition does not list the current platform. |
| **Skipped: target is not a local player's** | **Local Player Only** is on and the play's target did not belong to a player on this machine. |
| **Skipped: target N cm away** | The target was farther from the local camera than the track's **Max Distance**. |
| **Skipped this play (chance N%)** | The track's **Chance** roll failed for this play. |
| **Skipped: the play had no instigator** | **Applies To** is **Instigator** and the play had none ([Ref: ch07_instigator]). |
| **Removed by comfort settings** | The player's scale for the track's group is 0 and the track is not essential ([Ref: ch08_essential]). |
| **Substitute plays (comfort)** | The track is essential, its group's scale is 0, and its substitute step plays instead. |
| **Flash suppressed by the flash limiter** | The flash came faster than the player's limit allows, and the limiter is set to suppress ([Ref: ch08_flash_limiter]). |
| **Flash softened by the flash limiter (xN)** | The same, with the limiter set to soften: the flash plays at N of its strength. |
| **Picked: ...**, **Picked nothing** | A Random Choice track, and the option it picked for this play ([Ref: ch07_random]). |
| **Comfort xN** | The track plays, scaled by the player's comfort to N of its strength. |

**Release recipe rows** (Pro). The **On Full Release** and **On Early Release** rows under the tracks ([Ref: ch05_preview]) carry labels of their own after the recipe's name:

| Label | Meaning |
|---|---|
| **Plays if ... reaches Release At**, **Plays if released before ... reaches Release At**, **Plays on release** | The play is still looping: the release recipe waits for the release ([Ref: ch07_sustain]). |
| **Plays: ... reached Release At**, **Plays: released before ... reached Release At**, **Plays: released** | The play was released this way, and the release recipe plays. |
| **Skipped: released before ... reached Release At**, **Skipped: ... reached Release At** | The play was released the other way, so this release recipe does not play. |
| **Never plays: no Release Parameter**, **Never plays: no sustain region** | **On Full Release** without a declared **Release Parameter**, or a release recipe on a recipe whose sustain region is off or empty. |
| **Skipped: this is the recipe itself** | The setting names the recipe it belongs to, which cannot play itself. |

Separately, **No preview** after the channel name marks a step that acts only in a running game, such as a hitstop or controller vibration: the preview skips it, and the game plays it.

The normal preview treats its target as the local player's, at an unknown distance and with an instigator, so only the **Chance** and **Platforms** conditions can skip a track there. Choosing a preset in the **Comfort** menu shows the comfort labels ([Ref: ch05_preview]). A replayed play shows every label, from the values the game recorded.

## showdebug feel and stat Feel {#ch10_showdebug}
[edition: Pro]

Two console commands show FeelKit's state over the running game, which helps in a packaged development build where the Debugger is not available:

- `showdebug feel` draws the plays, accumulators and comfort of the local player over the game view, where Unreal draws its other `showdebug` pages. Enter it again to hide it.
- `stat Feel` shows the time FeelKit's update takes and the number of plays it runs. FeelKit updates only while something plays, so the time reads zero while it is idle.

[Ref: ch19] lists every line of both. Neither command is available in a Shipping build, like the rest of Unreal's debug display.

## Log messages {#ch10_log}

FeelKit writes to the Output Log under **LogFeel**, and the Blueprint Event step under **LogFeelSteps**. Every message is a warning that names what to change. Filter the Output Log by `LogFeel` to see only these.

| Message begins with | Cause | What to do |
|---|---|---|
| `PlayFeel: ... was given an actor or component target that is not valid` | The target actor or component was destroyed, or never set. | Check the value passed to the target node ([Ref: ch06_play]). |
| `Send Feel Event ...: no Feel Map entry matches, so nothing plays` | **Pro.** No row of any Feel Map matches the event and its context tags. The message counts the maps that were searched. Shown once per event. | Add a row for the event, or check its required tags ([Ref: ch06_events]). |
| `Send Feel Event ...: the matching Feel Map entry has no recipe` | **Pro.** A row matched, but its **Recipe** is empty. | Set the recipe on that row. |
| `Accumulator ... is not defined` | **Pro.** A node or notify names an accumulator the project settings do not define. | Add it under **Accumulators** ([Ref: ch07_accumulators]). |
| `Controller vibration is off for this player` | The player's **Haptics** or **Master** comfort is 0, so no vibration reaches the controller. Shown once, with the scales responsible. | Nothing, if the player chose it; otherwise check the comfort settings ([Ref: ch08_engine]). |
| `Show Feel Comfort Menu needs a local player controller` | The node was given a controller that is not a local player's. | Pass the local player's controller. |
| `Show Feel Comfort Menu: no comfort menu to open` | **Comfort Menu Class** is empty and the node passed no menu. | Set the class in the project settings ([Ref: ch08_menu]). |
| `...: this level already has a Feel Switch` | A second Feel Switch in the same level; it does nothing. | Keep one Feel Switch per level ([Ref: ch06_switch]). |
| `Blueprint Event: ... has no event or function named ...` | The Blueprint Event step names an event the target does not have. | Check the event name and the target ([Ref: ch16]). |
| `Blueprint Event: ... must take no inputs or a single float` | The event exists, but its inputs do not fit. | Give the event no inputs, or one float input for the intensity. |

Saving a recipe also checks it and reports problems in the Message Log, on its **Asset Check** page ([Ref: ch05_validation]).

## Capture GIF {#ch10_gif}
[edition: Pro]

**Capture GIF** in the Timeline toolbar records the preview twice, without and with the recipe, and saves both side by side as one animated GIF: the left half without the recipe, the right half with it. It shows the difference a recipe makes, for a design review, a bug report or a store page.

- Each half is 480 pixels wide, at 20 frames per second.
- The capture lasts the recipe's length plus a quarter of a second, between half a second and six seconds.
- The file goes to the project's `Saved/FeelKit/Captures` folder. A notification names it and offers **Show in Explorer**.
- Keep the preview visible while it records; a hidden preview cannot be captured, and the notification says so.

The GIF shows what the preview shows, so steps marked **No preview**, such as hitstops and controller vibration, are not in it, and sounds are not recorded. The comfort preset chosen in the preview applies to the capture.
