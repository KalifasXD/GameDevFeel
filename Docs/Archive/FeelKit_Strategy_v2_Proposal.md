# FeelKit Strategy v2: Proposal

| Field | Value |
|---|---|
| Date | 2026-09-16 |
| Status | **Accepted 2026-09-16 with changes**: editions are free Lite (with comfort) + Pro (everything), no Studio tier; name FeelKit kept; one documented tutorial only. See `FeelKit_ProjectLog.md` D-012 to D-019 |
| Inputs | `Docs/Research/Competitor_Analysis_2026-09-16.md` (T-1), `Docs/Research/Adjacent_Systems_and_Demos_2026-09-16.md` (T-2), `FeelKit_Strategy.md` (Phase 0), current code |
| Supersedes | Nothing yet. If accepted, replaces section 3 ranking of `FeelKit_Strategy.md` |

---

## 1. The honest answer

**A. Are we already meaningfully ahead?** No, not as a product. We are ahead as an *editor*.

| What we lead | What a buyer sees first, where we trail |
|---|---|
| Only timeline authoring in Unreal; composed multi-channel preview with scrubbing and audio; cross-channel arbiters with guaranteed time restore; per-player comfort with presets and persistence | ~15 effects vs 28 (Game Juice Pro, $29.99) and 150+ (Feel); no chance / random ranges / distance falloff; no triggers; no networking; no persistent effects (low-health vignette); no presets, demos, docs or community; UE 5.6 only; planned price 2x the category |

Two leads are eroding: Feel (Unity) shipped edit-mode preview in July 2026, and Sparkle (Godot, $20) previews too. "Preview without Play" will be expected by 2027.

**B. What makes us clearly different?** Not more effects. Both studies independently land on the same structural fact:

> **Great feedback in shipped games is a response to an event, chosen by context and scaled by game state.** FeelKit today only has the response half. It has no *what happened* (event and context), no *how big* (parameters, escalation), and no *while it lasts* (sustained recipes).

GAS cues and Lyra Context Effects route events but end in fire-and-forget spawners: no timeline, no ordering, no arbitration, no comfort, no combined preview. Game Juice Pro and Feel have parameters but no routing by context and no visual authoring. **No product combines the four:** event routing with context variants, parameters visualised on a timeline, comfort that holds up to the Xbox camera-comfort check, and scrubbable capture of the real in-game moment.

## 2. Position

> **FeelKit is the response layer of an Unreal game.** Gameplay says *what happened* (a tag plus context). FeelKit decides *how it feels*: which variant, how strong, in what order, across camera, time, screen, audio, haptics, UI and VFX. Comfort-safe, previewable and scrubbable before anyone presses Play, and capturable and scrubbable after.

What we are not: a combat system, hit-reaction animation, VFX/SFX library, camera rig, settings menu, event bus, or haptics synthesizer (T-2 section 7 lists the "tells" for each).

## 3. Where I disagree with or sharpen the research

Being aggressive about our own thinking also means being aggressive about the research.

1. **"Close step breadth to ~30" (T-1) is the wrong target.** Counting effects is Game Juice Pro's game; we lose it to Feel forever. Prioritise steps by the demo matrix (T-2 section 6), which picks the ~10 that every showcase actually needs (particles, screen color, audio mix, overlay hit-flash, light flash, UI punch/shake/flash, number pop, decal, nested/random). Step count then lands around 25 as a side effect.
2. **The research reports conflict on audio-to-haptics.** T-1 ranks "generate haptics from audio" as a differentiator; T-2 says avoid conversion. Resolution: **avoid HD-haptics synthesis** (free tools own it); **allow a simple envelope follower** that turns a Play Sound track's amplitude into a Force Feedback curve and a shake envelope on the same timeline. That is a timeline feature, not DSP.
3. **Moment capture is the best unique feature, but it is the wrong next thing.** It becomes far more valuable once plays carry parameters and context (the capture must record them). Build inputs first, capture second.
4. **Networking is parity, but a missing checkbox kills sales to multiplayer buyers.** Ship it thin (replicate the event and its context, never the evaluation) before launch, and do not market it.
5. **Our Phase 5A work was necessary but is the least buyer-visible.** The editor was already our lead; we widened it. The next phases must attack the gaps a Fab page shows.
6. **Two latent defects undercut our strongest claims** and must be fixed first, whatever else is decided:
   - **I-007:** `EssentialFloor` keeps shake alive after a player sets shake to 0, which fails the Xbox Camera Comfort check. Our "comfort" pitch is currently false for motion channels.
   - **I-006:** `FFeelTrack::Conditions` (Chance, MaxDistance, ...) is editable but never evaluated. A designer who sets Chance = 0.2 gets silently ignored.
