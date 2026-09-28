# FeelKit launch readiness, 2026-09-23

**Superseded on 2026-09-24 by `FeelKit_GoLive_Readiness_2026-09-24.md`.** Kept for its history and effort tables.

| Field | Value |
|---|---|
| Date | 2026-09-23 (night) |
| Sources | `CLAUDE.md`, `Docs/FeelKit_ProjectLog.md` (sections 1, 1a, 3, 4, 7), `FeelKit_Requirements.md`, `Docs/FeelKit_Strategy_v2_Proposal.md`, `Docs/FeelKit_Demos.md`, `Docs/FeelKit_Phase6B_UserTests.md`, `Docs/FeelKit_Phase6B_Demos_Design.md`, `Docs/Research/GAS_Optional_Integration_2026-09-23.md`, folder listings of the plugin, FeelDemoFP and `Tools/FeelKitScripts` |
| Estimates | Every number of hours, days or rounds in sections 3 and 4 is an **estimate**. Each one says what it is based on. |
| Scope | Read only. No code or assets were changed for this report. |

In short: the plugin is feature complete and verified on UE 5.6. Four demos are approved and are now in a last tuning pass. What stands between the project and a Fab release is mostly launch work that has not started: the Lite/Pro split, packaging for Fab, UE 5.7 and 5.8 builds, buyer delivery of the demo kits, the listing and its media, and a set of user decisions. Documentation may follow the release (D-051), but a minimum set is advised at launch.

---

## 1. What is done

### Runtime (FeelCore, plus three optional modules)
- Subsystem that ticks only while something plays, on real (unscaled) time (RT-001 to RT-003, RT-005 to RT-007).
- Blueprint and C++ API: Play Feel, Play Feel With Context, Stop, Stop All, Release, Set Feel Parameter, Play Feel And Wait, started and finished delegates, `feel.Enabled`, `feel.GlobalScale`, `stat Feel`.
- Arbiters for camera, time, screen and haptics; camera modifier added automatically; flash through a scene view extension.
- 37 step types (40 step classes, 3 of them base classes): camera, time, screen, actor, audio, haptics, UI, spawn, nested recipe, random choice, Blueprint event, hit flash.
- Recipe parameters, play context, track conditions, random ranges, sustained recipes, accumulators (D-020, I-006).
- Feel Events and Feel Maps, anim notifies (Play Feel, Send Feel Event, Play Feel (Window), Set Feel Value).
- Feel Trigger component: damage, landed, hit, overlap, jumped, air jumped, launched; Play On other actor; Only For Players (D-058).
- Networking: Feel Replication component with Everyone, Owner Only, Skip Owner and relevancy (NET-002, NET-003, D-035); dedicated server no-op; multiplayer hitstop policy (D-028).
- Feel Switch actor: feel off and on while playing, Tab and controller View / Share (D-057).
- Optional modules: FeelNiagara (Spawn Particle), FeelGAS (gameplay cue notifies), FeelEnhancedInput (input actions to recipes).
- Actor motion never moves a collision root or a physics body (D-037, I-037).

### Comfort
- Per-player scales for seven groups, built-in and custom presets, project defaults, SaveGame persistence per local player, storage interface (CMF-001 to CMF-012).
- Essential tracks with substitutes and floors; a player's 0 on camera shake or motion always means 0 (D-012, I-007).
- Flash limiter and motion comfort (CMF-030 to CMF-032, CMF-040, CMF-041).
- Comfort reaches engine camera shakes and force feedback; comfort multiplies the game's own vibration scale and says when vibration is silenced (D-046).
- Comfort Audit, worded as a readiness helper (D-025).

