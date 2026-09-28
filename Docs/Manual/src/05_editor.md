# The recipe editor {#ch05}

The recipe editor is where recipes are built and tuned. It shows a recipe's tracks on a timeline, plays the recipe in its own preview while it is being edited, and uses the same evaluation code as the game ([see: ch02_preview]), so that what the preview shows is what the player sees. Its parts are described below in the order in which a recipe is usually built.

[video: V2]

## Creating and opening recipes {#ch05_create}

A recipe is an asset of type **Feel Recipe**. There are three ways to obtain one:

- **A new, empty recipe.** Right-click an empty area of the Content Browser and choose **FeelKit** > **Feel Recipe**, then name the asset. FeelKit's own recipes begin with `FR_`; any name works.
- **A copy of a library recipe.** Open a recipe under **FeelKit Content** > **Library** and click **Copy to Project** in the banner above the timeline, as described in [Ref: ch04_pick].
- **A recipe from a template** (Pro). Right-click a folder and choose **Recipe from Template...**, which lists the library recipes and places an editable copy in that folder ([Ref: ch09]).

Double-click a recipe to open it. The editor opens in its own window, or as a tab of the main window when that is how the editor was last arranged.

## The window: Preview, Timeline, Details, Intensity {#ch05_window}

The editor has four tabs. Like every Unreal editor tab, each can be moved, docked elsewhere or closed, and **Window** reopens a closed one.

[shot: S05-01 | The recipe editor. 1 Preview; 2 Timeline; 3 Intensity; 4 Details]

| Tab | What it shows |
|---|---|
| **Preview** | A small scene with a camera and a preview mesh, on which the recipe plays. The line in its top-left corner shows the playhead time. |
| **Timeline** | The toolbar, the ruler and one row per track. Selecting a track opens its intensity curve below it. |
| **Intensity** | The combined intensity of each channel over the length of the recipe ([see: ch05_comfort]). |
| **Details** | The recipe's settings, or the selected track's settings when a track is selected ([see: ch05_details]). |

The preview uses the standard viewport navigation of the Unreal editor: hold the right mouse button to look around and use W, A, S and D to move.

## The toolbar {#ch05_toolbar}

The toolbar at the top of the **Timeline** tab controls playback and the preview.

[shot: S05-02 | The Timeline toolbar]

| Control | What it does |
|---|---|
| **Play** / **Pause** | Plays or pauses the preview. Space does the same while the timeline has keyboard focus. |
| **Stop** | Stops and returns the playhead to the start. |
| **Release** | Ends the sustain loop of a sustained recipe, as **Release Feel** does in the game. Shown only for recipes with **Sustain** on, and enabled while the loop plays ([see: ch05_preview]). |
| **Loop** | Repeats the preview. |
| **Snap** | Snaps track edits to frames. Holding Shift while dragging ignores snapping. |
| Frame rate | The frame rate that snapping uses: 60 fps by default, with common rates and a custom value in the menu. The choice is a personal editor preference ([see: ch18]). |
| **Comfort** | The comfort settings the preview applies ([see: ch05_comfort]). |
| **Recent Plays** | **Pro.** Replays a play of this recipe that was recorded during Play In Editor ([Ref: ch10]). |
| **Play in PIE** | Plays the recipe in the running Play In Editor session ([see: ch05_pie]). |
| **Capture GIF** | **Pro.** Records the preview without and with the recipe and saves both side by side as an animated GIF ([Ref: ch10]). |
| **Fit** | Fits the whole recipe into the timeline. F does the same. |
| Time | The playhead time and the length of the recipe, in seconds. |

## Adding, moving and resizing tracks {#ch05_tracks}

Each row of the timeline is a track. Its header on the left shows the step's name, a mute toggle and a solo toggle; its bar on the right spans the track's start time and length and carries the track's channel. The bar's color follows the channel's family: camera, screen, actor, time, audio, haptics and so on.

**Adding a track.** Click **+ Track** in the top-left corner of the timeline and choose a step. The menu lists every step by name, including steps written in Blueprint ([Ref: ch20]), and has a search box. The new track starts at the playhead, or at 0 s after **Stop**, and takes the step's default channel and intensity curve. [Ref: ch16] describes every step.

