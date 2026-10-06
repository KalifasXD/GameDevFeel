# FeelKit: Launch

Everything left before FeelKit goes live on Fab, and everything the listing needs. Updated 2026-09-26.

This document replaces the go-live check, the launch readiness report, the two UE 5.7 / 5.8 reports, the Fab rules and
price research, and the marketing brief (originals in `Docs/Archive/`).

## 1. Where we are

- **The plugin is ready.** Feature complete; 117 of 117 tests and 7 of 7 editor window tests; strict Fab builds with
  0 warnings on UE 5.6, 5.7 and 5.8; the GAS add-on proven on all three; no tests in the shipped package; all four demos
  approved; comfort menu built and in every demo level.
- **What stops the launch is launch work:** the manual, the Lite package, the listing and its media, the demo handout,
  and the final builds.

## 2. Work left

### 2.1 Developer

| Item | State | Blocks launch? | Estimate |
|---|---|---|---|
| **Manual** (Word and PDF) | **Written 2026-09-26:** chapters 1 to 23 and appendix A, 200 pages, every figure taken by script or drawn; left: appendix B (the sword-hit walkthrough), planned as Later, the PRO label design is D4, a tinted band on Pro headings (D-110, built 2026-09-28); the Feel for Unity appendix is dropped (D-108) | Yes (Fab 4.3.8). **Online 2026-09-28:** https://drive.google.com/file/d/1OlIEERggkVAoG0rnG2FBbLPwHgKcjn_R/view?usp=sharing, linked in both listings and as `DocsURL` (D-111) | |
| **Lite package:** Pro-only code left out, no clashes when a project moves to Pro, upgrade path, one package per engine version | **Done 2026-09-26** (D-100, D-101, D-104): six packages checked, upgrade checked | Yes | |
| **Listing text:** description, technical information, third-party declaration, tags (no "juice"), for Lite and Pro | **Done 2026-09-26:** `Publish/<Edition>/02-Fab-Listing.md` (D-102) | Yes | |
| **Marketing plan** per your brief (section 7): gallery, hero image, shot list, video scripts, asset inventory, order | Gallery and hero done 2026-09-26 (`Publish/<Edition>/01-Images`, all real captures); video scripts in the Video Guide | Yes, before media | |
| **Demo handout script:** copy of each 5.6 demo project without our development tools, test content or the plugin, plus a readme (D-091) | Not started | For the source offer | 4 to 8 h |
| **Final builds and tests** on 5.6, 5.7 and 5.8 after all the above; the Fab zips | **Done 2026-09-26:** six zips in `Publish/<Edition>/03-Upload`; rebuild only if the plugin changes again | Yes | |
| Add `FabURL` after Fab's first review and rebuild the zips (D-103) | After submission: `package_all.ps1 -FabUrlLite -FabUrlPro`, then `make_uploads.py` | Yes | 0.5 h |
| **Release by parameter** (D-113) and **release recipes** (D-114): Release Parameter, Release At, Jump to End on Release, On Full Release and On Early Release; ChargeUp releases itself at full charge and plays ChargedRelease; library import crash fixed (I-049, I-051); Play Recipe flashes through the flash limiter (I-050); FR_SHOOT_Hurt follows Damage | **Built 2026-10-06, not yet compiled:** the user builds, runs the tests (126) and the editor window tests (7), re-imports ChargeUp, judges the charge in the editor and in play, retakes S05-05 and S05-06, rebuilds the manual and the Video Guide. On the launch branch, so the next package build (the FabURL rebuild) includes it: `package_all.ps1`, `check_packages.ps1`, `check_upgrade.ps1`, `make_uploads.py`, then the new zips go to Fab | Yes, once built: it is in the next packages | 2 to 3 h on the PC |
| Performance number for the listing (50 plays under 0.2 ms, NF-002) | Never measured | Advised | 2 to 4 h |
| Split-screen check (DEL-004) | Never tested | Advised, or say "not tested" | 2 to 3 h |
| Plugin icon (`Resources/Icon128.png`) | Deferred to the screenshot work | Advised | 1 to 2 h |

