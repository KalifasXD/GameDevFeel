"""Checks an installed FeelKit package from inside the editor (run by Tools/Run/check_packages.ps1).

Prints lines starting with CHECKPKG: the plugin, every class that one edition must or must not have, the Blueprint
nodes, and every asset under /FeelKit loaded one by one (recipes: every track must have its step). For Lite it also
lists sample assets that nothing in the package uses.
"""
import unreal

EDITION = "@EDITION@"

LITE_CLASSES = [
    "FeelCore.FeelStep_ProceduralShake", "FeelCore.FeelStep_CameraPunch", "FeelCore.FeelStep_FOVKick",
    "FeelCore.FeelStep_GlobalHitstop", "FeelCore.FeelStep_SlowMoRamp", "FeelCore.FeelStep_ScreenFlash",
    "FeelCore.FeelStep_VignettePulse", "FeelCore.FeelStep_ScalePunch", "FeelCore.FeelStep_SquashStretch",
    "FeelCore.FeelStep_PlaySound", "FeelCore.FeelStep_ForceFeedbackCurve", "FeelCore.FeelStep_BlueprintEvent",
    "FeelCore.FeelRecipe", "FeelCore.FeelSubsystem", "FeelCore.FeelComfortSubsystem", "FeelCore.FeelComfortPreset",
    "FeelCore.FeelComfortMenu", "FeelCore.FeelSwitch", "FeelCore.FeelSettings", "FeelCore.FeelBlueprintLibrary",
    "FeelEditor.FeelEditorScripting",
]
PRO_CLASSES = [
    "FeelCore.FeelStep_ActorHitstop", "FeelCore.FeelStep_ChromaticAberration", "FeelCore.FeelStep_MaterialPulse",
    "FeelCore.FeelStep_HitFlash", "FeelCore.FeelStep_MeshWobble", "FeelCore.FeelStep_LightFlash",
    "FeelCore.FeelStep_SoundClassDuck", "FeelCore.FeelStep_PitchBend", "FeelCore.FeelStep_LowPassSweep",
    "FeelCore.FeelStep_HapticPattern", "FeelCore.FeelStep_CameraRoll", "FeelCore.FeelStep_CameraZoom",
    "FeelCore.FeelStep_LookAtNudge", "FeelCore.FeelStep_Desaturate", "FeelCore.FeelStep_ColorTint",
    "FeelCore.FeelStep_ScreenFade", "FeelCore.FeelStep_PostProcessMaterialPulse", "FeelCore.FeelStep_NumberPop",
    "FeelCore.FeelStep_SpawnDecal", "FeelCore.FeelStep_WidgetPunch", "FeelCore.FeelStep_WidgetShake",
    "FeelCore.FeelStep_WidgetFlash", "FeelCore.AnimNotify_PlayFeel", "FeelCore.AnimNotifyState_PlayFeel",
    "FeelCore.FeelMap", "FeelCore.FeelTriggerComponent", "FeelCore.FeelReplicationComponent",
    "FeelNiagara.FeelStep_SpawnParticle", "FeelEnhancedInput.FeelInputComponent",
]
LITE_FUNCTIONS = ["play_feel", "stop_feel", "stop_all_feel", "is_feel_playing", "make_feel_target_from_actor",
                  "get_feel_comfort", "set_feel_enabled", "toggle_feel", "is_feel_enabled"]
PRO_FUNCTIONS = ["play_feel_with_context", "send_feel_event", "release_feel", "set_feel_parameter",
                 "add_to_feel_accumulator", "get_feel_accumulator"]
PRO_RECIPE_FIELDS = ["parameters", "sustain", "jump_to_end_on_release", "release_parameter", "release_at",
                     "full_release_recipe", "early_release_recipe", "feeling", "description"]


def log(text):
    unreal.log_warning("CHECKPKG " + text)


def has_class(path):
    module, name = path.split(".")
    return unreal.find_object(None, f"/Script/{module}.{name}") is not None


pro = EDITION == "Pro"
problems = []

plugins = unreal.PluginBlueprintLibrary.get_enabled_plugin_names()
log(f"edition {EDITION}, FeelKit enabled {'FeelKit' in plugins}, FeelKitGAS enabled {'FeelKitGAS' in plugins}")
if "FeelKit" not in plugins:
    problems.append("FeelKit is not enabled")

