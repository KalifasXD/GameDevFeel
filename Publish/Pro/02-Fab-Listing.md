# FeelKit Pro: Fab listing

Everything to enter in the Fab Publisher Portal for the paid listing, field by field, in the order the form asks. Text in
section 2 is ready to paste as it is. Items marked **MANUAL ACTION REQUIRED** need something only the publisher can do;
`PUBLISH-README.md` (one folder up) explains each one.

## 1. Fields

| Field | Value |
|---|---|
| Title (80 characters max) | FeelKit Pro: Game Feel, Hitstop and Screen Shake on a Timeline |
| Description | The text in section 2.1 |
| Product type | Tools & Plugins |
| Category | Tools & Plugins > Gameplay Features |
| License type | Standard License, Paid |
| Price, Personal | USD 34.99 |
| Price, Professional | USD 64.99 |
| Discount | Not applicable: Fab allows no sale in the first 30 days, and the launch price stands for that month |
| Tags (25 max) | feedback, shake, camera, impact, hit, freeze, slow, flash, punch, squash, stretch, force, controller, comfort, accessibility, timeline, editor, blueprint, codeplugin, combat, reaction, gameplay, multiplayer, gas, niagara |
| How to enter the tags | Fab only accepts tags its picker already has: one lowercase word each, no spaces. Type each word and pick it from the list. Every tag here was checked on 2026-09-26 against Fab's own search and is used by other listings. Hitstop has no tag of its own; `freeze` is what Fab's hitstop plugins use. `juice` and `juicy` exist but stay out (D-082) |
| Thumbnail image | `01-Images/01-Main.png` (1920 x 1080, PNG, 0.5 MB) |
| Media gallery, in this order | `01-Images/01-Main.png`, `02-Recipe-Editor.png`, `03-Demos.jpg`, `04-Blueprint.png`, `05-Library.png`, `06-Parameters.png`, `07-Triggers.png`, `08-Comfort-Menu.png`, `09-Essential-Tracks.png`, `10-Editions.png` (10 files, 3.7 MB in total; every file 1920 x 1080 and under 3 MB) |
| Gallery videos | None at first submission. The feel off and on listing video from the Video Guide goes in second place once it is recorded |
| Epic Developer Community forum post | No. Support runs through Discord and email, which are monitored; a forum thread would be a third official channel to keep answering (Fab 1.2.b) |
| Mature content | No |
| Disallow use by Generative AI Programs | Ticked. It only adds Fab's "NoAI" tag against AI data collection; buyers' use of the plugin is not affected |
| Use of generative AI tools | Yes (D-079) |
| Promotional content | No. FeelKit contains no paid advertising for any brand or product |
| FAQs | The questions and answers in section 2.2, one entry each |
| Format | Unreal Engine (three versions, below) |
| Additional files | None |

### Unreal Engine versions (Add new format > Unreal Engine, once per engine)

| Field | UE 5.6 | UE 5.7 | UE 5.8 |
|---|---|---|---|
| Version title (30 characters max) | FeelKit Pro 1.0.0 for UE 5.6 | FeelKit Pro 1.0.0 for UE 5.7 | FeelKit Pro 1.0.0 for UE 5.8 |
| Project file | `03-Upload/UE5.6/FeelKit_Pro_1.0.0_UE5.6.zip` | `03-Upload/UE5.7/FeelKit_Pro_1.0.0_UE5.7.zip` | `03-Upload/UE5.8/FeelKit_Pro_1.0.0_UE5.8.zip` |
| Project file link (https) | https://dounavis-home-cloud-storage.quickconnect.to/d/s/1A2rAKGE1Ktj7cOiOvS8R3jqJdkfhbRS/ugp-eErRsSLLl2r0TuH_2cbfLcOR5tNL-yLtAEIfziA0 | https://dounavis-home-cloud-storage.quickconnect.to/d/s/1A2rAWY5OT8QLG8k4a63eZQU0P5vdA3X/RJ7sVYBdbBERRVwCRhfNyNZpYB9CjHI2-0rugwI7ziA0 | https://dounavis-home-cloud-storage.quickconnect.to/d/s/1A2rAkQjAt1rEzJRi5nHKbCsUAfCJjmc/7BvFZito10Cx8MlIxWlAEQLFSgWpBfly-3bvAVZfziA0 |
| Supported engine version | 5.6 | 5.7 | 5.8 |
| Supported target platforms | Windows, Mac, Linux, Android, iOS | Windows, Mac, Linux, Android, iOS | Windows, Mac, Linux, Android, iOS |
| Version notes | The text in section 2.3 | The text in section 2.3 | The text in section 2.3 |

