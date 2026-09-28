# FeelKit Phase 6B: Demos, Design

| Field | Value |
|---|---|
| Date | 2026-09-18 |
| Status | Action/RPG kit built 2026-09-18; user checklist `Docs/FeelKit_Phase6B_UserTests.md` |
| Decisions | D-052 kits for Unreal's templates, D-053 all five demos before release, D-054 Action/RPG first, D-055 videos decided in 6D |
| Sources | `Docs/FeelKit_Demos.md` (approved demo plan), `Docs/Research/Adjacent_Systems_and_Demos_2026-09-16.md` section 5, the Combat template in the host project |

## 1. What a demo kit is

A buyer creates a project from one of Unreal's templates (for example Third Person with the Combat variant), enables FeelKit, and follows the kit's short guide. The guide adds FeelKit to the template in a few minutes: markers in the template's animations and one FeelKit node in a few of its Blueprint events. Nothing of Epic's is shipped.

Each kit ships inside the plugin, under `/FeelKit/Demos/<Genre>/`:

| Part | What it is | Who makes it |
|---|---|---|
| Recipes | The demo's recipes, written as JSON by me, imported by you (as for the library) | JSON: me. Assets: you |
| Feel Map | Which event plays which recipe, with variants | Me as a step list, you as the asset |
| Small samples | Only what the template lacks, for example a hit-flash material | Step list: me. Asset: you |
| Guide | "Add FeelKit to the Combat template": every click, in the same style as our checklists | Me |

**Off/On:** the template's own feedback (its camera shakes and spark effect) stays in place. `feel.Enabled 0` switches FeelKit off, so every moment can be shown as "the template as it ships" against "the template with FeelKit". That is the comparison the videos need (6D).

## 2. The Action/RPG kit, "Weight Class" (Combat template)

### What the template gives us

- **Combo attack** (`AM_ComboAttack`): three swings, sections Melee01, Melee02, Melee03, each with an attack trace (0.467 s, 1.467 s, 2.400 s).
- **Charged attack** (`AM_ChargedAttack`): a Charge section that loops while the button is held (0.333 to 0.8 s), then the strike (trace at 1.161 s).
- **Blueprint events:** the player's *Dealt Damage* (damage, impact point) and *Received Damage* (damage, impact point, direction); the enemy's *Received Damage* and *On Enemy Died*; the training dummy's *On Dummy Damaged*.
- **Every hit deals the same damage (1.0)**, and the Blueprints cannot see which swing it was. So "light, heavy, finisher" cannot come from the damage.

### How the kit tells the swings apart (no template code changes)

Each swing in the animations gets a FeelKit marker at its attack frame that sets a named value, **Swing**: 0.35 for swing 1, 0.65 for swing 2, 1.0 for the finisher, and 1.0 plus **Charged** = 1 for the charged strike. When the hit lands, the one FeelKit node in *Dealt Damage* or *Received Damage* sends a hit event; the hit recipes read Swing and Charged and scale themselves. A swing that misses only plays its swing sound and a light camera nudge.

### The moments

| Moment | When | What the player feels |
|---|---|---|
| A1 Light hit | Swing 1 connects | Short freeze on both fighters, small camera punch along the swing, slash sound, hit flash on the enemy, small rumble |
| A2 Heavy hit | Swing 2 connects | Same recipe, stronger: longer freeze, bigger punch, a small FOV kick |
| A3 Finisher | Swing 3 connects | Heavier again, plus a chromatic aberration pulse and a deeper sound |
| A4 Charge and release | Holding the charged attack, then the strike | While holding: a rising hum, growing shake and rumble (sustained). On the strike: a crit hit with slow motion, a flash and a number pop |
| A5 Enemy reaction | The enemy receives damage | Hit flash, squash on the enemy, a number pop over it, a push away from the hit direction |
| A6 Enemy death | An enemy dies | A short slow-motion beat, colour drains for a moment, a heavy thud |
| A7 Combo streak | Hits that follow each other quickly | Each hit in a streak lands a little harder; a streak counter pops (uses the Combo accumulator from the library) |
| A8 Taking damage | The player receives damage | Camera pushed away from the hit, red wash at the edges, low rumble |
| Swing (miss) | Every swing | A whoosh and a light camera nudge |

