# FeelKit Phase 6B: demos, what to do

Updated 2026-09-23 (night).

## Where things stand

| Item | State |
|---|---|
| Action/RPG demo "Weight Class" (`Lvl_Combat`) | Approved 2026-09-19; camera, guards and parries played 2026-09-23 ("good job"). **Done.** Hit approved again 2026-09-23 ("That's it we got it!!!"). |
| Feel Switch (feel off and on while playing) | **Done.** Approved 2026-09-22. In all four demo levels. |
| Platformer demo "Bounce Feel" (`Lvl_Platforming`) | **Done.** Landing change approved 2026-09-23 ("nice"). |
| Shooter demo "Every Bullet Has an Opinion" (`Lvl_Shooter` in **FeelDemoFP**) | **Done.** HUD reactions approved 2026-09-23 ("nice"). |
| Horror demo "Heartbeat" (`Lvl_Horror` in **FeelDemoFP**) | **Done.** Sprint meter approved 2026-09-23 ("nice"). |
| UI demo "Juicy Menus" | **Dropped** (your decision 2026-09-23). UI feel now lives in the Shooter and Horror HUDs. The menu level and its code are removed; a copy is in `Backups/2026-09-23_170544_ui_demo_removed`. |

## What you need to do now

Nothing is open: all four demo levels are approved. The sections below stay for reference. In every level, **Tab**
(controller: View / Share) turns the feel off and on.

### Earlier: camera, guards and parries (played 2026-09-23, good)

What changed:
- **The camera sits 3.5 m behind the character instead of 1 m** (30 cm to the right, field of view 90), so you see the
  character and what it hits. Chosen by measuring 73 camera setups at the moment of impact: the target went from about
  25% visible to about 69%.
- **Punches hit as hard as before, without the double smear.** After your "we lost a lot of feel": each punch now moves
  the picture as far as the first version did (measured 8.0% of the screen width against 8.2% before; the softened
  version only managed 1 to 2%). The difference to the first version: the camera jolts once and eases back slowly,
  instead of swinging back and forth, so there is one short blur at the moment of impact and then a clear picture.
  The lean into each swing, blocks and guard breaks were scaled up the same way.
- **Controls on screen:** under the FEEL badge in the top right corner, every level now lists its controls.
- **Guards.** Hold **Left Shift** (controller: **left trigger**) to raise a blue energy shield. Hits from the front meet
  the shield instead of you. Raise it **just before** a hit lands and it becomes a **parry**. You walk slower while
  guarding, and attacking lowers the shield for the swing.
- **A guarding enemy** stands about 4.5 m to the left of the training dummy (as you face it from the start). Its shield
  is orange. It turns to face you, keeps its guard up, and every few seconds lowers it to attack you.

1. Open `Lvl_Combat` (`/Game/Variant_Combat/Lvl_Combat`) and press **Play**. Controls: attack **left mouse** (controller:
   right shoulder), charged attack **hold right mouse** (right trigger), block **hold Left Shift** (left trigger).
2. Walk to the guarding enemy and try, in this order:
   - **Hit its guard with the combo.** Each hit clangs off the shield, sparks fly, the shield flares, your swing stops dead
     for a moment and the enemy slides back. The third, heaviest swing clangs deeper and pushes harder. It loses no
     health.
   - **Hit it with a charged attack.** The shield shatters: a freeze into slow motion, a warm flash, the enemy buckles.
     For about two and a half seconds it has no guard: now your hits land and hurt it as usual.
   - **Hold block and let it attack you.** Its swings clang off your shield and push you back; you lose no health.
   - **Parry:** let go of block, wait until it starts a swing, then press block just before the swing lands. A clear ring,
     a longer freeze, a white flash; the enemy flashes white, stops its attack and staggers.
   - **Its charged attack** (it winds up for a second or two first) breaks your guard if you only hold block. Parry it
     instead, or get out of the way.
   - **Walk behind it and hit its back:** the guard only covers the front.
   - When it dies, a fresh one takes its place a few seconds later.
3. Also fight the normal enemies and the dummy: the camera and the softer hit apply everywhere. Your block and parry
   work against every enemy.
4. Tell me: whether each punch lands like it used to, whether the camera distance and angle work, and whether block,
   guard break and parry each feel clearly different. The controls panel is in every level (sections 2 to 4 too); tell
   me if you want PlayStation button names next to the Xbox ones.

### Approved 2026-09-23: Platformer landings

