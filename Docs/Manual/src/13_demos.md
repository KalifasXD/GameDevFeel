# Demo levels {#ch13}
[edition: Pro]

Four demo levels show FeelKit in games of different kinds. Each is built on one of Unreal's own templates and adds FeelKit to it, so the feedback can be compared with the template as Epic ships it: every level has a Feel Switch that turns FeelKit off and on while playing ([Ref: ch06_switch]). The recipes are part of FeelKit Pro. Below, each level is described with what it shows, how it is hooked up, and what its recipes do, so the levels can serve as working examples.

## Getting the demos {#ch13_get}

- **Playable builds** of all four levels are free downloads, linked from the FeelKit listings on Fab. They run without Unreal.
- **Source projects.** Owners of FeelKit can ask for the Unreal Engine 5.6 projects behind the levels, on the FeelKit Discord or by email ([Ref: ch14_help]). They need FeelKit Pro installed in the engine.
- **Engine version.** The levels use Epic's 5.6 template content, whose levels crash in Play In Editor on 5.7 and 5.8 even without FeelKit. The source projects are therefore for 5.6 only. On 5.7 and 5.8, the recipes themselves work; they can be hooked into that engine's own templates in the same way.

In every level, **Tab** or the controller's **View** button turns FeelKit off and on, and **O** or the controller's **Menu** button opens the comfort menu ([Ref: ch08_menu]). Each level lists its controls under the FEEL ON / OFF badge.

## Action/RPG: Weight Class {#ch13_arpg}
[edition: Pro]

[shot: S13-01 | Action/RPG: a combo hit lands on the training dummy]

**Template:** Third Person, Combat variant. **Controls:** move WASD or left stick; look mouse or right stick; attack left mouse or RB; charged attack hold right mouse or RT; block and parry hold Left Shift or LT.

The player fights training dummies and enemies with a three-hit combo and a charged attack, and can block and parry. Hits get heavier through the combo, a charged strike breaks an enemy's guard, and a well-timed block parries.

**How it is hooked up.**

- **Animation notifies** on the combo and charged-attack montages: **Set Feel Value** sets the accumulator `Swing` to 0.35, 0.65 and 1.0 on the three attacks, and `Charged` and `ChargeLevel` during the charge; **Play Feel** plays the swing recipe ([Ref: ch06_notifies]).
- **Feel Map** `FM_ARPG`: the template's damage events send `Feel.Event.Hit.Landed`, `Feel.Event.Hit.Received`, `Feel.Event.Hurt` and `Feel.Event.Death` ([Ref: ch06_events]).
- **Guards:** a guarding enemy and the player's block play the guard recipes on the defender.
- The player's Blueprint adds 1 to `Combo` on every hit ([Ref: ch07_accumulators]).

| Recipe | What it does |
|---|---|
| `FR_ARPG_Swing` | Every swing, hit or miss: a whoosh and a slight lean of the camera into the swing. |
| `FR_ARPG_HitLanded` | The player's hit connects. Light, heavy and finisher from one recipe, driven by `Swing`; the charged strike adds a critical hit on top. |
| `FR_ARPG_EnemyHurt` | What was hit reacts: a hard white flash, a freeze, a squash and wobble, sparks, and a streak counter from the second hit on. |
| `FR_ARPG_EnemyDeath` | An enemy goes down: a freeze, a slow-motion beat with the color drained, a zoom, a heavy thud and a long rumble. |
| `FR_ARPG_ChargePulse` | One loop of holding a charged attack: a hum, a shake and a rumble that grow the longer it is held. |
| `FR_ARPG_GuardBlock` | A guard holds: the shield flares, sparks fly, a clang that deepens with heavier swings, a short freeze, and the attacker's swing stops dead. |
| `FR_ARPG_GuardBreak` | A charged strike breaks a guard: the shield shatters over a heavy clang, a freeze into slow motion, a warm flash. |
| `FR_ARPG_Parry` | A guard raised just before the hit turns it aside: a clear ring, a longer freeze, a white flash and a small zoom on the one who parried. |

