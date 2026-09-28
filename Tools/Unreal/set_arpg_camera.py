"""Sets the Action/RPG demo camera on BP_CombatCharacter: the template's camera sits 1 m behind the character, which
then covers whatever it hits. Values chosen by measurement (DiagFeel.ARPGCameraStudy, project log D-065). Backs up the
Blueprint first. Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import datetime
import os
import shutil
import unreal

PROJECT = r'B:\NewUE5Project\GameFeelDev'
ASSET = '/Game/Variant_Combat/Blueprints/BP_CombatCharacter'
FILE = os.path.join(PROJECT, 'Content', 'Variant_Combat', 'Blueprints', 'BP_CombatCharacter.uasset')
BACKUP_ROOT = r'B:\NewUE5Project\Backups'

ARM = 350.0
MOUNT = unreal.Vector(0.0, 30.0, 70.0)
FIELD_OF_VIEW = 90.0
DEATH_ARM = 550.0


def log(message):
    # Commandlet logs only show warnings and errors from Python.
    unreal.log_warning('ARPGCAM ' + message)


stamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
target = os.path.join(BACKUP_ROOT, stamp, 'Content', 'Variant_Combat', 'Blueprints')
os.makedirs(target, exist_ok=True)
shutil.copy2(FILE, target)
log('backed up BP_CombatCharacter to ' + target)

blueprint = unreal.load_asset(ASSET)
defaults = unreal.get_default_object(blueprint.generated_class())
boom = defaults.get_editor_property('camera_boom')
camera = defaults.get_editor_property('follow_camera')
for thing in (defaults, boom, camera):
    thing.modify()

boom.set_editor_property('target_arm_length', ARM)
boom.set_editor_property('relative_location', MOUNT)
camera.set_editor_property('field_of_view', FIELD_OF_VIEW)
defaults.set_editor_property('default_camera_distance', ARM)
defaults.set_editor_property('death_camera_distance', DEATH_ARM)

unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
log('saved: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False)))

# Read back through a fresh load of the defaults.
defaults = unreal.get_default_object(blueprint.generated_class())
boom = defaults.get_editor_property('camera_boom')
log('arm {0}, mount {1}, field of view {2}, default distance {3}, death distance {4}'.format(
    boom.get_editor_property('target_arm_length'), boom.get_editor_property('relative_location'),
    defaults.get_editor_property('follow_camera').get_editor_property('field_of_view'),
    defaults.get_editor_property('default_camera_distance'), defaults.get_editor_property('death_camera_distance')))
