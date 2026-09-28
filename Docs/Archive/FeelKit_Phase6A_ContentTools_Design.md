# FeelKit Phase 6A: Content Tools and Recipe Library (design)

| Field | Value |
|---|---|
| Date | 2026-09-17 |
| Status | Design agreed section by section with the user; waiting for review of this document |
| Phase | 6 (Content and launch), part A. Parts B-E (demos, documentation, launch readiness) get their own designs |
| Requirements | TL-001, TL-005, TL-006, CT-001, CT-004, CT-005 |
| Decisions | D-038 to D-043 (project log) |

## 0. Scope

**In scope**
- Library metadata on recipes (feeling, genres, description) and registry tags for fast filtering.
- Recipe browser tab with filters and live hover preview (TL-001).
- Recipe from Template and read-only library recipes (TL-006).
- Content Browser thumbnail and tooltip for recipes (TL-005).
- A library of 38 recipes written as JSON, imported by the user into plugin content (CT-001).
- A CC0 sample pack of 15 sounds and 2 materials with a credits file (CT-004, CT-005).
- Automated tests, a user checklist and a content import checklist.

**Out of scope for 6A**
- Which recipes go into the free Lite version (launch readiness part).
- Demo maps, videos, tutorial, trailer (demos part).
- Documentation pages (documentation part), except the checklists and credits file this part needs.
- Browser favorites, ratings, metadata editing in the browser, batch operations.
- Animated Content Browser tiles and pre-rendered GIFs.

## 1. Data and library layout

### 1.1 Recipe metadata (FeelCore, editor-only)
New **Library** category on `UFeelRecipe`, inside `WITH_EDITORONLY_DATA` (stripped from cooked builds, like `PreviewMesh`):

| Property | Type | Notes |
|---|---|---|
| `Feeling` | `FGameplayTag` | One tag under `Feel.Feeling`; empty allowed |
| `Genres` | `FGameplayTagContainer` | Tags under `Feel.Genre` |
| `Description` | `FText` | One or two sentences, general wording (no single use case framing) |
| `BasedOn` | `FSoftObjectPath` | Set by Recipe from Template; shown read-only |

Native gameplay tags defined in FeelCore (`FeelTags`):
- Feelings: `Feel.Feeling.Impact`, `Feel.Feeling.Weight`, `Feel.Feeling.Power`, `Feel.Feeling.Speed`, `Feel.Feeling.Reward`, `Feel.Feeling.Danger`, `Feel.Feeling.Dread`, `Feel.Feeling.Denial`, `Feel.Feeling.Interface`.
- Genres: `Feel.Genre.Action`, `Feel.Genre.Shooter`, `Feel.Genre.Platformer`, `Feel.Genre.Horror`, `Feel.Genre.UI`, `Feel.Genre.Vehicle`.

Studios can add tags under the same roots in Project Settings > Gameplay Tags; the browser lists every tag found under the roots.

### 1.2 Asset registry tags
`UFeelRecipe::GetAssetRegistryTags` publishes, next to the existing `FeelParameters` tag (editor only, since the metadata is editor-only):

| Tag | Content |
|---|---|
| `FeelFeeling` | Feeling tag name |
| `FeelGenres` | Comma-separated genre tag names |
| `FeelDescription` | Description text (shown in the tooltip) |
| `FeelChannels` | Comma-separated channel tags computed from enabled tracks, including steps inside Play Recipe and Random Choice (one level of nesting is resolved through their recipe or options) |
| `FeelTrackCount` | Number of tracks |
| `FeelLength` | Recipe length in seconds |
| `FeelSustained` | `True` or `False` |

Channels are always computed, so channel filters work without manual tagging and stay correct when tracks change.

### 1.3 Library location and names
- Recipes: `/FeelKit/Library/<Feeling>/FR_<Feeling>_<Name>` (plugin content, `Plugins/FeelKit/Content/Library/...`).
- Sample sounds: `/FeelKit/Samples/Sounds/S_FK_<Name>`.
- Sample materials: `/FeelKit/Samples/Materials/M_FK_Decal_Scorch`, `M_FK_PP_Pulse`.
- Credits: `Plugins/FeelKit/Credits.md`.
- A recipe is a **library recipe** when its package path starts with `/FeelKit/Library/`. One helper (`FeelLibrary::IsLibraryRecipe`) decides this for all features.

### 1.4 JSON source
- Source files: `Plugins/FeelKit/Library/<Feeling>/FR_<Feeling>_<Name>.json` (outside `Content`, so they are not cooked). They are the editable source for later changes.
- The recipe JSON format gains `library: { feeling, genres, description }`. `BasedOn` is not exported. Files without the `library` object still import (older files). The schema version is raised by one.
- The plugin's `FilterPlugin.ini` includes `/Library/...` and `Credits.md` so the JSON sources and credits ship with the plugin.

