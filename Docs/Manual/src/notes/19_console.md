Hand-written text for chapter 19. gen_reference.py copies each block below into src/19_console.generated.md.

@@ chapter.19
Enter these in the console of Play In Editor or of a development build; Shipping builds have no console. In Blueprint, the node **Execute Console Command** runs the same commands.

@@ cvar:feel.Enabled
Set to 0 to stop every playing recipe at once and to block new plays; time slowed by FeelKit returns to normal. Set to 1 to turn playback back on. The value is not saved. The Feel Switch actor and the nodes **Set Feel Enabled** and **Toggle Feel** change the same value ([see: ref_nodes_switch]). Only FeelKit is affected: the game's own camera shakes, sounds and effects keep working.

@@ cvar:feel.GlobalScale
Multiplies the intensity of every play on top of the call intensity. At 0 nothing shows, but plays continue and come back at full strength when the value goes back up. Values above 1 make every recipe stronger, and negative values count as 0. There is no Blueprint node for it; use **Execute Console Command** with, for example, `feel.GlobalScale 0.5`.

@@ cmd:stat
`stat Feel` shows two counters: {COUNTERS}. Feel Tick is the time FeelKit's update takes, and Feel Instances the number of plays it is running. FeelKit updates only while something plays or still has to be restored, so Feel Tick shows no time while FeelKit is idle.

@@ cmd:showdebug
`showdebug feel` draws FeelKit's state over the game view, where Unreal draws its other showdebug pages. Enter the same command again to hide it. The lines are:

- A header with the number of playing recipes, accumulator values and number pops.
- One line per playing recipe: its name, its target, its time and length in seconds, "(sustaining)" while it loops in its sustain region, its intensity and the values of its parameters.
- One line per accumulator value, global or for one actor.
- The comfort scales of the viewing player, with the flash limiter and camera roll settings.
- **Controller vibration**: the comfort scale for vibration and the scale on the player controller. When the controller's scale is 0 the line turns red and ends with NOTHING REACHES THE CONTROLLER.

[shot: S19-01 | The showdebug feel lines during play, with two recipes playing]
