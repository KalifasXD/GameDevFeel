Hand-written text for chapter 17. gen_reference.py copies each block below into src/17_nodes.generated.md.
Keys: "section.<anchor>" for section text, "fn:<Class>.<Function>" for a node, "type:<Class>" for a class or structure.

@@ section.ref_nodes_play
These nodes are in the **Feel** category of the Blueprint context menu.

[shot: S17-01 | The Blueprint context menu searched for Feel]

@@ fn:UFeelBlueprintLibrary.PlayFeel
The returned handle is invalid when the recipe could not start: no recipe was given, a **Cooldown** or **Max Concurrent** limit of the recipe is reached on this target, `feel.Enabled` is 0, the game runs as a dedicated server, or the target is an actor or component that no longer exists (this last case also logs a warning).

@@ fn:UFeelBlueprintLibrary.StopFeel
With **Blend Out** on, the recipe fades out over **Blend Out Time** from the project settings ([see: set_playback]) and still counts as playing until the fade ends.

@@ section.ref_nodes_targets
A Feel Target left at its default (type None) sends camera and screen effects to the first local player. A target that belongs to a player on another machine gets no camera, screen or controller effects on this machine; that player's own machine shows them.

@@ fn:UFeelBlueprintLibrary.MakeFeelTargetFromActor
Use this node for characters: scale, wobble and other actor motion then go to the character's mesh and never move its capsule.

@@ section.ref_nodes_context
In Blueprint, build the structure with **Make FeelPlayContext** and connect it to the **Context** pin of Play Feel with Context, Send Feel Event, Play Feel and Wait or the networked nodes. Every field is optional.

@@ section.ref_nodes_parameters
**Set Feel Parameter** changes a value of one play. The accumulator nodes change named values that any play can read through a recipe parameter's **Accumulator** field; accumulators are defined in the project settings ([see: set_playback]). The name pins of the accumulator nodes are dropdowns that list the accumulators of the project.

@@ section.ref_nodes_comfort
Comfort settings belong to a local player. **Get Feel Comfort** returns the player's Feel Comfort Subsystem, and the other comfort nodes are called on it. The settings are loaded when the local player is created, before any recipe plays, and saved automatically after each change while **Auto Save Comfort** is on ([see: set_comfort_storage]).

@@ type:UFeelComfortPreset
Create one in the Content Browser as a Data Asset of the class Feel Comfort Preset, then apply it with **Apply Custom Comfort Preset**.

@@ fn:IFeelComfortStorage.LoadComfortScales
To keep comfort settings in your own save system, make a class that implements the Feel Comfort Storage interface and implement this function and Save Comfort Scales. Then pick the class in **Comfort Storage Class** in the project settings, or pass an object of it to **Set Comfort Storage**.

@@ section.ref_nodes_switch
**Set Feel Enabled**, **Toggle Feel** and **Is Feel Enabled** work with or without a Feel Switch actor in the level. They change and read the same value as the console variable `feel.Enabled` ([see: con_feel_enabled]).

@@ type:AFeelSwitch
Place one Feel Switch in a level. A second one in the same level logs a warning and does nothing. The switch applies to the whole game on this machine, so every Play In Editor window switches together, and the choice is not saved.

@@ section.ref_nodes_notifies
Add these to an animation's **Notifies** track (right-click the track, **Add Notify** or **Add Notify State**). They play when the animation reaches them in the game, and also in the animation editor's preview, so actor effects can be tuned while scrubbing the animation.

@@ type:UFeelTriggerComponent
The component ticks only when one of its entries uses Jumped, Air Jumped or Launched, because those have no engine event to bind to. The other events are bound when play begins.

@@ type:UFeelInputComponent
The input actions must be in an input mapping context that the player has added.

@@ type:UFeelReplicationComponent
The component is replicated by default.

@@ section.ref_nodes_gas
These classes are in the GAS add-on, the separate plugin FeelKit GAS with the module FeelGAS. It is not active until you copy it into your project. To use one in Blueprint, create a Blueprint class with the class below as its parent and set its **Gameplay Cue Tag** as for any other gameplay cue notify.

@@ section.ref_nodes_scripting
These functions are in the category **FeelKit** > **Editor Scripting**, for Editor Utility Blueprints and Python in the editor. They are not available in packaged games. In Python they are called as `unreal.FeelEditorScripting.import_recipe_from_json_file(...)` and so on.

@@ section.ref_cpp_body
FeelKit's runtime module is FeelCore. Add it to the dependencies of your game module in its `.Build.cs` file; add FeelEnhancedInput to use the Feel Input component from C++, and FeelGAS for the GAS add-on.

```
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "FeelCore" });
```

Plays go through the world's Feel Subsystem:

```
#include "FeelSubsystem.h"

if (UFeelSubsystem* Feel = UFeelSubsystem::Get(this))
{
    HitHandle = Feel->PlayFeel(HitRecipe, FFeelTarget::FromActor(this));
}
```

Here `HitRecipe` is a `UFeelRecipe` property of the class and `HitHandle` an `FFeelHandle` member. `FFeelTarget` has the static functions `FromActor`, `FromComponent`, `AtLocation`, `FromLocalPlayerCamera` and `FromWidget`, which match the Make Feel Target nodes. The nodes in the **Feel** categories are static functions of `UFeelBlueprintLibrary` and can be called from C++ as well. The table lists the functions of `UFeelSubsystem` for C++ code.