### Technical details (inside each Unreal Engine version, the same for 5.6, 5.7 and 5.8)

| Field | Value |
|---|---|
| Supported development platforms | Windows, macOS, Linux (the editor module allows all three; only Windows was built and tested, as the description says) |
| Distribution method | Plugin |
| Third party software usage | **This product uses third party software.** The 43 sample sounds (Kenney and artisticdude) come from other authors under CC0 1.0; every file is listed with its source in Credits.md. Selecting "does not include" would be wrong: Fab counts sounds from other sources as third-party software |
| Third-party software to declare, if the form asks for details | 43 sample sounds (Kenney and artisticdude), CC0 1.0 (public domain), from https://kenney.nl and https://opengameart.org/content/swishes-sound-pack; listed in Credits.md. Fab's Third Party Software form: section 3 |
| Is open source | No. The full C++ source is included, but under Fab's Standard License, not an open source license |
| Tool type | Plugin |
| Blueprints | 1 (the comfort menu, WBP_FeelComfortMenu) |
| C++ classes | 101 |
| Additional information | Built and tested on Windows with Unreal Engine 5.6, 5.7 and 5.8. Engine plugins it turns on: Niagara, Enhanced Input. GAS add-on: copy Extras/FeelKitGAS from the FeelKit folder in your engine into your project's Plugins folder. Packaging a Blueprint-only project with FeelKit needs Visual Studio, as with any code plugin. On Windows, PlayStation and other non-Xbox controllers vibrate through Steam Input. Documentation: the FeelKit Manual (PDF), <MANUAL LINK>. Support: https://discord.gg/AtJ6RdwaxA and billoue4@gmail.com |

## 2. Text to paste

### 2.1 Description

Use the editor's bold for the lines in bold and its bullet list for the lists.

---

FeelKit is a game feel plugin for Unreal Engine. It handles the moment after something happens in your game: the hit, the landing, the pickup, the door that will not open.

You build that response as a recipe, a short timeline of tracks such as hitstop, camera shake, camera punch, screen flash, sound and controller rumble. You tune it in the recipe editor while the preview plays it, and your game plays it with one node. The preview runs the same code as the game, so what you see while tuning is what the player gets.

Every effect passes through the player's comfort settings. A player who turns camera shake off gets no camera shake, from FeelKit or from the engine's own camera shakes.

**Playable demos**

Four levels built on Unreal's own templates. Each has a Feel Switch: press Tab, or View on a controller, to turn FeelKit off and on while you play.

- Action/RPG: https://drive.google.com/file/d/1439QD9WBBOfkhdBi9PYWdEE0jxMGH-Ow/view?usp=sharing
- Platformer: https://drive.google.com/file/d/1dzFHOMr03-rhNYP136canMOdSchmUExf/view?usp=sharing
- Shooter: https://drive.google.com/file/d/136TpVvZeGb1wYiVQnvFN9dw47Ej3ffHW/view?usp=sharing
- Horror: https://drive.google.com/file/d/1Akb1gguqHMuv_-cLY3VhWjjfTunTtPkU/view?usp=sharing

