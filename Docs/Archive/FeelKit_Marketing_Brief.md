# FeelKit launch brief: Fab marketing, screenshots, tutorials and documentation

Given by the user on 2026-09-23 as the brief for the launch material (Phase 6C documentation and 6D launch). Kept
below with every requirement and question, the wording condensed. Working notes on how it applies:

- Verified Fab rules are in `Docs/Research/Fab_Requirements_2026-09-23.md`; the brief's assumed limits (1080p minimum,
  PNG/JPEG, about 3 MB per image, about 25 MB total) match the current Fab documentation.
- The Lite / Pro split is not decided yet (launch decision round). Sections 3, 7, 12 and 13 of the brief depend on it.
- The developer writes all documentation (D-077). Documentation must exist before launch (Fab Technical Requirements
  4.3.8), which replaces D-051.
- "Bill" in the brief is the user.

---

# FeelKit — Fab Marketing, Screenshots, Tutorials & Documentation Master Planning

You are now responsible for planning the complete **marketing, presentation, tutorial, and documentation asset package** for **FeelKit**, an Unreal Engine plugin that will be published on **Fab**.

This is an important production task. Do not jump directly into suggesting screenshots or videos.

Your first responsibility is to **understand the entire FeelKit project**, and your second responsibility is to turn that understanding into a concrete, production-ready plan for everything we need to create before publishing.

## 1. First: full project research

Before making recommendations, inspect every relevant document, file, specification, source file, existing screenshot, design document, requirements document, README, technical document, or other project material available. Do not rely on a single requirements document if additional project material exists. Search broadly and systematically.

Understand: what FeelKit does; the problem it solves; who it is for; the core workflow; what makes it different from implementing effects manually; the Recipe system; Recipe authoring; timeline/sequencing; Blueprint integration; the trigger workflow; available effect types; camera, screen, audio, haptics and UI-related effects; Niagara integration; Enhanced Input integration; comfort settings and presets (Reduced Motion, Reduced Flashing, No Haptics); global scaling; runtime behaviour; handles / playback control; other major features; what is in Lite, what is in Pro, what is intentionally not in Lite; supported Unreal Engine versions; installation/setup workflow; dependencies and optional dependencies; limitations; technical concepts that need explaining to a buyer.

Treat the existing project documentation and implementation as the source of truth. If documentation and implementation disagree, identify the discrepancy instead of silently choosing one. Create a mental model of the product before designing the marketing material.

## 2. Verify Fab requirements

Do not assume the requirements below are correct. Research the current official Epic/Fab documentation and verify the requirements for: product screenshots/images, image dimensions, accepted formats, maximum individual image file size, maximum combined size, number of images allowed, thumbnail/cover requirements, gallery requirements, video requirements, documentation requirements, product description requirements, other relevant submission/marketing restrictions.

Assumed (not verified by the user): 1080p minimum resolution; PNG/JPEG; about 3 MB per image; about 25 MB combined. If Fab has different rules for different image types, distinguish them. End this section with a table: Requirement | Current Rule | Source | Implication for FeelKit. Do not design the final asset package around unverified limits.

## 3. Understand the two products

FeelKit Lite (reduced) and FeelKit Pro / Full (complete) must be marketed accurately and separately. Determine from the actual project documentation: what Lite contains; what Pro contains; Pro-exclusive features; shared features; workflow differences; UI differences; authoring differences; runtime differences. Do not invent feature differences.

## 4. Create the Fab screenshot strategy

Do not simply suggest "around 20 screenshots". Determine the minimum number necessary to communicate the product extremely well without repetition, the maximum sensible number before the page becomes redundant, then the recommended number. At a glance the buyer should understand: what FeelKit is; the workflow; the authoring experience; what Recipes are; the timeline; how effects are combined; how a Recipe is triggered; the Blueprint workflow; the runtime result; comfort settings; what makes FeelKit different; what Lite includes; what Pro adds; compelling advanced features; what the buyer is actually purchasing. Do not create one screenshot per item; combine concepts where one visual communicates several things.

## 5. Design each screenshot individually

For each: number; purpose; product (Lite / Pro / Both); main message (understood in 2 to 3 seconds); exact UI/content to show; what not to show; composition; required labels/text (exact wording); whether it uses real Unreal UI, real FeelKit UI, gameplay footage, AI-generated artwork, graphic design elements or a combination; source material (what must come from the project, what can be designed); production method (capture directly, capture gameplay, graphic around a screenshot, manual labels/arrows, Claude/image generation, combined captures); priority (Critical / Important / Optional); filename convention.

## 6. Do not make the screenshots look like generic AI marketing

FeelKit is an Unreal Engine developer tool. Buyers need to see the actual product: authentic FeelKit/Unreal UI and real functionality. Raw Unreal screenshots can be visually weak, so determine where a hybrid is right: real FeelKit UI + real gameplay + clean designed overlays (real timeline, real Blueprint node, real Recipe configuration, real runtime result, clean annotations, small callouts, controlled composition). Do not cover the UI with marketing graphics. Goal: "This is clearly a real, polished product, and I can immediately understand why I would use it."

## 7. Lite vs Pro visual strategy

Decide between separate Lite shots, separate Pro shots, shared shots, comparison shots, feature comparison graphics, or a dedicated Lite vs Pro visual. Consider whether one or more comparison visuals should appear in both listings (Lite: features A, B, C; Pro: everything in Lite plus D, E, F), using only confirmed features. Decide whether it is a comparison graphic, a UI screenshot, a feature matrix or a combination, and explain why.

