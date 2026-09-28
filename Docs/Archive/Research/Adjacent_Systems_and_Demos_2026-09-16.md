# FeelKit: Adjacent Systems and Demo Research

| Field | Value |
|---|---|
| Date | 2026-09-16 |
| Question 1 | Which adjacent systems should FeelKit own or integrate with to become much more valuable without turning into an "everything plugin"? |
| Question 2 | Which game-scenario demos best show what FeelKit enables, and which capabilities do those demos reveal as priorities? |
| Inputs | `FeelKit_Strategy.md`, `FeelKit_Requirements.md` v0.1, `CLAUDE.md` (Phase 5A), FeelKit source (read-only check), web research below |
| Status | Research and proposal only. Nothing here changes requirements or phase scope until Bill accepts it |
| Evidence labels | **VERIFIED** = I read the source page (or its transcript). **SEARCH** = seen only in a search-engine summary, page not opened or blocked (403). **INFERRED** = my reasoning or general engine knowledge, not checked against a source |

---

## 1. Executive summary

### 1.1 The finding in one paragraph

In every game studied, great feedback is not a list of effects. It is a **response to a game event, chosen by context and scaled by game state**. Sekiro's deflect and block sound and look different; Apex numbers change color with shield tier; Balatro's shake has tiers tied to score size; Hollow Knight muffles audio during i-frames; Monster Hunter's hitstop changes by weapon and hit. Unreal already has a way to route events to effects (GAS Gameplay Cues, Lyra Context Effects). But those routes end in fire-and-forget spawners: no timeline, no arbitration, no comfort layer, no preview of the combined result. FeelKit already owns the *response* half very well. It is missing the three *input* pieces that turn a response into a system: **what happened** (event plus context), **how big** (parameters and escalation) and **when it keeps going** (sustained recipes). My check of the code confirms `UFeelRecipe` has no `Parameters` field and `FFeelTarget` carries no direction, instigator, hit normal or surface. Strategy section 2.3 asked to reserve `Parameters`, but it has not been done.

### 1.2 Strategic position

> **FeelKit is the response layer of an Unreal game. Gameplay reports *what happened* (a tag plus context). FeelKit decides *how it feels*: which variant, how strong, in what order, across camera, time, screen, audio, haptics, UI and VFX. The result is comfort-safe and can be previewed and scrubbed before anyone presses Play.**

This keeps FeelKit out of combat logic, animation, VFX content and settings menus. It moves FeelKit up one level: from "a player for juice recipes" to "the place where gameplay events become feel". Competitors in Unreal (Game Juice Pro, GameJuice) stop at "call a node". Agentic FeedbackFX already routes tag plus magnitude to effects, but it needs GAS and Niagara and has no timeline (SEARCH/VERIFIED, see 4.3). Unity's Feel added an edit-mode preview in v6.0 (July 2026, VERIFIED), so "preview" alone is now a shrinking moat. **Context-driven routing + parameters + comfort + scrubbable preview** together is the defensible package.

### 1.3 Recommendations

| # | System | Verdict | One-line justification |
|---|---|---|---|
| 1 | **Recipe Parameters and Play Context** (named inputs like Damage, Speed and Distance; hit direction, normal, instigator, surface) | **OWN** (core, foundation) | Every one of the 5 demos needs it. One recipe then covers a scratch through a crit, and punches can go *away from the hit*. It is a data-model change, so it must land before content (CT-001) |
| 2 | **Feel Events: tag-based routing with context variants** (a "Feel Map" asset: event tag + context tags → recipe; anim notify / notify state; trigger component) | **OWN** (core) + **INTEGRATE** (GAS cue and Enhanced Input in optional modules) | This removes most Blueprint wiring. It is the engine-agnostic version of what GAS cues and Lyra Context Effects do, without their limits (4 of 5 demos) |
| 3 | **Escalation and sustained recipes** (streak/trauma accumulators; recipes that hold while a state is active) | **OWN** (core) | Combos, low health, heartbeat, charge-ups and score tiers cannot be built today. Cheap once #1 exists; keeps pure evaluation because values are sampled by the evaluator |
| 4 | **Complete the sensory channels as steps**: Spawn Particle (Niagara module), Decal, Light Flash, Hit-Flash Overlay, screen color steps, audio mix steps (duck, low-pass, pitch) | **OWN the steps, AVOID the content** | Nearly every showcase moment has a particle and a light. Without them a recipe cannot represent "the whole hit", and buyers go back to Blueprint |
| 5 | **UI feedback steps + a lean Number Pop / Hit Marker emitter** | **OWN (thin)** | Needed by 4 of 5 demos, and comfort and preview apply to it. Stop at pooled pop-ups driven by recipes; no HUD framework, no style library |
| 6 | **Moment capture + Rewind Debugger track + arbiter overlay** | **OWN** capture, **INTEGRATE** with Rewind Debugger | Already in the strategy (3.2). Unreal's Rewind Debugger accepts custom tracks (VERIFIED), so FeelKit plays can appear next to animation data instead of FeelKit building a second recorder UI |
| 7 | **Accessibility beyond scales**: flash limiter, sensory substitution (a feedback event can emit a visual or haptic cue for players who cannot hear it), comfort for effects FeelKit did not play | **OWN** | Builds directly on Comfort and essential-track substitution. The Last of Us Part II ships combat vibration cues (VERIFIED); XAG 118 gives concrete flash thresholds (VERIFIED). Nobody on Fab sells this as part of a feel tool |
| 8 | **Advanced haptics** (Unreal device properties such as trigger resistance and effect; audio-based vibration; `.haptic` clips) | **INTEGRATE** (optional FeelHaptics module) | Unreal already routes device properties through Force Feedback assets (VERIFIED). DualSense plugins and Interhaptics/Meta SDKs own conversion and drivers. FeelKit should *sequence* haptics, not synthesize them |
| 9 | **Networked feel events** | **OWN (thin)**, as already planned (NET-002) | Required to sell to multiplayer projects, but 0 of 5 single-player demos need it. Replicate the event and its context; never replicate evaluation |
| 10 | **Beat sync** (quantize plays to a Quartz clock) | **INTEGRATE later** | Real (Hi-Fi Rush) but niche. 0 of 5 demos need it. An optional "quantize to Quartz beat" play option is enough |
| 11 | **Hit reactions, VFX/SFX libraries, damage systems, camera rigs, settings menus, telemetry, audio-to-haptic conversion, recipe marketplace** | **AVOID** | Crowded, content-heavy or outside feel authoring. Each one would dilute the "response layer" identity (section 7) |

---

## 2. Game case studies

Each entry lists the **systems around the feedback**, which is the point of this research. "Implication" connects each game to a FeelKit capability.

### 2.1 Vlambeer: Nuclear Throne and "The Art of Screenshake" (Jan Willem Nijman, INDIGO 2013)
- **VERIFIED** (artificials.ch transcription of the 30 tricks): the list mixes presentation tricks (muzzle flash, impact effects, white hit flash, screen shake, 20 ms "sleep", camera kick, more bass) with **gameplay-side tricks** (lower enemy HP, knockback, permanence of bodies and shells, gun delay).
- **INFERRED:** about half of "juice" is presentation that FeelKit can own. The rest (knockback, HP, fire rate) is gameplay that FeelKit must not own. Permanence (shells, decals, corpses) sits on the edge: a decal step is presentation; a corpse system is not.
- **Implication:** Spawn Particle, Decal, Light Flash (muzzle) and Hit-Flash are core juice. Character knockback stays gameplay (AVOID); cosmetic physics impulse on props is optional.

### 2.2 Squirrel Eiserloh, "Juicing Your Cameras With Math" (GDC 2016)
- **VERIFIED** (Internet Archive transcript): shake is driven by a **trauma** value from 0 to 1. Events add to it, it decays linearly, and shake strength is trauma squared or cubed. In 3D, rotational shake works better than translational. Perlin noise with per-axis seeds gives replay reproducibility and works under slow motion.
- **Implication:** FeelKit's pure, seeded evaluation already matches this. What is missing is the **accumulator**: a per-target value that plays add to and that decays. This is exactly recommendation #3 (escalation).

### 2.3 DOOM (2016) / DOOM Eternal
- **VERIFIED** (Game Developer article on "push forward" combat, GDC talk page): glory kills, the flame belcher and the chainsaw restore health, armor and ammo, so the player keeps engaging.
- **SEARCH** (StrategyWiki, Doom wiki): a staggered demon glows **blue**, and turns **orange** when the player is in glory-kill range.
- **INFERRED:** the feedback is **state-driven and sustained**: a glow that lasts while the enemy is staggered and changes with the player's distance. One-shot recipes cannot express this.
- **Implication:** sustained recipes with a stop condition, plus a Hit-Flash/overlay style step, plus a context parameter (distance to player).

### 2.4 Hollow Knight
- **VERIFIED** (Steam discussion): when the Knight takes damage, the game briefly slows and audio is **muffled during the invincibility frames**. Players argued whether the effect is too extreme.
- **Implication:** audio mix steps (low-pass, duck) are part of hit feedback, not just sound effects. The player debate is an argument for comfort scaling of hitstop and audio effects.

### 2.5 Celeste
- **VERIFIED** (Maddy Thorson, "Celeste & Forgiveness"): most feel comes from movement forgiveness (coyote time, jump buffering, corner correction, half gravity at the apex).
- **SEARCH** (Steam threads, changelog): screen shake setting with 50% default since v1.4; Photosensitive Mode reduces flashes; freeze frames on refill orbs were shortened so they cannot eat inputs.
- **Implication:** forgiveness mechanics are gameplay and belong out of scope (the requirements already exclude coyote time). The freeze-frame lesson matters: **hitstop must never swallow input**. This belongs in the docs and in the Global Hitstop step's defaults.

### 2.6 Dead Cells
- **VERIFIED** (official wiki, Assist Mode and Accessibility): separate options for bright flashes, screen shake, a particle limit, and the size of enemy attack signs.
- **SEARCH:** many Steam threads ask for ways to disable shake and camera motion because of motion sickness.
- **Implication:** comfort groups map almost one to one to what shipped games expose. A **particle-limit comfort group** becomes relevant as soon as FeelKit spawns particles.

