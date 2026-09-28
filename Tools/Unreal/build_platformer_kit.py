"""Builds the Platformer demo kit: backs up every file it touches, creates or refreshes the five recipes from JSON,
adds a Feel Trigger component to the Platforming character (jump, air jump, launch, landing), puts a Play Feel marker at
the start of the dash animation, and places the Feel Switch in the Platforming level.
Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
(set FEELKIT_PROJECT to the project folder when it is not GameFeelDev)
"""
import datetime
import glob
import os
import shutil
import unreal

# Another project (for example a fresh template project on a newer engine) can be named in FEELKIT_PROJECT.
PROJECT = os.environ.get('FEELKIT_PROJECT', r'B:\NewUE5Project\GameFeelDev')
JSON_DIR = os.path.join(PROJECT, 'Plugins', 'FeelKit', 'Demos', 'Platformer')
KIT_DIR = os.path.join(PROJECT, 'Plugins', 'FeelKit', 'Content', 'Demos', 'Platformer')
PACKAGE_PATH = '/FeelKit/Demos/Platformer'
CHARACTER = '/Game/Variant_Platforming/Blueprints/BP_PlatformingCharacter'
DASH = '/Game/Variant_Platforming/Anims/AM_Dash'
LEVEL = '/Game/Variant_Platforming/Lvl_Platforming'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'


def log(message):
    # Commandlet logs only show warnings and errors from Python.
    unreal.log_warning('PLATKIT ' + message)


# 1. Back up every file this script may change (no version control in this project).
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
backup = os.path.join(BACKUP_ROOT, stamp)
sources = glob.glob(os.path.join(KIT_DIR, '*.uasset')) + [
    os.path.join(PROJECT, 'Content', 'Variant_Platforming', 'Blueprints', 'BP_PlatformingCharacter.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Platforming', 'Anims', 'AM_Dash.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Platforming', 'Lvl_Platforming.umap'),
    os.path.join(PROJECT, 'Content', '__ExternalActors__', 'Variant_Platforming', 'Lvl_Platforming'),
    os.path.join(PROJECT, 'Content', '__ExternalObjects__', 'Variant_Platforming', 'Lvl_Platforming'),
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
log('backed up {0} paths to {1}'.format(len(sources), backup))

# 2. Recipes from JSON.
recipes = {}
for path in sorted(glob.glob(os.path.join(JSON_DIR, '*.json'))):
    name = os.path.splitext(os.path.basename(path))[0]
    recipe, message = unreal.FeelEditorScripting.create_recipe_from_json_file(PACKAGE_PATH, name, path)
    recipes[name] = recipe
    log('{0}: {1}'.format(name, message) if recipe else 'FAILED {0}: {1}'.format(name, message))

# 3. Feel Trigger component on the character.
blueprint = unreal.load_asset(CHARACTER)
subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = subobjects.k2_gather_subobject_data_for_blueprint(blueprint)
trigger = None
for handle in handles:
    data = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
    candidate = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    if isinstance(candidate, unreal.FeelTriggerComponent):
        trigger = candidate
        log('character already has {0}; updating it'.format(candidate.get_name()))
        break
if trigger is None:
    params = unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=unreal.FeelTriggerComponent, blueprint_context=blueprint)
    new_handle, failure = subobjects.add_new_subobject(params)
    if not failure.is_empty():
        log('FAILED adding the Feel Trigger: {0}'.format(failure))
    else:
        subobjects.rename_subobject(new_handle, unreal.Text('FeelTrigger'))
        trigger = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(new_handle))
        log('added {0} to the character'.format(trigger.get_name()))


def entry(event, recipe_name, value_parameter=''):
    item = unreal.FeelTriggerEntry()
    item.set_editor_property('event', event)
    item.set_editor_property('recipe', recipes.get(recipe_name))
    item.set_editor_property('value_parameter', value_parameter)
    item.set_editor_property('target_skeletal_mesh', True)
    return item


if trigger is not None:
    trigger.set_editor_property('triggers', [
        entry(unreal.FeelTriggerEvent.JUMPED, 'FR_PLAT_Jump'),
        entry(unreal.FeelTriggerEvent.AIR_JUMPED, 'FR_PLAT_AirJump', 'JumpNumber'),
        entry(unreal.FeelTriggerEvent.LAUNCHED, 'FR_PLAT_WallJump', 'LaunchSpeed'),
        entry(unreal.FeelTriggerEvent.LANDED, 'FR_PLAT_Land', 'LandSpeed'),
    ])
    for item in trigger.get_editor_property('triggers'):
        recipe = item.get_editor_property('recipe')
        log('trigger {0} -> {1} (value {2})'.format(item.get_editor_property('event'), recipe.get_name() if recipe else 'NONE', item.get_editor_property('value_parameter')))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    log('character saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False)))

# 4. Play Feel marker at the start of the dash.
dash = unreal.load_asset(DASH)
track_name = 'FeelKit'
existing = [n for n in unreal.AnimationLibrary.get_animation_notify_events(dash) if isinstance(n.notify, unreal.AnimNotify_PlayFeel)]
if existing:
    notify = existing[0].notify
    log('dash already has a Play Feel marker; updating it')
else:
    if track_name not in [str(n) for n in unreal.AnimationLibrary.get_animation_notify_track_names(dash)]:
        unreal.AnimationLibrary.add_animation_notify_track(dash, track_name)
    notify = unreal.AnimationLibrary.add_animation_notify_event(dash, track_name, 0.0, unreal.AnimNotify_PlayFeel)
    log('added a Play Feel marker to the dash')
notify.set_editor_property('recipe', recipes.get('FR_PLAT_Dash'))
log('dash marker recipe: {0}'.format(notify.get_editor_property('recipe').get_name() if notify.get_editor_property('recipe') else 'NONE'))
log('dash saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(dash, False)))

# 5. Feel Switch in the level.
if unreal.EditorLoadingAndSavingUtils.load_map(LEVEL):
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    switches = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.FeelSwitch)]
    switch = switches[0] if switches else actors.spawn_actor_from_class(unreal.FeelSwitch, unreal.Vector(0.0, 0.0, 0.0))
    if not switches:
        switch.set_actor_label('FeelSwitch')
    switch.set_editor_property('start_card_title', 'FeelKit Demo: Bounce Feel')
    switch.set_editor_property('start_card_text', "Unreal's Platforming template. Jump, wall jump, dash and drop off something high, then try it all again with the feel off.")
    log('level saved: {0}'.format(unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)))
else:
    log('FAILED loading {0}'.format(LEVEL))
