# FeelKit: Verdict and Dominance Plan

| Field | Value |
|---|---|
| Date | 2026-09-15 |
| Inputs | `CLAUDE.md`, `FeelKit_Requirements.md` v0.1, Phase 0 web research |
| Status | Proposal. Nothing here changes the requirements until Bill accepts it |

---

## 1. Verdict

**Build it.** The gap is real and still open. But the plan as written would ship a good juice plugin into a crowded, cheap category. To dominate, FeelKit has to stop being "another juice plugin" and become **the place Unreal game feel gets authored, tuned, and made accessible**.

### 1.1 Phase 0 findings (kill criteria)

| Check | Result |
|---|---|
| K-001: competitor ships timeline/preview or comfort layer | **Pass, with one gap.** Adrenaline GameJuice is 20 components set up in the Details panel. It has no timeline and no preview. For comfort, its docs only suggest driving one global intensity scale from a slider. Agentic FeedbackFX is data-asset configured and requires GAS + Niagara on UE 5.5+. Cronus Hitstop is hitstop only. **Not verified:** the Fab listing for Game Juice Pro (28 types, "Play Juice" node, preset data assets) returned 403. Open it manually and look for any editor tooling before Phase 1 code starts. |
| K-005: Epic native feedback framework | **Pass.** The UE 5.8 release notes have nothing on shakes, feedback, or comfort. Epic does ship a **Camera Shake Previewer** (Window menu), so "preview" alone is not a moat. Composed multi-channel preview on a timeline is. |
| K-004: a fourth juice plugin | **Already true in practice.** Game Juice Pro, GameJuice, FeedbackFX, Cronus Hitstop, plus several camera shake packs. Re-evaluate means: win on workflow, not on step count. |
| Benchmark | More Mountains **Feel** (Unity): 150+ feedback types, 4000+ Discord members, no Unreal version. Matching its step count is not realistic. Matching its *reputation* is. |

### 1.2 What is strong in the current plan
- Timeline + preview is the right differentiator, and Phase 1 correctly attacks the hardest risk first.
- "Steps compute, sinks apply" with a shared `FFeelEvaluator` is the right architecture. It is also what makes most of the ideas in section 3 cheap.
- Zero-dependency core, Blueprint-only support, Lite tier: all correct for Fab ranking.

### 1.3 What is weak or risky
1. **Name collision.** "Feel" is More Mountains' brand, the benchmark every Unity convert already knows. `FeelKit`, the `Feel` class prefix and `PlayFeel` will lose search to it and may attract a trademark complaint. **The class prefix is baked into every saved recipe asset** (and into buyers' Blueprints), so OD-001 must be resolved *before* Phase 1 code, not at launch.
2. **Scrubbing vs. the step lifecycle.** Requirements 4.3 define a stateful `OnStart / OnUpdate / OnStop` lifecycle. Scrubbing backwards (PV-002) only works if a step's output is a pure function of `(time, intensity, seed)`. A Perlin shake that advances internal state will look different every scrub. This is an architecture decision for Phase 1 (see 2.2).
3. **Competing on count.** MVP 17 steps / V1 35-40 is fine, but it should not be the pitch.
4. **Comfort is a studio sale, not a search term.** Solo devs do not search for "comfort layer". Leads and producers facing platform accessibility guidelines do. Position it for them, and give it away (see 3.1).
5. **Price.** $70-80 V1 is well above the juice plugins buyers will compare against. Justifiable only if the editor is obviously better within the first 60 seconds of the trailer.

---

## 2. Changes needed before Phase 1 code

These are small, but expensive to change later.

### 2.1 Decide the name and prefix (OD-001)
Pick a name that is not "Feel". Keep the `Recipe / Track / Step` vocabulary; it is good. If the name changes, update CLAUDE.md hard rule 8 in the same commit.

### 2.2 Make evaluation pure
- Add `UFeelStep::Evaluate(const FFeelEvalParams&, IFeelOutputSink&) const`, where the params are normalized time, local time, intensity and a per-instance seed.
- Noise is sampled from time + seed, never accumulated.
- `OnStart/OnStop` stay for side effects (sounds, spawns, events). Steps that need state (springs, physics) return `false` from a new `SupportsScrub()`, and the timeline shows them the same way as PV-004.
- Evaluator tests add: **same time + seed gives identical output, in any order.**

### 2.3 Reserve data model hooks (no feature work)
Add the fields now so recipe assets never need migration later. Leave the behavior for its phase.
- `FFeelTrack::Seed` (int32) and `RandomizeSeedPerPlay` (bool)
- `UFeelRecipe::Parameters`: named float inputs such as `Damage` or `Speed`. The per-track curves that consume them come later (3.3).
- A recipe schema version int for future upgrades.

### 2.4 Fix the environment
- This machine has Visual Studio **2019** only. From `B:\UE_5.6\Engine\Config\Windows\Windows_SDK.json`, UE 5.6 needs Visual Studio **2022 17.8+** with MSVC **14.38** (preferred; 14.39 and 14.40 are banned), Windows 11 SDK **10.0.22621**, and the "Game development with C++" workload. No Phase 1 build is possible until it is installed.
- `CLAUDE.md` points to `Docs/FeelKit_Requirements.md`, but the file sits at the project root.
- There is no `.uproject` yet. Phase 1 needs a C++ host project, created from text files only.

