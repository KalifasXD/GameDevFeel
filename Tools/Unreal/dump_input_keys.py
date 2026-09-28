"""Lists every key used by every Input Mapping Context in the project (read only)."""
import unreal
reg = unreal.AssetRegistryHelpers.get_asset_registry()
reg.search_all_assets(True)
reg.wait_for_completion()
assets = reg.get_assets_by_class(unreal.TopLevelAssetPath('/Script/EnhancedInput', 'InputMappingContext'), True)
for data in assets:
    imc = data.get_asset()
    keys = sorted(set('{0}->{1}'.format(m.key.get_editor_property('key_name'), m.action.get_name() if m.action else '?') for m in imc.get_editor_property('mappings')))
    unreal.log('IMCKEYS {0}: {1}'.format(data.package_name, ', '.join(keys)))