The template has no pickups, no stagger and no "last enemy" signal, so the research's pickup, stagger and last-enemy moments are left out of this kit. Pickups and streak pitch appear in the Platformer kit.

### What the buyer adds (the guide)

| Where | What |
|---|---|
| `AM_ComboAttack` | Three FeelKit "Set Feel Value" markers (Swing 0.35, 0.65, 1.0) and three "Play Feel" markers for the swing whoosh |
| `AM_ChargedAttack` | A "Play Feel (Window)" marker over the Charge section (sustained charge), and a "Set Feel Value" marker at the strike (Swing 1.0, Charged 1) |
| `BP_CombatCharacter` | One node in *Dealt Damage* (hit landed), one in *Received Damage* (player hurt) |
| `BP_CombatEnemy` | One node in *Received Damage* (enemy reaction), and *On Enemy Died* bound to one node (death) |
| `BP_CombatDummy` | One node in *On Dummy Damaged*, so the training dummy reacts like an enemy |
| Project Settings | The Swing, Charged and Combo values (accumulators) |

## 3. Small additions to FeelKit this needs

These are gaps the demo exposed; each is general-purpose, not demo-only.

1. **"Set Feel Value" anim notify:** sets a named accumulator on the animated actor (or globally) at a point of an animation. Lets animations tell recipes what kind of move this is without any Blueprint.
2. **Recipes read the instigator's values too:** a recipe played on the enemy, with the player as instigator, reads the player's Swing. Today a recipe reads its target's value, then the global one; the instigator is added between the two.
3. **Hit Flash step (overlay):** flashes a mesh with a colour through Unreal's overlay material slot, so it works on any mesh, including characters whose materials have no flash parameter (Manny). Ships with a small sample material you create from steps.

### Changes made while building (2026-09-18)

- **User direction:** trigger FeelKit with anim notifies on each animation. Done: markers mark each swing and play the whoosh; the hit itself still comes from the template's damage events, because a marker cannot know whether the swing connected (hit effects on a miss would feel wrong). The swing strengths are one number per swing marker, to be tuned by feel; the user suspects light, heavy and finisher may feel best the same.
- **Charge:** the template loops the Charge section while the button is held, so an animation window would stop and restart every loop. Instead each loop adds 0.34 to a ChargeLevel value (Set Feel Value in **Add** mode) and plays a pulse that grows with it.
- **Events:** FeelKit now has four built-in starter events, `Feel.Event.Hit.Landed`, `Feel.Event.Hit.Received`, `Feel.Event.Hurt`, `Feel.Event.Death` (tags in code, because plugin tag files are not loaded automatically in UE 5.6).
- **Player hurt** uses the library recipe `FR_Danger_DirectionalDamage` through the Feel Map.
- **Values:** Swing, Charged and ChargeLevel accumulators (decay 0); the streak uses the library's Combo accumulator, added to in Dealt Damage.

### Feel Switch: feel off and on while playing (agreed 2026-09-22, D-057)

User idea: every demo starts with a way for players to turn the feel off and on, so they can see and feel the difference. Agreed design:

- **Part of FeelKit** (Lite and Pro), not demo-only: an actor **Feel Switch** (`AFeelSwitch`, FeelCore) placed in a level. No input assets or Blueprint changes needed. Buyers can use it to show playtesters, their team or a publisher the before and after in their own game.
- **Off means only FeelKit:** the template's own effects stay, so OFF is the game as it ships. Uses the existing `feel.Enabled` switch (stops playing effects, restores slowed time, nothing new plays).
- **Blueprint nodes:** Set Feel Enabled, Toggle Feel, Is Feel Enabled; the actor has an On Feel Switched event (also fires when `feel.Enabled` is changed from the console).
- **Keys:** keyboard **Tab** and the controller **View / Share** button (`Gamepad_Special_Left`), both changeable on the actor. Checked 2026-09-22: no Input Mapping Context in the project uses either (F is used by the Side Scrolling template's Interact; F1 to F4 are editor view modes in PIE). Recheck when the First Person template is added. Keys are read from the player's own key state, so it works next to Enhanced Input or any other input setup, and FeelCore stays free of Enhanced Input.
- **Start card:** demo title, one line of description and how to switch; the game is not paused; it fades after about 8 seconds or on any key or button.
- **Corner badge:** always shows FEEL: ON or FEEL: OFF with a small green or grey dot. **Not intrusive (user):** switching confirms on the badge itself (it briefly grows and brightens, then settles); nothing appears in the middle of the screen.
- **Look:** quiet, like Unreal's own on-screen text: dark translucent panel, white text, colour only for the ON/OFF dot. Built in Slate, no assets. Shown per local player; nothing on a dedicated server. A Show Built-in UI option lets buyers hide it and use the event for their own UI.
- **Every level start begins ON** by default (option on the actor); the choice is not saved.
- **Limitation:** the switch is process-wide, so multiplayer windows inside one editor switch together; separate machines are independent.
- **Demos:** the Action/RPG level gets a Feel Switch by script (with backup); every later demo too; the buyer guide gets one step.

## 4. Order of work for the Action/RPG kit

1. The three additions, with automated tests.
2. The kit recipes as JSON and the Feel Map as a step list.
3. Your part: import the recipes, create the Feel Map and the hit-flash material, follow the guide in this project's Combat template.
4. Tuning together in PIE until it feels right; then the kit is done and the same pattern repeats for the next demo.

## 4b. The Platformer kit, "Bounce Feel" (Platforming variant, built 2026-09-22)

### What the template gives us
`BP_PlatformingCharacter` (C++ `APlatformingCharacter`): jump with hold, double jump, wall jump (`LaunchCharacter` away from the wall), coyote time, dash (`AM_Dash` montage, gravity off). The level holds only the course (135 static meshes, no pickups or hazards), so the demo is movement feel. Only effect of its own: the jump trail (`NS_Jump_Trail`), kept.

### How it is hooked up (no Blueprint graph changes)
- **Feel Trigger component** on the character with four entries. New general events added to the component (D-058): **Jumped** (from the ground, including a late jump off a ledge), **Air Jumped** (value: jump number), **Launched** (Launch Character: wall jumps, jump pads, knockbacks; value: launch speed; Direction: launch direction). Landed already existed (value: landing speed).
- **Dash:** a Play Feel marker at the start of `AM_Dash`.
- **Feel Switch** in the level ("FeelKit Demo: Bounce Feel").

### The moments (deliberately strong first pass)
| Moment | Recipe | What you get |
|---|---|---|
| Jump | FR_PLAT_Jump | stretch up, quick whoosh and a foot scuff, small camera lift, light rumble tick |
| Double jump | FR_PLAT_AirJump | bigger springy stretch, rising lift sound on the whoosh, FOV widens, camera pops up |
| Wall jump | FR_PLAT_WallJump | 45 ms grip freeze, camera thrown along the launch direction, squash, wall thump + whoosh, colour fringe, rumble |
| Dash | FR_PLAT_Dash | FOV +12, colour fringe, light vignette, camera lags back, deep whoosh, rumble |
| Landing | FR_PLAT_Land | scaled by fall speed (300 to 2000 cm/s): step sound always; from a normal jump (about 600) a squash, camera dip and soft thud; from a long fall (above about 1500) also a heavy impact, a 50 ms freeze and a shake |

### What the buyer adds (guide)
Add a Feel Trigger component to their platforming character with the four entries (event, recipe, value parameter); add the Play Feel marker to the dash montage; drag a Feel Switch into the level.

## 4c. The Shooter kit, "Every Bullet Has an Opinion" (First Person template, Shooter variant, started 2026-09-22)

### Host project
`B:/NewUE5Project/FeelDemoFP`, created from the engine's C++ First Person template (`B:/UE_5.6/Templates/TP_FirstPerson`) with the engine's own project creator (`GameProjectUtils::CreateProject`, run by the host-project tool `DiagFeel.CreateProjectFromTemplate`), so it is exactly what New Project makes, feature packs included. FeelKit is not copied: the project loads it from `../GameFeelDev/Plugins` (AdditionalPluginDirectories), one plugin source for both projects. Levels: `Lvl_Shooter`, `Lvl_Horror`.

### What the template gives us
Pistol, rifle (full auto) and grenade launcher (Blueprint subclasses of `AShooterWeapon`), projectiles (`AShooterProjectile`: single hit or explosion), enemies (`AShooterNPC`, ragdoll on death), the player (`AShooterCharacter`, HP, death). Almost no feedback of its own: bullets stick in walls with a wiggle, the explosion is a growing sphere, no hit reactions, no impact sounds.

### How it is hooked up
The template does not expose enemy hits or deaths (its NPC damage skips Unreal's damage event, and its death event is a plain C++ member), and it is the C++ template, so the kit adds a few clearly marked lines to the template code (`// FeelKit`), the way a buyer following the guide would:
- `AShooterWeapon`: **Fire Feel** (played on the player when the player fires) and **Other Fire Feel** (played when anyone else fires it, such as an enemy; sound only, no camera). Every shot also adds 1 to the **ShotHeat** accumulator of the shooter, so sustained fire can build up.
- `AShooterProjectile`: **Impact Feel** (hits on the world: sound, scorch at the hit point) and **Explosion Feel** (on the local player's camera, with the distance to the explosion as the Distance parameter).
- `AShooterNPC`: **Hit Feel** and **Kill Feel**, played on the player who dealt the damage (hit confirmation), with the enemy as the instigator for its own flash and freeze. Only for damage from a player.
- `AShooterCharacter`: **Hurt Feel** on the player, with the direction from the damage source; **Death Feel**; after the first play test also **Pickup Feel** (walking over a weapon pickup), **Switch Weapon Feel** and **No Weapon Feel** (pulling the trigger empty-handed). These three stay quiet on purpose: a sound, at most a light tap or a slight dip of the view.
- The recipe slots are set on the weapon and projectile Blueprints and the enemy and player Blueprints by the kit script.
- **Feel Switch** in `Lvl_Shooter`.

Why not only markers or the Feel Trigger: the fire montage is shared by the weapons, and the enemy offers no event to hook; FeelKit's general fallback (a play without a player goes to the first local player) is right for explosions but would kick the player's camera for enemy shots, hence the separate Other Fire Feel.

### Sounds
Kenney Sci-fi Sounds (CC0, already on disk): lasers for shots, explosion crunches, metal impacts; Kenney Impact and Interface packs for hit and kill confirmation. The weapons are stylized, so sci-fi shots fit.

## 4d. The Horror kit, "Heartbeat" (First Person template, Horror variant, started 2026-09-22)

### What the template gives us
`AHorrorCharacter`: walk (250 cm/s), sprint with a stamina meter (3 s of sprint at 600 cm/s; when it runs out the character is slowed to 150 cm/s until the meter is full again), a flashlight (spot light on the character). `Lvl_Horror`: 17 `Light` actors (spot light + mesh, some flickering), 4 door frames (plain geometry, nothing opens), walls. **No threat and no scares.**

### Agreed direction (user, 2026-09-22)
Atmosphere plus a few scares: feel on what the template has, plus 3 to 4 scare spots placed in the level. No new gameplay.

### Moments (reworked 2026-09-23 after the first play test)
| Moment | Recipe | Hook |
|---|---|---|
| Sprinting | FR_HOR_Sprint (sustained): the view widens a touch, sways with the run, heavy footfalls | template C++ `AHorrorCharacter`, **Sprint Feel** |
| Out of breath, until the stamina has recovered | FR_HOR_OutOfBreath (sustained): heartbeat, the edges close in, colour drains, the world sounds muffled, the view heaves | template C++, **Out Of Breath Feel** |
| Walking under a failing lamp | FR_HOR_LightUnease: the lamp buzzes, surges and drops out; the colour goes cold | a sphere on each flickering `Light` actor with a Feel Trigger that **plays on the lamp**, so the sound comes from it and you can see the cause |
| Stepping through a doorway | FR_HOR_Doorway: the frame creaks above you | Trigger Box in each of the four doorways, plays on the box (the sound is at the doorway) |
| Landing after a real drop | FR_HOR_Land: a soft thud and a dip, growing with the fall; small steps stay silent | Feel Trigger on the character (Landed, LandSpeed) |
| Scare spots (three doorways) | FR_HOR_Scare (long cooldown): the flashlight dies, the screen dips nearly black, a low boom, the heart jumps, the camera flinches | Trigger Box that plays on the player, because the flashlight is the player's |

**Why the first version felt random (user, 2026-09-23):** the unease played from invisible boxes that were larger than the lamps and the sound was 2D, so it seemed to come from nowhere. Every moment now has a visible source and every world sound is positioned with attenuation.

**Deliberately not done:** footsteps while walking (the template has no walking sound or animation notify to hang them on; it would need a step timer in the character code, which is sound design rather than feel) and a jump effort sound (no usable breath sound in the CC0 packs).

### Hooks
- `AHorrorCharacter` gets **Sprint Feel** and **Out Of Breath Feel** (both sustained recipes, released when the state ends). Marked `// FeelKit`, like the Shooter.
- Trigger Boxes: the engine's own `ATriggerBox` with a Feel Trigger component added to the placed actor, which is what a buyer would do in their own level. Placed by the kit script (lights: every Light actor with flicker on; scares: at door frames).
- **Feel Switch** in `Lvl_Horror`.

## 4e. The UI kit, "Juicy Menus" (built 2026-09-23, dropped the same night, D-064)

**Dropped.** A made-up menu with no game behind it showed UI feel out of context, and "Juicy" in the name sat next to a
competitor called Game Juice Pro. UI feel moved into the Shooter HUD (bullet counter, life bar, score) and the Horror
sprint meter (section 4g). The menu level, widget, code and recipes are removed; a copy is in
`Backups/2026-09-23_170544_ui_demo_removed`. The record below is kept for reference.

### Why this one is different
The other four kits hook into an Unreal template. No template ships a menu, so this kit brings its own: a menu level in
GameFeelDev (`/Game/Demo_UI/Lvl_Menu`) with the widget `WBP_FeelMenu` and a small C++ class behind it. The widget lives in
the demo project, not in the plugin, because a plugin asset cannot point at a class in a project.

### Agreed direction (user, 2026-09-23)
Menu with a score tally, in the dark card look of the Feel Switch.

### What the menu is
One card in the middle of the screen (reworked 2026-09-23 after the first look: "the menu UI has to be looking a lotta
more clean"): a heading with a rule under it, then the five buttons on the left and the score inset on the right, and a
footer line about the switch. The buttons carry a one pixel edge that turns green under the pointer; Claim reward is
dimmed and says "count first" until the score has been counted; the score shows the total, a row of marks that fill as the count goes on, and
the "+score" chips flying off beside it. A message panel sits above the card.

The Feel Switch's start card is turned off in this level, because it would cover the menu; the badge stays in the corner
and the footer line says what the switch does.

The menu runs the same with the feel off: buttons still work, the score still counts, the chips still float. FeelKit only
adds the reactions.

### Moments
| Moment | Recipe | Where it comes from |
|---|---|---|
| Pointing at a button | FR_UI_Hover: a small lift and a soft tick | `UFeelMenuDemo`, On Hovered |
| Pressing a button | FR_UI_Press: squash inward, a white flash, a click, a light rumble | On Clicked |
| Claim reward while locked | FR_UI_Denied: a red shake on the button and a thud | On Clicked before the score has been counted |
| A message arriving | FR_UI_Notice: the panel drops in from above and settles | every message |
| One step of the count-up | FR_UI_Tally (parameter Heat): the number kicks, warms and ticks; the kick grows and the tick climbs through four pitches as the count goes on | fourteen steps, each faster than the last |
| Each won amount | FR_UI_Chip (parameter Heat): the "+score" pops in beside the panel | one per step |
| The last step | FR_UI_TallyDone: the panel swells, the screen brightens for a moment, the total lands | after the fourteenth step |
| Claiming the reward | FR_UI_Vault: the panel heaves, a green flash, a low thump and a chime; the score doubles | Claim reward, once the score is counted |
| Leaving for a level | FR_UI_Transition: the screen wipes to black with a whoosh, then the level opens | the two demo buttons |

The count-up gets its rising pitch from four tick tracks, each switched on for its own quarter of Heat, because a step's
pitch is a property of the sound and only the intensity follows a parameter.

### How it is built
- `GameFeelDev/Source/GameFeelDev/FeelMenuDemo.h/.cpp`: `UFeelMenuDemo` (what the buttons do, the count, the chips, the
  messages) and `AFeelMenuGameMode` (no pawn; shows the menu on the player's own screen layer, under the Feel Switch).
- `FeelMenuDemoBuilder.cpp`: builds the layout of `WBP_FeelMenu` through the engine's Blueprint and UMG code, so the
  Widget Blueprint can be opened and changed in the editor afterwards (hard rule 2). Rerunning rebuilds the layout inside
  the same asset.
- `Tools/FeelKitScripts/build_ui_kit.py`: recipes from JSON, the widget, the recipe assignments, the level, its game mode,
  a player start and the Feel Switch.
- Sounds: the existing UI hover, click, denied, pickup, level-up and whoosh, plus three new CC0 picks measured with
  `DiagFeel.SoundStats` (`S_FK_UI_Tick`, `S_FK_UI_Reward`, `S_FK_UI_Thump`).
- Buttons are not keyboard focusable, so Tab reaches the Feel Switch instead of moving focus between buttons.
- Buttons draw nothing and only take the pointer. A face panel inside each button (not hit-testable) draws it, changes
  look on hover and press, and takes every reaction, so the clickable area never moves under the pointer (I-044).
- Before opening a level the menu puts the mouse back the way the project starts a game, because the game window keeps
  the menu's input settings across levels (I-045).
- Button names (D-063, after the first play): **Count score** (was "Tally the run") and **Claim reward** (was "Vault").
  Widget and recipe names still use Tally and Vault.
- The card is a fixed width and takes its height from its content, so nothing is cut off; every panel is a rounded box
  with a one pixel edge, which stays crisp at any window size.

### What it shows that the other demos do not
Widget Punch, Widget Shake and Widget Flash, a parameter that escalates within one moment, and the same screen with every
reaction switched off.

## 4f. Action/RPG additions: camera and guards (2026-09-23, D-065, D-068, D-069)

### Camera
The template's camera sits 1 m behind the character, which then covers whatever it hits. Measured with
`DiagFeel.ARPGCameraStudy` (73 setups, the dummy's visibility at every frame of the combo): arm 350 cm, mounted 30 cm to
the right and 70 cm up, field of view 90; Default Camera Distance 350 (respawn), Death Camera Distance 550. Applied by
`Tools/FeelKitScripts/set_arpg_camera.py`. Camera effects are sized by how far they move the picture (knock, D-070):
FR_ARPG_HitLanded moves the target about as far on screen as the first version did at 1 m (8.0% of the screen width
against 8.2%) with one fast Kick push (turn 8.5, push 40 cm) and no rebound; the first version's spring swung back past
rest and smeared the frame several times under motion blur (D-068). Swing lean and guard recipes are scaled the same way.

### Controls on screen
Each level's Feel Switch lists the level's controls under its badge (D-071), set by
`Tools/FeelKitScripts/set_level_controls.py` from the keys each level's player controller really maps.

### Guards
No block animation exists in the project, so a raised guard is an energy shield in front of the character
(`CombatGuardComponent`, a static mesh component using M_FK_GuardShield with Raise, Glow and ShieldColor). Attack traces
ask the guard before dealing damage, so a blocked hit never also plays the hit recipes.

| What happens | When | Reaction |
|---|---|---|
| Block | A hit from the front meets a raised guard | FR_ARPG_GuardBlock on the shield, Power = the attacker's Swing value (heavier swings clang deeper, push harder, freeze longer); the attacker's swing freezes for a moment |
| Guard break | A charged strike (Charged marker) meets the guard | FR_ARPG_GuardBreak on the defender; the guard stays down (enemy 2.5 s, player 1.6 s) |
| Parry | The guard went up within 0.2 s before the hit (player only) | FR_ARPG_Parry on the player with the attacker as instigator; the attacker is interrupted, pushed back and staggered |

The player holds IA_Block (Left Shift, left trigger), walks slower while guarding, and attacking lowers the guard for
the swing without reopening the parry window. The guard enemy (`CombatGuardEnemy`, BP_CombatGuardEnemy copied from
BP_CombatEnemy) runs without the template AI: it faces the player, guards, and every 2.5 to 4.5 s within 2.3 m attacks
(30% charged strikes). A fresh one replaces it 6 s after it dies. Built by `Tools/FeelKitScripts/build_arpg_guard.py`;
checked by `DiagFeel.ARPGGuardPlaythrough` (18 checks by hit points and recipes).

## 4g. HUD feel in the Shooter and Horror kits (2026-09-23, D-064)

| Moment | Recipe | Widget | Hook |
|---|---|---|---|
| A shot | FR_SHOOT_HudShot (Ammo: red below a quarter) | Image_33, the bullet strip | UShooterBulletCounterUI::UpdateBulletCounter |
| Magazine refilled (in this template, the last shot) | FR_SHOOT_HudReload | Image_33 | same |
| Magazine at 0 (not in this template, which refills at once) | FR_SHOOT_HudEmpty | Image_33 | same |
| Life lost | FR_SHOOT_HudHurt | Border_0 around the life bar | UShooterBulletCounterUI::Damaged |
| A team's number goes up | FR_SHOOT_HudScore (PlayerScored: gold or red) | Team1Score / Team2Score | UShooterUI::UpdateScore |
| Sprint meter empty until full | FR_HOR_HudBreathless (sustained, two beats per loop) | Border_0 around the meter | UHorrorUI::OnSprintMeterUpdated |
| Sprint meter full again | FR_HOR_HudRecovered | Border_0 | same |

The hooks call the template's own Blueprint events first, then play on the widget named in the class defaults.

## 5. The other four demos (later, same pattern)

| Demo | Template | Hooks we expect to use |
|---|---|---|
| Platformer "Bounce Feel" | Third Person, Platforming variant (in this project) | Built, section 4b |
| Shooter "Every Bullet Has an Opinion" | First Person, Shooter variant (project FeelDemoFP) | Section 4c |
| Horror "Heartbeat" | First Person, Horror variant (project FeelDemoFP) | Section 4d |
| UI "Juicy Menus" | Its own menu level in GameFeelDev | Built, section 4e |

Each gets its own section in this document before work on it starts.