Owners of FeelKit can request the Unreal Engine 5.6 projects behind the playable demos on our Discord or by email.

**Recipe editor**

- Tracks on a timeline, colored by channel: drag, trim, snap to frames, zoom
- Intensity curves edited right on the track
- Scrub forward and backward, loop, mute and solo tracks, copy and paste tracks between recipes, full undo
- Preview on any static or skeletal mesh, with comfort presets and recipe parameters applied live
- Play in PIE sends the recipe to the running game, and edits apply to the next play
- Validation on save catches broken curves, missing assets and tracks with no length

**37 effects**

- Camera: procedural shake, camera punch, FOV kick, roll, zoom, look-at nudge
- Time: global hitstop, actor hitstop, slow motion ramp
- Screen: flash, vignette, chromatic aberration, desaturate, color tint, fade, post process material pulse
- Actor: scale punch, squash and stretch, material parameter pulse, hit flash, mesh wobble, light flash
- Audio: play sound, sound class duck, pitch bend, low-pass sweep
- Controller: force feedback curve, haptic pattern
- UI: widget punch, widget shake, widget flash, number pop
- Spawn: decal, Niagara particle
- Recipe inside a recipe, random choice, Blueprint event

**Playing recipes**

- Play Feel on an actor, a component, a widget, a location or the player's camera, and stop it through its handle
- Play Feel and Wait for Blueprint sequences
- Parameters such as Damage from 0 to 100 that scale any track
- Accumulators for values that build up and decay, such as combo streaks
- Sustained recipes that hold while the player charges, sprints or stays at low health
- A cooldown and a limit on copies playing at once, per recipe

**Hooking it up**

- Feel Maps: send an event such as Feel.Event.Hit with context tags, and the most specific row picks the recipe (heavy, critical, by surface)
- Anim notifies on montages: Play Feel, Play Feel (Window), Set Feel Value, Send Feel Event
- Feel Trigger component: jumped, air jumped, launched, landed, damage taken, hits and overlaps, with no Blueprint code
- Enhanced Input: play recipes from input actions
- Gameplay Ability System: a Gameplay Cue that plays a recipe, in an optional add-on plugin
- Multiplayer: effects stay local and cosmetic, and the Feel Replication component sends plays to other machines

**Comfort for players**

- Settings per local player: master, camera shake, camera motion, flashes, hitstop and slow motion, screen distortion, controller vibration
- Presets: Default, Reduced Motion, Reduced Flashing, No Haptics, plus your own preset assets
- Flash limiter, camera roll on or off, zoom speed limit
- The engine's own camera shakes and force feedback follow the settings too
- Essential tracks play a substitute, a vignette in place of a flash for example, when a player turns that effect off
- A ready comfort menu for keyboard, mouse and gamepad that you can copy and restyle
- Saved per player, or routed to your own save system
- The Comfort Audit lists recipes with rapid or saturated red flashes before you ship

**Library**

38 ready recipes grouped by feeling: Impact, Weight, Power, Speed, Reward, Danger, Dread, Denial and Interface. Browse them in the Recipe Browser with filters and a live preview on hover, or start a new recipe from any of them. Library recipes are read-only, so copy one into your project to change it.

**Debugging and tools**

- FeelKit Debugger: what is playing, on which target, at what intensity, and the comfort in effect
- showdebug feel and stat Feel
- Recent Plays: open a moment from your play session in the recipe editor and scrub it
- GIF capture of a recipe with the feel off and on
- Sound waveforms on tracks, snapping to sound onsets, and tracks made from a sound
- JSON import and export of recipes

**Documentation and support**

- Manual (PDF): <MANUAL LINK>
- Discord: https://discord.gg/AtJ6RdwaxA
- Email: billoue4@gmail.com

**Technical details**

