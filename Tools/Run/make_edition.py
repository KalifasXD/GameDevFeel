"""Turns a staged copy of the FeelKit plugin into the shipping source of one edition.

    python make_edition.py --edition Lite --stage B:/NewUE5Project/Build/Stage/Lite_56/FeelKit
    python make_edition.py --edition Pro  --stage B:/NewUE5Project/Build/Stage/Pro_56/FeelKit

The development source marks code that only FeelKit Pro contains:
    // FEELKIT_PRO_BEGIN ... // FEELKIT_PRO_END   a Pro-only region
    // FEELKIT_LITE: <code>                        a line that replaces a Pro region in Lite
Pro keeps every region and drops the marker lines. Lite drops the regions, turns the Lite lines into code, removes the
Pro-only files, modules, content and the GAS add-on, keeps the library recipes that use only Lite steps, and hides the
Pro-only fields from the editor while keeping them in the data, so recipes open unchanged in either edition.
Called by package_plugin.ps1; run on a copy only, never on the development plugin.
"""
import argparse
import json
import re
import shutil
import sys
from pathlib import Path

BEGIN, END, LITE = "// FEELKIT_PRO_BEGIN", "// FEELKIT_PRO_END", "// FEELKIT_LITE: "

# Files and folders (relative to the plugin root) that only FeelKit Pro contains.
LITE_REMOVE = [
    "Source/FeelNiagara",
    "Source/FeelEnhancedInput",
    "Extras",
    "Demos",
    "Content/Demos",
] + [
    f"Source/FeelCore/{side}/Steps/FeelStep_{name}.{ext}"
    for name in ("ActorExtras", "AudioMix", "CameraExtras", "ChromaticAberration", "HitFlash", "MaterialPulse",
                 "ScreenColor", "Spawn", "Widget")
    for side, ext in (("Public", "h"), ("Private", "cpp"))
] + [
    f"Source/FeelCore/{side}/{name}.{ext}"
    for name in ("FeelAnimNotifies", "FeelMap", "FeelPlayAndWaitAction", "FeelReplicationComponent", "FeelTriggerComponent")
    for side, ext in (("Public", "h"), ("Private", "cpp"))
] + [
    f"Source/FeelEditor/Private/{name}.{ext}"
    for name in ("FeelAudioAnalysis", "FeelComfortAudit", "FeelGraphPinFactory", "FeelMapAssetTypes", "FeelRecipeFilter",
                 "FeelRecipeJson", "SFeelDebugger", "SFeelRecipeBrowser")
    for ext in ("h", "cpp")
]

# Library recipes that use only the steps of FeelKit Lite and none of its Pro-only fields.
LITE_LIBRARY = [
    "Danger/FR_Danger_DamageTaken",
    "Denial/FR_Denial_Blocked",
    "Denial/FR_Denial_Locked",
    "Dread/FR_Dread_JumpScare",
    "Impact/FR_Impact_HeavyHit",
    "Impact/FR_Impact_LightHit",
    "Reward/FR_Reward_KillConfirm",
    "Reward/FR_Reward_Pickup",
    "Speed/FR_Speed_SprintStart",
    "Weight/FR_Weight_HeavyFootstep",
    "Weight/FR_Weight_Stomp",
]

# Sample assets the Lite content uses (the library recipes above and the comfort menu previews); the other samples serve
# Pro steps and the demo recipes. check_packages.ps1 reports sample assets nothing in a package uses.
LITE_SAMPLES = ["S_FK_Alarm", "S_FK_Denied", "S_FK_Hit_Crit", "S_FK_Hit_Heavy", "S_FK_Hit_Light", "S_FK_Land_Thud",
                "S_FK_Pickup"]

# Steps whose classes the evaluator core is built around: kept in Lite, hidden from every class picker.
LITE_HIDDEN_CLASSES = [("Source/FeelCore/Public/Steps/FeelStep_Meta.h", "UFeelStep_Recipe"),
                       ("Source/FeelCore/Public/Steps/FeelStep_Meta.h", "UFeelStep_RandomChoice")]

# Pro-only structures to keep out of Lite Blueprints. FFeelPlayContext stays: Blueprint steps, which Lite supports, read
# it through the step context.
LITE_HIDDEN_STRUCTS = []

