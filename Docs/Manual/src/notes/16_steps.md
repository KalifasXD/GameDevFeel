Hand-written text for chapter 16. gen_reference.py copies each block below into src/16_steps.generated.md.
Each block starts with a line "@@ key". Keys are step class names, or section and group names.

@@ section.recipe_fields
The recipe editor's **Details** tab shows these fields. **Tracks** is hidden there, because tracks are edited on the timeline: when a track is selected, its fields appear in the same tab and the **Recipe** category folds away.

@@ section.track_fields
Select a track on the timeline to see these fields in **Details**. The fields of the **Track** category appear under the track's number (**Track 1**, **Track 2** and so on), followed by **Parameter Mappings**, **Randomness**, **Comfort** and **Conditions**. The settings of the track's step are inside **Step**.

@@ section.step_entries
Every step below has the same parts:

- **Class** and module: the C++ class, for searching the source and for C++ code. The **+ Track** menu of the recipe editor lists steps by their display name, which is the heading of the entry.
- **Default channel**: the channel a new track of this step gets. You can change it on the track; comfort and arbitration follow the track's channel, not the step.
- **Comfort group**: the group that scales the default channel under the default **Channel Comfort Groups** in the project settings ([see: set_comfort]).
- **Editor preview**: whether the recipe editor's preview shows the step. Steps it cannot show still play in the game, and their tracks carry a **No preview** label.
- **Track length**: steps that act once when the track starts can sit on an instant track of length 0.
- **New tracks start with**: the intensity curve a new track of the step gets.
- The table: every setting of the step as the **Details** tab shows it, with its default and units, followed by what it does, its allowed range and when the editor shows it.
- **Checks when the recipe is saved**: problems with the step's settings that data validation reports.

Rotation values are listed as pitch, yaw and roll. The **Details** tab shows a rotator as X (roll), Y (pitch) and Z (yaw). Camera offsets are in camera space: X points forward, Y right and Z up.

[shot: S16-01 | The + Track menu of the recipe editor, listing every step]

[shot: S16-02 | A Global Hitstop track, labeled No preview, above a Camera Punch track that previews]

@@ section.motion_settings
Camera Punch, FOV Kick and the other steps in the table below move along a motion shape. The shape gives the form of the motion over the track, from 0 to a peak of 1 and back; the step's own settings give the size at the peak, and the track's intensity multiplies the result. New tracks of these steps start with a flat intensity curve, so the shape alone sets the motion.

With **Spring**, the motion follows time in seconds, not the length of the track: a track shorter than the spring's settling time cuts the motion off, and a longer track stays at rest after it settles. **Kick** and **Smooth** stretch to fill the track. **Repeats** splits the track into equal parts and plays the shape once in each.

@@ group.camera
Camera steps offset the view of the target's local player, through a camera modifier that FeelKit adds to each local player's camera manager. Tracks of one play add up; separate plays are combined by the **Camera Arbitration** setting ([see: set_camera]).

@@ UFeelStep_ProceduralShake
Moves and turns the camera with noise or a sine wave computed from the track time and the seed, so the same seed gives the same shake every time and the preview shows exact frames when scrubbing. **Perlin** gives smooth random motion on each axis. **Sine** gives a regular oscillation with a random phase per axis. **Directional** moves back and forth along **Direction** only, with the rotation swinging in step. The intensity curve sets the strength over time; the default curve gives a shake that dies away over the track.

@@ UFeelStep_CameraPunch
Pushes and turns the camera to a peak and back along the motion shape; with the default Spring it overshoots and settles like a physical hit. **Location Punch** and **Rotation Punch** are the offsets at the peak. **Direction Source** can point the punch along the play's Direction, or away from or toward the play's Location, as seen from the camera when the play started. The strength stays that of the step settings, and the step settings are used when the play passed no direction or location. **Direction Jitter** turns the direction by a random angle for each play, which stays fixed for the whole play.

@@ UFeelStep_FOVKick
Changes the field of view by **Field Of View Kick** at the peak and back. Positive values widen the view, which reads as speed; negative values narrow it, which reads as an impact zoom. The default Kick shape rises fast and eases back.

@@ UFeelStep_CameraRoll
Rolls the camera around its view axis to **Roll Degrees** at the peak and back. With **Random Direction** on, each play picks clockwise or counterclockwise from its seed. Players who turn **Allow Camera Roll** off in their comfort settings see no camera roll from any recipe.

@@ UFeelStep_CameraZoom
Eases the field of view by **Field Of View Change** over the first part of the track (**Ease in Fraction**), holds it, and eases back over the last part (**Ease Out Fraction**). Negative values zoom in. The track length sets how long the zoom holds.