### Editor (FeelEditor)
- Recipe editor with a Sequencer-style timeline and toolbar, preview with audio, scrubbing, inline curve keys, track copy and paste, Play in PIE, comfort dropdown, intensity graph, preview mesh, validation on save (D-049, D-050).
- Moment capture with Recent Plays replay, decision labels on track bars.
- FeelKit Debugger (Outliner style) and `showdebug feel`.
- Capture GIF, Off/On side by side (D-026 closed: GIF only).
- Waveforms on sound tracks, onset snapping, shake and force feedback tracks made from a sound.
- Recipe Browser with live hover preview and filters (D-047), Recipe from Template, read-only library with Copy to Project (D-040), Content Browser tiles and tooltips.
- Recipe JSON import and export (D-045); editor scripting functions used by the kit builders.

### Content library
- 38 library recipes across nine feelings (Impact, Weight, Power, Speed, Reward, Danger, Dread, Denial, Interface), shipped as JSON and as assets under `/FeelKit/Library` (D-039, D-042).
- CC0 sample sounds (Kenney, artisticdude, StarNinjas) and FeelKit-made materials, all listed in `Plugins/FeelKit/Credits.md`.
- Phase 6A user checks all passed on 2026-09-18 ("Everything is perfect").

### Demos (Phase 6B)
| Demo | Level | Project | State |
|---|---|---|---|
| Action/RPG "Weight Class" | `Lvl_Combat` | GameFeelDev | Approved 2026-09-19. Camera, softer hit turn, guards, parry and guarding enemy added 2026-09-23 (D-065, D-068, D-069); being retuned |
| Platformer "Bounce Feel" | `Lvl_Platforming` | GameFeelDev | Approved 2026-09-22. Landing change (D-067) waiting for play |
| Shooter "Every Bullet Has an Opinion" | `Lvl_Shooter` | FeelDemoFP | Approved 2026-09-22, jump pads approved 2026-09-23. HUD reactions (D-064) waiting for play |
| Horror "Heartbeat" | `Lvl_Horror` | FeelDemoFP | Approved 2026-09-23. Sprint meter reactions (D-064) waiting for play |
| UI "Juicy Menus" | removed | | Dropped by the user (D-064); UI feel moved into the Shooter and Horror HUDs; backup kept |

- Feel Switch placed in all four levels.
- Kit recipe JSON in the plugin: Action/RPG 8, Platformer 5, Shooter 20, Horror 8.
- Kit builders, sound imports and runners in `Tools/FeelKitScripts` (13 Python scripts, 11 PowerShell runners).

### Tests and builds (last verification, 2026-09-23 evening)
- **116/116** automation tests in the FeelKit suite, **6/6** UI tests in a rendering session.
- GameFeelDev editor, Game Development and Game Shipping: **0 warnings**.
- FeelDemoFP editor, Game Development and Game Shipping: **0 warnings**.
- Strict `BuildPlugin` to `Build/FKPkg5`: **BUILD SUCCESSFUL, 0 warnings**.
- Playthrough diagnostics: Action/RPG 0 PIE warnings, Action/RPG guards **18/18** checks, Platformer every moment, landing never above rest, Shooter and Horror all moments, all with 0 PIE warnings.
- No-AI-trace sweep of every new or changed file: clean (D-061).

---

## 2. What is left before going live

Labels used below:
- **Blocker**: the release should not go out without it.
- **Advised**: not strictly required, but launching without it costs sales or reviews.
- **After release**: can follow the release.

### 2.1 Phase 6B demos still open
| Item | State | Label |
|---|---|---|
| Action/RPG: restore the "knock" of each hit after the camera moved to 3.5 m and the hit turn was softened (D-068) | In progress: the on-screen knock of each punch is being measured | Blocker (D-053: demos before release) |
| Action/RPG: user play of camera, guards, parry, guard break (D-065, D-069) | Built and checked (18/18), waiting for play | Blocker |
| Controls panel (key list) in the top right under the Feel Switch badge, in all four levels | In progress. Open design point: part of the Feel Switch (ships to buyers, Lite and Pro) or demo-only | Blocker (user request for every demo) |
| Platformer landing, view only goes down (D-067) | Built and measured, waiting for play | Blocker |
| Shooter HUD reactions and Horror sprint meter (D-064) | Built, waiting for play | Blocker |
| Final sign-off of all four demos together, after the last changes | Not started | Blocker |

