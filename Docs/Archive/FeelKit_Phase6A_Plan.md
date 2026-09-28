# FeelKit Phase 6A Implementation Plan

> Executed inline in this session, task by task. Steps use checkboxes. Design: `Docs/FeelKit_Phase6A_ContentTools_Design.md`. Every task ends with a build, the tests named in it, and a project log entry.

**Goal:** Recipe browser with live preview, Recipe from Template, read-only library recipes, Content Browser recipe tiles, and a tested library of 38 recipes as JSON with a CC0 sample pack plan.

**Architecture:** Library metadata lives on `UFeelRecipe` as editor-only data and is published as asset registry tags (FeelCore). Everything else is FeelEditor: plain, testable logic classes (filter, thumbnail layout, library helpers, template copy) with thin Slate/renderer layers on top. Read-only protection uses Unreal's writable-folder permission list (blocks saving, renaming and deleting) plus a read-only mode in the recipe editor state.

**Tech stack:** UE 5.6 C++, Slate, Asset Registry, AssetTools, ToolMenus, UThumbnailRenderer, UE automation tests.

## Global constraints
- UE 5.6 floor; no APIs after 5.6 without version macros.
- Never create or edit `.uasset` / `.umap`; the user creates assets from checklists.
- FeelCore depends only on Core, CoreUObject, Engine, InputCore, Slate, SlateCore, UMG, GameplayTags, DeveloperSettings.
- FeelEditor is Editor-only and never referenced by runtime modules.
- Zero compiler warnings; `Feel` prefix on public types.
- Builds need the editor closed; before any handoff: editor target, Game Development and Shipping targets, strict BuildPlugin to `B:/NewUE5Project/Build/FKPkg4`, all FeelKit tests.
- Tooltips and descriptions are general (no single use case framing), no AI wording.
- No git: "commit" steps are replaced by project log entries.
- User checklists spell out every setup step.

## Findings from reading the code (2026-09-17)
- `UFeelRecipe` already has two unused runtime properties, `Category` (FGameplayTag, `FeelRecipe` categories) and `Description` (FText). No code or asset uses them. Task 1 removes `Category` and moves `Description` into the editor-only Library category (logged as a design detail, D-044).
- The recipe JSON export writes every non-transient property of the recipe object, so the library fields travel inside the `recipe` object automatically. The design's separate `library` object is not needed; `BasedOn` is excluded explicitly (D-044).
- Unreal's `FAssetToolsModule::GetWritableFolderPermissionList()` is checked by the editor before saving a package and by the Content Browser for rename, delete and move. Read-only folders stay visible by default. Task 3 adds `/FeelKit/Library/` as a deny entry while Allow Library Editing is off.
- Channel colors already live in `FeelEditor/Private/FeelEditorColors.h` (`GetChannelColor`), shared by the timeline and intensity graph; the thumbnail reuses it.

---

### Task 1: Library metadata and registry tags (FeelCore)

**Files:**
- Modify: `FeelCore/Public/FeelTags.h`, `FeelCore/Private/FeelTags.cpp` (feeling and genre tags)
- Modify: `FeelCore/Public/FeelRecipe.h`, `FeelCore/Private/FeelRecipe.cpp`
- Test: `FeelCore/Private/Tests/FeelLibraryMetadataTests.cpp` (new)

**Interfaces produced:**
- Tags `FeelTags::Feeling_Impact` ... `Feeling_Interface`, `FeelTags::Genre_Action` ... `Genre_Vehicle`; roots `Feel.Feeling`, `Feel.Genre`.
- On `UFeelRecipe` (editor-only data, category "Library"): `FGameplayTag Feeling` (meta Categories "Feel.Feeling"), `FGameplayTagContainer Genres` (meta Categories "Feel.Genre"), `FText Description` (MultiLine), `FSoftObjectPath BasedOn` (VisibleAnywhere).
- `void UFeelRecipe::GatherChannels(TSet<FGameplayTag>& OutChannels) const` (runtime, not editor-only): enabled tracks' channels (falling back to the step default channel); for Play Recipe steps the inner recipe's channels (one level); for Random Choice the default channels of every option step.
- Registry tag names as static `FName`s on `UFeelRecipe`: `FeelingTagName` ("FeelFeeling"), `GenresTagName` ("FeelGenres"), `DescriptionTagName` ("FeelDescription"), `ChannelsTagName` ("FeelChannels"), `TrackCountTagName` ("FeelTrackCount"), `LengthTagName` ("FeelLength"), `SustainedTagName` ("FeelSustained"). Feeling, genres, description and channels written only `WITH_EDITORONLY_DATA`; channels, track count, length, sustained always.
- `CurrentSchemaVersion` becomes 2 ("2: library metadata").