**Moving and resizing.** Drag a bar to move the track in time. Drag either end of the bar to change its start or its length. With **Snap** on, edits snap to frames at the chosen frame rate, and edges are also drawn to the loud moments of any sound played by another track of the recipe, which makes it easy to align a flash or a hitstop with a sound. Hold Shift while dragging to ignore both kinds of snapping.

**The track menu.** Right-click a track for these commands:

| Command | Shortcut | What it does |
|---|---|---|
| **Copy** | Ctrl+C | Copies the track, to paste into this recipe or any other open recipe. |
| **Copy All Tracks** | | Copies every track of the recipe. |
| **Paste** | Ctrl+V | Pastes copied tracks at the playhead, keeping their timing relative to each other. |
| **Duplicate** | Ctrl+D | Adds a copy of the track. |
| **Delete** | Delete | Deletes the track. |
| **Move Up**, **Move Down** | | Changes the track's row. The order has no effect on the result. |
| **Mute** / **Unmute** | | Turns the track off in the preview and in the game ([see: ch05_details]). |
| **Solo** / **Unsolo** | | Plays only soloed tracks, in the preview. Soloing is an editing aid and does not affect the game. |

A Play Sound track adds two commands to this menu, described in [see: ch05_sound].

Every edit can be undone with Ctrl+Z.

**Labels on bars.** Some bars carry a short label after the channel name. **No preview** marks steps that act only in a running game, such as a hitstop or controller vibration: they are skipped in the preview and play normally in the game. Other labels explain why a track is silent or changed in the current preview, for example **Muted**, **Removed by comfort settings** or **Substitute plays (comfort)**; [Ref: ch10] lists them all.

## Shaping intensity with curve keys {#ch05_curves}

Each track has an intensity curve that sets how strongly the step plays across the length of the track: 1 is full strength and 0 is silence. The horizontal axis is the track's own time, from its start to its end, so the curve stretches with the track when the track is resized.

Select a track to open its curve in a lane directly below it. The lane repeats its controls as hints:

- Double-click the lane to add a key; double-click a key to delete it.
- Drag a key to move it. Holding Shift ignores snapping.
- Right-click a key to choose how it leads to the next key: **Linear** (a straight line), **Smooth** (an eased curve) or **Constant** (the value is held until the next key). **Delete Key** is in the same menu.

[shot: S05-03 | A selected Procedural Shake track with its intensity curve in the lane below it]

A curve without keys plays at full intensity throughout, and the lane says so. New tracks of most steps start with a curve that falls from 1 to 0, which suits a hit that fades out. Steps that shape their own motion, such as Camera Punch or Slow-mo Ramp, start with a flat curve at 1, because their motion already rises and settles.

The curve can also be edited in **Details**: the **Intensity Curve** field shows a small preview and an **Edit...** button that opens it in Unreal's curve editor. A track can use a Curve Float asset instead of its own keys; the lane then says so, and the asset is edited in its own editor.

## Recipe and track settings in Details {#ch05_details}

With no track selected, **Details** shows the recipe's settings. With a track selected, it shows that track under a category named after it (**Track 1**, **Track 2** and so on), followed by the track's other categories; the recipe's own category is folded away. Click an empty area of the timeline to return to the recipe.

[shot: S05-04 | Details for a selected track: the track's timing and channel, then Step with the step's own settings]

**Recipe settings:**

| Setting | Default | Purpose |
|---|---|---|
| **Cooldown** | 0 s | Time before the recipe can play again on the same target. |
| **Max Concurrent** | 0 | How many plays of the recipe may run at once on the same target. 0 means no limit. |
| **Default Intensity** | 1 | Multiplies every play of the recipe. |
| **Parameters** | empty | **Pro.** Named values that the game passes when it plays the recipe ([Ref: ch07]). |
| **Sustain**, **Sustain Start**, **Sustain End** | Off, 0 s, 1 s | **Pro.** A region that loops until the play is released ([Ref: ch07_sustain]). |
| **Library** | | **Pro.** Feeling, genres and description, used by the Recipe Browser and in Content Browser tooltips ([Ref: ch09_tagging]). |
| **Preview Mesh** | None | The mesh shown in the preview: any static or skeletal mesh, or a cube when empty. It is saved with the recipe but used only by the editor. |