Done on 2026-09-25: American spelling everywhere buyers read (D-097), recipes re-imported. Done on 2026-09-24: buyer-visible text clean-up (internal codes out of tooltips, settings paths, color spelling in
code), Credits.md corrected, the Action/RPG kit moved out of the plugin, the comfort menu and its key in every demo,
engine-plugin release check on 5.8 (Blueprint-only project, installed like Fab does, packaged successfully).

### 2.2 You

| Item | Blocks launch? |
|---|---|
| Playable demo downloads and their links | Done 2026-09-24 (links in 4.1) |
| Screenshots from the shot list that a script cannot take (list in the Manual Plan) | None left: every shot is taken by script since 2026-09-25 |
| The four basic YouTube tutorials and the two listing videos (Pro, Lite), from the Video Guide: `Docs/Manual/out/FeelKit_Video_Guide.pdf`, source in the last part of the Manual Plan; about 59 hours, V1's Fab install shot only after the listings are live | Yes |
| Gallery images | Done by script 2026-09-26 (`Publish/<Edition>/01-Images`); the listing videos go in second place later |
| A read of the finished manual; upload the PDF to Google Drive and share the link, then put the link in both descriptions | Yes |
| Fab submission: follow `Publish/PUBLISH-README.md` section 2 (zip links, both listings field by field, Publisher Profile support channels) | Yes |
| Send me the product IDs after the first submission (for `FabURL`) | Yes |
| Answer Fab's review | Yes |

**Video guide: for your review after the manual is done (saved 2026-09-25 at your request).**
- **Needs your OK:** record on UE 5.8 in a new Third Person project (not GameFeelDev); capture at 2560 x 1440 through AMD
  Virtual Super Resolution on the RX 7900 XTX, with Unreal's Application Scale at 1.5; record the picture first and the
  voice second; set the monitors to 120 or 60 Hz while recording (at 144 Hz a 60 fps capture judders).
- **Recording:** the narration and the hands-on editor parts are yours (a synthetic voice or a scripted cursor would read
  as generated; about 90% of the running time). I can add a "video mode" to the demo playthrough runners that plays the
  same input twice, feel off then on, in real time while you record with OBS, for frame-identical comparisons with sound
  (about one day of work). OBS, DaVinci Resolve and ffmpeg are not installed yet.
- **Problem found:** no shipped comfort preset turns Flashes fully off (Reduced Flashing uses 0.2), and a substitute plays
  only at 0, so the editor preview never shows "Substitute plays (comfort)" with the presets. The comfort video shows the
  substitute in the running game instead, and the comfort chapter's shot of it is taken in play.

### 2.3 Order

1. Manual look sample and the new shot list (you approve the look).
2. Me: manual chapters, Lite package, handout script, listing text and marketing plan.
3. You: screenshots, tutorials, gallery media, demo downloads.
4. Final builds and zips, submission, `FabURL`, resubmission if Fab asks.

## 3. Decisions taken

| Topic | Decision |
|---|---|
| Editions | Two Fab listings: free Lite, paid Pro (D-081); split in the Product document, section 2 (D-087) |
| Price | Pro $34.99 Personal, $64.99 Professional; Lite free (D-090). Early price; $39.99 Personal possible after reviews |
| Name and wording | FeelKit; "hitstop", "screen shake", "game feel" in title and tags; **no "juice" anywhere in the listing** (D-082) |
| Version | 1.0.0, not beta (D-074) |
| Publisher | Billo, seller page https://www.fab.com/sellers/Billo/about |
| Support | billoue4@gmail.com (in the plugin, the listings and the manual; changed from the earlier address on 2026-09-26, D-105) and Discord https://discord.gg/AtJ6RdwaxA (docs, listing, Publisher Profile) (D-078) |
| Platforms | Every platform Unreal supports; the listing says only Windows was built and tested (D-073) |
| Engine versions | 5.6, 5.7, 5.8 (5.8 is required at the first submission) |
| Tests | Not shipped (D-075) |
| GAS | Add-on folder `Extras/FeelKitGAS` that buyers copy into their project (D-076) |
| Created with AI | Ticked on the listings (D-079) |
| Documentation | One Word manual plus PDF (D-080), written by the developer (D-077), PDF hosted on Google Drive (D-086) |
| Demos | Public playable downloads; owners can request the UE 5.6 source projects on Discord or by email (D-086) |
| PlayStation rumble | Platform note: works through Steam Input; FAQ and listing (D-083) |