@@ UFeelStep_LookAtNudge
Turns the camera part of the way toward the play's Location, or along the play's Direction when it passed no location, and back. **Turn Fraction** is the share of the angle to that point reached at the peak, and **Max Turn Degrees** limits the turn in yaw and in pitch. A play with neither a location nor a direction produces no motion.

@@ group.time
Time steps change time dilation. Recipes are timed in real time, so a track keeps its length and the rest of the recipe keeps playing while the world is slowed. When time requests overlap on the same clock, the highest **Priority** wins, and among equal priorities the slowest dilation wins. When no request remains, the clock gets back the dilation it had before FeelKit changed it. In networked games a global request slows only the play's target unless **Allow Global Time Dilation in Multiplayer** is on ([see: set_playback]).

@@ UFeelStep_GlobalHitstop
Slows the whole world to **Time Dilation** while the track plays. The track's intensity, clamped to 0 to 1, blends between normal speed at 0 and **Time Dilation** at 1, so a flat curve gives a hard stop and a fading curve lets time recover over the track. The editor preview cannot slow time; use **Play in PIE** to judge a hitstop.

@@ UFeelStep_ActorHitstop
Works like Global Hitstop, but slows only one actor through its custom time dilation, so the rest of the world keeps moving: the play's target, or the instigator when the track's **Applies To** is Instigator.

@@ UFeelStep_SlowMoRamp
Eases the whole world into slow motion over **Ramp in Time**, holds at **Time Dilation**, and eases back to normal speed over **Ramp Out Time** before the track ends. Both ramps are real seconds with a smooth start and end. The intensity, clamped to 0 to 1, sets how far time slows: at 0.5 it slows half as far.

@@ group.screen
Screen steps change the whole view of the target's local player. Flashes and fades are drawn over the view; tints, vignette, chromatic aberration, saturation and post-process materials are blended into the view's post-process settings. Across separate plays the strongest contribution wins for each of these.

@@ UFeelStep_ScreenFlash
Covers the view with **Color** at up to **Max Opacity**, drawn on top of the game's own camera fades. The intensity curve shapes the flash: the default curve starts at full opacity and clears over the track. Screen Flash tracks count toward the flash limiter of each player's comfort settings.

@@ UFeelStep_VignettePulse
Darkens the edges of the view by blending the scene's vignette toward **Vignette Intensity** and back. The engine's own vignette is about 0.4, so values near 1 give a strong darkening. With **Repeats** above 1 it pulses several times across the track, for example as a heartbeat.

@@ UFeelStep_ChromaticAberration
Splits the colors toward the edges of the view by blending the scene's chromatic aberration toward **Fringe Intensity** and back. The default Kick shape rises fast and eases back.

@@ UFeelStep_Desaturate
Drains color from the scene and brings it back. **Amount** 1 makes the picture grayscale at the peak; the default 0.8 leaves a trace of color.

@@ UFeelStep_ColorTint
Multiplies the scene toward **Tint Color** and back. **Strength** sets how far the scene moves toward the tint at the peak. A white tint leaves the scene unchanged.

@@ UFeelStep_ScreenFade
Fades the view to **Fade Color**, holds, and fades back: the first **Fade in Fraction** of the track fades in, the last **Fade Out Fraction** fades out, and the view holds at **Max Opacity** in between. With **Fade Out Fraction** at 0 the view stays faded until the track ends. The fade is drawn under flashes, so a flash during a fade stays visible.

@@ UFeelStep_PostProcessMaterialPulse
Blends a post-process material over the scene and back, for effects the engine has no setting for, such as radial blur, scanlines or heat haze. The material must use the Post Process material domain. Unreal draws an active post-process material at full strength, so FeelKit passes the current weight, from 0 to 1, to the scalar parameter named in **Weight Parameter**. The material should use that value to fade its own effect, for example as the Alpha of a Lerp from Scene Texture PostProcessInput0 to the effect. The sample material `M_FK_PP_Pulse` in /FeelKit/Samples/Materials is set up this way.

@@ group.actor
Actor steps change the actor the track applies to: the play's target, or the instigator when the track's **Applies To** is Instigator. On an actor whose root takes part in collision, such as a character's capsule, motion and scale go to its visible child components, so collision and movement stay unchanged. FeelKit restores every original value once no track changes it any more.

@@ UFeelStep_ScalePunch
Scales the target up by **Amount** and back with a springy swing: the scale starts and ends at the target's own scale, and each of the **Bounces** adds one more swing below and above it in between. **Amount** is relative, so 0.3 makes the target 30% larger at the peak. The intensity curve sets how quickly the swings die down.

@@ UFeelStep_SquashStretch
Stretches the target along its local **Axis** by **Amount** at the peak; negative amounts squash it. With **Preserve Volume** on, the other two axes thin out while it stretches and bulge while it squashes, so the target keeps its volume. With the default Spring shape a stretch overshoots into a squash before it settles.