## 8. Hero / first-impression visual

Determine what the first visual must communicate ("What is FeelKit?"): gameplay + FeelKit UI, timeline + gameplay result, Recipe + Blueprint + gameplay, a product montage, or another composition. Choose by what communicates best, not by looks. Separate recommendations for Lite and Pro if they should differ.

## 9. YouTube tutorial strategy

A structured roadmap, not "make a tutorial". Investigate: Getting Started (installation, enabling the plugin, first setup, first Recipe, first trigger); Core Workflow (creating a Recipe, the Recipe system, the timeline, combining effects, triggering, reusing, modifying); Advanced Authoring (complex sequences, multiple effect types, camera, screen, audio, haptics, Niagara, other confirmed systems); Comfort (settings, Reduced Motion, Reduced Flashing, No Haptics, global scaling); Practical Workflow (a hit reaction, an impact, damage feedback, a satisfying interaction, or other examples that fit the real feature set). Determine the actual minimum useful tutorial library that benefits from video rather than a documentation section with screenshots. For each video: title, purpose, audience, Lite/Pro/Both, duration, what it demonstrates, prerequisites, exact steps shown, assets/project setup needed, public on YouTube or not, referenced from documentation or not, required for launch or later. Prioritise videos that answer questions a buyer has before purchasing.

## 10. Documentation requirements

A complete documentation structure. Candidates: introduction; installation; requirements; supported engine versions; quick start; first Recipe; Recipe authoring; timeline; effects; Blueprint integration; playback/handles; comfort settings; Lite vs Pro; Niagara integration; Enhanced Input integration; troubleshooting; performance; best practices; FAQ; API/reference; example workflows. Determine what is actually necessary, split into: must exist before launch; strongly recommended; can be added later.

## 11. Buyer journey

Search result → product thumbnail → first screenshot → gallery → description → features → Lite/Pro distinction → documentation → tutorial → purchase. At each stage: the buyer's question; the visual/text that answers it; what uncertainty might make them leave; what must not be buried. Clarity and accurate representation, not persuasion.

## 12. Information gaps

A section "Information We Still Need": what is missing, why it matters, which asset depends on it, whether Bill must provide it, whether it can be determined from the project, priority. Do not invent missing information.

## 13. Final production checklist

A. FeelKit Lite (screenshots, graphics, comparison visuals, videos, documentation, other). B. FeelKit Pro (same). C. Shared assets (shared screenshots, videos, documentation, brand/product graphics, comparison graphics).

## 14. Exact asset inventory

Master table: ID | Asset | Lite | Pro | Shared | Type | Priority | Source | Production Method | Status. Every asset needed, no artificial inflation (hero image, gallery screenshots, feature/timeline/Blueprint/Recipe screenshots, gameplay demonstration, Lite/Pro comparison, tutorial thumbnails, Getting Started video, installation documentation, and so on).

## 15. Screenshot shooting script

For every screenshot needing a real capture: "Shot 01: open [UI], configure [thing], set [values], frame the editor like this, capture at [resolution/aspect], then add [overlay]", so Bill can work through the list in Unreal one by one.

## 16. Video recording script

For every launch-critical tutorial: opening; what the viewer sees; narration/topic; actions in Unreal; what to emphasise; what not to over-explain; ending; duration. Practical and concise, not lectures.

## 17. Marketing, technically honest

No "revolutionary", "the best", "ultimate", "game-changing" unless substantiated. Let the visuals show the value: create a feel effect once → save it as a Recipe → reuse it wherever needed. Show the workflow; let the product demonstrate itself.

## 18. Real product UI vs marketing graphics

For every visual mark: REAL (must come from the actual FeelKit project), DESIGNED (can be a marketing graphic), OPTIONAL (improves presentation, not necessary). AI tools may later help with some marketing graphics; product functionality must stay authentic.

## 19. Optimise for Fab

Around the verified rules: resolution, aspect ratio, file size, compression, readability, text legibility, number of images, gallery order, thumbnail visibility, mobile/desktop readability, whether shots still communicate when shown small. Do not sacrifice clarity to fit an arbitrary count.

## 20. Final output structure

1 Executive Summary. 2 Project Understanding. 3 Verified Fab Requirements. 4 Lite vs Pro. 5 Recommended Fab Gallery (exact number and why). 6 Screenshot-by-Screenshot Plan. 7 Lite Screenshot Plan. 8 Pro Screenshot Plan. 9 Shared/Comparison Visuals. 10 YouTube Tutorial Plan. 11 Documentation Plan. 12 Screenshot Shooting Script. 13 Video Recording Scripts. 14 Master Asset Inventory. 15 Launch-Critical vs Later. 16 Information Gaps. 17 Recommended Production Order.

**Most important:** understand FeelKit deeply first, then determine what a prospective Unreal developer needs to see to understand the product confidently. The result is a production blueprint for launching FeelKit on Fab. Every screenshot, video and documentation item needs a reason to exist. Avoid redundancy. Do not invent features. Do not assume Lite and Pro are identical. Do not assume Fab requirements; verify them. Use the actual project as the source of truth. Do not just say what could be made: say exactly what should be made, why, what appears in it, how it is produced, and in what order.
