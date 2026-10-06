"""Makes the manual's script-taken screenshots from the pictures the DiagFeel.ManualShots* diagnostics save
(GameFeelDev/Saved/FeelKit/Manual): crops each to the part the text explains, then adds numbered markers, a spotlight
(the part that matters stays bright inside a green outline, the rest is dimmed) or a before/after pair.
Writes Docs/Manual/Screenshots/<ShotID>.png. Run after the diagnostics: python prepare_shots.py
The diagnostics save pictures at DENSITY times the screen's pixels (FeelManualCapture.h). All coordinates below are in
screen pixels (the layout as it appears on a 1920x1080 screen) and are multiplied by DENSITY here. A game view is two
files, <name>_scene.png and <name>_ui.png (the interface with transparency); the source name "<name>.game" stands for
the two put together. The saved pictures carry DENSITY in their resolution tag, which the manual build reads to print
them at the size of a screen picture.
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SRC = Path("B:/NewUE5Project/GameFeelDev/Saved/FeelKit/Manual")
DST = Path("B:/NewUE5Project/Docs/Manual/Screenshots")
GREEN = (46, 139, 87)
WHITE = (255, 255, 255)
DENSITY = 2
PPI = 165 * DENSITY
LABEL_FONT = ImageFont.truetype("C:/Windows/Fonts/seguisb.ttf", 24 * DENSITY)

# Shot ID: source, crop box, and any of: markers [(n, x, y)], spotlight [(x0, y0, x1, y1)]
SHOTS = {
    "S02-01": dict(src="Editor_HeavyHit.png", crop=(8, 690, 1370, 1022),
                   markers=[(1, 262, 856), (2, 1225, 856), (3, 590, 926), (4, 462, 752)]),
    "S02-02": dict(src="Editor_ScalableHit.png", crop=(8, 540, 1366, 1016)),
    "S04-01": dict(src="QS_ContentBrowser.png", crop=(0, 0, 1010, 560),
                   markers=[(1, 210, 529), (2, 690, 146)]),
    "S04-02": dict(src="QS_LibraryRecipe.png", crop=(8, 452, 1368, 694),
                   spotlight=[(10, 494, 1366, 528)]),
    "S04-03": dict(src="Editor_HeavyHitRest.png", crop=(8, 128, 1370, 800),
                   spotlight=[(14, 690, 292, 730), (330, 742, 1366, 770)]),
    "S04-04": dict(src="QS_Blueprint.png", crop=(300, 330, 1440, 850)),
    "S04-06": dict(src="QS_SwitchStart.game", crop=(600, 0, 1899, 600),
                   spotlight=[(691, 262, 1208, 430), (1645, 22, 1877, 85)]),
    "S04-07": dict(src="QS_SwitchOff.game", crop=(1180, 0, 1899, 112),
                   spotlight=[(1656, 22, 1877, 52)]),
    # DiagFeel.ManualShotsGuide (chapters 3 and 5) and DiagFeel.ManualShots
    "S01-01": dict(src="Editor_HeavyHitRest.png", crop=(0, 120, 1906, 1020)),
    "S03-02": dict(src="Guide_Plugins.png", crop=(404, 128, 1500, 410)),
    "S03-03": dict(src="Guide_FeelKitContent.png", crop=(298, 40, 1504, 290)),
    "S05-01": dict(src="Editor_ScalableHit.png", crop=(0, 120, 1906, 1020),
                   markers=[(1, 232, 143), (2, 232, 472), (3, 232, 711), (4, 1592, 143)]),
    "S05-02": dict(src="Guide_TrackSelected.png", crop=(8, 684, 1368, 724)),
    "S05-03": dict(src="Guide_TrackSelected.png", crop=(8, 735, 1368, 1000),
                   spotlight=[(10, 830, 1292, 940)]),
    "S05-04": dict(src="Guide_TrackSelected.png", crop=(1368, 205, 1885, 520)),
    "S05-05": dict(src="Guide_Sustain.png", crop=(8, 684, 1368, 762),
                   spotlight=[(10, 730, 470, 757)]),
    # Five tracks and the On Full Release row; the sustain region (0.35 to 0.95 s) on the ruler; the release row.
    "S05-06": dict(src="Guide_Sustain.png", crop=(8, 684, 1368, 1012),
                   spotlight=[(217, 690, 312, 717), (545, 775, 925, 800), (10, 979, 1295, 1006)]),
    "S05-07": dict(src="Editor_HeavyHitRest.png", crop=(8, 740, 1368, 1015),
                   spotlight=[(332, 946, 1297, 977)]),
    "S05-08": dict(src="Guide_ComfortMenu.png", crop=(340, 675, 1368, 900),
                   spotlight=[(1030, 727, 1226, 878), (547, 690, 693, 717)]),
    "S05-09": dict(src="Guide_Validation.png", crop=(28, 88, 1240, 160)),
    # DiagFeel.ManualShotsPlaying (chapter 6)
    "S06-01": dict(src="Play_Targets.png", crop=(615, 350, 2460, 825)),
    "S06-02": dict(src="Play_Context.png", crop=(720, 320, 2360, 860)),
    "S06-03": dict(src="Play_Handles.png", crop=(530, 330, 2560, 850)),
    "S06-04": dict(src="Play_Wait.png", crop=(920, 370, 2160, 800)),
    "S06-05": dict(src="Play_FeelMap.png", crop=(4, 95, 1000, 1040)),
    "S06-06": dict(src="Play_ProjectFeelMaps.png", crop=(460, 130, 1500, 510)),
    "S06-07": dict(src="Play_Montage.png", crop=(262, 440, 925, 645)),
    "S06-08": dict(src="Play_Trigger.png", crop=(4, 85, 1100, 690)),
    "S06-09": dict(src="Play_Input.png", crop=(4, 85, 1100, 585)),
    "S06-10": dict(src="Play_Switch.png", crop=(4, 95, 1090, 685)),
    # DiagFeel.ManualShotsLibrary (chapter 9)
    "S09-01": dict(src="Library_Browser.png", crop=(0, 32, 1906, 1066)),
    # DiagFeel.ManualShotsVariation (chapter 7)
    "S07-01": dict(src="Var_Parameters.png", crop=(0, 560, 1900, 962),
                   spotlight=[(8, 734, 526, 761), (1372, 588, 1876, 860), (1372, 928, 1876, 960)]),
    "S07-02": dict(src="Ref_ProjectSettings1.png", crop=(478, 546, 1482, 752)),
    "S07-03": dict(src="Var_ChoiceOptions.png", crop=(4, 100, 1000, 480)),
    # DiagFeel.ManualShotsDebugging (chapter 10)
    "S10-01": dict(src="Debug_Debugger.png", crop=(8, 84, 1928, 630)),
    "S10-02": dict(src="Debug_Replay.png", crop=(8, 686, 1366, 1015),
                   spotlight=[(742, 690, 992, 720), (900, 803, 1004, 827), (334, 838, 522, 861), (334, 908, 563, 931)]),
    # DiagFeel.ManualShotsNetwork (chapter 11); S11-02 reuses the Project Settings picture of chapter 18
    "S11-01": dict(src="Net_Graph.png", crop=(906, 306, 2172, 872)),
    "S11-02": dict(src="Ref_ProjectSettings1.png", crop=(478, 448, 1482, 553),
                   spotlight=[(482, 514, 1478, 546)]),
    # Chapter 13: 3840 x 2160 scene pictures from Tools/Run/run_gallery.ps1, copied to the manual folder as Demo_*.png;
    # scaled to 2400 px wide, enough for the page width
    "S13-01": dict(src="Demo_ARPG.png", crop=(0, 0, 1920, 1080), width=2400),
    "S13-02": dict(src="Demo_Platformer.png", crop=(0, 0, 1920, 1080), width=2400),
    "S13-03": dict(src="Demo_Shooter.png", crop=(0, 0, 1920, 1080), width=2400),
    "S13-04": dict(src="Demo_Horror.png", crop=(0, 0, 1920, 1080), width=2400),
    # Tools/Run/manual_concepts_shot.ps1 (chapter 2)
    "S02-03": dict(src="Concepts_FeelMap.png", crop=(5, 82, 1000, 915),
                   spotlight=[(416, 388, 506, 412), (416, 584, 506, 608)]),
    # DiagFeel.ManualShotsComfort (chapter 8)
    "S08-01": dict(src="Comfort_Nodes.png", crop=(655, 345, 2420, 830)),
    "S08-02": dict(src="Comfort_Essential.png", crop=(1368, 205, 1897, 915),
                   spotlight=[(1372, 725, 1893, 853)]),
    "S08-03": dict(src="Comfort_Menu.game", crop=(620, 60, 1255, 770)),
    "S08-04": dict(src="Comfort_Audit.png", crop=(20, 100, 1900, 455)),
    # DiagFeel.ManualShotsReference
    "S16-01": dict(src="Ref_TrackMenu.png", crop=(8, 128, 1370, 1020),
                   spotlight=[(38, 140, 386, 588), (14, 738, 114, 765)]),
    "S16-02": dict(src="Ref_NoPreview.png", crop=(8, 650, 1366, 840),
                   spotlight=[(330, 766, 1292, 797)]),
    "S17-01": dict(src="Ref_BlueprintMenu.png", crop=(300, 128, 1430, 860),
                   spotlight=[(772, 318, 1282, 828)]),
    "S18-01": dict(src="Ref_ProjectSettings1.png", crop=(464, 140, 1484, 826)),
    "S18-02": dict(src="Ref_ProjectSettings2.png", crop=(464, 140, 1484, 1045)),
    "S18-03": dict(src="Ref_ProjectSettings3.png", crop=(464, 140, 1484, 1045)),
    "S18-04": dict(src="Ref_EditorPreferences.png", crop=(406, 140, 1484, 400)),
    # Read from the desktop at the screen's pixels, with the debug text drawn at twice its size.
    "S19-01": dict(src="Ref_ShowDebug.png", crop=(0, 368, 1500, 545), density=1),
}

# Before/after pairs: ID, (source, label) twice, shared crop box.
PAIRS = {
    "S04-05": ((("QS_SwitchOff_scene.png", "Before the hit"), ("QS_Hit_scene.png", "0.07 s after pressing 1")), (556, 130, 1346, 890)),
}


def load(name: str) -> Image.Image:
    """A diagnostic picture; "<name>.game" is the scene with the interface layer on top."""
    if not name.endswith(".game"):
        return Image.open(SRC / name).convert("RGB")
    base = name[:-len(".game")]
    scene = Image.open(SRC / f"{base}_scene.png").convert("RGBA")
    ui = Image.open(SRC / f"{base}_ui.png").convert("RGBA")
    if ui.size != scene.size:
        ui = ui.resize(scene.size, Image.LANCZOS)
    return Image.alpha_composite(scene, ui).convert("RGB")


def sources(spec) -> list[str]:
    name = spec["src"]
    if name.endswith(".game"):
        base = name[:-len(".game")]
        return [f"{base}_scene.png", f"{base}_ui.png"]
    return [name]


def scaled(box, k=DENSITY):
    return tuple(v * k for v in box)


def marker(d: ImageDraw.ImageDraw, n: int, x: float, y: float, k: int = DENSITY) -> None:
    r = 18 * k
    ring = 3 * k
    d.ellipse((x - r - ring, y - r - ring, x + r + ring, y + r + ring), fill=WHITE)
    d.ellipse((x - r, y - r, x + r, y + r), fill=GREEN)
    font = ImageFont.truetype("C:/Windows/Fonts/segoeuib.ttf", 22 * k)
    w = d.textlength(str(n), font=font)
    d.text((x - w / 2, y - 15 * k), str(n), font=font, fill=WHITE)


def spotlight(img: Image.Image, boxes, ox: int, oy: int, k: int = DENSITY) -> Image.Image:
    """Dims everything outside the boxes and outlines each box in green."""
    shade = Image.new("L", img.size, 105)
    d = ImageDraw.Draw(shade)
    pad, radius = 6 * k, 10 * k
    for x0, y0, x1, y1 in boxes:
        d.rounded_rectangle((x0 - ox - pad, y0 - oy - pad, x1 - ox + pad, y1 - oy + pad), radius, fill=0)
    dark = Image.new("RGB", img.size, (10, 12, 16))
    out = Image.composite(dark, img, shade)
    draw = ImageDraw.Draw(out)
    for x0, y0, x1, y1 in boxes:
        draw.rounded_rectangle((x0 - ox - pad, y0 - oy - pad, x1 - ox + pad, y1 - oy + pad), radius, outline=GREEN,
                               width=3 * k)
    return out


def make(shot_id: str, spec: dict) -> Image.Image:
    k = spec.get("density", DENSITY)
    box = scaled(spec["crop"], k)
    img = load(spec["src"]).crop(box)
    if spec.get("spotlight"):
        img = spotlight(img, [scaled(b, k) for b in spec["spotlight"]], box[0], box[1], k)
    d = ImageDraw.Draw(img)
    for n, x, y in spec.get("markers", []):
        marker(d, n, x * k - box[0], y * k - box[1], k)
    return img


def pair(items, box) -> Image.Image:
    frames = [load(name).crop(scaled(box)) for name, _ in items]
    w, h = frames[0].size
    gap, label_h = 24 * DENSITY, 46 * DENSITY
    out = Image.new("RGB", (w * 2 + gap, h + label_h), WHITE)
    d = ImageDraw.Draw(out)
    for i, (frame, (_, label)) in enumerate(zip(frames, items)):
        x = i * (w + gap)
        out.paste(frame, (x, 0))
        tw = d.textlength(label, font=LABEL_FONT)
        d.text((x + (w - tw) / 2, h + 10 * DENSITY), label, font=LABEL_FONT, fill=(90, 98, 110))
    return out


def save_checked(img: Image.Image, path: Path, ppi: float) -> None:
    """Saves outside the synced folder, checks the file, copies it in and checks the copy. Writing a large PNG straight
    into Docs/ was twice left broken by the folder's sync client (2026-09-26)."""
    import shutil
    import tempfile
    for attempt in range(3):
        with tempfile.TemporaryDirectory() as folder:
            temp = Path(folder) / path.name
            img.save(temp, optimize=True, dpi=(ppi, ppi))
            Image.open(temp).load()
            shutil.copyfile(temp, path)
        try:
            Image.open(path).load()
            return
        except Exception:
            continue
    raise SystemExit(f"{path} could not be written intact")


if __name__ == "__main__":
    for shot_id, spec in SHOTS.items():
        if all((SRC / name).exists() for name in sources(spec)):
            img = make(shot_id, spec)
            ppi = 165 * spec.get("density", DENSITY)
            if spec.get("width") and img.width > spec["width"]:
                # Photographs: fewer pixels, same size on the page.
                ppi = ppi * spec["width"] / img.width
                img = img.resize((spec["width"], round(img.height * spec["width"] / img.width)), Image.LANCZOS)
            save_checked(img, DST / f"{shot_id}.png", ppi)
            print(shot_id, img.size)
    for shot_id, (items, box) in PAIRS.items():
        if all((SRC / name).exists() for name, _ in items):
            img = pair(items, box)
            save_checked(img, DST / f"{shot_id}.png", PPI)
            print(shot_id, img.size)
