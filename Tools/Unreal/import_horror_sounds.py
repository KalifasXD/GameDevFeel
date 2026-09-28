"""Imports the chosen Horror kit sounds into /FeelKit/Samples/Sounds under their FeelKit names and removes the temporary
audition folder. Picks are based on
DiagFeel.SoundStats (2026-09-22). Run with the editor closed:
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
    'S_FK_Dread_Drone': find('kenney_sci-fi-sounds', 'spaceEngineLow_002.ogg'),
    'S_FK_Light_Crackle': find('kenney_interface-sounds', 'glitch_001.ogg'),
    'S_FK_Scare_Boom': find('kenney_sci-fi-sounds', 'lowFrequency_explosion_000.ogg'),
    'S_FK_Light_Buzz': find('kenney_sci-fi-sounds', 'forceField_001.ogg'),
    'S_FK_Door_Creak': find('kenney_rpg-audio', 'creak3.ogg'),
}

tasks = []
for name, path in PICKS.items():
    if unreal.EditorAssetLibrary.does_asset_exist('/FeelKit/Samples/Sounds/' + name):
        unreal.log_warning('SOUNDIMPORT {0} already exists; left as it is'.format(name))
        continue
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = '/FeelKit/Samples/Sounds'
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