- [ ] Write `FeelKit.Library.Metadata` test: a transient recipe with a shake, a muted flash, a Play Recipe (inner recipe with a sound track) and a Random Choice (flash + haptics) → `GatherChannels` returns shake, audio, flash (from the choice), haptics and not the muted flash's channel; registry tags from `GetAssetRegistryTags` contain the expected feeling, genre list, description, channel list, track count, length and sustained values.
- [ ] Build (expect a compile failure: members missing), implement tags, properties, `GatherChannels`, registry tags; remove `Category`.
- [ ] Build, run `FeelKit` tests: all pass (97 + new).
- [ ] Log in the project log (Phase 6A task 1).

### Task 2: JSON library fields

**Files:**
- Modify: `FeelEditor/Private/FeelRecipeJson.cpp`
- Test: `FeelEditor/Private/Tests/FeelRecipeJsonTests.cpp` (extend)

**Interfaces:** `FFeelRecipeJson::Export/Import` unchanged in signature. Export omits `BasedOn`; Import never copies `BasedOn` (keeps the target's value); files with schema 0-2 import.

- [ ] Extend `FeelKit.Editor.RecipeJson`: feeling, genres and description round trip; `BasedOn` is not in the exported text and a target's `BasedOn` survives import; a schema 1 file without library fields imports and leaves the fields empty.
- [ ] Implement (skip `BasedOn` in export by removing the field from the JSON object; skip it in the property copy loop).
- [ ] Build, run tests, log.

### Task 3: Library detection and read-only protection

**Files:**
- Create: `FeelEditor/Private/FeelLibrary.h`, `FeelEditor/Private/FeelLibrary.cpp`
- Modify: `FeelEditor/Private/FeelEditorSettings.h` (`bAllowLibraryEditing`), `FeelEditorModule.cpp` (startup/shutdown), `FeelRecipeEditorState.h/.cpp` (`IsReadOnly()`), `FeelRecipeEditorToolkit.cpp` (banner, details read-only), `SFeelTimeline.cpp` (edits disabled), `FeelRecipeJson.cpp` (import disabled), `FeelEditor.Build.cs` (AssetTools)
- Test: `FeelEditor/Private/Tests/FeelLibraryTests.cpp` (new)

**Interfaces produced (namespace `FeelLibrary`):**
- `const TCHAR* LibraryRoot` = `/FeelKit/Library/`
- `bool IsLibraryPath(FStringView PackageName)` (exact root prefix; `/FeelKitLibrary`, `/Game/FeelKit/Library` are not library)
- `bool IsLibraryRecipe(const UFeelRecipe* Recipe)`, `bool IsLibraryAsset(const FAssetData& Asset)`
- `bool IsLibraryEditingAllowed()` (reads `UFeelEditorSettings::bAllowLibraryEditing`)
- `bool IsReadOnly(const UFeelRecipe* Recipe)` = library recipe and editing not allowed
- `void Startup()` / `void Shutdown()`: add or remove the `/FeelKit/Library/` deny entry (owner name `FeelKitLibrary`) in the writable-folder permission list; re-applied when the setting changes (`UFeelEditorSettings::PostEditChangeProperty` calls `FeelLibrary::ApplyWritePermission()`).
- `FFeelRecipeEditorState::IsReadOnly() const`; every editing method (`AddTrack`, `DeleteTrack`, `DuplicateTrack`, `MoveTrack`, `ToggleMute`, `ToggleSolo`, `PasteTracks`, curve key methods, `CreateTrackFromSound`) returns early when read-only.

- [ ] Write `FeelKit.Library.ReadOnly` test: path cases; with the setting off a transient recipe renamed into a `/FeelKit/Library/Impact/` package is read-only, `FFeelRecipeEditorState` edits leave track count unchanged, and the writable-folder list rejects the package; a `/Game/` recipe is editable; with the setting on everything is editable and the deny entry is gone; the setting is restored afterwards.
- [ ] Implement helpers, setting, permission entry, state guards.
- [ ] Toolkit: when read-only, a banner row above the tabs' content ("Library recipe (read-only). Copy it to your project to edit." + Copy to Project button, wired in Task 4) and `DetailsView->SetIsPropertyEditingEnabledDelegate` returning false; timeline drag/resize/menus check `State->IsReadOnly()`.
- [ ] JSON import menu entry: `CanExecuteAction` false for read-only recipes, tooltip explains the preference.
- [ ] Build, run tests, log.

### Task 4: Recipe from Template (copy to project)

**Files:**
- Modify: `FeelEditor/Private/FeelLibrary.h/.cpp`
- Modify: `FeelEditorModule.cpp` (Content Browser folder menu entries), `FeelRecipeEditorToolkit.cpp` (banner button)
- Test: `FeelEditor/Private/Tests/FeelLibraryTests.cpp` (extend)

**Interfaces produced:**
- `FString FeelLibrary::SuggestCopyName(const FString& LibraryAssetName)`: `FR_Impact_HeavyHit` → `HeavyHit`; names without the pattern are returned unchanged.
- `UFeelRecipe* FeelLibrary::DuplicateRecipe(const UFeelRecipe& Source, const FString& PackagePath, const FString& AssetName)`: creates the asset through `IAssetTools::DuplicateAsset`, sets `BasedOn` to the source path, marks dirty; returns null on failure.
- `UFeelRecipe* FeelLibrary::CopyToProjectWithDialog(const UFeelRecipe& Source, const FString& DefaultFolder)`: Save Asset As dialog (`IContentBrowserSingleton::CreateSaveAssetDialog`), then `DuplicateRecipe`, then opens the asset editor. Null when cancelled.
- `FString FeelLibrary::GetDefaultCopyFolder()`: Content Browser current path, else `/Game`.

- [ ] Extend tests (`FeelKit.Library.TemplateCopy`): suggested names; `DuplicateRecipe` of a transient library-path recipe into `/Game/FeelKitTests_Temp/` gives a new asset with equal track count, different step objects owned by the copy, same feeling/genres/description, `BasedOn` set, a referenced sound object unchanged; editing the copy leaves the source unchanged; temp assets deleted at the end.
- [ ] Implement; Content Browser folder context menu (`ContentBrowser.FolderContextMenu` and `ContentBrowser.AddNewContextMenu`) section FeelKit: **Recipe from Template...** (opens the browser picker from Task 7; until Task 7 lands, entry added in Task 7) and **Browse Recipes...**.
- [ ] Banner Copy to Project button calls `CopyToProjectWithDialog`.
- [ ] Build, run tests, log.

### Task 5: Content Browser tile and tooltip

**Files:**
- Create: `FeelEditor/Private/FeelThumbnailLayout.h/.cpp`, `FeelEditor/Private/FeelRecipeThumbnailRenderer.h/.cpp`
- Modify: `FeelEditor/Private/FeelEditorColors.h` (`GetFeelingColor`), `FeelEditorModule.cpp` (register/unregister renderer), `FeelCore/Private/FeelRecipe.cpp` (`GetAssetRegistryTagMetadata` display names) or `AssetDefinition_FeelRecipe` as appropriate
- Test: `FeelEditor/Private/Tests/FeelThumbnailLayoutTests.cpp` (new)

**Interfaces produced:**
- `struct FFeelThumbnailBar { FLinearColor Color; float StartX; float EndX; TArray<float, TInlineAllocator<16>> Heights; bool bFaded; }` (X in 0..1 of the drawing width; heights 0..1 sampled across the bar)
- `struct FFeelThumbnailLayout { TArray<FFeelThumbnailBar> Bars; int32 HiddenTrackCount; bool bHasSustain; float SustainStartX; float SustainEndX; float Length; FLinearColor FeelingColor; bool bLibrary; }`
- `FFeelThumbnailLayout FeelThumbnailLayout::Build(const UFeelRecipe& Recipe, int32 MaxBars = 8, int32 SamplesPerBar = 16)`
- `FLinearColor FeelEditorColors::GetFeelingColor(const FGameplayTag& Feeling)`
- `UFeelRecipeThumbnailRenderer : UThumbnailRenderer` (`Draw` uses the layout; `CanVisualizeAsset` true for recipes)

- [ ] Write `FeelKit.Editor.ThumbnailLayout` test: two tracks (0-0.5 s constant, 0.25-1.0 s ramp down) in a 1 s recipe → bar X ranges 0-0.5 and 0.25-1.0, first bar heights all 1, second bar first height 1 and last 0; a muted track is faded; 10 tracks → 8 bars and HiddenTrackCount 2; sustain region X positions; length 1.0.
- [ ] Implement layout, colors, renderer; register in module startup, unregister in shutdown.
- [ ] Registry tag display names so the tooltip reads Description, Feeling, Genres, Channels, Length.
- [ ] Build, run tests, log.

### Task 6: Browser filter logic

**Files:**
- Create: `FeelEditor/Private/FeelRecipeFilter.h/.cpp`
- Test: `FeelEditor/Private/Tests/FeelRecipeFilterTests.cpp` (new)

**Interfaces produced:**
- `enum class EFeelRecipeSource : uint8 { Both, Library, Project };`
- `struct FFeelRecipeFilterState { EFeelRecipeSource Source; TSet<FName> Feelings; TSet<FName> Genres; TSet<FName> ChannelGroups; FString Search; };` (channel groups: `Feel.Camera`, `Feel.Screen`, `Feel.Actor`, `Feel.Time`, `Feel.Audio`, `Feel.Haptics`, `Feel.UI`, `Feel.Spawn`; a channel matches its group by tag prefix)
- `struct FFeelRecipeEntry { FAssetData Asset; FName Feeling; TArray<FName> Genres; TArray<FName> Channels; FString Description; TArray<FString> Parameters; float Length; bool bLibrary; }`
- `FFeelRecipeEntry FeelRecipeFilter::MakeEntry(const FAssetData& Asset)` (reads registry tags only)
- `TArray<FFeelRecipeEntry> FeelRecipeFilter::Apply(const TArray<FFeelRecipeEntry>& All, const FFeelRecipeFilterState& State)` (sorted by feeling then name)
- `TMap<FName, int32> FeelRecipeFilter::CountByFeeling(const TArray<FFeelRecipeEntry>& All, const FFeelRecipeFilterState& State)` (counts with every filter except Feeling applied)

- [ ] Write `FeelKit.Editor.RecipeFilter` test with hand-built entries (library and project; feelings; genres; channels; descriptions; parameter names): source filter; feeling OR within group; feeling AND genre; channel group prefix match; search over name, description and parameter names (case-insensitive); sorting; counts ignore the feeling selection but respect the others; `MakeEntry` reads tags from an `FAssetData` built with tag values.
- [ ] Implement; build; run tests; log.

### Task 7: Recipe browser tab and picker

**Files:**
- Create: `FeelEditor/Private/SFeelRecipeBrowser.h/.cpp`
- Modify: `FeelEditorModule.cpp` (tab registration, Tools menu entry, Content Browser entries from Task 4), `SFeelPreviewViewport` only if a change is needed to host it outside the toolkit
- Test: `FeelEditor/Private/Tests/FeelRecipeBrowserUITests.cpp` (rendering session, skipped under -nullrhi)

**Interfaces produced:**
- `SFeelRecipeBrowser` args: `bPickerMode` (Library only, button Create), `OnRecipePicked` (picker callback); `static void RegisterTab()`, `static void UnregisterTab()`, `static const FName TabName`; `static void OpenPicker(const FString& TargetFolder)` (modal window).
- Preview: an `FFeelRecipeEditorState` plus `SFeelPreviewViewport` rebuilt for the hovered/selected recipe (0.25 s hover delay; hover plays once; selection loops); mute toggle (sound steps skipped while muted through the state's preview scene); parameter sliders built from `Recipe->Parameters` calling `SetPreviewParameterValue`; comfort preset combo calling `SetPreviewComfortPreset`.
- Row widget draws the mini timeline strip with `FeelThumbnailLayout::Build` (loads only when the row is visible; unloaded rows show the feeling chip only).

- [ ] Implement filters column, list, preview column, actions (Use, Open, Show in Content Browser; double-click rule), asset registry refresh (added, removed, renamed, updated).
- [ ] Register Tools > FeelKit Recipe Browser, Content Browser Browse Recipes... and Recipe from Template... (picker).
- [ ] UI test `FeelKit.Editor.RecipeBrowserUI`: tab spawns, lists transient test entries injected through a test hook, simulated hover selects an entry and the preview state reports playing.
- [ ] Build, run tests (including the rendering session UI tests), log.

### Task 8: The 38 library recipes as JSON

**Files:**
- Create: `Plugins/FeelKit/Library/<Feeling>/FR_<Feeling>_<Name>.json` (38 files)
- Create: `FeelEditor/Private/Tests/FeelLibraryDataTests.cpp`
- Modify: `Plugins/FeelKit/Config/FilterPlugin.ini` (ship `/Library/...`, `Credits.md`)

**Interfaces consumed:** JSON format from Task 2; sample asset paths `/FeelKit/Samples/Sounds/S_FK_<Name>.S_FK_<Name>`, `/FeelKit/Samples/Materials/M_FK_Decal_Scorch.M_FK_Decal_Scorch`, `/FeelKit/Samples/Materials/M_FK_PP_Pulse.M_FK_PP_Pulse`.

- [ ] Write `FeelKit.Library.Data` test: finds every JSON under the plugin's `Library` folder (expects 38); for each: imports into a transient recipe, file name equals the `name` field, folder equals the feeling suffix, feeling set, at least one genre, description not empty, parameters only from {Damage, FallSpeed, Health, Fear, Charge, Distance} and accumulator only `Combo`, data validation has no errors except missing sample assets (reported as a separate count until the sample pack exists), evaluation over the whole length at each parameter's min, default and max produces finite output.
- [ ] Author the recipes feeling by feeling (Impact, Weight, Power, Speed, Reward, Danger, Dread, Denial, Interface), running the data test after each feeling.
- [ ] Log per feeling.

### Task 9: Sample pack sources, credits and checklists

**Files:**
- Create: `Plugins/FeelKit/Credits.md`
- Create: `Docs/FeelKit_Phase6A_UserTests.md` (tools checklist and content checklist)

- [ ] Research CC0 sources for the 15 sounds (web search and the built-in browser; license read on each file's own page). Record source page, author, license per sound in `Credits.md` and in the project log. Downloads are left to the user.
- [ ] Write the tools checklist (browser, template, read-only, tiles, preference) and the content checklist: enable Show Plugin Content; create folders; download each sound (link, file to pick); import to the exact folder and rename; create both materials node by node; add the `Combo` accumulator in Project Settings; turn on Allow Library Editing; for each of the 38 recipes: create a Feel Recipe with the exact name in the exact folder, Import from JSON with the exact file; turn the preference off; PIE spot check per feeling with exact Blueprint steps.
- [ ] Log.

### Task 10: Verification and handoff

- [ ] Editor closed; build editor target; all `FeelKit` tests; rendering-session UI tests; Game Development and Shipping targets; strict BuildPlugin; plugin DLLs newer than sources.
- [ ] Update project log (status, test counts), `CLAUDE.md` status, memory.
- [ ] Hand the user the tools checklist first, then the content checklist.
