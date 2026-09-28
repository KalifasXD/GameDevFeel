Hand-written text for chapter 18. gen_reference.py copies each block below into src/18_settings.generated.md.

@@ section.set_files
Project settings are under **Edit** > **Project Settings** > **Plugins** > **FeelKit**. They are saved in the project's `Config/DefaultGame.ini`, in the section `[/Script/FeelCore.FeelSettings]`, so they are shared by everyone who works on the project and are packaged with the game.

Editor preferences are under **Edit** > **Editor Preferences** > **Plugins** > **FeelKit**. They belong to one user of one project and are saved in `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` on Windows.

[shot: S18-01 | Project Settings, Plugins, FeelKit: the Camera and Playback categories, with one accumulator (Combo) added as an example]

[shot: S18-02 | Project Settings, Plugins, FeelKit: the Comfort category with the default comfort scales]

[shot: S18-03 | Project Settings, Plugins, FeelKit: the comfort presets, with the Reduced Motion preset expanded]

[shot: S18-04 | Editor Preferences, Plugins, FeelKit]

@@ section.set_camera
These settings decide how the camera output of separate plays combines for one player. Tracks inside one play always add up. With **Strongest Wins**, the strongest location offset, rotation offset and field of view change each win on their own. With **Additive Capped**, the plays add up and the sum is limited by the three caps.

@@ section.set_comfort
Players start with **Default Comfort Scales** until they change their settings or saved settings are loaded. **Channel Comfort Groups** decides which group scales each channel; a mapping covers the channel and every tag below it, and when several mappings match, the most specific tag wins.