missing = [c for c in LITE_CLASSES if not has_class(c)]
pro_found = [c for c in PRO_CLASSES if has_class(c)]
log(f"core classes present {len(LITE_CLASSES) - len(missing)}/{len(LITE_CLASSES)}; Pro classes present {len(pro_found)}/{len(PRO_CLASSES)}")
if missing:
    problems.append("missing classes: " + ", ".join(missing))
if pro and len(pro_found) != len(PRO_CLASSES):
    problems.append("Pro classes missing: " + ", ".join(c for c in PRO_CLASSES if c not in pro_found))
if not pro and pro_found:
    problems.append("Pro classes in Lite: " + ", ".join(pro_found))

library = unreal.FeelBlueprintLibrary
fn_missing = [f for f in LITE_FUNCTIONS if not hasattr(library, f)]
fn_pro = [f for f in PRO_FUNCTIONS if hasattr(library, f)]
log(f"Blueprint nodes: core missing {fn_missing}, Pro nodes present {len(fn_pro)}/{len(PRO_FUNCTIONS)}")
if fn_missing:
    problems.append("missing nodes: " + ", ".join(fn_missing))
if pro and len(fn_pro) != len(PRO_FUNCTIONS):
    problems.append("Pro nodes missing")
if not pro and fn_pro:
    problems.append("Pro nodes in Lite: " + ", ".join(fn_pro))

registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.scan_paths_synchronous(["/FeelKit"], True)
assets = registry.get_assets_by_path("/FeelKit", recursive=True)
by_folder = {}
failed = []
recipes = 0
bad_tracks = []
recipe_names = []
for data in assets:
    path = str(data.package_name)
    folder = path.split("/")[2] if len(path.split("/")) > 3 else "(root)"
    by_folder[folder] = by_folder.get(folder, 0) + 1
    asset = unreal.load_asset(path)
    if asset is None:
        failed.append(path)
        continue
    if isinstance(asset, unreal.FeelRecipe):
        recipes += 1
        recipe_names.append(path)
        for index, track in enumerate(asset.get_editor_property("tracks")):
            try:
                step = track.get_editor_property("step")
            except Exception:
                step = None
            if step is None:
                bad_tracks.append(f"{path} track {index}")
log(f"assets {len(assets)} by folder {sorted(by_folder.items())}; failed to load {len(failed)}; recipes {recipes}; tracks without a step {len(bad_tracks)}")
for item in failed + bad_tracks:
    log("  problem " + item)
if failed or bad_tracks:
    problems.append("assets failed or tracks without steps")
library_recipes = [r for r in recipe_names if r.startswith("/FeelKit/Library/")]
log(f"library recipes {len(library_recipes)}: " + ", ".join(r.split("/")[-1] for r in sorted(library_recipes)))

menu = unreal.load_asset("/FeelKit/UI/WBP_FeelComfortMenu")
log(f"comfort menu loads {menu is not None}")
if menu is None:
    problems.append("comfort menu does not load")

# Pro-only recipe fields: shown to Pro, hidden from Lite (the data stays in the asset).
probe = unreal.load_asset(library_recipes[0]) if library_recipes else None
if probe:
    visible = []
    for field in PRO_RECIPE_FIELDS:
        try:
            probe.get_editor_property(field)
            visible.append(field)
        except Exception:
            pass
    log(f"Pro recipe fields visible to scripting: {visible}")
    if pro and len(visible) != len(PRO_RECIPE_FIELDS):
        problems.append("Pro recipe fields hidden in Pro")
    if not pro and visible:
        problems.append("Pro recipe fields visible in Lite")

# Sample content that nothing else in the package uses.
unused = []
for data in assets:
    path = str(data.package_name)
    if not path.startswith("/FeelKit/Samples/"):
        continue
    options = unreal.AssetRegistryDependencyOptions(True, True, True, True, True)
    users = [str(p) for p in registry.get_referencers(path, options) if str(p).startswith("/FeelKit/")]
    if not users:
        unused.append(path.split("/")[-1])
log(f"sample assets used by nothing in the package: {len(unused)} {unused}")

log("RESULT " + ("PASS" if not problems else "FAIL: " + "; ".join(problems)))
