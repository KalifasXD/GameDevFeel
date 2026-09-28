# FeelKit Lite: Fab listing

Everything to enter in the Fab Publisher Portal for the free listing, field by field, in the order the form asks. Text
in section 2 is ready to paste as it is. Items marked **MANUAL ACTION REQUIRED** need something only the publisher can
do; `PUBLISH-README.md` (one folder up) explains each one.

## 1. Fields

| Field | Value |
|---|---|
| Title (80 characters max) | FeelKit Lite (Free): Game Feel, Hitstop and Screen Shake on a Timeline |
| Description | The text in section 2.1 |
| Product type | Tools & Plugins |
| Category | Tools & Plugins > Gameplay Features |
| License type | Standard License, Free |
| Price | Free |
| Discount | Not applicable: the product is free |
| Tags (25 max) | feedback, shake, camera, impact, hit, freeze, slow, flash, punch, squash, stretch, force, controller, gamepad, comfort, accessibility, timeline, editor, blueprint, codeplugin, combat, reaction, gameplay, plugin, freebie |
| How to enter the tags | Fab only accepts tags its picker already has: one lowercase word each, no spaces. Type each word and pick it from the list. Every tag here was checked on 2026-09-26 against Fab's own search and is used by other listings. Hitstop has no tag of its own; `freeze` is what Fab's hitstop plugins use. `juice` and `juicy` exist but stay out (D-082) |
| Thumbnail image | `01-Images/01-Main.png` (1920 x 1080, PNG, 0.5 MB) |
| Media gallery, in this order | `01-Images/01-Main.png`, `02-Recipe-Editor.png`, `03-Blueprint.png`, `04-Library.png`, `05-Comfort-Menu.png`, `06-Essential-Tracks.png`, `07-Editions.png` (7 files, 2.5 MB in total; every file 1920 x 1080 and under 3 MB) |
| Gallery videos | None at first submission. The Lite listing video from the Video Guide goes in second place once it is recorded |
| Epic Developer Community forum post | No. Support runs through Discord and email, which are monitored; a forum thread would be a third official channel to keep answering (Fab 1.2.b) |
| Mature content | No |
| Disallow use by Generative AI Programs | Ticked. It only adds Fab's "NoAI" tag against AI data collection; buyers' use of the plugin is not affected |
| Use of generative AI tools | Yes (D-079) |
| Promotional content | No. FeelKit Lite contains no paid advertising for any brand or product. The description names FeelKit Pro, the publisher's own paid edition, as the upgrade; that is product information, not a paid advertisement |
| FAQs | The questions and answers in section 2.2, one entry each |
| Format | Unreal Engine (three versions, below) |
| Additional files | None |

### Unreal Engine versions (Add new format > Unreal Engine, once per engine)

| Field | UE 5.6 | UE 5.7 | UE 5.8 |
|---|---|---|---|
| Version title (30 characters max) | FeelKit Lite 1.0.0 for UE 5.6 | FeelKit Lite 1.0.0 for UE 5.7 | FeelKit Lite 1.0.0 for UE 5.8 |
| Project file | `03-Upload/UE5.6/FeelKit_Lite_1.0.0_UE5.6.zip` | `03-Upload/UE5.7/FeelKit_Lite_1.0.0_UE5.7.zip` | `03-Upload/UE5.8/FeelKit_Lite_1.0.0_UE5.8.zip` |
| Project file link (https) | **MANUAL ACTION REQUIRED:** share link of the 5.6 zip | **MANUAL ACTION REQUIRED:** share link of the 5.7 zip | **MANUAL ACTION REQUIRED:** share link of the 5.8 zip |
| Supported engine version | 5.6 | 5.7 | 5.8 |
| Supported target platforms | Windows, Mac, Linux, Android, iOS | Windows, Mac, Linux, Android, iOS | Windows, Mac, Linux, Android, iOS |
| Version notes | The text in section 2.3 | The text in section 2.3 | The text in section 2.3 |

### Technical details (inside each Unreal Engine version, the same for 5.6, 5.7 and 5.8)

| Field | Value |
|---|---|
| Supported development platforms | Windows, macOS, Linux (the editor module allows all three; only Windows was built and tested, as the description says) |
| Distribution method | Plugin |
| Third party software usage | **This product uses third party software.** The 7 sample sounds (Kenney) come from other authors under CC0 1.0; every file is listed with its source in Credits.md. Selecting "does not include" would be wrong: Fab counts sounds from other sources as third-party software |
| Third-party software to declare, if the form asks for details | 7 sample sounds (Kenney), CC0 1.0 (public domain), from https://kenney.nl; listed in Credits.md. Fab's Third Party Software form: section 3 |
| Is open source | No. The full C++ source is included, but under Fab's Standard License, not an open source license |
| Tool type | Plugin |
| Blueprints | 1 (the comfort menu, WBP_FeelComfortMenu) |
| C++ classes | 58 |
| Additional information | Built and tested on Windows with Unreal Engine 5.6, 5.7 and 5.8. Needs no other plugin. Packaging a Blueprint-only project with FeelKit needs Visual Studio, as with any code plugin. On Windows, PlayStation and other non-Xbox controllers vibrate through Steam Input. Documentation: the FeelKit Manual (PDF), https://drive.google.com/file/d/1OlIEERggkVAoG0rnG2FBbLPwHgKcjn_R/view?usp=sharing. Support: https://discord.gg/AtJ6RdwaxA and billoue4@gmail.com |

