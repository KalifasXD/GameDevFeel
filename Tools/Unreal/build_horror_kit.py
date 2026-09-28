"""Builds the Horror demo kit in the FeelDemoFP project: backs up every file it touches, creates or refreshes the recipes
from JSON, fills the FeelKit recipe slots on the Horror character, places Trigger Boxes with Feel Trigger components
(doorway creaks and scare spots), gives each flickering lamp a reaction of its own, and places the Feel Switch.
Run with the editor closed:
UnrealEditor-Cmd.exe FeelDemoFP.uproject -run=pythonscript -script=<this file>
"""
import datetime
import glob
import os
import shutil
import unreal

PROJECT = r'B:\NewUE5Project\FeelDemoFP'
PLUGIN = r'B:\NewUE5Project\GameFeelDev\Plugins\FeelKit'
JSON_DIR = os.path.join(PLUGIN, 'Demos', 'Horror')
KIT_DIR = os.path.join(PLUGIN, 'Content', 'Demos', 'Horror')
PACKAGE_PATH = '/FeelKit/Demos/Horror'
CHARACTER = '/Game/Variant_Horror/Blueprints/BP_HorrorCharacter'
HUD = '/Game/Variant_Horror/UI/UI_Horror'
LEVEL = '/Game/Variant_Horror/Lvl_Horror'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'
PLACED_PREFIX = 'FeelKit_'


def log(message):
    # Commandlet logs only show warnings and errors from Python.
    unreal.log_warning('HORRORKIT ' + message)


# 1. Back up every file this script may change.
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
backup = os.path.join(BACKUP_ROOT, stamp)
sources = glob.glob(os.path.join(KIT_DIR, '*.uasset')) + [
    os.path.join(PROJECT, 'Content', 'Variant_Horror', 'Blueprints', 'BP_HorrorCharacter.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Horror', 'UI', 'UI_Horror.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Horror', 'Lvl_Horror.umap'),
    os.path.join(PROJECT, 'Content', '__ExternalActors__', 'Variant_Horror', 'Lvl_Horror'),
    os.path.join(PROJECT, 'Content', '__ExternalObjects__', 'Variant_Horror', 'Lvl_Horror'),
]
for source in sources:
    if not os.path.exists(source):
        continue
    base = PLUGIN if source.startswith(PLUGIN) else PROJECT
    target = os.path.join(backup, os.path.basename(base), os.path.relpath(source, base))
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

# 3. Recipe slots on the character.
blueprint = unreal.load_asset(CHARACTER)
defaults = unreal.get_default_object(blueprint.generated_class())
for prop, recipe_name in (('sprint_feel', 'FR_HOR_Sprint'), ('out_of_breath_feel', 'FR_HOR_OutOfBreath')):
    defaults.set_editor_property(prop, recipes.get(recipe_name))
    value = defaults.get_editor_property(prop)
    log('BP_HorrorCharacter.{0} = {1}'.format(prop, value.get_name() if value else 'NONE'))

# A Feel Trigger on the character for landings.
subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = subobjects.k2_gather_subobject_data_for_blueprint(blueprint)
character_trigger = None
for handle in handles:
    candidate = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle))
    if isinstance(candidate, unreal.FeelTriggerComponent):
        character_trigger = candidate
        break
if character_trigger is None:
    new_handle, failure = subobjects.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=unreal.FeelTriggerComponent, blueprint_context=blueprint))
    if failure.is_empty():
        subobjects.rename_subobject(new_handle, unreal.Text('FeelTrigger'))
        character_trigger = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(new_handle))
    else:
        log('FAILED adding a Feel Trigger to the character: {0}'.format(failure))
if character_trigger is not None:
    landing = unreal.FeelTriggerEntry()
    landing.set_editor_property('event', unreal.FeelTriggerEvent.LANDED)
    landing.set_editor_property('recipe', recipes.get('FR_HOR_Land'))
    landing.set_editor_property('value_parameter', 'LandSpeed')
    landing.set_editor_property('target_skeletal_mesh', False)
    character_trigger.set_editor_property('triggers', [landing])
    log('character landings -> FR_HOR_Land')
unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
log('BP_HorrorCharacter saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False)))

# HUD (D-064): the sprint meter beats while it refills from empty, and brightens when it is full again.
hud = unreal.load_asset(HUD)
hud_defaults = unreal.get_default_object(hud.generated_class())
hud_defaults.modify()
for prop, recipe_name in (('breathless_feel', 'FR_HOR_HudBreathless'), ('recovered_feel', 'FR_HOR_HudRecovered')):
    hud_defaults.set_editor_property(prop, recipes.get(recipe_name))
    value = hud_defaults.get_editor_property(prop)
    log('UI_Horror.{0} = {1}'.format(prop, value.get_name() if value else 'NONE'))
unreal.BlueprintEditorLibrary.compile_blueprint(hud)
log('UI_Horror saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(hud, False)))

# 4. The level: trigger boxes and the Feel Switch.
if not unreal.EditorLoadingAndSavingUtils.load_map(LEVEL):
    log('FAILED loading ' + LEVEL)
    raise SystemExit
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level_actors = actors.get_all_level_actors()

# Boxes this script placed before are replaced, not stacked.
for actor in level_actors:
    if actor.get_actor_label().startswith(PLACED_PREFIX):
        actors.destroy_actor(actor)
level_actors = actors.get_all_level_actors()


def feel_trigger_on(actor):
    """The Feel Trigger component of a placed actor, added if it has none."""
    for component in actor.get_components_by_class(unreal.FeelTriggerComponent):
        return component
    handles = subobjects.k2_gather_subobject_data_for_instance(actor)
    new_handle, failure = subobjects.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=unreal.FeelTriggerComponent, blueprint_context=None))
    if not failure.is_empty():
        log('FAILED adding a Feel Trigger to {0}: {1}'.format(actor.get_actor_label(), failure))
        return None
    return unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(new_handle))


