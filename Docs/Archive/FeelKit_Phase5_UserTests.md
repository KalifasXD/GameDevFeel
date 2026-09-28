# FeelKit Phase 5: user retest checklist (round 5)

Updated 2026-09-17 (late night) after retest 4. Only checks that are still open are listed.

**Passed and not repeated:** A1-A10, B1, B2, B3, B5, B6, B7, B8, B9, C1-C9, D1, D2 (main check), D4, D5, E1.

**Logged as a go-live blocker, not part of this retest:** PlayStation controller rumble on Windows (project log I-029). If you want to help narrow it down, the `feeltest.rumble` steps are kept at the end.

Mark each check **Pass** or **Fail** with a note.

---

## 0. Before you start

1. Everything is built. Open `B:\NewUE5Project\GameFeelDev\GameFeelDev.uproject`.
2. Everything below reuses the assets from earlier passes (`Content/FeelKitTests`, the key events in `BP_ThirdPersonCharacter`).
3. **Comfort reset** when a step says so: stop PIE, delete `B:\NewUE5Project\GameFeelDev\Saved\SaveGames`.

---

## D2 optional: Allow Global Time Dilation In Multiplayer

Measured automatically in your setup (2 players, listen server, one process, both characters walking, L pressed on the server and on the client):
- **Option on:** world time goes to 0.05 on both machines for the hitstop and back to 1.00. Both characters slow to about a tenth of their speed on both screens and then walk on normally, with no position jumps.
- **Option off:** only the character that pressed L slows, on both screens; the other keeps walking.

That matches the expected result, so I need to know what looked wrong to you.

1. Project Settings > FeelKit > **Allow Global Time Dilation In Multiplayer** on.
2. Two-player listen server PIE. In both windows, walk with the characters.
3. In the listen server window press **L**.
4. Please describe what you see in each window during the second after pressing L, and right after it, for example: which character stops or slows, whether the camera still turns with the mouse, whether a character jumps back to an earlier position afterwards, whether it looks different when pressing L in the client window.
5. Turn the option back off.

What is expected, so you can compare: during the hitstop **everything in the world** (both characters, their animations, anything moving) runs at about 5% speed on both screens for about a second, then continues normally. FeelKit's own shakes and flashes keep running at normal speed (they use real time on purpose). It is not a full stop: at 0.05 things still crawl.

---

## D3 Gameplay Ability System cues

**Why it failed to compile:** the three gameplay cue nodes have a **Parameters** pin that Unreal requires to be connected (it is passed "by reference"), and my steps did not say so. The fix is a Make GameplayCueParameters node with its default values.

1. `BP_ThirdPersonCharacter` > Components > **Add** > **Ability System** (skip if it is already there). Compile, Save.
2. Right-click in `FeelKitTests` > **Blueprint Class** > All Classes > **Feel Gameplay Cue Notify**, name `GC_FeelHit` (skip if it exists). Open it: Gameplay Cue Tag `GameplayCue.Feel.Hit`; Feel > Recipe `R_Hit`. Compile, Save.
3. Right-click > **Blueprint Class** > All Classes > **Feel Gameplay Cue Notify (Actor)**, name `GC_FeelCharge` (skip if it exists). Gameplay Cue Tag `GameplayCue.Feel.Charge`; Feel > Recipe `R_Sustain`. Compile, Save.
4. In the character, the nodes you already have: key **Z**: **Execute Gameplay Cue On Actor (Burst)** (Target Self, tag `GameplayCue.Feel.Hit`). Key **X**: **Add Gameplay Cue On Actor (Looping)** (Target Self, tag `GameplayCue.Feel.Charge`). Key **C**: **Remove Gameplay Cue On Actor (Looping)** (Target Self, tag `GameplayCue.Feel.Charge`).
5. **New:** on each of the three nodes, drag from the **Parameters** pin into empty graph space, type `Make GameplayCueParameters`, pick it. Leave all its values at their defaults. (One Make node per cue node; you can also connect one Make node to all three Parameters pins.)
6. Compile. **Expected:** no errors. Save.
7. Single-player PIE: press **Z**. **Expected:** the `R_Hit` effect. Press **X**, wait 3 s, press **C**. **Expected:** a continuous shake that fades out after C.
8. If nothing plays: Window > Output Log, search "GameplayCue", note any message.

---

## Optional: controller rumble probe (go-live blocker I-029)

Only if you want to help narrow down I-029 now; it does not block Phase 5.

1. Open `BP_ThirdPersonCharacter`. Find the **H** event whose Pressed pin goes to **Set Comfort Group Scale** (Group Haptics, Scale 0), select that H event node and press Delete. Right-click an empty spot in the graph, type `J`, pick **Input > Keyboard Events > J**, and connect its **Pressed** pin to that Set Comfort Group Scale node. Compile, Save. Now only one H event exists (the one with Client Play Force Feedback).
2. Turn the controller on and start PIE the way you normally do (the same way the controller moves the character). Move the character with the stick once, so Unreal knows it is the last gamepad used.
3. Open the console (backtick) and type `feeltest.rumble`, Enter. Put the controller in your hand immediately. Messages appear at the top left for 20 s:
   - `[1] Windows XInput`: how many XInput controllers the editor sees.
   - `[2] Player` and `[2] Last device used ... Last gamepad ...`: what Unreal thinks the controller is.
   - `[3] FeelKit comfort`: haptics should be 1.00.
   - After 2 s, **STAGE 1**: Windows rumbles the controller directly, without Unreal (1.5 s).
   - After 6 s, **STAGE 2**: Unreal's own force feedback (1.5 s).
   - After 10 s, **STAGE 3**: FeelKit `R_Haptic`.
4. Note which stages rumbled and copy the `[1]` and `[2]` lines (or take a screenshot). This tells us exactly where the chain stops:
   - Stage 1 does not rumble or `[1]` says no XInput controller: the controller is not reaching Windows as XInput. That is the device side (Steam Input / translation tool), before Unreal.
   - Stage 1 rumbles, stage 2 does not: Unreal's side; send me the `[2]` lines.
   - Stages 1 and 2 rumble, stage 3 does not: FeelKit.
5. If stage 3 rumbles: press **E**. **Expected:** the two-peak rumble. Press **H**. **Expected:** the `FF_Test` rumble. Press **J**, then **E** and **H**. **Expected:** no rumble; `showdebug forcefeedback` values stay at 0.
6. Stop PIE, reset comfort.


---

## Notes for later documentation (from testing)

- **Gameplay cue nodes (D3):** Execute / Add / Remove Gameplay Cue On Actor need a Make GameplayCueParameters node on their Parameters pin (by reference), even when all values are defaults.
- **Global time in multiplayer (D2):** with Allow Global Time Dilation In Multiplayer, a Global Hitstop slows the whole world on every machine; FeelKit effects keep real time.

- **Post Process Material Pulse material (B2):** Scene Texture `PostProcessInput0` Color is 4 channels, so the effect colour must be a **Constant4Vector** (or mask the scene colour to RGB); a Constant3Vector does not compile into the Lerp.
- **Character targets (D1):** actor scale and offset effects on a character (or any actor whose root is a collision shape) move its visible mesh, never the capsule, so collision and movement stay unchanged and nothing replicates.
- **Controller rumble (B4/C10):** Unreal on Windows rumbles only XInput controllers; the on-screen `showdebug forcefeedback` values are what Unreal sends, not proof that a device received them.
- **Decal materials (B6):** connect Constant nodes to Base Color and Opacity; values typed on the result node's pins are not used by decals.

## After testing

Reset comfort. Report results like "B6 pass, D1 step 5: Debugger shows ...".