## 2. Text to paste

### 2.1 Description

Use the editor's bold for the lines in bold and its bullet list for the lists.

---

FeelKit Lite is the free edition of FeelKit, a game feel plugin for Unreal Engine. It handles the moment after something happens in your game: the hit, the landing, the pickup, the door that will not open.

You build that response as a recipe, a short timeline of tracks such as hitstop, camera shake, camera punch, screen flash, sound and controller rumble. You tune it in the recipe editor while the preview plays it, and your game plays it with one node. The preview runs the same code as the game, so what you see while tuning is what the player gets.

Every effect passes through the player's comfort settings, and Lite has all of them. A player who turns camera shake off gets no camera shake, from FeelKit or from the engine's own camera shakes.

Lite is a complete tool, not a trial: no time limit, no watermark, and you can ship games with it.

**Recipe editor**

- Tracks on a timeline, colored by channel: drag, trim, snap to frames, zoom
- Intensity curves edited right on the track
- Scrub forward and backward, loop, mute and solo tracks, copy and paste tracks between recipes, full undo
- Preview on any static or skeletal mesh, with the comfort presets applied live
- Play in PIE sends the recipe to the running game, and edits apply to the next play
- Validation on save catches broken curves, missing assets and tracks with no length

**12 effects**

- Camera: procedural shake, camera punch, FOV kick
- Time: global hitstop, slow motion ramp
- Screen: flash, vignette
- Actor: scale punch, squash and stretch
- Audio: play sound
- Controller: force feedback curve
- Blueprint event, for anything else your game does at that moment

**Playing recipes**

- Play Feel on an actor, a component, a widget, a location or the player's camera, from Blueprint or C++
- Stop a play through its handle, or stop everything at once
- A cooldown and a limit on copies playing at once, per recipe
- Feel Switch: an actor that turns FeelKit off and on while you play, to compare

**Comfort for players**

- Settings per local player: master, camera shake, camera motion, flashes, hitstop and slow motion, screen distortion, controller vibration
- Presets: Default, Reduced Motion, Reduced Flashing, No Haptics, plus your own preset assets
- Flash limiter, camera roll on or off, zoom speed limit
- The engine's own camera shakes and force feedback follow the settings too
- Essential tracks play a substitute, a vignette in place of a flash for example, when a player turns that effect off
- A ready comfort menu for keyboard, mouse and gamepad that you can copy and restyle
- Saved per player, or routed to your own save system

**Library**

11 ready recipes: light and heavy hits, damage taken, blocked and locked, a jump scare, kill confirm, pickup, sprint start, heavy footstep and stomp. Play them as they are, or copy one into your project and change it.

**What FeelKit Pro adds**

FeelKit Pro, from the same publisher on Fab, adds 25 more effects (actor hitstop, chromatic aberration, hit flash, UI punches, Niagara particles, decals and more), 27 more library recipes with the Recipe Browser, recipe parameters, accumulators and sustained recipes, Feel Maps and events, anim notifies, the Feel Trigger component, Enhanced Input, a GAS add-on, multiplayer replication, the FeelKit Debugger, Recent Plays, GIF capture, the Comfort Audit, tracks from sound and JSON import and export. Recipes made in Lite open unchanged in Pro.

**Playable demos**

Four demo levels show what FeelKit does in a game. They are built with FeelKit Pro. Press Tab, or View on a controller, to turn FeelKit off and on while you play.

- Action/RPG: https://drive.google.com/file/d/1439QD9WBBOfkhdBi9PYWdEE0jxMGH-Ow/view?usp=sharing
- Platformer: https://drive.google.com/file/d/1dzFHOMr03-rhNYP136canMOdSchmUExf/view?usp=sharing
- Shooter: https://drive.google.com/file/d/136TpVvZeGb1wYiVQnvFN9dw47Ej3ffHW/view?usp=sharing
- Horror: https://drive.google.com/file/d/1Akb1gguqHMuv_-cLY3VhWjjfTunTtPkU/view?usp=sharing

**Documentation and support**

- Manual (PDF): https://drive.google.com/file/d/1OlIEERggkVAoG0rnG2FBbLPwHgKcjn_R/view?usp=sharing
- Discord: https://discord.gg/AtJ6RdwaxA
- Email: billoue4@gmail.com

