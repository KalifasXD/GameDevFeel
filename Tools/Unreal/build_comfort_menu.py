"""Builds the comfort menu that ships with FeelKit (D-085, D-092): the eight FR_Comfort_* preview recipes from
Tools/Recipes/ComfortMenu/*.json into /FeelKit/UI/Preview, then the Widget Blueprint /FeelKit/UI/WBP_FeelComfortMenu
through FeelKit's editor helper (layout, named controls, look, preview recipes). Backs up /FeelKit/UI first.
Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import datetime
import glob
import os
import shutil
import unreal

PLUGIN = r'B:\NewUE5Project\GameFeelDev\Plugins\FeelKit'
JSON_DIR = r'B:\NewUE5Project\Tools\Recipes\ComfortMenu'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'
UI_PATH = '/FeelKit/UI'
PREVIEW_PATH = '/FeelKit/UI/Preview'


def log(message):
    unreal.log_warning('COMFORTMENU ' + message)


ui_dir = os.path.join(PLUGIN, 'Content', 'UI')
if os.path.isdir(ui_dir):
    stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S') + '_comfort_menu'
    target = os.path.join(BACKUP_ROOT, stamp, 'Plugins', 'FeelKit', 'Content', 'UI')
    shutil.copytree(ui_dir, target)
    log('backed up ' + ui_dir + ' to ' + target)

for path in sorted(glob.glob(os.path.join(JSON_DIR, '*.json'))):
    name = os.path.splitext(os.path.basename(path))[0]
    recipe, message = unreal.FeelEditorScripting.create_recipe_from_json_file(PREVIEW_PATH, name, path)
    log('{0}: {1}'.format(name, message) if recipe else 'FAILED {0}: {1}'.format(name, message))

result = unreal.FeelEditorScripting.build_comfort_menu_widget(UI_PATH, 'WBP_FeelComfortMenu', PREVIEW_PATH)
# Unreal returns the out message, or (return value, out message) depending on the binding.
log(str(result))
