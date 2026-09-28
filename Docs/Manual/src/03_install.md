# Installing FeelKit {#ch03}

FeelKit is an engine plugin: it is installed once per engine version and then enabled in each project that uses it. Two tasks need Visual Studio: adding the GAS add-on and packaging a Blueprint-only project.

[video: V1]

## Requirements {#ch03_requirements}

| Item | Requirement |
|---|---|
| Unreal Engine | 5.6, 5.7 or 5.8. Fab provides a separate download for each version. |
| Platforms | Windows 64-bit is built and tested. The runtime modules also list Mac, Linux, Android and iOS, and the editor module Mac and Linux, but these platforms have not been tested. |
| Project type | Blueprint and C++ projects. |
| Visual Studio | Not needed to install FeelKit or to work with it in the editor, since Fab supplies ready-built binaries. Needed to package a Blueprint-only project ([see: ch03_packaging]), to use the GAS add-on ([see: ch03_gas]) and, as always, to build a C++ project. |
| Other plugins | Pro: Niagara and Enhanced Input, both part of the engine; FeelKit enables them itself ([see: ch03_dependencies]). Lite needs no other plugin. |

## Installing from Fab and enabling the plugin {#ch03_install}

1. In the Epic Games Launcher, open **Unreal Engine** > **Library** and scroll to **Fab Library**.
2. Search for FeelKit and click **Install to Engine**. Choose the engine version and click **Install**. The launcher places the plugin in a folder under the engine's `Engine/Plugins` folder, usually `Engine/Plugins/Marketplace`, where every project that uses this engine version can find it.
3. Open the project and choose **Edit** > **Plugins**. Type **FeelKit** in the search box. The entry is called **FeelKit** for Pro and **FeelKit Lite** for Lite.
4. Tick **Enabled** on the entry and click **Restart Now**.

Install one edition per engine version. Both editions are the same plugin, FeelKit, as far as Unreal is concerned, so an engine with both installed finds two plugins of one name ([see: ch03_upgrade]).

[shot: S03-02 | Edit > Plugins with FeelKit found by the search box. FeelKit GAS appears only in a project that has the add-on]

After the restart, the Content Browser can create recipes (right-click > **FeelKit** > **Feel Recipe**) and the Blueprint action menu lists the **Feel** nodes. In Pro, the **Tools** menu also holds the FeelKit windows: the Recipe Browser, the Debugger and the Comfort Audit. Enabling adds FeelKit to the `Plugins` list of the project's `.uproject` file; nothing else in the project changes.

An update from Fab replaces the installed plugin in the engine folder. Anything edited inside that folder, such as FeelKit's own comfort menu, is replaced as well; [Ref: ch08] describes how to restyle the comfort menu safely.

## What FeelKit switches on for you {#ch03_dependencies}
[edition: Pro]

FeelKit Pro depends on two plugins that ship with the engine, and enabling FeelKit enables them too. FeelKit Lite has neither feature and enables no other plugin.

| Plugin | Used for |
|---|---|
| **Niagara** | The Spawn Particle step. |
| **Enhanced Input** | The Feel Input component, which plays recipes from input actions. |

A project that already uses both notices no change. A project that has disabled either one will find it enabled again, because FeelKit cannot load without it.

## Showing FeelKit content in the Content Browser {#ch03_content}

The Content Browser hides plugin content until it is asked to show it:

1. In the Content Browser, click **Settings**, the gear icon at the right end of the Content Browser's toolbar, and tick **Show Plugin Content**.
2. In the folder tree, open **All** > **Plugins** > **FeelKit Content**.

**FeelKit Content** (**FeelKit Lite Content** in Lite) holds these folders:

