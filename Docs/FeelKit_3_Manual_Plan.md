# FeelKit: Manual Plan

## Contents for your approval

The deliverable is one Word document, the **FeelKit Manual**, with a PDF made from it. It has two parts: a guide you read in order, and a reference you look things up in. **[Video n]** means the section links to one of the four YouTube tutorials and still covers the same steps in text and screenshots.

Priority: **Launch** = must exist before launch. **Recommended** = should exist at launch. **Later** = can follow in an update. **To decide** = waits on a decision of yours.

**Part One: Using FeelKit**

1. **Welcome** (Launch)
   - 1.1 What FeelKit does: gameplay says what happened, and a recipe decides how it feels.
   - 1.2 What is in the package: the plugin, 38 library recipes, sample sounds and materials, demo recipes, the GAS add-on.
   - 1.3 How to use this manual: the two parts and the four videos.
   - 1.4 Getting help: Discord and email.
2. **Core ideas** (Launch)
   - 2.1 Recipes, tracks and steps.
   - 2.2 Channels, and what happens when effects overlap.
   - 2.3 Intensity: how strong each effect plays.
   - 2.4 Targets: what a recipe plays on.
   - 2.5 Parameters, accumulators and sustained recipes, in brief.
   - 2.6 Events and Feel Maps, in brief.
   - 2.7 Comfort on one page.
   - 2.8 Why the editor preview matches the game.
3. **Installing FeelKit** [Video 1] (Launch)
   - 3.1 Requirements: engine versions, platforms, and when Visual Studio is needed.
   - 3.2 Installing from Fab and enabling the plugin.
   - 3.3 What FeelKit switches on for you (Niagara, Enhanced Input).
   - 3.4 Showing FeelKit content in the Content Browser.
   - 3.5 Adding the GAS add-on (a folder you copy).
   - 3.6 Blueprint-only projects and packaging.
   - 3.7 Moving a project from Lite to Pro (added 2026-09-26, D-104).
4. **Quick start: your first recipe** [Video 1] (Launch)
   - 4.1 Pick a recipe from the library.
   - 4.2 Preview it without pressing Play.
   - 4.3 Play it from a Blueprint.
   - 4.4 Compare with the feel off (Feel Switch).
   - 4.5 Where to go next.
5. **The recipe editor** [Video 2] (Launch)
   - 5.1 Creating and opening recipes.
   - 5.2 The window: Preview, Timeline, Details, Intensity.
   - 5.3 The toolbar.
   - 5.4 Adding, moving and resizing tracks.
   - 5.5 Shaping intensity with curve keys.
   - 5.6 Recipe and track settings in Details.
   - 5.7 The preview: play, loop, scrub, preview mesh, parameter sliders.
   - 5.8 Sound tracks: waveforms, snapping, tracks made from a sound.
   - 5.9 Checking comfort, and the intensity graph.
   - 5.10 Play in PIE, and editing while the game runs.
   - 5.11 Copy, paste and keyboard shortcuts.
   - 5.12 Validation messages on save.
6. **Playing recipes in your game** [Video 3] (Launch)
   - 6.1 Play Feel, and choosing a target.
   - 6.2 Passing context with Play Feel With Context.
   - 6.3 Handles: stop, release, change a parameter.
   - 6.4 Play Feel And Wait.
   - 6.5 Events and Feel Maps.
   - 6.6 Animation notifies.
   - 6.7 The Feel Trigger component (no Blueprint wiring).
   - 6.8 Enhanced Input: the Feel Input component.
   - 6.9 Gameplay Ability System cues (add-on).
   - 6.10 Using FeelKit from C++.
   - 6.11 The Feel Switch: feel off and on while playing.
7. **Parameters, context and variation** (Recommended)
   - 7.1 Parameters and track mappings.
   - 7.2 The Distance parameter.
   - 7.3 Accumulators: effects that build up with repeated plays.
   - 7.4 Sustained recipes.
   - 7.5 The instigator, and the Applies To setting.
   - 7.6 Directions and locations.
   - 7.7 Randomness, conditions and Random Choice.
   - 7.8 Recipes inside recipes.
8. **Comfort and accessibility** [Video 4] (Launch)
   - 8.1 Comfort groups, and how the scales apply.
   - 8.2 Presets.
   - 8.3 Reading and changing a player's settings.
   - 8.4 Essential tracks and substitutes.
   - 8.5 The flash limiter.
   - 8.6 Motion comfort: camera roll and field of view speed.
   - 8.7 Engine camera shakes and controller vibration.
   - 8.8 Saving settings, or using your own save system.
   - 8.9 Building a comfort menu for your players.
   - 8.10 The Comfort Audit.
9. **The recipe library and browser** (Launch)
   - 9.1 The Recipe Browser.
   - 9.2 Filters, search and live preview.
   - 9.3 Copying a library recipe into your project.
   - 9.4 Why library recipes are read-only.
   - 9.5 Content Browser tiles and tooltips.
   - 9.6 The 38 library recipes at a glance.
   - 9.7 Tagging your own recipes.
10. **Debugging and tuning** (10.1 to 10.3 Launch, the rest Recommended)
    - 10.1 The FeelKit Debugger.
    - 10.2 Replaying a recent play in the editor.
    - 10.3 Track labels that explain why a track did not play.
    - 10.4 showdebug feel and stat Feel.
    - 10.5 Log messages.
    - 10.6 Capture GIF.
11. **Multiplayer** (Recommended)
    - 11.1 What travels over the network.
    - 11.2 The Feel Replication component and its modes.
    - 11.3 Relevancy distance.
    - 11.4 Hitstop and slow motion in networked games.
    - 11.5 Dedicated servers.
12. **Performance** (written 2026-09-26, no measured numbers)
    - 12.1 When FeelKit runs. 12.2 What a play costs. 12.3 Measuring it. 12.4 Keeping many plays in check. 12.5 Assets and memory.
13. **Demo kits** (To decide: depends on how the kits reach buyers)
    - 13.1 The four demos and what each shows.
    - 13.2 Action/RPG on the Combat template.
    - 13.3 Platformer on the Platforming template.
    - 13.4 Shooter on the First Person Shooter template.
    - 13.5 Horror on the First Person Horror template.
14. **Troubleshooting and questions** (Launch; written 2026-09-26)
    - 14.1 Nothing plays. 14.2 Controllers and vibration. 14.3 Camera and screen effects. 14.4 Materials and decals.
    - 14.5 Blueprints, animations and GAS. 14.6 Recipes and the library. 14.7 Installing, packaging and engine versions. 14.8 Getting help.
15. **Lite and Pro** (Launch; written 2026-09-26 from the built editions, D-100)
    - 15.1 What each edition contains. 15.2 What Lite leaves out. 15.3 The Lite library. 15.4 Recipes and data in Lite. 15.5 Moving from Lite to Pro. 15.6 Licenses.

**Part Two: Reference**

16. **Step reference**: all 37 steps, grouped by camera, time, screen, actor, audio, haptics, UI, spawn and meta (Launch).
17. **Blueprint nodes and components** (Launch).
18. **Project settings and editor preferences** (Launch).
19. **Console commands** (Launch).
20. **Writing your own steps** in Blueprint and C++ (Recommended).
21. **Recipe JSON files and editor scripting** (Later).
22. **Credits and licenses** (Launch).
23. **Version history** (Launch).

**Appendices**
- A. Glossary (Recommended).
- B. Walkthrough: building the Action/RPG sword hit. This is the one "how I made it" tutorial agreed earlier (Later).

**Videos** (each about 6 to 9 minutes):
- 1: Install FeelKit and play your first recipe.
- 2: Build a recipe on the timeline.
- 3: Trigger recipes from your game.
- 4: Comfort settings for your players.

---

## 1. Audience, format and production

### Audience
- Unreal developers who bought a game-feel plugin.
- Blueprint users. Chapters 1 to 10 are written for them, and every task has a Blueprint path.
- C++ users. They get 6.10, 17, 20 and 21.
- Designers who only author recipes. They get 5, 7 and 9.
- Assume the reader knows the Unreal editor, but not FeelKit's own terms.

### One document or several
Recommend **one .docx with two parts** (Guide, then Reference), for four reasons:
- Buyers get one link, and one Ctrl+F searches everything.
- Cross-references inside one document work as real links, in Word and in the PDF. Links between separate files break once the files are hosted.
- `DocsURL` in `FeelKit.uplugin` holds one address.
- The reference tables are generated from the headers either way.

If the document grows past about 150 pages, the same build script can also produce the Reference as a separate file from the same sources. It is not needed at launch.

### Word structure
- **Headings.** Built-in styles only: Heading 1 for chapters, Heading 2 for sections, Heading 3 for subsections. Numbers are written into the heading text by the build script, for example "5.4 Adding, moving and resizing tracks", so they stay stable in the PDF.
- **Navigation.** Word's Navigation Pane and a TOC field (`TOC \o "1-3" \h \z \u`) placed after the title page give the table of contents.
- **Page setup.** A4, margins 2 cm, body text 10.5 pt. The page size is a gap (see section 6).
- **Text styles.** UI labels in bold, exactly as the editor shows them. Code names in a monospace character style. Tables use one table style with a header row.
- **Cross-references.** Every heading gets a bookmark (for example `ch05_tracks`). A reference in the text reads "see 5.4 Adding, moving and resizing tracks" and is an internal hyperlink to that bookmark. Page numbers are optional (a PAGEREF field, filled when Word updates fields).
- **Screenshots.** In the source text a shot is written as `[shot: S05-02 | caption]`.
  - If `Docs/Manual/shots/S05-02.png` exists, the script inserts it at full text width (up to 17 cm) with a caption below: "Figure 5.2: caption", numbered by a SEQ field.
  - If the file is missing, the script inserts a gray bordered box (a one-cell table) that shows the shot ID, the description and the required state, so the gaps are visible in a draft.
- **Video links.** Written as `[video: V2]`. The script turns it into a shaded box: "Video: Build a recipe on the timeline (YouTube, about 9 minutes)" plus the URL, taken from `Docs/Manual/videos.csv`. The URLs do not exist yet.

### Production and keeping it in sync
Files (not shipped inside the plugin). Sources and output in `Docs/Manual/`, scripts in `Tools/Manual/`:

| Path | Content |
|---|---|
| `Docs/Manual/src/NN_name.md` | One source file per chapter, in a restricted Markdown subset: `#`/`##`/`###` headings, `{#anchor}`, paragraphs, lists, pipe tables, fenced code, `**bold**`, `` `code` ``, plus `[see: anchor]`, `[shot: ...]`, `[video: ...]` |
| `Tools/Manual/gen_reference.py` | Reads `Source/*/Public/*.h` and the step `.cpp` constructors and `GetDefaultChannel_Implementation`/`SupportsPreview_Implementation`. Writes `src/16_steps.generated.md`, `17_nodes.generated.md` and `18_settings.generated.md`: class, display name, property display names, defaults and tooltips. A small hand-kept `overrides.yaml` holds defaults set in `.cpp` constructors (for example FOV Kick's Shape = Kick). Rerun it whenever the plugin changes. |
| `Tools/Manual/build_manual.py` | Builds the .docx with python-docx. With `--pdf`, it drives Word through COM (pywin32) to update all fields (TOC, SEQ, PAGEREF), save, and export a PDF with heading bookmarks (`ExportAsFixedFormat`, CreateBookmarks = heading bookmarks). |
| `Tools/Manual/lint_manual.py` | Enforces the style rules (section 8), including shipped-text rules the eye misses (em dashes, internal IDs). |
| `Docs/Manual/Screenshots/` | `<ShotID>.png`, one file per shot in the screenshot table at the end of this document |
| `Tools/Manual/manual_tables.py` | Reads the outline, screenshot and video tables at the end of this document for the other scripts |
| `Docs/Manual/out/` | `FeelKit_Manual_<version>.docx` and `.pdf` |

Checked on this machine (read-only): Python 3.13.2, python-docx 1.2.0, pywin32 and Pillow are installed, and Word (Office16) is present. Pandoc, LibreOffice and the Python `markdown` package are not. So the script parses its own Markdown subset.

**Metadata, required by hard rule 9 (no AI trace).** python-docx's default template stamps author "python-docx" and a "generated by python-docx" comment. The script must overwrite the core properties:
- Title: FeelKit Manual.
- Author and Last Modified By: Billo.
- Subject: the version.
- Comments: empty.

The Word save step rewrites the app properties. The lint must read the finished file's properties back.

### PDF and Fab
- Fab rule 4.3.8 accepts a web guide, txt/pdf, comments, videos or in-editor tutorials. A .docx is not named. So the **PDF is the published form**: it goes behind `DocsURL` and the listing link. The .docx is the editable master and can be offered next to it.
- Shipping the PDF inside the plugin (a `/Docs/...` line in `Config/FilterPlugin.ini`) is optional. Fab installs plugins under `Engine/Plugins/Fab`, where buyers rarely look. That is your decision (section 6).
- Download links must not start a download automatically (4.3.8).

---

## 2. The four YouTube videos

All four are recorded in a clean project made from Epic's **Third Person** template with FeelKit installed the way a buyer gets it, not in GameFeelDev, which holds test content. The steps below are what the manual covers in text too. Real play needs the user's hardware and voice (launch readiness 3.2), so recording is yours.

**Video 1: Install FeelKit and play your first recipe.** About 6 minutes. Linked from 3.2 and 4.1.
1. Add FeelKit to the engine from Fab (the launcher's install-to-engine flow; exact buttons to be captured at the first real install).
2. Create a Third Person project.
3. Edit > Plugins, search "FeelKit", enable it, restart. Say that Niagara and Enhanced Input are enabled with it.
4. Content Browser **Settings** > **Show Plugin Content**, then **All** > **Plugins** > **FeelKit Content**.
5. **Tools** > **FeelKit Recipe Browser**. Hover over a few recipes to hear and see them. Tick the Impact feeling.
6. Select `FR_Impact_HeavyHit` > **Use** > save it as `HeavyHit` in `/Game`.
7. In `BP_ThirdPersonCharacter`, add a keyboard key event > **Play Feel** (Recipe HeavyHit, Target **Make Feel Target From Actor** with self). Compile.
8. Press Play, press the key.
9. Drag a **Feel Switch** into the level, press Play, and press Tab to compare.

**Video 2: Build a recipe on the timeline.** About 9 minutes. Linked from 5.1 and 7.1.
1. Right-click > **FeelKit** > **Feel Recipe**, name it `Hit`, and open it.
2. **+ Track** > Global Hitstop. Drag the track to about 0.08 s. Then add Camera Punch, Procedural Shake, Screen Flash, Play Sound (`S_FK_Hit_Heavy`) and Force Feedback Curve.
3. Move and resize tracks with Snap on. Hold Shift to ignore snapping.
4. Double-click the Intensity lane to add keys. Right-click a key to change interpolation.
5. Change the Camera Punch settings in Details. Play, Loop and scrub the preview. Point out the "No preview" labels on the hitstop and rumble tracks.
6. Add the parameter `Damage` (0 to 100) and a mapping on the shake and punch. Drag the **Preview parameters** slider.
7. Switch the **Comfort** dropdown to Reduced Motion. Open the **Intensity** tab.
8. Start PIE, press **Play in PIE**, change a value, and play again.
9. Save, and show a validation warning.

**Video 3: Trigger recipes from your game.** About 9 minutes. Linked from 6.1, 6.5, 6.6 and 6.7.
1. **Play Feel With Context** with **Make FeelPlayContext**: Location, Instigator, a Damage parameter.
2. Keep the handle. **Stop Feel**. **Release Feel** on a copy of `FR_Power_ChargeUp` held on a key. **Set Feel Parameter** with its dropdown, which lists the parameters of the recipe the handle came from.
3. Create a Feel Map with the row `Feel.Event.Hit.Landed` > `Hit`, add it under Project Settings > Plugins > FeelKit > **Feel Maps**, and call **Send Feel Event**.
4. Add a **Play Feel** notify in an animation, and show that it plays while scrubbing the animation editor.
5. Add a **Feel Trigger** component with a Landed entry > a copy of `FR_Weight_Land`, Value Parameter `FallSpeed`.
6. Open **Tools** > **FeelKit Debugger** during PIE.

**Video 4: Comfort settings for your players.** About 7 minutes. Linked from 8.1 and 8.3.
1. Use the preview **Comfort** dropdown on a copy of `FR_Dread_JumpScare` and see "Substitute plays (comfort)".
2. Project Settings > Plugins > FeelKit > **Comfort**: default scales and the three presets.
3. Blueprint: **Get Feel Comfort** > **Apply Comfort Preset** (Reduced Motion) and **Set Comfort Group Scale** on keys. Play and compare. Show that the settings survive restarting Play.
4. Mark a track **Essential** with a **Substitute Step**.
5. Show the flash limiter labels.
6. Show the Debugger comfort rows and the "Controller vibration" row.
7. **Tools** > **FeelKit Comfort Audit**, and read the Message Log page.

If Lite ships without the recipe editor (still open), Video 2 is Pro-only and Videos 1 and 3 need a Lite variant. See section 6.

---

## 3. Chapter plan

For each chapter: source file, the question it answers, outline notes, facts with where each comes from, and screenshots.

**Screenshot method codes:**
- DIAG: an existing picture diagnostic in the development copy: `DiagFeel.RecipeEditorShot` (writes `Saved/FeelKit/RecipeEditor.png` and `RecipeEditorSelected.png`), `DiagFeel.BrowserShot` (`RecipeBrowser.png`), `DiagFeel.DebuggerShot` (`Debugger.png`), `DiagFeel.ThumbnailSheet`, `DiagFeel.WindowShot` (every open editor window, after you set the state by hand), `FeelKit.Editor.FeelSwitchInPlay` (`FeelSwitch_<n>_<label>.png`), `DiagFeel.ARPGGuardPlaythrough` (`Guard_<n>.png`).
- HAND: captured by hand at 1920x1080 or larger, PNG.
- Project codes: CLEAN = the clean Third Person project, GFD = GameFeelDev, FP = FeelDemoFP.
- Before shooting library tiles, the library thumbnails are already refreshed (`DiagFeel.RefreshRecipeThumbnails`, log 2026-09-23).

### Chapter 1. Welcome (`src/01_welcome.md`)
**Buyer question:** What did I buy, and where do I get help?

**Facts and sources:**
- Version 1.0.0, not beta; publisher Billo (`FeelKit.uplugin`; D-074).
- Modules: FeelCore (runtime), FeelEditor (editor), FeelNiagara, FeelEnhancedInput (`FeelKit.uplugin`). The GAS add-on FeelKitGAS with module FeelGAS is in `Extras/FeelKitGAS` (`FeelKitGAS.uplugin`, D-076).
- 38 library recipes across 9 feelings (`Library/`).
- Demo recipes: Action/RPG 8, Platformer 5, Shooter 20, Horror 8 (`Demos/`). Feel Map `FM_ARPG` (`Content/Demos/ActionRPG`).
- Samples: 53 sounds and the attenuation `ATT_FK_World` (`Content/Samples/Sounds`). Recount when writing.
- Materials: `M_FK_Decal_Scorch`, `M_FK_HitFlash` and `M_FK_PP_Pulse` in `Content/Samples/Materials`, and `M_FK_GuardShield` in `Content/Demos/ActionRPG`.
- Support: Discord https://discord.gg/AtJ6RdwaxA and email (D-078; `SupportURL`).
- Position line: CLAUDE.md "Product decisions". Rewrite it in plain words, not as a tagline.

**Shots:**
- S01-01: recipe editor on a project copy of `FR_Impact_HeavyHit`, timeline with 7 tracks, preview mid-hit (HAND, CLEAN).
- S01-02: Content Browser at **FeelKit Content** showing Library, Samples and Demos, tile view (HAND).

### Chapter 2. Core ideas (`src/02_concepts.md`)
**Buyer question:** What are recipes, tracks, steps, channels and intensity, and how do they fit together?

**Facts:**
- **Recipe** (`UFeelRecipe`, a data asset).
  - Tracks.
  - Cooldown 0, Max Concurrent 0 (0 = unlimited, counted per target), Default Intensity 1.
  - Parameters.
  - Sustain off, with Sustain Start 0 and Sustain End 1.
  - Editor-only Library data (Feeling, Genres, Description, Based On) and Preview Mesh (`FeelRecipe.h`).
- **Track** (`FFeelTrack`).
  - Step, Start Time 0, Duration 0.5 s (0 = an instant track), Channel, Intensity Curve (empty = constant 1), Seed.
  - Applies To (Play Target or Instigator), Parameter Mappings, Random Intensity and Random Duration Scale (1 to 1), Enabled.
  - Comfort: Essential, Substitute Step, Essential Floor 0.
  - Conditions: Local Player Only, Max Distance 0 (unlimited), Chance 1, Platforms (`FeelTrack.h`).
- **Step:** computes output from time, intensity and seed only. That is why scrubbing works in any direction (`FeelStep.h` class comment; CLAUDE.md "Pure evaluation").
- **Channels:** the 18 `Feel.*` tags (`FeelTags.cpp`), for example `Feel.Camera.Shake` and `Feel.Screen.Flash`.
- **Overlap rules:**
  - Tracks inside one play add up; separate plays compete (CLAUDE.md Phase 2 decisions).
  - Camera: Strongest Wins by default, or Additive Capped (`FeelSettings.h`).
  - Time: one owner per clock, highest Priority wins (`FeelStep_Hitstop.h`, `FeelOutputSink.h`).
  - Screen and haptics: strongest wins (`FeelOutputSink.h`).
- **Intensity:** call intensity x Default Intensity x curve x parameter mappings x random intensity (`FeelEvaluator.h`, `ComputeTrackIntensity`), then x comfort (group x Master) and x `feel.GlobalScale` (`FeelSubsystem.cpp` line 1258). A Feel Map row adds its Intensity Scale.
- **Timing:** recipes run in real time, so hitstops never slow the recipe itself. The runtime ticks only while something plays or needs restoring (`FeelSubsystem.cpp` `IsTickable`).
- **Same output in editor and game:** CLAUDE.md "Key design decision".

**Examples:** `FR_Impact_ScalableHit` (Damage 0 to 100) and `FR_HOR_OutOfBreath` (sustained 0.1 to 1.0 s).

**Shots:**
- S02-01: timeline of the `HeavyHit` copy with numbered callouts for recipe, track, step name and channel color (HAND, callouts added in Word).
- S02-02: **Intensity** tab of `FR_Impact_ScalableHit` (HAND).
- S02-03: `FM_ARPG` open in its editor with 4 rows (HAND, GFD).
- D02-01 (a designed diagram, not a screenshot): event > Feel Map > recipe > tracks > camera, screen, actor, audio, haptics, UI.

### Chapter 3. Installing FeelKit [Video 1] (`src/03_install.md`)
**Buyer question:** Will it run in my engine and project, and what does it change?

**Facts:**
- Engine: built and tested on UE 5.6 (`EngineVersion` 5.6.0). 5.7 and 5.8 are pending, and Fab requires the latest engine at first submission (Fab_Requirements 4.2.2.b). Fill this in after the ports.
- Platforms: the runtime modules allow Win64, Mac, Linux, Android and iOS, and FeelEditor allows Win64, Mac and Linux. Only Windows was built and tested (D-073). Say it plainly.
- Enabling FeelKit enables Niagara and EnhancedInput (the `Plugins` section of `FeelKit.uplugin`).
- Fab installs purchased plugins under `Engine/Plugins/Fab` (Fab_Requirements 4.3.6.3).
- **GAS add-on:**
  - Copy `<engine>/Engine/Plugins/Fab/<FeelKit folder>/Extras/FeelKitGAS` into `<project>/Plugins/FeelKitGAS`, then build the project (description in `FeelKitGAS.uplugin`).
  - It needs FeelKit and GameplayAbilities, and is enabled by default once in the project.
  - Without the copy, GAS stays off (gas_proof, log 2026-09-23).
  - It compiles with the project, so the project must be a C++ project or have Visual Studio. This is inferred from the add-on shipping without prebuilt binaries: BuildPlugin builds only the modules listed in FeelKit.uplugin (GAS research, option B).
  - The Fab folder name under `Engine/Plugins/Fab` is not verified.
- Blueprint-only projects: packaging needs Visual Studio with the C++ workload (requirement C-007). Not yet tested (TS-006, TS-007).
- Show plugin content: Content Browser **Settings** > **Show Plugin Content**, then **All** > **Plugins** > **FeelKit Content** (6B user tests A0, verified by you).

**Shots:**
- S03-01: the launcher or Fab install-to-engine step for FeelKit (HAND, after publishing).
- S03-02: Plugins window with FeelKit found and enabled, version 1.0.0 visible (HAND, CLEAN).
- S03-03: Content Browser Settings menu with Show Plugin Content ticked (HAND).
- S03-04: two Explorer windows, the engine Fab folder's `Extras/FeelKitGAS` and the project's `Plugins/FeelKitGAS` (HAND).
- S03-05: Plugins window showing "FeelKit GAS" enabled (HAND).

### Chapter 4. Quick start [Video 1] (`src/04_quickstart.md`)
**Buyer question:** Can I get a result in five minutes (requirement NF-020)?

**Facts:**
- **Tools** > **FeelKit Recipe Browser**. Hovering a tile for 0.25 s plays it once, and selecting it loops (`SFeelRecipeBrowser.cpp` HoverDelaySeconds; log Task 7).
- **Use** on a library recipe opens Save Asset As; the default name drops the `FR_<Feeling>_` prefix (D-040; `FeelLibrary.cpp`).
- Node **Play Feel** (Recipe, Target, Intensity 1) returns a handle (`FeelBlueprintLibrary.h`). **Make Feel Target From Actor**: on a character, actor effects go to the visible mesh and never the capsule (D-037).
- **Feel Switch:** Tab or controller View / Share, start card, ON/OFF badge, always starts ON, not saved (`FeelSwitch.h`/`.cpp`; D-057).

**Shots:**
- S04-01: Recipe Browser with Impact ticked and `FR_Impact_HeavyHit` selected (DIAG `BrowserShot`, then HAND for the selection).
- S04-02: the Save Asset As dialog after **Use** (HAND).
- S04-03: Blueprint graph with key event > Play Feel > Make Feel Target From Actor (self) (HAND).
- S04-04: PIE frame at the moment of the hit (HAND).
- S04-05: Feel Switch start card and badge FEEL: ON (DIAG `FeelSwitchInPlay`).
- S04-06: badge FEEL: OFF (same).

### Chapter 5. The recipe editor [Video 2] (`src/05_editor.md`)
**Buyer question:** How do I author and tune a recipe?

**Facts** (all labels from `SFeelTimeline.cpp`, `FeelRecipeEditorToolkit.cpp` and `SFeelPreviewViewport.cpp`):
- Create: right-click > **FeelKit** > **Feel Recipe** (6B user tests A3).
- Tabs: Preview, Timeline, Details, Intensity.
- Toolbar: Play/Pause, Stop, Release (only for sustained recipes), Loop, Snap, frame rate (default 60, per user `UFeelEditorSettings`), Comfort, Recent Plays, Play in PIE, Capture GIF, Fit.
- **+ Track** lists every concrete step class, including Blueprint steps.
- Editing: Shift disables snapping. F fits. Ctrl+Wheel zooms. Shift+Wheel or middle-drag scrolls. Space plays or pauses.
- Track menu: Copy (Ctrl+C), Copy All Tracks, Paste at the playhead (Ctrl+V), Duplicate (Ctrl+D), Delete, Move Up/Down, Mute, Solo.
- Curve lane: double-click adds or deletes a key; right-click a key for Constant, Linear or Smooth.
- Curves in Details show a preview and **Edit...** (D-033).
- Preview Mesh: static or skeletal, editor-only, a cube when empty (`FeelRecipe.h`).
- Preview parameters and **Defaults**; values are not saved.
- Waveforms on Play Sound tracks. Onset snapping; Shift disables it.
- **Create Force Feedback Track From Sound** / **Create Shake Track From Sound** need a Sound Wave, or a Sound Cue that plays one. MetaSounds cannot be read (tooltips in `SFeelTimeline.cpp`; I-018).
- **Play in PIE** plays on the player pawn (its skeletal mesh when it has one). Edits apply to the next play during PIE (PV-009).
- Validation messages: `FeelRecipe.cpp` lines 124 to 292 and the step `ValidateStep` texts. List them in a table.
- Read-only library banner: "Library recipe (read-only). Copy it to your project to edit." with **Copy to Project**.

**Examples:** `FR_Power_ChargeUp` (sustain 0.35 to 0.95), `FR_Impact_ScalableHit`.

**Shots:**
- S05-01: context menu FeelKit > Feel Recipe (HAND).
- S05-02: the whole editor on the `HeavyHit` copy with numbered areas (HAND).
- S05-03: toolbar close-up (crop of S05-02).
- S05-04: the + Track picker open (HAND).
- S05-05: selected track with the curve lane and three keys (DIAG `RecipeEditorShot` Selected picture, or HAND).
- S05-06: key right-click menu (HAND).
- S05-07: Details of a selected track with the categories Track, Parameter Mappings, Randomness, Comfort, Conditions expanded (HAND).
- S05-08: preview parameter slider on `FR_Impact_ScalableHit` (HAND).
- S05-09: sustain region on the ruler and **Release** enabled while looping (HAND).
- S05-10: Play Sound waveform, with the track menu showing Create Shake Track From Sound (HAND).
- S05-11: Comfort dropdown open (HAND).
- S05-12: Intensity tab (HAND).
- S05-13: library recipe with the read-only banner (DIAG `RecipeEditorShot`, `RecipeEditor.png`).
- S05-14: Message Log with a validation warning after save (HAND).

### Chapter 6. Playing recipes in your game [Video 3] (`src/06_triggering.md`)
**User question:** How do I fire recipes from gameplay?

**Facts** (`FeelBlueprintLibrary.h`, `FeelPlayAndWaitAction.h`, `FeelMap.h`, `FeelAnimNotifies.h`, `FeelTriggerComponent.h`, `FeelInputComponent.h`, `FeelGameplayCue.h`, `FeelSwitch.h`, `FeelSubsystem.h`):

- **Targets:** Make Feel Target From Actor, From Component, At Location, From Local Player Camera, From Widget. With no target, camera and screen effects go to the first local player. A target owned by a remote player gets no camera or screen effects on this machine (D-032, `FeelTypes.h`).
- **Play Feel** fails and returns an invalid handle on cooldown, Max Concurrent, `feel.Enabled 0`, a dedicated server, or a null recipe (`FeelSubsystem.h`).
- **Play Feel With Context:** Parameters, Instigator, Direction, Location, Normal, Context Tags. With an empty context it behaves exactly like Play Feel.
- **Handles:**
  - **Stop Feel** (Blend Out, default on; uses Blend Out Time 0.2 s).
  - **Stop All Feel** (optional Target).
  - **Release Feel**.
  - **Set Feel Parameter**: its name pin is a dropdown that follows the handle to the recipe (D-029; log "Doc note"). Give this its own screenshot.
  - **Is Feel Playing** and **Is Valid (Feel Handle)**.
- **Play Feel And Wait:** On Finished, and On Canceled (stopped early, target destroyed, or could not start).
- **Feel Maps:**
  - Row fields: Event, Required Tags, Recipe, Intensity Scale 1, Priority 0.
  - Matching: a more specific event wins, then more required tags, then higher Priority, then the earlier row.
  - Maps on a Feel Trigger component on the target are checked before the project maps.
  - Starter events: `Feel.Event.Hit.Landed`, `Feel.Event.Hit.Received`, `Feel.Event.Hurt`, `Feel.Event.Death` (`FeelTags.cpp`).
  - A miss logs a warning once per event (I-036).
- **Anim notifies:**
  - **Play Feel**, **Send Feel Event**, **Set Feel Value** (Values map, Mode Set/Add, Scope Owning Actor/Global) and **Play Feel (Window)** (Let Non Sustained Recipe Finish).
  - Shared settings: Target Mesh or Owning Actor, Intensity, Parameters, Context Tags.
  - Notifies also play in the animation editor preview.
  - Adding one: Notifies track > Add Notify (6B user tests B).
- **Feel Trigger:**
  - Events: Take Any Damage, Landed, Component Hit, Begin Overlap, End Overlap, Jumped, Air Jumped, Launched, each with its event value.
  - Entry fields: Recipe or Feel Event, Value Parameter, Intensity 1, Target Skeletal Mesh (on), Other Actor As Instigator (on), Play On (Owner or Other Actor), Only For Players (off), Context Tags.
  - **Fire Event** to fire entries by hand.
  - The component ticks only when jump or launch entries exist.
- **Feel Input** (Enhanced Input):
  - Bindings: Action, Play On (Started, Triggered, Completed, Canceled), Recipe or Event, Intensity, Scale Intensity By Value, Value Parameter, End When Input Ends, Once Per Press.
  - Plays target the owner. **Handle Input** simulates input.
- **GAS** (add-on only):
  - **Feel Gameplay Cue Notify** plays on Executed and on Added (OnActive).
  - **Feel Gameplay Cue Notify (Actor)** plays while the cue is active, with Stop On Remove.
  - Settings: Recipe or Event (empty = the cue tag), Scale Intensity By Magnitude, Intensity, Raw Magnitude Parameter, Normalized Magnitude Parameter, Effect Causer As Instigator.
  - The Blueprint cue nodes need a **Make GameplayCueParameters** node on the Parameters pin (I-031).
- **C++:** `UFeelSubsystem::Get(WorldContext)->PlayFeel(...)`. Native delegates `OnFeelStarted` and `OnFeelFinished`. Module dependency `FeelCore`, as FeelDemoFP.Build.cs does (log Shooter record).
- **Feel Switch:**
  - Switch Keys (Tab, Gamepad_Special_Left), Start Enabled, Show Built-in Display, Show Start Card, Start Card Title "FeelKit", Start Card Text, Start Card Seconds 8, Controls list, On Feel Switched, Switch.
  - One per level; a second one warns and does nothing.
  - It is process-wide, so PIE windows switch together (6B design).
  - Nodes: **Set Feel Enabled**, **Toggle Feel**, **Is Feel Enabled**.

**Examples:**
- Platformer Feel Trigger entries (Jumped, Air Jumped JumpNumber, Launched LaunchSpeed, Landed LandSpeed).
- `BP_JumpPad`: Begin Overlap, Play On Other Actor, Only For Players.
- `FM_ARPG` rows.
- `AM_ComboAttack` Swing values 0.35, 0.65, 1.

**Shots:**
- S06-01: target maker nodes (HAND).
- S06-02: Play Feel With Context with Make FeelPlayContext expanded (HAND).
- S06-03: Set Feel Parameter dropdown open, listing the handle's recipe parameters (HAND).
- S06-04: Play Feel And Wait pins (HAND).
- S06-05: `FM_ARPG` rows (HAND, GFD).
- S06-06: Project Settings > Plugins > FeelKit > Events > Feel Maps (HAND).
- S06-07: `AM_ComboAttack` Notifies track with Set Feel Value and Play Feel markers (HAND, GFD).
- S06-08: Feel Trigger Details on the platforming character with 4 entries (HAND, GFD).
- S06-09: Feel Input Details (HAND).
- S06-10: Feel Gameplay Cue Notify Blueprint defaults (HAND).
- S06-11: Feel Switch Details with Controls (HAND, GFD `Lvl_Combat`).

### Chapter 7. Parameters, context and variation (`src/07_variation.md`)
**Facts:**
- **Parameter:** Name, Default Value 0, Min Value 0, Max Value 1, Accumulator, Description.
  - Mappings read min as 0 and max as 1. A mapping curve that is empty behaves as a straight 0 to 1 line.
  - `Distance` is filled in with the distance to the nearest local camera when not passed (`FeelParameters.h`).
- **Accumulator:** Name, Max Value 1, Decay Per Second 1, Decay Delay 0 (`FeelSettings.h`).
  - Nodes: **Add To Feel Accumulator**, **Set Feel Accumulator**, **Get Feel Accumulator**, with name dropdowns.
  - Read order: the target's value, then the instigator's, then the global value (`FeelSubsystem.cpp` `RefreshAccumulatorParameters`).
- **Sustain:** looping until Release Feel, the end of an anim notify window, or Stop Feel.
- **Applies To = Instigator:** the track is skipped when there is no instigator.
- **Camera Punch Direction Source:** Step Settings, Play Direction, Away From Play Location, Toward Play Location.
- **Conditions:** decided once per play. Chance is re-rolled each play but stays stable while scrubbing. Platforms use Unreal ini platform names (D-019, A-004).
- **Random Choice:** picked per play by weight. **Play Recipe:** nesting depth up to 4.

**Examples:**
- `FR_SHOOT_Rifle` Heat reads ShotHeat (max 8, decay 10/s after 0.12 s; log Shooter record).
- `FR_Reward_ComboStep` reads Combo (10, 1, 1.5; log A6).
- Instigator: `FR_SHOOT_Hit`, `FR_SHOOT_Kill`, `FR_ARPG_Parry`.
- No shipped recipe uses Random Choice, Play Recipe or Conditions. Use examples the reader builds, and do not claim they ship.

**Shots** (as taken, 2026-09-25, `DiagFeel.ManualShotsVariation`; table in "Lists read by the build script"):
- S07-01: the Damage slider, parameter and Parameter Mappings of a copy of `FR_Impact_ScalableHit`.
- S07-02: Accumulators in Project Settings (Combo).
- S07-03: Random Choice options.

### Chapter 8. Comfort [Video 4] (`src/08_comfort.md`)
**Buyer question:** How do I respect players' comfort, and what do I have to build myself?

**Facts** (`FeelComfortTypes.h`, `FeelSettings.cpp`, `FeelComfortSubsystem.h`, `FeelComfortStorage.h`):
- **Groups:** Master, Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion, Haptics.
- **Default channel mapping:**
  - Shake to Camera Shake; Motion to Camera Motion.
  - Screen.Flash and Actor.Light to Flashes.
  - Screen.Distortion and Screen.Color to Screen Distortion.
  - Time.* to Hitstop and Slow-mo; Haptics to Haptics.
  - Unmapped, so Master only: Screen.Fade, Actor.Transform, Actor.Material, Audio, Audio.Mix, UI, Spawn, Meta.*.
  - The most specific tag wins.
- **Presets:**
  - Reduced Motion: Shake 0.25, Motion 0.25, Distortion 0.5, roll off, 40°/s FOV cap.
  - Reduced Flashing: Flashes 0.2, 1 flash per second, Suppress.
  - No Haptics: Haptics 0.
  - Custom presets are `UFeelComfortPreset` data assets (Display Name, Scales).
- **Nodes:**
  - **Get Feel Comfort** (from a player controller).
  - **Get Comfort Scales**, **Set Comfort Scales**, **Set Master Comfort Scale**, **Set Comfort Group Scale**.
  - **Apply Comfort Preset**, **Apply Custom Comfort Preset**.
  - **Save Comfort Settings**, **Load Comfort Settings**, **Set Comfort Storage**, **Get Effective Force Feedback Scale**.
- **Essential tracks:** at scale 0 a substitute plays. Otherwise the scale never drops below Essential Floor, except on Camera Shake and Camera Motion, where 0 always means 0 (D-012). Non-essential tracks at 0 are skipped.
- **Flash limiter:** on by default, 3 per second, Soften to 0.3. It counts tracks on channels mapped to Flashes. It is a helper, not a certification.
- **Engine comfort:**
  - Apply Comfort To Engine Camera Shakes (on).
  - Apply Comfort To Engine Force Feedback (on). It multiplies the game's own scale and restores it, and a warning says when vibration is silenced (D-046).
- **Storage:** Auto Save Comfort (on). Slot `FeelComfort_<local player index>` (`FeelComfortSaveGame.cpp`), or your own class implementing Feel Comfort Storage (Load Comfort Scales / Save Comfort Scales).
- **No sample comfort widget ships.** 8.9 shows how to build one with the nodes above; the widget is not a FeelKit asset (see discrepancy 8).
- **Comfort Audit:** menu path, checks and wording from `FeelComfortAudit.cpp` (3 flashes per second, saturated red at opacity above 0.5, sustain loops, channel outside its step's group, essential flash floors, project settings).

**Examples:** `FR_Dread_JumpScare`, `FR_Danger_DamageTaken` and `FR_Danger_DirectionalDamage` all have an essential Screen Flash with a Vignette Pulse substitute (floors 0.3, 0.35, 0.35).

**Shots:**
- S08-01: Project Settings Comfort and Presets expanded (HAND).
- S08-02: Blueprint comfort nodes (HAND).
- S08-03: the Comfort category of the jump scare's flash track (HAND).
- S08-04: "Substitute plays (comfort)" label with Reduced Flashing preview (HAND).
- S08-05: "Flash softened by the flash limiter" label (HAND).
- S08-06: Debugger Player 0 comfort and Controller vibration rows (DIAG `DebuggerShot`).
- S08-07: Message Log "FeelKit Comfort Audit" page (HAND).

### Chapter 9. Library and browser (`src/09_library.md`)
**Facts:**
- Browser areas: Source (Library, Project, both), Feeling with counts, Genre ("any of them"), Affects ("all of them") (D-047).
- Search covers names, descriptions and parameter names. Mute, and a comfort menu.
- **Use**, **Open**, **Show in Content Browser**. Footer "N items".
- **Recipe from Template...** and **Browse Recipes...** on folder right-click.
- Read-only rules and **Allow Library Editing** (Editor Preferences > Plugins > FeelKit) (D-040, `FeelEditorSettings.h`).
- Tiles and tooltip fields: Description, Feeling, Genres, Channels, Tracks, Length, Sustained (`FeelRecipe.cpp` metadata).
- Library table: the 38 names with their descriptions (from `Library/*.json`; copy the descriptions, do not reword).
- Shared parameter names: Damage, FallSpeed, Health, Fear, Charge, the Combo accumulator, Distance (D-042).
- Combo must be added to Accumulators by the buyer. Library sounds and materials live in `/FeelKit/Samples`.
- Custom tags go under `Feel.Feeling` and `Feel.Genre`.

**Shots:**
- S09-01: Recipe Browser full (DIAG `BrowserShot`).
- S09-02: the Recipe from Template picker (HAND).
- S09-03: Library/Impact tiles in the Content Browser (HAND).
- S09-04: a tile tooltip (HAND).

### Chapter 10. Debugging (`src/10_debugging.md`)
**Facts:**
- Debugger columns: Name, Target, Time, Value, Details, Replay. Folders: Playing, Accumulators, Player N comfort, Controller vibration, Recent plays. **Clear Recent Plays**; double-click opens and replays (`SFeelDebugger.cpp`).
- Captures: the last 64 plays, kept after PIE stops, not in Shipping builds (`FeelPlayCapture.h` MaxCaptures; `FeelSubsystem.cpp` `!UE_BUILD_SHIPPING`).
- Decision labels: list from `SFeelTimeline.cpp` lines 898 to 961.
- `showdebug feel` output lines (`FeelSubsystem.cpp` 478 to 537). `stat Feel`: Feel Tick and Feel Instances.
- Log messages: `LogFeel` warnings (unmatched event, undefined accumulator, invalid target, silenced vibration, second Feel Switch) and `LogFeelSteps` (Blueprint Event).
- Capture GIF: 20 fps, up to 6 s, two 480 px wide halves, saved to `Saved/FeelKit/Captures` (`SFeelPreviewViewport.cpp`).

**Shots:**
- S10-01: Debugger in PIE (DIAG `DebuggerShot`).
- S10-02: Recent Plays menu and the "Replaying a Recorded Play" state (HAND).
- S10-03: decision labels (HAND).
- S10-04: `showdebug feel` HUD (HAND).
- S10-05: `stat Feel` (HAND).
- S10-06: a still of a captured GIF, plus the notification (HAND).

### Chapter 11. Multiplayer (`src/11_multiplayer.md`)
**Facts** (`FeelReplicationComponent.h`, D-028, D-032, D-035):
- Only the request travels. Each machine evaluates with its own comfort.
- **Play Feel Networked** and **Send Feel Event Networked**. Modes: Everyone, Owner Only, Skip Owner. Relevancy Distance 0 = unlimited.
- An owning client plays at once and the server forwards the play.
- RPCs are unreliable. Widget targets are not sent.
- A dedicated server never plays.
- Global hitstop and slow-mo slow only the target actor in networked worlds unless **Allow Global Time Dilation In Multiplayer** is on.

**Shots:**
- S11-01: Feel Replication and a Play Feel Networked node with the Mode menu (HAND).
- S11-02: the Playback settings (HAND).

### Chapter 12. Performance (`src/12_performance.md`)
**Facts:**
- No tick while nothing plays (RT-002).
- Plays use real time.
- Nothing is spawned per play except what steps spawn (sounds, decals, particles).
- `stat Feel`.
- Library tiles are drawn without a 3D scene (6A design section 4).

**Do not publish numbers:** instance pooling (RT-004) and the 50-instance budget (NF-002) were never measured.

**Shot:** S12-01, reuse S10-05.

### Chapter 13. Demo kits (`src/13_demos.md`, To decide)
**Facts** per demo: what it shows, the recipes it uses, and its hooks (`FeelKit_1_Product.md` section 6; details in `Archive/FeelKit_Phase6B_Demos_Design.md`). For the Action/RPG hit, use the D-072 values (spring 9 Hz 5.5°, rattle 24 Hz 3°, no full-screen flash on heavy strikes); the design text is stale here. Controls lists come from D-071.

**Dependencies to state:**
- The Action/RPG recipes need the Combat template's `NS_Damage` (see discrepancy 22) and the accumulators Swing, Charged, ChargeLevel and Combo.
- Shooter needs ShotHeat.
- Shooter and Horror need C++ edits to the template.
- The guard enemy is a copy of an Epic Blueprint and cannot ship (launch readiness 2.2).

**Material to draft from:** the archived Action/RPG guide (6B user tests, sections A and B). It predates the guards and needs rewriting.

**Shots**, gameplay stills with FEEL: ON and the controls panel (HAND per level):
- S13-01, S13-02: Action/RPG (combat hit; parry from DIAG `ARPGGuardPlaythrough`).
- S13-03, S13-04: Platformer.
- S13-05, S13-06: Shooter (FP).
- S13-07, S13-08: Horror (FP).

### Chapter 14. Troubleshooting and questions (`src/14_faq.md`)
**Entries:**
1. **The controller does not rumble.** On Windows, Unreal sends vibration only to XInput pads. PlayStation and other pads need Steam Input or a translation tool (CLAUDE.md Platform notes; I-024, I-029). `showdebug forcefeedback` lists per-effect values before the controller's scale (I-032). The FeelKit Debugger's "Controller vibration" row shows what really reaches the pad.
2. **A Feel Event plays nothing.** Read the log warning (I-036).
3. **An accumulator is not defined.**
4. **A post-process material shows at full strength.** Add the Weight parameter, and use a Constant4Vector in the Lerp (D-030, I-025).
5. **A decal is invisible.** Connect nodes to the decal material instead of typing values on the pins, and check Surface Search Distance (I-022, D-036).
6. **An effect moved the character or changed its collision.** It no longer does (D-037).
7. **The Blueprint Event step does nothing.** The event must take no inputs or a single float.
8. **Hitstop in multiplayer.**
9. **Library recipes cannot be edited.**
10. **Packaging a Blueprint-only project** needs Visual Studio. The exact message is to be verified.
11. **GAS cue nodes fail to compile** without Make GameplayCueParameters (I-031).
12. **Two camera recipes pointing different ways:** Strongest Wins switches between them in one frame (I-046 product note).
13. **UI reactions:** move a panel inside a button, never the button itself (I-044).
14. **Mouse capture:** a menu that changes the input mode should restore it before opening a level (I-045).

**Shots:**
- S14-01: `showdebug feel` "Controller vibration" line next to `showdebug forcefeedback` (HAND).
- S14-02: Output Log unmatched-event warning (HAND).

### Chapter 15. Lite and Pro (`src/15_editions.md`, To decide)
Write nothing beyond what is decided. The project records so far:
- Free Lite includes the full comfort layer; Pro has everything; there is no studio tier (D-015).
- The requirements (section 13) describe Lite as a FeelCore subset (steps tagged L: Procedural Shake, Camera Punch, FOV Kick, Global Hitstop, Screen Flash, Vignette Pulse, Scale Punch, Play Sound), the comfort layer, a basic sample widget and a few recipes, with no editor module.
- The Feel Switch is in both (D-057).
- The library subset is undecided (D-042).
- One listing or two is open (OD-006).

### Part Two
- **16 Step reference:** generated tables (data in section 4 of this plan). One paragraph per step for behavior. Shot S16-01: + Track picker listing all steps. Shot S16-02: a "No preview" bar.
- **17 Nodes and components:** every node listed in section 4 of this plan, with pins and tooltips from the headers. Shot S17-01: the node search showing the Feel category.
- **18 Settings:** every `UFeelSettings` and `UFeelEditorSettings` property with its default (section 4). Shots S18-01 to S18-03: the Project Settings page; S18-04: Editor Preferences.
- **19 Console:** `feel.Enabled` (default 1: 0 stops all plays and blocks new ones), `feel.GlobalScale` (default 1: multiplies every recipe's intensity; 0 silences without stopping; there is no Blueprint node, use Execute Console Command), `stat Feel`, `showdebug feel` (`FeelSubsystem.cpp` 46 to 56).
- **20 Custom steps:**
  - Blueprint: Blueprint Class > parent **Feel Step**. The overridable functions are **On Start**, **On Stop**, **Get Default Channel** and **Supports Preview**. Continuous output (`Evaluate`) is C++ only (`FeelStep.h`). See discrepancy 4.
  - C++: subclass `UFeelStep`, override `Evaluate(const FFeelStepEvalContext&, IFeelOutputSink&) const`, and send output through the sink functions (`FeelOutputSink.h`). Keep `Evaluate` pure; return false from `SupportsScrub` when a step keeps state; use `ValidateStep` for checks.
  - Shots S20-01 (the parent class picker) and S20-02 (the overrides list), both HAND.
- **21 JSON and scripting:**
  - Right-click a recipe: **Export to JSON...** (several recipes at once) and **Import from JSON...** (one recipe). Undoable. Missing asset references are left empty and reported (D-045).
  - File header: `"format": "FeelKitRecipe"` and `"schemaVersion": 2`.
  - Editor scripting (category FeelKit|Editor Scripting): `ImportRecipeFromJsonFile`, `CreateRecipeFromJsonFile`, `AddFeelMapToProjectSettings`, `SetAccumulatorInProjectSettings` (`FeelEditorScripting.h`).
  - Shot S21-01: the context menu.
- **22 Credits:** from `Credits.md`, corrected (discrepancy 14). Fab's license terms as you decide.
- **23 Version history:** 1.0.0.

---

## 4. Reference data for the writer

All 37 steps with their real class names, display names, default channel, comfort group under the default mapping, whether the editor preview shows them, and key properties with defaults. Sources: `Source/FeelCore/Public/Steps/*.h`, the constructors and `GetDefaultChannel_Implementation` in `Private/Steps/*.cpp`, and `FeelNiagara/Public/FeelStep_SpawnParticle.h`.

**Shaped steps.** Steps built on `UFeelStep_ShapedMotion` share these settings:
- Shape: Spring by default. Spring = springy overshoot that settles; Kick = fast rise then an eased return; Smooth = smooth rise and fall.
- Frequency 5 Hz and Damping 7 (Spring only).
- Attack Fraction 0.15 (Kick only).
- Repeats 1 (up to 32).
- New tracks start with a flat intensity curve.

"Preview" means the step's `SupportsPreview` value. Items marked "verify" default to true but were never checked visually.

**Camera**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Procedural Shake (`UFeelStep_ProceduralShake`) | Camera.Shake | Camera Shake | yes | Mode Perlin (Perlin, Sine, Directional); Frequency 12 Hz; Location Amplitude (0, 3, 3); Direction (0, 0, 1) and Directional Amplitude 8 cm for Directional; Rotation Amplitude pitch 1, yaw 1, roll 0.5; Field Of View Amplitude 0 |
| Camera Punch (`UFeelStep_CameraPunch`, shaped, Spring) | Camera.Motion | Camera Motion | yes | Location Punch (-10, 0, -4); Rotation Punch pitch -2.5; Direction Source Step Settings; Direction Jitter 0° |
| FOV Kick (`UFeelStep_FOVKick`, shaped, Kick) | Camera.Motion | Camera Motion | yes | Field Of View Kick 8° (-60 to 60) |
| Camera Roll (`UFeelStep_CameraRoll`, shaped, Spring) | Camera.Motion | Camera Motion, and removed when a player turns roll off | yes | Roll Degrees 4; Random Direction on |
| Camera Zoom (`UFeelStep_CameraZoom`) | Camera.Motion | Camera Motion | yes | Field Of View Change -10; Ease In Fraction 0.25; Ease Out Fraction 0.35 |
| Look-at Nudge (`UFeelStep_LookAtNudge`, shaped, Kick) | Camera.Motion | Camera Motion | yes | Turn Fraction 0.2; Max Turn Degrees 6; needs a play Location or Direction |

**Time**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Global Hitstop (`UFeelStep_GlobalHitstop`) | Time.Hitstop | Hitstop and Slow-mo | no | Time Dilation 0.05; Priority 0 |
| Actor Hitstop (`UFeelStep_ActorHitstop`) | Time.Hitstop | Hitstop and Slow-mo | no | Same as Global Hitstop; slows the target's custom time dilation |
| Slow-mo Ramp (`UFeelStep_SlowMoRamp`) | Time.SlowMo | Hitstop and Slow-mo | no | Time Dilation 0.3; Ramp In Time 0.15 s; Ramp Out Time 0.3 s; Priority 0 |

**Screen**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Screen Flash (`UFeelStep_ScreenFlash`) | Screen.Flash | Flashes | yes | Color white; Max Opacity 0.6 |
| Vignette Pulse (shaped, Smooth) | Screen.Distortion | Screen Distortion | yes | Vignette Intensity 1 |
| Chromatic Aberration (shaped, Kick) | Screen.Distortion | Screen Distortion | yes | Fringe Intensity 3 (0 to 5) |
| Desaturate (shaped, Smooth) | Screen.Color | Screen Distortion | yes | Amount 0.8 |
| Color Tint (shaped, Smooth) | Screen.Color | Screen Distortion | yes | Tint Color (1, 0.4, 0.35); Strength 1 |
| Screen Fade (`UFeelStep_ScreenFade`) | Screen.Fade | none, Master only | yes | Fade Color black; Max Opacity 1; Fade In Fraction 0.3; Fade Out Fraction 0.3; drawn under flashes |
| Post Process Material Pulse (shaped, Smooth) | Screen.Distortion | Screen Distortion | yes | Material (Post Process domain); Max Weight 1; Weight Parameter "Weight"; sample material `M_FK_PP_Pulse` |

**Actor**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Scale Punch (`UFeelStep_ScalePunch`) | Actor.Transform | none | yes | Amount (0.3, 0.3, 0.3); Bounces 1 |
| Squash and Stretch (shaped, Spring) | Actor.Transform | none | yes | Axis Z; Amount 0.3 (-0.9 to 3); Preserve Volume on |
| Material Parameter Pulse (`UFeelStep_MaterialPulse`, shaped, Smooth) | Actor.Material | none | yes | One parameter: Parameter Name "Color", Is Color on, Color (1, 0.15, 0.1), Scalar Amount 1; Route Material Instance or Custom Primitive Data |
| Hit Flash (`UFeelStep_HitFlash`, shaped, Smooth) | Actor.Material | none | yes | Flash Material (needs FlashColor and FlashAmount parameters; sample `M_FK_HitFlash`); Color white; drawn through the overlay material slot |
| Mesh Wobble (`UFeelStep_MeshWobble`) | Actor.Transform | none | yes | Tilt Amplitude roll 8; Move Amplitude 0; Frequency 10 Hz; Decay 1.5; Noise off |
| Light Flash (`UFeelStep_LightFlash`, shaped, Kick) | Actor.Light | Flashes | no | Intensity Change 2 (-1 to 50); Color white; Color Strength 0; Flicker off; Flicker Rate 18 Hz |

**Audio**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Play Sound (`UFeelStep_PlaySound`) | Audio | none | yes, audible | Placement 2D (2D, Attached to Target, At Target Location); Attach Socket Name; Volume Multiplier 1; Scale Volume With Intensity on; Volume Variation 0.05; Pitch Multiplier 1; Pitch Variation 0.05; Sound Start Time 0; Stop At Track End off; Stop When Recipe Stops on; Fade Out Time 0.15 s; Attenuation Settings; Concurrency Settings; can be an instant track |
| Sound Class Duck (`UFeelStep_SoundClassDuck`) | Audio.Mix | none | verify | Sound Class (empty = the project default sound class); Attack Fraction 0.1; Release Fraction 0.4; Volume Reduction 0.6 |
| Pitch Bend (`UFeelStep_PitchBend`) | Audio.Mix | none | verify | Same base settings; Pitch Change -0.3 |
| Low-pass Sweep (`UFeelStep_LowPassSweep`) | Audio.Mix | none | verify | Same base settings; Cutoff Frequency 800 Hz; the cutoff moves in 7 preset steps (log chunk 2) |

**Haptics**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Force Feedback Curve (shaped, Kick) | Haptics | Haptics | no | Left Large 1; Left Small 0.4; Right Large 1; Right Small 0.4; Ripple Frequency 0; Ripple Depth 0.5 |
| Haptic Pattern (`UFeelStep_HapticPattern`) | Haptics | Haptics (can switch it off, cannot scale it) | no | Effect (Force Feedback Effect asset); Looping off; Stop With Track on |

**UI.** The three widget steps need a widget target.

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Widget Punch (shaped, Spring) | UI | none | no | Scale Change (0.2, 0.2); Translation (0, 0); Angle Degrees 0 |
| Widget Shake (`UFeelStep_WidgetShake`) | UI | none | no | Amplitude (8, 3); Angle Amplitude 0; Frequency 30 Hz; Decay 1 |
| Widget Flash (shaped, Kick) | UI | none | no | Color (1, 0.2, 0.2); Strength 1 |
| Number Pop (`UFeelStep_NumberPop`) | UI | none | no | Value Parameter; Decimals 0; Text; Prefix; Suffix; Color white; Font Size 28; Scale With Intensity on; Location Play Location or Target; World Offset (0, 0, 60); Rise Distance 60; Spread 24; Pop Scale 1.6; Default Lifetime 1 s |

**Spawn**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Spawn Decal (`UFeelStep_SpawnDecal`) | Spawn | none | verify | Decal Material; Decal Size (16, 48, 48); Location Play Location or Target; Lifetime 5 s; Fade Out Time 1 s; Random Rotation on; Find Surface on; Surface Search Distance 500 cm |
| Spawn Particle (`UFeelStep_SpawnParticle`, module FeelNiagara) | Spawn | none | yes | System; Location; Attach To Target off; Attach Socket Name; Orient To Normal on; Scale (1, 1, 1); Parameters (User Parameter, Recipe Parameter); Deactivate With Track on |

**Meta**

| Display name (class) | Channel | Comfort group | Preview | Key properties and defaults |
|---|---|---|---|---|
| Play Recipe (`UFeelStep_Recipe`) | Meta.Recipe | the inner recipe's tracks keep their own channels | like the inner recipe | Recipe; Intensity Scale 1; nesting up to depth 4 |
| Random Choice (`UFeelStep_RandomChoice`) | the first option's channel, else Meta.Recipe | per option | per option | Options (Step, Weight 1) |
| Blueprint Event (`UFeelStep_BlueprintEvent`) | Meta.Event | none | no | Receiver Target Actor (Player Pawn, Player Controller, Level Blueprint); Start Event Name; Stop Event Name; Call Stop Event When Interrupted on |

**Blueprint nodes** (display names derived from the function names; check the capitalisation against the editor when capturing):

| Group | Nodes |
|---|---|
| Feel | Play Feel; Play Feel With Context; Send Feel Event; Release Feel; Set Feel Parameter; Stop Feel; Stop All Feel; Is Feel Playing; Is Valid (Feel Handle); Make Feel Target From Actor, From Component, At Location, From Local Player Camera, From Widget; Play Feel And Wait; Get Duration (on a recipe) |
| Feel\|Accumulators | Add To Feel Accumulator; Set Feel Accumulator; Get Feel Accumulator |
| Feel\|Comfort | Get Feel Comfort, plus the comfort nodes listed in chapter 8 |
| Feel\|Switch | Set Feel Enabled; Toggle Feel; Is Feel Enabled; Switch (on the Feel Switch actor) |
| Feel\|Network | Play Feel Networked; Send Feel Event Networked |
| Components | Fire Event (Feel Trigger); Handle Input (Feel Input) |

**Settings, Project Settings > Plugins > FeelKit** (`FeelSettings.h`):

| Area | Settings and defaults |
|---|---|
| Camera | Camera Arbitration Strongest Wins; Max Camera Location Offset 30 cm; Max Camera Rotation Offset 8°; Max Field Of View Offset 15° (the three caps apply in Additive Capped only) |
| Playback | Blend Out Time 0.2 s; Allow Global Time Dilation In Multiplayer off; Accumulators |
| Events | Feel Maps |
| Comfort | Default Comfort Scales; Channel Comfort Groups; Reduced Motion Preset; Reduced Flashing Preset; No Haptics Preset; Apply Comfort To Engine Camera Shakes on; Apply Comfort To Engine Force Feedback on |
| Comfort Storage | Auto Save Comfort on; Comfort Save Slot Prefix "FeelComfort"; Comfort Storage Class (empty) |

**Editor Preferences > Plugins > FeelKit** (`FeelEditorSettings.h`): Snap To Frames on; Snap Frame Rate 60; Allow Library Editing off.

---

## 5. Discrepancies (not resolved here)

1. **Stale status in the log and CLAUDE.md.**
   - The log's top status still says the GAS choice waits until launch. D-076 decided it.
   - CLAUDE.md still says "Documentation may follow the release (D-051)", which D-077 and Fab rule 4.3.8 replace.
   - CLAUDE.md and log section 7 call I-029 a go-live blocker, while the I-029 row says fixed (D-046). What remains is the platform limit.
2. **Optional modules.** Requirement C-003 asks for Niagara and Enhanced Input modules that compile only when those plugins are enabled. `FeelKit.uplugin` enables both plugins outright. Only GAS is optional, as an add-on.
3. **Step lifecycle.** Requirements 4.3 list `OnUpdate(Context, Alpha, Intensity)`. The code has `Evaluate` in C++ only. So Blueprint custom steps can only act at start and stop, which limits the custom-step guide (DOC-004).
4. **Renamed or replaced steps.**
   - ST-014 Radial Blur is Post Process Material Pulse with your own material.
   - ST-023 Submix Duck is Sound Class Duck.
   - ST-028 Adaptive Trigger and CMF-042 were not built.
   - The Haptic Pattern tooltip mentions trigger resistance; this was never tested on hardware.
5. **Debugger.** TL-003 asks for a Gameplay Debugger category. What was built is `showdebug feel`.
6. **Recipe fields.** Requirements 4.1 still list Category and a runtime Description; D-044 removed Category and moved Description into editor-only Library data.
7. **Requirements never recorded as built:** RT-004 pooling, NF-002 measured numbers, DEL-004 and TS-008 split-screen, CT-003, CT-006 (launch readiness 2.5).
8. **No sample comfort widget.** CMF-050 and CMF-051 (the widget, basic version planned for Lite) were not built. The brief asks to document it "if it exists"; it does not.
9. **Global scaling.** The brief lists it as a feature. It exists only as the console variable `feel.GlobalScale`; there is no Blueprint node (`FeelSubsystem.cpp` 52 to 56, 1258).
10. **Settings path in messages.** Tooltips and log messages say "Project Settings > FeelKit > ...". The real path is Project Settings > Plugins > FeelKit (`FeelSettings.h` `GetCategoryName`; verified by you in 6B test A2).
11. **Internal IDs in shipped text.** About 105 header doc comments carry requirement IDs, for example "(ST-001)" and "(CMF-005)". Class comments become tooltips in the editor, including the + Track picker. The console help text says "(RT-007)" and "(RT-008)".
12. **Mixed spelling in user-facing text.** Tooltips use "color". The Comfort Audit and recipe descriptions use "color". `Credits.md` uses both "License" and "license".
13. **Content lists and text disagree.**
    - `Credits.md` Materials lists only `M_FK_Decal_Scorch` and `M_FK_PP_Pulse`. It leaves out `M_FK_HitFlash` and `M_FK_GuardShield`.
    - Its intro says everything is CC0, then lists items covered by the FeelKit license.
    - `S_FK_Dread_Drone` ships, but no kit uses it any more (Horror rework).
    - `FR_SHOOT_Hit`'s JSON description says "freezes for a moment"; check it against its tracks if it is quoted.
14. **6A design versus code.**
    - The design has a separate `library` JSON object; D-044 and the code put those fields inside `recipe`.
    - The design names `UFeelEditorUserSettings`; the code has `UFeelEditorSettings`.
    - The design says values inside a filter group combine with OR; channels now combine with AND (D-047).
    - The design shades sustain regions on tiles; that shading was removed.
15. **6B design is stale.**
    - Section 4c still describes `AdditionalPluginDirectories`, replaced by a junction (I-039).
    - Section 4f describes the D-070 hit (Kick, 8.5°), while D-072 restored the approved spring and rattle.
    - Section 2's moments still include flashes that were removed from heavy strikes (I-046).
    - It names a "Show Built-in UI" option; the property is **Show Built-in Display**.
    - Section 1 says buyers import the recipes as JSON; they now ship as assets.
16. **6B user tests.** The "Earlier" section describes the D-070 camera ("jolts once and eases back"), which D-072 superseded.
17. **GIF size.** The D-026 row says 400 px per side. The code writes 480 px halves (I-017).
18. **An incomplete tooltip.** `FFeelStepEvalContext.Intensity` describes intensity as call x default x curve. It leaves out mappings, random intensity and comfort (`FeelEvaluator.h` `ComputeTrackIntensity`).
19. **Light Flash preview.** Light Flash returns `SupportsPreview` false, yet the chunk 2 log says the preview applies light delivery.
20. **Launch readiness report is partly stale.** Its section 2.4 metadata, copyright, platform-list and GAS items were done later the same night (log "Launch prep" and "Launch brief").
21. **Kits versus what ships.**
    - The 6B design says a kit is recipes, a Feel Map and a guide, with no template code.
    - The Shooter and Horror kits and the Action/RPG guards depend on template C++ edits and `BP_CombatGuardEnemy`, which is a copy of an Epic asset. None of these ship.
    - Only `FM_ARPG` ships as a Feel Map.
22. **Plugin content that points into template content.** `FR_ARPG_EnemyHurt`, `FR_ARPG_GuardBlock`, `FR_ARPG_GuardBreak` and `FR_ARPG_Parry` reference `/Game/Variant_Combat/VFX/NS_Damage` (seven references). In a project without the Combat template, those references are missing.
23. **Accumulators the shipped recipes need are not shipped.** Combo, Swing, Charged, ChargeLevel and ShotHeat are not defined by default. Saving warnings are known for `FR_SHOOT_Rifle` and `FR_HOR_HudBreathless` (log, "Found while saving").
24. **Platforms.** The runtime modules allow Mac, Linux, Android and iOS, but only Win64 was built (D-073).
25. **Document format.** Fab 4.3.8 does not list .docx, so the PDF is the Fab-facing file.

---

## 6. Information gaps

| Gap | Why it matters | Who |
|---|---|---|
| Lite/Pro split, one listing or two, and whether Lite has the editor | Chapter 15; which chapters and videos apply to Lite | You |
| UE 5.7/5.8 results | Chapter 3 requirements; Fab needs 5.8 at first submission | Developer, after you install them |
| Where the PDF is hosted, and the `DocsURL` | Links, the listing, the uplugin | You |
| YouTube channel, video URLs, and who records and voices the videos | the video table and the video boxes | You |
| How the demo kits reach buyers (guide, code files, or scripts) | Chapter 13 | You decide, developer builds |
| Example project download (Fab 4.3.6.3) | Chapter 13 links | You |
| Exact FeelKit folder name under `Engine/Plugins/Fab` | The GAS copy step in 3.5 | Check at the first real install (developer with you) |
| Blueprint-only packaging behavior and message wording | 3.6 and the FAQ | Developer; the case without Visual Studio needs a second machine |
| Performance numbers | Chapter 12 | Developer measures |
| Split-screen behavior | Chapters 6 and 8 | Developer tests |
| Support email to print (the uplugin has your personal address) | 1.4 | You |
| Page size A4 or Letter (default A4) | Layout | You |
| Ship the PDF inside the plugin or not | FilterPlugin | You |
| Fab license wording for chapter 22 | Credits and licenses | You |
| PlayStation rumble decision (FAQ only, or more work) | FAQ wording | You |
| Comfort widget: build one or not | 8.9 | You |
| Created with AI flag (listing, not the manual) | Fab 1.8.8.a | You |
| A clean docs project for screenshots | Every HAND shot | Developer |

---

## 7. Writing order

1. **Pass 1: foundations.**
   - Build `build_manual.py`, `gen_reference.py` and `lint_manual.py`.
   - Generate chapters 16 to 19.
   - Write chapter 2 and Appendix A.
   - Result: every later page can link to concepts and reference entries.
2. **Pass 2: the buyer path.** Chapters 3, 4, 5 and 6, then 9, together with shot list rows and video boxes (URLs empty).
3. **Pass 3: trust.** Chapters 8, 14, 22, 23 and 1. Chapter 1 is written last, because it summarises the rest.
4. **Pass 4: depth.** Chapters 7, 10, 11, 12 and 20.
5. **Pass 5: after decisions.** Chapters 13, 15 and 21, and Appendices B and C.
6. **Pass 6: pictures and release.**
   - Capture the shots in the screenshot table.
   - Fill in the video URLs.
   - Lint.
   - Build the .docx and PDF.
   - Read the PDF once, checking bookmarks, the table of contents and link targets.

Rerun `gen_reference.py` and the build after any plugin change and before every release.

---

## 8. Style rules (your hard rule, enforced by the lint)

- **Plain, specific wording.** Say what a control does and what the reader sees.
- **Banned words:** seamless, robust, leverage, comprehensive, delve, "whether you're", unlock, elevate, empower, game-changing, ultimate, revolutionary. Also no "the best" and no hype.
- **No em dashes** (U+2014). No dash used as a pause.
- **No emoji.**
- **No filler.** No introductory or summary paragraphs, and no "In this chapter we will".
- **No internal IDs:** none of D-, I-, requirement codes or phase numbers.
- **Examples come from the real shipped recipes** (section 3). Never invent content that is described as shipped.
- **General wording in reference text and tooltips** (D-023): describe behavior generally, not tied to one use case.
- **Describe only what exists in the code.** Anything planned or undecided is marked in the draft only.
- **Spelling: American** (D-097): behavior, color, center, license, organized, canceled. Quote UI labels, property names and code exactly as the editor shows them, for example Color Tint, Field Of View Kick, `FFeelTarget`.
- **Formatting:** UI labels in bold; menu paths as **Tools** > **FeelKit Debugger**.
- **One task per numbered list,** and each step names the window, the panel and the control (the process note from I-035 and the B1 checklist fixes).
- **Numbers** come with units and the default.

---

### Critical files for implementation
- B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Source\FeelCore\Public\FeelBlueprintLibrary.h
- B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Source\FeelCore\Public\FeelSettings.h
- B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Source\FeelCore\Public\Steps\ (all step headers) and B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Source\FeelEditor\Private\SFeelTimeline.cpp
- B:\NewUE5Project\Docs\FeelKit_1_Product.md
- B:\NewUE5Project\Docs\FeelKit_4_Log.md

---

## Additions after approval (must be written)

### Comfort menu (chapter 8, replaces the "build your own" plan for 8.9; D-085, D-092, D-093)
- **8.9 The comfort menu:** what `WBP_FeelComfortMenu` contains (Master and six comfort sliders, presets, Reset, advanced
  section with camera roll, field of view speed limit and flash limiter, Try button), how to open it with
  **Show Feel Comfort Menu**, mouse, keyboard and gamepad use, Lite and Pro.
- **8.10 Restyling the comfort menu (important, the user asked for it explicitly):**
  - Why a copy: Fab installs FeelKit into the engine folder, not into the project. A change to FeelKit's own
    `WBP_FeelComfortMenu` is shared by every project on that engine, and the next FeelKit update from Fab replaces it
    with the default without warning.
  - How: **Copy to Project** on the menu, restyle the copy freely in the UI designer, set the copy as the menu class on
    **Show Feel Comfort Menu**.
  - What must stay: the names of the sliders and buttons the logic looks for (list them); anything left out is skipped.
  - Editing FeelKit's own copy works, but does not survive an update.
- **FAQ entry:** "I restyled the comfort menu and my changes are gone after updating FeelKit" with the same answer.
- **Every comfort setting documented** (user request D-085): what each slider, preset, option and project setting does,
  its range and default.
- Shots: the menu in game, the menu in the UI designer, the Copy to Project action, the node with the menu class.
- **Feel Switch opens the menu (D-095):** 6.11 and 8.9 mention Comfort Menu Keys (O and controller Menu / Options by
  default, same key closes), Comfort Menu Class, Pause In Comfort Menu, the automatic "Comfort settings" row in the
  controls panel, and the menu's Close Keys list.

---

## Lists read by the build script

The three tables below replace the old `outline.csv`, `videos.csv` and `shots.csv`. The manual build script
(`Tools/Manual/build_manual.py`) reads them from this file, so keep the column names and the marker line above each
table.

### Outline

Every chapter and section, with the anchor the manual's cross-references use. A reference to a section that is not
written yet shows in the draft as highlighted text.

<!-- table: outline -->
| Anchor | Number | Title |
|---|---|---|
| ch01 | 1 | Welcome |
| ch01_what | 1.1 | What FeelKit does |
| ch01_package | 1.2 | What is in the package |
| ch01_manual | 1.3 | How to use this manual |
| ch01_help | 1.4 | Getting help |
| ch02 | 2 | Core ideas |
| ch02_recipes | 2.1 | Recipes, tracks and steps |
| ch02_channels | 2.2 | Channels, and what happens when effects overlap |
| ch02_intensity | 2.3 | Intensity: how strong each effect plays |
| ch02_targets | 2.4 | Targets: what a recipe plays on |
| ch02_parameters | 2.5 | Parameters, accumulators and sustained recipes, in brief |
| ch02_events | 2.6 | Events and Feel Maps, in brief |
| ch02_comfort | 2.7 | Comfort on one page |
| ch02_preview | 2.8 | Why the editor preview matches the game |
| ch03 | 3 | Installing FeelKit |
| ch03_requirements | 3.1 | Requirements: engine versions, platforms, and when Visual Studio is needed |
| ch03_install | 3.2 | Installing from Fab and enabling the plugin |
| ch03_dependencies | 3.3 | What FeelKit switches on for you (Niagara, Enhanced Input) |
| ch03_content | 3.4 | Showing FeelKit content in the Content Browser |
| ch03_gas | 3.5 | Adding the GAS add-on (a folder you copy) |
| ch03_packaging | 3.6 | Blueprint-only projects and packaging |
| ch03_upgrade | 3.7 | Moving a project from Lite to Pro |
| ch04 | 4 | Quick start: your first recipe |
| ch04_pick | 4.1 | Pick a recipe from the library |
| ch04_preview | 4.2 | Preview it without pressing Play |
| ch04_play | 4.3 | Play it from a Blueprint |
| ch04_switch | 4.4 | Compare with the feel off (Feel Switch) |
| ch04_next | 4.5 | Where to go next |
| ch05 | 5 | The recipe editor |
| ch05_create | 5.1 | Creating and opening recipes |
| ch05_window | 5.2 | The window: Preview, Timeline, Details, Intensity |
| ch05_toolbar | 5.3 | The toolbar |
| ch05_tracks | 5.4 | Adding, moving and resizing tracks |
| ch05_curves | 5.5 | Shaping intensity with curve keys |
| ch05_details | 5.6 | Recipe and track settings in Details |
| ch05_preview | 5.7 | The preview: play, loop, scrub, preview mesh, parameter sliders |
| ch05_sound | 5.8 | Sound tracks: waveforms, snapping, tracks made from a sound |
| ch05_comfort | 5.9 | Checking comfort, and the intensity graph |
| ch05_pie | 5.10 | Play in PIE, and editing while the game runs |
| ch05_shortcuts | 5.11 | Copy, paste and keyboard shortcuts |
| ch05_validation | 5.12 | Validation messages on save |
| ch06 | 6 | Playing recipes in your game |
| ch06_play | 6.1 | Play Feel, and choosing a target |
| ch06_context | 6.2 | Passing context with Play Feel with Context |
| ch06_handles | 6.3 | Handles: stop, release, change a parameter |
| ch06_wait | 6.4 | Play Feel and Wait |
| ch06_events | 6.5 | Events and Feel Maps |
| ch06_notifies | 6.6 | Animation notifies |
| ch06_trigger | 6.7 | The Feel Trigger component (no Blueprint wiring) |
| ch06_input | 6.8 | Enhanced Input: the Feel Input component |
| ch06_gas | 6.9 | Gameplay Ability System cues (add-on) |
| ch06_cpp | 6.10 | Using FeelKit from C++ |
| ch06_switch | 6.11 | The Feel Switch: feel off and on while playing |
| ch07 | 7 | Parameters, context and variation |
| ch07_parameters | 7.1 | Parameters and track mappings |
| ch07_distance | 7.2 | The Distance parameter |
| ch07_accumulators | 7.3 | Accumulators: effects that build up with repeated plays |
| ch07_sustain | 7.4 | Sustained recipes |
| ch07_instigator | 7.5 | The instigator, and the Applies To setting |
| ch07_directions | 7.6 | Directions and locations |
| ch07_random | 7.7 | Randomness, conditions and Random Choice |
| ch07_nested | 7.8 | Recipes inside recipes |
| ch08 | 8 | Comfort and accessibility |
| ch08_groups | 8.1 | Comfort groups, and how the scales apply |
| ch08_presets | 8.2 | Presets |
| ch08_settings | 8.3 | Reading and changing a player's settings |
| ch08_essential | 8.4 | Essential tracks and substitutes |
| ch08_flash_limiter | 8.5 | The flash limiter |
| ch08_motion | 8.6 | Motion comfort: camera roll and field of view speed |
| ch08_engine | 8.7 | Engine camera shakes and controller vibration |
| ch08_storage | 8.8 | Saving settings, or using your own save system |
| ch08_menu | 8.9 | The comfort menu for players |
| ch08_restyle | 8.10 | Restyling the comfort menu |
| ch08_audit | 8.11 | The Comfort Audit |
| ch09 | 9 | The recipe library and browser |
| ch09_browser | 9.1 | The Recipe Browser |
| ch09_filters | 9.2 | Filters, search and live preview |
| ch09_copy | 9.3 | Copying a library recipe into your project |
| ch09_readonly | 9.4 | Why library recipes are read-only |
| ch09_tiles | 9.5 | Content Browser tiles and tooltips |
| ch09_recipes | 9.6 | The 38 library recipes at a glance |
| ch09_tagging | 9.7 | Tagging your own recipes |
| ch10 | 10 | Debugging and tuning |
| ch10_debugger | 10.1 | The FeelKit Debugger |
| ch10_replay | 10.2 | Replaying a recent play in the editor |
| ch10_labels | 10.3 | Track labels that explain why a track did not play |
| ch10_showdebug | 10.4 | showdebug feel and stat Feel |
| ch10_log | 10.5 | Log messages |
| ch10_gif | 10.6 | Capture GIF |
| ch11 | 11 | Multiplayer |
| ch11_what | 11.1 | What travels over the network |
| ch11_component | 11.2 | The Feel Replication component and its modes |
| ch11_relevancy | 11.3 | Relevancy distance |
| ch11_time | 11.4 | Hitstop and slow motion in networked games |
| ch11_server | 11.5 | Dedicated servers |
| ch12 | 12 | Performance |
| ch12_idle | 12.1 | When FeelKit runs |
| ch12_play | 12.2 | What a play costs |
| ch12_measure | 12.3 | Measuring it |
| ch12_limits | 12.4 | Keeping many plays in check |
| ch12_assets | 12.5 | Assets and memory |
| ch13 | 13 | Demo levels |
| ch13_get | 13.1 | Getting the demos |
| ch13_arpg | 13.2 | Action/RPG: Weight Class |
| ch13_platformer | 13.3 | Platformer: Bounce Feel |
| ch13_shooter | 13.4 | Shooter: Every Bullet Has an Opinion |
| ch13_horror | 13.5 | Horror: Heartbeat |
| ch14 | 14 | Troubleshooting and questions |
| ch14_nothing | 14.1 | Nothing plays |
| ch14_vibration | 14.2 | Controllers and vibration |
| ch14_camera | 14.3 | Camera and screen effects |
| ch14_materials | 14.4 | Materials and decals |
| ch14_blueprints | 14.5 | Blueprints, animations and GAS |
| ch14_editor | 14.6 | Recipes and the library |
| ch14_install | 14.7 | Installing, packaging and engine versions |
| ch14_help | 14.8 | Getting help |
| ch15 | 15 | Lite and Pro |
| ch15_contents | 15.1 | What each edition contains |
| ch15_lite | 15.2 | What Lite leaves out |
| ch15_recipes | 15.3 | The Lite library |
| ch15_data | 15.4 | Recipes and data in Lite |
| ch15_upgrade | 15.5 | Moving from Lite to Pro |
| ch15_license | 15.6 | Licenses |
| ch16 | 16 | Step reference |
| ch17 | 17 | Blueprint nodes and components |
| ch18 | 18 | Project settings and editor preferences |
| ch19 | 19 | Console commands |
| ch20 | 20 | Writing your own steps in Blueprint and C++ |
| ch20_model | 20.1 | How a step works |
| ch20_blueprint | 20.2 | A step in Blueprint |
| ch20_cpp | 20.3 | A step in C++ |
| ch20_channels | 20.4 | Channels and comfort |
| ch21 | 21 | Recipe JSON files and editor scripting |
| ch21_menu | 21.1 | Exporting and importing in the Content Browser |
| ch21_format | 21.2 | The file format |
| ch21_scripting | 21.3 | Editor scripting |
| ch22 | 22 | Credits and licenses |
| ch22_license | 22.1 | FeelKit's license |
| ch22_sounds | 22.2 | Sample sounds |
| ch22_materials | 22.3 | Materials |
| ch22_demos | 22.4 | Demo projects |
| ch22_project | 22.5 | Your own project |
| ch23 | 23 | Version history |
| ch23_100 | 23.1 | 1.0.0 |
| appA | A | Glossary |
| appB | B | Walkthrough: building the Action/RPG sword hit |

### Screenshots

What each column means:
- **ID:** S = screenshot, D = drawing; then the chapter and a number. The picture file is `Docs/Manual/Screenshots/<ID>.png`.
- **Shows:** what must be visible in the picture.
- **Window** and **State:** where to take it and how things must be set up first.
- **Taken by:** **You** (by hand in Unreal), **Script** (I take it with a picture script), **Drawing** (a diagram I make; not a screenshot).
- **Project:** **Clean project** = a new Third Person project with only FeelKit installed, so pictures show what a buyer
  sees; **GameFeelDev** or **FeelDemoFP** = our demo projects.
- **Status:** missing, taken, or in the manual.

**Your list (what is left for you to take): nothing at the moment.** Since 2026-09-25 every screenshot is taken by
script at twice the screen's pixels (the `DiagFeel.ManualShots*` diagnostics paint the editor off screen at 2x; game
views are a 2x scene render with the interface layered on top), then cropped by `Tools/Manual/prepare_shots.py`. The
nine shots you took by hand were retaken this way for sharper print; your originals stay in `Screenshots/Originals`.
D02-01 is a drawing I make. New shots come with each chapter as it is written and appear here first.

**Picture quality rules.** A picture is printed at no more than 330 pixels per inch and no less than 165 (the size of a
normal screen picture); the PDF export puts the original pictures back after Word, which would otherwise resample every
picture to 200 ppi JPEG. The one exception is S19-01: the showdebug text exists only on screen, so it is read from the
desktop with the text drawn at twice its size.

<!-- table: shots -->
| ID | Chapter | Shows | Window | State | Taken by | Project | Status |
|---|---|---|---|---|---|---|---|
| S01-01 | 1 | The recipe editor on the HeavyHit copy, playhead at 0.05 s | Recipe editor | Intensity tab closed | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S02-01 | 2 | The recipe editor on a project copy of FR_Impact_HeavyHit (saved as HeavyHit), seven tracks, numbered markers 1 to 6 (steps, a track, channel, playhead, recipe settings, preview) | Recipe editor | Playhead at 0.05 s | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S02-02 | 2 | The Intensity tab of FR_Impact_ScalableHit with one line per channel across the recipe | Recipe editor | Default window layout (your editor remembers an older layout without the Intensity tab); Damage at 50 | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S02-03 | 2 | An example Feel Map with four rows: Feel.Event.Hit.Landed plays LightHit; with Required Tag Hit.Heavy it plays HeavyHit; with Hit.Critical CriticalHit; Feel.Event.Hit.Received plays DirectionalDamage. Hit.Heavy and Hit.Critical are project gameplay tags added for the example | Feel Map editor | All rows expanded | Script | GameFeelDev | taken 2026-09-26 by script at twice the screen resolution (`Tools/Run/manual_concepts_shot.ps1`; the two example tags are added for the run and removed) |
| D02-01 | 2 | Drawing, not a screenshot: gameplay sends a Feel Event (tag and context), a Feel Map picks the recipe, the recipe's tracks reach camera, screen, actor, audio, controller and UI | Drawing | The manual's palette | Drawing | none | drawn 2026-09-26 (`Tools/Manual/draw_diagrams.py`) |
| S03-02 | 3 | Edit > Plugins searched for FeelKit: the FeelKit and FeelKit GAS entries | Plugins | FeelKit GAS present through the add-on | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S03-03 | 3 | Content Browser at FeelKit Content: Demos, Library, Samples, UI | Content Browser | Show Plugin Content on | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-01 | 4 | Content Browser at FeelKit Content > Library > Impact with FR_Impact_HeavyHit selected; markers 1 folder, 2 recipe | Content Browser | Show Plugin Content on | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-02 | 4 | The library recipe open read-only: banner and Copy to Project, spotlight on the banner | Recipe editor | Allow Library Editing off | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-03 | 4 | Preview above the Timeline toolbar, spotlight on Play, Stop, Loop and the ruler | Recipe editor | HeavyHit copy, playhead 0.05 s | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-04 | 4 | Event Graph: 1 (Pressed) to Play Feel (Recipe HeavyHit), Target from Make Feel Target from Actor with Self | Blueprint editor | Character Blueprint made by the script | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-05 | 4 | Before and 0.07 s after pressing 1, side by side | Play In Editor | Lvl_ThirdPerson, motion blur off for the picture | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-06 | 4 | Feel Switch start card and FEEL: ON badge, spotlighted | Play In Editor | Default Feel Switch | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S04-07 | 4 | The FEEL: OFF badge, spotlighted | Play In Editor | After Tab | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-01 | 5 | The recipe editor on the ScalableHit copy with markers 1 Preview, 2 Timeline, 3 Intensity, 4 Details | Recipe editor | Intensity tab open | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-02 | 5 | The Timeline toolbar | Recipe editor | HeavyHit copy | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-03 | 5 | The Procedural Shake track selected, its intensity lane spotlighted | Recipe editor | HeavyHit copy, track 2 selected | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-04 | 5 | Details of the selected track: Track category and Step | Recipe editor | HeavyHit copy, track 2 selected | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-05 | 5 | The Preview parameters row (Charge) with Defaults | Recipe editor | Transient copy of FR_Power_ChargeUp | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-06 | 5 | Sustain region on the ruler and Release enabled while looping | Recipe editor | Transient copy of FR_Power_ChargeUp, Loop on, playing | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-07 | 5 | The waveform of the Play Sound track, spotlighted | Recipe editor | HeavyHit copy | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-08 | 5 | The Comfort menu of the preview, open | Recipe editor | HeavyHit copy | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S05-09 | 5 | Asset Check page after saving a recipe with a Play Sound track without a sound | Message Log | Temporary recipe, saved and deleted by the script | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-01 | 6 | Target nodes; Make Feel Target from Actor with Self feeds Play Feel | Blueprint editor | Transient character Blueprint | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-02 | 6 | Play Feel with Context, Make FeelPlayContext with a Make Map (Damage 50) and Self as Instigator | Blueprint editor | Transient character Blueprint | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-03 | 6 | Play Feel handle into Set Feel Parameter (Charge) and Release Feel | Blueprint editor | Transient character Blueprint | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-04 | 6 | Play Feel and Wait with On Finished and On Cancelled | Blueprint editor | Transient character Blueprint | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-05 | 6 | FM_ARPG with its four rows expanded | Details window | Demo content in GameFeelDev | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-06 | 6 | Project Settings, Events, Feel Maps with FM_ARPG | Project Settings | Feel Maps list reduced to FM_ARPG for the picture | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-07 | 6 | Notifies track of AM_ComboAttack with Feel markers | Animation montage editor | Combat template content | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-08 | 6 | Feel Trigger of BP_PlatformingCharacter, entries 0 and 3 expanded | Details window | Demo content in GameFeelDev | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-09 | 6 | Feel Input with one binding (IA_ChargedAttack, FR_Power_ChargeUp) | Details window | Transient component | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S06-10 | 6 | Feel Switch defaults with Switch Keys, Display and Comfort Menu expanded | Details window | Class defaults | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S07-01 | 7 | The Damage slider under the Timeline toolbar, the Damage parameter in Details, and the track's Parameter Mappings row | Recipe editor | Transient copy of FR_Impact_ScalableHit, Procedural Shake track selected | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution; crop fixed 2026-09-26 |
| S07-02 | 7 | Accumulators in Project Settings: Combo, Max Value 10, Decay Per Second 1, Decay Delay 1.5 s | Project Settings | Accumulators reduced to Combo for the picture | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution (from the S18-01 picture) |
| S07-03 | 7 | A Random Choice track's three Play Sound options with weights 2, 1, 1 | Details window | Transient recipe | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S08-01 | 8 | Get Feel Comfort, Apply Comfort Preset (Reduced Motion), Set Comfort Group Scale (Camera Shake 0.5) | Blueprint editor | Transient character Blueprint | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S08-02 | 8 | The essential flash track of FR_Dread_JumpScare with only its Comfort category open | Recipe editor | Transient copy; details category expansion set for the picture and put back | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S08-03 | 8 | The comfort menu during play at the project defaults | Play In Editor | Settings reset without saving | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S08-04 | 8 | The FeelKit Comfort Audit page of the Message Log | Message Log | Audit run on the development project | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S09-01 | 9 | The Recipe Browser, Source Library (38 items), FR_Impact_HeavyHit selected with its details | Recipe Browser | Source set to Library by a scripted click | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S10-01 | 10 | The FeelKit Debugger during Play In Editor: three recipes playing (one sustaining, one with Damage 80), the Combo accumulator, Player 0 comfort with Camera Shake 0.25, two recent plays | FeelKit Debugger | Plays started by the script; comfort set for the session without saving | Script | GameFeelDev | taken 2026-09-26 by script at twice the screen resolution (`DiagFeel.ManualShotsDebugging`) |
| S10-02 | 10 | A replayed play of a HeavyHit copy: Replaying a Recorded Play, Comfort x0.25, Skipped: target 400 cm away, Flash softened by the flash limiter | Recipe editor | Camera Punch given a Max Distance of 100 cm; the play recorded during Play In Editor | Script | GameFeelDev | taken 2026-09-26 by script at twice the screen resolution; the Recent Plays menu picture was dropped (the menu painted away from its button) |
| S11-01 | 11 | A character Blueprint with a Feel Replication component; OnHitLanded calls Play Feel Networked with HeavyHit, a target from Self and Mode Everyone | Blueprint editor | Transient Blueprint, graph at 1:1 | Script | GameFeelDev | taken 2026-09-26 by script at twice the screen resolution (`DiagFeel.ManualShotsNetwork`) |
| S11-02 | 11 | Playback settings: Blend Out Time and Allow Global Time Dilation in Multiplayer (highlighted) | Project Settings | Default values | Script | GameFeelDev | 2026-09-26, cropped from the S18-01 picture |
| S13-01 | 13 | Action/RPG: a combo hit on the training dummy | Play In Editor | Scene only, 3840 x 2160 | Script | GameFeelDev | taken 2026-09-26 by `Tools/Run/run_gallery.ps1` |
| S13-02 | 13 | Platformer: a dash | Play In Editor | Scene only, 3840 x 2160 | Script | GameFeelDev | taken 2026-09-26 by `Tools/Run/run_gallery.ps1` |
| S13-03 | 13 | Shooter: firing the rifle | Play In Editor | Scene only, 3840 x 2160 | Script | FeelDemoFP | taken 2026-09-26 by `Tools/Run/run_gallery.ps1` |
| S13-04 | 13 | Horror: sprinting through the corridor | Play In Editor | Scene only, 3840 x 2160 | Script | FeelDemoFP | taken 2026-09-26 by `Tools/Run/run_gallery.ps1` |
| S16-01 | 16 | The + Track menu open, listing every step by display name | Recipe editor | Menu opened from the Timeline toolbar | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S16-02 | 16 | A Global Hitstop track with its No preview label, next to a Camera Punch track that previews | Recipe editor | Recipe with both tracks | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S17-01 | 17 | The Blueprint context menu with "Feel" typed in the search box, showing the Feel categories | Blueprint editor | Event Graph of BP_ThirdPersonCharacter; Context Sensitive on | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S18-01 | 18 | Project Settings, Plugins, FeelKit, with the Camera and Playback categories expanded and one example accumulator (Combo) | Project Settings | Default values | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S18-02 | 18 | Project Settings, Plugins, FeelKit, with the Comfort category and Default Comfort Scales expanded | Project Settings | Default values | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S18-03 | 18 | Project Settings, Plugins, FeelKit, with Presets and the Reduced Motion preset expanded | Project Settings | Default values | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S18-04 | 18 | Editor Preferences, Plugins, FeelKit | Editor Preferences | Default values | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |
| S19-01 | 19 | The FEELKIT lines of showdebug feel while two recipes play, with the comfort and Controller vibration lines (debug text drawn at twice its size) | Play In Editor | Two sustained recipes playing on the character | Script | GameFeelDev | taken 2026-09-25 by script at twice the screen resolution |

### Videos

<!-- table: videos -->
| ID | Title | Minutes | Link | Linked from sections |
|---|---|---|---|---|
| V1 | Install FeelKit and play your first recipe | 6 |  | 3.2 and 4.1 |
| V2 | Build a recipe on the timeline | 9 |  | 5.1 and 7.1 |
| V3 | Trigger recipes from your game | 9 |  | 6.1, 6.5, 6.6 and 6.7 |
| V4 | Comfort settings for your players | 7 |  | 8.1 and 8.3 |

---

## Video production guide

How to record, edit and publish FeelKit's videos: the four tutorials in the video table above and two listing videos
for Fab. `Tools/Manual/build_video_guide.py` builds this part into `Docs/Manual/out/FeelKit_Video_Guide.docx` (and a
PDF with `--pdf`) in the manual's look. Headings below start one level down: `###` is a chapter of the guide, `####` a
section and `#####` a subsection. Tables whose first column is **Shot** are the shot-by-shot scripts and print on
landscape pages. Facts about this machine were measured on 2026-09-25.

### The videos {#vg_videos}

The tutorials repeat the manual's steps in motion for readers who would rather watch, and each one belongs to one
chapter of the manual, which embeds it as a video box. The listing videos answer a different question: what FeelKit
changes in a game, shown on the Fab page in under 90 seconds.

#### The set {#vg_set}

{widths: 6,22,20,20,9,23}
| ID | Title | Where it is used | Audience | Length | Edition |
|---|---|---|---|---|---|
| V1 | Install FeelKit and play your first recipe | Manual 3.2 and 4.1; YouTube playlist; both listing descriptions | New owners of Lite or Pro, Blueprint users | 6 min | Lite and Pro. One shot names the Recipe Browser with a PRO label |
| V2 | Build a recipe on the timeline | Manual 5.1 and 7.1 | Designers and programmers who author recipes | 9 min | Lite and Pro. The parameter shot and the waveform are Pro |
| V3 | Trigger recipes from your game | Manual 6.1, 6.5, 6.6 and 6.7 | Blueprint users who connect gameplay to recipes | 9 min | **Pro.** Everything after Play Feel and Stop Feel is Pro |
| V4 | Comfort settings for your players | Manual 8.1 and 8.3 | Developers preparing accessibility options | 7 min | Lite and Pro. The Debugger and Comfort Audit shots are Pro |
| T1 | FeelKit Pro listing video | Fab gallery of the Pro listing; YouTube | Fab visitors comparing products | 75 s | **Pro.** Shows the four demos, which use Pro features |
| T2 | FeelKit Lite listing video | Fab gallery of the Lite listing | Fab visitors looking for a free option | 40 s | Lite. Shows only what the Lite download contains |

Sources for the listing videos: the Launch document allows a gallery video (1920 x 1080, up to 300 MB, MP4, MOV or
WEBM) and names a feel off / feel on clip as a candidate; the marketing brief asks for video scripts and marks every
visual as real; the Product document lists trailer material among the content. The video table above feeds the
manual's video boxes and holds only V1 to V4; the T1 and T2 links go into the Launch document (section 4.1).

Not covered here: the one "how I made it" walkthrough of the Action/RPG sword hit. It is Appendix B of the manual,
planned as a document with screenshots and marked Later.

#### Why these lengths {#vg_lengths}

- **One task per video.** A tutorial viewer follows along in their own editor and pauses often. Each video covers one
  manual chapter, so a viewer who needs one task watches one video. All four stay under 10 minutes; short, single-task
  videos are easier to review and to find again.
- **Time per action.** An action that the viewer copies (point at the control, click, see the result) needs 20 to 45
  seconds at a pace that can be followed without pausing. V1 has 15 such actions plus an opening result and an end card
  (about 6 minutes). V2 has 13 actions, several with two results each (about 9 minutes). V3 covers six ways to play a
  recipe at 60 to 90 seconds each (about 9 minutes). V4 has 10 comfort topics at 30 to 60 seconds each (about 7 minutes).
- **Narration density.** At 140 words per minute, the scripts need 4.4 minutes of speech in V1's 6 minutes (72%), 4.3
  in V2's 9 (48%), 4.2 in V3's 9 (47%) and 3.8 in V4's 7 (54%). V2 and V3 spend more time on silent work: typing
  names, wiring nodes, waiting for results. The build script prints these figures and names any shot whose narration
  runs faster than about 160 words per minute (none at present). If an edited video comes out shorter than planned,
  the shorter length is correct: silence is cut, not filled.
- **Listing videos.** Fab visitors skim a gallery. The first 12 seconds must show one feel off / feel on difference.
  T1 gives each of the four demos 10 to 12 seconds, the editor 13, one node 7, comfort 8 and the end card 5
  (75 seconds). T2 keeps to what Lite contains and needs 40 seconds.
- **The Minutes column** of the video table prints in the manual as "about N minutes". After the edit, enter the
  final length rounded to whole minutes.

### Recording setup {#vg_setup}

#### This machine {#vg_machine}

{widths: 16,34,50}
| Item | Measured | Consequence |
|---|---|---|
| Monitors | Two, each 1920 x 1080, running at 144 Hz | Record monitor 1; keep OBS, this guide and notes on monitor 2 |
| Graphics card | AMD Radeon RX 7900 XTX | Hardware encoder is AMD AMF: in OBS "AMD HW H.264 (AVC)" or "AMD HW H.265 (HEVC)". NVENC is NVIDIA only and does not apply here |
| Processor | AMD Ryzen 7 5700X, 8 cores | x264 software encoding at 1080p60 works but competes with Unreal for the processor; use the hardware encoder |
| Free space | D: 698 GB, B: 146 GB | Raw recordings go to `D:/FeelKitVideo/`, outside the folder Synology Drive syncs |
| Installed | No OBS Studio, no DaVinci Resolve, no ffmpeg (the PATH names two ffmpeg folders that do not exist) | Install the software in the next section before the first session |

**Refresh rate.** At 144 Hz, a 60 fps capture takes frames at uneven intervals (144 / 60 = 2.4 refreshes per frame),
and motion judders. For recording sessions, set monitor 1 to 120 Hz (every captured frame is exactly two refreshes)
or 60 Hz: Windows **Settings** > **System** > **Display** > **Advanced display** > **Choose a refresh rate**. In
Unreal, cap the frame rate with the console command `t.MaxFPS 60`.

#### Software {#vg_software}

{widths: 30,36,24,10}
| Program | Use | Where from | Cost |
|---|---|---|---|
| OBS Studio (current release) | Screen and audio recording | obsproject.com | Free |
| DaVinci Resolve (free version) | Editing, captions check, loudness | blackmagicdesign.com | Free |
| Microsoft PowerToys | Mouse Highlighter for clicks | Microsoft Store or GitHub | Free |
| ffmpeg (optional) | Only for the automated captures and loudness checks in the last chapter | ffmpeg.org builds | Free |

The person recording downloads and installs these; the developer does not download or install programs.

#### Capture resolution and application scale {#vg_resolution}

Three ways to capture. The recommended one is 2560 x 1440 through AMD Virtual Super Resolution: the 1080p monitor then
offers higher resolutions in Windows, Unreal renders the editor with more pixels, and OBS records all of them. Text
stays sharp after YouTube's compression, and zoom-ins in the edit stay sharp up to 1.33x.

- Turn it on in **AMD Software: Adrenalin Edition** > **Gaming** > **Display** > **Virtual Super Resolution**, then pick
  2560 x 1440 for monitor 1 in Windows display settings. The physical screen shows the picture scaled down.
- Unreal's **Application Scale** keeps the interface at a readable size (table below). At 1.5 on a 2560 x 1440 desktop
  shown on a 1080p screen, the person recording sees the interface at 1.125 times its normal size.
- Verify while preparing: 2560 x 1440 appears in the Windows resolution list after VSR is on, and Unreal stays at 60
  fps in the Third Person level at that resolution.

{widths: 20,15,25,20,20}
| Capture resolution | Application Scale | Interface size on the recording | Sharp zoom in a 1080p export | Export for YouTube |
|---|---|---|---|---|
| 1920 x 1080 (native) | 1.25 | Reference size | None (a zoom enlarges pixels; keep zooms at 1.25x or less) | 1920 x 1080, 60 fps |
| 2560 x 1440 (VSR), recommended | 1.5 | 90% of reference, slightly more room in panels | Up to 1.33x | 2560 x 1440, 60 fps (YouTube streams 1440p at a higher bitrate) |
| 3840 x 2160 (VSR) | 2.25 | 90% of reference | Up to 2x | 2560 x 1440 or 3840 x 2160, 60 fps |

The listing videos are always exported at 1920 x 1080, the size Fab asks for; a 1440p recording scaled down gives a
sharper 1080p file.

#### OBS profile, scene and sources {#vg_obs}

1. **Profile** > **New**, name `FeelKit 1440p` (or `FeelKit 1080p`). **Scene Collection** > **New**, name `FeelKit`.
2. Add a scene named `Unreal` with a **Display Capture** of monitor 1. Window Capture misses Unreal's context menus,
   dropdowns and tooltips, which are separate windows, so it is not suitable for tutorials.
3. Add **Application Audio Capture**, set to the Unreal Editor window, for the game and editor sound. In **Settings** >
   **Audio**, set **Desktop Audio** to **Disabled**, so Windows sounds and other programs are not recorded.
4. Add the microphone as **Audio Input Capture** and give it the filters in the microphone section.
5. **Edit** > **Advanced Audio Properties**: microphone on track 1 only, Unreal on track 2 only. Separate tracks let
   the edit lower the game under the narration and raise it for results.

{widths: 30,23,23,24}
| Setting (where in OBS) | 1920 x 1080 source | 2560 x 1440 source | 3840 x 2160 source |
|---|---|---|---|
| Base (Canvas) Resolution (Settings > Video) | 1920 x 1080 | 2560 x 1440 | 3840 x 2160 |
| Output (Scaled) Resolution | 1920 x 1080 | 2560 x 1440 | 3840 x 2160 |
| Common FPS Values | 60 | 60 | 60 |
| Output Mode (Settings > Output) | Advanced | Advanced | Advanced |
| Recording Format | Hybrid MP4 when offered, otherwise MKV (then File > Remux Recordings) | Same | Same |
| Video Encoder | AMD HW H.264 (AVC) | AMD HW H.265 (HEVC) | AMD HW H.265 (HEVC) |
| Rate Control | CQP | CQP | CQP |
| CQP value | 18 | 18 | 20 |
| Keyframe Interval | 1 s | 1 s | 1 s |
| Preset | Quality | Quality | Balanced if frames drop, otherwise Quality |
| B-Frames | 0 (faster scrubbing in the editor) | 0 | 0 |
| Approximate data rate | 40 to 80 Mbps, up to 0.6 GB per minute | 60 to 110 Mbps | 100 to 180 Mbps |
| Audio Track (Recording tab) | Tracks 1 and 2 ticked | Same | Same |
| Audio Encoder and bitrate (Audio tab) | AAC, 320 kbps per track | Same | Same |
| Color Format, Color Space, Color Range (Settings > Advanced) | NV12, Rec. 709, Limited | Same | Same |
| Sample Rate, Channels (Settings > Audio) | 48 kHz, Stereo | Same | Same |
| Recording Path | `D:/FeelKitVideo/Raw` | Same | Same |

Equivalents on another machine. NVIDIA: NVENC H.264 or HEVC, CQP 18, preset P5, Look-ahead off, Psycho Visual Tuning
on, B-frames 0. No hardware encoder: x264, CRF 16, preset veryfast, profile high, keyframe interval 1 s.

Why constant quality and not a fixed bitrate: the recording is an intermediate file. Constant quality keeps still
editor frames small and fast gameplay sharp, and the final bitrate is chosen at export. HEVC halves the file size at
equal quality at 1440p and 4K. Verify on the first import that Resolve plays the HEVC files smoothly; if not, record
H.264 at CQP 18 instead.

#### Microphone and room {#vg_mic}

- **Room.** The smallest furnished room available, curtains closed, door shut, no fan or air conditioning. Record the
  narration while Unreal is idle, so the computer's fans stay slow.
- **Position.** 10 to 15 cm from the mouth, aimed at the corner of the mouth rather than straight at it, with a pop
  filter. The microphone model is not recorded in the project; a condenser microphone picks up more of the room than a
  dynamic one, so the room matters more with it.
- **Windows.** **Settings** > **System** > **Sound** > the microphone > **Audio enhancements** off. In **Control
  Panel** > **Sound** > **Recording** > the microphone > **Properties** > **Advanced**, choose 1 channel, 24 bit,
  48000 Hz.
- **Level.** Normal speech peaks around -12 dBFS and loud words stay below -6 dBFS on the OBS mixer: mostly in the
  yellow section, never red.
- **Room tone.** At the start of each session, record 10 seconds of silence with the microphone open. The edit uses it
  to fill gaps and to measure the noise floor.
- **Monitoring.** Closed headphones. Speakers leak the game's sound into the microphone.

Filters on the microphone source (right-click the source > **Filters**), in this order:

{widths: 7,19,44,30}
| Order | Filter | Settings | Purpose |
|---|---|---|---|
| 1 | Noise Suppression | Method RNNoise | Removes steady fan noise and hum |
| 2 | Noise Gate | Close Threshold -45 dB, Open Threshold -38 dB, Attack 25 ms, Hold 200 ms, Release 150 ms | Silences the room between phrases. Set Close about 6 dB above the room tone shown on the meter; if word endings are cut, lower both thresholds by 5 dB |
| 3 | Compressor | Ratio 3:1, Threshold -20 dB, Attack 6 ms, Release 60 ms, Output Gain +3 dB, no sidechain | Brings loud and soft words closer together |
| 4 | Limiter | Threshold -3 dB, Release 60 ms | Stops clipping on sudden loud words |

OBS writes the filtered sound into the recording; the unfiltered sound is not kept. When the edit is to do its own
noise reduction, turn filters 1 and 2 off and keep 3 and 4. The final loudness is set in the edit, not in OBS.

#### Windows preparation {#vg_windows}

- **Do not disturb** on (**Settings** > **System** > **Notifications**). Close Discord, Steam, browsers and chat
  programs.
- **Pause Synology Drive** for the session. It syncs `B:/NewUE5Project`, and it restored deleted folders once before.
- **Taskbar** hidden on monitor 1 (**Settings** > **Personalization** > **Taskbar** > **Taskbar behaviors** >
  **Automatically hide the taskbar**); desktop icons hidden; a plain dark gray wallpaper.
- **Display scale** 100% on monitor 1 (**Settings** > **System** > **Display** > **Scale**), so Unreal's Application
  Scale alone sets the interface size.
- **Pointer.** **Settings** > **Accessibility** > **Mouse pointer and touch**: white style, size 2. PowerToys **Mouse
  Highlighter**: primary button color 2E8B57 at 60% opacity, secondary button color 1F6FB2, radius 20 px, fade delay
  400 ms, fade duration 250 ms, turned on with Win+Shift+H. Leave **Find My Mouse** off.
- **Keyboard layout** English (US), so the keys named in the narration match the keyboard.
- **Power.** Power mode **Best performance**; screen saver and sleep off.

#### Unreal preparation {#vg_unreal}

**The recording project.** A new project made for the videos, never GameFeelDev (it holds test content and tools) or
FeelDemoFP.

- **Engine:** UE 5.8, the newest version the listing supports and the one a new buyer most likely has. FeelKit's own
  windows are the same on 5.6, 5.7 and 5.8. Verify the Third Person template's names on 5.8 while preparing
  (`BP_ThirdPersonCharacter`, `Lvl_ThirdPerson`, the animations `MM_Jump` and `MM_Land`).
- **Template:** **Games** > **Third Person**, **Blueprint**, **Desktop**, quality **Maximum**, **Starter Content**
  off. Name `FeelTutorial`, location `D:/FeelKitVideo/Projects`.
- **FeelKit installed the buyer's way.** After the listings are live: from Fab through the Epic Games Launcher (V1
  shows this). Before that: the release package for 5.8 from `Tools/Run/package_plugin.ps1`, placed in the engine the
  way `Tools/Run/check_engine_plugin.ps1` does; the developer prepares it. Remove that copy before installing from Fab.
- **Snapshots.** With the editor closed, copy the project folder (without `DerivedDataCache` and `Saved`) after each
  video: `FeelTutorial_after_V1`, `_after_V2`, and so on. A retake then starts from the exact state the script assumes.

**Editor settings** (Editor Preferences unless stated):

- **General** > **Appearance** > **User Interface** > **Application Scale**: 1.25 at 1080p, 1.5 at 1440p, 2.25 at 4K.
- **General** > **Performance**: **Use Less CPU when in Background** off; **Show Frame Rate and Memory** off.
- **General** > **Loading & Saving**: automatic saving off for the recording sessions (its notification would appear
  in shots). Save by hand after each video.
- **Level Editor** > **Play**: **Game Gets Mouse Control** on, so a click into the viewport is not needed. Shift+F1
  frees the mouse during Play In Editor.
- **Layout:** **Window** > **Load Layout** > **Default Editor Layout**. Close any window that does not appear in the
  scripts. Dock the Content Browser at the bottom (**Dock in Layout** in the Content Browser drawer) so it stays visible.
  Then **Window** > **Save Layout** > **Save Layout As**, name `FeelKit Video`, and load it at the start of each session.
- **Frame rate:** `t.MaxFPS 60` in the Output Log's command line at the start of each session.
- **Motion blur:** leave it as the template ships it. The manual turned it off only for one still picture (the hit
  picture in 4.3), because blur in a still hides the effect; a video shows the game as players see it.
- **Sound:** Unreal at full volume, Windows volume fixed for the whole production (for example 50%).
- **Controller:** an Xbox-compatible (XInput) controller for the rumble shots. Vibration is invisible on screen: the
  narration names it, and V4 shows it in the Debugger. PlayStation controllers vibrate on Windows only through Steam
  Input.

#### Pre-recording checklist {#vg_checklist}

1. Monitor 1 at 120 Hz or 60 Hz, at the capture resolution; VSR on if recording 1440p or 4K.
2. Synology Drive paused; Do not disturb on; Discord, Steam, browsers and chat closed.
3. Taskbar hidden, desktop icons hidden, plain wallpaper.
4. PowerToys Mouse Highlighter on (Win+Shift+H), pointer size 2.
5. OBS: profile and scene collection loaded, recording path on D:, free space above 50 GB.
6. OBS mixer: microphone peaks at -12 dBFS on a test sentence; Unreal on track 2; Desktop Audio disabled.
7. Unreal: project snapshot for this video opened; layout `FeelKit Video` loaded; `t.MaxFPS 60` entered.
8. Application Scale set for the capture resolution; auto save off.
9. The starting state in the video's setup table checked item by item.
10. Controller connected (V1, V2, V4), headphones on.
11. A 10-second test recording played back in Resolve: picture sharp, both audio tracks present, no dropped frames
    (**View** > **Stats** in OBS shows 0 skipped frames).
12. Room tone recorded (10 seconds).
13. Script open on monitor 2 at the shot about to be recorded; take log ready (next chapter).

### Narration and screen style {#vg_style}

#### Voice and wording {#vg_voice}

- The narration is recorded in the person's own voice. It uses the manual's tone: precise, formal and plain, in
  complete sentences, present tense, and "you" for the viewer.
- **Pace:** 140 words per minute. Pause about half a second after naming a control and before clicking it, and a full
  second after a result appears.
- **Labels:** say each label exactly as the editor shows it. Menu paths are spoken as a list: "Tools, FeelKit Recipe
  Browser".
- **Asset names:** read the prefix as letters: "F R Impact HeavyHit" for `FR_Impact_HeavyHit`, "B P Third Person
  Character" for `BP_ThirdPersonCharacter`.
- **Terms:** "hitstop" is one word. "FOV Kick" is read as it is written. Say "Play In Editor" in full.
- **Numbers:** read as displayed, with the unit: "zero point zero eight seconds".
- **Claims:** describe only what is on screen. No product comparisons, no superlatives and no sales adjectives; the
  words the manual's lint rejects are rejected here too.
- **Spelling** on screen is American (color, behavior, center). Unreal's own labels keep their spelling.

#### Pointer and keyboard {#vg_pointer}

- Move the pointer at about half the usual speed, in straight lines. Keep it still while a result plays, and take the
  hand off the mouse during result shots.
- Rest on a control for half a second before clicking it, so the viewer can find it on their own screen. Tooltips
  appear when resting longer; wait for one only when the script asks for it.
- Say the key before pressing it ("Press Tab"). The edit adds a key caption at the moment of the press.
- A mistake means a retake of the shot. A cut inside a pointer movement makes the pointer jump, which reads as edited.

#### Takes and the take log {#vg_takes}

**Method: picture first, voice second.** Record each shot (or a group of shots marked in the Notes column) as its own
clip, with 2 seconds of stillness before and after. Then record the narration shot by shot while watching the clip,
two takes per shot. The edit trims the picture to the voice. This keeps the pointer slow and deliberate and lets a
sentence be redone without redoing the actions. Recording the voice live while acting also works for V1; the timings
in the scripts assume the edit either way.

OBS cannot name files by shot. Keep a take log on paper or in a spreadsheet that is not saved in the project folder,
with the columns: clip file name (OBS time stamp), video, shot, take, keep (yes or no), note. Rename kept clips to
`V2_S07_T2.mp4` (video, shot, take) at the end of the session.

### Video 1: Install FeelKit and play your first recipe {#vg_v1}

#### Setup {#vg_v1_setup}

The flow follows the manual's chapter 4 (written and approved) and the install steps planned for 3.2 and 3.4. The
earlier plan in section 2 of this document picked the recipe in the Recipe Browser with **Use**; the approved chapter 4
uses the Content Browser and **Copy to Project**, which works in Lite and Pro, so the script follows chapter 4.

{widths: 20,80}
| Item | State |
|---|---|
| Starting state | UE 5.8 installed; FeelKit not yet installed (shot 3) or installed from the release package (all other shots); Epic Games Launcher open and signed in; no project |
| Ending state | Project `FeelTutorial` with `/Game/HeavyHit` (copy of `FR_Impact_HeavyHit`), the 1 key in `BP_ThirdPersonCharacter` playing it, a Feel Switch in `Lvl_ThirdPerson`; snapshot `FeelTutorial_after_V1` |
| Keys used | 1, Tab, O, Space, Esc |
| Edition | Lite and Pro. Verify while recording: `FR_Impact_HeavyHit` is in the Lite library once the Lite recipe list is decided. If it is not, use the Lite recipe chosen then; the steps do not change |
| Record when | Shots 1 and 4 to 18 before submission; shot 3 after the listings are live, since the Fab Library only lists a product once it is published |
| YouTube chapters | 0:00 The result; 0:27 Installing from Fab; 0:50 A Third Person project; 1:05 Enabling the plugin; 1:30 FeelKit content; 1:55 A library recipe; 2:50 Previewing; 3:55 Playing it from a Blueprint; 4:55 In the game; 5:15 The Feel Switch; 6:00 Next |

#### Script {#vg_v1_script}

{widths: 4,7,12,19,29,13,16}
| Shot | Time | On screen | Actions | Narration | On-screen text | Notes |
|---|---|---|---|---|---|---|
| 1 | 0:00-0:15 | Play In Editor in `Lvl_ThirdPerson`, badge FEEL: ON | Press 1 twice, 2 s apart. Press Tab (FEEL: OFF), press 1. Press Tab (FEEL: ON), press 1 | This is FeelKit on Unreal's Third Person template. The 1 key plays a recipe: a short freeze, a camera kick, a flash, a sound and a controller rumble. With FeelKit switched off, the same key does nothing. | Title, lower left: Install FeelKit and play your first recipe | Record last, from the finished state. Follow the feel off / feel on rules in the editing chapter |
| 2 | 0:15-0:27 | Title card over a still of the recipe editor | None | In the next six minutes: install FeelKit, copy a recipe from its library, preview it, play it from a Blueprint, and compare the game with the feel on and off. | Title card, 12 s | Title card design in the editing chapter |
| 3 | 0:27-0:50 | Epic Games Launcher | **Unreal Engine** > **Library**; scroll to **Fab Library**; search FeelKit; **Install to Engine**; choose 5.8; **Install** | Like every code plugin from Fab, FeelKit installs into the engine. In the Epic Games Launcher, open Unreal Engine, then Library. In the Fab Library, find FeelKit and click Install to Engine. Choose your engine version and click Install. | Callout on Install to Engine | Verify while recording: launcher labels at the first real install. Cut the download wait. Hide the account name if it shows (blur in the edit) |
| 4 | 0:50-1:05 | Unreal Project Browser | **Games** > **Third Person** > **Blueprint**; Starter Content off; name `FeelTutorial`; **Create** | Create a project from the Third Person template. A Blueprint project is enough: nothing in this video needs C++. | Lower third: Games > Third Person > Blueprint | Cut the project creation and shader compile. The Recent Projects list must show no other projects |
| 5 | 1:05-1:30 | Editor, then the Plugins window | **Edit** > **Plugins**; type FeelKit; tick **Enabled**; **Restart Now** | Open Edit, Plugins, and search for FeelKit. Tick Enabled and restart the editor. FeelKit also turns on the two engine plugins it uses, Niagara and Enhanced Input, so nothing else needs enabling. | Callout on Enabled; version 1.0.0 readable | Hold 1 s on the version. Verify while recording whether Unreal asks to confirm the dependent plugins; if it does, click Yes and keep it in the shot. Cut the restart |
| 6 | 1:30-1:55 | Content Browser | Content Browser **Settings** > **Show Plugin Content**; in the source tree **All** > **Plugins** > **FeelKit Content**; open **Library** > **Impact** | FeelKit's content is part of the plugin, and the Content Browser hides plugin content by default. Open Settings in the Content Browser and turn on Show Plugin Content. FeelKit Content now appears under Plugins. Its Library folder holds ready-made recipes, grouped by feeling. Open Impact. | Lower third: Settings > Show Plugin Content | Zoom 1.33x on the Settings menu (1440p source) |
| 7 | 1:55-2:15 | Impact folder | Rest on `FR_Impact_HeavyHit` until the tooltip shows; double-click it | A recipe is a short timeline of feedback effects. F R Impact HeavyHit is a heavy blow: a brief freeze, a camera shake and punch, a scale punch on the character, a flash, a sound and a rumble. Double-click it to open the recipe editor. | None | The tooltip shows Description, Tracks and Length; hold it 2 s |
| 8 | 2:15-2:40 | Recipe editor, read-only banner | Point at the banner; **Copy to Project**; in the save dialog keep `HeavyHit` in the Content folder; **Save** | Library recipes are read-only, so that an update of FeelKit can replace them without touching your project. The banner says so. Click Copy to Project, keep the name HeavyHit, and save it in your project. The copy opens, and you can change it freely. | Spotlight on the banner, as in the manual's figure | The save dialog must not show a Windows user path |
| 9 | 2:40-2:50 | Still: Tools menu and the Recipe Browser | None | In FeelKit Pro, Tools, FeelKit Recipe Browser shows the same library with filters, and plays each recipe when you rest the pointer on it. | **Pro.** Label, top right | A still is enough; recorded with Pro installed |
| 10 | 2:50-3:20 | Recipe editor with `HeavyHit` | **Play** in the Timeline toolbar; then **Loop**; let it loop three times | The recipe editor plays the recipe on its own camera and mesh, with the same code as the game. Click Play in the Timeline toolbar, or press Space. Click Loop to repeat it while you watch and listen. | Callouts on Play and Loop | Loop gives two flashes per second; stay under three flashes in any second of the video |
| 11 | 3:20-3:40 | Timeline | Loop off; drag the red playhead slowly from 0 to 0.5 s and back | Drag the playhead along the ruler to step through the recipe. Scrubbing backward shows exactly the frames that playing forward shows, because each effect is computed from the time alone. | None | Slow drag, about 4 s each way |
| 12 | 3:40-3:55 | Timeline, two rows | Rest on the **No preview** label of Global Hitstop, then of Force Feedback Curve | Two tracks say No preview: Global Hitstop, which slows the game's time, and Force Feedback Curve, which drives the controller. Both act only in a running game. | Callout on each label | |
| 13 | 3:55-4:25 | Content Browser, then the Blueprint editor | Open `BP_ThirdPersonCharacter` (ThirdPerson > Blueprints); **Event Graph**; right-click an empty area; type 1; **Input** > **Keyboard Events** > **1** | Now play it from a Blueprint. Open the character Blueprint, B P Third Person Character, and its Event Graph. Right-click an empty area, type 1, and choose the keyboard event for the 1 key. | Lower third: Input > Keyboard Events > 1 | Verify the Blueprint's folder on the 5.8 template |
| 14 | 4:25-4:55 | Event Graph | Drag from **Pressed**, type Play Feel, choose **Play Feel** (Feel); set **Recipe** to `HeavyHit`; drag from **Target**, choose **Make Feel Target from Actor**; right-click, **Get a reference to self**, connect to **Actor**; **Compile**; **Save** | Drag from Pressed and add Play Feel, from the Feel category. Set Recipe to HeavyHit. Drag from Target, choose Make Feel Target from Actor, and connect a reference to self. The target decides where each effect goes. Compile and save. | Zoom 1.33x on the node while the pins are set | Same graph as the manual's figure 4.4 |
| 15 | 4:55-5:15 | Play In Editor | **Play**; press 1 twice, 3 s apart | Press Play, then 1. The camera kicks and shakes, the screen flashes, the character's mesh punches in scale, time freezes for a tenth of a second, and a connected controller rumbles. FeelKit moves and scales the mesh, never the collision capsule. | Key caption: 1. Caption at the second press: Controller rumbles | Keep the character still and the camera level |
| 16 | 5:15-5:40 | Editor, Place Actors panel | Esc; **Window** > **Place Actors**; search Feel Switch; drag it into the level; **Play** | To compare, add a Feel Switch. Open Window, Place Actors, search for Feel Switch and drag it into the level. Press Play. A start card explains the switch, and a badge shows FEEL: ON. | Lower third: Window > Place Actors | Wait for the start card before speaking its sentence |
| 17 | 5:40-6:00 | Play In Editor | Press 1; Tab; 1; Tab; 1; then O (menu opens), O (menu closes) | Press Tab to turn FeelKit off, and press 1 again: the same moment, without FeelKit. Tab turns it back on. O opens the comfort settings for players, which the comfort video covers. | Key captions: Tab, O | The Feel Switch opens the menu with O by default |
| 18 | 6:00-6:08 | End card | None | Chapter 4 of the manual has these steps in writing. The next video builds a recipe on the timeline. | End card: next video and the manual link | Hold 8 s if a YouTube end screen is added |

### Video 2: Build a recipe on the timeline {#vg_v2}

#### Setup {#vg_v2_setup}

{widths: 20,80}
| Item | State |
|---|---|
| Starting state | Snapshot `FeelTutorial_after_V1`; FeelKit Pro installed; recipe editor windows closed; controller connected |
| Ending state | `/Game/Hit` with six tracks (Global Hitstop, Camera Punch, Procedural Shake, Screen Flash, Play Sound with `S_FK_Hit_Heavy`, Force Feedback Curve) and the parameter Damage mapped on the shake and the punch; snapshot `FeelTutorial_after_V2` |
| Keys used | Space, F, Shift, Ctrl+wheel, Ctrl+C, Ctrl+V, Ctrl+S, Delete, Shift+F1 |
| Edition | Lite and Pro. Shot 11 (parameters) is Pro. The waveform on the Play Sound track is Pro; in Lite the row shows no waveform |
| YouTube chapters | 0:00 The result; 0:15 A new recipe; 0:40 Adding tracks; 2:35 Moving and resizing; 3:20 Intensity curves; 4:05 Step settings; 4:45 Previewing; 5:15 Parameters (Pro); 6:05 Comfort preview; 6:40 Play in PIE; 7:40 Validation; 8:20 Copy and paste; 8:40 Next |

#### Script {#vg_v2_script}

{widths: 4,7,12,19,29,13,16}
| Shot | Time | On screen | Actions | Narration | On-screen text | Notes |
|---|---|---|---|---|---|---|
| 1 | 0:00-0:15 | Finished `Hit` looping in the recipe editor, then one hit in Play In Editor | Loop on for 6 s; cut to the game, press 2 | This video builds a hit recipe from an empty timeline: a freeze, a camera punch, a shake, a flash, a sound and a rumble, tuned in the preview without pressing Play. | Title, lower left: Build a recipe on the timeline | Record last. Key 2 is wired in V3; for this shot, use the Play in PIE button instead |
| 2 | 0:15-0:40 | Content Browser, then the recipe editor | Right-click in the Content folder > **FeelKit** > **Feel Recipe**; name `Hit`; double-click it | Right-click in the Content Browser and choose FeelKit, Feel Recipe. Name it Hit and open it. The recipe editor has four areas: Preview, Timeline, Details and Intensity. | Numbered markers 1 to 4 on the areas, 4 s | Markers match the manual's figure style |
| 3 | 0:40-1:05 | Timeline | **+ Track** > **Global Hitstop**; drag the track's right edge to about 0.08 s | Click plus Track and choose Global Hitstop. A hitstop freezes the game for a moment, and it works best when it is short. Drag the end of the track to about zero point zero eight seconds. The edge snaps to frames. | Callout on + Track | Snap on (toolbar) |
| 4 | 1:05-1:50 | Timeline | **+ Track** > **Camera Punch**; **+ Track** > **Procedural Shake**; **+ Track** > **Screen Flash** | Add Camera Punch, Procedural Shake and Screen Flash the same way. Each track plays one step. Each step belongs to a channel, which colors the row and decides which comfort setting scales it. | None | One continuous take; rest the pointer on each new row for 1 s |
| 5 | 1:50-2:15 | Timeline and Details | **+ Track** > **Play Sound**; in Details set **Sound** to `S_FK_Hit_Heavy` | Add Play Sound, and in Details set Sound to S F K Hit Heavy, one of the sample sounds that come with FeelKit. In FeelKit Pro, the row shows the sound's waveform. | **Pro.** Label while the waveform is visible | Type Hit_Heavy in the asset picker's search |
| 6 | 2:15-2:35 | Timeline | **+ Track** > **Force Feedback Curve** | Last, add Force Feedback Curve for the controller. Like the hitstop, it says No preview, because it acts only in a running game. | None | |
| 7 | 2:35-3:20 | Timeline | Resize Camera Punch to 0.4 s, Procedural Shake to 0.3 s, Screen Flash to 0.12 s; drag one track while holding Shift; press F; Ctrl+wheel to zoom in and out | Drag a track to move it, and drag its ends to change its length. Snap keeps edges on frame steps; hold Shift to place them freely. Press F to fit the whole recipe into view, and use Ctrl and the mouse wheel to zoom. | Key captions: Shift, F, Ctrl + wheel | |
| 8 | 3:20-4:05 | Timeline, curve lane of the shake | Select Procedural Shake; double-click the lane at the start and near the end; drag the last key down to 0; right-click a key > **Smooth** | Select the shake track, and its Intensity lane opens below it. With no keys, the track plays at full strength. Double-click to add a key at the start and one near the end, then drag the last key down to zero, so the shake fades out. Right-click a key to choose Constant, Linear or Smooth. | Callout on the lane | Zoom 1.33x on the lane while adding keys |
| 9 | 4:05-4:45 | Details of Camera Punch | Select Camera Punch; in Details set **Shape** to **Kick**; raise **Rotation Punch** pitch; click **Play** | Select Camera Punch, and Details shows the step's settings. The default shape, Spring, overshoots and settles. Kick pushes fast and returns smoothly. Switch Shape to Kick, and raise the Rotation Punch pitch for a stronger kick. | Zoom 1.25x on the Details category | Use the property names exactly as Details shows them |
| 10 | 4:45-5:15 | Preview and Timeline | **Loop** on; scrub the playhead to the frame of the strongest punch; rest on the No preview labels | Loop the preview while you tune. Scrub the playhead to find the exact frame of the punch. The hitstop and the rumble are marked No preview; Play in PIE, later in this video, shows them. | None | Two flashes per second while looping; acceptable |
| 11 | 5:15-6:05 | Details (recipe), Timeline slider | Click an empty area of the timeline so no track is selected; in Details **Parameters** add **Damage**, Min 0, Max 100, Default 50; select the shake, **Parameter Mappings** add **Damage**; same on Camera Punch; drag the **Damage** slider under **Preview parameters** from 0 to 100 while looping | In FeelKit Pro, one recipe can serve light and heavy hits. With no track selected, Details shows the recipe. Add a parameter named Damage, from 0 to 100. On the shake and the punch, add a Parameter Mapping to Damage. A Damage slider now appears under Preview parameters: drag it, and the preview follows. In the game, Play Feel with Context passes the value. | **Pro.** Label, top right | Verify while recording: an empty-area click clears the track selection. The slider values are not saved |
| 12 | 6:05-6:40 | Timeline toolbar, Intensity tab | **Comfort** > **Reduced Motion**; play; **Comfort** > **Neutral**; open the **Intensity** tab | The Comfort dropdown previews the recipe as a player with other settings would see it. Reduced Motion plays camera shake and camera motion at a quarter of their strength, and each row shows the scale it received. The Intensity tab plots each channel's strength across the recipe. | Callout on the Comfort x0.25 labels | |
| 13 | 6:40-7:40 | Level editor and recipe editor side by side | **Play**; Shift+F1; in the recipe editor click **Play in PIE**; change **Screen Flash** color; **Play in PIE** again | Play in PIE shows what the preview cannot. Start Play In Editor, then click Play in PIE in the recipe editor. The recipe plays on the player's character, with the hitstop and the rumble. Change a value, click Play in PIE again, and the change is already there, without restarting the game. | Key caption: Shift + F1. Caption: Controller rumbles | Arrange the recipe editor on the right half of monitor 1 before recording |
| 14 | 7:40-8:20 | Timeline, Message Log | Stop play; **+ Track** > **Play Sound** without a sound; Ctrl+S; read the message; select the track; Delete; Ctrl+S | Saving checks the recipe. A Play Sound track without a sound gives a message that names the track and the problem: Track 7, Play Sound has no sound. Delete the track, or set a sound, and save again. | Zoom 1.33x on the message | Verify while recording where the message appears (Message Log or notification) |
| 15 | 8:20-8:40 | Two recipe editors | Select the Screen Flash track of `Hit`; Ctrl+C; open `HeavyHit`; move the playhead; Ctrl+V | Tracks copy between recipes. Ctrl+C copies the selected track, and Ctrl+V in another recipe pastes it at the playhead. | Key captions: Ctrl + C, Ctrl + V | Undo the paste (Ctrl+Z) after the shot, so HeavyHit keeps its seven tracks |
| 16 | 8:40-9:00 | End card | None | Chapter 5 of the manual describes every part of the recipe editor. The next video plays recipes from gameplay: from Blueprints, events, animations and components. | End card | |

### Video 3: Trigger recipes from your game {#vg_v3}

#### Setup {#vg_v3_setup}

{widths: 20,80}
| Item | State |
|---|---|
| Starting state | Snapshot `FeelTutorial_after_V2`. Prepared off camera with **Copy to Project**: `/Game/ChargeUp` (from `FR_Power_ChargeUp`, parameter Charge, sustain from 0.35 to 0.95 s) and `/Game/Land` (from `FR_Weight_Land`, parameter FallSpeed, 0 to 1600, default 600) |
| Ending state | Keys 2 to 5 wired in `BP_ThirdPersonCharacter`; Feel Map `/Game/FM_Game` in the project settings; a Feel Trigger on the character; snapshot `FeelTutorial_after_V3` |
| Keys used | 1 to 5, Space (jump), Esc, Shift+F1 |
| Edition | **Pro.** Play Feel and Stop Feel are also in Lite; the video is published as a Pro tutorial with a PRO label in the title card |
| YouTube chapters | 0:00 Six ways to play a recipe; 0:15 Play Feel with Context; 1:15 Handles, release and parameters; 2:30 Events and Feel Maps; 3:45 Animation notifies; 5:05 The Feel Trigger component; 6:40 The Debugger; 7:50 More ways; 8:30 Next |

#### Script {#vg_v3_script}

{widths: 4,7,12,19,29,13,16}
| Shot | Time | On screen | Actions | Narration | On-screen text | Notes |
|---|---|---|---|---|---|---|
| 1 | 0:00-0:15 | Play In Editor montage | Press 2; hold and release 3; jump from a ledge | Gameplay decides when something happens, and FeelKit decides how it feels. This video connects recipes to gameplay in six ways, from a single node to a component that needs no Blueprint at all. | **Pro.** Title: Trigger recipes from your game, with the label | Record last |
| 2 | 0:15-0:55 | Event Graph of the character Blueprint | Add keyboard event **2**; **Play Feel with Context**: Recipe `Hit`, Target from **Make Feel Target from Actor** with self; drag from **Context** > **Make FeelPlayContext**; under **Parameters** add Damage = 90 | Play Feel with Context plays a recipe with extra information. Connect Make FeelPlayContext to Context, and under Parameters, add Damage with the value 90. Instigator and Location serve tracks that act on a second actor or need a position; this recipe uses neither. With an empty context, the node behaves exactly like Play Feel. | Lower third: Feel > Play Feel with Context | Verify while recording: if the Parameters pin takes no typed values, drag from it and add a Make Map node with the key Damage and the value 90 |
| 3 | 0:55-1:15 | Play In Editor | Compile; Play; press 2; Esc; set the value to 20; Play; press 2 | Press 2: the hit plays at a damage of 90. With the value set to 20, the same recipe plays as a light hit. | Captions: Damage 90, Damage 20 | Keep both hits in the same camera position |
| 4 | 1:15-2:05 | Event Graph | Key **3** Pressed > **Play Feel** (`ChargeUp`); right-click **Return Value** > **Promote to Variable**, name `ChargeHandle`; Released > **Release Feel** (Handle `ChargeHandle`); key **4** > **Set Feel Parameter** (Handle `ChargeHandle`, open the name dropdown, choose Charge, Value 1) | Play Feel returns a handle: a reference to that one play. Promote it to a variable. ChargeUp, a copy of the library's charge recipe, is sustained: it loops its middle part until released. When the key is released, Release Feel ends the loop, and the recipe plays its ending. Set Feel Parameter changes a value while the recipe plays. Its dropdown lists the parameters of the recipe the handle came from, here Charge. | Callout on the dropdown | Stop Feel is mentioned in shot 5 |
| 5 | 2:05-2:30 | Play In Editor | Play; hold 3 for 3 s; press 4; release 3 | Hold 3: the charge builds and holds. Press 4, and the charge jumps to full. Release the key, and the recipe finishes. Stop Feel, from the same handle, would end it at once with a short blend out. | Key captions: 3 held, 4 | |
| 6 | 2:30-3:30 | Content Browser, Feel Map editor, Project Settings, Event Graph | Right-click > **FeelKit** > **Feel Map**, name `FM_Game`; open; **Entries** add: Event `Feel.Event.Hit.Landed`, Recipe `Hit`; save; **Edit** > **Project Settings** > **Plugins** > **FeelKit** > **Feel Maps** add `FM_Game`; key **5** > **Send Feel Event** (Event `Feel.Event.Hit.Landed`, Target self) | Events keep recipe choices out of gameplay code. Create a Feel Map: right-click, FeelKit, Feel Map. Add a row with the event Feel Event Hit Landed and the recipe Hit. In Project Settings, Plugins, FeelKit, add the map under Feel Maps. Send Feel Event with that event now plays whatever the map assigns. Rows with Required Tags choose variants, such as a critical hit, without a branch in Blueprint. | Lower third: Project Settings > Plugins > FeelKit > Feel Maps | One continuous take is not needed; cut between the four windows |
| 7 | 3:30-3:45 | Play In Editor | Play; press 5 | Press 5, and the event plays Hit. When no row matches an event, the Output Log says so, once per event. | Key caption: 5 | |
| 8 | 3:45-4:45 | Animation editor, `MM_Land` | Open `MM_Land`; right-click the **Notifies** track > **Add Notify** > **Play Feel**; in Details set **Recipe** to `Land`; scrub over the notify | Animation notifies play recipes at an exact frame. Open an animation, here the template's landing animation. Right-click the Notifies track, choose Add Notify, then Play Feel, and set Recipe to Land. Notifies also play in the animation editor, so actor effects, such as the squash on landing, can be timed while you scrub. | Lower third: Notifies > Add Notify > Play Feel | Verify while recording: the `MM_Land` path on 5.8, and which of Land's effects show in the animation preview (actor effects only) |
| 9 | 4:45-5:05 | Notify menu | Open **Add Notify** and **Add Notify State** menus without choosing | Three more notifies exist: Send Feel Event, Set Feel Value, which passes a value such as the weight of a swing to later recipes, and Play Feel Window, a notify state that plays a recipe for the length of a window. | None | |
| 10 | 5:05-6:10 | Animation editor, then the Blueprint editor | Delete the Play Feel notify from `MM_Land`; save; in `BP_ThirdPersonCharacter` **Add** > **Feel Trigger**; in Details **Triggers** add: **Event** Landed, **Recipe** `Land`, **Value Parameter** FallSpeed; Compile; Save | The notify plays every landing at the same strength. The Feel Trigger component can pass the real landing speed. Remove the notify, and add a Feel Trigger to the character. Add an entry: Event Landed, Recipe Land, Value Parameter FallSpeed. The landing speed goes into the recipe's FallSpeed parameter, and no Blueprint wiring is needed. | Lower third: Add > Feel Trigger | |
| 11 | 6:10-6:40 | Play In Editor | Play; a small jump on flat ground; then run off the highest ledge | A short hop lands softly. A long fall lands hard, from the same entry. The Feel Trigger also answers damage, hits, overlaps, jumps, air jumps and launches. | None | Keep the camera behind the character for both landings |
| 12 | 6:40-7:50 | Debugger tab beside the game | **Tools** > **FeelKit Debugger**; play 2, 3 and 5 in the game; stop play; in **Recent plays** double-click the `Hit` play | Tools, FeelKit Debugger lists what plays right now, the accumulators, each player's comfort settings and the controller's vibration. Every play that ends is kept under Recent plays. Double-click one, and the recipe editor opens that exact play, with its parameters, to scrub it frame by frame. | Callout on Recent plays | Recent plays are kept after play stops, not in Shipping builds |
| 13 | 7:50-8:30 | Stills: Feel Input details, a Gameplay Cue Blueprint, a line of C++ | None | Chapter 6 of the manual covers three more ways: the Feel Input component for Enhanced Input actions, Gameplay Cue notifies from the GAS add-on, and the Feel subsystem for C++. | Captions naming each still | Stills from the manual's figures when available |
| 14 | 8:30-9:00 | End card | None | The next video covers comfort: the settings that let each player choose how much shake, flash and rumble they get. | End card | |

### Video 4: Comfort settings for your players {#vg_v4}

#### Setup {#vg_v4_setup}

{widths: 20,80}
| Item | State |
|---|---|
| Starting state | Snapshot `FeelTutorial_after_V3`; comfort save slots deleted (the project's `Saved/SaveGames` folder emptied with the editor closed), so the player starts with the default settings |
| Ending state | Keys 7, 8 and 9 wired to comfort nodes; the Screen Flash track of `HeavyHit` marked Essential with a Vignette Pulse substitute; snapshot `FeelTutorial_after_V4` |
| Keys used | 1, 7, 8, 9, O, Ctrl+D, Esc |
| Edition | Lite and Pro. Shots 10 and 11 are Pro |
| YouTube chapters | 0:00 Comfort in FeelKit; 0:15 Groups and presets; 0:50 Comfort in the preview; 1:25 Changing settings from Blueprint; 2:30 Saved settings; 3:00 Essential tracks; 4:00 The flash limiter; 4:40 The comfort menu; 5:30 Engine shakes and vibration; 6:05 The Debugger (Pro); 6:35 The Comfort Audit (Pro); 6:55 Next |

Found while writing this script: the recipe editor shows "Substitute plays (comfort)" only when the preview's Flashes
scale is 0, and none of the presets as shipped sets it to 0 (Reduced Flashing uses 0.2). The code plays a substitute
only at a scale of 0 (`FeelEvaluator.cpp`). Shot 6 therefore shows the substitute in the game, after setting Flashes to
0 from Blueprint. The planned screenshot S08-04 needs the same change.

#### Script {#vg_v4_script}

{widths: 4,7,12,19,29,13,16}
| Shot | Time | On screen | Actions | Narration | On-screen text | Notes |
|---|---|---|---|---|---|---|
| 1 | 0:00-0:15 | Play In Editor with the comfort menu open | O; move **Camera shake** from 100% to 25% with the arrow keys; **Try it** | Players differ in how much shake, flash and rumble they can take. FeelKit gives every player comfort settings that scale its effects, and also the game's own camera shakes and controller vibration. | Title: Comfort settings for your players | Record last |
| 2 | 0:15-0:50 | Project Settings | **Edit** > **Project Settings** > **Plugins** > **FeelKit**; expand **Default Comfort Scales** and the three presets | Comfort settings belong to each player: seven scales from 0 to 1. Master scales everything; the six groups are Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion and Haptics. A track's channel decides its group. New players start with the Default Comfort Scales, and the presets Reduced Motion, Reduced Flashing and No Haptics are edited here too. | Lower third: Project Settings > Plugins > FeelKit | Slow scroll; zoom 1.25x on the scales |
| 3 | 0:50-1:25 | Recipe editor with `HeavyHit` | **Comfort** > **Reduced Motion**; Play; **Comfort** > **No Haptics**; **Comfort** > **Neutral** | The recipe editor previews these settings. With Reduced Motion, the shake and the punch play at a quarter of their strength, and each row shows the scale it received. No Haptics removes the rumble. Neutral plays everything at full strength. | Callout on the Comfort x0.25 labels | |
| 4 | 1:25-2:30 | Event Graph of the character Blueprint | Key **7**: **Get Player Controller** > **Get Feel Comfort** > **Apply Comfort Preset** (Reduced Motion). Key **8**: **Set Comfort Group Scale** (Group Flashes, Scale 0). Key **9**: **Apply Comfort Preset** (Default). Compile | In a game, the settings change through the player's comfort. Get Feel Comfort takes a player controller. On key 7, apply the preset Reduced Motion. On key 8, set the Flashes group to 0. On key 9, apply Default again. A real game connects these nodes to its options menu, or uses the comfort menu shown later in this video. | Lower third: Feel > Comfort | |
| 5 | 2:30-3:00 | Play In Editor | Play; 1; 7; 1; Esc; Play; 1; 9 | Press 1 for the full effect, then 7 and 1 again: less shake and a smaller kick. Stop the game and start it again: the setting is still there. FeelKit saves each player's settings automatically, or hands them to your own save system. | Key captions | Same camera for all three presses |
| 6 | 3:00-4:00 | Recipe editor, then Play In Editor | In `HeavyHit` select **Screen Flash**; Details > **Comfort**: **Essential** on, **Substitute Step** Vignette Pulse; save; Play; 1; 8; 1 | Some effects carry information, such as a flash that signals damage. Mark such a track Essential. A player who turns Flashes to 0 then gets the Substitute Step instead, here a Vignette Pulse, so the signal stays without the flash. Without a substitute, the track keeps a minimum strength, its Essential Floor. On camera shake and camera motion, a player's 0 always wins. | Captions: Flashes 1, Flashes 0 | See the note above the script |
| 7 | 4:00-4:40 | Recipe editor | Select **Screen Flash**; Ctrl+D; drag the copy to 0.25 s; **Comfort** > **Reduced Flashing**; Play; rest on the label of the second flash | The flash limiter counts flashes per second. Reduced Flashing allows one flash per second and suppresses the rest; the second flash's row says so. With the default settings, flashes beyond three per second are softened. The limiter is a helper, not a photosensitivity certification. | Callout on the label | Verify while recording that the label reads Flash suppressed by the flash limiter. Do not loop: two flashes 0.25 s apart. Delete the copy afterward |
| 8 | 4:40-5:30 | Play In Editor, comfort menu | Play; O; move through the rows with the arrow keys; click **Reduced motion**; **Try it**; **Close**; then a still of `/FeelKit/UI/WBP_FeelComfortMenu` in the Content Browser with its right-click menu | Players change these settings in the comfort menu. The Feel Switch opens it with O, and in your own game the node Show Feel Comfort Menu opens it. It has Master and the six groups, the presets, camera roll, a zoom speed limit and the flash limiter. Moving a slider plays a small matching effect, and Try it plays a sample. To restyle it, use Copy to Project on the menu: FeelKit's own copy is replaced by each update. | Lower third: Show Feel Comfort Menu | Verify the right-click entry's label on the menu asset |
| 9 | 5:30-6:05 | Project Settings | Show **Apply Comfort To Engine Camera Shakes** and **Apply Comfort To Engine Force Feedback** | Comfort also scales the engine's own camera shakes and controller vibration, so effects your game already has follow the same settings. Both options are on by default. | None | |
| 10 | 6:05-6:35 | Debugger beside the game | **Tools** > **FeelKit Debugger**; press 7 in the game; expand **Player 0 comfort** and **Controller vibration** | In FeelKit Pro, the Debugger shows each player's comfort settings and what really reaches the controller: the comfort scale and the controller scale. | **Pro.** Label, top right | |
| 11 | 6:35-6:55 | Message Log | **Tools** > **FeelKit Comfort Audit**; scroll the **FeelKit Comfort Audit** page | Tools, FeelKit Comfort Audit checks every recipe and the comfort settings for likely problems, such as more than three flashes in a second or saturated red flashes, and lists them in the Message Log. It helps prepare for an accessibility review; it does not certify a game. | **Pro.** Label, top right | |
| 12 | 6:55-7:05 | End card | None | Chapter 8 of the manual lists every comfort setting with its range and default. | End card with the playlist | |

### Listing videos {#vg_listing}

No narration: game sound and short on-screen text carry the message, because Fab visitors may watch without sound.
Every frame is FeelKit running in Unreal; nothing is added in the edit except the text and the cuts (Fab requires media
to show the real product). The comparison rules in the editing chapter apply to every off / on pair.

#### T1: FeelKit Pro {#vg_t1}

{widths: 20,80}
| Item | State |
|---|---|
| Source | The four demo levels in GameFeelDev (`Lvl_Combat`, `Lvl_Platforming`) and FeelDemoFP (`Lvl_Shooter`, `Lvl_Horror`) on UE 5.6, as the playable demos ship; the recording project for the editor shots |
| Capture | 2560 x 1440 through VSR, exported at 1920 x 1080. Each pair recorded as one take, pressing Tab between the repeats (or with the automated input described in the last chapter) |
| Export | 1920 x 1080, 60 fps, H.264, 16 Mbps: about 150 MB for 75 s, under Fab's 300 MB |

{widths: 4,7,12,19,29,13,16}
| Shot | Time | On screen | Actions | Narration | On-screen text | Notes |
|---|---|---|---|---|---|---|
| 1 | 0:00-0:06 | Action/RPG, badge FEEL: OFF | Light, light, heavy combo on the dummy | None | Same hit. FeelKit off. | Camera 3.5 m behind, level |
| 2 | 0:06-0:12 | Same place and camera, FEEL: ON | Same combo | None | FeelKit on. | Same length as shot 1 within 5 frames |
| 3 | 0:12-0:22 | Platformer, off then on | Jump, air jump, land on the same platform, twice | None | Jumps and landings | 5 s each half |
| 4 | 0:22-0:32 | Shooter, off then on | A 1 s rifle burst at the same target | None | Every shot | 5 s each half |
| 5 | 0:32-0:42 | Horror, off then on | Sprint down the same corridor into a scare spot | None | Tension | 5 s each half; check the flash count |
| 6 | 0:42-0:55 | Recipe editor, `HeavyHit` copy | Scrub the playhead; resize the Camera Punch track; Play | None | Build it on a timeline. Preview without pressing Play. | Real editor, slowed pointer |
| 7 | 0:55-1:02 | Blueprint node, then one hit in the game | Hold on the Play Feel node 2 s; cut to the hit | None | Play it with one node. | |
| 8 | 1:02-1:10 | Comfort menu in the Action/RPG level | Move Camera shake to 0%; Try it | None | Players choose how much they feel. | |
| 9 | 1:10-1:15 | End card | None | None | FeelKit Pro. Unreal Engine 5.6 to 5.8. Playable demos and the manual are linked below. | Cover style: dark background, green accent |

#### T2: FeelKit Lite {#vg_t2}

{widths: 20,80}
| Item | State |
|---|---|
| Source | The recording project with the Lite package installed (the Lite package is not built yet); V1 footage where it shows only Lite content |
| Rule | Nothing on screen may be Pro-only: no Recipe Browser, no waveform on sound tracks, no Debugger, no demo levels that use Feel Triggers or Feel Maps |
| Export | 1920 x 1080, 60 fps, H.264, 16 Mbps (about 80 MB) |

{widths: 4,7,12,19,29,13,16}
| Shot | Time | On screen | Actions | Narration | On-screen text | Notes |
|---|---|---|---|---|---|---|
| 1 | 0:00-0:10 | Third Person level, off then on | Press 1 with FEEL: OFF, Tab, press 1 with FEEL: ON | None | One key. FeelKit off, then on. | From V1 shot 1 material |
| 2 | 0:10-0:22 | Recipe editor with Lite | Add a Camera Punch track; drag its end; Play | None | A timeline editor with a live preview. | Record with the Lite package |
| 3 | 0:22-0:30 | Blueprint and the game | Play Feel node; hit in the game | None | Play Feel from Blueprint or C++. | |
| 4 | 0:30-0:37 | Comfort menu | Reduced motion; Try it | None | The full comfort layer, free. | |
| 5 | 0:37-0:40 | End card | None | None | FeelKit Lite. Free. | |

### Editing {#vg_editing}

#### Project and folders {#vg_edit_project}

- DaVinci Resolve, one project per video. **Project Settings**: timeline resolution equal to the capture (2560 x 1440
  or 1920 x 1080), timeline and playback frame rate 60, color science DaVinci YRGB with Rec.709 Gamma 2.4.
- Folders on D:: `D:/FeelKitVideo/Raw` (OBS), `Voice`, `Edit` (Resolve media and cache), `Export`, `Captions`,
  `Thumbnails`, `Music`. Nothing under `B:/NewUE5Project`, which is synced.
- MKV recordings: remux to MP4 in OBS (**File** > **Remux Recordings**) before importing.

#### Timeline structure {#vg_timeline}

{widths: 15,85}
| Track | Content |
|---|---|
| V1 | Screen clips, cut to the narration |
| V2 | Spotlight and marker overlays |
| V3 | Lower thirds and key captions |
| V4 | Title card and end card |
| A1 | Narration |
| A2 | Unreal sound (game and editor) |
| A3 | Music (listing videos only) |

Order of work: place the narration clips on A1 in shot order with 0.3 to 0.5 s between sentences; cut the screen
clips on V1 to them; then overlays, captions, the mix and the loudness pass.

- Cut waiting (downloads, compiling, restarts) with a straight cut. Where the screen layout changes across the cut, add
  a 6-frame cross dissolve. Never speed up interface footage; sped-up pointers look edited.
- Never cut while an effect plays. A result stays on screen at least 1.5 s after it ends.
- Finish each sentence before the key press it describes, so the effect's sound is heard clearly.

#### Zoom-ins {#vg_zoom}

The pattern for each zoom: full screen for at least 1 s, start the zoom 1 s before the click, hold while the action
happens and 1 s after, return. At most one zoom every 10 s. In Resolve: **Inspector** > **Transform** > **Zoom** and
**Position** keyframes with ease in and out.

{widths: 34,18,16,16,16}
| Subject | Scale (1440p source) | Scale (1080p source) | Zoom in | Zoom out |
|---|---|---|---|---|
| A small control: check box, pin, dropdown entry, curve key | 1.33x | 1.25x | 12 frames (0.2 s) | 12 frames |
| A panel area: Details category, a Feel Map row, a message | 1.25x | 1.15x | 15 frames (0.25 s) | 15 frames |
| Game view during an effect | Never | Never | | |

Gameplay is never zoomed or reframed: a zoom changes the apparent size of camera shakes and punches.

#### Callouts and text {#vg_callouts}

The same look as the manual's figures, drawn once as transparent PNG overlays at the timeline's resolution (sizes
below are for 1920 x 1080; multiply by 1.33 at 1440p).

{widths: 20,80}
| Element | Specification |
|---|---|
| Spotlight | Everything outside the subject covered by black 0A0C10 at 40% opacity; the subject inside a rounded rectangle, 3 px outline 2E8B57, corner radius 10 px, 6 px padding. Fades in and out over 8 frames |
| Numbered marker | Circle 36 px across, fill 2E8B57, 3 px white ring, white number in Segoe UI Bold 22 px |
| Lower third (menu paths, node names) | Segoe UI Semibold 30 px, white, on 161B22 at 90% opacity, corner radius 12 px, padding 24 px, 60 px from the bottom left corner; 3 to 5 s on screen, 8-frame fades |
| Key caption | Same box, key names in Consolas 30 px ("Tab", "Ctrl + D"), bottom center, from the key press for 1.5 s |
| PRO label | "PRO" in Segoe UI Semibold 20 px, 2E8B57 on EAF5EF with a 2 px 2E8B57 outline, top right, 40 px from the edges, for the whole Pro segment |
| Title card | Background gradient 0E1116 to 161B22 as on the cover; title Segoe UI Semibold 64 px white; subtitle Segoe UI 32 px 3FB973; a 12 px by 120 px 3FB973 bar above the title |
| End card | Same background; next video title, the manual link, Discord; 5 s at least |

No moving arrows, no sound effects on callouts (FeelKit's sounds must be the only effects the viewer hears), no text
over the game view during an effect.

#### Captions {#vg_captions}

- The narration column of each script is the transcript. After the edit, collect the final narration text in order
  (the developer can extract it from this document).
- YouTube Studio > **Subtitles** > **Add language** (English) > **Add** > **Auto-sync**, paste the text. YouTube sets
  the timing from the audio. Check it once, correct any caption that starts before its words, then download the file
  as SRT to `D:/FeelKitVideo/Captions/V1.en.srt`.
- Caption rules: one or two lines, at most 42 characters per line, 1 to 7 s each, UI labels spelled as on screen.
- The listing videos carry their text in the picture and have no captions track.

#### Loudness and mix {#vg_loudness}

- **Narration (A1):** high-pass at 80 Hz; a de-esser only if "s" sounds are sharp; no further compression when the OBS
  compressor was on.
- **Unreal sound (A2):** 10 dB below the narration while speech runs; during result shots without speech, as loud as
  the narration. Every result shot must have its sound.
- **Target:** -14 LUFS integrated over the whole video, true peak at or below -1 dBTP. In Resolve: select all clips >
  **Normalize Audio Levels**, ITU-R BS.1770-4, Target Loudness -14 LUFS, True Peak -1 dBTP, **Relative**; then check
  the whole timeline with the Fairlight **Loudness** meter.
- **Why -14:** YouTube lowers louder uploads to about -14 LUFS and does not raise quieter ones, so a quieter video plays
  quieter than the one before it.
- Listing videos use the same target.

#### Music {#vg_music}

- **Tutorials:** no music. The narration must stay clear, and FeelKit's own sounds are part of what the viewer judges.
- **Listing videos:** music is optional, mixed so that hits and landings stay at least 12 dB louder than the music.
- **Licenses:** CC0, or a paid license that allows commercial use on YouTube and outside it (the Fab gallery file). No
  NC (non-commercial) or ND (no derivatives: cutting and looping is a derivative) licenses. No tracks licensed for
  YouTube only. CC-BY is unsuitable for the Fab file, which has no place for a credit.
- Keep the license text or receipt in `D:/FeelKitVideo/Music` and record the track, source and license in the log.
  After upload, check YouTube Studio's copyright check before publishing.

#### Feel off and feel on {#vg_comparison}

Rules for a fair comparison:

1. Same build, level, graphics settings, camera distance and angle. Only the Feel Switch changes.
2. The same action at the same place. Best: one continuous take in which the action repeats, with Tab pressed between
   the repeats, so the camera and lighting cannot differ. For identical input timing, use the automated input in the
   last chapter.
3. The in-game FEEL: ON / FEEL: OFF badge stays visible in every comparison shot. No second label that could disagree
   with it.
4. No edit effects on either side: no added shake, zoom, flash, speed change, color correction or sound, and the same
   audio level on both. The off side keeps the game's own sounds.
5. Off first, then on, each 3 to 6 s, equal lengths within 5 frames.
6. Split screen only after the sequential pair: each half is the center 960 px of its frame (cropped, not scaled, so
   camera motion keeps its size), aligned on the frame where the action connects. The on side then runs longer by the
   hitstop; do not stretch time to hide it.
7. Both sides at 60 fps with the game's motion blur.

### Export and publishing {#vg_publishing}

#### Export settings {#vg_export}

{widths: 26,37,37}
| Setting (Resolve Deliver page) | Tutorials (YouTube) | Listing videos (Fab and YouTube) |
|---|---|---|
| Format and codec | MP4, H.264, High profile | MP4, H.264, High profile |
| Resolution | 2560 x 1440 when recorded at 1440p or 4K, otherwise 1920 x 1080 | 1920 x 1080 |
| Frame rate | 60 | 60 |
| Bitrate | Restrict to 24 Mbps at 1080p, 40 Mbps at 1440p (YouTube recommends 12 and 24 Mbps for 60 fps; the rest is headroom for YouTube's re-encode) | 16 Mbps |
| Keyframes | Automatic | Automatic |
| Audio | AAC, 48 kHz, stereo, 320 kbps or higher | Same |
| Color tags | Rec.709 Gamma 2.4 | Same |
| File name | `FeelKit_V1_install_first_recipe.mp4` | `FeelKit_Pro_listing.mp4`, `FeelKit_Lite_listing.mp4` |

#### YouTube upload {#vg_youtube}

- **Channel:** not decided yet (an open item in this document's information gaps: channel, video URLs, who records).
- **Visibility:** Unlisted while checking; Public when the manual that links them is published.
- **Title:** at most 70 characters, for example "FeelKit tutorial 1: Install FeelKit and play your first recipe".
- **Settings:** audience "No, it's not made for kids"; category Science & Technology; video language English;
  Standard YouTube License; embedding allowed. **Altered or synthetic content:** No, when the voice is the person's own
  and the picture is real screen capture.
- **Chapters:** the list in each video's setup table goes into the description, first line 0:00, at least three
  chapters, each at least 10 s, in rising order. Correct the times after the edit.
- **Playlist:** "FeelKit tutorials", V1 to V4 in order. End screen on the end card: the next video and the playlist.
- **Thumbnail:** 1280 x 720, PNG or JPG under 2 MB. A real frame from the video (the timeline or the hit), a dark band
  over the left third with the video number and a title of three or four words in Segoe UI Semibold 88 px, white, and
  a 3FB973 accent bar. It must read at 320 x 180; check it at that size.

Description template:

```
Installs FeelKit from Fab, copies a recipe from its library, previews it in the
recipe editor and plays it from a Blueprint in Unreal Engine 5.

Manual (PDF): <link>
FeelKit Pro on Fab: <link>
FeelKit Lite on Fab: <link>
Support on Discord: https://discord.gg/AtJ6RdwaxA

0:00 The result
0:27 Installing from Fab
...
```

#### Where the links go {#vg_links}

- **Manual:** the video table of this document (under "Lists read by the build script", marker `table: videos`).
  Fill the **Link** column of V1 to V4 with the full address (`https://www.youtube.com/watch?v=<id>`) and set
  **Minutes** to the final length. `build_manual.py` reads that column: the video boxes then print the link instead of
  "The video will be added after publishing". Rebuild with `python Tools/Manual/build_manual.py --pdf`.
- **Hosted PDF:** replace the Google Drive file as a new version of the same file (**Manage versions** > **Upload new
  version** in Drive), which keeps the share link, the one behind `DocsURL` and the listings. Verify the menu names in
  Drive when doing it.
- **Listings:** the playlist link in both descriptions; the T1 YouTube link in the Pro description (Launch document,
  section 4.1).
- **Fab gallery:** T1 uploaded to the Pro listing, T2 to the Lite listing: 1920 x 1080, up to 300 MB, MP4, MOV or WEBM
  (Launch document, section 4.2). Whether the Fab form also takes YouTube links, and where a video sits in the gallery
  order, is not recorded; check in the publishing form.

### Quality control and schedule {#vg_qc}

#### Checks for every video {#vg_qc_all}

1. Length within 30 s of the target; chapters valid (0:00 first, at least three, each 10 s or longer).
2. Every spoken label matches the screen.
3. No personal data on screen: e-mail addresses, the Windows user name in paths, other projects in the Recent Projects
   list, account names in the launcher, notifications.
4. No pointer jumps inside a shot; the click highlight stays on for the whole video.
5. Every result is audible and stays on screen at least 1.5 s.
6. Never more than three flashes in any one second of the video.
7. Loudness -14 LUFS (within 1 LU), true peak at or below -1 dBTP.
8. Captions in sync, American spelling, at most 42 characters per line.
9. Title, description, captions and on-screen text follow the writing rules: no em dashes, no hype words, no
   exclamation marks.
10. Pro segments carry the PRO label; nothing Pro-only appears unlabeled in a Lite and Pro video.
11. Thumbnail readable at 320 x 180.
12. Watched once full screen at 1440p or 1080p, once on a phone.

#### Checks per video {#vg_qc_each}

{widths: 10,90}
| Video | Checks |
|---|---|
| V1 | Version 1.0.0 readable in the Plugins window; banner text readable; the copy saved as HeavyHit in the project; start card readable; O opens and closes the comfort menu |
| V2 | No preview labels readable; Damage slider visible with the PRO label; the validation message readable; the Play in PIE result visible with the mouse free |
| V3 | The Damage 90 and 20 hits in the same camera; the Set Feel Parameter dropdown readable; the Feel Map row readable; both landings in the same camera; the replayed play opens in the recipe editor |
| V4 | The substitute vignette shown in the same camera as the flash; the flash limiter label readable; the comfort menu text readable; the two Pro shots labeled |
| T1 | Every pair follows the comparison rules; the badge visible in every pair; file 1920 x 1080 and under 300 MB |
| T2 | Only Lite content on screen; recorded with the Lite package; file under 300 MB |

#### Production schedule {#vg_schedule}

{widths: 6,48,24,11}
| Order | Work | When | Hours |
|---|---|---|---|
| 1 | Install OBS, Resolve and PowerToys; turn on VSR; set up the OBS profile; a 2-minute test recording checked in Resolve (sharpness, both audio tracks, loudness) | First | 3 |
| 2 | Recording project, editor settings, layout, snapshot | First | 1.5 |
| 3 | V1 shots 1, 2 and 4 to 18 with the release package installed: picture 2 h, voice 1 h, edit 4 h | Before submission | 7 |
| 4 | V2: picture 3 h, voice 1.5 h, edit 6 h | Before submission | 10.5 |
| 5 | V3: picture 3 h, voice 1.5 h, edit 6 h | Before submission | 10.5 |
| 6 | V4: picture 2.5 h, voice 1 h, edit 5 h | Before submission | 8.5 |
| 7 | T1: demo captures in the four levels 3 h, edit 5 h | Before submission | 8 |
| 8 | T2 | When the Lite package exists | 3 |
| 9 | Captions, thumbnails, unlisted uploads, checks | Before submission | 4 |
| 10 | V1 shot 3 (install from Fab), final V1 edit, publish all, links into the video table, manual rebuilt and replaced on Drive | After the listings are live | 3 |
| | Total | | 59 |

V1 cannot show the Fab install before the listings are live, so V1 goes public shortly after launch; V2 to V4 and the
listing videos can be public at launch. Recording in the order above also builds the project state each later video
starts from.

### What can be automated {#vg_automation}

The developer can drive the Unreal editor and the demo levels through automation tests and editor Python. The runners
in `Tools/Run` already play every demo level with scripted input (`run_arpg.ps1`, `run_arpg_guard.ps1`,
`run_platformer.ps1`, `run_shooter.ps1`, `run_horror.ps1`), switch the feel off for measurements
(`run_arpg_camera.ps1 -FeelOff`) and save still pictures (`run_feelswitch_ui.ps1`, the DiagFeel picture diagnostics).
None of them records video yet. What follows is what that could and could not add.

#### Smooth frames at a fixed time step {#vg_auto_frames}

- A standalone game started with `-game -benchmark -fps=60 -dumpmovie` renders every frame exactly 1/60 s of game time
  apart and writes one image per frame to the project's `Saved/Screenshots` folder. It runs slower than real time, so
  no frame is ever dropped. Verify on 5.6: the image format and folder, and that the demo levels' scripted input works
  outside the editor.
- FeelKit advances its recipes by the engine's frame time (`FApp::GetDeltaTime` in `FeelSubsystem.cpp`), which the
  fixed step sets to 1/60 s, so recipes play at their true speed in the frames. Only the cooldown check reads the wall
  clock, so a recipe with a Cooldown may be allowed or blocked differently during a slow capture.
- **No sound.** Frame dumping records no audio, and because the capture runs slower than real time, a separate audio
  recording (the audio mixer's Start Recording Output and Finish Recording Output nodes write a WAV) would not line up
  with the frames. Game feel needs its sound, so silent frames suit only close studies of a hit (slow motion,
  frame by frame), not the listing videos as a whole.
- **Movie Render Queue** renders Level Sequences, not a played game. Gameplay would first have to be recorded with Take
  Recorder, and FeelKit's camera modifier, post process and time dilation are applied at runtime by the camera manager
  and the world, which a recorded sequence does not replay. It is not a route for FeelKit footage.
- Turning an image sequence into a video file needs ffmpeg, which is not installed. Resolve can import numbered image
  sequences directly, so ffmpeg is needed only if the developer is to encode or check files.

#### Identical takes for the comparisons {#vg_auto_takes}

The most useful automation: a "video" mode for the four playthrough runners that plays the level in real time at 60
fps with a fixed camera, runs the same input twice (Feel Switch off, then on) and shows nothing but the game and the
badge. The person records it with OBS, so the sound is captured in real time. Both takes then have the same input to
the frame, which makes the off / on pairs in T1 fair by construction. Estimated work: about one to one and a half days
for the four levels, plus a check that the input stays in sync with the frame rate on this machine.

#### Split between the developer and the person recording {#vg_auto_split}

{widths: 24,14,31,31}
| Part | Who | Why | What the developer prepares |
|---|---|---|---|
| Narration in all videos | Person | Their own voice. A synthetic voice reads as generated, and YouTube asks for a disclosure of synthetic content | The narration text, word counts and speaking time per shot (printed by the build script) |
| Hands-on tutorial actions (V1 to V4) | Person | The viewer copies real pointer movement. A scripted editor changes values without menus opening or the pointer moving, which reads as generated | Project snapshots, the recipe copies, the editor layout and the starting state of each video, by script |
| Recipe editor loops and scrubbing | Person (OBS) | The editor preview runs in real time; there is no fixed-step capture of editor windows | The recipe and the window arrangement |
| Demo off / on pairs for T1 | Both | Input by script, recording by the person in real time for the sound | The runners' video mode (not built) |
| Silent slow-motion studies of a hit | Developer | Fixed-step frames are exact | A frame-dump runner (not built, about one day) |
| Overlays, title and end cards, thumbnails | Developer | Static graphics at a known specification, drawn the same way as the manual's cover and figures | PNG files at the timeline's resolution |
| Caption text, chapters, descriptions | Developer | Text taken from the scripts | Text files for pasting into YouTube Studio |
| Checks of the exported files | Developer, after ffmpeg is installed | Resolution, frame rate, file size, loudness (ffprobe and the ebur128 filter) | A check script (not built) |

**In short:** of about 33 finished minutes, automation can supply the input for roughly one minute of listing
footage, a few seconds of silent slow-motion material, and every static graphic and text. About 90% of the running
time is hands-on work with narration, which the person records. Automation saves most of its time in preparation:
exact starting states, overlays and text, rather than in footage.
