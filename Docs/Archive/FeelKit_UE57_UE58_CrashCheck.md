# Checking the 5.7 / 5.8 demo crash yourself

Updated 2026-09-24.

This lets you see for yourself that the demo crash on UE 5.7 and 5.8 comes from Epic's 5.6 template levels and not
from FeelKit. You open the same levels in three kinds of projects and press Play:

- **A. Our demo projects** (with FeelKit) on 5.7 / 5.8.
- **B. Epic's untouched 5.6 templates** (no FeelKit at all) on 5.7 / 5.8.
- **C. Epic's own 5.7 / 5.8 templates** (no FeelKit) on 5.7 / 5.8.

All projects are copies under `B:\NewUE5Project\Build\`. Your real 5.6 projects are not involved. Nothing you save in
the copies matters; they are rebuilt whenever the check runs again. Everything is already compiled for its engine.

Allow about 20 minutes for 5.8 alone, 40 for both.

## Before you start

- Close any open Unreal Editor.
- A crash shows Unreal's crash reporter window. That is expected for the levels marked **crash** below. Close the
  reporter (sending the report to Epic is up to you) and continue with the next project.
- Double-clicking a project opens it straight in the right engine (checked for a 5.8 copy). If Unreal still asks
  which engine to use, pick the one in the folder name (UE57 = `D:\EpicGames\UE_5.7`, UE58 = `D:\EpicGames\UE_5.8`).
  If it offers to rebuild modules or convert the project, choose **No**: every project is already built.
- To open a level: in the Content Browser, open the folder named below and double-click the level. Press the green
  **Play** button in the toolbar. A crash happens a few seconds after pressing Play; if nothing happens for about
  15 seconds, the level plays. Press **Esc** to stop.

## UE 5.8

### A. Our demos with FeelKit

1. Double-click `B:\NewUE5Project\Build\UE58\GameFeelDev\GameFeelDev.uproject`.
   - `Variant_Combat` → `Lvl_Combat` → Play. **Expected: crash.**
   - (Reopen the project after the crash.) `Variant_Platforming` → `Lvl_Platforming` → Play. **Expected: crash.**
2. Double-click `B:\NewUE5Project\Build\UE58\FeelDemoFP\FeelDemoFP.uproject`.
   - `Variant_Horror` → `Lvl_Horror` → Play. **Expected: plays, with FeelKit working** (sprint with Left Shift, Tab
     turns the feel off and on).
   - `Variant_Shooter` → `Lvl_Shooter` → Play. **Expected: crash.**

### B. Epic's 5.6 templates, no FeelKit

3. Double-click `B:\NewUE5Project\Build\CrashCheck\UE58_Epic56_ThirdPerson\TP_ThirdPerson.uproject`.
   - `Variant_Combat` → `Lvl_Combat` → Play. **Expected: crash**, the same as in A.
   - `Variant_Platforming` → `Lvl_Platforming` → Play. **Expected: crash.**
   - Optional: `ThirdPerson` → `Lvl_ThirdPerson` plays; `Variant_SideScrolling` → `Lvl_SideScrolling` crashes.
4. Double-click `B:\NewUE5Project\Build\CrashCheck\UE58_Epic56_FirstPerson\TP_FirstPerson.uproject`.
   - `Variant_Shooter` → `Lvl_Shooter` → Play. **Expected: crash.**
   - `Variant_Horror` → `Lvl_Horror` → Play. **Expected: plays.**

### C. Epic's 5.8 templates, no FeelKit

5. Double-click `B:\NewUE5Project\Build\CrashCheck\UE58_Epic58_ThirdPerson\TP_ThirdPerson.uproject`.
   - `Lvl_Combat` and `Lvl_Platforming`. **Expected: both play.**
6. Double-click `B:\NewUE5Project\Build\CrashCheck\UE58_Epic58_FirstPerson\TP_FirstPerson.uproject`.
   - `Lvl_Shooter`. **Expected: plays.**

## UE 5.7

The same steps with `UE57` in every path:
- A: `Build\UE57\GameFeelDev` and `Build\UE57\FeelDemoFP`
- B: `Build\CrashCheck\UE57_Epic56_ThirdPerson` and `UE57_Epic56_FirstPerson`
- C: `Build\CrashCheck\UE57_Epic57_ThirdPerson` and `UE57_Epic57_FirstPerson`

The expected results are the same.

## What I measured before handing this over

Every level was started with Play on both engines (an automated run that starts Play the same way as the button;
viewport Play was confirmed separately on the 5.8 Combat level in the full editor).

| Level | Our demos (A) | Epic 5.6 template (B) | Epic 5.7 / 5.8 template (C) |
|---|---|---|---|
| Combat | crash | crash | plays |
| Platforming | crash | crash | plays |
| Shooter | crash | crash | plays |
| Horror | plays | plays | plays |
| Side Scrolling (not a demo) | not in our demos | crash | plays |
| Third Person, First Person (starter levels) | not tested | plays | plays |

Every crash is the same one: an access violation reading address 0x170 inside Unreal's engine code, a few seconds after
Play starts.

## Small changes in the copies (so they open and can be tested)

- Each copy is set to its engine. The 5.6 templates use the newer engine's build settings, which is what Unreal's own
  project upgrade does.
- Epic's 5.6 First Person template has one line that 5.7 and 5.8 refuse to compile. The copies use the same fix Epic
  made in their 5.7 / 5.8 template.
- The template copies (B and C) contain one extra test file, `StockPIESmoke.cpp`, used for the automated run above. It
  only starts and stops Play.

The script that makes the B and C projects is `Tools/FeelKitScripts/Run/make_crash_check.ps1`. The A copies come from
`verify_engine.ps1`.