### 1.5 Engine versions
Library and sample assets are created and saved in UE 5.6 only (C-001c). 5.7 and 5.8 load them unchanged.

## 2. Recipe browser tab (TL-001)

**Entry points:** Tools > **FeelKit Recipe Browser** (nomad tab, same registration pattern as the FeelKit Debugger); Content Browser right-click > FeelKit > **Browse Recipes...**.

**Layout (three columns):**
1. **Filters:** Source (Library, Project, Both; default Both); Feeling list with counts; Genre checkboxes; Channel checkboxes grouped as Camera, Screen, Actor, Time, Audio, Haptics, UI, Spawn; search box over name, description and parameter names. Groups combine with AND; values inside a group combine with OR.
2. **Recipe list:** name, feeling color chip, one-line description, length, mini timeline strip (same drawing code as the thumbnail, section 4). Sorted by feeling, then name. Lock icon on library recipes.
3. **Preview:** the recipe editor's preview viewport (shared preview state, not a copy). Hover a row for 0.25 s: plays once. Select a row: loops. Audio audible, mute toggle. Parameter sliders when the recipe has parameters. Comfort preset dropdown. Below the viewport: full description, genres, channels, track count.

**Actions:** **Use** (library: copy to project, section 3), **Open** (recipe editor; library opens read-only), **Show in Content Browser**. Double-click: Use for library recipes, Open for project recipes.

**Data source:** the asset registry only (no loading to list or filter). The recipe under preview is loaded when hovered or selected. The list refreshes on asset added, removed, renamed and updated events.

**Structure:** filter logic lives in a plain, testable class (`FFeelRecipeFilter`: input asset data list + filter state → result list and per-feeling counts); the Slate widget (`SFeelRecipeBrowser`) only displays.

## 3. Templates and read-only library recipes (TL-006)

**Recipe from Template**
- Content Browser right-click empty space > FeelKit > **Recipe from Template...** opens the browser as a modal picker (Source fixed to Library, button **Create**).
- The browser tab's **Use** and the read-only banner's **Copy to Project** use the same flow.
- Flow: Unreal's standard Save Asset As dialog; default folder = the right-clicked folder, else the Content Browser's current folder; default name = library name without `FR_<Feeling>_` (`FR_Impact_HeavyHit` → `HeavyHit`).
- The new asset is a full duplicate (tracks and step objects duplicated, never shared), keeps Feeling, Genres and Description, sets `BasedOn` to the library recipe, keeps references to `/FeelKit/Samples` assets, and opens in the recipe editor.
- Cancelling the dialog creates nothing.

**Read-only protection** (when Allow Library Editing is off)
- Recipe editor on a library recipe: banner "Library recipe (read-only). Copy it to your project to edit." with **Copy to Project**.
- Disabled: Details editing, timeline drag/resize, curve editing, add/delete/reorder/mute/solo tracks, paste, Import from JSON (context menu entry disabled with a tooltip explaining why).
- Available: preview, scrubbing, parameter sliders, comfort presets, Play in PIE, Capture GIF, Export to JSON, Copy Tracks, Recent Plays replay.
- Saving a modified library recipe (for example changed by a script) is blocked with a notification naming the preference.

**Allow Library Editing:** per-user editor preference (Editor Preferences > Plugins > FeelKit, `UFeelEditorUserSettings`, saved per machine, default off). On: no banner, no restrictions. Used by the developer and the user while authoring the library; buyers never need it.

## 4. Content Browser tile and tooltip (TL-005)

**Thumbnail renderer** (`UFeelRecipeThumbnailRenderer`, FeelEditor, registered for `UFeelRecipe`; canvas drawing only, no 3D scene, not realtime):
- Dark background; top band in the feeling color (neutral grey without a feeling).
- One bar per track at its start time and length; height follows the intensity curve (sampled); color = the channel color used by the timeline (moved into one shared helper so both use identical colors).
- Muted or disabled tracks faded. More than 8 tracks: the first 8 plus a "+N" marker.
- Sustain region shaded for sustained recipes.
- Length label bottom right ("0.6 s"). Lock icon on library recipes.
- Layout is computed by a plain function (`FeelThumbnailLayout::Build(recipe, size)` → bars and markers) that the renderer draws and tests check.

**Tooltip:** registry tags `FeelDescription`, `FeelFeeling`, `FeelGenres`, `FeelChannels`, `FeelLength` are marked for display so the Content Browser tooltip shows them with readable names.

## 5. Recipe library and sample pack (CT-001, CT-004, CT-005)

### 5.1 Recipes (38)