# Pro-only fields: kept in the data, not shown in the Lite editor or to Lite Blueprints.
LITE_HIDDEN_FIELDS = {
    "Source/FeelCore/Public/FeelRecipe.h": ["Parameters", "bSustain", "SustainStart", "SustainEnd", "bJumpToEndOnRelease",
                                            "ReleaseParameter", "ReleaseAt", "FullReleaseRecipe", "EarlyReleaseRecipe",
                                            "Feeling", "Genres", "Description", "BasedOn"],
    "Source/FeelCore/Public/FeelTrack.h": ["AppliesTo", "ParameterMappings", "RandomIntensity", "RandomDurationScale",
                                           "Conditions"],
    "Source/FeelCore/Public/FeelSettings.h": ["bAllowGlobalTimeDilationInMultiplayer", "Accumulators"],
}
EDIT_SPECIFIERS = ("EditAnywhere", "EditDefaultsOnly", "EditInstanceOnly", "VisibleAnywhere", "BlueprintReadWrite",
                   "BlueprintReadOnly")


def strip_markers(text: str, lite: bool, where: str) -> str:
    out, depth = [], 0
    for line in text.split("\n"):
        stripped = line.strip()
        if stripped == BEGIN:
            if depth:
                raise SystemExit(f"nested {BEGIN} in {where}")
            depth = 1
            continue
        if stripped == END:
            if not depth:
                raise SystemExit(f"{END} without {BEGIN} in {where}")
            depth = 0
            continue
        if stripped.startswith(LITE):
            if lite:
                indent = line[: len(line) - len(line.lstrip())]
                out.append(indent + stripped[len(LITE):])
            continue
        if depth and lite:
            continue
        out.append(line)
    if depth:
        raise SystemExit(f"{BEGIN} without {END} in {where}")
    text = "\n".join(out)
    # A removed region can leave two blank lines behind.
    return re.sub(r"\n{3,}(?=\t*\S)", "\n\n", text) if lite else text


def hide_field(text: str, field: str, where: str) -> str:
    """Removes the edit and Blueprint specifiers from the UPROPERTY of `field`."""
    decl = re.search(r"^[ \t]*[\w<>:, ]+?\b" + re.escape(field) + r"\b\s*(?:=[^;]*|\{[^;]*\})?;", text, re.M)
    if not decl:
        raise SystemExit(f"field {field} not found in {where}")
    head = text[: decl.start()]
    prop = head.rfind("UPROPERTY(")
    if prop < 0 or head.count("UPROPERTY(", prop) != 1:
        raise SystemExit(f"no UPROPERTY for {field} in {where}")
    close = text.index("\n", prop)
    line = text[prop:close]
    for spec in EDIT_SPECIFIERS:
        line = re.sub(r"\b" + spec + r"\b\s*,\s*", "", line)
        line = re.sub(r",\s*\b" + spec + r"\b", "", line)
        line = re.sub(r"\(\s*" + spec + r"\s*\)", "()", line)
    # A category on a field the editor does not show is an error for Unreal's header tool.
    line = re.sub(r"\bCategory\s*=\s*\"[^\"]*\"\s*,\s*", "", line)
    line = re.sub(r",\s*\bCategory\s*=\s*\"[^\"]*\"", "", line)
    line = re.sub(r"\(\s*Category\s*=\s*\"[^\"]*\"\s*\)", "()", line)
    return text[:prop] + line + text[close:]


def hide_class(text: str, cls: str, where: str) -> str:
    """Adds HideDropdown to the UCLASS of `cls`."""
    at = text.index(f"class FEELCORE_API {cls}")
    ucls = text.rfind("UCLASS(", 0, at)
    if ucls < 0:
        raise SystemExit(f"no UCLASS for {cls} in {where}")
    return text[:ucls] + "UCLASS(HideDropdown, " + text[ucls + len("UCLASS("):] if not text.startswith("UCLASS()", ucls) \
        else text[:ucls] + "UCLASS(HideDropdown)" + text[ucls + len("UCLASS()"):]


def hide_struct(text: str, struct: str, where: str) -> str:
    """Removes BlueprintType from the USTRUCT of `struct`."""
    at = text.index(f"struct FEELCORE_API {struct}")
    ustruct = text.rfind("USTRUCT(BlueprintType)", 0, at)
    if ustruct < 0 or text.count("USTRUCT(", ustruct, at) != 1:
        raise SystemExit(f"no USTRUCT(BlueprintType) for {struct} in {where}")
    return text[:ustruct] + "USTRUCT()" + text[ustruct + len("USTRUCT(BlueprintType)"):]