- Code modules: FeelCore (Runtime), FeelEditor (Editor), FeelNiagara (Runtime), FeelEnhancedInput (Runtime); the GAS add-on plugin FeelKitGAS (module FeelGAS, Runtime) is in the plugin's Extras folder
- FeelCore uses only stock engine modules. FeelKit turns on the engine plugins Niagara and Enhanced Input
- Engine versions: 5.6, 5.7, 5.8
- Platforms: built and tested on Windows. The plugin allows Mac, Linux, Android and iOS, which have not been built or tested
- Network replicated: yes, through the Feel Replication component; effects play locally on each machine
- Nothing runs while no recipe is playing
- Packaging a Blueprint-only project with FeelKit needs Visual Studio, as with any code plugin
- On Windows, PlayStation and other non-Xbox controllers vibrate through Steam Input
- GAS add-on: copy Extras/FeelKitGAS from the FeelKit folder in your engine into your project's Plugins folder
- Sample sounds are CC0 (Kenney, artisticdude), listed with their sources in Credits.md

---

### 2.2 FAQs

**Do I need C++?**
No. Every feature has Blueprint nodes, components or assets, and C++ projects can call the same functions. Packaging a Blueprint-only project with FeelKit needs Visual Studio installed, because Unreal builds the project's own game executable whenever a project uses a code plugin.

**Which engine versions and platforms are supported?**
Unreal Engine 5.6, 5.7 and 5.8. FeelKit is built and tested on Windows. The plugin allows Mac, Linux, Android and iOS, but those have not been built or tested.

**Will the effects look the same in the editor and in my game?**
Yes, for every effect the preview shows: the recipe editor's preview and the game run the same evaluation code, and recipes use real time, so they play the same at any frame rate. Hitstop, slow motion, controller vibration, Blueprint events and UI steps need a running game; Play in PIE plays the recipe there.

**Does FeelKit replace my camera shakes or force feedback?**
No. It adds its own camera modifier and leaves your camera shakes and force feedback as they are. The comfort settings scale the engine's camera shakes and force feedback as well, so the player's choices apply to everything.

**Does it work in multiplayer?**
Yes. Effects are cosmetic and play locally. Add the Feel Replication component to send plays to other machines: to everyone, to the owner only, or to everyone but the owner. A hitstop in multiplayer slows only its target on that machine.

**My PlayStation controller does not vibrate.**
On Windows, Unreal sends vibration only to Xbox-style controllers. PlayStation, Switch and other controllers vibrate when the game runs through Steam with Steam Input on.

**How do I use FeelKit with the Gameplay Ability System?**
Copy Extras/FeelKitGAS from the FeelKit folder in your engine (usually under Engine/Plugins/Marketplace) into your project's Plugins folder; it is on as soon as it is there. It adds two Gameplay Cue notify classes that play recipes. Projects without the add-on never load GAS.

**Can I move a project from FeelKit Lite to Pro?**
Yes. Remove Lite from your engine, install Pro, and your recipes open unchanged. Both editions use the plugin name FeelKit, so install one of them per engine.

**Can I get the demo projects?**
The playable builds are linked in the description. Owners of FeelKit can request the Unreal Engine 5.6 projects behind them on Discord or by email. They use Epic's 5.6 template content, which does not run in 5.7 or 5.8, so the source is for 5.6 only.

### 2.3 Version notes

First release of FeelKit Pro, 1.0.0. Built and tested on Windows. Includes the optional GAS add-on in Extras/FeelKitGAS.

## 3. Third Party Software form (Fab review, 2026-09-28)

Fab's first review failed the listing on the third-party declaration: the box "This product uses third party
software" is ticked, so Fab asks for its Third Party Software Submission Form (https://forms.gle/sgXJHReig6nSM1FE7).
FeelKit Pro has no third-party code. The only third-party content is 43 CC0 sample sounds by Kenney and artisticdude,
from 6 source packs, so the form gets one entry per pack.

| Form field | Value |
|---|---|
| Seller Name | Billo |
| Plugin Name | FeelKit Pro |
| Total Number of TPS | 6 |