@@ UFeelStep_MaterialPulse
Pulses material parameters on the target's meshes and puts their original values back afterwards. For each entry of **Parameters**, a color parameter blends from its original color toward **Color**, or a scalar parameter gets **Scalar Amount** added at the peak. **Route** chooses how the values reach the materials: **Material Instance** creates dynamic material instances and works with any material parameter; **Custom Primitive Data** needs no material instances, but the material has to read the parameter as custom primitive data. When several tracks pulse the same parameter, scalar changes add up and the strongest color wins.

@@ UFeelStep_HitFlash
Flashes the target's meshes with **Color** by drawing an instance of **Flash Material** in each mesh's overlay material slot, so it works on any mesh without changes to its own materials. The flash material must be a translucent overlay material with a vector parameter FlashColor and a scalar parameter FlashAmount from 0 to 1. The sample `M_FK_HitFlash` in /FeelKit/Samples/Materials is one. The mesh's own overlay material is put back afterwards.

@@ UFeelStep_MeshWobble
Tilts and moves the target back and forth around wherever it is, fading out over the track, so it also works on moving targets. **Tilt Amplitude** and **Move Amplitude** are the largest swings at the start, **Frequency** the swings per second, and **Decay** how quickly the swing dies: 0 keeps full strength, 1 fades linearly and higher values fade faster. Each axis swings with its own phase. **Noise** replaces the regular swing with irregular motion. A component whose physics simulates, such as a ragdoll, is left to physics.

@@ UFeelStep_LightFlash
Brightens the target's lights and can shift their color, then returns them. **Intensity Change** is a multiple of each light's own intensity: 2 makes it three times as bright and -1 turns it off. It applies to the target when the target is a light component, and otherwise to every light component of the target actor. **Color Strength** moves the light color toward **Color**. With **Flicker** on, the light switches between flashed and normal **Flicker Rate** times per second, in an on and off pattern taken from the seed. Light Flash tracks count toward the flash limiter under the default channel mapping.

@@ group.audio
Audio steps play sounds or change the mix. They have no comfort group under the default mapping, so only Master scales them.

@@ UFeelStep_PlaySound
Plays **Sound** when the track starts, in 2D, attached to the target (at **Attach Socket Name** when set) or at the target's location. **Attached to Target** falls back to the target's location when the target has no component. Each play varies the volume and pitch at random by up to **Volume Variation** and **Pitch Variation**. With **Scale Volume with Intensity** on, the volume follows the track's intensity at the moment the track starts, comfort included. On a track with a length, **Stop at Track End** and **Stop when Recipe Stops** fade the sound out over **Fade Out Time**; on an instant track the sound plays to its end. The sound is audible in the editor preview.

@@ UFeelStep_SoundClassDuck
Lowers the volume of **Sound Class** and its child classes, then brings it back. The change builds up over the first **Attack Fraction** of the track, holds, and releases over the last **Release Fraction**. **Volume Reduction** 1 silences the class at full intensity. With **Sound Class** empty, the project's default sound class is used (Project Settings > Audio). FeelKit applies the change through a sound mix it creates while the track plays. When several tracks change the same class, the lowest volume wins.

@@ UFeelStep_PitchBend
Bends the pitch of **Sound Class** and its child classes by **Pitch Change** and back, with the same attack, hold and release settings as Sound Class Duck. -0.3 plays 30% lower and 0.5 plays 50% higher. When several tracks change the same class, the pitch farthest from normal wins.

@@ UFeelStep_LowPassSweep
Filters the high frequencies of **Sound Class** and its child classes down to **Cutoff Frequency** at full strength, then opens the filter again, with the same attack, hold and release settings as Sound Class Duck. The cutoff moves between fixed levels rather than continuously: no filter (20000 Hz), 8000, 4000, 2000, 1000, 500 and 250 Hz. The level closest to the current cutoff is used. When several tracks change the same class, the lowest cutoff wins.

@@ group.haptics
Haptics steps vibrate the controller of the target's local player. On Windows, Xbox controllers vibrate directly; PlayStation and other controllers vibrate when the game runs through Steam with Steam Input.

@@ UFeelStep_ForceFeedbackCurve
Vibrates the controller with four motor strengths, which are the values at full intensity. The motion shape (Kick by default) and the intensity curve shape the vibration over the track. **Ripple Frequency** adds regular dips in strength, for textures such as an engine or a heartbeat, and **Ripple Depth** sets how deep the dips go. When several plays vibrate the same controller, the strongest value wins for each motor.

