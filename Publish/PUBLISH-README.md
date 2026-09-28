# FeelKit: Fab publishing package

Everything needed to put FeelKit on Fab as two listings: **FeelKit Lite** (free) and **FeelKit Pro** (paid). Prepared
and checked on 2026-09-26. The two editions are kept strictly apart: nothing in `Lite/` is used by `Pro/`, and the
other way round.

```
Publish/
  PUBLISH-README.md              this file
  Lite/
    01-Images/                   gallery in upload order, 01-Main.png is also the thumbnail
    02-Fab-Listing.md            every Fab field with its final value, and the text to paste
    03-Upload/UE5.6|UE5.7|UE5.8  FeelKit_Lite_1.0.0_UE5.x.zip, the file Fab downloads for that engine
  Pro/
    01-Images/
    02-Fab-Listing.md
    03-Upload/UE5.6|UE5.7|UE5.8  FeelKit_Pro_1.0.0_UE5.x.zip
```

## 1. What was checked

| Check | Result |
|---|---|
| Strict BuildPlugin (`-StrictIncludes`), both editions on UE 5.6, 5.7 and 5.8 | 6 of 6 BUILD SUCCESSFUL, 0 warnings |
| Each package loaded in a new project on its own engine (`Tools/Run/check_packages.ps1`) | 6 of 6 pass: plugin on, every asset under /FeelKit loads, every recipe track has its step, the comfort menu loads, no load warnings |
| Lite contains only Lite | 12 effects and the core nodes present; 0 of 29 Pro classes, 0 of 6 Pro nodes, Pro-only recipe fields hidden; no Niagara, Enhanced Input, GAS, demo content or JSON sources; 11 library recipes; 7 sample sounds, all of them used |
| Pro contains everything | 29 of 29 Pro classes, 6 of 6 Pro nodes, 38 library recipes, demo recipes, GAS add-on in Extras |
| Moving a project from Lite to Pro (`Tools/Run/check_upgrade.ps1`, UE 5.6 and 5.8) | A copied library recipe (7 tracks) and a recipe made from scratch in Lite load in Pro with every step and value |
| Automation tests on the development project | 118 of 118 pass; the 7 editor window tests pass |
| Upload zips (`Tools/Run/make_uploads.py`) | Plugin source only, no Binaries, Intermediate or tests; EngineVersion per engine; each zip re-read after writing |
| Gallery images (`Tools/Run/make_gallery.py`) | All 1920 x 1080, every file under 3 MB, Pro 3.7 MB and Lite 2.5 MB in total; every picture is a real capture of FeelKit or its demos |

Only Windows was built and tested (decision D-073); both listings say so.

## 2. MANUAL ACTION REQUIRED

Do these in order. Everything else is ready.

### 2.1 Put the six zips online

- **Pro: done (2026-09-26).** The three Pro zips are on the publisher's Synology share; the links are in the version
  table of `Pro/02-Fab-Listing.md`. Check once, in a private browser window, that each link downloads the zip without a
  login.
- **Lite: to do.** Upload the three zips in `Lite/03-Upload/` the same way and paste the links into the version table
  of `Lite/02-Fab-Listing.md`.

The Pro zips online were made before the support email changed, so their plugin descriptor still names the old
address. The listings and the manual use billoue4@gmail.com. The FabURL rebuild after Fab's review (2.4) replaces all
six zips anyway, and carries the new address.

### 2.2 Documentation: under construction

By the publisher's decision (2026-09-26), both descriptions say the manual is under construction and point to Discord
and email until it is published. When the PDF is online, replace the "Manual (PDF)" line under "Documentation and
support" with its link, and the "Documentation" sentence at the end of "Additional information" in the technical
details. Fab's rule 4.3.8 asks for documentation at launch, so the review may ask for it; the answer is the link once
it exists.

### 2.3 Create the two listings

1. In the Fab Publisher Portal, create a new listing for **FeelKit Pro** and fill every field from
   `Pro/02-Fab-Listing.md`, in the order of its table.
2. Upload the gallery from `Pro/01-Images/` in file-name order; set `01-Main.png` as the thumbnail.
3. Add the three Unreal Engine versions (Add new format > Unreal Engine) from the version table.
4. Do the same for **FeelKit Lite** from `Lite/02-Fab-Listing.md` and `Lite/01-Images/`.
5. Check that your Publisher Profile lists the support channels: Discord https://discord.gg/AtJ6RdwaxA and
   billoue4@gmail.com (Fab 1.1.b).