**Technical details**

- Code modules: FeelCore (Runtime), FeelEditor (Editor)
- FeelCore uses only stock engine modules, and Lite turns on no other plugins
- Engine versions: 5.6, 5.7, 5.8
- Platforms: built and tested on Windows. The plugin allows Mac, Linux, Android and iOS, which have not been built or tested
- Network replicated: no. Effects are cosmetic and play on the machine that calls Play Feel
- Nothing runs while no recipe is playing
- Packaging a Blueprint-only project with FeelKit needs Visual Studio, as with any code plugin
- On Windows, PlayStation and other non-Xbox controllers vibrate through Steam Input
- Sample sounds are CC0 (Kenney), listed with their sources in Credits.md

---

### 2.2 FAQs

**Is FeelKit Lite really free for commercial games?**
Yes. It is free under Fab's Standard License, with no time limit and no watermark, and you can ship games made with it.

**Do I need C++?**
No. Every feature has Blueprint nodes or assets, and C++ projects can call the same functions. Packaging a Blueprint-only project with FeelKit needs Visual Studio installed, because Unreal builds the project's own game executable whenever a project uses a code plugin.

**Which engine versions and platforms are supported?**
Unreal Engine 5.6, 5.7 and 5.8. FeelKit is built and tested on Windows. The plugin allows Mac, Linux, Android and iOS, but those have not been built or tested.

**Will the effects look the same in the editor and in my game?**
Yes, for every effect the preview shows: the recipe editor's preview and the game run the same evaluation code, and recipes use real time, so they play the same at any frame rate. Hitstop, slow motion, controller vibration and Blueprint events need a running game; Play in PIE plays the recipe there.

**Does FeelKit replace my camera shakes or force feedback?**
No. It adds its own camera modifier and leaves your camera shakes and force feedback as they are. The comfort settings scale the engine's camera shakes and force feedback as well, so the player's choices apply to everything.

**My PlayStation controller does not vibrate.**
On Windows, Unreal sends vibration only to Xbox-style controllers. PlayStation, Switch and other controllers vibrate when the game runs through Steam with Steam Input on.

**What is the difference between Lite and Pro?**
Lite has the recipe editor, 12 effects, the full comfort layer with its menu, the Feel Switch and 11 library recipes. Pro adds 25 effects, 27 recipes, parameters, event and trigger hookups, GAS and multiplayer support, and the debugging tools. The last image in the gallery lists both side by side.

**Can I move a project from Lite to Pro later?**
Yes. Remove Lite from your engine, install Pro, and your recipes open unchanged. Both editions use the plugin name FeelKit, so install one of them per engine.

### 2.3 Version notes

First release of FeelKit Lite, 1.0.0. Built and tested on Windows.

## 3. Third Party Software form (Fab review, 2026-09-28)

Fab's first review failed the listing on the third-party declaration: the box "This product uses third party
software" is ticked, so Fab asks for its Third Party Software Submission Form (https://forms.gle/sgXJHReig6nSM1FE7).
FeelKit Lite has no third-party code. The only third-party content is 7 CC0 sample sounds by Kenney,
from 3 source packs, so the form gets one entry per pack.

| Form field | Value |
|---|---|
| Seller Name | Billo |
| Plugin Name | FeelKit Lite |
| Total Number of TPS | 3 |

Each entry asks for four fields. Paste them as they are.

#### TPS #1: Impact Sounds

| Field | Answer |
|---|---|
| Software Name and Version | Impact Sounds by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/impact-sounds |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Lite ships 4 of its sounds (S_FK_Hit_Light, S_FK_Hit_Heavy, S_FK_Hit_Crit, S_FK_Land_Thud), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #2: Interface Sounds

| Field | Answer |
|---|---|
| Software Name and Version | Interface Sounds by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/interface-sounds |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Lite ships 2 of its sounds (S_FK_Pickup, S_FK_Denied), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #3: Sci-fi Sounds

| Field | Answer |
|---|---|
| Software Name and Version | Sci-fi Sounds by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/sci-fi-sounds |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Lite ships 1 of its sounds (S_FK_Alarm), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

### Notes to the reviewer (paste when resubmitting)

---

Documentation: the manual is online at https://drive.google.com/file/d/1OlIEERggkVAoG0rnG2FBbLPwHgKcjn_R/view?usp=sharing. It is linked from the description and from the technical details of each engine version.

Third-party software: the Third Party Software form is filled in for FeelKit Lite (3 entries). FeelKit contains no third-party code or libraries: every file under Source is our own code and carries the Billo copyright notice. The only third-party content is 7 sample sounds under CC0 1.0 by Kenney, imported as Sound Wave assets in Content/Samples/Sounds and listed file by file, with their source packs, in Credits.md at the plugin root. There is no ThirdParty folder under Source because there is no third-party code to put in it.

---
