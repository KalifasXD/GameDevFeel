# Recipe JSON files and editor scripting {#ch21}
[edition: Pro]

A recipe can be written out as a JSON text file and read back in. Text is easy to review in version control, to share on a forum or in a bug report, to edit in bulk with a script, and to move between projects. FeelKit Pro also offers a few editor scripting functions for setting up recipes and project settings from Python or Editor Utility Blueprints.

## Exporting and importing in the Content Browser {#ch21_menu}
[edition: Pro]

**Exporting.** Select one or more recipes in the Content Browser, right-click, and choose **Export to JSON...**. FeelKit asks where to save each recipe and names the file after it. A notification confirms each file.

**Importing.** Right-click one recipe and choose **Import from JSON...**, then pick a file. The recipe's contents are replaced by the file's: its settings, parameters and tracks. The import is one undo step, so **Edit** > **Undo** puts the recipe back as it was. To import into a new recipe, create an empty one first ([Ref: ch05_create]).

- Asset references in the file, such as sounds, materials and curves, are looked up by their paths in this project. A reference the project does not have is left empty, the rest of the recipe still imports, and the notification lists what is missing.
- Library recipes are read-only, so they cannot be import targets; copy one into the project first ([Ref: ch09_copy]).
- A file that is not valid JSON, or not a FeelKit recipe, changes nothing. A file written by a newer version of FeelKit, with a newer recipe format, is refused with a message saying so.

## The file format {#ch21_format}
[edition: Pro]

A recipe file is JSON with a short header and the recipe's properties:

```json
{
	"format": "FeelKitRecipe",
	"schemaVersion": 2,
	"name": "FR_Denial_Locked",
	"recipe": {
		"tracks": [
			{
				"step": {
					"_ClassName": "/Script/FeelCore.FeelStep_CameraPunch",
					"frequency": 16.0,
					"damping": 10.0
				},
				"startTime": 0.0,
				"duration": 0.3,
				"channel": { "tagName": "Feel.Camera.Motion" }
			}
		]
	}
}
```

- **format** is always `FeelKitRecipe`, and **schemaVersion** the recipe format of the FeelKit that wrote it.
- **recipe** holds the recipe's properties under their names in lower camel case: `tracks`, `cooldown`, `maxConcurrent`, `parameters` and so on, as they appear in [Ref: ch16].
- Each track's **step** names its class in `_ClassName`, followed by that step's settings. Steps nested inside other steps, such as the options of a Random Choice, are written the same way.
- Asset references are written as asset paths, such as `/FeelKit/Samples/Sounds/S_FK_Denied.S_FK_Denied`.

A property left out of the file keeps its default value on import, so a hand-written file needs only what differs from the defaults. The recipes of the FeelKit library ship in this format, in the plugin's `Library` folder, and make good starting points.

## Editor scripting {#ch21_scripting}
[edition: Pro]

These functions are in the category **FeelKit** > **Editor Scripting**. They run in the editor only, from Python, from Editor Utility Blueprints, or from C++ editor code.

| Function | What it does |
|---|---|
| **Create Recipe from Json File** | Creates a recipe asset in a folder, imports a JSON file into it and saves it. Returns the recipe and a message saying what happened, including asset references that are not in the project. |
| **Import Recipe from Json File** | Imports a JSON file into an existing recipe and saves it. Returns a message saying what was imported, whether the recipe was saved, and which asset references are not in the project. |
| **Add Feel Map to Project Settings** | Adds a Feel Map to **Project Settings** > **Plugins** > **FeelKit** > **Feel Maps**, if it is not there yet ([Ref: ch06_events]). |
| **Set Accumulator in Project Settings** | Adds an accumulator to the project settings, or updates the one with that name, with its maximum, decay per second and decay delay ([Ref: ch07_accumulators]). |

In Python, the functions are on `unreal.FeelEditorScripting`. Values that Blueprint returns through output pins come back from Python as return values:

```python
import unreal

# Create a recipe from a file: returns the recipe and a message.
recipe, message = unreal.FeelEditorScripting.create_recipe_from_json_file(
    "/Game/Feel", "FR_MyLocked", "C:/Recipes/FR_Denial_Locked.json")
unreal.log(message)

# Import into an existing recipe: returns the message, or None when the import failed.
message = unreal.FeelEditorScripting.import_recipe_from_json_file(recipe, "C:/Recipes/FR_Denial_Locked.json")

# Accumulator Combo: maximum 10, decay 1 per second after 1.5 s.
unreal.FeelEditorScripting.set_accumulator_in_project_settings("Combo", 10.0, 1.0, 1.5)
```

These calls were run against FeelKit Pro on Unreal Engine 5.6. As with any editor script that saves assets, keep the project under version control or take a copy before running a script over many recipes.