---

## 3. Dominance plan

Ranked by leverage per unit of effort. Each item names the phase where it fits, so none of it breaks the "stay inside the current phase" rule.

### 3.1 Make Comfort free and universal (Phase 3, Lite)
- **The whole comfort layer ships in Lite, including settings persistence and the sample widget.** Every free install adds another game whose settings menu runs on FeelKit.
- **Comfort applies to effects FeelKit did not play.** Scale engine-native camera shakes and force feedback through the same channel sliders, so "turn on reduced motion" works on a project's existing effects on day one. *Needs a technical spike:* confirm which engine hooks allow this without engine changes.
- **Public query API** (`GetComfortScale(Player, ChannelTag)`) so other plugins and game code can respect the same settings. Comfort becomes a small standard, not a feature.
- Keep the non-certification wording (2.2 of the requirements) everywhere.

### 3.2 Tune in the running game (promote PV-005, PV-009, TL-002 to MVP)
The preview viewport proves the workflow. Winning is tuning *in the actual game*:
- **Live tuning:** edits during PIE apply on the next play.
- **Moment capture:** the subsystem keeps a ring buffer of the last ~10 s of recipe plays. From it, "Open last hit in timeline" replays that moment and lets you scrub it. Because evaluation is pure (2.2), this costs little.
- **Event overlay** on the timeline: which arbiter capped or suppressed what, and why.

No competitor can demo "I hit the enemy, clicked, and scrubbed that exact hit."

### 3.3 Recipe parameters (Phase 4)
Competitors have one intensity scalar. FeelKit lets a recipe take named inputs (`Damage`, `Speed`, `Distance`), each track mapping them through a curve. One `Hit` recipe covers a scratch through a critical hit. Distance falloff is the same mechanism, with a built-in parameter.

### 3.4 Audio-aware timeline (Phase 4, with ST-022)
Draw the waveform on Play Sound tracks and snap other tracks to its transients. Lining up hitstop, flash and shake with the impact sound is the most common tuning task, and nobody visualizes it.

### 3.5 Text recipe format and AI authoring (Phase 5)
- JSON export/import of recipes: diffable, reviewable, easy to share in Discord.
- An editor command that turns a description ("heavy sword hit, crunchy, short hitstop") into a JSON recipe. It can use an LLM the user configures, or an MCP tool if the engine's MCP support is viable. *Verify the UE 5.8 MCP status before committing.* FeedbackFX already markets itself as "AI-native"; this beats it on its own pitch.

### 3.6 Built-in GIF/MP4 capture (Phase 4, before validation posts)
One button in the recipe editor renders a side-by-side **Off / On** clip of the preview. Every buyer's social post becomes an ad. It also produces the K-003 validation GIFs and the Fab gallery at no extra cost.

### 3.7 Lower the switching cost (Phase 5)
- **Import engine Camera Shake assets / `UCameraShakeBase` classes as recipe tracks.**
- A written "Coming from Feel (Unity)" guide that maps concepts one to one. Unity refugees are the easiest customers to reach.

### 3.8 Content is the product people see (Phase 6)
- Grow CT-001 to 50+ recipes. Every recipe gets an **animated thumbnail** (promote TL-005 to V1), rendered by the capture tool.
- Organize the recipe browser by *feeling* ("crunchy", "floaty", "heavy"), not only by genre.

### 3.9 Community and ecosystem (from Phase 4)
- Discord at the first validation post. Feel's moat is its community as much as its code.
- Step SDK doc + public GitHub repo of community steps (MIT). Third-party steps widen coverage without growing the core.

### 3.10 Pricing and launch
- Early access at or near competitor pricing, with a public roadmap. Raise the price at V1 once the editor is proven.
- The trailer opens with the timeline scrubbing a hit. No logo intro.

---

## 4. Proposed exit gates (additions)

| Phase | Added gate |
|---|---|
| 1 | Scrubbing any step backwards and forwards gives identical frames (determinism test) |
| 2 | Moment capture skeleton: last plays recorded with seeds |
| 3 | Comfort scales an engine-native camera shake the plugin did not play (spike result documented) |
| 4 | Capture tool produces the validation GIFs |

---

## 5. Decisions needed from Bill

1. **Name and class prefix** (blocks Phase 1 code).
2. **Adopt 2.2 pure evaluation** (blocks Phase 1 code).
3. Which section 3 items go into Requirements v0.2, and in which phase.
4. Install Visual Studio 2022 (blocks every build).

---

## Sources
- [Game Juice Pro on Fab](https://www.fab.com/listings/05737cd2-05de-48ad-a52b-ded62a390987) (403 on fetch, summary from search index)
- [Adrenaline Games GameJuice documentation](https://adrenalinegames.pl/GameJuice.html)
- [Agentic FeedbackFX on Fab](https://www.fab.com/listings/ad3d6526-fca0-4760-99b2-9adf368080b6)
- [Cronus Hitstop on Fab](https://www.fab.com/listings/31adfbe6-c4c9-42d6-afba-31af742f648d)
- [Feel by More Mountains](https://feel.moremountains.com/)
- [UE 5.8 release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes?lang=en-US)
- [Camera Shakes in Unreal Engine (Camera Shake Previewer)](https://dev.epicgames.com/documentation/unreal-engine/camera-shakes-in-unreal-engine)
