# Fab requirements for FeelKit, verified 2026-09-23

Sources read on 2026-09-23:
- **Fab Technical Requirements** (fab.com/technical-requirements, "Last Updated: September 21, 2026"), sections 1 General and 4 Unreal Engine. Read in a browser; the page is rendered by script.
- **Asset File Format and Structure Requirements in Fab** (dev.epicgames.com/documentation/en-us/fab/asset-file-format-and-structure-requirements-in-fab): media sizes.
- **Publishing Assets for Sale or Free Download in Fab** (dev.epicgames.com/documentation/en-us/fab/publishing-assets-for-sale-or-free-download-in-fab): listing fields.
- **(Fab) NoAI meta tags and Created with AI self-declaration** (support.fab.com, Jan 24, 2025).

## Media

| Requirement | Current rule | Source | Implication for FeelKit |
|---|---|---|---|
| Gallery image size | Minimum 1920 x 1080 | Asset File Format and Structure Requirements | Capture at 1920 x 1080 or larger; 2560 x 1440 captures scaled down keep editor text sharp |
| Gallery image format | JPEG or PNG | same | PNG for UI shots (sharp text); JPEG for gameplay if a PNG goes over 3 MB |
| Gallery image file size | Less than 3 MB each | same | Full-screen editor PNGs at 1920 x 1080 usually fit; check each file |
| Gallery total | All 2D images less than 25 MB in total | same | At 2 to 2.5 MB per image, about 10 to 12 images fit; more only with JPEG compression |
| Gallery video | 1920 x 1080, up to 300 MB, MP4, MOV or WEBM | same | A short feel-off / feel-on demo clip can be uploaded directly |
| Thumbnail | "clear, easy to understand, and properly represents your product"; Fab generates tags from it. No separate size rule found | Publishing Assets for Sale or Free Download | Treat as a gallery image (1920 x 1080 minimum); must read at small size |
| Number of gallery items | No limit found in the sources | same | The 25 MB total is the practical limit |
| Media accuracy | "Images, videos, and models must accurately display the contents of the product" (1.8.6.a); branding per the Fab Brand Guidelines (1.8.6.b) | Technical Requirements | Every visual shows real FeelKit; designed graphics only frame real captures |

## Listing and publisher

