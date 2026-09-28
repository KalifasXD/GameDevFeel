"""Places (or updates) the Feel Switch in a demo level. Backs up the level and its actor files first.
Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script="<this file> <level> <title> <text>"
Without arguments it sets up the Action/RPG demo level.
"""
import datetime
import os
import shutil
import sys
import unreal

PROJECT = r'B:\NewUE5Project\GameFeelDev'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'

DEMOS = {
    '/Game/Variant_Combat/Lvl_Combat': (
        'FeelKit Demo: Weight Class',
        "Unreal's Combat template, unchanged apart from FeelKit. Hit the training dummies, then press the switch and hit them again.",
    ),
}

args = sys.argv[1:]
level = args[0] if args else '/Game/Variant_Combat/Lvl_Combat'
title, text = (args[1], args[2]) if len(args) >= 3 else DEMOS[level]

# 1. Back up the level and its one-file-per-actor folders.
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
relative = level[len('/Game/'):].replace('/', os.sep)
sources = [
    os.path.join(PROJECT, 'Content', relative + '.umap'),
    os.path.join(PROJECT, 'Content', '__ExternalActors__', relative),
    os.path.join(PROJECT, 'Content', '__ExternalObjects__', relative),
]
for source in sources:
    if not os.path.exists(source):
        continue
    target = os.path.join(BACKUP_ROOT, stamp, os.path.relpath(source, PROJECT))
    if os.path.isdir(source):
        shutil.copytree(source, target)
    else:
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copy2(source, target)
unreal.log_warning('FEELSWITCH backed up {0} to {1}'.format(level, os.path.join(BACKUP_ROOT, stamp)))

# 2. Load the level, find or add the Feel Switch, set its start card.
if not unreal.EditorLoadingAndSavingUtils.load_map(level):
    unreal.log_error('FEELSWITCH could not load {0}'.format(level))
else:
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    switches = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.FeelSwitch)]
    if switches:
        switch = switches[0]
        unreal.log_warning('FEELSWITCH updating the existing {0}'.format(switch.get_name()))
    else:
        switch = actors.spawn_actor_from_class(unreal.FeelSwitch, unreal.Vector(0.0, 0.0, 0.0))
        switch.set_actor_label('FeelSwitch')
        unreal.log_warning('FEELSWITCH added {0}'.format(switch.get_name()))
    switch.set_editor_property('start_card_title', title)
    switch.set_editor_property('start_card_text', text)
    saved = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log_warning('FEELSWITCH saved: {0}'.format(saved))
