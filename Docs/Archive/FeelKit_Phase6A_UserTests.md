# FeelKit Phase 6A: what is left to check

Updated 2026-09-18.

**Phase 6A checklist: passed.** You ran every check: T1 to T5, part A (sample pack, materials, Combo accumulator, 38 library recipes) and part B (B1 read-only library, B2 the preference, B3 tiles, B4 spot check in the game, B5 the Recipe Browser). Everything found along the way was fixed and is in the project log (I-033 Mute, I-034 library recipes not opening, I-035 recipe editor disabled, the label and icon fixes).

**Left: one short look check (about 10 minutes).** After your test pass, three windows were restyled to look like their Unreal counterparts (decisions D-049 and D-050). You approved the recipe editor from pictures; the other three have only been checked as pictures. The build is done; just open the project.

---

## C1 The Recipe Browser (counterpart: the Content Browser)

1. **Tools** > **FeelKit Recipe Browser**.
2. **Expected:** on the left, collapsible sections **Source**, **Feeling**, **Genre** and **Affects** (click a section title to fold it). Each feeling has a colored bar and a count.
3. **Expected:** in the middle, tiles like the Content Browser's: a small timeline picture, an orange line under it, the name, and "Feel Recipe (Library)" or "Feel Recipe" in small grey text. At the bottom: "60 items" (your count may differ).
4. Click a tile once. **Expected:** its name area turns blue, the bottom says "(1 selected)", and on the right a **Recipe** section lists Name, Description, Feeling, Genres, Affects, Tracks, Length and Source. The **Use** button is blue.
5. Rest the mouse on another tile. **Expected:** it plays in the preview, as before.
6. Tell me if anything looks out of place next to the Content Browser.

## C2 The FeelKit Debugger (counterpart: the Outliner)

1. Press **Play** (PIE). **Tools** > **FeelKit Debugger**.
2. Trigger a few recipes in the game (your keys from B4).
3. **Expected:** a list with column headers **Name, Target, Time, Value, Details, Replay**. Under the level (world icon): a **Playing** folder with the recipes that are running, an **Accumulators** folder, and **Player 0 comfort** with one row per comfort group (grey while the value is 1.00) and **Controller vibration**.
4. **Expected:** after a recipe ends it appears under **Recent plays** with a small play button in the **Replay** column. Double-click the row or press that button: the recipe editor opens and replays it.
5. Type part of a recipe name in the search box. **Expected:** only matching rows stay.
6. Stop PIE.

## C3 The Content Browser tiles

1. Open the Recipe Browser once (C1) so the recipes are loaded, then in the Content Browser go to **All** > **Plugins** > **FeelKit Content** > **Library** > **Impact**.
2. **Expected:** each tile is a small dark timeline, like a tiny Sequencer: the feeling's color as a thin bar on the left, one muted colored bar per track with a faint curve line, a padlock at the top right, and no text inside the picture (the name and type are under it).
3. Note: a recipe that has not been loaded yet can still show the picture saved inside the asset the last time it was saved (the older look). Loading it (opening the Recipe Browser does that) redraws it.

## C4 The recipe editor, live

1. Double-click `FR_Impact_HeavyHit`. **Expected:** the look you approved from the picture: Sequencer colors, eye and headphone icons, one label per bar, words next to the toolbar icons, no green bar in the preview.
2. **Passed (2026-09-18)** after one change you asked for: Play, Stop and Loop are back at the start of the top toolbar, in the same style as the other buttons, Loop is an on/off toggle, and the bottom bar is gone. Nothing to recheck unless you want to see it.

---

## After this

Tell me what looks wrong or out of place. When C1 to C4 are fine, Phase 6A is closed and Phase 6B (the demos) starts.
