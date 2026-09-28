# FeelKit Competitor Analysis (Red Team)

| Field | Value |
|---|---|
| Date | 2026-09-16 |
| Scope | Unreal (Fab + engine-native), Unity (Feel and others), Godot, haptics and accessibility tooling |
| Method | Fab listing pages and Fab's own listing/review JSON (read in a browser session), vendor docs, release notes, Unity Asset Store pages, Epic docs, Microsoft XAG docs. FeelKit state checked against `GameFeelDev/Plugins/FeelKit/Source` |
| Evidence labels | **VERIFIED** = I read the source on 2026-09-16. **INFERRED** = my conclusion from indirect evidence, or absence of a feature in docs I read |
| Prices | Fab search "starting price" in USD. On 2026-09-16 Fab's per-license prices were about 13% lower on several listings (for example 25.99 vs 29.99), which looks like a running sale (INFERRED) |

---

## 1. Summary verdict

### A. Is FeelKit already meaningfully ahead?

**Short answer: ahead on two axes (authoring UX, comfort), behind on almost everything a buyer checks first on a Fab page.** Today FeelKit is a better *editor* sitting on a *thinner product*. The current Unreal leader, Game Juice Pro, is a Feel-style clone with 28 feedback types, multiplayer, chance, randomization, distance falloff and presets, at $29.99. Its first review says Unreal "was desperately lacking a solid equivalent" of Feel. That is the position FeelKit wants, and someone else already holds it in buyers' minds.

| Dimension | Verdict | Evidence (details in sections 2 and 3) |
|---|---|---|
| Authoring workflow (timeline, tracks, curves on rows) | **Ahead** | No Unreal competitor has a timeline. Game Juice Pro presets are a Details-panel array (VERIFIED, docs "Guía de uso"). Agentic FeedbackFX is one Data Asset config (VERIFIED). Adrenaline GameJuice is 20 components set in Details (VERIFIED). Feel (Unity) is an inspector list. In v6.1 (Sep 2026) you can change a duration by double-clicking a list entry (VERIFIED). Only Juicee (Godot, free) has a visual graph (VERIFIED). **Caveat:** no competitor review asks for a timeline. Reviews praise presets, ease and multiplayer (VERIFIED, Fab reviews). |
| Preview without PIE | **Ahead in Unreal, parity cross-engine** | No UE juice plugin previews in editor. Game Juice Pro says to test in PIE (VERIFIED). Feel 6.0 (Jul 2026) added an experimental preview mode that covers only some feedbacks (VERIFIED). Sparkle (Godot) previews without play mode (VERIFIED). Epic's Camera Shake Previewer covers shakes only (VERIFIED). FeelKit's composed multi-channel preview with scrubbing, audio and post-process is still the best of these. |
| Deterministic scrub / pure evaluation | **Ahead (invisible to buyers)** | Nobody else claims it. Buyers do not search for it. It pays off only through features built on it (moment capture, capture-to-GIF). |
| Runtime arbitration (camera caps, time owner restore, strongest-wins per channel) | **Ahead** | Only Cronus Hitstop prioritizes hitstops (VERIFIED). Game Juice Pro stops scale buildup (VERIFIED). Nobody else arbitrates across channels (INFERRED from docs). |
| Step breadth | **Behind** | FeelKit ships 15 step types counting Global and Actor Hitstop and the Blueprint Event step (15 headers under `Steps/`, one of them the shaped-motion base). Game Juice Pro has 28, GameJuice 20, Feel 150+ (all VERIFIED). Missing effects buyers expect include particles/Niagara, bloom/DoF/color grade, ghost trail, rim glow, blink/flicker, camera tilt/roll, speed lines, UI punch, adaptive triggers and spawn. |
| Parameterization (chance, randomization, distance falloff, magnitude inputs) | **Behind** | Game Juice Pro: chance, random intensity and duration, distance falloff curves, "intensity interval", pins generated from preset content (VERIFIED). Feel: chance, random duration, range falloff (VERIFIED). FeedbackFX: magnitude scaling and custom context params (VERIFIED). FeelKit: one call intensity plus a random seed per play. `FFeelConditions` (Chance, MaxDistance, LocalPlayerOnly, Platforms) exists as data only and is **never evaluated** (VERIFIED in `FeelTrack.h` and `FeelCore/Private`). |
| Triggers without code | **Behind** | FeedbackFX: GAS attribute-threshold and status-tag triggers (VERIFIED). Natural Impacts: UE damage events (VERIFIED). Feel: auto-play on start/enable and channel events (VERIFIED). FeelKit: Blueprint call only. |
| Networking | **Behind** | Game Juice Pro: Local, Multicast, Server Authoritative, Owner Only, Skip Owner, relevancy distance, only-if-visible (VERIFIED). GameJuice: multicast with per-observer falloff (VERIFIED). FeedbackFX: "LocalVisual" hitstop (VERIFIED). FeelKit: none. |
| Comfort / accessibility | **Ahead (unique), with a compliance flaw** | No juice product has per-player channel comfort, presets or persistence. Feel has one global on/off switch (VERIFIED). GameJuice has one Global Intensity Scale (VERIFIED). **Flaw:** Xbox "Camera Comfort" fails a game if the screen still shakes after the player disables shake, or if a slider only goes down to 10% (VERIFIED, XAG feature tags). FeelKit's `EssentialFloor` on a motion or shake channel would cause exactly that failure. Also, FeelKit comfort does not scale engine-native shakes the game plays itself. |
| Haptics | **Parity at best** | FeelKit: per-motor curve, strongest-wins. GameJuice has a PS5 Adaptive Trigger component (VERIFIED). Feel has multi-gamepad haptics and generates AHAP from audio (VERIFIED). Interhaptics (free) and Meta Haptics Studio do audio-to-haptics and HD haptics (VERIFIED). |
| Audio | **Parity** | FeelKit: 2D/attached/location, seeded pitch/volume, fade. Game Juice Pro: random selection, pitch/volume variation, attenuation, attach (VERIFIED). Feel: sound manager with tracks and an audio analyzer (VERIFIED). |
| Debugging | **Behind (runtime), ahead (editor graph)** | FeelKit has a per-channel intensity graph in the editor but no runtime debugger (TL-002 not built). Reflex ships an in-world 3D debugger (VERIFIED). Gameplay Cameras has a camera debugger (VERIFIED). |
| AI capabilities | **Parity (nobody real), opportunity** | "Agentic" FeedbackFX means tag-driven automation, not AI (VERIFIED from its own page). Epic's UE 5.8 Unreal MCP lets plugins expose C++ `AICallable` toolsets (VERIFIED, experimental). |
| Content, presets, demos | **Behind** | Game Juice Pro has a preset video and a playable demo (VERIFIED links). FeedbackFX ships 6 hitstop and 6 post-process presets (VERIFIED). Feel ships many demos (VERIFIED). FeelKit has 0 shipped recipes and no demo map yet. |
| Docs and onboarding | **Behind** | Every competitor has docs. FeelKit has none yet (DOC-001..008 not started). |
| Performance claims | **Parity** | FeelKit: no tick when idle (tested). Nobody publishes numbers (INFERRED). Pooling (RT-004) is not done. |
| Engine version range | **Behind** | Game Juice Pro 5.3–5.8, GameJuice 5.4–5.7, FeedbackFX 5.6–5.8 on Fab (all VERIFIED). FeelKit is built and tested only on 5.6 so far. |
| Price | **Behind** | Planned $70–80 vs $15–40 for Unreal competitors. Feel lists at €46 and was €23 on sale on 2026-09-16 (VERIFIED). |
| Community and support | **Behind** | Feel's site claims a 4000+ Discord (VERIFIED claim). FeedbackFX, Cronus and ECSP all link Discords (VERIFIED). FeelKit has none. |