**Track settings.** The **Track N** category holds **Step**, **Start Time**, **Duration**, **Channel**, **Intensity Curve**, **Seed**, **Applies To** and **Enabled**. **Step** unfolds to the settings of the step itself, such as a shake's amplitude and frequency; these are documented per step in [Ref: ch16]. The remaining categories are:

| Category | Contents |
|---|---|
| **Parameter Mappings** | **Pro.** Recipe parameters that scale this track's intensity ([Ref: ch07_parameters]). |
| **Randomness** | **Pro.** A random intensity and a random length for each play. |
| **Comfort** | **Essential**, **Substitute Step** and **Essential Floor**, which decide what happens when a player turns this kind of effect down ([Ref: ch08_essential]). |
| **Conditions** | **Pro.** **Local Player Only**, **Max Distance**, **Chance** and **Platforms** ([Ref: ch07_random]). |

A **Duration** of 0 makes an instant track, for steps that act once when the track starts, such as Play Sound or Blueprint Event. **Seed** changes the pattern of steps that use noise, such as Procedural Shake, so that two tracks of the same step do not move in step with each other. **Enabled** is the same switch as the mute toggle in the track header.

## The preview: play, loop, scrub, preview mesh, parameter sliders {#ch05_preview}

**Playing and scrubbing.** **Play** plays the recipe once and **Loop** repeats it. Click or drag in the ruler to move the playhead; the preview shows that exact moment. Because steps keep no state between frames, scrubbing backwards shows the same frames as playing forwards, so a fast effect can be examined frame by frame.

**What the preview shows.** Camera effects move the preview camera, screen effects color the preview, actor effects move, scale and recolor the preview mesh, and sounds play. Effects on game time, controller vibration, UI widgets and Blueprint events act only in a running game; their tracks carry **No preview**.

**The preview mesh.** Set **Preview Mesh** in the recipe's Details to see actor effects on a representative object, for example a character for a squash and stretch, or a sword for a material flash.

**Preview parameters** (Pro). When the recipe has parameters, a row of sliders appears below the toolbar, one per parameter, under **Preview parameters:**. They play the recipe as if the game had passed those values, so the whole range of a parameter can be checked without starting the game. The values are not saved; **Defaults** returns every slider to the parameter's default.

[shot: S05-05 | The preview parameter slider of a copy of FR_Power_ChargeUp, which has one parameter, Charge]

**Sustained recipes** (Pro). When **Sustain** is on, the ruler marks the sustain region, and playback loops inside it as it would while the game holds the play. Click **Release** to leave the loop and play the rest of the recipe, as **Release Feel** does in the game.

[shot: S05-06 | A copy of FR_Power_ChargeUp looping in its sustain region (0.35 s to 0.95 s, marked on the ruler), with Release enabled]

## Sound tracks: waveforms, snapping, tracks made from a sound {#ch05_sound}
[edition: Pro]

A Play Sound track draws the waveform of its sound on its bar, aligned with the timeline, so that the loud moments of the sound are visible. As described in [see: ch05_tracks], other tracks snap to those moments when they are dragged.

[shot: S05-07 | The waveform of the Play Sound track of HeavyHit]

Right-clicking a Play Sound track adds two commands to the track menu:

- **Create Force Feedback Track From Sound** adds a Force Feedback Curve track below the sound, whose rumble follows the sound's loudness.
- **Create Shake Track From Sound** adds a Procedural Shake track below the sound, whose strength follows the sound's loudness.

Both commands read the sound's audio data, so they need a Sound Wave, or a Sound Cue that plays one. MetaSounds and procedural sounds cannot be read: they show no waveform, and the two commands are unavailable for them.

## Checking comfort, and the intensity graph {#ch05_comfort}

Players can turn down effects that they find uncomfortable ([Ref: ch08]). The **Comfort** menu in the toolbar shows how the recipe plays for them:

| Choice | The preview plays with |
|---|---|
| **Neutral** | No comfort scaling: every effect at full strength. The default. |
| **Project Defaults** | The comfort scales that new players start with. |
| **Reduced Motion**, **Reduced Flashing**, **No Haptics** | The built-in presets, with the values set in the project settings. |

[shot: S05-08 | The Comfort menu of the preview]

With a preset chosen, tracks that comfort changes say so on their bar, for example **Comfort x0.25** for a shake that plays at a quarter of its strength, **Removed by comfort settings**, or **Substitute plays (comfort)** for an essential track whose substitute plays instead. Flash tracks that the flash limiter softens say **Flash softened by the flash limiter**.

The **Intensity** tab draws the combined intensity of each channel over the length of the recipe, one line per channel, with the preview parameters and the comfort choice applied. It shows at a glance where effects pile up, and how a parameter or a preset changes the recipe as a whole. [Ref: ch02_intensity] shows an example.

## Play in PIE, and editing while the game runs {#ch05_pie}

Effects on game time, controller vibration and UI can only be judged in a running game. **Play in PIE** plays the open recipe in the running Play In Editor session:

1. Start Play In Editor.
2. In the recipe editor, click **Play in PIE**. The recipe plays on the player's pawn; on a character, actor effects apply to its visible mesh.

The recipe can be edited while the game runs. Each change applies to the next play of the recipe, whether it comes from **Play in PIE** or from the game itself, so a value can be adjusted and tried again without restarting Play In Editor.

## Copy, paste and keyboard shortcuts {#ch05_shortcuts}

Copied tracks stay available while the editor is open, so tracks can be copied from one recipe and pasted into another. A pasted track is an independent copy with its own step settings.

The shortcuts below work while the timeline has keyboard focus; click the timeline once to give it focus.

| Key | Action |
|---|---|
| Space | Play or pause |
| F | Fit the whole recipe in view |
| Ctrl+Mouse Wheel | Zoom the timeline |
| Shift+Mouse Wheel, or drag with the middle mouse button | Scroll the timeline |
| Ctrl+C, Ctrl+V | Copy the selected track; paste at the playhead |
| Ctrl+D | Duplicate the selected track |
| Delete | Delete the selected track |
| Shift while dragging | Ignore snapping |
| Ctrl+Z, Ctrl+Y | Undo, redo |

## Validation messages on save {#ch05_validation}

When a recipe is saved, FeelKit checks it through Unreal's data validation and reports problems on the **Asset Check** page of the Message Log. An error means that part of the recipe cannot work as set up; a warning points out a setting that probably does not do what was intended. The recipe is saved in both cases. Messages about a track begin with its number and step, for example "Track 2 (Play Sound): Play Sound has no sound".

[shot: S05-09 | The Asset Check page after saving a recipe whose Play Sound track has no sound]

| Message | Meaning |
|---|---|
| Recipe has no tracks. | The recipe plays nothing. |
| Sustain is on, but Sustain End is not after Sustain Start, so nothing loops. | The sustain region is empty. |
| Sustain End is after the last track ends, so part of the loop is silent. | The loop includes time in which no track plays. |
| ... has no step, so it is skipped. | A track without a step. |
| ... has a length of 0, but this step needs a length to produce output. | Only steps that act once can be instant. |
| ... intensity curve has keys outside 0 to 1. | The curve is read over the track's time from 0 to 1, so those keys never play. |
| ... intensity curve never rises above 0, so the track produces no output. | The track is silent. |
| ... is essential but has no substitute step and an essential floor of 0, so comfort settings can remove it completely. | See [Ref: ch08_essential]. |
| ... has a substitute step but is not essential, so the substitute never plays. | Turn on **Essential**, or remove the substitute. |
| ... has a chance of 0, so it never plays. | The track's **Chance** condition is 0. |
| ... maps parameter N, which the recipe does not declare, so the mapping is ignored. | A parameter mapping names a parameter the recipe does not have. |
| Parameter N reads accumulator A, which is not defined in Project Settings > Plugins > FeelKit, so it uses its default value. | Add the accumulator to the project settings ([Ref: ch07_accumulators]). |

Steps add checks of their own, such as a Play Sound track without a sound; [Ref: ch16] lists them under each step.