7. **The name decision (OD-001) gets more expensive every phase.** Feel Events and Feel Maps would add more buyer-facing `Feel` asset types. Fab search does not surface "feel"; it surfaces "juice" and "hitstop". Decide before Phase 5B, or accept the name permanently.
8. **Price.** $70-80 against a $15-40 category with single-digit review counts is not credible at launch. The research suggests Core ~$39.99 plus a Studio tier (capture, comfort audit, MCP tools). I agree, with a free Lite that includes comfort (Strategy 3.1) to build installs and reviews.

## 4. Proposed roadmap (phase placement is the user's decision)

Order is driven by: structural first (data/API changes before content), then what demos need, then trust and proof.

| Part | Name | Contents | Why here | Req IDs / new |
|---|---|---|---|---|
| 5A | Editor power (current) | Close check 6 | In progress | ED-007, ED-009, PV-005..009, TL-004 |
| **5B** | **Inputs** | Fix I-007 and I-006. Recipe Parameters (named, with defaults and range; built-in Distance) mapped per track through curves, with preview sliders that redraw the intensity graph. Play Context (direction, instigator/victim, normal, surface, context tags) and direction sources on directional steps. Conditions evaluated (chance, max distance, local player only) plus random intensity/duration ranges. Sustained recipes (sustain region, `StopFeel` releases into tail, `SetFeelParameter` on a live handle). Escalation accumulators (gain, decay, cap, settable). Feel Events: `SendFeelEvent` + `UFeelMap` (event tag + context tags → recipe, most-specific match wins, same rule as CMF-006). Triggers: `AnimNotify` / `AnimNotifyState`, `UFeelTriggerComponent` (landed, jumped, damaged, hit, overlap). Schema version on recipes. | Every demo needs these; they change the data model and API, so they must land before any recipe content | Strategy 3.3; API-006; TRG-001..004; CMF-021; new IDs needed |
| **5C** | **Senses** | Steps chosen by demo count: Spawn Particle (optional FeelNiagara module), Screen Color (desaturate, tint, radial blur, fade), Audio Mix (duck, low-pass, pitch), Hit-Flash Overlay (OverlayMaterial, save/restore), Light Flash/Flicker, UI Widget Punch/Shake/Flash, Number Pop / Hit Marker (pooled, thin), Decal, Nested and Random recipe. MetaSound parameter forwarding on Play Sound. | Makes a recipe represent "the whole hit" | ST-013..016, ST-021, ST-023..025, ST-029..035, ST-037, new (overlay flash) |
| **5D** | **Proof** (the uncopyable part) | Moment Capture: ring buffer of recent plays with seed, parameters, context, comfort, arbiter decisions; "Open in recipe editor" to scrub the exact hit with overlays; tweak and "Apply to recipe". Rewind Debugger track. Comfort Audit (scan recipes, shake assets, notifies; flag XAG camera-comfort risks; readiness helper, not certification). Flash limiter with XAG 118 thresholds and sensory substitution. Spike: comfort scaling of engine-native camera shakes and force feedback. Capture Off/On clip to GIF/MP4. Waveform on Play Sound tracks with transient snapping and the envelope follower. | Built on pure evaluation; competitors built on timers, springs or coroutines cannot replay exactly without a rewrite | Strategy 3.1, 3.2, 3.4, 3.6; TL-002/003; CMF-030..042; NF-005 |
| **5E** | **Reach** | Networked Feel Events (Multicast, OwnerOnly, SkipOwner, relevancy, documented hitstop-in-MP policy). Optional FeelGAS module (`GameplayCueNotify` that plays a recipe, magnitude → parameters). Optional FeelEnhancedInput binding. UE 5.7 and 5.8 builds (C-001). Gameplay Cameras compatibility spike. JSON import/export; MCP toolset on 5.8 behind version macros. | Parity and distribution; turns GAS from a free competitor into a channel | NET-002..005, C-001, C-003, Strategy 3.5 |
| 6 | Content and launch | Demos (see `FeelKit_Demos.md`), recipe library organised by feeling, animated thumbnails, docs, "Coming from Feel" guide, Discord, trailer | Needs 5B-5D to exist | CT-*, DOC-*, TL-001, TL-005, TL-006 |