| Feeling | Recipes (`FR_<Feeling>_...`) |
|---|---|
| Impact | `LightHit`, `HeavyHit`, `CriticalHit`, `ScalableHit` (Damage 0-100 light to heavy), `BulletImpact` (decal + sound) |
| Weight | `Land` (FallSpeed scaled), `Stomp`, `HeavyFootstep`, `Slam` |
| Power | `ChargeUp` (sustained, releases into a tail), `ChargedRelease`, `Explosion` (Distance falloff, push away from the blast), `AbilityCast` |
| Speed | `Dash` (camera punch along the dash direction), `Boost` (sustained FOV and chromatic aberration), `Whoosh`, `SprintStart` |
| Reward | `Pickup`, `ComboStep` (escalates with the Combo accumulator), `LevelUp`, `KillConfirm` |
| Danger | `DamageTaken`, `DirectionalDamage` (punch from the hit direction), `LowHealth` (sustained, Health 0-1), `Alarm` |
| Dread | `Heartbeat` (sustained, Fear 0-1 drives rate and haptics), `Unease` (sustained desaturate, vignette, low-pass), `JumpScare` (essential flash with a substitute), `FailingLight` (light flicker) |
| Denial | `Blocked`, `OutOfAmmo`, `Locked`, `WrongInput` (widget shake + buzz) |
| Interface | `ButtonHover`, `ButtonPress`, `Notification`, `ScoreTick` (Number Pop), `ScreenTransition` (fade) |

**Shared parameter names:** `Damage` (0-100), `FallSpeed` (cm/s), `Health` (0-1), `Fear` (0-1), `Charge` (0-1), accumulator `Combo`, built-in `Distance`. Recipes that use the `Combo` accumulator document that it must exist in Project Settings > FeelKit > Accumulators (the import checklist says how to add it).

Every recipe works with the sample pack; buyers can replace sounds and materials in their copies.

### 5.2 Sample pack
- Sounds (15, CC0 only): `S_FK_Hit_Light`, `S_FK_Hit_Heavy`, `S_FK_Hit_Crit`, `S_FK_Impact_Bullet`, `S_FK_Land_Thud`, `S_FK_Explosion`, `S_FK_Charge_Loop`, `S_FK_Whoosh`, `S_FK_Pickup`, `S_FK_LevelUp`, `S_FK_Heartbeat`, `S_FK_Alarm`, `S_FK_Denied`, `S_FK_UI_Hover`, `S_FK_UI_Click`.
- Materials (2): `M_FK_Decal_Scorch` (deferred decal, connected Base Color and Opacity nodes), `M_FK_PP_Pulse` (post process, scalar parameter `Weight`, Constant4Vector where scene color is 4 channels).
- `Credits.md`: every sound with file name, source page, author and license. Sources are proposed by the developer (candidates such as Kenney's CC0 audio packs), each license confirmed on the file's own page; the user approves and downloads (the developer does not download files).

### 5.3 Order of work
1. Code: sections 1-4, with tests; verified builds.
2. The 38 JSON files, proven by the library data test (section 6).
3. Sample sound sources proposed with pages and licenses.
4. User: downloads and imports sounds, creates the two materials, turns on Allow Library Editing, imports the 38 recipes, spot checks each feeling in PIE.

## 6. Testing and verification

**Automated (FeelKit suite)**
- Metadata: save/load round trip; registry tags for channels (including nested Play Recipe and Random Choice), track count, length, sustained; metadata not present in cooked data.
- Library detection: `/FeelKit/Library/...` is library; look-alike paths (`/FeelKitLibrary`, `/Game/FeelKit/Library`) are not.
- Read-only: preference off blocks timeline edits, JSON import and saving after an outside change on library recipes; project recipes unaffected; preference on allows everything.
- Template copy: independent duplicate in the chosen folder, suggested name, duplicated step objects, metadata kept, BasedOn set, sample references unchanged, original untouched after editing the copy.
- Filter logic: feeling, genre, channel, source and search combinations over test asset data; per-feeling counts.
- Thumbnail layout: bar positions and heights, "+N" overflow, sustain region.
- JSON: library fields round trip; files without `library` still import.
- Library data test: each of the 38 JSON files imports into a transient recipe without errors, passes data validation, has name/feeling/folder agreement, uses only the shared parameter names, and evaluates across its whole length at parameter minimum, default and maximum without errors.

**UI (rendering session, skipped under -nullrhi):** browser tab opens and lists test recipes; hovering starts preview playback; a library recipe opens with the banner and a disabled timeline.

**Builds before handoff:** editor target, Game Development and Shipping targets, strict BuildPlugin; 0 warnings; plugin binaries newer than sources.

**User checklists** (every setup step spelled out)
1. Tools checklist: browser filters, hover preview, sliders, Use; Recipe from Template; read-only banner and Copy to Project; Content Browser tiles and tooltips; Allow Library Editing.
2. Content checklist: sample sounds, the two materials, Combo accumulator, importing the 38 recipes, PIE spot check per feeling.
