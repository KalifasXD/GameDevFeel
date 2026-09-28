# FeelKit on UE 5.7 and 5.8: go or no-go

Checked 2026-09-24 (night) on UE 5.7.4 (`D:/EpicGames/UE_5.7`) and UE 5.8.3 (`D:/EpicGames/UE_5.8`). The 5.6 projects were
not changed; every check on the new engines ran on copies.

## Verdict

| | UE 5.7 | UE 5.8 |
|---|---|---|
| **FeelKit plugin (what buyers get)** | **GO** | **GO** |
| **GAS add-on** | **GO** | **GO** |
| Our demo levels, opened as they are | Not portable: Epic's 5.6 template levels crash in PIE on 5.7 (see below) | Same on 5.8 |

**What this means:** FeelKit can be submitted to Fab for 5.6, 5.7 and 5.8. Fab requires 5.8 at the first submission, and
5.8 passes. The demo levels need to be rebuilt on each engine's own template before they can be shown or shipped for
that engine, because Epic's 5.6 template levels themselves crash when opened in 5.7 or 5.8, with or without FeelKit.

## What was checked

| Check | 5.6 | 5.7 | 5.8 |
|---|---|---|---|
| Fab package build (strict, the way Epic builds it) | Passes, 0 warnings | Passes, 0 warnings | Passes, 0 warnings |
| Package contents | 0 test files, GAS add-on included, 4.3 MB | same | same |
| GAS proof, project without the add-on | GAS off, not loaded | GAS off, not loaded | GAS off, not loaded |
| GAS proof, project with the add-on | Builds 0 warnings, GAS test passes | same | same |
| FeelKit automated tests | 116 of 116 | 116 of 116 | 116 of 116 |
| Editor window tests (recipe editor, browser, preview, Feel Switch in play) | 6 of 6 | 6 of 6 | 6 of 6 |
| Horror demo playthrough (FeelDemoFP) | Passes, 0 warnings | Passes, 0 warnings | Passes, 0 warnings |
| Action/RPG, guards, Platformer, Shooter playthroughs | Pass | Crash when Play starts | Crash when Play starts |

## Code changes needed for 5.7 and 5.8 (done)

Three changes, all in FeelKit's editor module; 5.6 builds unchanged:
1. Unreal 5.7 added a new required version of a curve-editor function (`GetCurves`) that FeelKit's curve fields provide.
   FeelKit now provides it for 5.7 and later behind an engine version check. The check was confirmed by the 5.7 build.
2. Two calls that list objects used an argument that 5.8 deprecates. They now use the engine's default, which behaves
   the same on 5.6.
3. The engine version header is included explicitly, because Fab's strict build does not pull it in indirectly.

## The demo level crash: not FeelKit

On 5.7 and 5.8, Play In Editor crashes on the Combat, Platforming and Shooter levels about 14 seconds after it starts,
before any test step runs. The crash is inside Unreal's own engine code (reading address 0x170).

How it was narrowed down:
- It still crashes with FeelKit switched off (`feel.Enabled 0`).
- **Epic's own 5.6 Third Person template, with no FeelKit at all, opened in 5.8 and in 5.7: the Combat and Platforming
  levels crash the same way, at the same address.**
- **Epic's 5.8 Third Person template on 5.8: the same levels run without problems.**
- The Horror level, which does not use those template parts, runs fully on both engines with FeelKit.
- Later the same morning every level was checked in Epic's untouched 5.6 templates and in Epic's own 5.7 and 5.8
  templates, on both engines: Combat, Platforming, Side Scrolling and Shooter crash only in the 5.6 templates; every
  level plays in the 5.7 and 5.8 templates; Play in the level viewport crashes the same way. Steps to check this
  yourself: `Docs/FeelKit_UE57_UE58_CrashCheck.md`.

So the crash comes from Epic's 5.6 template content being opened in a newer engine. Buyers on 5.7 or 5.8 create their
projects from their engine's own template, where these levels work.

## Steps to be a go on the demos for 5.7 and 5.8

1. **Decide how the demos reach buyers** (already an open launch question). The results above favour building each
   demo on the template of the engine it is shown on.
2. **Rebuild the demo projects on the 5.8 template** (and 5.7 if the demos are offered there): a new C++ Third Person
   project and a new First Person project made from the 5.8 templates, with FeelKit added. Then:
   - re-apply our template code changes (the lines marked `// FeelKit`): the Action/RPG guard, block and parry (about
     6 files), and the Shooter and Horror HUD hooks (about 5 files);
   - run the existing kit scripts (`Tools/FeelKitScripts`) to rebuild the recipes, sounds, Feel Maps, Feel Switch,
     controls panels and guard enemy;
   - run the playthroughs on 5.8.
   Estimate: one working day for 5.8, about half a day more for 5.7, with no work from you until you play the result.
3. **You play the rebuilt levels once on 5.8.** The feel values are the same recipes you approved, but the 5.8
   templates may differ slightly (camera, animations).

Nothing in these steps changes FeelKit itself.

## Fab uploads

Fab wants one upload per engine version. `Tools/FeelKitScripts/Run/package_plugin.ps1` makes each one:
- 5.6: `package_plugin.ps1`, output `Build/FKPkg_56`;
- 5.7: `-Engine D:\EpicGames\UE_5.7 -Version 5.7`, output `Build/FKPkg_57`;
- 5.8: `-Engine D:\EpicGames\UE_5.8 -Version 5.8`, output `Build/FKPkg_58`.

Each sets the plugin's engine version correctly and leaves the tests out. The zips for Fab are made from these folders,
without Binaries and Intermediate, at submission time.

## Small things found in our own test projects (not shipped)

- Epic's StateTree template code shows two deprecation warnings on 5.8.
- Our network test harness uses a material function that 5.7 and 5.8 deprecate.
- Epic's 5.6 Shooter template has one line that 5.7 and 5.8 refuse to compile; Epic's 5.8 template contains the fix.

None of these are in FeelKit. They only matter if the demo projects move to the newer templates, which step 2 above
does anyway.