### 2.2 How buyers get the demo kits
The kits were built by scripts in the developer's projects. A buyer only gets what ships in the plugin (recipes, sounds, materials, Feel Switch). The rest lives in the host projects:
- Template C++ edits marked `// FeelKit`: Shooter (weapon, projectile, NPC, character, two HUD classes) and Horror (character, HUD).
- New demo C++ in GameFeelDev: `CombatGuardComponent`, `CombatGuardEnemy`, guard checks in `CombatCharacter` and `CombatEnemy`.
- Blueprint and level changes: Feel Trigger components on characters and `BP_JumpPad`, trigger boxes and lamp spheres in `Lvl_Horror`, `BP_CombatGuardEnemy` (a copy of Epic's `BP_CombatEnemy`, so it cannot ship, D-052), camera values, `IA_Block` mapping.

| Item | Label |
|---|---|
| Decide the delivery: a guide with code to paste, source files to copy from the kit folder, or the Python kit builders shipped for buyers to run on a fresh template project | Blocker if the kits ship in 1.0 |
| Update the Action/RPG guide (the archive in `FeelKit_Phase6B_UserTests.md` predates D-056, the guards and the camera) and write the Platformer, Shooter and Horror guides | Blocker if the kits ship in 1.0 |
| Verify each kit on a fresh project made from Epic's template (Combat, Platforming, Shooter, Horror) | Blocker if the kits ship in 1.0 |
| Note for the listing: the Shooter and Horror kits need a C++ project, because their hooks are C++ edits | Blocker (honest listing) |

The fallback is to ship 1.0 with the recipes only and add the kits in an update. Not advised: the demos are the main sales material.

### 2.3 Phase 6C documentation (DOC-001 to DOC-008)
D-051 allows documentation after the release. Nothing is written yet. Doc notes already collected in the log: the parameter name dropdown (DOC-003), the Constant4Vector material note (I-025), the Make GameplayCueParameters step (I-031), UI reactions on a panel inside a button (I-044), input mode before opening a level (I-045), `showdebug forcefeedback` reading (I-032).

| Item | Label |
|---|---|
| Quick Start (DOC-001) with the 5-minute path (NF-020) | Advised at launch (a buyer with no Quick Start writes the first review) |
| FAQ (DOC-007): Blueprint-only projects need Visual Studio to package (C-007), controller rumble on Windows (I-029), the "temp target" message | Advised at launch |
| Changelog (DOC-008) | Advised at launch (one page) |
| A place to host the docs, and the `DocsURL` for the plugin and the listing | Advised at launch. Whether Fab requires a documentation link for code plugins is to be checked in Fab's current requirements |
| Concepts, step reference, custom steps, comfort guide, multiplayer guide (DOC-002 to DOC-006) | After release |
| "Coming from Feel" guide (Strategy v2) | After release |
| The one how-I-made-it tutorial, Action/RPG sword hit (D-016) | After release |

### 2.4 Phase 6D launch readiness

**Editions and packaging**
| Item | State | Label |
|---|---|---|
| Lite/Pro split (D-015): what Lite contains (requirements say FeelCore steps tagged L, comfort, no editor module; D-057 puts the Feel Switch in Lite; the library subset was left open in D-042), whether Lite has the recipe editor, one listing or two (OD-006), and the upgrade path from Lite to Pro (plugin and module names must not clash, recipes made in Lite must open in Pro) | Not started, needs user decisions | Blocker |
| Pro price and Fab license tiers (requirements: about $69.99 to $79.99; Strategy v2: about $39.99) | Not decided anywhere in the log | Blocker |
| `FeelKit.uplugin` metadata: `CreatedBy`, `CreatedByURL`, `DocsURL`, `SupportURL`, `MarketplaceURL` are empty, `IsBetaVersion` is true, `VersionName` is 0.1.0 | Found while reading the file | Blocker |
| `PlatformAllowList` lists Win64, Linux and Mac, but only Win64 was ever built or tested (C-004) | Found while reading the file | Blocker (restrict to Win64 or test the others) |
| Copyright line at the top of source files: 0 of 226 source files have one | Found while reading the files. Check whether Fab's code plugin requirements ask for it (NF-030) | Blocker if Fab requires it |
| Move demo diagnostics out of the plugin (ARPG, Platformer, PlatformerSnap, Shooter, Horror playthroughs and the Blueprint dump live in `FeelEditor/Private/Tests` and name template classes) | Listed in the log | Blocker |
| Re-save the library packages so unloaded tiles show the new thumbnail look (can now be done by editor scripting under D-056) | Listed in the log | Blocker (small) |
| Fab zip per supported engine version, `FilterPlugin.ini` check, no Binaries or Intermediate in the upload | Not started | Blocker |

**GAS add-on (D-027, D-066)**
- Today `FeelKit.uplugin` enables GameplayAbilities in every project that enables FeelKit, which also pulls in Data Registry.
- Research recommends option B: a GAS add-on shipped inside the FeelKit folder, inactive until copied into a project. It must be checked with a real package and two test projects. Fallback: a separate free listing (option C).
- Label: **Blocker** (the decision, and the package test if B is chosen).

**Engine versions (C-001, C-001a, NF-031)**
- The requirements ask for UE 5.6, 5.7 and 5.8 at launch, with tests and zero warnings on each.
- Only UE 5.6 is installed on this machine. The Gameplay Cameras check and the MCP toolset also need 5.7 or 5.8.
- Content must stay saved in 5.6 (C-001c). Newer APIs only behind version macros (hard rule 1).
- Label: **Blocker** per C-001, unless the user decides to launch on 5.6 only and add versions in an update. The main competitor lists UE 5.3 to 5.8 (T-1), so a 5.6-only launch is weak.

**Controller rumble (I-029)**
- The log is not consistent here. The I-029 row says "Fixed (D-046), waiting for the user to confirm on the device". On 2026-09-18 the user said the controller fix works. Section 7 still lists "PlayStation controller rumble on Windows" as a go-live blocker.
- The remaining fact is a platform limit: Unreal on Windows sends vibration only to XInput controllers. PlayStation and other pads rumble through Steam Input or an XInput translation tool.
- Options: (a) close as a platform limit with an FAQ entry and a listing note; (b) a short spike on the engine's own GameInput or RawInput plugins (whether they help is not verified); (c) native DualSense support as the premium FeelHaptics module (ST-028, OD-005), after launch.
- Label: **Blocker until the user decides**. With option (a) it becomes an FAQ item.

**Fab listing and media**
| Item | Label |
|---|---|
| Fab seller account, identity, tax and payout details | Done: the user already has a Fab seller account with other products on it (2026-09-23) |
| Title and tags: review "juice" in the title (D-014 against D-064), "hitstop", "screen shake", "game feel" | Blocker |
| Description, technical details (modules, platforms, engine versions, dependencies), Credits | Blocker |
| Screenshots and GIFs (DiagFeel window pictures and Capture GIF exist as tools) | Blocker |
| Support channel (email or Discord) for `SupportURL` | Blocker, user creates it |
| Fab review and any fixes it asks for | Blocker, time not in anyone's control |

**Trailer and demo videos (D-055)**
- Method not decided. Planned in `FeelKit_Demos.md`: one 30 to 60 second video per demo with an Off/On comparison, a combined trailer, a short comfort segment.
- Recording needs real play with sound. OBS is installed on this machine. A scripted capture by the developer is possible but has no controller feel and needs audio capture work.
- Music needs a licence that allows use in a trailer.
- Check the Epic content licence for showing template content in videos (open since `FeelKit_Demos.md` section 4).
- Label: **Advised** before launch. A first release can go out with GIFs and screenshots if the user accepts that.

**First-hit stall**
- The frame after the first hit of a session takes about 80 to 90 ms, with FeelKit on or off, so it is the template or the editor warming something up. (An earlier figure of 270 ms included the measuring tool's own picture capture.)
- Check it in a packaged build. If it remains, warm the effect up in the demo level.
- Label: **Advised** before videos and before any downloadable demo.
- Packaged Shooter and Horror builds exist in `FeelDemoFP/PackagedFeelFP` (dated 2026-09-23 14:36, before the HUD work). They are not recorded in the log.

**Release checks**
| Item | Label |
|---|---|
| Install the packaged plugin the way Fab does (engine plugin, launcher engine) and use it in a fresh Blueprint-only project (C-006, TS-005) | Blocker |
| Package a Blueprint-only project on Windows with Visual Studio (TS-006) | Blocker |
| Packaging message on a machine without Visual Studio (TS-007) | Advised; needs another machine or a clean virtual machine |
| Performance capture for NF-002 (50 instances under 0.2 ms) and the published number (TS-009, OD-007 minimum hardware) | Advised |
| Tooltip sweep (NF-022), no-AI-trace sweep of everything user-facing including the listing (D-061) | Blocker |
| Final strict builds, tests and BuildPlugin on every supported engine version (TS-010) | Blocker |

### 2.5 Requirements never recorded as done or as dropped
These are V1 items in `FeelKit_Requirements.md`. The log neither records them as built nor as dropped. Each needs a user decision: build before launch, move after launch, or drop.

| ID | Requirement | Note |
|---|---|---|
| CMF-050, CMF-051 | Sample comfort settings widget (sliders, preset dropdown, gamepad navigable), basic version in Lite | The comfort layer is the Lite pitch; buyers need a way to show it to players. Advised |
| CT-003 | In-world comfort panel in the demo map | Could be the same widget in one demo level |
| CT-006 | Playable packaged demo for Windows | Useful as a listing link. Check whether a packaged game with Epic template content may be offered as a download |
| RT-004, NF-003 | Instance pooling, no per-frame heap allocation | The competitor report says pooling is not done |
| DEL-004, TS-008 | Split-screen: effects reach only the intended local player, and a split-screen test | The Feel Switch code mentions split screen; no test is recorded |
| C-004 | Linux and Mac | See the platform list item in 2.4 |
| TL-003 | Gameplay Debugger category | `showdebug feel` was built instead; record it as the answer |

### 2.6 Open decisions (user)
1. Lite contents, Lite with or without the recipe editor, one listing or two (D-015, OD-006, D-042 library subset).
2. Pro price and licence tiers.
3. GAS: option B or C (D-066).
4. I-029: close as a platform limit with an FAQ entry, or more work before launch.
5. Engine versions at launch: 5.6 to 5.8 (C-001) or 5.6 first.
6. How buyers get the kits' template code and Blueprint changes (2.2).
7. The requirement gaps in 2.5, especially the comfort widget.
8. Trailer and video method (D-055).
9. "Juice" in the Fab title (D-064).
10. Controls panel: plugin feature or demo-only.

### 2.7 Cannot be done on this machine
- UE 5.7 and 5.8 builds and tests, the Gameplay Cameras check, the MCP toolset: only UE 5.6 is installed (the user installs the others).
- Mac builds: need a Mac. Linux builds: need the Linux cross-compile toolchain, not installed.
- The packaging-without-Visual-Studio check (TS-007): needs a machine or virtual machine without Visual Studio.
- Feeling rumble on the user's PlayStation controllers: the hardware is here, but only the user can hold it.

### 2.8 Record keeping found while reading
Not blockers, but they will confuse a later reader:
- `CLAUDE.md` still says all five demos are built and the UI demo is waiting on the user (changed by D-064).
- `CLAUDE.md` points to `Docs/FeelKit_Requirements.md`; the file is at the project root.
- The I-029 row and section 7 of the log disagree (see 2.4).
- `FeelKit_Demos.md` section 4 still says the user creates every asset (changed by D-056).
- The Shooter section of the demo design still describes `AdditionalPluginDirectories` (replaced by the junction, I-039).
- The two items in progress tonight (the hit knock measurement and the controls panel) are not in the log yet.

---

## 3. Effort for each remaining item

### 3.1 What the estimates are based on

**Developer pace, from the log.**
- Phase 6A, ten tasks, was built on one day (2026-09-17).
- The Platformer kit was built end to end in one afternoon (2026-09-22).
- The Shooter and the Horror kits were each built in one evening (2026-09-22).
- The Horror rework took about half a day (2026-09-23).
- The UI menu took about a day including two reworks.
- Estimates below are focused developer hours at that pace. They include the build and test cycle the project requires: editor and game targets in both projects, strict BuildPlugin, tests, pictures before handover.

**User rounds, from the log.** A round is: the user plays or reviews something and replies.

| Piece of work | Rounds | What happened |
|---|---|---|
| Phase 5 user test pass | 6 | pass 1, then retests 1 to 5 |
| Phase 6A tools, library, editor look | about 12 | checklist fixes, I-033, I-034, I-035, four look rounds |
| Action/RPG kit, first version (user built assets from step lists) | 5 | nothing plays (I-036), marker wording, "sounds terrible", download approval, "crazy good" with warnings (I-037) |
| Action/RPG additions (camera, guards) | 3 so far, still open | camera request, guard design choices, "lost feel" |
| Feel Switch | 1 | plus the title crop found in the Shooter round |
| Platformer | 2 to approval | snap (I-038), approved; 1 more open for landings |
| Shooter | 2 to approval | missing feel, approved; 1 for jump pads; HUD open |
| Horror | 2 to approval | random sounds, approved; meter open |
| UI menu | 3, then dropped | look, bugs and names (I-044, I-045), dropped (D-064) |

The pattern:
- Work the developer builds end to end and the user only plays takes **2 to 3 rounds**.
- Work the user operates through step lists takes **5 to 12 rounds**.
- Every checklist error cost a full round (I-022, I-025, I-031, A6, B1, B3).
- Feel tuning with a trade-off (the Action/RPG hit under motion blur) takes the most rounds.

### 3.2 Per item

| Item | Developer (estimate) | User rounds (estimate) | What the user does | Only the user can do it |
|---|---|---|---|---|
| Action/RPG hit knock | 3 to 6 h | 1 to 3 | Play and judge whether hits feel heavy again | Judge feel |
| Action/RPG camera, guards, parry follow-ups | 2 to 6 h | 1 to 2 (same sessions as above) | Play and judge | Judge feel |
| Controls panel in four levels | 4 to 8 h | 1 to 2 | Decide plugin or demo-only; look at it in play | Approve the look |
| Platformer landing, Shooter HUD, Horror meter follow-ups | 2 to 6 h | 1 (same session) | Play all three | Judge feel |
| Final four-demo sign-off | 2 to 3 h | 1 | Play all four levels once more | Approve |
| **6B subtotal** | **13 to 29 h** | **3 to 6** (rounds are batched: one session covers every open item) | | |
| Kit delivery to buyers (decision, four guides, code packaging, fresh-template checks) | 12 to 24 h | 2 to 3 | Choose the method; follow one guide on a fresh template project | Be the first buyer |
| Minimum launch docs (Quick Start, FAQ, changelog, hosting) | 8 to 16 h | 1 to 2 | Review; pick where docs are hosted | Create the hosting account if one is needed |
| Lite/Pro split and upgrade path | 12 to 24 h | 1 to 2 (decision), 0 to 1 (check) | Decide Lite contents and listing shape | Decide |
| Pro price and licence tiers | 0 to 2 h (comparison table) | 1 (same decision round) | Decide | Decide |
| Packaging hygiene (metadata, copyright lines, platform list, zip layout) | 6 to 12 h | 0 to 1 | Give the publisher name and URLs | Publisher name, URLs |
| Move demo diagnostics out of the plugin | 3 to 6 h | 0 | | |
| GAS option B package test | 4 to 8 h | 1 (same decision round) | Choose B or C | Decide |
| Library thumbnail re-save | 1 to 2 h | 0 | | |
| First-hit stall in a packaged build | 2 to 8 h | 0 to 1 | Only if a demo-side change needs a look | |
| UE 5.7 and 5.8 builds and tests | 8 to 24 h after install (depends on API changes and new deprecation warnings under the zero-warnings rule) | 1 | Install UE 5.7 and 5.8 in the Epic Games Launcher (large downloads, disk space) | Install engines |
| I-029 decision (FAQ, optional spike) | 1 to 8 h | 1 (same decision round), 0 to 1 on the pad | Decide; optionally test a PlayStation pad through Steam Input | Hold the controller |
| Release checks (Blueprint-only project, packaging, performance number, tooltip and wording sweeps, final builds per version) | 12 to 20 h | 0 to 1 | Run TS-007 on a machine without Visual Studio, if wanted | Another machine |
| Comfort widget (CMF-050), if chosen | 8 to 16 h | 1 to 2 | Look at it and try it with a controller | Approve the look |
| Fab listing copy, screenshots, GIFs | 8 to 16 h | 1 to 2 | Approve copy and media | Account, tax, payout, upload, submit |
| Trailer and four demo videos | 16 to 40 h (depends on the method) | 2 to 4 | Decide the method; record takes on own hardware; review cuts | Record with own hardware and voice; buy music if no free licence fits |
| Fab review fixes | 0 to 8 h | 0 to 2 | Pass Fab's messages on; resubmit | Everything in the Fab account |
| **After release:** full docs (DOC-002 to DOC-006, Coming from Feel, the tutorial) | 30 to 50 h | 2 to 4 | Review; record the tutorial if it is a video | Voice, recording |
| **After release, optional:** native DualSense module (FeelHaptics, ST-028) | 40 to 100 h (low confidence, not researched) | 2 to 4 | Test on own controllers | Hold the controller |

What the developer cannot do at all:
- Anything in the Fab publisher account: creating it, identity and tax details, payout, accepting Fab's terms, uploading, submitting, answering Fab review.
- Payments: music, a competitor purchase, a second machine.
- Legal terms and prices.
- Installing engine versions without the user's go-ahead.
- Recording with the user's hardware, voice or controller.
- Judging feel.

---

## 4. Total

### 4.1 Ranges (estimates)
- **Developer effort to go live:** about **105 to 255 focused hours**, most likely around **150 hours**. That is about 18 to 42 working days at 6 focused hours a day.
  - Based on the per-item table in 3.2, at the pace recorded in the log.
  - The three largest swings: the videos (16 to 40 h), the engine ports (8 to 24 h) and the Lite/Pro split (12 to 24 h).
- **User rounds to go live:** about **12 to 28**, most likely around **18**.
  - Based on 2 to 3 rounds per developer-built piece of work, and on batching several open items into one play session, as happened on 2026-09-23.
  - Checklist-driven work has taken 5 to 12 rounds in the past. Every step list handed to the user should be verified before handover.
- **After release:** 30 to 50 hours of documentation and 2 to 4 more rounds.
- **Calendar time** is set by user availability, Fab account verification and Fab review, not by developer hours.

For comparison, the log's estimate of 2026-09-18 put the project at about 60% of the way to launch. Since then the demos went from about 30% to nearly done. Documentation and launch preparation have not moved.

### 4.2 Critical path
1. **Close 6B:** finish the Action/RPG knock and the controls panel. Hand over in one play session that also covers the landing, the Shooter HUD and the Horror meter. Repeat until all four demos are signed off.
2. **Start the long waits now, in parallel with step 1:**
   - The Fab seller account already exists (the user has other products on Fab).
   - The user installs UE 5.7 and 5.8.
3. **One decision round in plain words:** Lite contents and listing, price, GAS, I-029, engine versions, kit delivery, comfort widget, videos, "juice" in the title.
4. **Developer-only launch prep, in parallel with steps 1 to 3:** move the diagnostics, re-save thumbnails, uplugin metadata and platform list, copyright lines if required, first-hit stall check, GAS package test.
5. **Engine ports** once 5.7 and 5.8 are installed, then the **Lite and Pro packages** for every version.
6. **Kit guides and fresh-template checks** (needs the final demos), then the user follows one guide.
7. **Minimum docs** hosted, so `DocsURL` and `SupportURL` can be filled.
8. **Media:** screenshots and GIFs, then the trailer and videos (needs the final demos and the stall check).
9. **Listing, submission, Fab review**, fixes, go live.

The longest chain is steps 1, 6, 8 and 9 in that order. The engine ports (5) and the Fab account (2) must finish before 9, so they should start early.

### 4.3 Main risks
| Risk | Why it matters | Mitigation |
|---|---|---|
| PlayStation rumble on Windows (I-029) | Buyers test with the pad they own; a "rumble does not work" review hurts | Decide early. State the platform limit plainly in the FAQ and the listing. The Debugger already says when vibration is silenced |
| Fab review | Rejections for technical rules (copyright lines, platform list, required plugins, the extra GAS folder in option B) and unknown waiting time | Check Fab's current code plugin requirements before packaging. Plan for one resubmission. Keep option C ready for GAS |
| UE 5.7 and 5.8 API changes | Zero-warnings rule (hard rule 7) turns every new deprecation into work; content must stay saved in 5.6 | Install early; wrap newer APIs in version macros (hard rule 1); never re-save content in newer versions (C-001c) |
| Lite/Pro upgrade path | Two plugins with the same module names cannot both be enabled; recipes made in Lite must open in Pro | Decide the naming before building anything; test the upgrade in one project |
| Action/RPG feel under motion blur | Punch and readability pull in opposite directions (D-068); already 3 rounds | Measure the on-screen knock (in progress); offer options that do not turn the camera fast (push, freeze, FOV, flash, sound) |
| Kit delivery | Shooter and Horror need C++ edits; the guard enemy is a copy of Epic's Blueprint and cannot ship | Ship code files and guides, verify on fresh templates, say "C++ project" in the listing |
| Launching before the docs (D-051) | Without a Quick Start, first-time buyers struggle (NF-020) | Ship the minimum set: Quick Start, FAQ, changelog |
| Epic content in videos and a downloadable demo | Licence to be confirmed | Check before recording |
| Name "juice" next to Game Juice Pro (D-064) | Search value against confusion with the main competitor | Decide in the listing round |
| No version control (D-001) | A bad script run or packaging step can lose work; backups are per script | Keep the backup-first rule; take one full copy of both projects before the launch packaging |

---

## 5. Next five steps
1. Finish the Action/RPG knock measurement and retune the hit. Add the controls panel. Hand over one play session for all four levels (Action/RPG, Platformer landing, Shooter HUD, Horror meter), only after the builds are verified.
2. User: install UE 5.7 and 5.8 (planned for 2026-09-24 morning). The Fab seller account already exists.
3. Hold one decision round in plain words: Lite contents and listing, Pro price, GAS option (D-066), I-029 wording, engine versions at launch, kit delivery method, comfort widget yes or no, video method.
4. While the user plays and decides: move the demo diagnostics out of the plugin, re-save the library thumbnails, fix the uplugin metadata and platform list, check the copyright-line rule, run the first-hit stall check in a packaged build, test GAS option B in a real package.
5. Update the log and `CLAUDE.md` with tonight's work and the inconsistencies in 2.8, then port and verify on 5.7 and 5.8 as soon as they are installed.