| Requirement | Current rule | Source | Implication for FeelKit |
|---|---|---|---|
| Language | All text in English, correct spelling and grammar (1.8.1.a) | Technical Requirements | Listing and docs in English |
| Technical information | All relevant fields filled; must name dependencies, prerequisites, requirements (1.8.4) | Technical Requirements | List Niagara and Enhanced Input (engine plugins FeelKit enables), the GAS add-on and its needs, tested platform (Windows only, D-073) |
| Created with AI | "Products must reflect if generative AI tools were used during creation" (1.8.8.a); "We require Fab publishers who use AI to generate content for distribution to enable the Created with AI flag" (support article) | Technical Requirements, support article | **Decision for the user.** FeelKit's code, docs and recipes were written with an AI assistant. By the wording of 1.8.8.a this likely requires the Created with AI flag regardless of the icon or images. Ask Fab support how it applies to code if unsure |
| Support | Publishers must provide and monitor support channels (1.1.b), set in the Publisher Profile | Technical Requirements | Support email and Discord (the user's) |
| Marketing | Publishers do their own promotion (1.6.a) | Technical Requirements | Videos and posts are ours |
| Sales | No sale in the first 30 days after publishing (1.7.1.c) | Technical Requirements | Launch price must stand for a month |
| Blueprint-based projects | Must include a demo project or a video URL in the Long Description (4.1.1.c) | Technical Requirements | Not strictly a code-plugin rule, but a video URL in the description covers it either way |
| Third-party software | Must declare if the product includes or depends on third-party software, which includes **sounds, fonts, graphics** from other sources; code plugins declaring it fill a detailed form (4.2.5) | Technical Requirements | The demo sounds (CC0 Kenney and others in `Credits.md`) count: declare "uses third-party software" and fill the form |

## Code plugin files

| Requirement | Current rule | Source | Implication for FeelKit |
|---|---|---|---|
| Supported engine versions | On first submission, the product must support **the latest engine version** (4.2.2.b); at least one of the three latest at all times (4.2.2.c) | Technical Requirements | **UE 5.8 is required at launch.** 5.6 and 5.7 optional (5.6 is still one of the three latest) |
| One upload per engine version | A separate Project Version and plugin folder per engine version, even if only `EngineVersion` differs (4.2.2.d) | Technical Requirements | One zip per engine version (5.6, 5.7, 5.8) |
| Epic builds the binaries | Plugins ship with binaries built by Epic; test with BuildPlugin from installed engine builds (4.3.6.2.b); Epic builds the three latest engine versions (4.3.6.2.d) | Technical Requirements | `package_plugin.ps1` per engine version before each upload |
| No errors or warnings | Code plugins must generate no errors or consequential warnings (4.3.6.2.a) | Technical Requirements | Already our rule |
| `EngineVersion` key | Required, e.g. "5.6.0" (4.3.6.a) | Technical Requirements | Present; set per upload |
| Platform key per module | Each module needs `PlatformAllowList` or `PlatformDenyList` (4.3.6.b) | Technical Requirements | **Fixed 2026-09-23:** runtime modules Win64, Mac, Linux, Android, IOS; FeelEditor Win64, Mac, Linux |
| `FabURL` key | Required; after submission set to `com.epicgames.launcher://ue/Fab/product/<id>` (4.3.6.c) | Technical Requirements | Add once the product has its Publisher Portal ID |
| Dependencies | May depend on engine plugins, **not on other user-made plugins** (4.3.6.d) | Technical Requirements | FeelKit depends only on engine plugins. A separate Fab listing for the GAS add-on (D-066 option C) would depend on FeelKit and break this rule, so option B (add-on inside FeelKit, D-076) is the allowed route |
| Source included | No closed source for engine-dependent code (4.3.6.1.a) | Technical Requirements | Full source ships |
| Copyright notice | Every source and header file: commented notice with the publisher's name **and the year of intended publishing**, not the Epic text (4.3.6.1.b) | Technical Requirements | **Fixed 2026-09-23:** `// Copyright 2026 Billo. All Rights Reserved.` in all 231 files. Update the year if launch moves to 2027 |
| Purpose | Must add editor functionality, integrate third-party systems or expose complex gameplay logic to Blueprints (4.3.6.1.d) | Technical Requirements | Met |
| No .exe or .msi | (4.3.6.1.e) | Technical Requirements | None |
| Installed as engine plugin | Purchased plugins install to `Engine/Plugins/Fab`, not the project; an Example Project is strongly encouraged, linked in Technical Information as "Example Project:", hosted externally, depending on the plugin but not containing it (4.3.6.3) | Technical Requirements | The GAS add-on is copied from the engine's Fab folder into the project: document that path. The demo levels live in template projects: an example project (or the kits) should be downloadable separately |
| Folder layout | Only Config, Content, Resources, Source and the .uplugin; other folders need `Config/FilterPlugin.ini` (4.3.7.3) | Technical Requirements | FilterPlugin lists Library, Demos, Extras, Credits.md. `Resources` (icon) still missing |
| Path length | 170 characters or less from the plugin folder (4.3.7.3.c) | Technical Requirements | Checked 2026-09-23: none over |
| Names | English letters, digits and underscores only; not vague ("Assets", "NewFolder") (4.3.7.1) | Technical Requirements | Checked 2026-09-23: all names pass |
| Documentation | Free documentation covering implementation, application, modification; web guide, txt/pdf, comments, videos or in-editor tutorials; English; download links must not start automatically (4.3.8) | Technical Requirements | **Docs must exist before launch** (changes D-051, which allowed docs after release) |
| Experimental features | Dependencies on experimental or beta engine features must be stated in the description (4.3.1.d) | Technical Requirements | Check Niagara and Enhanced Input status per engine version (both are standard in 5.6+) |
| Works in binary engine builds | (4.3.1.i) | Technical Requirements | Met (built against the installed 5.6) |
