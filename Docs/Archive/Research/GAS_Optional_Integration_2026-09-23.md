# Offering GAS support only to buyers who want it (D-027, D-066)

Research for the decision on FeelKit's optional Gameplay Ability System module, 2026-09-23. Each finding is marked
VERIFIED (read in the source named) or INFERRED.

## The problem

FeelKit contains the module FeelGAS (gameplay cue notifies that play recipes). Because it lives inside the FeelKit
plugin, FeelKit.uplugin lists GameplayAbilities as a required plugin, so enabling FeelKit enables GAS in every buyer's
project. The user's direction: Enhanced Input is fine (it is on in every project anyway); GAS only when the buyer wants it.

## Findings

1. **Fab takes one plugin per product.** "The Project File Link must host the download of a zip archive that includes
   only one Unreal Engine project or plugin." VERIFIED: [Fab asset file format and structure requirements](https://dev.epicgames.com/documentation/fab/asset-file-format-and-structure-requirements-in-fab?lang=en-US).
2. **What enabling GAS pulls in.** GameplayAbilities.uplugin has EnabledByDefault false and needs GameplayTagsEditor,
   Niagara and DataRegistry. EnhancedInput.uplugin has EnabledByDefault true. Niagara is on by default. VERIFIED: the
   engine's .uplugin files in `B:/UE_5.6/Engine/Plugins`.
3. **Unreal has no build-time "only if that plugin is enabled" dependency.** Epic's answer on the forum: modifying build
   settings in response to it "would break the installed engine build"; the suggested route is a separate module that
   is conditionally enabled and loaded at runtime. VERIFIED: [Creating Plugin with Optional Dependencies](https://forums.unrealengine.com/t/creating-plugin-with-optional-dependencies/131525) (Ben Marsh, post 2).
4. **`"Optional": true` does not solve it.** In the same thread, the flag is described as covering a plugin missing at
   run time, not build-time handling, and the missing-dependency warnings remain (posts 7, 9, 10). VERIFIED as what the
   thread says; not tested here. It would also break FeelKit's zero-warnings rule.
5. **A plugin inside another plugin's folder is ignored by Unreal.** Plugin discovery stops descending once it finds a
   .uplugin ("once we find one .uplugin file ... there shouldn't be anymore in the same folder hierarchy"). VERIFIED:
   `Engine/Source/Runtime/Projects/Private/PluginManager.cpp`, `FindPluginsInDirectory`. So an add-on plugin can ship
   inside FeelKit's folder and stays inactive until someone copies it into a project's Plugins folder.
6. **How GAS-related products ship on Fab.** GAS helpers are sold as their own products (GAS Companion, GAS Helper,
   Ninja G.A.S., Generic Gameplay Abilities). Ninja Bear Studio sells Ninja G.A.S. as a standalone plugin that its other
   products (Ninja Combat, Ninja Inventory) integrate with. VERIFIED: listings exist ([GAS Companion](https://www.fab.com/listings/72e2bc50-658e-43cd-bd40-535f46d2f113),
   [GAS Helper](https://www.fab.com/listings/2f5abf03-71f5-444f-82cf-ffbfe1ebbb86), [Ninja G.A.S.](https://www.fab.com/listings/074018e7-fba6-4e34-8c9b-64208ea1c65b)).
   How the Ninja products detect each other was not verified (their documentation site did not resolve, Fab blocks
   automated reads).
7. **GAS projects are C++ projects in practice.** Attribute sets are C++ classes, so a buyer using GAS already compiles
   code. INFERRED from how GAS is set up (see the engine's GAS documentation and the Lyra sample).

## Options

| Option | What the buyer does | Cost |
|---|---|---|
| A. Keep FeelGAS inside FeelKit (today) | Nothing | GAS and Data Registry are switched on in every project that enables FeelKit |
| B. GAS add-on shipped inside FeelKit, inactive (e.g. `FeelKit/Extras/FeelKitGAS/FeelKitGAS.uplugin`) | Copies one folder into the project's Plugins folder; it compiles with the project | One product. Needs checking in 6D: Fab's package check and BuildPlugin must accept the extra folder (BuildPlugin builds only the modules listed in FeelKit.uplugin) |
| C. Separate free Fab listing "FeelKit GAS" | Installs it and ticks it in the Plugins window | A second product to build and update for every engine version, working with both Lite and Pro |
| D. Optional flag plus runtime loading inside FeelKit | Nothing | Build warnings (breaks the zero-warnings rule); GAS still needed at build time in some setups. Rejected |

## Recommendation

**B for launch**, because it keeps one product, nothing changes in projects that do not use GAS, and the buyers who
want it already work in C++. Check it in Phase 6D with a real package: BuildPlugin output, a fresh project with FeelKit
(GameplayAbilities must stay off), and a GAS project with the add-on copied in (the existing `FeelKit.GAS.GameplayCueNotify`
test must pass there). If Fab rejects the extra folder, fall back to C.

Decision stays with the user (D-066: leave as it is until launch).