6. In each Unreal Engine version's **Technical details**, choose **This product uses third party software**: the sample
   sounds are CC0 sounds by Kenney (and artisticdude in Pro), and Fab counts sounds from other sources as third-party
   software. `Credits.md` inside each zip lists every file with its source. The other technical fields are in the
   listing files, under "Technical details".

### 2.4 FabURL, after Fab's first review

Fab requires a `FabURL` in the plugin descriptor (Fab 4.3.6.c). The link only exists for the live product; the ID shown
while a listing is a draft is a different one. So the first submission goes without it, and Fab's review sends the
correct link. Then:

1. Send me the two links, or run these yourself with the editor closed:
   ```
   powershell -File B:\NewUE5Project\Tools\Run\package_all.ps1 -FabUrlLite "<Lite link from Fab>" -FabUrlPro "<Pro link from Fab>"
   python B:\NewUE5Project\Tools\Run\make_uploads.py
   ```
2. Replace the six zips online with the new ones and resubmit.

### 2.5 After launch

- No sale or discount in the first 30 days (Fab 1.7.1.c).
- Watch Discord and the email address for questions; the FAQ answers the ones expected first.
- Add the two listing videos from the Video Guide to the galleries when they are recorded (second place in each gallery).

## 3. Decisions taken for the listings

| Topic | Decision | Why |
|---|---|---|
| Category | Tools & Plugins > Gameplay Features | Where the closest game feel plugins on Fab are listed (checked on fab.com, 2026-09-26) |
| Titles | "FeelKit Pro: Game Feel, Hitstop and Screen Shake on a Timeline"; "FeelKit Lite (Free): ..." | The search terms from D-082 up front, no "juice" |
| Price | Pro USD 34.99 Personal, 64.99 Professional; Lite free | D-090 |
| Tags | 25 per listing, each an existing Fab tag (one lowercase word, checked against Fab's search on 2026-09-26); `freeze` stands for hitstop; no "juice" or "juicy" | Fab's tag picker accepts only tags it already has (D-107); D-082 |
| Target platforms | Windows, Mac, Linux, Android, iOS, with "only Windows built and tested" in the description | D-073, and the plugin descriptor allows exactly these |
| Forum post | No | A forum thread becomes an official support channel (Fab 1.2.b); support stays on Discord and email |
| Disallow use by Generative AI Programs | Ticked | It only adds Fab's NoAI tag against data collection; buyers are not restricted |
| Use of generative AI tools | Yes | D-079 |
| Promotional content | No | The field is for paid advertising inside a product; FeelKit has none |
| Lite contents | Only what Lite code and content use: 7 of the 43 sample sounds, no Pro materials, no recipe JSON (the JSON import is a Pro tool) | "No unnecessary files"; Lite's `Credits.md` lists only its 7 sounds |
| Demos on the Lite page | Linked, with "built with FeelKit Pro" | They show what FeelKit does; saying which edition made them keeps it accurate |
| Gallery pictures | Real captures framed on a dark background with one headline each; Lite pictures taken from a project with the Lite package, so they show only Lite | Fab 1.8.6.a: media must show the real product |

## 4. Rebuilding any of this

| What | Command (editor closed) |
|---|---|
| Six packages | `Tools/Run/package_all.ps1` (optionally `-Editions Lite` or `-Editions Pro`) |
| Check the packages | `Tools/Run/check_packages.ps1`, then `Tools/Run/check_upgrade.ps1` |
| Upload zips | `python Tools/Run/make_uploads.py` |
| Demo pictures | `Tools/Run/run_gallery.ps1` |
| Pro editor pictures | `DiagFeel.GalleryEditorShots` in GameFeelDev (rendering session) |
| Lite editor pictures | `Tools/Run/lite_gallery_shots.ps1` (after the Lite packages) |
| Gallery images | `python Tools/Run/make_gallery.py` |

The edition split itself is `Tools/Run/make_edition.py`: Pro-only code in the development source sits between
`// FEELKIT_PRO_BEGIN` and `// FEELKIT_PRO_END`, and Lite is made from a copy with those parts removed.
