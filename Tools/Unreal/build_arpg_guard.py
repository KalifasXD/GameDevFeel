"""Builds the guard part of the Action/RPG demo kit (D-069): the shield material, the Block input (Left Shift, left
trigger), the player's guard recipes, the guard enemy Blueprint (a copy of BP_CombatEnemy on CombatGuardEnemy) and one
guard enemy placed in Lvl_Combat near the training dummy. Backs up every existing file it touches first. Needs the C++
classes CombatGuardComponent and CombatGuardEnemy, and the recipes from build_arpg_kit.py. Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import datetime
import glob
import os
import shutil
import unreal

PROJECT = r'B:\NewUE5Project\GameFeelDev'
BACKUP_ROOT = r'B:\NewUE5Project\Backups'
KIT = '/Game/FeelKitDemos/ActionRPG'
MATERIAL = KIT + '/M_FK_GuardShield'
ACTIONS = '/Game/Variant_Combat/Input/Actions'
IMC = '/Game/Variant_Combat/Input/IMC_Combat'
PLAYER = '/Game/Variant_Combat/Blueprints/BP_CombatCharacter'
ENEMY = '/Game/Variant_Combat/Blueprints/AI/BP_CombatEnemy'
GUARD_ENEMY = '/Game/Variant_Combat/Blueprints/AI/BP_CombatGuardEnemy'
LEVEL = '/Game/Variant_Combat/Lvl_Combat'
GUARD_LABEL = 'FeelKit_GuardEnemy'


def log(message):
    # Commandlet logs only show warnings and errors from Python.
    unreal.log_warning('GUARDKIT ' + message)


# 1. Back up every file this script may change (no version control in this project).
stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
backup = os.path.join(BACKUP_ROOT, stamp + '_guard')
sources = [
    os.path.join(PROJECT, 'Content', 'FeelKitDemos', 'ActionRPG', 'M_FK_GuardShield.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Combat', 'Input'),
    os.path.join(PROJECT, 'Content', 'Variant_Combat', 'Blueprints', 'BP_CombatCharacter.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Combat', 'Blueprints', 'AI', 'BP_CombatGuardEnemy.uasset'),
    os.path.join(PROJECT, 'Content', 'Variant_Combat', 'Lvl_Combat.umap'),
    os.path.join(PROJECT, 'Content', '__ExternalActors__', 'Variant_Combat', 'Lvl_Combat'),
    os.path.join(PROJECT, 'Content', '__ExternalObjects__', 'Variant_Combat', 'Lvl_Combat'),
]
copied = 0
for source in sources:
    if not os.path.exists(source):
        continue
    target = os.path.join(backup, os.path.relpath(source, PROJECT))
    if os.path.isdir(source):
        shutil.copytree(source, target)
    else:
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copy2(source, target)
    copied += 1
log('backed up {0} paths to {1}'.format(copied, backup))

assets = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary

# 2. The shield material: see-through, bright at its edges, fading in with Raise, flaring with Glow.
material = unreal.load_asset(MATERIAL)
if not material:
    material = assets.create_asset('M_FK_GuardShield', KIT, unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(material)
material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
material.set_editor_property('two_sided', True)


def expression(kind, x, y, **properties):
    node = mel.create_material_expression(material, kind, x, y)
    for key, value in properties.items():
        node.set_editor_property(key, value)
    return node


color = expression(unreal.MaterialExpressionVectorParameter, -900, -200, parameter_name='ShieldColor', default_value=unreal.LinearColor(0.15, 0.65, 1.0, 1.0))
glow = expression(unreal.MaterialExpressionScalarParameter, -900, 50, parameter_name='Glow', default_value=1.0)
raise_ = expression(unreal.MaterialExpressionScalarParameter, -900, 300, parameter_name='Raise', default_value=0.0)
fresnel = expression(unreal.MaterialExpressionFresnel, -900, 500, exponent=2.5)

# Emissive = ShieldColor x (0.5 + 3 x Fresnel) x Glow
rim = expression(unreal.MaterialExpressionMultiply, -600, 450, const_b=3.0)
mel.connect_material_expressions(fresnel, '', rim, 'A')
rim_base = expression(unreal.MaterialExpressionAdd, -450, 450, const_b=0.5)
mel.connect_material_expressions(rim, '', rim_base, 'A')
tinted = expression(unreal.MaterialExpressionMultiply, -300, -150)
mel.connect_material_expressions(color, '', tinted, 'A')
mel.connect_material_expressions(rim_base, '', tinted, 'B')
emissive = expression(unreal.MaterialExpressionMultiply, -150, -100)
mel.connect_material_expressions(tinted, '', emissive, 'A')
mel.connect_material_expressions(glow, '', emissive, 'B')
mel.connect_material_property(emissive, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)

# Opacity = Raise x clamp(0.12 x Glow + 0.6 x Fresnel)
edge = expression(unreal.MaterialExpressionMultiply, -600, 650, const_b=0.6)
mel.connect_material_expressions(fresnel, '', edge, 'A')
body = expression(unreal.MaterialExpressionMultiply, -600, 150, const_b=0.12)
mel.connect_material_expressions(glow, '', body, 'A')
sum_ = expression(unreal.MaterialExpressionAdd, -450, 300)
mel.connect_material_expressions(body, '', sum_, 'A')
mel.connect_material_expressions(edge, '', sum_, 'B')
clamped = expression(unreal.MaterialExpressionClamp, -300, 300)
mel.connect_material_expressions(sum_, '', clamped, '')
opacity = expression(unreal.MaterialExpressionMultiply, -150, 300)
mel.connect_material_expressions(clamped, '', opacity, 'A')
mel.connect_material_expressions(raise_, '', opacity, 'B')
mel.connect_material_property(opacity, '', unreal.MaterialProperty.MP_OPACITY)
mel.recompile_material(material)
log('material saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(material, False)))

# 3. The Block input: hold to guard.
block = unreal.load_asset(ACTIONS + '/IA_Block')
if not block:
    block = assets.create_asset('IA_Block', ACTIONS, unreal.InputAction, unreal.InputAction_Factory())
block.set_editor_property('value_type', unreal.InputActionValueType.BOOLEAN)
unreal.EditorAssetLibrary.save_loaded_asset(block, False)

imc = unreal.load_asset(IMC)
wanted = ['LeftShift', 'Gamepad_LeftTriggerAxis']
present = [m.get_editor_property('key').get_editor_property('key_name') for m in imc.get_editor_property('mappings')
           if m.get_editor_property('action') == block]
for key_name in wanted:
    if key_name in present:
        continue
    key = unreal.Key()
    key.set_editor_property('key_name', key_name)
    imc.map_key(block, key)
    log('mapped {0} to IA_Block'.format(key_name))
log('input saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(imc, False)))

recipes = {name: unreal.load_asset('{0}/FR_ARPG_{1}'.format(KIT, name)) for name in ('GuardBlock', 'GuardBreak', 'Parry')}
for name, recipe in recipes.items():
    if not recipe:
        log('MISSING recipe FR_ARPG_{0}: run build_arpg_kit.py first'.format(name))


def set_guard_recipes(guard, parry):
    guard.modify()
    guard.set_editor_property('block_feel', recipes['GuardBlock'])
    guard.set_editor_property('break_feel', recipes['GuardBreak'])
    guard.set_editor_property('parry_feel', recipes['Parry'] if parry else None)


# 4. The player: block input and guard recipes.
player = unreal.load_asset(PLAYER)
defaults = unreal.get_default_object(player.generated_class())
defaults.modify()
defaults.set_editor_property('block_action', block)
set_guard_recipes(defaults.get_editor_property('guard'), True)
unreal.BlueprintEditorLibrary.compile_blueprint(player)
log('player saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(player, False)))

# 5. The guard enemy: a copy of the template enemy (looks, animations, life bar, the kit's hurt and death hooks) on the
#    CombatGuardEnemy class.
guard_enemy = unreal.load_asset(GUARD_ENEMY)
if not guard_enemy:
    guard_enemy = unreal.EditorAssetLibrary.duplicate_asset(ENEMY, GUARD_ENEMY)
    unreal.BlueprintEditorLibrary.reparent_blueprint(guard_enemy, unreal.CombatGuardEnemy.static_class())
    log('created BP_CombatGuardEnemy from BP_CombatEnemy')
unreal.BlueprintEditorLibrary.compile_blueprint(guard_enemy)
enemy_defaults = unreal.get_default_object(guard_enemy.generated_class())
enemy_defaults.modify()
enemy_defaults.set_editor_property('auto_possess_ai', unreal.AutoPossessAI.DISABLED)
enemy_defaults.set_editor_property('max_hp', 6.0)
set_guard_recipes(enemy_defaults.get_editor_property('guard'), False)
unreal.BlueprintEditorLibrary.compile_blueprint(guard_enemy)
log('guard enemy saved: {0}, class {1}'.format(unreal.EditorAssetLibrary.save_loaded_asset(guard_enemy, False),
                                               unreal.get_default_object(guard_enemy.generated_class()).get_class().get_name()))

# 6. One guard enemy in Lvl_Combat, between the player start and the training dummy's open side, facing the player start.
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels.load_level(LEVEL)
for actor in actors.get_all_level_actors():
    if actor.get_actor_label() == GUARD_LABEL:
        actors.destroy_actor(actor)
        log('removed the previous ' + GUARD_LABEL)
start = next(a for a in actors.get_all_level_actors() if a.get_class().get_name() == 'PlayerStart')
dummy = next(a for a in actors.get_all_level_actors() if a.get_class().get_name().startswith('BP_CombatDummy'))
start_location = start.get_actor_location()
dummy_location = dummy.get_actor_location()
spot = unreal.Vector(dummy_location.x, dummy_location.y - 450.0, dummy_location.z + 100.0)
facing = unreal.MathLibrary.find_look_at_rotation(spot, unreal.Vector(start_location.x, start_location.y, spot.z))
placed = actors.spawn_actor_from_class(guard_enemy.generated_class(), spot, unreal.Rotator(roll=0.0, pitch=0.0, yaw=facing.yaw))
placed.modify()
placed.set_actor_label(GUARD_LABEL)
log('placed {0} at {1}'.format(GUARD_LABEL, placed.get_actor_location()))
levels.save_current_level()
log('level saved: ' + LEVEL)