### 2.7 Hades / Hades II
- **SEARCH only.** I found no Supergiant talk or postmortem on combat feedback (their GDC talks cover dialogue and narrative). Treat any claims about Hades feedback internals as **INFERRED**: dash and hit flashes, short hitstop on heavy hits, and strongly layered god-boon sounds and colors.
- **Implication (INFERRED):** boon-colored hit variants are a **context variant** problem: the same hit gets different color and sound by damage type. This supports recommendation #2.

### 2.8 Sekiro / Elden Ring
- **SEARCH** (Medium analysis blocked by 403; Nexus and Steam snippets): a perfect deflect gives a big orange spark burst and a louder, higher-pitched clang. A mistimed block gives no sparks and a dull sound. Blade deflects are brighter and higher-pitched than fist or kick deflects.
- **INFERRED:** Elden Ring posture breaks and ripostes use the same pattern: outcome selects the response.
- **Implication:** the strongest example of **outcome × material variant selection** (deflect/block × blade/fist). A Feel Map with context tags handles it with no Blueprint branching.

### 2.9 Monster Hunter (World / Wilds)
- **SEARCH** (GamesRadar and OpenCritic snippets; full article could not be parsed): Wilds players said they missed World's hitstop; after the beta, Capcom tuned hitstop across all weapon types, not just the Great Sword.
- **INFERRED:** hitstop length depends on weapon, move and whether the hit lands on a weak spot.
- **Implication:** hitstop is a **parameterized** track (weapon class, charge level, weak spot), not a constant. This supports Parameters (#1) and context variants (#2).

### 2.10 Street Fighter 6 / Guilty Gear Strive
- **SEARCH** (SuperCombo wiki, blocked on fetch): heavier attacks have longer hit freeze; some Punish Counters have extra-long hitstop; counter hits add frame advantage.
- **SEARCH** (Guilty Gear wiki, Steam): counter hits increase hitstop, and the victim gets more than the attacker; Strive's hard counters add a slowdown.
- **Implication:** **asymmetric hitstop** (attacker and victim frozen for different times) needs a play context that knows *both* actors. FeelKit's Actor Hitstop targets one actor; a context-aware recipe could target instigator and victim separately.

### 2.11 God of War (2018)
- **VERIFIED** (PlayStation Blog, other studios' designers on GoW combat): hit stop on both Kratos and the target, high-frequency effort audio, big animation arcs, exaggerated enemy reactions, and camera shake kept low for target readability.
- **SEARCH** (Push Square; page 403): in the axe recall, haptics swell until the axe lands with a heavy thud; small details such as wiggle time and the throw arc add up.
- **Implication:** the recall is a **sustained, building recipe** (haptics ramp while the axe flies, then peak on catch). The duration comes from gameplay (flight time), so the recipe needs either a sustain region or a Duration parameter.

### 2.12 Returnal, Astro Bot, Ratchet & Clank: Rift Apart (haptics)
- **VERIFIED** (PlayStation Blog, Housemarque): **sound designers** authored Returnal's haptics; rain drops are **procedurally synthesized at runtime**, which let them tune while playing; adaptive trigger: half pull to aim, a click, full pull for alt-fire.
- **VERIFIED** (One More Game interview with Nicolas Doucet): Astro Bot haptics are sound-based ("sounds you cannot hear but resonate"); the lead gameplay engineer owns both character control and haptics.
- **SEARCH** (PSU, TheGamer): Rift Apart gives each of its weapons its own trigger resistance, often per fire mode.
- **Implication:** high-end haptics are **audio-authored assets** played in sync with sound. FeelKit should sequence haptic assets on the same timeline as sound (INTEGRATE), not build a synthesizer. Trigger resistance is weapon *state*, which again points at sustained recipes.

### 2.13 Resident Evil 4 remake / Dead Space remake (horror)
- **VERIFIED** (GamingBolt): Dead Space's **Intensity Director** changes spawns, particles, sound effects and lighting, and Isaac's heartbeat and breathing respond to it.
- **SEARCH** (Digital Trends, PushSquare, GitHub mod): RE4's DualSense support is praised. A PC mod rebuilt haptics from real in-game audio and split them per event (parry windows, knife variants), which shows buyers want per-event variation rather than one generic pulse.
- **Implication:** horror feedback is mostly **continuous and driven by game state** (fear, distance, health). Parameters plus sustained recipes are the core requirement for a horror demo.

### 2.14 Hi-Fi Rush
- **VERIFIED** (Game Anim, CEDEC 2023 summary): animations were authored at 120 BPM / 60 fps with key poses on beats, then re-timed to each song's BPM; effects and input windows follow the same beat grid.
- **Implication:** beat sync is a whole-game architecture choice, not a feedback add-on. FeelKit's part is small: **start a play on the next beat** (Quartz quantization, VERIFIED in section 3.8). INTEGRATE later.

### 2.15 Balatro / Vampire Survivors (UI juice)
- **VERIFIED** (Blake Crosley design guide on Balatro): score digits roll like slot reels; **screen shake has three tiers tied to score size**; each scored card plays a rising note; jokers bounce in sequence as they trigger; a threshold crossing gives a bass drop plus a flash.
- **INFERRED** for Vampire Survivors: collection sounds and screen-filling particle feedback. No source read.
- **Implication:** UI juice needs (a) **widget steps**, (b) **parameter-driven tiers**, and (c) **sequenced repetition with rising pitch**. Parameters, escalation and UI steps together cover it.

### 2.16 Overwatch
- **VERIFIED** (GDC 2016 transcript, Lawlor and Neumann, "Play by Sound"): enemy sounds are prioritized by a weighted **threat score**: damaged (0.5), distance (0.3), enemy screen size (0.1), player size on the enemy's screen (0.3), scripted events (0.4), being seen (0.3), being shot at (0.6). Scores sort into High, Normal, Low and Cull buckets. VO callouts are stimulus-driven and differ by the listener's perspective.
- **SEARCH** (a soundboard site): the hit marker was built from a bottle-cap pop. The source is weak; do not repeat it.
- **Implication:** FeelKit's arbiters already cap simultaneous camera and time effects. **Importance-based culling of plays** (a priority per recipe plus a budget per channel) is a natural arbiter extension for busy scenes. It is small, and nothing on Fab offers it.

### 2.17 Destiny 2, Apex Legends, Titanfall 2 (shooter confirmations)
- **SEARCH** (Apex guides): damage number color shows the target's shield tier, and headshots have their own color. Once the shield breaks, numbers turn red.
- **SEARCH** (Destiny wiki): precision hits show yellow numbers; some damage sources show none.
- **Titanfall 2:** the GDC talk page is behind a membership. No verified feedback details.
- **Implication:** hit confirms are **context-variant UI plus sound** (target state × hit zone). This supports Feel Map variants (#2) plus Number Pop (#5).

### 2.18 Devil May Cry 5 / Bayonetta
- **SEARCH** (Shacknews, wiki): the style rank D to SSS rises with varied attacks and drops when hit; announcer calls from B upward; characters react when hit at S or higher.
- **INFERRED** (Bayonetta Witch Time): the slow-motion window after a perfect dodge is a sustained time-dilation state with a screen tint.
- **Implication:** rank-based escalation needs an **accumulator parameter that the game can set** (not only one that decays). Witch Time equals Slow-mo Ramp + sustain + color tint.

### 2.19 Diablo IV
- **SEARCH** (Blizzard VFX article snippet): legendary drops are exciting, but visual hierarchy ranks them below reacting to a monster attack that can kill.
- **Implication:** priority between feedback events is a design need (see Overwatch). This is also a reason for FeelKit's priority and culling features.

### 2.20 Accessibility references
- **VERIFIED** (Xbox Accessibility Guideline 118): a flash is a luminance change of 10% or more with the darker value below 0.8. Failure: more than about **3 flashes per second** covering about **20% or more of the screen**. Red flashes (R/(R+G+B) ≥ 0.8) have a lower threshold. Removing triggers is preferred over warnings. Harding FPA is the named test tool.
- **SEARCH** (Game Accessibility Guidelines): avoid more than three flashes per second over 25% of the screen; important audio information should also be shown visually.
- **VERIFIED** (PlayStation page, The Last of Us Part II): sliders for camera shake and motion blur, an option to turn off the dolly zoom, **combat vibration cues** and **traversal/combat audio cues**, slow motion while aiming, high contrast mode.
- **VERIFIED** (Dead Cells wiki): separate flash, shake and particle options.
- **Implication:** comfort scales are the baseline. The next step that shipped games take is **sensory substitution**: the same event produces a haptic, visual or audio cue for players who miss one channel. FeelKit's essential-track substitute already models this per track. Extending it to "cue variants per comfort profile" is cheap and unique.

---

## 3. Workflow and engine analysis

### 3.1 GAS Gameplay Cues (VERIFIED, Epic docs and Python API)
- Cues are **routed by gameplay tag**. `GameplayCueNotify_Burst` is non-instanced and one-off (no latent actions); `_Looping` has application, while-active and removal phases; `_Actor` is instanced.
- `GameplayCueNotify_BurstEffects` holds particles, sounds, a camera shake, a camera lens effect, force feedback and a decal.
- `GameplayCueNotify_SpawnCondition` provides **allowed and rejected physical surface types**, **chance to play**, and a local-control policy and source.
- Device property effects can also be fired from Burst cues (VERIFIED, Device Properties docs).
- **Takeaways for FeelKit:**
  1. Epic's own design validates **tag routing + surface filter + burst/looping split**. FeelKit should adopt the same vocabulary (burst and sustained recipes, surface context) so GAS users feel at home.
  2. GAS cues **have no timeline, no ordering between effects, no arbitration, no comfort, and no combined preview**. They also require GAS, so Blueprint-only and non-GAS projects cannot use them.
  3. **Integration shape:** an optional `FeelGAS` module with a `UFeelGameplayCueNotify` (Burst and Looping) that plays a recipe and maps `FGameplayCueParameters` (normalized magnitude, raw magnitude, location, normal, instigator, physical material, aggregated tags) into FeelKit's play context. FeelKit then becomes "the best thing to put inside a GAS cue".

### 3.2 Lyra (VERIFIED: x157 notes, DeepWiki summary)
- Hit markers: `ULyraWeaponStateComponent` on the controller keeps local hit markers and confirms them with the server; `SHitMarkerConfirmationWidget` draws them.
- Impacts and fire: GameplayCues `GameplayCue.Weapon.Rifle.Fire`, `...Impact`, `...Melee.Hit`.
- **Context Effects** (`ULyraContextEffectsLibrary`, subsystem, component, `UAnimNotify_LyraContextEffects`): **effect tag + context tags (such as surface) → sound and VFX assets**. The notify traces for the surface and can preview in the editor. Physical surfaces map to context tags in project settings.
- Messaging: the Gameplay Message Router (`GameplayMessageRuntime`) ships **only inside Lyra**, not as an engine plugin (VERIFIED via forum and community copies). Messages are struct payloads on tag channels, for example `Lyra.Elimination.Message`.
- **Takeaways:** Lyra's Context Effects is the closest existing design to recommendation #2. But it only spawns sounds and particles, lives in a sample project, and has no camera, time, haptics, comfort or timeline. FeelKit can **generalize this pattern to full recipes**. Do not depend on the Message Router, since it is not in the engine. A FeelKit "Feel Event" API can offer the same tag-channel convenience with no dependency.
- **Licensing (open question):** do not ship Lyra code or assets in a Fab product. A written "Using FeelKit in Lyra" guide is safe.

### 3.3 Anim Notifies and Notify States (VERIFIED, Epic docs and community)
- Notify States guarantee Begin and End with Tick in between. Point notifies can be **skipped at non-standard play rates**, because notify evaluation is tick-driven (Bugnet article). This matters when hitstop changes time dilation.
- **Takeaway:** ship `AnimNotify_PlayFeel` (API-006) **and** `AnimNotifyState_PlayFeel`. The state version maps to sustained recipes (charge, weapon trail window) and stops cleanly. Previewing the recipe inside the animation editor is the killer feature: scrub the montage and see the shake. It reuses FeelKit's preview sink. Neither Lyra Context Effects' preview nor GAS cues combine a timeline with that.

### 3.4 Physical materials and surface tables (VERIFIED where noted)
- Engine: `EPhysicalSurface` on `UPhysicalMaterial`, filtered in GAS cue spawn conditions (VERIFIED).
- Fab: "Physical Material Profiles – Footstep and Impact System" (Data Table profiles), "Easy Footstep Plugin", several bullet impact VFX packs with physical material setup (SEARCH).
- **Takeaway:** surface selection is simple and already on sale. FeelKit should **consume** surface context (map surface → context tag → recipe variant). It should not sell impact content or footstep systems.

### 3.5 MetaSounds, Wwise, FMOD
- MetaSounds inputs are set by name on an Audio Component (`SetFloatParameter` and similar) (VERIFIED, community docs).
- Wwise: `SetRTPCValue` (global or per actor), States and Switches (VERIFIED, integration docs). FMOD has the equivalent event parameters (INFERRED).
- Meta Haptics Studio exports `.haptic` clips that play **inside FMOD (2.03.11+)** and, as announced, Wwise from Q1 2026, triggered by the same events as audio (VERIFIED, Meta blog).
- **Takeaways:**
  1. The Play Sound step should **forward recipe parameters and track intensity to MetaSound inputs** (for example "Intensity" and "Damage"). That makes FeelKit's parameters audible with no Blueprint.
  2. Wwise and FMOD users trigger audio through their own events. Offer **"Post Wwise Event" / "Play FMOD Event" steps in optional modules** that forward parameters as RTPCs. This is a small integration with high value for studios. Middleware SDKs cannot be redistributed, so these modules must compile only when the buyer has the integration (INFERRED; same pattern as C-003).
  3. Do not build ducking or mixing systems beyond simple steps. Submix effects and Sound Classes exist in the engine.

### 3.6 Haptics pipelines (VERIFIED)
- Unreal Force Feedback Effect: multi-channel curves; `ForceFeedbackComponent` with **distance attenuation**; `SpawnForceFeedbackAtLocation` and `SpawnForceFeedbackAttached`.
- **Device Properties** (light color, trigger resistance and feedback) are applied through the Input Device Subsystem, through Force Feedback assets, or through Burst GameplayCues. Per-device overrides exist. Audio-based vibration is **PS5 DualSense only and experimental**.
- Third party: Unreal-Dualsense Pro (Windows/Linux/macOS/PS5; data-table trigger effects; converts an audio submix to haptics), the open-source Windows DualSense plugin (Project Borealis), Altercode ($17.99, Gumroad). Interhaptics is **free** (Razer, now WYVRN) with a composer, audio-to-haptics and Unreal support. Meta Haptics SDK for Quest.
- **Takeaway:** conversion and drivers are solved and often free. FeelKit's value is **sequencing** (haptics on the same timeline as sound and shake) and **comfort** (the Haptics group). Next steps: a "Play Force Feedback Asset" step that carries device properties, distance attenuation from play context, and an optional adapter interface for third-party haptic players. **INTEGRATE, do not build.**

### 3.7 Enhanced Input and input-driven feedback (INFERRED from engine knowledge)
- Enhanced Input triggers (Pressed, Hold, Released, Pulse) and Input Action events are the natural source for fire and charge feedback. A `FeelEnhancedInput` module (C-003 / TRG-003) binding an Input Action to a recipe (Started → sustained start, Completed → stop) covers charge-up feedback without Blueprint.
- **Caveat:** input-triggered feedback for *actions* (firing) often belongs on the gameplay result (the shot actually fired), not on raw input. Document this. Default to anim notifies or gameplay calls for combat, and use input bindings for UI and charge.

### 3.8 Quartz beat sync (VERIFIED, Epic docs)
- Quartz clocks run on the audio render thread with BPM and time signature. Gameplay can **subscribe to quantization events** (for example every beat) and schedule commands on bar or beat boundaries with sample accuracy.
- **Takeaway:** an optional "Quantize play to Quartz clock" setting on PlayFeel is a small feature (S). Beat-locked *tracks inside* a recipe would require re-timing recipes to BPM, which is out of scope.

### 3.9 Gameplay Cameras plugin (SEARCH/VERIFIED)
- Still **experimental in 5.6**; planned as beta then production in 5.7/5.8 (Ludovic Chabant's blog). Camera rigs are assets, and shakes are nodes in the rig graph.
- **Risk (INFERRED, open question):** FeelKit delivers through a `UCameraModifier` on the Player Camera Manager (DEL-001). Projects that move to Gameplay Cameras may route the view differently. **Spike before V1:** confirm that FeelKit's modifier still applies with GPC in 5.7/5.8. If not, add a GPC camera node that pulls FeelKit's camera contribution. INTEGRATE; never build camera rigs.

### 3.10 UMG and CommonUI (SEARCH)
- UMG timeline animations are authored per widget and cannot easily target procedurally built widgets; tween plugins such as UITween fill the gap.
- **Takeaway:** widget steps (punch, shake, flash, number pop) that target *any* widget by reference or name fill a real gap. They should be comfort-aware ("reduce UI motion"). Stop there; CommonUI owns navigation and styling.

### 3.11 Rewind Debugger (VERIFIED)
- It records PIE and lets you scrub with a timeline. **Custom tracks are supported** (Pose Search uses one). It needs the Animation Insights plugin.
- **Takeaway:** add a FeelKit trace channel (NF-005) plus a Rewind Debugger track showing plays, suppressed tracks and arbiter decisions next to animation. Keep FeelKit's own "Open last hit in recipe editor" as the headline. Rewind Debugger integration is the credible debugger for studios.

### 3.12 Unity Feel usage patterns (VERIFIED, Feel docs and release notes)
- MMF_Player: initialization modes, play direction, **global intensity**, **range-based feedback with a falloff curve**, cooldown, chance, initial delay, duration multiplier, events (OnPlay, OnComplete and more), **channels** to address players remotely, async play, loops and holding pauses (INFERRED for the last two).
- Recent systems: v5.5 **speed-based movement modes**; v5.9 multi-gamepad haptics and decoupled player events; **v6.0 (July 2026): preview without entering Play mode**, a sequential mode and custom sound manager tracks; v6.1 (September 2026) timing options. Other tools: MMFloatingText, springs, MMSoundManager, Nice Vibrations (HD haptics with audio-to-haptic via Lofelt).
- **Takeaways:**
  1. Feel treats **range falloff, channels, floating text and haptics as core**. Unity converts will expect them.
  2. Feel's edit-mode preview makes "preview" table stakes. FeelKit's lead must come from **timeline + scrubbing + comfort + context routing**, not from preview alone.

---

## 4. Adjacent-system evaluations

### 4.1 Scoring table

Criteria (the user's): **C1** makes existing recipes much more powerful; **C2** enables feedback devs could not easily create; **C3** reduces Blueprint/code work; **C4** makes authoring/tuning much easier; **C5** creates an ecosystem around recipes; **C6** makes FeelKit useful in more parts of development. Scale: H / M / L / – (none).
**Fit** = keeps FeelKit "the game-feel authoring and orchestration layer". **Creep** = scope-creep risk. **Dep** = dependency impact. **Uniq** = uniqueness against Fab and Unity. **Effort**: S (<1 wk), M (1–3 wk), L (3–6 wk), XL (>6 wk), for one experienced developer (INFERRED estimates).

| Candidate | C1 | C2 | C3 | C4 | C5 | C6 | Fit | Creep | Dep | Uniq | Effort | Verdict |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Recipe Parameters + Play Context | H | H | H | M | M | M | H | L | Core, none | H | L | **OWN** |
| Feel Events: tag routing, context variants, Feel Map | H | M | H | H | H | H | H | M | Core, none | H | L | **OWN** |
| Trigger layer: anim notify/state, trigger component | M | L | H | H | M | H | H | L | Core (Engine) | M | M | **OWN** |
| GAS cue adapter | M | L | H | M | M | H | H | L | Optional module (GameplayAbilities) | M | S | **INTEGRATE** |
| Enhanced Input binding | L | L | M | L | L | M | M | L | Optional module | L | S | **INTEGRATE** |
| Escalation accumulators (streak/trauma) | H | H | H | M | L | M | H | L | Core | H | S–M | **OWN** |
| Sustained/looping recipes | H | H | H | M | L | M | H | L | Core | M | M | **OWN** |
| Sensory steps: Niagara, decal, light, overlay flash, screen color, audio mix | H | M | H | M | M | M | H | M | Niagara in optional module | L | M–L | **OWN steps, AVOID content** |
| UI steps + Number Pop / Hit Marker emitter | M | M | H | M | M | H | M | **H** | Core (UMG) | M | M | **OWN (thin)** |
| Surface-aware impact tables | M | L | H | M | L | M | M | M | Core (physical materials) | L | S (as context) | **OWN as context only**; AVOID content |
| MetaSound parameter forwarding | M | M | H | M | L | M | H | L | Core (Engine audio) | M | S | **OWN** |
| Wwise / FMOD event steps | M | L | H | M | M | H | M | L | Optional modules, buyer SDK | M | M | **INTEGRATE** |
| Advanced haptics (device properties, `.haptic`, DualSense) | M | H | M | M | L | M | M | M | Optional module / adapters | M | M | **INTEGRATE** |
| Audio-to-haptic conversion | L | M | L | M | – | L | L | H | Heavy DSP | L (free tools) | XL | **AVOID** |
| Camera system integration (Gameplay Cameras) | L | L | L | L | – | M | M | M | Optional if needed | L | S–M | **INTEGRATE** (compatibility only) |
| Animation: hit reactions, additive recoil | L | M | M | L | – | M | L | **H** | Anim content | L (crowded) | XL | **AVOID** |
| Animation: montage play-rate hitstop step | M | M | M | L | – | L | M | L | Core | L | S | **OWN (small)** |
| Beat / music sync (Quartz quantize) | L | M | M | L | – | L | M | L | Core (AudioMixer) | M | S | **INTEGRATE later** |
| Networked feel events | M | L | H | L | L | H | H | M | Core | L | M | **OWN (thin)** |
| Flash limiter + sensory substitution | M | H | M | L | L | H | H | L | Core | H | M | **OWN** |
| Comfort for non-FeelKit effects | L | M | H | L | M | H | H | M | Spike needed | H | M | **OWN** (strategy 3.1) |
| Priority / importance culling (Overwatch-style) | M | M | M | M | L | M | H | L | Core | H | S–M | **OWN** |
| Telemetry / tuning analytics | L | L | L | M | L | M | L | H | Backend | L | L | **AVOID** |
| Moment capture + debugger + Rewind track | M | M | M | H | L | H | H | L | Editor / Trace | H | M–L | **OWN / INTEGRATE** |
| JSON recipes + AI-assisted authoring | L | L | M | M | H | M | M | M | LLM config | M (FeedbackFX markets "agentic") | M | **OWN JSON; AI later** |
| Capture to GIF/MP4 | L | – | – | M | H | L | M | L | Editor | H | M | **OWN** (strategy 3.6) |
| Recipe marketplace / preset store | L | – | L | M | H | L | M | **H** | Hosting, moderation | M | XL | **AVOID now**; GitHub/Discord sharing instead |

### 4.2 Detailed sections

#### 4.2.1 Recipe Parameters and Play Context (OWN, core)
**What:** extend `PlayFeel` with an optional `FFeelPlayContext`:
- `Parameters`: name → float (Damage, Speed, Charge, Fear). Recipe-declared, with defaults and a range.
- `Direction` (world vector), `ImpactNormal`, `ImpactLocation`, `Instigator` (actor), `SurfaceType`, `ContextTags` (FGameplayTagContainer).
- Built-in parameters: **Distance to local player** and **Instigator is local player**. This replaces the unevaluated `FFeelConditions.MaxDistance`; my grep found no evaluation of `Conditions` in `FeelCore/Private`.

**Track side:** each track gets optional **parameter mappings**: a parameter → curve → multiplier on track intensity, and (later) on a chosen step property (duration, amplitude, pitch). Steps with a direction (Camera Punch, Shake, Squash) get a **"Direction source"** option: Fixed / Context direction / Away from instigator / Toward camera. Camera Punch today is fixed camera-space plus jitter (verified in `FeelStep_CameraPunch.h`).

**Why it is the foundation:** Monster Hunter hitstop per weapon, Balatro shake tiers, horror heartbeat by distance, the platformer landing by fall speed, and explosion falloff all reduce to *parameter → curve → track*. Feel (Unity) exposes intensity and a range falloff (VERIFIED). FeedbackFX exposes magnitude (VERIFIED). **Named multi-parameter mapping on a timeline** exists in neither.

**Purity:** parameters are sampled when the play starts (or explicitly updated for sustained recipes). The evaluator receives them as inputs, so `Evaluate` stays a pure function of (time, alpha, intensity, seed, parameters). Scrubbing in the editor uses **preview parameter sliders**, which is a strong demo moment ("drag Damage from 5 to 100 and scrub").

**Risks:** data-model migration. Add the fields and a schema version **before any recipe content is authored** (CT-001). Keep the number of mapping targets small at first (intensity only, then 3–4 flagged step properties) to avoid a generic property-binding system.

#### 4.2.2 Feel Events and the Feel Map (OWN core; INTEGRATE GAS / Enhanced Input)
**What:**
- `SendFeelEvent(WorldContext, EventTag, Target, Context)`: fire-and-forget, Blueprint and C++.
- `UFeelMap` data asset: rows of **EventTag + required context tags (optional surface, damage type, outcome)** → Recipe, with **most-specific match wins**. This is the same rule FeelKit already uses for channel → comfort group (CMF-006), so it is consistent and testable. Maps can stack: a project default map plus per-character or per-weapon maps with priority. This is Lyra's "library per skin" idea (VERIFIED in BSLContextEffects and Lyra).
- Sources that call it: `AnimNotify_FeelEvent`, `AnimNotifyState_FeelEvent`, `UFeelTriggerComponent` (damage, landed, hit, overlap; TRG-001/002 already planned), and optional adapters: `UFeelGameplayCueNotify` (FeelGAS) and Input Action binding (FeelEnhancedInput).

**Why:**
- **C3:** gameplay code says `Event.Hit` with context once; designers own the rest.
- **C5:** a Feel Map is a *shareable unit* ("Shooter Feel Pack" = map + recipes).
- **C6:** animators (notifies), audio designers (MetaSound parameters) and UI designers (widget steps) all plug into the same map.
- **Sekiro-style outcome × material variants need no branching.**

**Differentiation:**
- GAS cues: tag routing, but GAS required, no timeline, comfort or arbitration.
- Lyra Context Effects: sample-only, sound and VFX only.
- BSLContextEffects (GitHub, MIT, 4 commits, UE 5.5, "active development"; VERIFIED): the closest open-source design, with no timeline, comfort or arbitration.
- FeedbackFX: tag + magnitude, but GAS and Niagara required, no variants by context tags beyond status and attribute triggers (VERIFIED via forum post).

**Risks:** becoming a generic event bus. Guardrails: the payload is fixed (`FFeelPlayContext`), there are no listeners other than recipes, and there is no gameplay return value.

#### 4.2.3 Escalation and sustained recipes (OWN, core)
**Escalation:** a `UFeelAccumulator` definition (tag, gain per event, decay per second, cap, optional reset on event), stored per target or per player in the subsystem, exposed as a **parameter source**. Uses: trauma shake (Eiserloh), combo count (DMC), coin streak pitch (Balatro-style rising notes), sustained-fire recoil. Games can also *set* it (style rank). Size S–M on top of parameters.

**Sustained recipes:** a recipe can mark a **sustain region** (loop between two times) or tracks flagged "hold until stop". `PlayFeel` returns a handle; `StopFeel` releases into the tail. Parameters can be **updated on a live handle** (`SetFeelParameter(Handle, Name, Value)`) for heartbeat-by-distance.

**Purity note:** loop time is computed from real time (`fmod` into the sustain region), and parameter changes are sampled per frame from the instance. Scrubbing a sustained recipe in the editor is deterministic, with a preview "hold for N seconds" setting. Steps with side effects (sounds) need loop-aware `OnStart`/`OnStop`, so each loop pass fires them again. Test this carefully.

Why it matters: 3 of 5 demos need sustain, and horror cannot be shown without it (section 5). GAS has a Looping cue (VERIFIED) and Feel has holding pauses (INFERRED), so buyers expect it.

#### 4.2.4 Sensory steps (OWN the steps, AVOID the content)
Priority order, from demo counts in section 6:
1. **Spawn Particle** (`FeelNiagara` optional module, ST-033): attach or at location; uses context normal and direction; forwards parameters to Niagara user parameters; comfort group "Particles" (Dead Cells has a particle limit, VERIFIED).
2. **Screen color steps** (ST-013 Desaturate, ST-015 Tint, ST-014 Radial Blur, ST-016 Fade): these reuse the Phase 4 post-process builder.
3. **Hit-Flash Overlay** (new): uses `UMeshComponent::OverlayMaterial` (in the 5.6 API, VERIFIED) with a plugin material, so a white or colored hit flash works on **any mesh without editing its materials**. Nijman trick #10. Today Material Parameter Pulse requires an authored parameter.
4. **Light Flash / Flicker** (ST-021): muzzle flashes, horror flicker.
5. **Audio mix steps** (ST-023 Duck, ST-025 Low-pass, ST-024 Pitch): Hollow Knight muffle, explosion tinnitus.
6. **Decal** (ST-034): impact permanence.
7. **Nested recipe / Random choice** (ST-035, ST-037): variant variety without duplicating recipes.

Content rule: ship **one neutral sample per step** (requirements 2.2 already say so). No impact VFX library; Fab is full of them (SEARCH).

#### 4.2.5 UI feedback and the Number Pop emitter (OWN, thin)
- Steps: **Widget Punch, Widget Shake, Widget Flash** (ST-029–031) that target a widget by reference or by name inside a user widget, with pure evaluation into render transform and opacity.
- **Number Pop** (ST-032) + **Hit Marker**: a small pooled screen-space emitter (one Slate/UMG layer, fixed pool, no per-hit widget allocation). Text, color and scale come from recipe parameters and context (crit, shield tier). Examples of performance-oriented designs: HitTicker on GitHub (ring buffer, per-frame budget, SEARCH).
- Comfort: a "UI motion" group.
- **Creep guardrail:** at most 3 built-in styles, no localization or formatting framework, no HUD layout. Fab already sells damage text plugins with 7–15 presets (SEARCH); FeelKit wins on *integration with the hit recipe*, not on styles.

#### 4.2.6 Moment capture, debugger and Rewind Debugger integration (OWN / INTEGRATE)
Already in strategy 3.2 and TL-002/003, NF-005. New from this research: Rewind Debugger **custom tracks** (VERIFIED) are the natural studio debugger. Record each play with its **full context and parameters** (made cheap by 4.2.1). "Open this moment in the recipe editor with the captured parameters" becomes the tutorial workflow for every demo in section 5.

#### 4.2.7 Accessibility extensions (OWN)
- **Flash limiter** (CMF-030–033) using XAG 118 numbers (3/s, 20% area, red rule). The limiter can only count FeelKit flashes; state this clearly in the docs.
- **Sensory substitution profiles:** a track can declare substitutes per comfort *need* (for example "if Audio is reduced, play Widget Flash + Haptic tick"), generalizing essential tracks. TLOU2's combat vibration cues are the reference (VERIFIED).
- **Particles group** and **UI motion group** when those steps ship.
- Keep the "not a certification" wording.

#### 4.2.8 Haptics (INTEGRATE)
A `FeelHaptics` module:
- a Force Feedback Asset step that carries device properties (trigger resistance and effect) and attenuation from context distance;
- an `IFeelHapticPlayer` adapter so DualSense, Interhaptics or Meta players can be driven from a track.

Audio-based haptics stay with Sony and the plugins (experimental in Unreal, VERIFIED). Premium tier (OD-005).

#### 4.2.9 Networking (OWN, thin)
Replicate `SendFeelEvent` (tag + compact context + parameters) with Multicast, OwnerOnly and SkipOwner (NET-002). Relevance by distance. For GAS projects, the cue adapter uses GAS replication instead, so no duplicate path is needed.

#### 4.2.10 Beat sync (INTEGRATE later)
`PlayFeel` option: `QuantizeTo = QuartzClock + Beat/Bar`. Size S. Wait until a buyer asks.

---

## 5. Demo concepts

Legend for tracks: **[E]** exists today; **[M: name]** missing, with the capability name used in section 6. Timings are starting values (INFERRED). "Target" follows FFeelTarget; "ctx" means play context.

Common tutorial base (licensing-safe): each tutorial starts with **"Create a new project from the UE 5.6 template variant X"** in the *buyer's* engine. FeelKit ships only recipes, Feel Maps and small sample assets it owns or that are CC0. Template variants verified in the Epic docs: Third Person **Combat** (melee combos, AI enemies, in-world health bars), **Platforming** (dash, wall jump), **Side Scroller** (pickups), First Person **Arena Shooter** (weapon pickups, bullet types, AI, ammo UI), **Survival Horror** (low-light setup, flashlight, sprint, flickering lights).

### 5.1 Shooter: "Every Bullet Has an Opinion" (base: Arena Shooter variant)

| Moment | Composition (in order) |
|---|---|
| **S1 Fire (auto rifle)** | Trigger: fire event from weapon BP [E via PlayFeel] or Feel Event [M: Feel Events] → Camera Punch pitch up, spring, jitter 4° [E] → FOV Kick −1° 60 ms [E] → Play Sound with pitch variation [E] → Light Flash on muzzle 40 ms [M: Light Flash] → Spawn muzzle particle [M: Spawn Particle] → Scale Punch on weapon mesh [E] → Force Feedback Curve 50 ms [E] → recoil and shake grow with a sustained-fire accumulator [M: Escalation] |
| **S2 Impact on surface** | Feel Event `Event.Impact` with ctx normal and surface [M: Play Context] → variant by surface (metal sparks / concrete dust / flesh) [M: Feel Map variants] → Spawn Particle along normal [M: Spawn Particle] → Decal [M: Decal] → Play Sound at location [E] |
| **S3 Hit confirm (body)** | Hit Marker pop [M: Number Pop/Hit Marker] → 2D "tick" sound [E] → Hit-Flash Overlay on enemy 80 ms [M: Overlay Flash] (Material Parameter Pulse [E] only if the enemy material has the parameter) → Number Pop with Damage [M: Number Pop, Parameters] |
| **S4 Headshot / crit** | Variant of S3 chosen by ctx tag `Hit.Zone.Head` [M: Feel Map variants] → Global Hitstop 35 ms [E] → Chromatic Aberration pulse [E] → layered crit sound [E] → gold hit marker and larger number [M: Number Pop] → Force Feedback spike [E] |
| **S5 Kill confirm** | Slow-mo Ramp 0.7× for 150 ms [E] → Vignette Pulse [E] → Desaturate pulse [M: Screen Color steps] → kill sound [E] → Screen Flash 5% [E] |
| **S6 Explosion nearby** | Shake scaled by Distance [M: Parameters (built-in Distance)] → Camera Punch *away from blast* [M: Play Context direction] → Radial Blur [M: Screen Color steps] → Screen Flash (flash-limited) [E + M: Flash Limiter] → Sound at location [E] → Low-pass "tinnitus" duck 1.2 s [M: Audio Mix steps] → Force Feedback attenuated by distance [M: Parameters] → Spawn Particle [M] |
| **S7 Taking damage + low health** | Camera Punch from damage direction [M: Play Context] → red Vignette [E] → low-health heartbeat loop while HP < 25% [M: Sustained recipes] → comfort: essential substitute when Flashes = 0 [E] |

**Capability gaps:** Play Context, Parameters, Feel Events + variants, Escalation, Sustained, Spawn Particle, Decal, Light Flash, Overlay Flash, Screen Color, Audio Mix, Number Pop/Hit Marker, Flash Limiter.
**Tutorial outline ("How I made it"):**
1. Create the Arena Shooter variant; enable FeelKit.
2. Build **R_Fire** on the timeline; preview in the editor with the weapon as preview mesh [E, PV-008].
3. Play in PIE [E, PV-005]; tune recoil live [E, PV-009].
4. Add the sustained-fire accumulator and show the timeline preview slider for it.
5. Create **FM_Shooter** (Feel Map): `Event.Impact` × surface, `Event.Hit` × zone.
6. Add one `SendFeelEvent` call in the projectile hit. This is the only Blueprint in the tutorial.
7. Explosion: scrub the Distance parameter, then check it in PIE.
8. Comfort: switch the preview to Reduced Flashing [E, PV-006] and show the limiter and substitutes.
9. Capture the last headshot and open it in the timeline [M: Moment capture].

**Content:** the template variant (buyer-side). Sounds must be own or licensed **with redistribution rights**. The Sonniss GDC bundle forbids redistributing raw files (SEARCH), so it can only be used in trailer video, not in shipped content. Particles and decals: a minimal plugin-owned Niagara sample.

### 5.2 Action/RPG: "Weight Class" (base: Third Person Combat variant)

| Moment | Composition |
|---|---|
| **A1 Light sword hit** | `AnimNotify_FeelEvent Event.Melee.Hit` [M: Triggers (anim notify)] → Actor Hitstop on attacker 40 ms [E] + on victim 60 ms (asymmetric) [M: Play Context instigator/victim] → Camera Punch along swing direction [M: Play Context] → slash sound [E] → Hit-Flash Overlay on victim [M] → slash particle at impact [M: Spawn Particle] → Squash & Stretch on victim [E] → Force Feedback [E] |
| **A2 Heavy hit** | Same recipe with Damage parameter: hitstop 60 → 120 ms, shake amplitude, FOV Kick, Chromatic Aberration mapped by curves [E steps + M: Parameters] |
| **A3 Crit** | Variant by `Hit.Crit` [M: Feel Map variants] → Slow-mo Ramp [E] → Screen Flash [E] → Number Pop (crit style) [M] → nested A2 recipe [M: Nested recipe] |
| **A4 Charged ability** | Anim Notify State or Input hold [M: Triggers] → sustained charge: shake and haptics rising with Charge [M: Sustained + Parameters] → release: FOV Kick [E], Radial Blur [M], Light Flash [M], particle [M], sound [E] |
| **A5 Stagger / posture break** | Hitstop 150 ms [E] → break sound [E] → Camera zoom-in [M: Camera Zoom ST-005, or FOV Kick E] → sustained glow on enemy while staggered [M: Sustained + Overlay Flash] → finisher prompt widget punch [M: UI steps] |
| **A6 Enemy death** | Slow-mo [E] → Desaturate [M] → Scale Punch then shrink [E] → particle [M] → "last enemy" variant with longer slow-mo [M: Feel Map variants] |
| **A7 Pickup + combo streak** | Scale Punch on pickup [E] → sound with pitch rising by streak [M: Escalation → parameter → pitch mapping] → counter Widget Punch [M: UI steps] → small Force Feedback [E] |

**Capability gaps:** Anim notify triggers, Play Context (asymmetric targets, direction), Parameters, Feel Map variants, Nested recipe, Sustained, Escalation, Spawn Particle, Overlay Flash, Light Flash, Screen Color, UI steps, Number Pop, Camera Zoom (optional).
**Tutorial outline:**
1. Create the Combat variant; open the attack montage.
2. Add `AnimNotify_FeelEvent`; **preview the recipe while scrubbing the montage** (headline feature).
3. Build R_Hit with a Damage parameter; drag the preview slider from light to heavy.
4. Add crit and stagger variants in FM_Melee.
5. Asymmetric hitstop from instigator/victim context.
6. Charge ability with sustain.
7. Streak pickups.
8. Comfort check: Hitstop group at 0.3.

**Content:** Combat variant (buyer-side). Plugin samples: overlay flash material, one slash particle. **Avoid** hit-reaction animation packs; say "pairs well with any hit reaction system" (Fab has several, SEARCH).

### 5.3 Platformer: "Bounce Feel" (base: Platforming or Side Scroller variant)

| Moment | Composition |
|---|---|
| **P1 Jump** | Trigger: character jump [M: Triggers (component; TRG-002 lists OnLanded but not OnJumped)] → Squash & Stretch (stretch) [E] → jump sound with pitch jitter [E] → dust particle [M: Spawn Particle] |
| **P2 Land** | OnLanded with fall speed [M: Triggers + Parameters] → Squash scaled by speed [E + M: Parameters] → Procedural Shake only above a speed threshold [M: Parameters] → dust by surface [M: Spawn Particle, Feel Map variants (optional)] → Force Feedback [E] → sound [E] |
| **P3 Dash (Celeste-style)** | Global Hitstop 30 ms (short, must not eat input) [E] → Camera Punch along dash direction [M: Play Context] → Chromatic Aberration [E] → afterimage trail [M: Ghost Trail (defer)] |
| **P4 Bounce pad** | Overlap trigger [M: Triggers] → Squash on pad [E] → Scale Punch on player [E] → FOV Kick [E] → boing sound [E] |
| **P5 Coin streak** | Scale Punch on coin [E] → pitch rising with streak [M: Escalation] → counter Widget Punch [M: UI steps] → "+1" Number Pop [M] |
| **P6 Take damage (Hollow Knight-style)** | Global Hitstop 80 ms [E] → Screen Flash [E] → audio low-pass while invulnerable [M: Audio Mix + Sustained] → Shake [E] → Material Pulse or Overlay Flash [E / M] → Force Feedback [E] |
| **P7 Crumbling platform** | Mesh shake/wobble on platform [M: Mesh Wobble ST-020, or Camera-independent Actor Shake] → dust particle [M] → rumble sound [E] |

**Capability gaps:** Triggers (jump, landed, overlap), Parameters, Play Context direction, Escalation, UI steps, Number Pop, Spawn Particle, Audio Mix, Sustained, Mesh Wobble, Ghost Trail (defer).
**Tutorial outline:**
1. Create the Platforming variant.
2. Add `UFeelTriggerComponent` to the character, mapping Jumped, Landed and Damaged to events. No Blueprint.
3. Tune Land with the fall-speed parameter preview.
4. Dash direction from context.
5. Coin streak escalation.
6. The damage muffle.
7. Before/after capture GIF [M: Capture to GIF].

**Content:** Platforming or Side Scroller variant (buyer-side); CC0 coin and pad meshes (Kenney, Quaternius; CC0 VERIFIED via license guides).

### 5.4 Horror: "Heartbeat" (base: Survival Horror variant)

| Moment | Composition |
|---|---|
| **H1 Ambient dread (sustained)** | Sustained recipe driven by Fear 0–1 [M: Sustained + Parameters] → low-frequency Procedural Shake (tiny) [E] → Vignette mapped to Fear [E + M: Parameters] → low-pass mapped to Fear [M: Audio Mix] |
| **H2 Heartbeat by monster distance** | Sustained; live parameter update from distance [M: Sustained + live parameters] → heartbeat Play Sound looped per beat [E + M: Sustained] → Force Feedback pulses synced to beats [E] → Vignette Pulse per beat [E] |
| **H3 Jump scare** | Audio duck 100 ms before the stinger [M: Audio Mix] → stinger sound [E] → Camera Punch back [E] → FOV Kick dolly effect [E] → Screen Flash **flash-limited** [E + M: Flash Limiter] → Chromatic Aberration [E] → Force Feedback [E] → Slow-mo Ramp 0.8× [E] |
| **H4 Flashlight failing** | Light Flicker on the flashlight component [M: Light Flash/Flicker] → electrical crackle sound [E] → optional sensory substitute: widget icon pulse for players with reduced audio [M: Sensory Substitution, UI steps] |
| **H5 Taking damage / low sanity** | Desaturate [M: Screen Color] → Radial Blur [M] → Pitch Bend on music [M: Audio Mix] → Chromatic Aberration [E] |
| **H6 Comfort showcase** | Toggle Reduced Motion and Reduced Flashing live in the preview [E, PV-006] → limiter visibly holds a flash burst under 3/s [M: Flash Limiter] → caption/haptic substitute for the heartbeat [M: Sensory Substitution] |

**Capability gaps:** Sustained, Parameters (live update), Audio Mix, Screen Color, Light Flicker, Flash Limiter, Sensory Substitution, UI steps.
**Tutorial outline:**
1. Create the Survival Horror variant.
2. Build the Fear-driven dread loop and preview it with the Fear slider.
3. Add a heartbeat with a distance parameter updated from the monster BP (one node: `SetFeelParameter`).
4. Build the jump scare on a trigger volume.
5. Flashlight flicker.
6. The accessibility pass: this chapter doubles as the "comfort is free" marketing piece for studios.

**Content:** Survival Horror variant (buyer-side). Heartbeat and stinger sounds must be plugin-owned or licensed for redistribution.

### 5.5 UI: "Juicy Menus" (base: blank project + plugin sample widget)

| Moment | Composition |
|---|---|
| **U1 Button hover / press** | Widget Punch 1.0 → 1.06 spring [M: UI steps] → click sound 2D [E] → tiny Force Feedback tick on gamepad [E] |
| **U2 Denied / error** | Widget Shake horizontal [M: UI steps] → buzz sound [E] → Widget Flash red [M: UI steps] |
| **U3 Notification / achievement** | Widget slide-in (UMG animation, owned by the buyer) → Widget Punch [M] → chime sound [E] → subtle Screen Flash 3% [E] |
| **U4 Score count-up (Balatro-style)** | Number Pop with count-up [M: Number Pop] → shake tier by score parameter (3 tiers) [E Shake + M: Parameters] → per-increment rising pitch [M: Escalation] → threshold crossing: bass sound + Screen Flash [E] |
| **U5 Screen transition** | Screen Fade [M: Screen Color steps (Fade ST-016)] → whoosh sound [E] |
| **U6 Floating damage numbers** | Number Pop in world space with crit variant and color by damage type [M: Number Pop + Feel Map variants] |

**Capability gaps:** UI steps, Number Pop, Parameters, Escalation, Screen Fade, UI preview in the recipe editor (the preview viewport has a mock HUD per PV-001 but no widget targets) [M: UI Preview].
**Tutorial outline:**
1. Add FeelKit's sample menu widget.
2. Bind button events to recipes (Blueprint call, or a small "Feel Button" behavior; keep it out of CommonUI styling).
3. Score counter tiers.
4. Floating numbers from the shooter demo.
5. Comfort "UI motion" slider.

**Content:** all plugin-owned (simple widgets, CC0 UI sounds such as Kenney's UI audio packs).

### 5.6 Cross-demo content and licensing notes
- **Template variants:** tutorials use them in the buyer's project. Shipping them *inside* the Fab package is an open question; Epic content redistribution rules on Fab could not be read (403). **Safe default:** do not ship template assets; ship recipes, Feel Maps and plugin-owned samples, and instruct buyers to create the variant.
- **Fab demo content:** Quixel, MetaHuman and "Permanently Free" collection assets are reportedly not allowed as demo content (SEARCH via Fab support summary). Treat as a constraint until verified.
- **Lyra:** do not ship; write a guide only.
- **CC0 sources:** Kenney and Quaternius (CC0 per license guides, SEARCH). Poly Haven (CC0, INFERRED).
- **Audio:** own recordings or libraries that allow redistribution inside a product. Sonniss GDC bundle forbids raw redistribution (SEARCH); the question is whether sounds shipped inside a code plugin count as "raw". Open question; avoid.
- **Trailer and marketing video** can use template variants freely inside a UE project (INFERRED; confirm the Epic Content EULA for promotional use).

---

## 6. Capability priority ranking (derived from the demos)

Count = number of the 5 demos that need the capability for at least one showcase moment. "Weight" adds judgement: whether the capability is **structural** (changes data or API, so later is costlier) and whether its absence **blocks a genre**.

| Rank | Capability | Demos | S / Sh / A / P / H / U | Structural? | Blocks a genre? | Existing req ID |
|---|---|---|---|---|---|---|
| 1 | **Recipe Parameters** (incl. built-in Distance, live update) | **5** | Sh A P H U | **Yes** (data model) | Horror, UI tiers | Strategy 3.3 (not in reqs) |
| 2 | **Play Context** (direction, instigator/victim, normal, surface, tags) | 4 | Sh A P H | **Yes** (API) | Shooter directional, Action asymmetric | new |
| 3 | **Feel Events + triggers** (anim notify/state, trigger component, SendFeelEvent) | 4 | Sh A P H | **Yes** (API) | none, but most of the "no Blueprint" pitch | API-006, TRG-001/002 |
| 4 | **Escalation accumulators** | 4 | Sh A P U | Partly | Action combos, UI tiers | new |
| 5 | **UI steps** (widget punch/shake/flash) | 4 | Sh A P U | No | UI | ST-029–031 |
| 6 | **Number Pop / Hit Marker** | 4 | Sh A P U | No | UI, Shooter | ST-032 |
| 7 | **Spawn Particle** (Niagara module) | 4 | Sh A P H | No | none, but every hit looks bare without it | ST-033 |
| 8 | **Screen Color steps** (desaturate, tint, radial blur, fade) | 4 | Sh A H U | No | Horror | ST-013–016 |
| 9 | **Sustained / looping recipes** | 3 | Sh A H (+P muffle) | **Yes** (runtime + editor) | **Horror** | new |
| 10 | **Audio Mix steps** (duck, low-pass, pitch) | 3 | Sh P H | No | Horror | ST-023–025 |
| 11 | **Hit-Flash Overlay** (overlay material) | 3 | Sh A P | No | none | new |
| 12 | **Light Flash / Flicker** | 3 | Sh A H | No | none | ST-021 |
| 13 | **Feel Map context variants** | 3 | Sh A (+U) | Part of #3 | Shooter surfaces/crits | new |
| 14 | Flash Limiter + Sensory Substitution | 2 | Sh H | No | Accessibility pitch | CMF-030–033 + new |
| 15 | Decal | 2 | Sh A | No | – | ST-034 |
| 16 | Nested / Random recipe | 2 | Sh A | No | – | ST-035, ST-037 |
| 17 | Moment capture / debugger | 5 as *tutorial aid*, 0 as need | all | Partly (record context) | – | TL-002, strategy 3.2 |
| 18 | Capture to GIF | 5 as *marketing*, 0 as need | all | No | – | strategy 3.6 |
| 19 | Mesh Wobble, Camera Zoom/Roll, Ghost Trail | 1 each | P / A / P | No | – | ST-020, ST-004/005 |
| 20 | Advanced haptics (trigger resistance) | 1–2 (optional) | Sh H | No | – | ST-027/028 |
| 21 | Networking | 0 (single-player demos) | – | Partly | Multiplayer buyers | NET-002/003 |
| 22 | Beat sync | 0 | – | No | – | new |

**Reading the ranking:**
1. **The top three are inputs, not effects.** The steps at ranks 5–12 are each small and already planned. Parameters, Context and Feel Events change the API and data model, and every later recipe depends on them. **Do them first, before recipe content (CT-001), or every shipped recipe gets migrated.**
2. **Sustained recipes rank 9 by count but block an entire genre.** Horror cannot be shown without them, and they are structural, so schedule them together with Parameters.
3. **Escalation is the cheapest high-count item** once Parameters exist.
4. **Networking and beat sync** are market requirements (multiplayer) or niche (rhythm), not showcase requirements. Keep them later.
5. A suggested grouping, for Bill to decide phase placement (not a phase change):
   - **5B "Inputs":** Parameters, Context, Feel Events, anim notify/state, trigger component, Escalation, Sustained.
   - **5C "Senses":** Spawn Particle, Screen Color, Audio Mix, Overlay Flash, Light Flash, UI steps, Number Pop, Decal, Nested/Random.
   - **5D "Trust":** Flash limiter, substitution, debugger/capture, networking.

---

## 7. Scope-creep warnings

Each item looks attractive and would dilute FeelKit. "Tell" = the sign you have crossed the line.

1. **Hit reaction / animation systems.** Crowded on Fab (Ultimate Hit Reaction System, HitReact Pro, animation packs, free ProcHitReact; SEARCH), content-heavy (100+ mocap clips) and a different buyer. *Tell:* FeelKit ships animations or a physical animation profile. **Allowed:** a montage play-rate hitstop step and a Blueprint Event step that calls the buyer's reaction.
2. **VFX, SFX and impact content libraries.** Requirements 2.2 already exclude them; Fab has many. *Tell:* more than one sample per step, or "18 surface types".
3. **A combat, damage or GAS replacement.** Apex-style shield tiers and crits are *game state* the game sends as context. *Tell:* FeelKit computes damage, stores health or decides hits.
4. **A generic event bus / message router.** The Feel Event payload stays fixed and has only recipe listeners. *Tell:* arbitrary struct payloads, gameplay code listening to FeelKit events, return values.
5. **A floating-text / HUD framework.** *Tell:* layout systems, many styles, localization formatting, health bars.
6. **UI animation framework competing with UMG/CommonUI.** *Tell:* transitions between screens, navigation, a widget-state machine. Punch, shake, flash, pop and fade only.
7. **Camera rigs or modes.** Requirements 2.2 exclude them; Gameplay Cameras is Epic's path. *Tell:* spring arms, follow logic, framing. Compatibility with GPC is allowed.
8. **Audio middleware features.** *Tell:* a mixer, snapshot system, music system or custom DSP. Steps that post to MetaSounds, Wwise or FMOD are fine.
9. **Audio-to-haptic conversion / haptic synthesis.** Solved and free (Interhaptics, Meta Haptics Studio; VERIFIED). *Tell:* FeelKit analyzing waveforms for haptics. (Waveform *display* for timeline snapping, strategy 3.4, is fine.)
10. **Telemetry and analytics.** Needs a backend, privacy work and dashboards. Nobody buys a feel plugin for it. *Tell:* anything that sends data off the machine.
11. **Recipe marketplace / preset store.** Needs hosting, moderation and payments. JSON + GitHub + Discord gets 80% of the ecosystem value. *Tell:* accounts or uploads.
12. **Settings menu framework.** Requirements 2.2 exclude it; Auto Settings exists on Fab (SEARCH). *Tell:* graphics or keybinding options in FeelKit's widget.
13. **Beat-synced recipe timelines.** Re-timing tracks to BPM is a rhythm-game engine. *Tell:* BPM fields on recipes. A quantized play start only.
14. **Movement feel (coyote time, input buffering, corner correction).** This is where much of Celeste's feel comes from (VERIFIED), but requirements 2.2 exclude it, and it is gameplay.
15. **General-purpose property binding on steps.** Parameter → any UPROPERTY is a slippery slope toward a visual scripting system. Start with intensity plus a small, explicitly flagged set of step properties.

---

## 8. Assumptions, open questions, sources

### 8.1 Assumptions
- A1. Effort estimates assume one experienced Unreal C++ developer familiar with the FeelKit codebase (INFERRED).
- A2. Demo counts reflect *showcase* moments as I designed them; different demo choices would shift counts slightly. The ranking of the top 3 holds under any reasonable variation.
- A3. Code facts come from a grep and read-only look at `FeelCore/Public` on 2026-09-16: no `Parameters` on `UFeelRecipe`, no context on `FFeelTarget`, `FFeelConditions` defined but no evaluation found under `FeelCore/Private`, no loop/sustain concept, Camera Punch direction fixed in camera space with jitter. Not a full audit.
- A4. Competitor states (Game Juice Pro, FeedbackFX) come from earlier strategy research plus forum/search snippets. Fab listing pages returned 403 and were not re-read.

### 8.2 Open questions
1. **Fab content rules:** can a code plugin ship assets derived from UE template variants or Epic Content? Can shipped plugin content include licensed sounds (Sonniss-style licenses)? Ask Fab support before building the demo map.
2. **Gameplay Cameras compatibility:** does a `UCameraModifier` still apply when a project uses Gameplay Cameras in 5.7/5.8? Needs a spike.
3. **Sustained recipes and pure evaluation:** how do `OnStart`/`OnStop` side effects (sounds, haptics) behave across loop passes and scrubbing? Needs a design note before implementation.
4. **Parameter mapping targets:** intensity only at first, or a fixed list of step properties? Decide before the data-model change.
5. **Feel Map matching rule:** most-specific match (as for comfort mapping) or first match with priority? Recommendation: most-specific, then priority, with an editor "which row wins" tester.
6. **Asymmetric targets:** one play with two targets (instigator and victim), or two plays? It affects API shape and cooldowns.
7. **Number Pop rendering:** UMG pooled widgets or a single Slate/Canvas layer? Performance vs. styling trade-off.
8. **Overlay Flash conflicts:** projects already using `OverlayMaterial` (outlines, highlights) would be overwritten. Save and restore, or skip with a warning?
9. **OD-001 name:** still open per strategy. Feel Events and Feel Maps put "Feel" into even more buyer-facing asset types, which raises the cost of renaming later.
10. **Monster Hunter, SF6, Sekiro and Hades details** are SEARCH or INFERRED. If these are quoted in marketing, verify first-hand.

### 8.3 Sources

**Games and talks**
- Jan Willem Nijman, The Art of Screenshake (video): https://www.youtube.com/watch?v=AJdEqssNZ-U
- Trick list transcription (VERIFIED): http://artificials.ch/juice-up-your-game/
- Squirrel Eiserloh, Juicing Your Cameras With Math, transcript (VERIFIED): https://archive.org/stream/GDC2016Eiserloh/GDC2016-Eiserloh_djvu.txt ; GDC Vault: https://gdcvault.com/play/1023146/Math-for-Game-Programmers-Juicing
- DOOM push forward combat (VERIFIED): https://www.gamedeveloper.com/game-platforms/pushing-push-forward-combat-with-gameplay ; GDC Vault: https://www.gdcvault.com/play/1024940/Embracing-Push-Forward-Combat-in
- DOOM glory kill cues (SEARCH): https://strategywiki.org/wiki/Doom_(2016)/Gameplay ; https://doom.fandom.com/wiki/Glory_Kill
- Hollow Knight damage effect discussion (VERIFIED): https://steamcommunity.com/app/367520/discussions/0/135510393196991561/
- Celeste & Forgiveness (VERIFIED): https://www.mattmakesgames.com/articles/celeste_and_forgiveness/index.html
- Celeste options (SEARCH): https://steamcommunity.com/app/504230/discussions/0/1779388938816968149/ ; https://maddymakesgamesinc.itch.io/celeste/devlog/237097/v1400-changelog
- Dead Cells accessibility (VERIFIED): https://deadcells.wiki.gg/wiki/Assist_Mode_and_Accessibility
- Sekiro deflect analysis (403) and snippets (SEARCH): https://medium.com/@gatherer286/song-of-sword-and-fist-sifu-sekiro-and-the-anatomy-of-a-perfect-parry-2f9c4c26867a ; https://parryeverything.com/category/sekiro/
- Monster Hunter Wilds hitstop (SEARCH): https://www.gamesradar.com/games/monster-hunter/putting-beta-feedback-into-action-monster-hunter-wilds-devs-show-off-improved-hitstop-and-reworked-weapons-that-feel-how-you-remember-them/ ; https://opencritic.com/news/8376/monster-hunter-wilds-players-miss-the-hit-stop-from-world
- Street Fighter 6 offense (SEARCH): https://wiki.supercombo.gg/w/Street_Fighter_6/Offense
- Guilty Gear counter hits (SEARCH): https://guiltygear.fandom.com/wiki/Offense
- God of War combat by other developers (VERIFIED): https://blog.playstation.com/2022/10/04/game-developers-explain-what-makes-god-of-war-2018s-combat-tick/
- Axe recall (SEARCH, 403): https://www.pushsquare.com/news/2018/04/heres_why_recalling_kratos_leviathan_axe_in_god_of_war_feels_amazing
- Returnal DualSense (VERIFIED): https://blog.playstation.com/2021/05/13/how-housemarque-created-returnals-immersive-dualsense-controller-effects/
- Astro Bot / Doucet interview (VERIFIED): https://onemoregame.ph/2024/09/astro-bot-director-nicolas-doucet-interview/
- Ratchet & Clank Rift Apart triggers (SEARCH): https://www.psu.com/news/ratchet-clank-rift-apart-dualsense-features-will-make-each-weapon-feel-unique/
- Dead Space remake Intensity Director (VERIFIED): https://gamingbolt.com/dead-space-remake-peeling-system-intensity-director-new-voicelines-for-isaac-and-more-detailed
- RE4 remake DualSense (SEARCH): https://www.digitaltrends.com/gaming/resident-evil-4-dualsense-controller/ ; https://github.com/RomarioCyrus/RE4R-DualSense-Enhanced-Edition
- Hi-Fi Rush music-synced animation (VERIFIED): https://www.gameanim.com/2023/09/08/hi-fi-rush-music-synced-animation/
- Balatro feedback guide (VERIFIED): https://blakecrosley.com/guides/design/balatro
- Overwatch, Play by Sound, transcript (VERIFIED): https://archive.org/stream/GDC2016Lawlor/GDC2016-Lawlor_djvu.txt ; https://gdcvault.com/play/1023317/Overwatch-The-Elusive-Goal-Play
- Apex damage number colors (SEARCH): https://www.shacknews.com/article/110042/how-to-change-damage-numbers-in-apex-legends
- Destiny precision damage (SEARCH): https://d2.destinygamewiki.com/wiki/Precision_Damage
- Titanfall 2 GDC (SEARCH, members only): https://gdcvault.com/play/1024056/Solving-Titan-Sized-Problems-Evolving
- DMC5 style ranks (SEARCH): https://www.shacknews.com/article/110322/stylish-points-and-style-ranks-in-devil-may-cry-5
- Diablo IV VFX readability (SEARCH): https://news.blizzard.com/en-gb/article/23938289/diablo-iv-open-beta-retrospective-transforming-feedback-into-change
- Hades talks (SEARCH, narrative only): https://gdcvault.com/play/1027149/Breathing-Life-into-Greek-Myth

**Accessibility**
- Xbox Accessibility Guideline 118 (VERIFIED): https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/118
- Game Accessibility Guidelines, flicker (SEARCH): https://gameaccessibilityguidelines.com/avoid-flickering-images-and-repetitive-patterns/
- Game Accessibility Guidelines, audio info replicated visually (SEARCH): https://gameaccessibilityguidelines.com/ensure-that-all-important-supplementary-information-eg-the-direction-you-are-being-shot-from-conveyed-by-audio-is-replicated-in-text-visuals/
- The Last of Us Part II accessibility (VERIFIED): https://www.playstation.com/en-us/games/the-last-of-us-part-ii/accessibility/
- Multisensory Accessibility Plugin for UE5 (VERIFIED): https://fortesdev.wordpress.com/2025/05/25/multi-sensory-accessibility-plugin-for-unreal-engine-5/

**Unreal engine and workflow**
- GameplayCueNotify_BurstEffects (VERIFIED): https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GameplayCueNotify_BurstEffects?application_version=5.1
- GameplayCueNotify_SpawnCondition (VERIFIED): http://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GameplayCueNotify_SpawnCondition?application_version=5.0
- UGameplayCueNotify_Burst (SEARCH): https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/GameplayAbilities/UGameplayCueNotify_Burst
- Lyra weapons (VERIFIED): https://x157.github.io/UE5/LyraStarterGame/Weapons/
- Lyra Context Effects (SEARCH): https://deepwiki.com/soatori/Lyraa/3.4-context-effects-system
- Gameplay Message Router outside Lyra (SEARCH): https://forums.unrealengine.com/t/i-want-to-use-the-gameplaymessagerouter-plugin-from-lyra-in-my-project/1337495
- Anim notify at non-standard play rates (SEARCH): https://bugnet.io/blog/fix-unreal-anim-notify-not-firing-at-playback-rate
- Force Feedback in Unreal (VERIFIED): https://dev.epicgames.com/documentation/en-us/unreal-engine/force-feedback-in-unreal-engine
- Device Properties (VERIFIED): https://dev.epicgames.com/documentation/unreal-engine/device-properties-in-unreal-engine?lang=en-US
- Quartz overview (SEARCH): https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-quartz-in-unreal-engine
- MetaSounds parameters (SEARCH): https://uhiyama-lab.com/en/notes/ue/metasounds-basics/
- Wwise RTPC in Unreal (SEARCH): https://alessandrofama.com/tutorials/wwise/unreal-engine/rtpcs
- Gameplay Cameras status (SEARCH): https://ludovic.chabant.com/blog/2025/06/06/ue5-gameplay-cameras-upgrading-to-5-6/ ; https://www.strayspark.studio/blog/gameplay-cameras-plugin-ue5-7-production-guide
- OverlayMaterial API 5.6 (SEARCH): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Components/UMeshComponent/OverlayMaterial
- Rewind Debugger (SEARCH): https://dev.epicgames.com/documentation/unreal-engine/animation-rewind-debugger-in-unreal-engine
- UMG animation (SEARCH): https://dev.epicgames.com/documentation/unreal-engine/animating-umg-widgets-in-unreal-engine
- Template variants (VERIFIED): https://dev.epicgames.com/documentation/unreal-engine/variants-in-game-templates

**Haptics tools**
- Meta Haptics Studio with FMOD/Wwise (VERIFIED): https://developers.meta.com/horizon/blog/meta-haptics-studio-meets-fmod-wwise/
- Meta Haptics SDK for Unreal (SEARCH): https://developers.meta.com/horizon/documentation/unreal/unreal-haptics-sdk/
- Interhaptics free SDK (SEARCH): https://www.xda-developers.com/razer-interhaptics-haptic-composer-free-game-developers/ ; https://doc.wyvrn.com/docs/interhaptics-sdk/haptics-unreal-sdk/
- Unreal-Dualsense Pro (VERIFIED): https://unreal-dualsense-pro.valoto.games/
- Windows DualSense for UE5 (SEARCH): https://github.com/LeoCodx62/WindowsDualsenseUnreal5
- Altercode DualSense (SEARCH): https://altercode.gumroad.com/l/unreal-engine-5-dualsense-plugin
- Nice Vibrations / Lofelt (SEARCH): https://github.com/Lofelt/NiceVibrations

**Competitors and adjacent products**
- Unity Feel docs (VERIFIED): https://feel-docs.moremountains.com/ ; MMF_Player (VERIFIED): https://feel-docs.moremountains.com/mmf-player.html ; releases (VERIFIED): https://feel.moremountains.com/feel-releases
- Agentic FeedbackFX forum post (VERIFIED): https://forums.unrealengine.com/t/insodimension-agentic-feedbackfx-screenfx-hitstop-impact-frames-haptics-shakes-c/2695683 ; Fab listing (403): https://www.fab.com/listings/ad3d6526-fca0-4760-99b2-9adf368080b6
- Game Juice Pro (SEARCH): https://www.fab.com/listings/05737cd2-05de-48ad-a52b-ded62a390987
- BSLContextEffects (VERIFIED): https://github.com/BajaShortLong/BSLContextEffects
- Combo Graph gameplay cues (VERIFIED): https://combo-graph.github.io/gameplay-cues/
- Physical Material Profiles (SEARCH): https://www.fab.com/listings/de83eb00-77a4-4732-a388-6234a8d3cc6c
- Easy Footstep Plugin (SEARCH): https://www.fab.com/listings/197f2bb4-e352-4cf7-a514-478262adb036
- Bullet Impact VFX (SEARCH): https://www.fab.com/listings/43bdd657-063d-4ece-8e1b-f131a9d1d406
- Floating damage text products (SEARCH): https://www.fab.com/listings/a389663e-1041-4b21-b010-339c1e763ef9 ; https://www.fab.com/listings/ff1928f0-023d-4226-bcf5-e7ead7e16054
- HitTicker (SEARCH): https://github.com/SimulatedFlow/ue-plugin-HitTicker
- Ultimate Hit Reaction System (SEARCH, 403): https://www.fab.com/listings/0c79e08b-0440-4dfc-9321-79ba4a9dcbe3
- HitReact Pro (SEARCH): https://www.fab.com/listings/dd1558d5-ce68-4e93-a931-954edd89d974
- UE Accessibility Toolkit (SEARCH): https://www.fab.com/listings/86b3b74f-7d8c-4ce3-9803-5d692ff124a4
- Auto Settings (SEARCH): https://www.fab.com/listings/6166897a-d04b-46d5-b2cc-a692a74b0698

**Licensing**
- Fab asset structure requirements (VERIFIED, silent on Epic content): https://dev.epicgames.com/documentation/fab/asset-file-format-and-structure-requirements-in-fab
- Fab demo content usage (page did not render; SEARCH summary): https://support.fab.com/s/article/Fab-Demo-Content-Usage?language=en_US
- Epic Content EULA: https://www.unrealengine.com/eula/content
- Sonniss GDC bundle license (SEARCH): https://sonniss.com/gdc-bundle-license/
- Quaternius license guide (SEARCH): https://www.licenseorg.com/guide/3d-assets/quaternius
- Kenney CC0 (SEARCH): https://forum.godotengine.org/t/kenneys-assets-free-and-creative-commons-cc0/36658