### B. What must change to be clearly differentiated

Being "the one with a timeline" will not win on its own. A buyer compares feature grids first and workflow second. To win, FeelKit has to (1) **close the table-stakes gaps** so the grid comparison is at least a draw, and (2) **ship 2–3 things nobody can copy in a weekend**. Those should build on pure evaluation and comfort, the two assets competitors lack architecturally.

**Close before launch (table stakes, not differentiators):**
1. **Evaluate `FFeelConditions`** (Chance, MaxDistance, LocalPlayerOnly), add **distance falloff curves** and **random intensity/duration ranges**. The data model already exists. Cost S–M.
2. **Recipe parameters** (strategy 3.3), e.g. `Damage` or `Distance` mapped per track through curves. This is the answer to Game Juice Pro's intensity interval and FeedbackFX's magnitude scaling. Cost M.
3. **Triggers:** anim notify (API-006), damage/hit/landed trigger component (TRG-001..004), and an optional GAS module (tag and attribute triggers, the FeedbackFX pitch). Cost M.
4. **Networking** (NET-002..003): Multicast / OwnerOnly / SkipOwner plus relevancy. Game Juice Pro sets the bar. Cost M.
5. **Step breadth to about 30**, favoring what reviews and listings show buyers expect: Niagara spawn, bloom/DoF/color grade, camera roll/tilt, blink/flicker/visibility, rim/fresnel flash, ghost trail, UI punch/shake, adaptive trigger, nested recipe, random choice. Cost L.
6. **Recipes, demo map, playable demo, docs, Discord.** Cost L.
7. **Fix the comfort compliance flaw:** a player-chosen 0 on Camera Shake or Camera Motion must mean 0. Make EssentialFloor never apply to motion groups, or require a non-motion substitute there, and warn in validation. Cost S.

**Build to differentiate (see section 6):** moment capture and scrub from PIE, a comfort audit report plus scaling of engine-native shakes, live parameter sliders in preview, audio-synced authoring (waveform, transient snap, audio-derived shake and haptics), and an MCP/JSON authoring surface.

---

## 2. Competitor profiles

### 2.1 Game Juice Pro (SpanZeto's Dev Tools), Unreal / Fab. **The primary threat.**