@@ UFeelStep_HapticPattern
Plays a Force Feedback Effect asset on the controller when the track starts, with the asset's own curves. With **Looping** on and a track length, the effect repeats until the track ends. **Stop with Track** stops it when the track ends or the recipe stops. The effect ignores time dilation, so hitstops do not stretch it. The track's intensity does not change the effect's strength. A player's Haptics comfort still applies: at 0 the track does not play, and with **Apply Comfort to Engine Force Feedback** on, the player controller's force feedback scale reduces this effect like any other vibration.

@@ group.ui
Widget Punch, Widget Shake and Widget Flash need a widget target ([see: node_target_from_widget]); on other targets they do nothing. They change the widget's render transform and color, which leaves the layout alone, and FeelKit restores both afterwards. Number Pop draws text over the game view of the target's player and works with any target.

@@ UFeelStep_WidgetPunch
Scales, moves and turns the widget target by **Scale Change**, **Translation** and **Angle Degrees** at the peak and back; the default Spring shape overshoots before it settles. Offsets from several tracks add up.

@@ UFeelStep_WidgetShake
Shakes the widget target with smooth noise on each axis. **Amplitude** is the largest movement at the start, **Frequency** the speed, and **Decay** how quickly the shake dies over the track: 0 keeps full strength and 1 fades linearly. **Angle Amplitude** adds rotation.

@@ UFeelStep_WidgetFlash
Tints the widget target toward **Color** and back. It applies to user widgets, images and text blocks (their color and opacity) and to borders (their brush color). **Strength** sets how far the color moves at the peak. When several tracks tint the same widget, the strongest wins.

@@ UFeelStep_NumberPop
Shows a number or short text that pops up at the target, or at the play's Location, and floats up over the game view of the target's player. With **Value Parameter** set, it shows that recipe parameter's value with **Decimals** digits after the point; otherwise it shows **Text**. **Prefix** and **Suffix** are added around it. The text appears at **Pop Scale** times its size and settles, rises by **Rise Distance** and gets a random sideways offset of up to **Spread**, so repeated pops do not cover each other. It stays for the length of the track, or for **Default Lifetime** on an instant track. With **Scale with Intensity** on, the font size follows the intensity, between 0.25 and 4 times **Font Size**.

@@ group.spawn
Spawn steps create something in the world when the track starts and leave it to the engine afterwards.

@@ UFeelStep_SpawnDecal
Spawns **Decal Material** at the play's Location, or at the target when the play passed none; with **Location** set to Target it always uses the target. With **Find Surface** on, FeelKit looks for a surface from that point along the projection direction, which is down unless the play passed a Normal, up to **Surface Search Distance** beyond the edge of the target, and places the decal on it, facing the surface. **Random Rotation** turns each decal around the normal. The decal stays for **Lifetime** and then fades over **Fade Out Time**. The decal material must use the Deferred Decal domain; `M_FK_Decal_Scorch` in /FeelKit/Samples/Materials is a sample.

@@ UFeelStep_SpawnParticle
Spawns the Niagara system **System** at the play's Location or at the target, or attached to the target at **Attach Socket Name** when **Attach to Target** is on. **Orient to Normal** turns the system's up axis to the play's Normal. **Parameters** pass values into float user parameters of the system when it spawns: the value of a recipe parameter, or the track's intensity when **Recipe Parameter** is empty. With **Deactivate with Track** on and a track length, the system stops spawning new particles when the track ends or the recipe stops. The step is in the FeelNiagara module, which is enabled with FeelKit.

@@ group.meta
Meta steps play other steps and recipes, or call into game code.

@@ UFeelStep_Recipe
Plays another recipe inside this track. The inner recipe starts with the track and plays for as long as the track lasts, so give the track at least the inner recipe's length. It uses the same target, context and parameters, and **Intensity Scale** multiplies its intensity on top of this track's intensity. Its tracks keep their own channels and comfort settings. Recipes nest up to a depth of 4; deeper tracks are skipped, which also stops a recipe from playing itself without end.

@@ UFeelStep_RandomChoice
Plays one of its **Options**, picked at random for each play by **Weight**: an option with weight 2 is picked twice as often as one with weight 1. The pick is fixed for the whole play, so scrubbing shows the same option, and every new play picks again. Options without a step or with weight 0 are never picked. Checks on save include the settings of every option's step.

@@ UFeelStep_BlueprintEvent
Calls a custom event or function by name when the track starts (**Start Event Name**) and when it ends (**Stop Event Name**). **Receiver** chooses the object that is called: the target actor, the pawn or the player controller of the target's local player, or the Level Blueprint. The event must take no inputs, or a single float that receives the track's intensity; with any other inputs, or when the name is not found, nothing is called and a warning appears in the log under LogFeelSteps. With **Call Stop Event when Interrupted** on, the stop event is also called when the recipe is stopped early. Blueprint calls need a game world, so the editor preview does not make them.
