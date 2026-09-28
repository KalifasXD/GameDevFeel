"""Imports the chosen guard, block and parry sounds of the Action/RPG kit into /Game/FeelKitDemos/ActionRPG/Sounds under their FeelKit
names and removes the temporary audition folder. Picks are based on DiagFeel.SoundStats (2026-09-23). Run with the editor
closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import os
import unreal

DOWNLOADS = r'C:\Users\Billo\Downloads'


def find(pack, name):
    for root, _, files in os.walk(os.path.join(DOWNLOADS, pack)):
        if name in files:
            return os.path.join(root, name)
    raise RuntimeError('missing ' + name)


PICKS = {
    'S_FK_Guard_Clang_Light': find('kenney_impact-sounds', 'impactMetal_light_001.ogg'),
    'S_FK_Guard_Clang_Heavy': find('kenney_impact-sounds', 'impactMetal_heavy_003.ogg'),
    'S_FK_Guard_Field': find('kenney_sci-fi-sounds', 'forceField_002.ogg'),
    'S_FK_Parry_Ring': find('kenney_impact-sounds', 'impactBell_heavy_000.ogg'),
    'S_FK_Guard_Break': find('kenney_impact-sounds', 'impactGlass_medium_001.ogg'),
}

tasks = []
for name, path in PICKS.items():
    if unreal.EditorAssetLibrary.does_asset_exist('/Game/FeelKitDemos/ActionRPG/Sounds/' + name):
        unreal.log_warning('SOUNDIMPORT {0} already exists; left as it is'.format(name))
        continue
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = '/Game/FeelKitDemos/ActionRPG/Sounds'
    task.destination_name = name
    task.automated = True
    task.replace_existing = False
    task.save = True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    unreal.log_warning('SOUNDIMPORT {0} -> {1}'.format(os.path.basename(task.filename), list(task.imported_object_paths)))

if unreal.EditorAssetLibrary.does_directory_exist('/Game/_FeelAudition'):
    unreal.EditorAssetLibrary.delete_directory('/Game/_FeelAudition')
    unreal.log_warning('SOUNDIMPORT removed /Game/_FeelAudition')