Sources (accessed 2026-09-16): [Fab listing](https://www.fab.com/listings/05737cd2-05de-48ad-a52b-ded62a390987), [docs index](https://spanzeto.dev/docs/game-juice-pro/), [feedback catalog](https://spanzeto.dev/docs/game-juice-pro/feedbacks/), [advanced config](https://spanzeto.dev/docs/game-juice-pro/configuracion/), [usage guide](https://spanzeto.dev/docs/game-juice-pro/uso/), [concepts](https://spanzeto.dev/docs/game-juice-pro/conceptos/), [multiplayer](https://spanzeto.dev/docs/game-juice-pro/multijugador/).

| Dimension | Finding | Label |
|---|---|---|
| Authoring | "Juice Preset" Data Asset with a "Feedback Templates" array edited in the Details panel. Preset-level intensity multiplier, duration multiplier, sequential toggle. No custom editor mentioned. | VERIFIED |
| Runtime | Session-based, "independent feedback cloning per execution". Play Juice node returns a Session ID. Stop Session / Preset / All. A **Game Juice Component is mandatory on actors** receiving effects. | VERIFIED |
| Feedbacks (28) | Scale, Squash & Stretch (spring, volume preserving), Jiggle, Look At, Bounce, Push (LaunchCharacter), Bob, Spin, Magnetic, Blink, Flicker, Material Flash, Ghost, Rim Glow, Fade, Chromatic Aberration, Bloom, DoF, Color Grade, Time Scale (CustomTimeDilation), Sound, Rumble, Camera Shake (simple or CameraShakeBase asset, distance attenuation), Camera Zoom, Particles (Niagara/Cascade), Spawn Object, Enable, Debug. | VERIFIED |
| Editor tooling / preview | None described. Testing: "Ejecuta el juego y dispara el evento" (run the game and fire the event). "Dynamic Blueprint Node": pins appear based on preset content. | VERIFIED |
| Parameterization | Cascading intensity (component, preset, call, distance). "Intensity Interval" (feedback fires only within an intensity range). Distance falloff curves. Random intensity and duration. Chance 0–100. Scaled or unscaled time. Repeats, delays, play-count limits. Direction conditions (OnlyWhenForwards/Backwards). Channels by index or Gameplay Tag. | VERIFIED |
| Comfort | None documented. | INFERRED (absent from docs read) |
| Haptics | Rumble with left/right motor intensities, "Affect All Players". | VERIFIED |
| Audio | Random selection, pitch/volume variation, 3D attenuation, attach to target. | VERIFIED |
| Networking | Local Only, Server Multicast, Server Authoritative, Owner Only, Skip Owner. Relevancy distance (default 5000). "Only Replicate If Visible". Automatic network proxy component. | VERIFIED |
| Triggers | Blueprint node or component auto-play. No anim notify or collision triggers mentioned. | VERIFIED / INFERRED |
| Debugging | A "Debug" feedback (on-screen/log messages). | VERIFIED |
| AI | None. Listing says "Allows usage with AI: Yes" (a Fab licensing flag, not a feature). | VERIFIED |
| Content / demos | Presets, a Google Drive demo, a trailer and a "Preset video demo". | VERIFIED (links) |
| Engines / platforms | UE 5.3–5.8. Win/Mac/Linux. | VERIFIED |
| Price | $29.99 starting (Fab search). Per-license €32.22–€48.34 shown incl. VAT. "Price will increase as the feature set expands." | VERIFIED |
| Cadence | Published 2026-02-17. Updates 02-18, 02-22 (PIE crash fix), 04-05 (attached actors fix), 06-26. | VERIFIED |
| Reviews | 5.0 from 6 ratings, 5 written reviews, all positive. Themes: presets, easy, multiplayer "replicated automatically", support. First review (2026-02-17): Unity has had Feel for years and Unreal lacked "a solid equivalent". | VERIFIED |
| DX overall | Feel-like mental model. Docs are Spanish-first. Requires a component on every target actor. | VERIFIED / INFERRED |

**Red team read:** Game Juice Pro copied Feel's *runtime model* (direction, chance, timing, channels, intensity layers) and added Unreal networking. It did not copy Feel's inspector UX, and nothing is authored visually. If SpanZeto adds a preview button, FeelKit's "preview without PIE" pitch shrinks to "our preview is better", a harder sell.

### 2.2 Agentic FeedbackFX (Insodimension), Unreal / Fab

Sources: [Fab listing](https://www.fab.com/listings/ad3d6526-fca0-4760-99b2-9adf368080b6), [product page](https://insodimension.com/products/feedbackfx), [forum thread](https://forums.unrealengine.com/t/insodimension-agentic-feedbackfx-screenfx-hitstop-impact-frames-haptics-shakes-c/2695683).

| Dimension | Finding | Label |
|---|---|---|
| Authoring | "Everything lives in one Data Asset config". Gameplay-tag driven: "Fire a Tag. Feel the Hit." | VERIFIED |
| Events (7 on site, 6 on Fab) | Camera Shake (magnitude scaled), Hit Stop (6 recovery curves), Post Process temp, Post Process persistent, Impact Frame (flash or Niagara), Audio, Haptics. | VERIFIED |
| Triggers | Attribute Threshold (with hysteresis) and Status Effect tag triggers, both via GAS. Extensible by subclassing `UAgenticFeedbackFXTrigger`. | VERIFIED |
| Parameterization | Magnitude scaling on every effect. Custom params passed to materials and Niagara. | VERIFIED |
| Preview / editor | None described. | INFERRED |
| Comfort | None. | INFERRED |
| Networking | "Multiplayer Safe, LocalVisual mode" hitstop applies dilation only to locally controlled actors. | VERIFIED |
| AI | "Agentic" is branding for automation. The studio brands itself "AI-Native Game Development". No LLM feature described. | VERIFIED |
| Requirements | UE 5.5+. The Fab listing says GAS and Niagara "Required". The FAQ says GAS is "Optional". Contradictory. | VERIFIED |
| Price | $39.99 on Fab. The product page says "GET FREE ON FAB", and a 2026-01-29 review thanks the author "for making it free for personal". Pricing likely changed after launch. | VERIFIED (conflict) |
| Cadence | Published 2026-01-29. Updates 07-08, 08-03, 09-11 (no notes). | VERIFIED |
| Reviews | 5.0 from 13 ratings. One reviewer owned "more than 4 screenfx assets" and says the plugin replaced their spaghetti Blueprints. | VERIFIED |

**Red team read:** its angle is status-driven persistent screen effects (low health vignette, burning), which FeelKit does not cover at all. FeelKit is fire-and-forget only, with no persistent or state-bound effects. It is also the closest to "GAS Gameplay Cues, done nicely".

### 2.3 GAME JUICE (Adrenaline Games), Unreal / Fab

Sources: [docs](https://adrenalinegames.pl/GameJuice.html), [studio site](https://adrenalinegames.pl/), Fab listing uid `536cb70a-1fa3-4bf4-96f8-9cacfb659f29` (read via Fab JSON).

| Dimension | Finding | Label |
|---|---|---|
| Authoring | 20 C++ components configured in the Details panel. Presets for shake, rumble, flash. `UCurveFloat` curves. | VERIFIED |
| Components | Camera Shake (distance falloff), FOV Punch, Camera Tilt, Lookahead, Camera Impulse, Hitstop, Slow-Mo, Time Dilation, Gamepad Rumble, **PS5 Adaptive Trigger**, Screen Flash, Speedlines, Damage Vignette, Chromatic Aberration, Coyote Time, Jump Buffer, Anticipation, Hit Reaction, Material Flash, Audio Pitch Variation. | VERIFIED |
| Comfort | `Global Intensity Scale` (0–3) only. | VERIFIED |
| Networking | Server multicast, per-observer distance falloff, dedicated-server skip. | VERIFIED |
| Runtime | "Real-time tickers (effects work even at TimeDilation = 0)". Non-destructive post-process. | VERIFIED |
| Preview | None. | VERIFIED (Details panel, call functions) |
| Engines | 5.4–5.7. | VERIFIED |
| Price | $19.99 on Fab. The studio site lists $10.99. | VERIFIED (conflict) |
| Cadence | Published 2026-05-06, 7 updates to 2026-08-20. | VERIFIED |
| Reviews | 0 ratings. | VERIFIED |

### 2.4 Cronus Hitstop (TheFirstOnes), Unreal / Fab

Source: Fab listing `31adfbe6-c4c9-42d6-afba-31af742f648d` (Fab JSON and page).

- Hitstop-only system. Tracks active hitstops and "automatically prioritize more important hitstops". Async node with completed/cancelled pins. 5 cosmetic classes (mesh shake, material blink, audio ducking, dynamic camera shake, container). Curves, noise, frame or time update rate, "Slice Attacks" slow instead of freeze, a class-browser menu entry. (VERIFIED)
- $29.99, 5.0 from 7 ratings. Published 2024-05-03, updates 2025-07, 2025-12, 2026-07. Has a Discord. (VERIFIED)
- **Where it beats FeelKit:** hitstop depth (slice mode, per-actor rules via interface, audio ducking during stop, async completion node). FeelKit's Actor Hitstop is simpler. (INFERRED)

### 2.5 Other Unreal / Fab listings in the space

All read via Fab JSON on 2026-09-16 (VERIFIED unless noted):

| Product | Price | Rating | What it is | Relevance |
|---|---|---|---|---|
| Enhanced Camera Shake Patterns (Hubert Mika) | $14.99 | 5.0 (5) | Extends native shake patterns: curve pattern with weighted random curve sets, eased blending, **parameter mapping** of amplitude/frequency to a float. Works in Sequencer. | Shows buyers want parameterized native shakes. FeelKit cannot import or scale them yet. |
| Natural Impacts – Damage Based Game Feedback (Blueprint House) | $49.99 | 5.0 (4) | Body and camera effects, "Integrated with UE's Default Damage Events", impact profiles per damage type. | Price point near FeelKit's. Trigger-first pitch. |
| Feedback Event Factory (Masonic Studios) | $14.99 | 4.43 (7) | Aggregates sounds, particles, force feedback, shakes. Owner-only vs all players. Since 2018. | Old, shows long-standing demand for "one place". |
| Reflex (LeleDev) | $39.99 | 5.0 (3) | Hit reactions/physics with profile data assets, rules (damage thresholds, chance, tags, bones), **integrated 3D visual debugger**. | Sets a debugging bar. |
| FPS Weapon Feel Component | $19.99 | 0 | Recoil/sway/bob with a **custom 2D recoil pattern editor with undo**. | Shows small sellers now ship custom editors. A custom editor alone is not unique. |
| Easy Cartoon Effects / Spline Juice (TechnicallyArtist) | $29.99 | 5.0 (1) | WPO squash/stretch/wobble/melt, templates. 20+ updates since 2026-06. | Competes on squash and stretch visuals. |
| Unreal Engine Accessibility Toolkit (Dink) | Free | 4.8 (15) | Visibility modes, subtitles, remapping Blueprints. | Accessibility demand exists. No motion or flash comfort. |
| AccessCheck (ECal Studios) | $69.99 | 0 | Editor-only UMG accessibility auditor. 21 rules. Its 31-item manual checklist covers "motion, flashing". Published 2026-07-27. | **Proves a $70 accessibility-tooling price point is being tried**, and that motion/flash audit is currently a manual checklist. |
| SDL Enhanced Input (Ares9323) | $14.99 | 5.0 (5) | SDL3 bridge: DualSense adaptive triggers, advanced haptics on non-XInput pads. | Solves the "PlayStation pad does not rumble on Windows" gap FeelKit documents. |
| Camera Shake Pro (SKAVA), Vibe Engine (Dizzy Media), All In 1 Springs | $15.99, $59.99, $39.99 | 0 | **Unity** assets sold on Fab. Vibe Engine: haptics studio with preview and comparison tools. | Fab search mixes Unity assets into Unreal results, adding noise for "game feel" searches. |
| Combat frameworks: ACF V4 ($349.99, 4.57/296), Generic Combat System ($119.99, 5.0/40), Tempest ($149.99, 4.77/115) | — | — | ACF changelog: "HitStop effects". GCS results "drive ... feedback". | Framework owners get basic feel bundled. FeelKit must integrate with them, not compete. |

Fab search quality (VERIFIED): the query "game feel" returns no feel plugins in the top 15. "juice" returns Game Juice Pro first. "hitstop" returns Cronus first. **Discoverability runs on the literal words "juice" and "hitstop", not "feel".**

### 2.6 Unreal native (free)

| Capability | Finding | Label / Source |
|---|---|---|
| Camera shakes | Perlin, Wave, **Sequence** (camera animation authored in Sequencer), Composite layering. Play spaces. Scale multiplier. **Camera Shake Previewer** (Window menu) plays shake sources in the editor without PIE. No accessibility or global scale notes. | VERIFIED, [Epic docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/camera-shakes-in-unreal-engine) |
| Force Feedback Effect assets | Per-channel curves per motor, hover "play" preview in the Content Browser, attenuation. DualSense trigger resistance/vibration and audio-to-speaker with `VibrationMode=Advanced`. | VERIFIED, [Epic docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/force-feedback-in-unreal-engine) |
| GAS Gameplay Cues | `GameplayCueNotify_Burst/Looping` with burst/looping camera shake, force feedback, particles, sounds. Tag-driven and replicated through GAS. | VERIFIED, [Python API](https://docs.unrealengine.com/5.0/en-US/PythonAPI/class/GameplayCueNotify_BurstEffects.html) |
| Gameplay Cameras (GPC) | Camera rigs as assets with a CameraShake node. Camera debugger. Experimental in 5.7, planned Beta for 5.8. A third-party guide says "Editor preview is not always accurate". | VERIFIED, [Chabant 5.7](https://ludovic.chabant.com/blog/2025/11/14/ue5-gameplay-cameras-upgrading-to-5-7/), [StraySpark](https://www.strayspark.studio/blog/gameplay-cameras-plugin-ue5-7-production-guide) |
| UE 5.8 (released 2026-06-17) | Headlines: Unreal MCP server plugin, MegaLights, mesh terrain, Lumen Lite. **Nothing** on shakes, feedback, haptics or comfort in the release notes I read (truncated page). | VERIFIED, [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/), [5.8 notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes) |
| Unreal MCP (5.8, Experimental) | Plugins can expose tools via Toolset Registry: C++ `UToolsetDefinition` with `UFUNCTION(meta=(AICallable))`, or Python toolsets in `Content/Python`, or `IModelContextProtocolTool`. | VERIFIED, [Epic docs](https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?lang=en-US) |
| Lyra | Settings include a force feedback toggle and colorblind options. No shake comfort. | INFERRED (source fetch failed) |

**Red team read:** a studio with GAS already has tag-driven, replicated, multi-channel burst feedback for free. Sequence shakes give a free Sequencer timeline for camera motion, and the previewer gives free preview for that one channel. The free stack lacks composition across channels, a preview of the composition, time arbitration, comfort, and non-camera effects on one timeline.

### 2.7 Feel (More Mountains), Unity. **The category benchmark.**

Sources: [site](https://feel.moremountains.com/), [docs](https://feel-docs.moremountains.com/), [MMF_Player](https://feel-docs.moremountains.com/mmf-player.html), [releases](https://feel.moremountains.com/feel-releases), [Asset Store](https://assetstore.unity.com/packages/tools/particles-effects/feel-183370), [reviews](https://assetstore.unity.com/packages/tools/particles-effects/feel-183370/reviews), [MMFeedbacks docs](https://feel-docs.moremountains.com/mmfeedbacks.html).

| Dimension | Finding | Label |
|---|---|---|
| Authoring | MMF_Player component with a stacked feedback list in a UIToolkit inspector (v5.0, Nov 2024). Search menu to add feedbacks (v5.3). Double-click to rename (v6.0) or change duration (v6.1). **No timeline view.** | VERIFIED |
| Breadth | 150+ feedbacks across audio, camera, post-process, UI, transform, particles, time, events, and more. "70 springs" per a third-party copy of the description. Extra tools: Nice Vibrations, MMSoundManager, MMAudioAnalyzer, MMSequencer, MMTimeManager, MMFloatingText. | VERIFIED (150+, tools); INFERRED (70 springs) |
| Runtime model | Parallel or (since v6.0) sequential playback. Direction (top-to-bottom, reverse, auto change). Intensity. Scaled or unscaled time. Initial delay, cooldown, repeat. Chance. Random duration. Range with falloff. Events (Play, Pause, Resume, Revert, Complete). Channels to trigger players without references. | VERIFIED |
| Preview / tuning | Inspector Play/Stop/Reset buttons in play mode. **"Keep Playmode Changes"** keeps runtime tweaks after exiting play mode. Copy All / Paste. **v6.0 (Jul 2026): "new preview mode to test feedbacks without having to enter play mode"**. It is experimental, must be enabled in Tools, and "only some feedback have a preview mode". | VERIFIED |
| Comfort | A global static switch `GlobalMMFeedbacksActive`. No per-player categories or presets found. | VERIFIED (switch); INFERRED (no presets) |
| Haptics | Nice Vibrations included. Multi-gamepad haptics (v5.9). NV Clip feedback generates AHAP from adjacent audio (v5.7). | VERIFIED |
| AI | Nothing in releases v5.0–v6.1. | VERIFIED |
| Price / popularity | €46 list, €23 on 50% sale (2026-09-16). 238–239 reviews, 9,067 favorites. Site claims a 4000+ user Discord. Won Unity Awards 2021 Best Artistic Tool (search snippet). | VERIFIED (price, counts); INFERRED (award) |
| Cadence | About monthly: v5.0 Nov 2024 through v6.1 Sep 2026. v6.x requires Unity 6000.5. | VERIFIED |
| Complaints | Visible reviews are 5-star. A search snippet quotes one negative review: won't compile on newer Unity and installs into the root folder. | VERIFIED (visible) / INFERRED (snippet) |
| Unreal version | None found from More Mountains. A piracy index lists "Feel V5.8 For Unreal Engine", but its description is the Unity asset's text, so it is almost certainly mislabeled. I did not open it. | INFERRED |

**Red team read:** Feel's moat is breadth, community and 5+ years of trust, not UX. Its v6.0 preview mode shows the benchmark moving toward FeelKit's pillar 2. By 2027, "preview without play mode" will be expected, not special.

### 2.8 Other Unity

- **Cinemachine Impulse** (free): Impulse Source/Listener with channel filtering like layers. Camera-only. (VERIFIED, [Unity docs](https://docs.unity3d.com/Packages/com.unity.cinemachine@2.9/manual/CinemachineImpulseSource.html))
- **Motion Suite – Game Feel Toolkit** (SpankyBoy): €14.71, released 2026-01-05, needs DOTween, marked "Created with AI". (VERIFIED, [Asset Store](https://assetstore.unity.com/packages/tools/animation/motion-suite-game-feel-toolkit-331892))
- **Vibe Engine** (Unity, also on Fab at $59.99): haptics "editor studio" with preview, visualization and comparison, plus Steam Input and console provider shells. (VERIFIED, Fab JSON)

### 2.9 Godot

- **Sparkle** (Neohex Interactive): 34 feedback types, FeedbackPlayer, "Preview without entering play mode", $20. A free Lite version is on the Asset Library. (VERIFIED, [itch](https://neohex-interactive.itch.io/sparkle), [Asset Library](https://godotengine.org/asset-library/asset/5066))
- **Juicee** (kelpekk): MIT, v1.2.0, 2026-06-27. Visual **graph editor** for shake, hit-stop, damage numbers, springs, shaders. "Inspired by FEEL". (VERIFIED, [Asset Library](https://godotengine.org/asset-library/asset/5218))
- Several itch.io Godot juice packs, $5–15 range (INFERRED from titles).

**Read:** in-editor preview and visual authoring are becoming common even in cheap and free tools.

### 2.10 Haptics authoring

- **Interhaptics (Razer/WYVRN):** free Haptic Composer + Unreal SDK. DualSense HD haptics and adaptive triggers, GameInput/XInput, Razer Sensa. (VERIFIED, [Wyvrn docs](https://doc.wyvrn.com/docs/interhaptics-sdk/haptic-composer/))
- **Meta Haptics Studio + Haptics SDK for Unreal:** audio-to-haptics analysis, audition, export clips. Quest controllers. Wwise native `.haptic` support in early 2026. (VERIFIED, [Meta docs](https://developers.meta.com/horizon/documentation/unreal/unreal-haptics-sdk/))
- **Read:** "audio in, haptics out" is the professional standard. A hand-drawn per-motor curve is a basic implementation. (INFERRED)

### 2.11 Accessibility standards (buyer-side requirement, not a competitor)

- **Xbox Accessibility Feature Tag "Camera Comfort"** (updated 2026-08): players must be able to turn off or adjust screen shake, bob, motion blur, arm sway and narrative camera movement. A slider must reach zero or have an off option. **Fail examples:** "an option to disable it but still the screen still shakes", and a slider with a minimum of 10%. (VERIFIED, [Microsoft Learn](https://learn.microsoft.com/en-us/xbox/accessibility/accessibility-feature-tags))
- XAG 118 covers photosensitivity with flashing thresholds. (VERIFIED title and summary, [XAG 118](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/118))

---

## 3. Comparison matrix

Legend: ✔ strong, ◐ partial, ✖ none, ? unknown. FeelKit column = current built state (2026-09-16).

| Dimension | FeelKit | Game Juice Pro | FeedbackFX | GameJuice (Adrenaline) | Cronus Hitstop | UE native (shakes + GAS cues + FF) | Feel (Unity) | Sparkle / Juicee (Godot) |
|---|---|---|---|---|---|---|---|---|
| Visual timeline authoring | ✔ | ✖ | ✖ | ✖ | ✖ | ◐ (Sequence shake in Sequencer, camera only) | ✖ (list) | ✖ / ◐ graph |
| Preview without play | ✔ all channels, scrub | ✖ | ✖ | ✖ | ✖ | ◐ shake previewer, FF hover play | ◐ experimental, some feedbacks | ✔ (Sparkle) |
| Keep runtime tweaks | ◐ edits in PIE apply next play | ✖ | ✖ | ✖ | ✖ | ✖ | ✔ Keep Playmode Changes | ? |
| Effect count | ~15 | 28 | 7 | 20 | 1 system + 5 cosmetics | many primitives | 150+ | 34 / ? |
| Chance / randomization | ◐ seed only | ✔ | ◐ pitch variation | ◐ | ◐ noise | ◐ | ✔ | ? |
| Distance falloff | ✖ | ✔ | ? | ✔ | ✔ camera distance curves | ✔ shake/FF attenuation | ✔ | ? |
| Magnitude / named params | ✖ | ◐ intensity interval, dynamic pins | ✔ magnitude + context params | ✖ | ◐ | ◐ (GAS cue magnitude) | ◐ intensity | ? |
| Triggers without code | ✖ | ◐ autoplay | ✔ GAS attribute/status | ✖ | ✖ | ✔ GAS tags | ◐ autoplay, channels | ? |
| Persistent / state effects | ✖ | ◐ infinite bob/spin | ✔ | ◐ vignette by HP | ✖ | ✔ looping cues | ✔ | ? |
| Cross-channel arbitration | ✔ | ◐ scale stacking guard | ✖ | ✖ | ◐ hitstop priority | ✖ | ✖ | ? |
| Time dilation safety / restore | ✔ tested | ? | ◐ LocalVisual | ◐ real-time tickers | ✔ | ✖ | ✔ unfreeze options | ? |
| Per-player comfort + presets + save | ✔ | ✖ | ✖ | ◐ global scale | ✖ | ✖ | ◐ global switch | ? |
| Networking | ✖ | ✔ 5 modes + relevancy | ◐ | ✔ | ? | ✔ via GAS | n/a (Unity) | ? |
| Haptics | ◐ per-motor curve | ◐ motors | ◐ motors | ✔ + adaptive trigger | ✖ | ✔ FF assets, DualSense triggers | ✔ Nice Vibrations, AHAP from audio | ? |
| Audio | ✔ | ✔ | ◐ | ◐ pitch variation | ◐ ducking | ✔ | ✔ | ◐ |
| Runtime debugger | ✖ | ◐ debug feedback | ? | ? | ? | ◐ showdebug, GPC debugger | ? | ? |
| AI authoring | ✖ | ✖ | ✖ (branding) | ✖ | ✖ | ◐ MCP (5.8 exp.) | ✖ | ✖ |
| Presets / demos | ✖ | ✔ | ◐ 12 curve presets | ◐ | ◐ | ◐ templates | ✔ | ◐ |
| Docs | ✖ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| Engine range | 5.6 (5.7/5.8 planned) | 5.3–5.8 | 5.6–5.8 | 5.4–5.7 | ? | all | Unity 6000.5+ | Godot 4 |
| Price (USD start) | planned $70–80 | $29.99 | $39.99 | $19.99 | $29.99 | free | ~$50 list | $20 / free |
| Social proof | none | 5.0 (6) | 5.0 (13) | 0 | 5.0 (7) | n/a | 238 reviews, 9k favs | small |

---

## 4. Red team: the strongest arguments against FeelKit

| # | Argument | Valid? | How to neutralize |
|---|---|---|---|
| R1 | "Game Juice Pro already is Feel for Unreal, for $30, with 28 effects and multiplayer." | **Valid today.** On a feature grid FeelKit loses ~15 vs 28 and has no networking. | Close grid gaps (section 1B items 1–5). Then make the trailer a head-to-head: tune the same hit in Game Juice Pro (PIE loop) and in FeelKit (scrub, capture, tweak). Sell time-to-good-feel, not effect count. |
| R2 | "GAS Gameplay Cues + camera shake assets + Force Feedback assets are free, replicated and tag driven." | **Valid for GAS studios.** Cues already orchestrate multi-channel bursts. | Do not fight GAS, ride it: an optional `FeelGAS` module with a `GameplayCueNotify_PlayFeel` (cue plays a recipe with magnitude mapped to recipe parameters). Pitch: "keep your cues, author what they play in a timeline, and make it respect comfort". |
| R3 | "Epic's Camera Shake Previewer and Sequence shakes already give preview and timelines." | **Partially valid** (camera channel only). | Import native shakes as tracks (strategy 3.7). Show a single timeline composing shake + hitstop + flash + sound + haptics, which no native tool does. |
| R4 | "$70–80 is too much. Everything in this category is $15–40, and Feel is €23 on sale." | **Valid.** Ratings volume signals a small market: 6, 13, 7 and 0 ratings for the four main UE listings (VERIFIED). Ratings undercount sales, but by an unknown factor (INFERRED). | Launch at $34.99–39.99 early access (requirements already say about $39.99). Free Lite with comfort. Raise the price only after capture, parameters and triggers ship. Or split: Core $39.99, a Pro tier with studio tooling (comfort audit, capture, MCP) at $79–99 aimed at teams. AccessCheck is testing $69.99 for audit tooling. |
| R5 | "A timeline is nice, but I need triggers. I won't wire Blueprint calls everywhere." | **Valid.** FeedbackFX's GAS triggers and Natural Impacts' damage events are their pitch. | Anim notify + trigger component + GAS cue module (TRG-001..004, API-006) before launch. |
| R6 | "Name collision with More Mountains 'Feel'." | **Valid risk, double-edged.** FeelKit, the `Feel` prefix and `PlayFeel` sit right next to a famous brand, and Game Juice Pro reviewers already frame the market as "Feel for Unreal". Upside: people search "Feel for Unreal". Downside: confusion, possible trademark complaint, and Fab search for "feel" returns junk (VERIFIED). | OD-001 is still open, and the `Feel` prefix is baked into saved assets. Decide now. If renaming, use core redirects (`[CoreRedirects]` class/struct/package) so internal test assets survive. For discoverability, put "juice", "hitstop", "screen shake", "game feel" in the Fab title and tags, because that is how Fab search works (VERIFIED). |
| R7 | "Comfort is a nice checkbox, but my shakes come from Sequencer, anim notifies and GAS cues, not your recipes." | **Valid.** FeelKit comfort only scales FeelKit-played effects. That does not earn the Xbox Camera Comfort tag, which tests all gameplay camera effects. | Build strategy 3.1: scale engine-native camera shakes and force feedback by comfort group. Plausible stock hooks: a camera modifier that scales `UCameraShakeBase` instances by class-to-channel mapping, and `APlayerController::ForceFeedbackScale` (**needs a spike**). Ship an audit that lists every shake source in the project (section 6, D2). |
| R8 | "Your EssentialFloor fails certification." | **Valid (new finding).** XAG fails titles where the screen still shakes after the player disables it. | For comfort groups Camera Shake and Camera Motion, 0 must mean 0. Allow only non-motion substitutes (for example flash or haptic cue), and validate. |
| R9 | "Feel 6.0 has preview mode now, and Sparkle previews too. Your preview is not special." | **Becoming valid.** | The moat has to move from "preview" to "capture, scrub, parameterize and verify": things a list-based inspector cannot do (section 6). |
| R10 | "15 effects, no docs, no demo, no reviews, one engine version, solo developer." | **Valid at launch risk.** | Content and docs phase with a playable demo, as Game Juice Pro and FPS Weapon Feel do (VERIFIED). Support 5.6–5.8 at launch (C-001 already requires it). |
| R11 | "No networking: useless for my multiplayer game." | **Valid.** | NET-002..005. Local-only default with explicit modes, relevancy and a documented hitstop-in-MP policy (FeedbackFX's LocalVisual is a good precedent). |
| R12 | "Pure evaluation and arbiters are engineering pride, not features." | **Valid as marketing.** | Only market what they enable: "scrub your actual last hit", "hitstop never leaves time slowed", "shakes never stack into nausea". |

---

## 5. Gaps where others are ahead

Cost: S under 1 week, M 1–3 weeks, L over 3 weeks (solo developer, INFERRED).

| # | Gap | Leader | Close it? | Differentiate around it? | Cost | Req / strategy |
|---|---|---|---|---|---|---|
| G1 | Chance, random ranges, distance falloff (Conditions not evaluated) | Game Juice Pro, Feel | **Yes, must** | Partly: show falloff live in preview with a draggable listener distance | S–M | 4.2 Conditions, strategy 3.3 |
| G2 | Magnitude / named inputs | FeedbackFX, Game Juice Pro | **Yes** | **Yes**: recipe parameters with live sliders and per-value intensity graph (D3) | M | strategy 3.3, TRG-004, PV-007 |
| G3 | Triggers without code (anim notify, damage, collision, GAS tags) | FeedbackFX, Natural Impacts, GAS | **Yes, must** | Partly: GAS cue that plays a recipe | M | API-006, TRG-001..004, C-003 |
| G4 | Networking | Game Juice Pro | **Yes, must** | No, pure parity | M | NET-001..005 |
| G5 | Effect breadth (~15 vs 20–28 vs 150) | Feel, Game Juice Pro | To about 30 | Yes: Blueprint step SDK + community steps (strategy 3.9), native shake import (3.7) | L | ST-004..037 |
| G6 | Persistent / state-bound effects (low health vignette, burning) | FeedbackFX, GAS looping cues | Yes | Comfort applies to persistent effects too, which nobody else does | M | New requirement (looping recipes, Stop on condition) |
| G7 | Presets, demo map, playable demo, docs | all | **Yes, must** | Animated thumbnails + recipes organized by feeling (strategy 3.8) | L | CT-001..006, DOC-001..008, TL-005 |
| G8 | Haptics depth (adaptive triggers, audio-to-haptics, non-XInput pads) | GameJuice, Feel, Interhaptics, SDL Enhanced Input | Partly (adaptive trigger via engine API) | **Yes**: derive haptics from the Play Sound track waveform in-editor (D4) | M | ST-027, ST-028, OD-005 |
| G9 | Engine range 5.3/5.4–5.8 | Game Juice Pro | Only to 5.6–5.8 (content rule C-001c) | No | S–M per version | C-001 |
| G10 | Runtime debugger | Reflex, GPC | Yes | **Yes**: arbiter decisions drawn on the timeline of a captured moment (D1) | M | TL-002, TL-003, ARB-005 |
| G11 | Keep runtime tweaks back to asset | Feel | Yes | Yes: tweak inside the captured moment, then "Apply to recipe" | S–M | PV-009 |
| G12 | Price / social proof / community | everyone | Partly | Free Lite with comfort (strategy 3.1) builds installs and reviews | S | §13, K-006 |
| G13 | Comfort covers only FeelKit effects; EssentialFloor fails XAG | nobody ahead, but the standard requires it | **Yes, must** | **Yes** (D2) | S (floor rule), M–L (native scaling spike) | CMF-021, strategy 3.1 |
| G14 | Discoverability ("feel" is a bad search term on Fab) | Game Juice Pro owns "juice" | Yes via title/tags | Name decision | S | OD-001 |

---

## 6. Candidate unique capabilities (ranked by leverage)

Leverage = (how hard for a competitor to copy) x (how visible in a 30-second trailer) x (how many buyers it matters to) / cost.

### D1. Moment Capture: "hit the enemy, click, scrub that exact hit" — Highest leverage
- The subsystem keeps a ring buffer of the last ~10 s of plays (recipe, seed, intensity, parameters, comfort, arbiter decisions, camera transform). "Open in Timeline" replays that moment in the recipe editor with overlays showing what was capped, suppressed or substituted. Edit it and "Apply to recipe".
- **Why others cannot copy quickly:** it needs deterministic evaluation (identical frames from time + seed) and a shared evaluator. Game Juice Pro clones feedback objects per session with timers and springs (VERIFIED "timer-based sequential scheduling", "spring physics"), so it cannot replay a moment exactly without a rewrite (INFERRED). Feel's feedbacks are coroutine-driven (VERIFIED "coroutine optimizations"), so the same applies (INFERRED).
- Also fills G10 (debugger) and G11 (keep runtime tweaks).
- Touches: strategy 3.2, PV-005, PV-009, TL-002, ARB-005, NF-005. Cost M–L.

### D2. Comfort you can certify: native effect scaling + Comfort Audit report
- (a) Scale engine-native camera shakes and force feedback through FeelKit comfort groups, so "Reduced Motion" works on a project's existing effects. (b) An editor **Comfort Audit**: scan recipes, camera shake assets, anim notifies and GAS cue notifies that play shakes/FF. Flag essential motion with a floor above 0 (XAG fail), missing substitutes, flash tracks above a rate threshold (preview-based luminance-change counter, CMF-030..033). Export a report mapping findings to the XAG Camera Comfort tag. Wording must be "readiness helper", not certification (§2.2).
- **Why it matters:** AccessCheck (July 2026, $69.99) shows people pay for audit tooling, and it leaves motion and flashing as a manual checklist (VERIFIED). No juice plugin touches comfort. A studio lead buys this; a solo dev gets the free Lite.
- Touches: strategy 3.1, CMF-021/023, CMF-030..042, TL-004. Cost M (audit), L (native scaling, spike first).

### D3. Recipe parameters with live preview sliders
- Named inputs (`Damage`, `Distance`, `Speed`) mapped per track through curves. The preview panel shows sliders, and the intensity graph (PV-007) redraws per value. Distance falloff is a built-in parameter with a draggable listener in the viewport. Output: one "Hit" recipe covers scratch to critical, and the designer *sees* both ends without PIE.
- Competitors have scalar intensity (Game Juice Pro intensity interval, FeedbackFX magnitude) but no visualization of the whole range (VERIFIED absence of editors).
- Touches: strategy 3.3, TRG-004, PV-007. Cost M.

### D4. Audio-driven authoring: waveform, transient snap, audio to shake/haptics
- Draw the waveform on Play Sound tracks. Snap tracks to detected transients. "Generate from audio" creates a force feedback curve and a shake envelope from the sound's amplitude envelope (Feel does AHAP-from-audio for mobile, Meta does audio-to-haptics for Quest, both VERIFIED; nobody does it in Unreal for gamepad + camera on one timeline, INFERRED).
- Touches: strategy 3.4, ST-022, ST-026, ST-027. Cost M.

### D5. Off/On capture to GIF/MP4 from the preview
- One click renders a side-by-side clip. It is viral marketing for buyers and produces FeelKit's own Fab gallery and thumbnails (TL-005). Nobody offers it (INFERRED). Pure evaluation makes frame-exact offline render straightforward.
- Touches: strategy 3.6, TL-005. Cost M.

### D6. MCP toolset + JSON recipes
- On UE 5.8 (behind `ENGINE_MINOR_VERSION >= 8`), expose `AICallable` tools: create/modify recipe, list steps, preview-render a recipe to frames, run the comfort audit. JSON import/export works on all versions.
- **Honest rating:** mid leverage. Epic's MCP is Experimental and APIs "subject to change" (VERIFIED). But it turns FeedbackFX's "Agentic" branding into something real, and it is cheap on top of D5 (the agent can "see" its result).
- Touches: strategy 3.5, C-001d. Cost S–M (after JSON).

### D7. GAS bridge: GameplayCueNotify that plays a recipe
- Turns R2 (GAS is free) into distribution: every GAS project can adopt FeelKit without replacing cues. It beats FeedbackFX on its own turf, with FeedbackFX's hard GAS dependency (VERIFIED) versus an optional module.
- Touches: C-003 (optional module), TRG-*. Cost S–M.

**Explicitly parity (do not market as differentiators):** networking, anim notifies, trigger component, Niagara spawn, UI steps, adaptive triggers, presets, copy/paste, undo, data validation, cooldown/max concurrent, Blueprint custom steps, and "preview without PIE" by itself.

---

## 7. Assumptions and open questions

**Assumptions**
1. Fab rating counts are a rough proxy for sales volume. The true ratio is unknown.
2. Fab search "starting price" is USD list price, and the lower per-license prices seen on 2026-09-16 reflect a temporary sale.
3. "No preview / no comfort" for competitors is inferred from their docs and listings, not from installing them. Game Juice Pro's 2026-06-26 update had no notes and could have added tooling.
4. Absence of accessibility features in Feel is based on release notes v5.0–v6.1 and the docs index. Older versions were not searched exhaustively.
5. Market-size judgements assume Fab is the main channel. Gumroad, itch and direct sales were not measured.

**Open questions (for Bill)**
1. **OD-001 name/prefix:** keep "Feel" (search adjacency, collision risk) or rename now with core redirects? This blocks marketing and Fab title decisions.
2. **Pricing structure:** a single $70–80 product, or Core (~$39.99) plus a Studio/Pro tier (capture, audit, MCP)?
3. **Is moment capture (D1) promoted into Phase 5**, ahead of V1 steps? It is the strongest "cannot get elsewhere" feature and also covers the debugger requirement.
4. **Comfort floor rule (R8):** accept that essential motion channels may only substitute, never floor?
5. **Native effect scaling spike (D2a):** allowed as a Phase 5 technical spike? It must stay within stock modules and no engine changes.
6. **Persistent/state-bound recipes (G6):** in scope for V1? FeedbackFX's low-health vignette use case is common.
7. **GAS bridge module (D7):** acceptable as an optional module like FeelNiagara and FeelEnhancedInput?
8. **Unverified items worth a manual check:** install Game Juice Pro (at $29.99) to confirm there is no in-editor preview, and check its 2026-06-26 update contents. Check whether Agentic FeedbackFX's Personal license is currently free. Confirm Lyra's settings (force feedback toggle).
