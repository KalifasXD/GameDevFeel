# FeelKit go-live check, 2026-09-24

Replaces `FeelKit_Launch_Readiness_2026-09-23.md` (kept for its history and effort tables). Built by reading every
project document (list and state in section 6), the project log and the plugin files.

## Verdict: not ready to go live yet

- **The plugin itself is ready:** feature complete, 116/116 tests on UE 5.6, 5.7 and 5.8, 6/6 editor window tests,
  strict Fab builds with 0 warnings on all three engines, GAS add-on proven, package without tests, all four demos
  approved.
- **What stops the launch** is launch work, not engineering: user decisions (Lite/Pro above all), the manual (Fab
  requires documentation), a buyer-facing text clean-up inside the plugin, the listing and its media, and a few release
  checks.

## 1. Done since the last report (2026-09-23)

| Item | Result |
|---|---|
| All four demos | Approved 2026-09-23 (Action/RPG hit restored to the approved version, D-072) |
| UE 5.7 and 5.8 | Go for the plugin and the GAS add-on (`FeelKit_UE57_UE58_Report_2026-09-24.md`); 5.8 is what Fab requires at the first submission |
| GAS | Add-on folder inside FeelKit, proven off/on in fresh projects on 5.6, 5.7, 5.8 (D-076) |
| Tests out of the shipped plugin | Stripped at packaging, dev copy keeps them (D-075) |
| Plugin details | Version 1.0.0, not beta, publisher Billo, seller page, support email (D-073, D-074, D-078) |
| Copyright lines | `// Copyright 2026 Billo. All Rights Reserved.` in all 231 source files (Fab 4.3.6.1.b) |
| Platform list per module | Present, as Fab requires (4.3.6.b) |
| Library thumbnails | Re-rendered and saved, 79 of 79 |
| Fab requirements | Verified and written up (`Research/Fab_Requirements_2026-09-23.md`) |
| Created with AI | Decided: tick it (D-079) |
| Documentation format | Word document plus PDF (D-080); plan and table of contents ready (`Manual/FeelKit_Manual_Plan.md`) |
| Demo delivery | Decided 2026-09-24: the user publishes a playable download per demo; buyers can request the 5.6 source projects (section 3) |
| PlayStation rumble fix | The software fix (D-046) is in; what remains is the Windows platform limit (section 2, decision 4) |

## 2. Decisions only you can make

Updated the same day with your answers.

| # | Decision | State |
|---|---|---|
| 1 | **Lite / Pro** | Two Fab listings; Lite is a usable taste with nothing crippled, Pro has everything (D-081). **Open:** the exact split (proposal given 2026-09-24: Lite with the recipe editor, about 12 core effects, full comfort and the comfort menu, Feel Switch, about 10 library recipes; Pro adds the other effects, triggers, integrations, multiplayer, the full library and the tools) |
| 2 | **Price** | **Open.** Documents: requirements $39.99 early access then $69.99 to $79.99; accepted strategy about $39.99; Game Juice Pro $29.99. Suggestion $39.99 for Pro |
| 3 | **"Juice"** | Decided: not in the title, not in the tags (D-082) |
| 4 | **PlayStation rumble** | Decided: platform note, Steam Input; FAQ and listing (D-083, I-029 closed) |
| 5 | **Demo recipes in the plugin** | Decided: recipes that need Epic's template content leave the plugin (D-084). Proposal: the whole Action/RPG kit (built around the Combat template's hooks and effect); Platformer, Shooter and Horror stay. **Open:** confirmation |
| 6 | **Comfort settings menu** | Decided: build it and document every setting (D-085). A short design goes to you before building |
| 7 | **Manual table of contents** | **Open:** you are reading it (`Manual/FeelKit_Manual_Plan.md`, first section) |
| 8 | **Manual hosting** | Decided: Google Drive (D-086); the link is needed for `DocsURL` |
| 9 | **Engine-plugin release check** | Allowed and done: passed (section 4.1) |

## 3. Your idea: demo source for buyers, 5.6 only

Understood and sound. The playable demos are public links. Owners of FeelKit can ask for the Unreal projects behind
them, for UE 5.6 only, on Discord or by email. Game Juice Pro offers a playable demo and no source, so this goes beyond
the main competitor.

