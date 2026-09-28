# Performance {#ch12}

FeelKit runs a few short timelines at a time, so its cost is small next to the rendering and gameplay it decorates. The sections below show where the cost is, how to see it, and how to keep large numbers of plays in check. It gives no timing figures: FeelKit's cost depends on the recipes and the machine, and `stat Feel` measures it for any game.

## When FeelKit runs {#ch12_idle}

FeelKit updates once per frame while there is something to do, and not at all otherwise. Its update runs while any of these is true:

- a recipe plays, or waits to start;
- a hitstop or slow motion still owns the time it changed;
- an actor, widget, material or sound still has to be put back after an effect;
- a flash, fade, tint or post-process material is still fading out;
- a controller still vibrates;
- an accumulator is still decaying.

Once everything has ended and been restored, the update stops, and an idle FeelKit costs nothing per frame. An automated test checks this.

## What a play costs {#ch12_play}

Each frame, FeelKit evaluates every active track of every playing recipe. Evaluation is arithmetic on the track's curve and step settings; it reads no assets and creates no objects. The results of all plays are then combined and delivered once per frame: one camera modifier per local player for camera effects, one screen overlay, one post-process blend, and the actor, widget, sound and controller changes.

A few steps create objects when their track starts, because the effect needs one:

| Step | Creates |
|---|---|
| **Play Sound** | An audio component, or a one-shot sound, as Unreal's own sound nodes do. |
| **Spawn Decal**, **Spawn Particle** | A decal or a Niagara system, removed after its lifetime. |
| **Number Pop** | Nothing per number: all numbers are drawn by one layer over the game view, added the first time a number appears. |
| **Material Parameter Pulse** | A dynamic material instance for each material it changes that has none, unless the step writes custom primitive data instead. The original materials are put back afterwards. |
| **Hit Flash** | A dynamic instance of its flash material, set as the mesh's overlay material while the flash plays. The mesh's own overlay material is put back afterwards. |
| **Post Process Material Pulse** | A dynamic instance of its material, kept for later plays. |

Every other step creates nothing.

## Measuring it {#ch12_measure}

In Play In Editor or a development build of a FeelKit Pro project, enter `stat Feel`. It shows **Feel Tick**, the time of FeelKit's update in the current frame, and **Feel Instances**, the number of plays running ([Ref: ch19]). In both editions, Unreal Insights shows FeelKit's update in its timing view, next to everything else the frame does.

Measure in a development or test build, not in the editor: the editor adds its own cost to every frame, and it records each finished play for the Debugger's recent plays ([Ref: ch10_replay]), which development builds without the editor do as well and Shipping builds do not.

## Keeping many plays in check {#ch12_limits}

A recipe on every footstep, bullet impact or coin is fine; a hundred of them in one frame is wasted work that no player can tell apart. Each recipe has its own limits for this:

- **Max Concurrent** refuses a play once that many copies of the recipe play on the same target. 1 or 2 suits footsteps and hit reactions.
- **Cooldown** refuses a play on the same target for that many seconds after the last one.
- **Max Distance** on a track skips it for targets far from the camera, and the **Distance** parameter can fade it out on the way ([Ref: ch07_random]).
- In a networked game, **Relevancy Distance** keeps far-away plays off other machines entirely ([Ref: ch11_relevancy]).

Camera and screen effects of simultaneous plays compete rather than add up, by default ([Ref: ch02_overlap_between]), so a burst of plays does not produce a burst of camera motion.

## Assets and memory {#ch12_assets}

A recipe is a small data asset. It loads with whatever references it, like any other asset, and its sounds, materials and particle systems load with it. Library recipes load only when a project uses them.

Editor-only data, such as a recipe's preview mesh, stays out of packaged games, and so does the whole FeelEditor module ([Ref: ch03_packaging]). The recipe tiles in the Content Browser are drawn from the recipe's data, without rendering a scene.