**Demo-driven checkpoint:** at the end of 5B, build the Action/RPG sword-hit vertical slice (demo A1-A3) as the acceptance test. If a designer cannot build light, heavy and crit hits from one recipe with no Blueprint branching, 5B is not done.

## 5. What would make an Unreal developer say "I cannot get this elsewhere"

Ranked by (hard to copy) x (visible in 30 s) x (buyers who care) / cost:
1. **Hit the enemy, click, scrub that exact hit** in the timeline with arbiter and comfort decisions drawn on it, tweak, apply (5D).
2. **One event, many right answers:** `Event.Melee.Hit` becomes a different, composed, timed response by outcome, surface and damage, authored in one Feel Map and previewed with sliders (5B).
3. **Preview the recipe while scrubbing the attack montage** in the animation editor (5B notify + existing preview sink).
4. **Comfort that survives an accessibility review:** one player setting reduces FeelKit and engine-native shakes, flash limiting, sensory substitution, plus an audit report (5D).
5. **Your GAS cues, authored on a timeline and comfort-aware** (5E).

Explicitly parity, never marketed as differentiators: networking, anim notifies, particles, UI steps, presets, undo, validation, preview by itself.

## 6. Risks

| Risk | Mitigation |
|---|---|
| Scope creep into combat, animation, VFX content | Guardrails from T-2 section 7; fixed Feel Event payload; parameter mapping limited to intensity plus a flagged list of step properties |
| Sustained recipes vs pure evaluation (sounds on loop passes, scrubbing) | Design note before implementation; loop-aware lifecycle tests |
| Data-model migration of existing test recipes | Schema version and core redirects; do it in 5B before content |
| Solo-developer bandwidth; 5B is large | Split 5B into 5B.1 (parameters, context, conditions, I-006/I-007) and 5B.2 (sustain, escalation, events, triggers) |
| Fab content licensing for demos (template variants, sounds) | Ask Fab support before building demo maps; default to buyer-side templates + plugin-owned CC0 samples |
| UE 5.7/5.8 API drift, Gameplay Cameras path | 5E spikes; version macros per hard rule 1 |

## 7. Decisions needed from the user

1. **Accept the position** (section 2) and the roadmap order 5B Inputs → 5C Senses → 5D Proof → 5E Reach → 6 Content?
2. **I-007 comfort fix:** on Camera Shake and Camera Motion groups a player's 0 always means 0 (floor not applied; only non-motion substitutes allowed, with a validation warning). This changes CMF-021. Approve?
3. **I-006 Conditions:** implement in 5B.1 (recommended), or hide the property until then?
4. **OD-001 name and prefix:** keep `Feel` permanently, or rename before 5B with core redirects?
5. **Pricing model:** free Lite (with comfort) + Core ~$39.99 + Studio tier, or the original single ~$70-80?
6. **Demo plan** (`FeelKit_Demos.md`): approve the five demos and the sword-hit vertical slice as the 5B acceptance test?
7. **Ask Fab support** about shipping template-derived and licensed audio content (the user must send this; I cannot contact Fab on your behalf).
8. **Manual competitor checks** worth a small spend: buy Game Juice Pro ($29.99) to confirm it has no editor preview and inspect its latest update.