What it needs:
- **A clean handout copy of both 5.6 projects** (Third Person: Action/RPG and Platformer; First Person: Shooter and
  Horror). Remove our development tools and test content (network test harness, rumble probe, diagnostics, project
  creator, `Content/FeelKitTests`, packaged builds, logs). Keep the demo code changes marked `// FeelKit`.
- **No FeelKit inside the handout.** Fab's rule for example projects is that they depend on the plugin but do not
  contain it; the buyer installs FeelKit from Fab first (4.3.6.3.c).
- **A one-page readme per project:** UE 5.6 only, install FeelKit from Fab first, the project crashes if opened in 5.7 or
  5.8 because of Epic's 5.6 template levels, where each demo level is, the controls.
- **One line in the manual and on the Fab page**, for example: "Owners of FeelKit can request the Unreal Engine 5.6
  projects behind the playable demos on our Discord or by email."
- **Proof of purchase:** ask for the Fab order number, or check the buyer's Fab name.
- Depends on decision 5: if the demo recipes leave the plugin, they go into these projects.

## 4. Work left

### 4.1 Developer (me)

| Item | Blocker? | Estimate |
|---|---|---|
| **Buyer-visible text in the plugin.** 76 internal requirement codes such as "(ST-001)" in public header comments, which Unreal shows as tooltips (59 files mention internal codes); 14 messages point to "Project Settings > FeelKit" instead of "Project Settings > Plugins > FeelKit"; "color" and "colour" mixed in user-facing text | Yes (hard rule 9, and buyers see it) | 3 to 5 h |
| **Credits.md:** says everything is CC0, then lists FeelKit-licensed materials; `M_FK_HitFlash` and `M_FK_GuardShield` are missing (all 53 sounds are listed) | Yes, small | 0.5 h |
| **Move the demo recipes that need Epic's templates out of the plugin** (D-084), into the demo project, and re-point the level | Yes | 3 to 6 h |
| **Handout copies of the two 5.6 demo projects** (section 3) | Yes, for the source offer | 4 to 8 h |
| **The manual** (.docx and PDF, after the table of contents is approved), including the source-on-request line | Yes (Fab 4.3.8) | 30 to 50 h |
| **Lite package** (D-081: Lite ships): Pro-only code left out of the Lite download, naming so Lite and Pro never clash, upgrade path, its own packages for 5.6/5.7/5.8 | Yes | 12 to 24 h |
| **Comfort settings menu** (D-085): design, build (Lite and Pro), document | Yes | 8 to 16 h |
| **Listing text:** description, technical information (modules, engine versions, platforms with Windows the only tested one, dependencies Niagara and Enhanced Input, the GAS add-on), third-party declaration for the CC0 sounds, tags | Yes | 4 to 8 h |
| **Marketing plan per your brief** (gallery, hero image, shot list, video scripts, asset inventory, production order) | Yes, before media work | 6 to 10 h |
| **Release checks:** FeelKit installed as an engine plugin in a Blueprint-only project on 5.8; package that project (Visual Studio present) | **Done 2026-09-24: passed** (`check_engine_plugin.ps1`) | 0 h |
| **Final builds and tests on 5.6, 5.7, 5.8** after all the above; make the three Fab zips | Yes | 3 to 4 h |
| **Record keeping:** the stale documents in section 6 | No, but they mislead | 1 to 2 h |
| Performance number for the listing (50 plays under 0.2 ms, never measured) | Advised | 2 to 4 h |
| Split-screen check (never tested) | Advised, or say "not tested" in the manual | 2 to 3 h |
| Plugin icon (`Resources/Icon128.png`), deferred to the screenshot work | Advised | 1 to 2 h |

**Total developer work to go live:** about **75 to 140 hours**, most of it the manual (Lite and the comfort menu are in).

### 4.2 You

