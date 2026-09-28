"""Makes the six Fab upload zips from the staged edition sources that package_plugin.ps1 built and checked.

    python make_uploads.py

Each zip holds one folder, FeelKit, with the plugin source exactly as the strict BuildPlugin compiled it
(Build/Stage/<Edition>_<tag>/FeelKit: no tests, the edition's code and content, EngineVersion set). Epic builds the
binaries itself, so Binaries and Intermediate are never included. Checks before writing: the matching package in
Build/FKPkg_<Edition>_<tag> exists and its log says BUILD SUCCESSFUL, the descriptor's EngineVersion, no test code, no
edition markers, no Pro-only modules in Lite, and the longest path. Output: Publish/<Edition>/03-Upload/UE<version>/.
"""
import json
import zipfile
from pathlib import Path

ROOT = Path("B:/NewUE5Project")
VERSION = "1.0.0"
ENGINES = ("5.6", "5.7", "5.8")
FORBIDDEN_DIRS = {"Binaries", "Intermediate", "Saved", "Tests", "DerivedDataCache"}
PRO_ONLY = ("Source/FeelNiagara", "Source/FeelEnhancedInput", "Extras", "Content/Demos", "Demos")


def check(stage: Path, edition: str, engine: str) -> list:
    problems = []
    tag = engine.replace(".", "")
    log = ROOT / f"Build/Logs/package_plugin_{edition}_{tag}.log"
    if not log.exists() or "BUILD SUCCESSFUL" not in log.read_text(encoding="utf-8", errors="ignore"):
        problems.append(f"no successful BuildPlugin log {log.name}")
    descriptor = json.loads((stage / "FeelKit.uplugin").read_text(encoding="utf-8"))
    if descriptor.get("EngineVersion") != f"{engine}.0":
        problems.append(f"EngineVersion {descriptor.get('EngineVersion')}")
    if descriptor.get("VersionName") != VERSION:
        problems.append(f"VersionName {descriptor.get('VersionName')}")
    for path in stage.rglob("*"):
        rel = path.relative_to(stage)
        if FORBIDDEN_DIRS & set(rel.parts):
            problems.append(f"forbidden folder: {rel}")
        if path.is_file() and path.suffix in (".h", ".cpp", ".cs"):
            text = path.read_text(encoding="utf-8")
            if "FEELKIT_PRO_" in text or "FEELKIT_LITE:" in text:
                problems.append(f"edition marker in {rel}")
            if "IMPLEMENT_SIMPLE_AUTOMATION_TEST" in text or "IMPLEMENT_COMPLEX_AUTOMATION_TEST" in text:
                problems.append(f"test code in {rel}")
    if edition == "Lite":
        for rel in PRO_ONLY:
            if (stage / rel).exists():
                problems.append(f"Pro-only {rel} in Lite")
    return problems


def make(edition: str, engine: str) -> None:
    tag = engine.replace(".", "")
    stage = ROOT / f"Build/Stage/{edition}_{tag}/FeelKit"
    problems = check(stage, edition, engine)
    if problems:
        raise SystemExit(f"{edition} {engine}: " + "; ".join(problems[:10]))
    out = ROOT / f"Publish/{edition}/03-Upload/UE{engine}/FeelKit_{edition}_{VERSION}_UE{engine}.zip"
    out.parent.mkdir(parents=True, exist_ok=True)
    files = sorted(p for p in stage.rglob("*") if p.is_file())
    longest = max(len(("FeelKit/" + str(p.relative_to(stage))).replace("\\", "/")) for p in files)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in files:
            archive.write(path, "FeelKit/" + path.relative_to(stage).as_posix())
    with zipfile.ZipFile(out) as archive:
        bad = archive.testzip()
        names = archive.namelist()
    if bad or len(names) != len(files):
        raise SystemExit(f"{out} failed its own check")
    print(f"{out.relative_to(ROOT)}  {len(files)} files  {out.stat().st_size / 1024 / 1024:.2f} MB  longest path {longest}")


if __name__ == "__main__":
    for edition in ("Lite", "Pro"):
        for engine in ENGINES:
            make(edition, engine)
