"""Imports every candidate sound for a demo kit (argument: ARPG or Platformer) into a temporary folder, /Game/_FeelAudition, so they can be
measured with FeelKit's audio analysis. The folder is deleted again after the picks are made.
Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import glob
import os
import unreal

DOWNLOADS = r'C:\Users\Billo\Downloads'
import sys
KIT = sys.argv[1] if len(sys.argv) > 1 else 'ARPG'
GROUP_SETS = {
    'ARPG': {
        'Swish': os.path.join(DOWNLOADS, 'oga_swishes', 'swishes', 'swish-*.wav'),
        'Sword': os.path.join(DOWNLOADS, 'oga_sword_-_starninjas_1', 'sword - StarNinjas', 'sword.*.ogg'),
        'Clash': os.path.join(DOWNLOADS, 'oga_sword_clash_-_starninjas_0', 'sword_clash.*.ogg'),
        'PunchMedium': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactPunch_medium_*.ogg'),
        'PunchHeavy': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactPunch_heavy_*.ogg'),
        'SoftHeavy': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactSoft_heavy_*.ogg'),
    },
    'Platformer': {
        'Swish': os.path.join(DOWNLOADS, 'oga_swishes', 'swishes', 'swish-*.wav'),
        'StepConcrete': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'footstep_concrete_*.ogg'),
        'SoftMedium': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactSoft_medium_*.ogg'),
        'SoftHeavy': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactSoft_heavy_*.ogg'),
        'GenericLight': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactGeneric_light_*.ogg'),
        'Pluck': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'pluck_*.ogg'),
        'Maximize': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'maximize_*.ogg'),
        'Cloth': os.path.join(DOWNLOADS, 'kenney_rpg-audio', '**', 'cloth*.ogg'),
    },
    'Shooter': {
        'LaserSmall': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'laserSmall_*.ogg'),
        'LaserLarge': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'laserLarge_*.ogg'),
        'LaserRetro': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'laserRetro_*.ogg'),
        'ExplosionCrunch': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'explosionCrunch_*.ogg'),
        'LowExplosion': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'lowFrequency_explosion_*.ogg'),
        'ImpactMetalSF': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'impactMetal_*.ogg'),
        'MetalLight': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactMetal_light_*.ogg'),
        'PlateLight': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactPlate_light_*.ogg'),
        'Click': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'click_*.ogg'),
        'Select': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'select_*.ogg'),
        'Confirm': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'confirmation_*.ogg'),
    },
    'ShooterExtra': {
        'MetalClick': os.path.join(DOWNLOADS, 'kenney_rpg-audio', '**', 'metalClick*.ogg'),
        'MetalLatch': os.path.join(DOWNLOADS, 'kenney_rpg-audio', '**', 'metalLatch*.ogg'),
        'BeltHandle': os.path.join(DOWNLOADS, 'kenney_rpg-audio', '**', 'beltHandle*.ogg'),
        'DrawKnife': os.path.join(DOWNLOADS, 'kenney_rpg-audio', '**', 'drawKnife*.ogg'),
        'Click': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'click_*.ogg'),
        'Switch': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'switch_*.ogg'),
    },
    'Guard': {
        'ForceField': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'forceField_*.ogg'),
        'MetalLight': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactMetal_light_*.ogg'),
        'MetalMedium': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactMetal_medium_*.ogg'),
        'MetalHeavy': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactMetal_heavy_*.ogg'),
        'Bell': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactBell_heavy_*.ogg'),
        'GlassMedium': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactGlass_medium_*.ogg'),
        'GlassHeavy': os.path.join(DOWNLOADS, 'kenney_impact-sounds', '**', 'impactGlass_heavy_*.ogg'),
    },
    'UI': {
        'Tick': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'tick_*.ogg'),
        'Pluck': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'pluck_*.ogg'),
        'Confirmation': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'confirmation_*.ogg'),
        'Bong': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'bong_*.ogg'),
        'Glass': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'glass_*.ogg'),
        'Toggle': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'toggle_*.ogg'),
        'Question': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'question_*.ogg'),
        'Maximize': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'maximize_*.ogg'),
    },
    'Horror': {
        'EngineCircular': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'engineCircular_*.ogg'),
        'SpaceEngineLow': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'spaceEngineLow_*.ogg'),
        'ComputerNoise': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'computerNoise_*.ogg'),
        'ForceField': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'forceField_*.ogg'),
        'LowExplosion': os.path.join(DOWNLOADS, 'kenney_sci-fi-sounds', '**', 'lowFrequency_explosion_*.ogg'),
        'Creak': os.path.join(DOWNLOADS, 'kenney_rpg-audio', '**', 'creak*.ogg'),
        'Glitch': os.path.join(DOWNLOADS, 'kenney_interface-sounds', '**', 'glitch_*.ogg'),
    },
}
GROUPS = GROUP_SETS[KIT]

tasks = []
for group, pattern in GROUPS.items():
    for path in sorted(glob.glob(pattern, recursive=True)):
        name = '{0}_{1}'.format(group, os.path.splitext(os.path.basename(path))[0].replace('.', '_').replace('-', '_'))
        task = unreal.AssetImportTask()
        task.filename = path
        task.destination_path = '/Game/_FeelAudition'
        task.destination_name = name
        task.automated = True
        task.replace_existing = True
        task.save = True
        tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = [t for t in tasks if t.imported_object_paths]
unreal.log_warning('AUDITION imported {0} of {1}'.format(len(imported), len(tasks)))
for task in tasks:
    if not task.imported_object_paths:
        unreal.log_warning('AUDITION failed {0}'.format(task.filename))
