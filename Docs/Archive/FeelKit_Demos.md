# FeelKit Demo Plan

| Field | Value |
|---|---|
| Date | 2026-09-16 |
| Status | **Approved 2026-09-16** with one change: only one documented tutorial (D-016). Free sounds allowed under redistribution-friendly licenses (D-017) |
| Source | `Docs/Research/Adjacent_Systems_and_Demos_2026-09-16.md` section 5 (full track compositions per moment), `Docs/FeelKit_Strategy_v2_Proposal.md` |

Demos are part of the product. They sell it, they teach it, and they decide what gets built: **every showcase moment is also an acceptance scenario**. A capability no demo needs is suspect; a demo moment we cannot build is a gap.

---

## 1. Two kinds of demo

| Kind | Viewer thinks | Format | Rule |
|---|---|---|---|
| **A. Showcase** | "This is what FeelKit lets me create." | Playable map per genre + 30-60 s video per genre + one combined trailer | Show composed moments, never a single effect in isolation. Every clip has an Off/On comparison. |
| **B. Workflow / tutorial** | "This is how quickly I create it." | **One** tutorial only (user decision 2026-09-16): a video or a doc with screenshots, linked as "see how to build something with FeelKit". Subject: the Action/RPG sword hit | Start with the finished moment, then open its recipe, then build it from an empty recipe: tracks, parameters, preview, tuning in PIE, trigger, comfort check. One Blueprint node at most. |

The per-genre "tutorial outline" lines in the research report are reference only; they are not produced.

Tutorial structure (the one documented tutorial):
1. Show the finished moment in game (Off, then On).
2. Open its recipe. Walk the timeline left to right (for example: Impact, Hitstop, Camera Punch, FOV Kick, Flash, Sound, Haptics).
3. Rebuild it from an empty recipe, previewing without PIE.
4. Add parameters and drag the preview slider (light to heavy).
5. Route it: Feel Map row or anim notify, no Blueprint branching.
6. Tune live in PIE; capture the last hit and scrub it.
7. Comfort pass: switch preview presets, show what changes and what is preserved.

---

## 2. The five demos

Detailed per-moment track lists: research report section 5. Summary and gating capabilities:

| Demo | Base (created in the buyer's project) | Showcase moments | Blocking capabilities (not built yet) |
|---|---|---|---|
| **Shooter**, "Every Bullet Has an Opinion" | UE 5.6 First Person Arena Shooter variant | Fire with sustained-fire escalation; surface impacts; hit confirm; headshot/crit; kill confirm; nearby explosion with distance falloff and punch away from blast; damage from direction and low-health heartbeat | Parameters, Play Context, Feel Events + variants, Escalation, Sustained, Spawn Particle, Decal, Light Flash, Overlay Flash, Screen Color, Audio Mix, Number Pop / Hit Marker, Flash Limiter |
| **Action/RPG**, "Weight Class" | Third Person Combat variant | Light, heavy and crit sword hits from one recipe; asymmetric hitstop (attacker vs victim); charged ability; stagger with sustained glow; death with last-enemy variant; pickups with combo streak | Anim notify triggers, Play Context, Parameters, Feel Map variants, Nested recipe, Sustained, Escalation, Spawn Particle, Overlay Flash, Light Flash, Screen Color, UI steps, Number Pop |
| **Platformer**, "Bounce Feel" | Third Person Platforming or Side Scroller variant | Jump; landing scaled by fall speed; dash along direction (hitstop that never eats input); bounce pad; coin streak with rising pitch; damage with audio muffle; crumbling platform | Trigger component, Parameters, Play Context, Escalation, UI steps, Number Pop, Spawn Particle, Audio Mix, Sustained, Mesh Wobble |
| **Horror**, "Heartbeat" | Survival Horror variant | Fear-driven ambient dread; heartbeat by monster distance with synced haptics; jump scare with flash limiting; failing flashlight; low sanity; live comfort showcase | Sustained + live parameters, Audio Mix, Screen Color, Light Flicker, Flash Limiter, Sensory Substitution, UI steps (**cannot be built at all without Sustained**) |
| **UI**, "Juicy Menus" | Blank project + plugin sample widget | Button hover/press; denied; notification; Balatro-style score count-up with shake tiers; screen transition; floating damage numbers | UI steps, Number Pop, Parameters, Escalation, Screen Fade, UI preview in the recipe editor |

**What exists today and can already be shown:** hitstop, camera punch, FOV kick, shake, flash, vignette, chromatic aberration, squash and stretch, scale punch, material pulse, sound, force feedback, Blueprint event; timeline, preview, comfort preview, intensity graph, Play in PIE, live edits.

**Signature moment for the trailer and the 5B acceptance test:** Action/RPG sword hit A1-A3. One `Event.Melee.Hit`, three results (light, heavy, crit) from Damage and a crit tag, previewed with a slider, triggered by an anim notify, then captured from PIE and scrubbed.

---

## 3. Capability priority from the demos

| Rank | Capability | Demos needing it (of 5) | Structural |
|---|---|---|---|
| 1 | Recipe Parameters (incl. Distance, live update) | 5 | Yes |
| 2 | Play Context | 4 | Yes |
| 3 | Feel Events + triggers | 4 | Yes |
| 4 | Escalation | 4 | Partly |
| 5-8 | UI steps; Number Pop / Hit Marker; Spawn Particle; Screen Color | 4 each | No |
| 9 | Sustained recipes | 3 (blocks Horror) | Yes |
| 10-13 | Audio Mix; Overlay Flash; Light Flash; Feel Map variants | 3 each | No |
| 14-16 | Flash Limiter + substitution; Decal; Nested/Random | 2 each | No |
| - | Moment capture, GIF capture | every tutorial / all marketing | Partly |
| - | Networking, beat sync | 0 | - |

---

## 4. Production constraints

- **Assets:** hard rule 2 means code work never creates `.uasset` / `.umap`. Every demo map, recipe, material, widget and sound asset is created by the user from step-by-step instructions. Plan time for this.
- **Licensing:** do not ship UE template content or Lyra inside the Fab package; demos build on the template variant in the buyer's project. Plugin samples must be plugin-owned or CC0 (Kenney, Quaternius). Trailer video may use template content (confirm Epic Content EULA).
- **Audio (decided 2026-09-16):** free sounds are fine if the license allows redistribution inside a product. Prefer **CC0** (for example Kenney audio packs, Freesound or OpenGameArt entries filtered to CC0): no attribution needed. **CC-BY** is allowed with a credits file in the plugin and the Fab description. Never use **NC** (non-commercial), **ND**, or "free to use but not to redistribute" libraries (Sonniss GDC bundle raw files, many free-tier sites). Keep a `Credits.md` listing every sound, source URL and license.
- **Accessibility:** every showcase video includes a short comfort segment; horror's comfort showcase doubles as the studio-facing accessibility piece.
- **Timing:** demos are built as each phase part lands, not at the end: the sword-hit slice after 5B, the senses pass after 5C, capture/limiter segments after 5D, full maps and videos in Phase 6.
