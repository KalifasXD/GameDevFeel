"""Builds the Action/RPG demo kit from its JSON files: backs up the existing kit assets, creates or refreshes the five
recipes, checks the Feel Map entries and registers the map in Project Settings. Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import datetime
import glob
import os
import shutil
import unreal

PROJECT = r'B:\NewUE5Project\GameFeelDev'
JSON_DIR = os.path.join(PROJECT, 'DemoKits', 'ActionRPG')
KIT_DIR = os.path.join(PROJECT, 'Content', 'FeelKitDemos', 'ActionRPG')
PACKAGE_PATH = '/Game/FeelKitDemos/ActionRPG'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'

# 1. Back up every kit asset before touching it (no version control in this project).
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
backup_dir = os.path.join(BACKUP_ROOT, stamp, 'Content', 'FeelKitDemos', 'ActionRPG')
existing = glob.glob(os.path.join(KIT_DIR, '*.uasset'))
if existing:
    os.makedirs(backup_dir, exist_ok=True)
    for path in existing:
        shutil.copy2(path, backup_dir)
config = os.path.join(PROJECT, 'Config', 'DefaultGame.ini')
config_backup = os.path.join(BACKUP_ROOT, stamp, 'Config')
os.makedirs(config_backup, exist_ok=True)
shutil.copy2(config, config_backup)
unreal.log('KITBUILD backed up {0} kit files and DefaultGame.ini to {1}'.format(len(existing), os.path.join(BACKUP_ROOT, stamp)))

# 2. Recipes from JSON.
for path in sorted(glob.glob(os.path.join(JSON_DIR, '*.json'))):
    name = os.path.splitext(os.path.basename(path))[0]
    recipe, message = unreal.FeelEditorScripting.create_recipe_from_json_file(PACKAGE_PATH, name, path)
    unreal.log('KITBUILD {0}: {1}'.format(name, message) if recipe else 'KITBUILD FAILED {0}: {1}'.format(name, message))

# 3. The Feel Map: report its entries and make sure Project Settings uses it.
feel_map = unreal.load_asset(PACKAGE_PATH + '/FM_ARPG')
if feel_map:
    for entry in feel_map.get_editor_property('entries'):
        recipe = entry.get_editor_property('recipe')
        unreal.log('KITBUILD map entry {0} -> {1}'.format(entry.get_editor_property('event'), recipe.get_name() if recipe else 'NONE'))
    registered = unreal.FeelEditorScripting.add_feel_map_to_project_settings(feel_map)
    unreal.log('KITBUILD FM_ARPG in Project Settings: {0}'.format(registered))
else:
    unreal.log_warning('KITBUILD FM_ARPG not found')
