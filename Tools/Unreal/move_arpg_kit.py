"""Moves the Action/RPG demo kit out of the FeelKit plugin into the GameFeelDev project (D-089, D-091): the eight
FR_ARPG recipes, the FM_ARPG Feel Map and the guard shield material go to /Game/FeelKitDemos/ActionRPG, and the ten
sounds only this kit uses go to /Game/FeelKitDemos/ActionRPG/Sounds. Uses Unreal's own rename, which updates every
Blueprint, animation and level that refers to them, then fixes up and removes the redirectors left behind.
Backs up the plugin's kit folder, the sounds, the project's Content and DefaultGame.ini first.
Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import datetime
import os
import shutil
import unreal

PROJECT = r'B:\NewUE5Project\GameFeelDev'
PLUGIN = os.path.join(PROJECT, 'Plugins', 'FeelKit')
BACKUP_ROOT = r'B:\NewUE5Project\Backups'
KIT_FROM = '/FeelKit/Demos/ActionRPG'
SOUNDS_FROM = '/FeelKit/Samples/Sounds'
KIT_TO = '/Game/FeelKitDemos/ActionRPG'
SOUNDS_TO = '/Game/FeelKitDemos/ActionRPG/Sounds'
KIT_ASSETS = ['FR_ARPG_ChargePulse', 'FR_ARPG_EnemyDeath', 'FR_ARPG_EnemyHurt', 'FR_ARPG_GuardBlock', 'FR_ARPG_GuardBreak',
              'FR_ARPG_HitLanded', 'FR_ARPG_Parry', 'FR_ARPG_Swing', 'FM_ARPG', 'M_FK_GuardShield']
SOUNDS = ['S_FK_Body_Hit_Heavy', 'S_FK_Guard_Break', 'S_FK_Guard_Clang_Heavy', 'S_FK_Guard_Clang_Light', 'S_FK_Guard_Field',
          'S_FK_Parry_Ring', 'S_FK_Sword_Clash', 'S_FK_Sword_Hit', 'S_FK_Sword_Swing_Heavy', 'S_FK_Sword_Swing_Light']


def log(message):
    unreal.log_warning('ARPGMOVE ' + message)


# 1. Backups.
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S') + '_arpg_move'
backup = os.path.join(BACKUP_ROOT, stamp)
shutil.copytree(os.path.join(PLUGIN, 'Content', 'Demos', 'ActionRPG'), os.path.join(backup, 'Plugins', 'FeelKit', 'Content', 'Demos', 'ActionRPG'))
os.makedirs(os.path.join(backup, 'Plugins', 'FeelKit', 'Content', 'Samples', 'Sounds'), exist_ok=True)
for name in SOUNDS:
    shutil.copy2(os.path.join(PLUGIN, 'Content', 'Samples', 'Sounds', name + '.uasset'), os.path.join(backup, 'Plugins', 'FeelKit', 'Content', 'Samples', 'Sounds'))
shutil.copytree(os.path.join(PROJECT, 'Content'), os.path.join(backup, 'Content'))
os.makedirs(os.path.join(backup, 'Config'), exist_ok=True)
shutil.copy2(os.path.join(PROJECT, 'Config', 'DefaultGame.ini'), os.path.join(backup, 'Config'))
log('backed up to ' + backup)

# 2. Move with Unreal's rename, which resaves every referencer.
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
tools = unreal.AssetToolsHelpers.get_asset_tools()
moves = []
for name in KIT_ASSETS:
    asset = unreal.load_asset('{0}/{1}'.format(KIT_FROM, name))
    if asset:
        moves.append(unreal.AssetRenameData(asset, KIT_TO, name))
    else:
        log('MISSING {0}/{1}'.format(KIT_FROM, name))
for name in SOUNDS:
    asset = unreal.load_asset('{0}/{1}'.format(SOUNDS_FROM, name))
    if asset:
        moves.append(unreal.AssetRenameData(asset, SOUNDS_TO, name))
    else:
        log('MISSING {0}/{1}'.format(SOUNDS_FROM, name))
log('renaming {0} assets: {1}'.format(len(moves), tools.rename_assets(moves)))

# 3. Point every referencer at the new paths directly and remove the redirectors.
registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
redirectors = []
for folder in (KIT_FROM, SOUNDS_FROM):
    for data in registry.get_assets_by_path(folder, recursive=True):
        if str(data.asset_class_path.asset_name) == 'ObjectRedirector':
            redirector = data.get_asset()
            if redirector:
                redirectors.append(redirector)
log('redirectors left behind: {0}'.format(len(redirectors)))
if redirectors:
    tools.fixup_referencers(redirectors)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

# 4. Report what is where now.
registry.search_all_assets(True)
for folder in (KIT_FROM, KIT_TO, SOUNDS_TO):
    names = sorted(str(d.asset_name) for d in registry.get_assets_by_path(folder, recursive=False))
    log('{0}: {1}'.format(folder, ', '.join(names) if names else '(empty)'))
left = [str(d.asset_name) for d in registry.get_assets_by_path(SOUNDS_FROM, recursive=False) if str(d.asset_name) in SOUNDS]
log('moved sounds still in the plugin: {0}'.format(', '.join(left) if left else 'none'))