## 4. The Fab listings

### 4.1 What goes on them

**Final values for every field are in `Publish/Pro/02-Fab-Listing.md` and `Publish/Lite/02-Fab-Listing.md`
(2026-09-26); the notes below are the brief they were written from.**

- **Title:** built around "hitstop", "screen shake", "game feel".
- **Description:** what FeelKit does, the editions, the demos and their download links, the demo source offer, the
  manual link, support channels. The source offer line: "Owners of FeelKit can request the Unreal Engine 5.6 projects
  behind the playable demos on our Discord or by email."
- **Technical information:** modules; engine versions 5.6, 5.7, 5.8; platforms with Windows the only tested one;
  engine plugins FeelKit enables (Niagara, Enhanced Input); the GAS add-on and how to install it (copy from the engine's
  Fab plugin folder into the project's Plugins folder); Blueprint-only projects need Visual Studio to package;
  PlayStation rumble through Steam Input; "Example Project:" line for the demo downloads.
- **Playable demo downloads** (Google Drive, 2026-09-24):
  - Action/RPG: https://drive.google.com/file/d/1439QD9WBBOfkhdBi9PYWdEE0jxMGH-Ow/view?usp=sharing
  - Platformer: https://drive.google.com/file/d/1dzFHOMr03-rhNYP136canMOdSchmUExf/view?usp=sharing
  - Shooter: https://drive.google.com/file/d/136TpVvZeGb1wYiVQnvFN9dw47Ej3ffHW/view?usp=sharing
  - Horror: https://drive.google.com/file/d/1Akb1gguqHMuv_-cLY3VhWjjfTunTtPkU/view?usp=sharing
- **Third-party declaration:** yes, for the CC0 and CC-BY sounds (form per Fab 4.2.5).
- **Created with AI:** ticked.
- **Tags:** 25 per listing, no "juice" (final lists in the listing files). Fab tags are single lowercase words from Fab's own list; phrases such as "screen shake" or "hitstop" do not exist as tags (D-107).

### 4.2 Fab rules, verified 2026-09-23

Sources: Fab Technical Requirements (updated September 21, 2026), sections 1 and 4; Asset File Format and Structure
Requirements; Publishing Assets for Sale or Free Download; the Created with AI support article.

**Media**

| Requirement | Rule | What it means for FeelKit |
|---|---|---|
| Gallery image size | At least 1920 x 1080 | Capture at 1920 x 1080 or larger |
| Gallery image format | JPEG or PNG | PNG for UI (sharp text); JPEG for gameplay if a PNG goes over 3 MB |
| Gallery image file size | Under 3 MB each | Check each file |
| Gallery total | All images under 25 MB in total | About 10 to 12 PNGs; more only with JPEG |
| Gallery video | 1920 x 1080, up to 300 MB, MP4, MOV or WEBM | A feel off / feel on clip can be uploaded directly |
| Thumbnail | Clear and representative; no separate size rule found | Treat as a gallery image; must read at small size |
| Media accuracy | Media must show the real product (1.8.6.a) | Every visual shows real FeelKit; designed graphics only frame real captures |

**Listing and publisher**

| Requirement | Rule | What it means for FeelKit |
|---|---|---|
| Language | English, correct spelling (1.8.1.a) | Listing and manual in English |
| Technical information | All fields; name dependencies and requirements (1.8.4) | See 4.1 |
| Created with AI | Must be declared (1.8.8.a) | Ticked (D-079) |
| Support | Provide and monitor support channels (1.1.b) | Email and Discord |
| Sales | No sale in the first 30 days (1.7.1.c); no raising the base price just to show a bigger discount later (1.7.2.g) | The launch price stands for a month |
| Third-party content | Sounds, fonts, graphics from other sources must be declared (4.2.5) | Declare the sounds |

**Code plugin files**

| Requirement | Rule | What it means for FeelKit |
|---|---|---|
| Engine versions | The newest engine at first submission (4.2.2.b) | 5.8 required; 5.6 and 5.7 also uploaded |
| One upload per engine version | Separate version and folder each (4.2.2.d) | `Tools/Run/package_plugin.ps1` per engine |
| Epic builds the binaries | Test with BuildPlugin from installed engines (4.3.6.2.b) | Done by `package_plugin.ps1` |
| No errors or warnings | (4.3.6.2.a) | Our rule already |
| `EngineVersion` | Required (4.3.6.a) | Set per package |
| Platform list per module | `PlatformAllowList` or `PlatformDenyList` (4.3.6.b) | Done: runtime modules Win64, Mac, Linux, Android, IOS; editor Win64, Mac, Linux |
| `FabURL` | Required, set after submission (4.3.6.c) | After the first submission |
| Dependencies | Engine plugins only, not other user-made plugins (4.3.6.d) | Met; why the GAS add-on is a folder inside FeelKit |
| Source | Full source (4.3.6.1.a) | Met |
| Copyright | Every source file, publisher name and year of publishing (4.3.6.1.b) | Done: `// Copyright 2026 Billo. All Rights Reserved.`; change the year if launch moves to 2027 |
| No .exe or .msi | (4.3.6.1.e) | Met |
| Installed as an engine plugin | Plugins install to the engine, not the project; an example project is encouraged, hosted outside, depending on the plugin but not containing it (4.3.6.3) | The GAS add-on copy path goes in the manual; demo handouts do not contain FeelKit |
| Folder layout | Only Config, Content, Resources, Source and the .uplugin, others listed in `Config/FilterPlugin.ini` (4.3.7.3) | FilterPlugin lists Library, Demos, Extras, Credits.md |
| Path length | 170 characters or less (4.3.7.3.c) | Checked, none over |
| Names | English letters, digits, underscores, not vague (4.3.7.1) | Checked |
| Documentation | Free, English, covering setup, use and changes (4.3.8) | The manual, before launch |

### 4.3 Price research (2026-09-24)

From Fab's listing search (213 listings for game feel, camera shake, hitstop, feedback, haptics and similar terms).

| Product | Personal | Professional | Rating (count) |
|---|---|---|---|
| Agentic FeedbackFX | $39.99 | $79.99 | 5.0 (13) |
| Game Juice Pro | $29.99 | $44.99 | 5.0 (6) |
| GAME JUICE (Adrenaline Games) | $19.99 | $39.99 | none |
| Feedback Event Factory | $14.99 | $14.99 | 4.43 (7) |
| Easy Player Feedback | $4.99 | $4.99 | 4.29 (7) |
| Cronus Hitstop (single purpose) | $29.99 | $39.99 | 5.0 (7) |

Full frameworks sit at $29.99 to $39.99 Personal; Professional is 1.5 to 2 times Personal. Nobody else has a free
edition. Fab takes 12%: $34.99 leaves $30.79 per sale.

## 5. Engine versions

- **Plugin and GAS add-on: go on 5.7.4 and 5.8.3** (checked 2026-09-24): strict packages 0 warnings, all tests, all
  editor window tests, GAS proofs with and without the add-on.
- Code changes that made this work, all behind engine version checks: a new curve-editor function 5.7 requires, two
  calls with an argument 5.8 deprecates, and an explicit engine version include.
- **Demo levels:** Epic's 5.6 template levels (Combat, Platforming, Side Scrolling, Shooter) crash in Play on 5.7 and
  5.8 even without FeelKit (engine access violation at 0x170); Epic's own 5.7 and 5.8 templates play fine. You checked
  this yourself. So demo source is offered for 5.6 only; buyers on 5.7 and 5.8 use their engine's own templates.
- **Recreating the checks:** the copies used for these checks were deleted on 2026-09-24 (they took 81 GB).
  `Tools/Run/verify_engine.ps1` (our projects on another engine), `make_crash_check.ps1` (Epic's templates),
  `make_poc.ps1` (a fresh 5.8 project with the Platformer kit), `gas_proof.ps1` and `check_engine_plugin.ps1` rebuild
  them in `Build/` when needed.
- **Engine locations:** 5.6 at `B:/UE_5.6`, 5.7 at `D:/EpicGames/UE_5.7`, 5.8 at `D:/EpicGames/UE_5.8`.

## 6. Demo source for buyers (5.6 only)

- The playable demos are public links (you publish them).
- Owners can ask for the Unreal projects behind them, UE 5.6 only, on Discord or by email. Proof of purchase: the Fab
  order number or the buyer's Fab name.
- The handout script (to build) copies GameFeelDev and FeelDemoFP; removes our development tools and test content
  (network test harness, rumble probe, diagnostics, project creator, `Content/FeelKitTests`, builds, logs) and the
  FeelKit plugin folder (Fab rule: example projects depend on the plugin but do not contain it); keeps the demo code
  changes marked `// FeelKit`; adds a readme: UE 5.6 only, install FeelKit from Fab first, why 5.7 and 5.8 crash, where
  each level is, the controls; zips it.

## 7. Marketing brief (yours, 2026-09-23)

Your brief for the marketing, screenshots, tutorials and documentation. Condensed; the full text is in
`Docs/Archive/FeelKit_Marketing_Brief.md`.

- **Research first:** understand the whole product from every document and the code before recommending anything;
  where documents and code disagree, say so.
- **Fab rules verified, not assumed** (done, section 4.2).
- **Lite and Pro marketed accurately and separately,** with no invented differences.
- **Screenshot strategy:** the smallest set that shows what FeelKit is, the workflow, recipes, the timeline, how a
  recipe is played, the Blueprint side, the result in game, comfort, what sets it apart, Lite and Pro. Combine ideas
  where one picture shows several.
- **Each screenshot designed:** purpose, edition, main message, exact content, what not to show, composition, labels,
  real UI or designed, source, production method, priority, file name.
- **Real product, not generic marketing art:** real FeelKit and Unreal UI with clean overlays, callouts and framing.
- **Lite versus Pro visuals:** decide between separate, shared and comparison shots, using only confirmed features.
- **Hero image:** decided by what explains "what is FeelKit" fastest.
- **YouTube tutorials:** the smallest useful set (the manual plan has four); for each, title, purpose, edition,
  length, steps, setup, where it is linked.
- **Documentation:** full structure split into must-have, recommended and later (done in the Manual Plan).
- **Buyer journey:** from search result to purchase, what question each step answers and what must not be buried.
- **Information gaps:** listed, never invented.
- **Checklists and inventory:** Lite, Pro and shared assets; one master table with ID, type, priority, source, method,
  status.
- **Shooting and recording scripts** precise enough to work through in Unreal one by one.
- **Honest wording:** no "revolutionary", "ultimate", "game-changing"; let the workflow show the value.
- **Mark every visual:** REAL, DESIGNED or OPTIONAL.
- **Optimize for Fab:** size, legibility at small size, gallery order.

## 8. Risks

| Risk | Mitigation |
|---|---|
| Fab review finds something | Rules checked (section 4.2); plan for one resubmission |
| A buyer on 5.7 or 5.8 opens the 5.6 demo source and it crashes | The readme and the request reply say 5.6 only and why |
| PlayStation owners report no rumble | FAQ entry and a listing note |
| The manual is late | Reference chapters come from the code; screenshots by script where possible |
| No version control | Backups before every change; one full copy of both projects and the plugin before the release packaging |
| A sync client restores moved or deleted files (happened 2026-09-24) | Pause Synology Drive during reorganizing and release packaging; check the plugin folder before each build |
