"""Imports one recipe JSON file into a kit folder, replacing the recipe of the same name. Backs up the existing asset
first. Arguments: the JSON file, then the package folder (default /Game/FeelKitDemos/ActionRPG). Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script="<this file> <json> [<package folder>]"
"""
import datetime
import os
import shutil
import sys
import unreal

PLUGIN_CONTENT = r'B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Content'
PROJECT_CONTENT = r'B:\NewUE5Project\GameFeelDev\Content'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'

json_path = sys.argv[1]
package_path = sys.argv[2] if len(sys.argv) > 2 else '/Game/FeelKitDemos/ActionRPG'
name = os.path.splitext(os.path.basename(json_path))[0]

root, prefix = (PROJECT_CONTENT, '/Game/') if package_path.startswith('/Game/') else (PLUGIN_CONTENT, '/FeelKit/')
existing = os.path.join(root, package_path[len(prefix):].replace('/', os.sep), name + '.uasset')
if os.path.exists(existing):
    stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
    target = os.path.join(BACKUP_ROOT, stamp + '_recipe', name + '.uasset')
    os.makedirs(os.path.dirname(target), exist_ok=True)
    shutil.copy2(existing, target)
    unreal.log_warning('RECIPEIMPORT backed up {0} to {1}'.format(name, target))

recipe, message = unreal.FeelEditorScripting.create_recipe_from_json_file(package_path, name, json_path)
unreal.log_warning('RECIPEIMPORT {0}: {1}'.format(name, message))
