"""Imports the chosen Action/RPG kit sounds into /Game/FeelKitDemos/ActionRPG/Sounds under their FeelKit names and removes the
temporary audition folder. Picks are based on DiagFeel.SoundStats (2026-09-18). Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import os
import unreal

DOWNLOADS = r'C:\Users\Billo\Downloads'
KENNEY = os.path.join(DOWNLOADS, 'kenney_impact-sounds')


def kenney(name):
    for root, _, files in os.walk(KENNEY):
        if name in files:
            return os.path.join(root, name)
    raise RuntimeError('missing ' + name)


PICKS = {
    'S_FK_Sword_Swing_Light': os.path.join(DOWNLOADS, 'oga_swishes', 'swishes', 'swish-11.wav'),
    'S_FK_Sword_Swing_Heavy': os.path.join(DOWNLOADS, 'oga_swishes', 'swishes', 'swish-3.wav'),
    'S_FK_Sword_Hit': os.path.join(DOWNLOADS, 'oga_sword_-_starninjas_1', 'sword - StarNinjas', 'sword.6.ogg'),
    'S_FK_Sword_Clash': os.path.join(DOWNLOADS, 'oga_sword_clash_-_starninjas_0', 'sword_clash.2.ogg'),
    'S_FK_Body_Hit': kenney('impactPunch_medium_001.ogg'),
    'S_FK_Body_Hit_Heavy': kenney('impactPunch_heavy_002.ogg'),
    'S_FK_Body_Fall': kenney('impactSoft_heavy_003.ogg'),
}

tasks = []
for name, path in PICKS.items():
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = '/Game/FeelKitDemos/ActionRPG/Sounds'
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    unreal.log('SOUNDIMPORT {0} -> {1}'.format(os.path.basename(task.filename), list(task.imported_object_paths)))

if unreal.EditorAssetLibrary.does_directory_exist('/Game/_FeelAudition'):
    unreal.EditorAssetLibrary.delete_directory('/Game/_FeelAudition')
    unreal.log('SOUNDIMPORT removed /Game/_FeelAudition')
