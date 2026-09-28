"""Imports the chosen Shooter kit sounds into /FeelKit/Samples/Sounds under their FeelKit names, creates the sample
attenuation ATT_FK_World for sounds placed in the world, and removes the temporary audition folder. Picks are based on
DiagFeel.SoundStats (2026-09-22). Run with the editor closed:
UnrealEditor-Cmd.exe GameFeelDev.uproject -run=pythonscript -script=<this file>
"""
import os
import unreal

DOWNLOADS = r'C:\Users\Billo\Downloads'


def find(pack, name):
    for root, _, files in os.walk(os.path.join(DOWNLOADS, pack)):
        if name in files:
            return os.path.join(root, name)
    raise RuntimeError('missing ' + name)


PICKS = {
    'S_FK_Shot_Pistol': find('kenney_sci-fi-sounds', 'laserRetro_001.ogg'),
    'S_FK_Shot_Rifle': find('kenney_sci-fi-sounds', 'laserRetro_000.ogg'),
    'S_FK_Shot_Thump': find('kenney_sci-fi-sounds', 'laserSmall_004.ogg'),
    'S_FK_Shot_Launcher': find('kenney_sci-fi-sounds', 'laserLarge_001.ogg'),
    'S_FK_Explosion_Crunch': find('kenney_sci-fi-sounds', 'explosionCrunch_000.ogg'),
    'S_FK_Explosion_Low': find('kenney_sci-fi-sounds', 'lowFrequency_explosion_001.ogg'),
    'S_FK_Impact_Ping': find('kenney_impact-sounds', 'impactMetal_light_001.ogg'),
    'S_FK_Impact_Thud': find('kenney_sci-fi-sounds', 'impactMetal_004.ogg'),
    'S_FK_Hit_Tick': find('kenney_interface-sounds', 'click_002.ogg'),
    'S_FK_Kill_Confirm': find('kenney_interface-sounds', 'confirmation_001.ogg'),
    'S_FK_Weapon_Switch': find('kenney_rpg-audio', 'metalLatch.ogg'),
    'S_FK_Dry_Click': find('kenney_interface-sounds', 'switch_004.ogg'),
}

tasks = []
for name, path in PICKS.items():
    if unreal.EditorAssetLibrary.does_asset_exist('/FeelKit/Samples/Sounds/' + name):
        unreal.log_warning('SOUNDIMPORT {0} already exists; left as it is'.format(name))
        continue
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = '/FeelKit/Samples/Sounds'
    task.destination_name = name
    task.automated = True
    task.replace_existing = False
    task.save = True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    unreal.log_warning('SOUNDIMPORT {0} -> {1}'.format(os.path.basename(task.filename), list(task.imported_object_paths)))

# Attenuation for sounds placed in the world: full volume within 2 m, fading out over 30 m.
ATT = '/FeelKit/Samples/Sounds/ATT_FK_World'
if not unreal.EditorAssetLibrary.does_asset_exist(ATT):
    attenuation = unreal.AssetToolsHelpers.get_asset_tools().create_asset('ATT_FK_World', '/FeelKit/Samples/Sounds', unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
    settings = attenuation.get_editor_property('attenuation')
    settings.set_editor_property('attenuation_shape_extents', unreal.Vector(200.0, 0.0, 0.0))
    settings.set_editor_property('falloff_distance', 3000.0)
    settings.set_editor_property('spatialize', True)
    attenuation.set_editor_property('attenuation', settings)
    unreal.log_warning('SOUNDIMPORT created ATT_FK_World: {0}'.format(unreal.EditorAssetLibrary.save_loaded_asset(attenuation, False)))

if unreal.EditorAssetLibrary.does_directory_exist('/Game/_FeelAudition'):
    unreal.EditorAssetLibrary.delete_directory('/Game/_FeelAudition')
    unreal.log_warning('SOUNDIMPORT removed /Game/_FeelAudition')