What changed: when you land, the view only drops and then settles back slowly, like knees taking the weight. There is no
up-and-down shake any more. The harder the landing, the deeper and longer the drop: barely anything after a hop, a clear
dip after a normal jump, a deep drop with a short freeze after a fall from a high platform. Measured from four heights:
the view never goes above where it started.

1. Open `Lvl_Platforming` (`/Game/Variant_Platforming/Lvl_Platforming`), press **Play**.
2. Hop in place, do a normal jump, then drop from a medium ledge and from the highest platform you can find.
3. Tell me whether the drop feels right at each height. The character's own squash on landing still springs back a little
   (it did before); say if you want that calmer too.

### Approved 2026-09-23: Shooter HUD

What changed (the bullet counter with the life bar, and the team score):
- **Every shot** kicks the bullet counter down a little. **When the magazine is low** (under a quarter) it flashes red
  with every shot.
- **The last shot of a magazine:** the template reloads at once, so the counter swells, brightens and clicks.
- **Getting shot:** the life bar shakes and flashes red (on top of the template's own damage effect).
- **Score:** when an enemy dies, the enemies' number goes up (that is how the template counts), jumps and flashes gold;
  when you die, your number goes up, jumps and flashes red.

1. Open `Lvl_Shooter` (`/Game/Variant_Shooter/Lvl_Shooter`), press **Play**, pick up the rifle.
2. Fire in bursts, then hold the trigger through a whole magazine. Get shot a few times. Kill an enemy.
3. Tell me whether the HUD reactions are easy to notice without being distracting.

### Approved 2026-09-23: Horror sprint meter

What changed: when the sprint meter runs out, it beats red in time with the heartbeat until it has refilled, then
brightens once to say you can sprint again.

1. Open `Lvl_Horror` (`/Game/Variant_Horror/Lvl_Horror`), press **Play**.
2. Sprint (**Left Shift**) until you are out of breath, then wait until the meter is full.
3. Tell me whether the beat fits the heartbeat and the moment of "ready" is clear.

---

# Archive: approved checks (kept for reference)

**Nothing to do here.** These are the checks you already passed.

## Horror demo "Heartbeat" (approved 2026-09-23)

What changed after the first play test:
- **Sounds with no visible source are gone.** The unease no longer comes from invisible boxes. Each failing lamp now reacts itself: walk under it and it buzzes, flickers and drops out for a moment, and the sound comes from the lamp, so you can see what caused it.
- **The drone is gone.** It was the weakest sound. The lamps use a buzz and a crackle instead.
- **New:** doorways creak as you step through them, landing after a real drop gives a thud and a dip (small steps stay silent), and sprinting now has footfalls.
- **Unchanged:** sprinting, out of breath and the scare spots, which you liked.

1. Open `Lvl_Horror` in `B:\NewUE5Project\FeelDemoFP\FeelDemoFP.uproject`, press **Play**.
2. Controls: move **WASD**, look with the mouse, sprint **Left Shift** / **left shoulder**, jump **Space**.
3. Walk under the four flickering lamps, through the doorways, sprint until you are out of breath, jump off something, and find the three scare spots (in doorways, on both floors).
4. Press **Tab** to compare with the template as it ships.
5. Tell me if anything still sounds like it comes from nowhere.

**What the template does not offer**, so FeelKit has nothing to hang it on: footsteps while walking (there is no walking sound or animation marker in the template; it would need a step timer written into the character, which is sound design rather than feel), doors that open, a flashlight switch, and any kind of threat. If you want walking footsteps anyway, say so and I will add them.

## Shooter demo "Every Bullet Has an Opinion" (approved 2026-09-22)

This one lives in a **second project**, made from Unreal's First Person template (C++ version), because the shooter and horror demos need that template: `B:\NewUE5Project\FeelDemoFP\FeelDemoFP.uproject`. It uses the same FeelKit plugin as GameFeelDev (linked, not copied), so FeelKit changes apply to both.

1. Open `B:\NewUE5Project\FeelDemoFP\FeelDemoFP.uproject` (Epic Games Launcher > Unreal Engine > Library, or double-click it). If it asks to rebuild modules, answer **Yes** (it is already built, so this should not happen).
2. Open `Lvl_Shooter` (`/Game/Variant_Shooter/Lvl_Shooter`), press **Play**. The start card says "FeelKit Demo: Every Bullet Has an Opinion".
3. Controls (the template's, read from its input settings): move **WASD** / left stick, look with the mouse / right stick, fire **left mouse button** / **right trigger** (or right shoulder), switch weapon **Left Shift** / **top face button** (Xbox Y, PlayStation Triangle), jump **Space** / **A**. You start without a weapon: walk over the **weapon pickups** in the level to get the pistol, rifle and grenade launcher.
4. What to feel:
   - **Firing:** each gun has its own crack over a low thump and kicks the view; the rifle's shake builds up the longer you hold the trigger; the grenade launcher booms and kicks hard.
   - **Bullet impacts:** a metallic ping and a scorch mark where bullets hit.
   - **Hitting an enemy:** a crisp tick, the enemy flashes white and freezes for a moment, the damage number pops up.
   - **Killing an enemy:** a chime, a red flash, a short freeze into slow motion and a small zoom.
   - **Grenade explosions:** a crunch and a low boom, a hard shake up close (with a freeze, a warm flash and colour fringes), only a rumble far away.
   - **Getting shot:** the view is knocked away from the shot with a red pulse at the edges.
   - **Enemy shots:** you hear them from where they are, quieter with distance.
   - **Firing with no weapon, picking up a weapon, switching weapons:** a dull click, a short chime with a light tap, a metal latch with a slight dip (added after your first play).
5. Press **Tab** (or View / Share) to compare with the template as it ships (it has almost no feedback of its own).

## Feel Switch (approved 2026-09-22)

Works the same in both demo levels.

1. Open `Lvl_Combat` or `Lvl_Platforming`, press **Play**.
2. A card appears near the middle with the demo name and how to switch. It fades by itself after about 8 seconds, or as soon as you press any key or button.
3. Top right corner: a small **FEEL: ON** badge with a green dot.
4. Press **Tab**: the badge says **FEEL: OFF** (grey dot) and briefly grows to confirm; nothing appears in the middle. Play a moment: you get the template as it ships.
5. Press **Tab** again: back ON.
6. With a controller: the **View / Share** button (the small button left of the centre; Xbox View, PlayStation Create/Share) does the same.
7. Stop and start Play again: it always starts ON.

## Platformer demo "Bounce Feel" (approved 2026-09-22)

1. Open `Lvl_Platforming` (`/Game/Variant_Platforming/Lvl_Platforming`), press **Play**. The start card says "FeelKit Demo: Bounce Feel".
2. Controls (the template's): move **WASD** / left stick, jump **Space** / **A** (bottom face button), dash **Left Shift** / **B** (right face button). Press jump again in the air for the **double jump**. **Wall jump:** jump toward a wall and press jump again while touching it.
3. What to feel:
   - **Jump:** the character stretches up, a quick whoosh and a scuff of the feet, the camera lifts a little.
   - **Double jump:** a bigger stretch (no bounce-back since the snap fix), a rising sound, the view widens for a moment.
   - **Wall jump:** a split-second grip on the wall (a tiny freeze), the camera is thrown along the jump, a thump off the wall.
   - **Dash:** the view stretches wide with colour fringes at the edges, a deep whoosh, the camera lags behind.
   - **Landing** grows with the fall: a light step after a hop, a squash, camera dip and thud after a normal jump, and after a fall from a high platform a heavy impact with a short freeze and a shake.
4. Press **Tab** (or View / Share) to compare with the template as it ships.

---

# Archive: Action/RPG setup steps (done by you on 2026-09-18)

**Nothing to do here.** These are the steps you completed for the Action/RPG kit. They stay only because they are the draft of the buyer guide. Since then I rebuilt the recipes and sounds myself; `feel.Enabled` in the console is replaced by the Feel Switch (Tab).

## A. The kit's assets (about 20 minutes)

### A0 Preparation

1. Content Browser **Settings** > **Show Plugin Content** ticked (it should still be on).
2. Go to **All** > **Plugins** > **FeelKit Content**. Right-click > **New Folder**: `Demos`. Inside it: `ActionRPG`.

### A1 The hit flash material `M_FK_HitFlash`

This is what makes an enemy flash white when hit. It works on any character, whatever its own materials are.

1. Go to `/FeelKit/Samples/Materials` (the folder from Phase 6A). Right-click > **Material**, name it `M_FK_HitFlash`, double-click it.
2. Click the big result node on the right (**M_FK_HitFlash**). In **Details** > **Material**:
   - **Blend Mode**: **Translucent**
   - **Shading Model**: **Unlit**
3. Right-click the graph, type `Vector Parameter`, pick it. In **Details**, **Parameter Name**: `FlashColor`, **Default Value**: white (R 1, G 1, B 1, A 1).
4. Right-click the graph, type `Scalar Parameter`, pick it. **Parameter Name**: `FlashAmount`, **Default Value**: `0`.
5. Right-click the graph, type `Multiply`, pick it. Connect **FlashColor**'s top output (the white circle, RGB) to Multiply's **A**, and **FlashAmount**'s output to Multiply's **B**.
6. Connect the **Multiply** output to **Emissive Color** on the result node.
7. Connect **FlashAmount**'s output also to **Opacity** on the result node.
8. **Apply** (top left), then **Save**. Close the material.

The names `FlashColor` and `FlashAmount` must be spelled exactly like this.

### A2 Three values in Project Settings

1. **Edit** > **Project Settings** > **Plugins** > **FeelKit**, section **Accumulators**. `Combo` is already there from Phase 6A.
2. Press **+** three times and fill in each new entry (expand it with the small arrow):

| Name | Max Value | Decay Per Second | Decay Delay |
|---|---|---|---|
| `Swing` | `1` | `0` | `0` |
| `Charged` | `1` | `0` | `0` |
| `ChargeLevel` | `1` | `0` | `0` |

Decay 0 means the value stays until an animation marker changes it.

### A3 Import the five recipes

For each row: in `/FeelKit/Demos/ActionRPG`, right-click > **FeelKit** > **Feel Recipe**, name it exactly as in the table, then right-click it > **Import from JSON...** and pick `B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Demos\ActionRPG\<Name>.json`. **Ctrl+S**.

| Recipe | What it is |
|---|---|
| `FR_ARPG_Swing` | Every swing, hit or miss: a whoosh and a slight lean of the camera |
| `FR_ARPG_ChargePulse` | One loop of holding the charged attack: a hum, shake and rumble that grow |
| `FR_ARPG_HitLanded` | Your hit connects: light, heavy or finisher from one recipe; the charged strike adds a crit |
| `FR_ARPG_EnemyHurt` | What you hit reacts: white flash, short freeze, squash, streak counter |
| `FR_ARPG_EnemyDeath` | An enemy goes down: slow-motion beat, colour drains, heavy thud |

**Expected:** each import says "Imported ... into ...", with no missing asset references (it uses the Phase 6A sounds and the new `M_FK_HitFlash`).

### A4 The Feel Map `FM_ARPG`

1. In `/FeelKit/Demos/ActionRPG`, right-click > **FeelKit** > **Feel Map**, name it `FM_ARPG`, double-click it.
2. Under **Entries**, press **+** four times and fill in (expand each entry):

| Event | Recipe |
|---|---|
| `Feel.Event.Hit.Landed` | `FR_ARPG_HitLanded` |
| `Feel.Event.Hit.Received` | `FR_ARPG_EnemyHurt` |
| `Feel.Event.Hurt` | `FR_Danger_DirectionalDamage` (from the library, `/FeelKit/Library/Danger`) |
| `Feel.Event.Death` | `FR_ARPG_EnemyDeath` |

   Leave Required Tags, Intensity Scale and Priority as they are. The `Feel.Event` tags are built into FeelKit; pick them from the tag list.
3. **Save**. Close it.
4. **Edit** > **Project Settings** > **Plugins** > **FeelKit**, section **Events**, **Feel Maps**: press **+** and pick `FM_ARPG`.

---

## B. Plug it into the Combat template (about 25 minutes)

The template's files are in `/Game/Variant_Combat` (**All** > **Content** > **Variant_Combat**).

**How to place a marker in an animation:** open the montage; below the preview there is a **Notifies** track. Right-click on it at the spot described > **Add Notify** > pick the FeelKit notify (**Set Feel Value** or **Play Feel**). Click the new marker to edit it in **Details**. You can drag a marker to move it.

**Where the markers go:** use what you can see as landmarks: the section names at the top of the timeline (Melee01, Melee02, ...) and the template's own markers on the Notifies track (**DoAttackTrace** is the moment the sword hits). Positions do not have to be exact; only the order matters. The editor usually counts in frames, not seconds, so no numbers are needed.

### B1 The combo attack `AM_ComboAttack` (in `Variant_Combat/Anims`)

The combo has three swings, one per section: **Melee01**, **Melee02**, **Melee03**. Each section has one **DoAttackTrace** marker. For each swing, add two markers so that the swing reads, from left to right: **Set Feel Value** → **Play Feel** → the template's **DoAttackTrace**.

| Swing | Marker 1: Set Feel Value, right at the start of the section | Marker 2: Play Feel, a little before that section's DoAttackTrace |
|---|---|---|
| Melee01 | **Values**: press + twice: `Swing` = `0.35`, `Charged` = `0`. Mode **Set**, Scope **Owning Actor** | **Recipe**: `FR_ARPG_Swing`. Target **Mesh** |
| Melee02 | `Swing` = `0.65`, `Charged` = `0` | `FR_ARPG_Swing` |
| Melee03 | `Swing` = `1`, `Charged` = `0` | `FR_ARPG_Swing` |

**Save**.

### B2 The charged attack `AM_ChargedAttack`

Three sections: **Default** (the wind-up), **Charge** (loops while you hold the button), **Attack** (the strike, with the template's **DoAttackTrace**). Add five markers:

| Where | Notify | Settings in Details |
|---|---|---|
| Inside **Default**, near its start | **Set Feel Value** | `ChargeLevel` = `0`, `Charged` = `0`. Mode **Set** |
| Inside **Charge**, shortly after it starts | **Set Feel Value** | `ChargeLevel` = `0.34`. Mode **Add** |
| Inside **Charge**, same spot | **Play Feel** | `FR_ARPG_ChargePulse`. Target **Mesh** |
| At the start of **Attack** | **Set Feel Value** | `Swing` = `1`, `Charged` = `1`, `ChargeLevel` = `0`. Mode **Set** |
| Inside **Attack**, a little before **DoAttackTrace** | **Play Feel** | `FR_ARPG_Swing` |

Two markers at the same spot both fire. If they sit on top of each other and are hard to click, right-click the track name **Notifies** > **Add Notify Track** and put one of them on the new row. **Save**.

### B3 The player `BP_CombatCharacter` (in `Variant_Combat/Blueprints`)

Open it, **Event Graph**.

**Hit landed.** Find **Event DealtDamage** (it plays a camera shake). From the camera shake node's white output arrow:

1. Drag out, type `Add To Feel Accumulator`, pick it. **Accumulator Name**: `Combo`, **Amount**: `1`. **Actor**: right-click the graph, type `self`, pick **Get a reference to self**, connect it.
2. From its white output arrow, drag out, type `Send Feel Event`, pick it. **Event**: `Feel.Event.Hit.Landed`.
3. **Target**: drag from the pin, type `Make Feel Target From Actor`, pick it, and connect **self** to its **Actor**.
4. **Context**: drag from the pin, type `Make FeelPlayContext`, pick it. Connect **Event DealtDamage**'s **Impact Point** to its **Location**, and **self** to its **Instigator**. (Click the small arrow at the bottom of the Make node if some pins are hidden.)

**Player hurt.** Find **Event ReceivedDamage** (it spawns sparks and plays a camera shake). From the last node's white output arrow:

5. Add **Send Feel Event**, **Event**: `Feel.Event.Hurt`, **Target**: Make Feel Target From Actor with **self**.
6. **Context**: Make FeelPlayContext with **Location** = the event's **Impact Point** and **Direction** = the event's **Damage Direction**.

**Compile**, **Save**.

### B4 The enemies `BP_CombatEnemy` (in `Variant_Combat/Blueprints/AI`)

Open it, **Event Graph**.

**Enemy hit.** Find **Event ReceivedDamage** (it spawns sparks). From the Spawn System node's white output arrow:

1. Add **Send Feel Event**, **Event**: `Feel.Event.Hit.Received`, **Target**: Make Feel Target From Actor with **self**.
2. **Context**: Make FeelPlayContext with **Location** = **Impact Point**, **Direction** = **Damage Direction**, and **Instigator** = a **Get Player Character** node (right-click the graph, type it; Player Index 0).

**Enemy death.**

3. Right-click the graph, type `Event BeginPlay`, pick it.
4. From its white output arrow, type `Bind Event to On Enemy Died`, pick it (Target: self is automatic).
5. From its red **Event** pin, drag out and pick **Add Custom Event**. Name it `OnDiedFeel`.
6. From `OnDiedFeel`'s white output arrow: **Send Feel Event**, **Event**: `Feel.Event.Death`, **Target**: Make Feel Target From Actor with **self**.

**Compile**, **Save**.

### B5 The training dummy `BP_CombatDummy` (in `Variant_Combat/Blueprints/Interactables`)

Open it, **Event Graph**. Find **Event On Dummy Damaged**. From the Spawn System node's white output arrow:

1. **Send Feel Event**, **Event**: `Feel.Event.Hit.Received`, **Target**: Make Feel Target From Actor with **self**.
2. **Context**: Make FeelPlayContext with **Location** = the event's **Location**, **Direction** = the event's **Direction**, **Instigator** = **Get Player Character**.

**Compile**, **Save**.

