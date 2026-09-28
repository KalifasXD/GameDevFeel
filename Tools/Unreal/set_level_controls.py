"""Fills the controls panel of the Feel Switch in each demo level (keys read from each level's player controller with
dump_level_controls.py, 2026-09-23). Backs up each level first. Run once in each project, with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
UnrealEditor-Cmd.exe FeelDemoFP.uproject -run=pythonscript -script=<this file>
Levels that do not exist in the project are skipped.
"""
import datetime
import os
import shutil
import unreal

LEVELS = {
    '/Game/Variant_Combat/Lvl_Combat': [
        ('Move', 'WASD / Left stick'),
        ('Look', 'Mouse / Right stick'),
        ('Attack', 'Left mouse / RB'),
        ('Charged attack', 'Hold right mouse / RT'),
        ('Block, parry', 'Hold Left Shift / LT'),
    ],
    '/Game/Variant_Platforming/Lvl_Platforming': [
        ('Move', 'WASD / Left stick'),
        ('Look', 'Mouse / Right stick'),
        ('Jump', 'Space / A'),
        ('Double jump', 'Jump again in the air'),
        ('Wall jump', 'Jump against a wall'),
        ('Dash', 'Left Shift / B'),
    ],
    '/Game/Variant_Shooter/Lvl_Shooter': [
        ('Move', 'WASD / Left stick'),
        ('Look', 'Mouse / Right stick'),
        ('Fire', 'Left mouse / RT'),
        ('Switch weapon', 'Left Shift / Y'),
        ('Jump', 'Space / A'),
    ],
    '/Game/Variant_Horror/Lvl_Horror': [
        ('Move', 'WASD / Left stick'),
        ('Look', 'Mouse / Right stick'),
        ('Sprint', 'Hold Left Shift / LB'),
        ('Jump', 'Space / A'),
    ],
}
BACKUP_ROOT = r'B:\NewUE5Project\Backups'


def log(message):
    unreal.log_warning('CONTROLS ' + message)


project = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

for level, rows in LEVELS.items():
    if not unreal.EditorAssetLibrary.does_asset_exist(level):
        continue
    relative = level[len('/Game/'):]
    for source in (os.path.join(project, 'Content', relative + '.umap'),
                   os.path.join(project, 'Content', '__ExternalActors__', relative),
                   os.path.join(project, 'Content', '__ExternalObjects__', relative)):
        if not os.path.exists(source):
            continue
        target = os.path.join(BACKUP_ROOT, stamp + '_controls', os.path.relpath(source, project))
        if os.path.isdir(source):
            shutil.copytree(source, target)
        else:
            os.makedirs(os.path.dirname(target), exist_ok=True)
            shutil.copy2(source, target)

    levels.load_level(level)
    switches = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.FeelSwitch)]
    if len(switches) != 1:
        log('{0}: expected one Feel Switch, found {1}'.format(level, len(switches)))
        continue
    switch = switches[0]
    switch.modify()
    controls = []
    for action, keys in rows:
        row = unreal.FeelSwitchControl()
        row.set_editor_property('action', unreal.Text(action))
        row.set_editor_property('keys', unreal.Text(keys))
        controls.append(row)
    switch.set_editor_property('controls', controls)
    levels.save_current_level()
    saved = switch.get_editor_property('controls')
    log('{0}: {1} rows ({2})'.format(level, len(saved), ', '.join(str(r.get_editor_property('action')) for r in saved)))
