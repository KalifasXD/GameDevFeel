"""Imports the chosen Platformer kit sounds into /FeelKit/Samples/Sounds under their FeelKit names and removes the
temporary audition folder. Picks are based on DiagFeel.SoundStats (2026-09-22). Run with the editor closed:
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
    'S_FK_Jump_Whoosh': os.path.join(DOWNLOADS, 'oga_swishes', 'swishes', 'swish-12.wav'),
    'S_FK_Jump_Scuff': find('kenney_impact-sounds', 'footstep_concrete_000.ogg'),
    'S_FK_Air_Lift': find('kenney_interface-sounds', 'maximize_009.ogg'),
    'S_FK_Wall_Thump': find('kenney_impact-sounds', 'impactSoft_medium_002.ogg'),
    'S_FK_Dash_Whoosh': os.path.join(DOWNLOADS, 'oga_swishes', 'swishes', 'swish-7.wav'),
    'S_FK_Land_Step': find('kenney_impact-sounds', 'footstep_concrete_003.ogg'),
    'S_FK_Land_Soft': find('kenney_impact-sounds', 'impactSoft_medium_001.ogg'),
    'S_FK_Land_Heavy': find('kenney_impact-sounds', 'impactSoft_heavy_001.ogg'),
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