| Folder | Contents |
|---|---|
| **Library** | The ready-made recipes, one folder per feeling ([Ref: ch09]): 38 in Pro, 11 in Lite. Read-only; a project uses its own copies. |
| **Demos** | Pro. The recipes of the Shooter, Horror and Platformer demo levels ([Ref: ch13]). |
| **Samples** | The sounds and materials the library and demo recipes use. Pro: 43 sounds, one sound attenuation asset and three materials. Lite: the 7 sounds its library recipes use. |
| **UI** | The comfort menu `WBP_FeelComfortMenu` and the short recipes its sliders play as previews ([Ref: ch08]). |

`Credits.md` in the plugin folder lists every sample sound with its source and license.

[shot: S03-03 | FeelKit Content in the Content Browser (Pro): Demos, Library, Samples and UI]

## Adding the GAS add-on {#ch03_gas}
[edition: Pro]

Support for the Gameplay Ability System is a separate plugin, **FeelKit GAS**, inside FeelKit's `Extras` folder. It stays out of projects that do not use GAS, so they never load the Gameplay Abilities plugin because of FeelKit. It is added per project, and it compiles with the project, so Visual Studio 2022 with the C++ workload is required.

1. Close the Unreal Editor.
2. In File Explorer, open the engine's `Engine/Plugins` folder and search it for `FeelKit.uplugin`. The folder that contains that file is FeelKit's folder, usually inside `Engine/Plugins/Marketplace`. Open its `Extras` folder.
3. Copy the `FeelKitGAS` folder into the project's `Plugins` folder, creating `Plugins` if it does not exist. The result is `<Project>/Plugins/FeelKitGAS/FeelKitGAS.uplugin`.
4. Open the project. Unreal reports that the FeelKit GAS module is missing or was built with a different engine version and offers to rebuild it; click **Yes**. In a C++ project, building the project from Visual Studio does the same.

| Location | Path |
|---|---|
| Copy from | `<FeelKit folder>/Extras/FeelKitGAS`, usually `<Engine>/Engine/Plugins/Marketplace/<FeelKit folder>/Extras/FeelKitGAS` |
| Copy to | `<Project>/Plugins/FeelKitGAS` |

FeelKit GAS is enabled as soon as it is in the project, and it enables Gameplay Abilities. The Gameplay Cue notifies it adds are described in [Ref: ch06_gas]. To remove it, delete the `Plugins/FeelKitGAS` folder while the editor is closed; FeelKit itself is unaffected.

## Blueprint-only projects and packaging {#ch03_packaging}

A Blueprint-only project can use FeelKit in the editor without any extra software. Packaging is different: when a project enables a code plugin that the engine does not enable by default, Unreal builds a game executable for that project instead of using the ready-made one, and treats the project as a code project for packaging. Every code plugin from Fab has this effect, FeelKit included.

To package a Blueprint-only project that uses FeelKit:

1. Install Visual Studio 2022 with the **Game development with C++** workload, as described in Epic's guide to setting up Visual Studio for Unreal Engine.
2. Package the project as usual (**Platforms** > **Windows** > **Package Project**). The project stays a Blueprint project; no C++ files are added to it.

The packaged game contains FeelKit's runtime modules only. The editor module, the Recipe Browser and the other editor tools are not included.

## Moving a project from Lite to Pro {#ch03_upgrade}

FeelKit Lite and FeelKit Pro use the same plugin name, the same modules and the same class names, and Lite's recipes and assets keep the same paths in Pro. A project made with Lite therefore opens in Pro unchanged: every recipe keeps its tracks, steps and values, and every Blueprint node stays connected.

1. Close the Unreal Editor.
2. In the Epic Games Launcher, open **Unreal Engine** > **Library**, find FeelKit Lite under the engine version's installed plugins and remove it.
3. Install FeelKit Pro to the same engine version ([see: ch03_install]).
4. Open the project. It uses Pro from now on; the Pro features appear in the editor, and the Pro fields of existing recipes show their default values.

The other direction is limited. Lite keeps the Pro fields of a recipe, such as parameters and random ranges, in the asset without showing them, so those values survive a round trip. A track whose effect exists only in Pro, however, has no step in Lite; saving such a recipe in Lite loses that step for good.
