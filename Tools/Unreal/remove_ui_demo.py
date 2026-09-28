"""Removes the dropped UI menu demo (D-064) from GameFeelDev and the plugin: the menu level and widget, the nine FR_UI
recipes and the two sounds only the menu used. Copies every file to Backups first. Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import datetime
import os
import shutil
import unreal

PROJECT = r'B:\NewUE5Project\GameFeelDev'
PLUGIN = os.path.join(PROJECT, 'Plugins', 'FeelKit')
BACKUP_ROOT = r'B:\NewUE5Project\Backups'


def log(message):
    unreal.log_warning('UIREMOVE ' + message)


stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
backup = os.path.join(BACKUP_ROOT, stamp + '_ui_demo_removed')
sources = [
    os.path.join(PROJECT, 'Content', 'Demo_UI'),
    os.path.join(PROJECT, 'Content', '__ExternalActors__', 'Demo_UI'),
    os.path.join(PROJECT, 'Content', '__ExternalObjects__', 'Demo_UI'),
    os.path.join(PLUGIN, 'Content', 'Demos', 'UI'),
    os.path.join(PLUGIN, 'Content', 'Samples', 'Sounds', 'S_FK_UI_Reward.uasset'),
    os.path.join(PLUGIN, 'Content', 'Samples', 'Sounds', 'S_FK_UI_Thump.uasset'),
]
for source in sources:
    if not os.path.exists(source):
        continue
    target = os.path.join(backup, os.path.relpath(source, PROJECT))
    if os.path.isdir(source):
        shutil.copytree(source, target)
    else:
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copy2(source, target)
log('backed up to ' + backup)

for directory in ('/Game/Demo_UI', '/FeelKit/Demos/UI'):
    if unreal.EditorAssetLibrary.does_directory_exist(directory):
        log('{0} deleted: {1}'.format(directory, unreal.EditorAssetLibrary.delete_directory(directory)))
for asset in ('/FeelKit/Samples/Sounds/S_FK_UI_Reward', '/FeelKit/Samples/Sounds/S_FK_UI_Thump'):
    if unreal.EditorAssetLibrary.does_asset_exist(asset):
        log('{0} deleted: {1}'.format(asset, unreal.EditorAssetLibrary.delete_asset(asset)))