def write_lite_credits(path: Path) -> None:
    """Credits for the samples Lite ships: the rows of those sounds from the full credits, nothing else."""
    rows = [line for line in path.read_text(encoding="utf-8").split("\n")
            if line.startswith("| `") and line.split("`")[1] in LITE_SAMPLES]
    if len(rows) != len(LITE_SAMPLES):
        raise SystemExit(f"Credits.md lists {len(rows)} of the {len(LITE_SAMPLES)} Lite sample sounds")
    text = "\n".join([
        "# FeelKit Lite sample content credits",
        "",
        "FeelKit Lite ships a few sample sounds so the library recipes play with sound as soon as it is installed. They come from Kenney's audio",
        "packs and are released under **CC0 (public domain)**: they can be used, changed and shipped in a commercial",
        "product, and no credit is required. They are credited anyway, because the work deserves it.",
        "",
        "| Asset in FeelKit | Source pack | Author | License |",
        "|---|---|---|---|",
        *rows,
        "",
        "License text: <https://creativecommons.org/publicdomain/zero/1.0/>",
        "",
        "## Your own project",
        "",
        "Nothing here has to stay. Every library recipe you copy into your project points at these samples, and you can swap",
        "in your own sounds without changing anything else.",
        "",
    ])
    path.write_text(text, encoding="utf-8")


def make(edition: str, stage: Path) -> None:
    lite = edition == "Lite"
    if not (stage / "FeelKit.uplugin").exists():
        raise SystemExit(f"no FeelKit.uplugin in {stage}")

    if lite:
        for rel in LITE_REMOVE:
            target = stage / rel
            if target.is_dir():
                shutil.rmtree(target)
            elif target.exists():
                target.unlink()
        keep = {name.split("/")[-1] for name in LITE_LIBRARY}
        for asset in (stage / "Content" / "Library").rglob("*.uasset"):
            if asset.stem not in keep:
                asset.unlink()
        # The recipes' JSON sources serve the JSON import, which only Pro has.
        shutil.rmtree(stage / "Library")
        for asset in (stage / "Content" / "Samples").rglob("*.uasset"):
            if asset.stem not in LITE_SAMPLES:
                asset.unlink()
        write_lite_credits(stage / "Credits.md")
        for base in ("Library", "Samples"):
            for folder in sorted((p for p in (stage / "Content" / base).rglob("*") if p.is_dir()), reverse=True):
                if not any(folder.iterdir()):
                    folder.rmdir()

    for path in list(stage.rglob("*")):
        if path.suffix in (".h", ".cpp", ".cs") and path.is_file():
            text = path.read_text(encoding="utf-8")
            new = strip_markers(text, lite, str(path))
            if new != text:
                path.write_text(new, encoding="utf-8")

    if lite:
        for rel, fields in LITE_HIDDEN_FIELDS.items():
            path = stage / rel
            text = path.read_text(encoding="utf-8")
            for field in fields:
                text = hide_field(text, field, rel)
            path.write_text(text, encoding="utf-8")
        for rel, struct in LITE_HIDDEN_STRUCTS:
            path = stage / rel
            path.write_text(hide_struct(path.read_text(encoding="utf-8"), struct, rel), encoding="utf-8")
        for rel, cls in LITE_HIDDEN_CLASSES:
            path = stage / rel
            path.write_text(hide_class(path.read_text(encoding="utf-8"), cls, rel), encoding="utf-8")

        descriptor = stage / "FeelKit.uplugin"
        data = json.loads(descriptor.read_text(encoding="utf-8"))
        data["FriendlyName"] = "FeelKit Lite"
        data["Description"] = ("Game feel framework, free edition: author Recipes of timed feedback Tracks on a timeline "
                               "with instant preview, and give players comfort settings.")
        data["Modules"] = [m for m in data["Modules"] if m["Name"] in ("FeelCore", "FeelEditor")]
        data.pop("Plugins", None)
        descriptor.write_text(json.dumps(data, indent="\t") + "\n", encoding="utf-8")

        filter_ini = stage / "Config" / "FilterPlugin.ini"
        lines = filter_ini.read_text(encoding="utf-8").split("\n")
        filter_ini.write_text("\n".join(l for l in lines if l.strip() not in ("/Demos/...", "/Extras/...", "/Library/...")), encoding="utf-8")

    # Nothing may remain of the markers in either edition.
    for path in stage.rglob("*"):
        if path.is_file() and path.suffix in (".h", ".cpp", ".cs"):
            text = path.read_text(encoding="utf-8")
            if "FEELKIT_PRO_" in text or "FEELKIT_LITE:" in text:
                raise SystemExit(f"marker left in {path}")
    print(f"EDITION {edition} ready in {stage}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--edition", choices=("Lite", "Pro"), required=True)
    parser.add_argument("--stage", type=Path, required=True)
    args = parser.parse_args()
    make(args.edition, args.stage)
    sys.exit(0)
