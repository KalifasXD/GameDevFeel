"""Builds the Shooter demo kit in the FeelDemoFP project: backs up every file it touches, creates or refreshes the
recipes from JSON, fills the FeelKit recipe slots the kit's code hooks added to the template (weapons, projectiles,
enemy, player), adds Feel Trigger components to the jump pad and the player, adds the ShotHeat accumulator to Project Settings, and places the Feel Switch in the Shooter level.
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
JSON_DIR = os.path.join(PLUGIN, 'Demos', 'Shooter')
KIT_DIR = os.path.join(PLUGIN, 'Content', 'Demos', 'Shooter')
PACKAGE_PATH = '/FeelKit/Demos/Shooter'
LEVEL = '/Game/Variant_Shooter/Lvl_Shooter'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'

BP = '/Game/Variant_Shooter/Blueprints/'
JUMP_PAD = '/Game/LevelPrototyping/Interactable/JumpPad/BP_JumpPad'
SLOTS = {
    BP + 'Pickups/Weapons/BP_ShooterWeapon_Pistol': {'fire_feel': 'FR_SHOOT_Pistol', 'other_fire_feel': 'FR_SHOOT_EnemyFire'},
    BP + 'Pickups/Weapons/BP_ShooterWeapon_Rifle': {'fire_feel': 'FR_SHOOT_Rifle', 'other_fire_feel': 'FR_SHOOT_EnemyFire'},
    BP + 'Pickups/Weapons/BP_ShooterWeapon_GrenadeLauncher': {'fire_feel': 'FR_SHOOT_Launcher', 'other_fire_feel': 'FR_SHOOT_EnemyFire'},
    BP + 'Pickups/Projectiles/BP_ShooterProjectile_Bullet': {'impact_feel': 'FR_SHOOT_Impact'},
    BP + 'Pickups/Projectiles/BP_ShooterProjectile_Grenade': {'explosion_feel': 'FR_SHOOT_Explosion'},
    BP + 'AI/BP_ShooterNPC': {'hit_feel': 'FR_SHOOT_Hit', 'kill_feel': 'FR_SHOOT_Kill'},
    BP + 'BP_ShooterCharacter': {'hurt_feel': 'FR_SHOOT_Hurt', 'death_feel': 'FR_SHOOT_Death', 'pickup_feel': 'FR_SHOOT_Pickup',
                                 'switch_weapon_feel': 'FR_SHOOT_Switch', 'no_weapon_feel': 'FR_SHOOT_DryFire'},
    # HUD (D-064): the bullet counter and the score react.
    '/Game/Variant_Shooter/UI/UI_ShooterBulletCounter': {'shot_feel': 'FR_SHOOT_HudShot', 'empty_feel': 'FR_SHOOT_HudEmpty',
                                                         'reload_feel': 'FR_SHOOT_HudReload', 'hurt_feel': 'FR_SHOOT_HudHurt'},
    '/Game/Variant_Shooter/UI/UI_Shooter': {'score_feel': 'FR_SHOOT_HudScore'},
}


def log(message):
    # Commandlet logs only show warnings and errors from Python.
    unreal.log_warning('SHOOTKIT ' + message)


def content_file(package):
    return os.path.join(PROJECT, 'Content', package[len('/Game/'):].replace('/', os.sep) + '.uasset')


# 1. Back up every file this script may change.
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
backup = os.path.join(BACKUP_ROOT, stamp)
sources = glob.glob(os.path.join(KIT_DIR, '*.uasset')) + [content_file(p) for p in SLOTS] + [content_file(JUMP_PAD)] + [
    os.path.join(PROJECT, 'Content', 'Variant_Shooter', 'Lvl_Shooter.umap'),
    os.path.join(PROJECT, 'Content', '__ExternalActors__', 'Variant_Shooter', 'Lvl_Shooter'),
    os.path.join(PROJECT, 'Content', '__ExternalObjects__', 'Variant_Shooter', 'Lvl_Shooter'),
    os.path.join(PROJECT, 'Config', 'DefaultGame.ini'),
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

# 3. Recipe slots on the template's Blueprints (class defaults).
for package, slots in SLOTS.items():
    blueprint = unreal.load_asset(package)
    if not blueprint:
        log('FAILED loading ' + package)
        continue
    defaults = unreal.get_default_object(blueprint.generated_class())
    for prop, recipe_name in slots.items():
        defaults.set_editor_property(prop, recipes.get(recipe_name))
        value = defaults.get_editor_property(prop)
        log('{0}.{1} = {2}'.format(os.path.basename(package), prop, value.get_name() if value else 'NONE'))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    log('{0} saved: {1}'.format(os.path.basename(package), unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False)))

# 4. Feel Trigger components: the jump pad (on the player who steps on it) and the player's landings.
subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)


def feel_trigger(blueprint):
    handles = subobjects.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        candidate = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle))
        if isinstance(candidate, unreal.FeelTriggerComponent):
            return candidate
    params = unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=unreal.FeelTriggerComponent, blueprint_context=blueprint)
    new_handle, failure = subobjects.add_new_subobject(params)
    if not failure.is_empty():
        log('FAILED adding a Feel Trigger to {0}: {1}'.format(blueprint.get_name(), failure))
        return None
    subobjects.rename_subobject(new_handle, unreal.Text('FeelTrigger'))
    return unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(new_handle))


def trigger_entry(event, recipe_name, value_parameter='', play_on_other=False, only_players=False, skeletal_mesh=True):
    item = unreal.FeelTriggerEntry()
    item.set_editor_property('event', event)
    item.set_editor_property('recipe', recipes.get(recipe_name))
    item.set_editor_property('value_parameter', value_parameter)
    item.set_editor_property('target_skeletal_mesh', skeletal_mesh)
    item.set_editor_property('play_on', unreal.FeelTriggerPlayOn.OTHER_ACTOR if play_on_other else unreal.FeelTriggerPlayOn.OWNER)
    item.set_editor_property('only_for_players', only_players)
    return item


for package, entries in (
        (JUMP_PAD, [trigger_entry(unreal.FeelTriggerEvent.BEGIN_OVERLAP, 'FR_SHOOT_JumpPad', play_on_other=True, only_players=True, skeletal_mesh=False)]),
        (BP + 'BP_ShooterCharacter', [trigger_entry(unreal.FeelTriggerEvent.LANDED, 'FR_SHOOT_Land', 'LandSpeed', skeletal_mesh=False)])):
    blueprint = unreal.load_asset(package)
    trigger = feel_trigger(blueprint) if blueprint else None
    if trigger is None:
        log('FAILED: no Feel Trigger on ' + package)
        continue
    trigger.set_editor_property('triggers', entries)
    for item in trigger.get_editor_property('triggers'):
        recipe = item.get_editor_property('recipe')
        log('{0} trigger {1} -> {2}'.format(os.path.basename(package), item.get_editor_property('event'), recipe.get_name() if recipe else 'NONE'))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    log('{0} saved: {1}'.format(os.path.basename(package), unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False)))

# 5. The accumulator the weapon hook adds to on every shot.
log('ShotHeat in Project Settings: {0}'.format(unreal.FeelEditorScripting.set_accumulator_in_project_settings('ShotHeat', 8.0, 10.0, 0.12)))

# 6. Feel Switch in the level.
if unreal.EditorLoadingAndSavingUtils.load_map(LEVEL):
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    switches = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.FeelSwitch)]
    switch = switches[0] if switches else actors.spawn_actor_from_class(unreal.FeelSwitch, unreal.Vector(0.0, 0.0, 0.0))
    if not switches:
        switch.set_actor_label('FeelSwitch')
    switch.set_editor_property('start_card_title', 'FeelKit Demo: Every Bullet Has an Opinion')
    switch.set_editor_property('start_card_text', "Unreal's First Person Shooter template with FeelKit on top. Grab the weapons lying around and find someone to shoot.")
    log('level saved: {0}'.format(unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)))
else:
    log('FAILED loading ' + LEVEL)
