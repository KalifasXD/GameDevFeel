"""Lists, for each demo's player controller, the Input Mapping Contexts it adds and every key in them (read only).
Run with -run=pythonscript in GameFeelDev or FeelDemoFP; controllers that do not exist in that project are skipped."""
import unreal

CONTROLLERS = [
    '/Game/Variant_Combat/Blueprints/BP_CombatPlayerController',
    '/Game/Variant_Platforming/Blueprints/BP_PlatformingPlayerController',
    '/Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController',
    '/Game/Variant_Horror/Blueprints/BP_HorrorPlayerController',
]

for path in CONTROLLERS:
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        continue
    defaults = unreal.get_default_object(unreal.load_asset(path).generated_class())
    for prop in ('default_mapping_contexts', 'mobile_excluded_mapping_contexts'):
        try:
            contexts = defaults.get_editor_property(prop)
        except Exception:
            continue
        for imc in contexts:
            if not imc:
                continue
            rows = sorted('{0} <- {1}'.format(m.action.get_name() if m.action else '?', m.key.get_editor_property('key_name'))
                          for m in imc.get_editor_property('mappings'))
            unreal.log_warning('CONTROLS {0} [{1}] {2}: {3}'.format(path.split('/')[-1], prop, imc.get_name(), '; '.join(rows)))