| Item | Blocker? |
|---|---|
| Decisions 1 to 9 in section 2 | Yes |
| The playable demo downloads and their links | Yes (you said you have this) |
| The four basic YouTube tutorials (recording, voice), from the scripts in the manual plan | Yes, by your plan |
| Screenshots and GIFs for the Fab gallery, from the shot list I prepare (1920 x 1080 or larger, under 3 MB each, under 25 MB total) | Yes |
| A read of the finished manual | Yes |
| Fab submission: three uploads (5.6, 5.7, 5.8), Created with AI ticked, third-party declaration, support channels in your publisher profile | Yes |
| After the first submission: send me the product ID so I add `FabURL` to the plugin (Fab 4.3.6.c) and rebuild the zips | Yes |
| Answer Fab review | Yes |

### 4.3 Suggested order

1. Decisions 1 to 9 (one short round).
2. Me, in parallel: text clean-up and Credits, demo recipes out (if chosen), handout projects, release checks.
3. Manual writing, starting with the reference chapters; marketing plan and shot list.
4. You: demo downloads, tutorials and screenshots from the shot list.
5. Final builds and zips, listing text, submission, `FabURL`, resubmission if Fab asks.

## 5. Risks

| Risk | Mitigation |
|---|---|
| Fab review finds something (unknown wait) | The rules are checked (Fab_Requirements); plan for one resubmission |
| Buyers on 5.7 / 5.8 open the 5.6 demo source and it crashes | The readme and the request message say 5.6 only and why |
| PlayStation owners report no rumble | FAQ entry and a listing note (decision 4) |
| Manual too late | Write the reference chapters from the headers first (generated); the guide chapters follow the approved outline |
| No version control | Take one full copy of both projects and the plugin before the release packaging |

## 6. Documents read, and how current they are

| Document | State |
|---|---|
| `CLAUDE.md` | Mostly current. Stale: "Documentation may follow the release (D-051)" (Fab now requires docs, D-077); I-029 still a "go-live blocker"; "library recipes ... imported by the user" (D-056 changed that) |
| `Docs/FeelKit_ProjectLog.md` | Records are current up to today. Stale: the status section at the top stops at 2026-09-23 night; issue rows I-006, I-007, I-010, I-011, I-013 to I-020 still say "user retest pending" although Phase 5 was confirmed complete on 2026-09-17; the I-029 row says "waiting for the user to confirm"; section 7 "Remaining work" still lists demos waiting for play and the GAS decision |
| `Docs/FeelKit_Launch_Readiness_2026-09-23.md` | Superseded by this report |
| `Docs/FeelKit_UE57_UE58_Report_2026-09-24.md`, `FeelKit_UE57_UE58_CrashCheck.md` | Current |
| `Docs/Manual/FeelKit_Manual_Plan.md` | Current; waits for your approval |
| `Docs/Launch/FeelKit_Marketing_Brief.md` | Current (your brief) |
| `Docs/Research/Fab_Requirements_2026-09-23.md`, `GAS_Optional_Integration_2026-09-23.md` | Current |
| `Docs/Research/Competitor_Analysis_2026-09-16.md` | Mostly current; FeelKit's own column is out of date (it lists 0 recipes and no demos) |
| `Docs/Research/Adjacent_Systems_and_Demos_2026-09-16.md` | Background research, still valid |
| `Docs/FeelKit_Phase6B_Demos_Design.md` | Stale in places: describes the replaced Action/RPG hit (D-070), flashes removed from heavy strikes, the replaced project-path setup, the old "Show Built-in UI" name, and kits delivered as guides |
| `Docs/FeelKit_Phase6B_UserTests.md` | Current status table; its "Earlier" section describes the D-070 camera, which D-072 replaced |
| `Docs/FeelKit_Demos.md` | Stale: five demos, and the user creating every asset (before D-056 and D-064) |
| `FeelKit_Requirements.md` | The original requirements. Several were changed by decisions (Lite contents, docs timing, optional modules, sample widget); read with the decisions table |
| `FeelKit_Strategy.md` | Superseded by `Docs/FeelKit_Strategy_v2_Proposal.md` |
| `Docs/FeelKit_Strategy_v2_Proposal.md` | Accepted strategy; its price idea was never decided |
| `Docs/FeelKit_Phase5_UserTests.md`, `FeelKit_Phase6A_*` | History, complete |