Each entry asks for four fields. Paste them as they are.

#### TPS #1: Impact Sounds

| Field | Answer |
|---|---|
| Software Name and Version | Impact Sounds by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/impact-sounds |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Pro ships 13 of its sounds (S_FK_Hit_Light, S_FK_Hit_Heavy, S_FK_Hit_Crit, S_FK_Impact_Bullet, S_FK_Land_Thud, S_FK_Body_Hit, S_FK_Body_Fall, S_FK_Jump_Scuff, S_FK_Land_Step, S_FK_Land_Soft, S_FK_Land_Heavy, S_FK_Wall_Thump, S_FK_Impact_Ping), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #2: Sci-fi Sounds

| Field | Answer |
|---|---|
| Software Name and Version | Sci-fi Sounds by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/sci-fi-sounds |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Pro ships 13 of its sounds (S_FK_Explosion, S_FK_Charge_Loop, S_FK_Alarm, S_FK_Shot_Pistol, S_FK_Shot_Rifle, S_FK_Shot_Thump, S_FK_Shot_Launcher, S_FK_Explosion_Crunch, S_FK_Explosion_Low, S_FK_Impact_Thud, S_FK_Dread_Drone, S_FK_Scare_Boom, S_FK_Light_Buzz), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #3: Interface Sounds

| Field | Answer |
|---|---|
| Software Name and Version | Interface Sounds by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/interface-sounds |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Pro ships 9 of its sounds (S_FK_Pickup, S_FK_LevelUp, S_FK_Denied, S_FK_Air_Lift, S_FK_Hit_Tick, S_FK_Kill_Confirm, S_FK_Dry_Click, S_FK_Light_Crackle, S_FK_UI_Tick), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #4: RPG Audio

| Field | Answer |
|---|---|
| Software Name and Version | RPG Audio by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/rpg-audio |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Pro ships 4 of its sounds (S_FK_Whoosh, S_FK_Heartbeat, S_FK_Weapon_Switch, S_FK_Door_Creak), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #5: UI Audio

| Field | Answer |
|---|---|
| Software Name and Version | UI Audio by Kenney (the pack has no version number) |
| Download URL | https://kenney.nl/assets/ui-audio |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by Kenney. FeelKit Pro ships 2 of its sounds (S_FK_UI_Hover, S_FK_UI_Click), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

#### TPS #6: Swishes Sound Pack

| Field | Answer |
|---|---|
| Software Name and Version | Swishes Sound Pack by artisticdude (the pack has no version number) |
| Download URL | https://opengameart.org/content/swishes-sound-pack |
| License File Link | https://creativecommons.org/publicdomain/zero/1.0/legalcode |
| Description | Not code: a pack of CC0 sound effect recordings by artisticdude. FeelKit Pro ships 2 of its sounds (S_FK_Jump_Whoosh, S_FK_Dash_Whoosh), imported as Unreal Sound Wave assets in Content/Samples/Sounds. The plugin needs them so its sample recipes play a sound the first time a buyer previews them; buyers can replace every sound with their own. It is neither statically nor dynamically linked, because no code or library is included, only audio assets that the engine loads like any other sound. It sends no data back to the creator: the files are audio only, with no code and no network access. |

### Notes to the reviewer (paste when resubmitting)

---

Documentation: the manual is online at <MANUAL LINK>. It is linked from the description and from the technical details of each engine version.

Third-party software: the Third Party Software form is filled in for FeelKit Pro (6 entries). FeelKit contains no third-party code or libraries: every file under Source is our own code and carries the Billo copyright notice. The only third-party content is 43 sample sounds under CC0 1.0 by Kenney and artisticdude, imported as Sound Wave assets in Content/Samples/Sounds and listed file by file, with their source packs, in Credits.md at the plugin root. There is no ThirdParty folder under Source because there is no third-party code to put in it.

---