**Project setup it relies on:** the accumulators `Combo` (maximum 10, decay 1 per second after 1.5 s), `Swing`, `Charged` and `ChargeLevel` (maximum 1, no decay), `FM_ARPG` in the project's Feel Maps, and the Combat template's `NS_Damage` particle system for the sparks. In the source project these recipes live in the project, under `/Game/FeelKitDemos/ActionRPG`, because they refer to the template's content.

## Platformer: Bounce Feel {#ch13_platformer}
[edition: Pro]

[shot: S13-02 | Platformer: a dash]

**Template:** Third Person, Platforming variant. **Controls:** move WASD or left stick; look mouse or right stick; jump Space or A; double jump by jumping again in the air; wall jump by jumping against a wall; dash Left Shift or B.

Every movement has its own response, and landings scale with the fall: a hop lands lightly, a long drop hits the ground hard. Movement cameras push one way and settle, without shaking back and forth, which keeps a fast platformer comfortable.

**How it is hooked up.** A **Feel Trigger** component on the character plays the recipes on **Jumped**, **Air Jumped**, **Launched** (the template's wall jump) and **Landed**, with the landing speed passed as a parameter ([Ref: ch06_trigger]). The dash plays its recipe from a marker in the dash.

| Recipe | What it does |
|---|---|
| `FR_PLAT_Jump` | A jump from the ground: the body stretches up, a quick whoosh with a scuff of the feet, the camera lifts a little. |
| `FR_PLAT_AirJump` | A jump in the air: a bigger stretch, a rising lift sound on top of the whoosh, the view widens and the camera pops up. |
| `FR_PLAT_WallJump` | A wall jump, or any launch: a split-second grip on the wall, the camera thrown along the jump, a thump off the wall and a whoosh. |
| `FR_PLAT_Dash` | A dash: the view stretches wide, color fringes at the edges, the camera lags behind, a deep whoosh and a rumble. |
| `FR_PLAT_Land` | Landing, scaled by the fall: a light step after a hop; a squash, a dip of the view and a thud after a real jump; a deep drop of the view after a long fall. |

## Shooter: Every Bullet Has an Opinion {#ch13_shooter}
[edition: Pro]

[shot: S13-03 | Shooter: firing the rifle]

**Template:** First Person, Shooter variant. **Controls:** move WASD or left stick; look mouse or right stick; fire left mouse or RT; switch weapon Left Shift or Y; jump Space or A.

Each weapon feels different, hits and kills confirm themselves, grenades shake the world by distance, and the interface reacts too: the bullet counter kicks with each shot and burns red when empty, the life bar flashes when hurt, the score jumps.

**How it is hooked up.** The template's C++ weapon, projectile, enemy and player classes got recipe slots, marked `// FeelKit` in the source project, which play the recipes at the right moments. The template's interface classes play the interface recipes on their widgets. Jump pads were added to the level.

| Recipe | What it does |
|---|---|
| `FR_SHOOT_Pistol` | A pistol shot: a sharp crack over a low thump, the view kicks up and zooms in a touch, a short rumble. |
| `FR_SHOOT_Rifle` | A rifle shot, fired in bursts: a small kick per shot and a shake that builds up the longer the trigger is held. |
| `FR_SHOOT_Launcher` | A grenade launch: a deep boom, a heavy kick up and back, the view widens, a strong rumble. |
| `FR_SHOOT_DryFire` | The trigger pulled with nothing in hand: a dull click, nothing more. |
| `FR_SHOOT_Switch` | Switching weapons: a metal latch and a slight dip of the view. |
| `FR_SHOOT_Pickup` | Walking over a weapon pickup: a short chime and a light tap on the controller. |
| `FR_SHOOT_Impact` | A bullet hits the world: a metallic ping over a dull thud, and a scorch mark that fades. |
| `FR_SHOOT_Hit` | The player hits an enemy: a crisp tick, the enemy flashes white and freezes for a moment, the damage pops up over it. |
| `FR_SHOOT_Kill` | The player takes an enemy down: a chime, a red flash on the enemy, a brief freeze into slow motion, a small zoom and a rumble. |
| `FR_SHOOT_Explosion` | A grenade explodes, scaled by the player's distance: a crunch over a low boom and a hard shake; up close, a freeze, a warm flash and color fringes. |
| `FR_SHOOT_EnemyFire` | Anyone else firing: the shot sounds from the shooter and fades with distance. No camera effects. |
| `FR_SHOOT_Hurt` | The player is hit: the view is knocked away from the shot, a red pulse at the edges, a body hit and a rumble. |
| `FR_SHOOT_Death` | The player dies: time slows, the color drains, the view closes in, a heavy fall and a long rumble. |
| `FR_SHOOT_JumpPad` | Stepping on a jump pad: a rising whoosh, the view widens on the way up, the camera pressed down for a moment, a smooth rumble. |
| `FR_SHOOT_Land` | Landing from a real drop: a camera dip and a thud that grow with the fall. Small steps stay silent. |
| `FR_SHOOT_HudShot` | One shot fired: the bullet counter kicks down; when the magazine runs low it flashes red with every shot. |
| `FR_SHOOT_HudEmpty` | The magazine ran dry: the bullet counter shakes and burns red. |
| `FR_SHOOT_HudReload` | The magazine is full again: the counter swells once and brightens, with a soft click. |
| `FR_SHOOT_HudHurt` | Life lost: the life bar shakes and flashes red. |
| `FR_SHOOT_HudScore` | A score changes: the number jumps and lands; gold for the player's point, red when the player's team lost someone. |

**Project setup it relies on:** the accumulator `ShotHeat` (maximum 8, decay 10 per second after 0.12 s), which the rifle's shake reads, and the C++ changes to the template.

## Horror: Heartbeat {#ch13_horror}
[edition: Pro]

[shot: S13-04 | Horror: sprinting through the corridor]

**Template:** First Person, Horror variant. **Controls:** move WASD or left stick; look mouse or right stick; sprint hold Left Shift or LB; jump Space or A.

Quiet feedback that builds dread: the sprint wears the player out, failing lights buzz and drop out, doorways creak, and scare spots hit hard. The sprint meter reacts as well.

**How it is hooked up.** Sprinting plays a sustained recipe that lasts as long as the sprint, and running out of stamina plays a sustained out-of-breath recipe until it refills ([Ref: ch07_sustain]). Lamps, doorways and scare spots carry **Feel Trigger** components that play their recipes when the player comes near or steps through. The template's sprint meter plays the interface recipes.

| Recipe | What it does |
|---|---|
| `FR_HOR_Sprint` | While sprinting: the view widens a touch, sways with the run, and heavy footfalls carry through the floor. |
| `FR_HOR_OutOfBreath` | Out of breath: a heavy heartbeat, the edges close in, color drains a little, the world sounds muffled, and the view heaves with each breath. |
| `FR_HOR_LightUnease` | Walking under a failing light: the lamp buzzes, surges and drops out for a moment, and the color goes cold. |
| `FR_HOR_Doorway` | Stepping through a doorway: the frame creaks somewhere above. |
| `FR_HOR_Scare` | A scare spot: the flashlight dies, the screen nearly goes black for a moment, a low boom, the heart jumps and the camera flinches back. |
| `FR_HOR_Land` | Landing after a real drop: a soft thud and a small dip that grow with the fall. |
| `FR_HOR_HudBreathless` | The sprint meter ran empty: it beats red with the heartbeat until it has refilled. |
| `FR_HOR_HudRecovered` | The sprint meter is full again after running empty: it brightens once. |

The Platformer, Shooter and Horror recipes ship with FeelKit Pro in **FeelKit Content** > **Demos**, so they can be opened and studied, or copied into a project, without the source projects.