def entry(recipe_name, play_on_other, value_parameter='', event=unreal.FeelTriggerEvent.BEGIN_OVERLAP):
    item = unreal.FeelTriggerEntry()
    item.set_editor_property('event', event)
    item.set_editor_property('recipe', recipes.get(recipe_name))
    item.set_editor_property('value_parameter', value_parameter)
    item.set_editor_property('play_on', unreal.FeelTriggerPlayOn.OTHER_ACTOR if play_on_other else unreal.FeelTriggerPlayOn.OWNER)
    item.set_editor_property('only_for_players', True)
    item.set_editor_property('target_skeletal_mesh', False)
    return item


def place_zone(label, location, extent, recipe_name, play_on_other):
    box = actors.spawn_actor_from_class(unreal.TriggerBox, location)
    box.set_actor_label(label)
    box.get_component_by_class(unreal.BoxComponent).set_editor_property('box_extent', extent)
    trigger = feel_trigger_on(box)
    if trigger is None:
        return
    trigger.set_editor_property('triggers', [entry(recipe_name, play_on_other)])
    log('placed {0} -> {1}'.format(label, recipe_name))


# The lamps themselves react: a sphere around each flickering lamp, and the unease plays on the lamp, so the buzz comes
# from it and you can see it drop out.
lights = [a for a in level_actors if a.get_class().get_name().startswith('Light_C')]
flickering = []
for light in lights:
    try:
        if light.get_editor_property('Use Light Flicker'):
            flickering.append(light)
    except Exception as error:
        log('could not read Use Light Flicker on {0}: {1}'.format(light.get_name(), error))
log('lights: {0}, flickering: {1}'.format(len(lights), len(flickering)))
for light in flickering:
    spheres = [c for c in light.get_components_by_class(unreal.SphereComponent)]
    if not spheres:
        handles = subobjects.k2_gather_subobject_data_for_instance(light)
        new_handle, failure = subobjects.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=unreal.SphereComponent, blueprint_context=None))
        if not failure.is_empty():
            log('FAILED adding a sphere to {0}: {1}'.format(light.get_actor_label(), failure))
            continue
        sphere = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(new_handle))
    else:
        sphere = spheres[0]
    sphere.set_editor_property('sphere_radius', 260.0)
    sphere.set_collision_profile_name('OverlapAllDynamic')
    sphere.set_editor_property('generate_overlap_events', True)
    sphere.set_relative_location(unreal.Vector(0.0, 0.0, -170.0), False, False)
    trigger = feel_trigger_on(light)
    if trigger is not None:
        trigger.set_editor_property('triggers', [entry('FR_HOR_LightUnease', play_on_other=False)])
        log('lamp {0} reacts when someone walks under it'.format(light.get_actor_label()))

# Every doorway creaks as you step through it.
doors = [a for a in level_actors if a.get_class().get_name().startswith('BP_DoorFrame')]
for index, door in enumerate(doors, 1):
    place_zone('{0}Doorway_{1}'.format(PLACED_PREFIX, index), door.get_actor_location() + unreal.Vector(0.0, 0.0, 120.0), unreal.Vector(70.0, 70.0, 140.0), 'FR_HOR_Doorway', play_on_other=False)

# Scare spots in three doorways: the ones farthest from where the player starts. These play on the player, because the
# flashlight that dies is the player's own.
starts = [a for a in level_actors if isinstance(a, unreal.PlayerStart)]
start = starts[0].get_actor_location() if starts else unreal.Vector(0.0, 0.0, 0.0)
doors.sort(key=lambda door: -(door.get_actor_location() - start).length())
for index, door in enumerate(doors[:3], 1):
    place_zone('{0}Scare_{1}'.format(PLACED_PREFIX, index), door.get_actor_location() + unreal.Vector(0.0, 0.0, 120.0), unreal.Vector(90.0, 90.0, 150.0), 'FR_HOR_Scare', play_on_other=True)

switches = [a for a in level_actors if isinstance(a, unreal.FeelSwitch)]
switch = switches[0] if switches else actors.spawn_actor_from_class(unreal.FeelSwitch, unreal.Vector(0.0, 0.0, 0.0))
if not switches:
    switch.set_actor_label('FeelSwitch')
switch.set_editor_property('start_card_title', 'FeelKit Demo: Heartbeat')
switch.set_editor_property('start_card_text', "Unreal's First Person Horror template. Walk the halls, run until you are out of breath, and keep an eye on the doorways.")
# Saving the level (not only the dirty packages) is what deletes the files of the boxes removed above.
log('level saved: {0}, other packages: {1}'.format(unreal.EditorLevelLibrary.save_current_level(), unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)))
