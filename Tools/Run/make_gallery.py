"""Builds the Fab gallery images of both editions from real captures.

    python make_gallery.py

Every picture frames real FeelKit captures on a plain dark background with one headline and one line of text:
  - editor pictures (2x density) from Saved/FeelKit/Manual of GameFeelDev (DiagFeel.GalleryEditorShots and the manual
    diagnostics) for Pro, and from Build/LiteGallery (Tools/Run/lite_gallery_shots.ps1) for Lite;
  - game pictures from Saved/FeelKit/Gallery of both demo projects (Tools/Run/run_gallery.ps1) and the comfort menu
    picture of FeelKit.Editor.ComfortMenuInPlay.
Writes Publish/<Edition>/01-Images/NN-Name.png (JPEG for photographs) at 1920 x 1080, each under 3 MB.
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path("B:/NewUE5Project")
MANUAL = ROOT / "GameFeelDev/Saved/FeelKit/Manual"
SAVED = ROOT / "GameFeelDev/Saved/FeelKit"
GALLERY_TP = ROOT / "GameFeelDev/Saved/FeelKit/Gallery"
GALLERY_FP = ROOT / "FeelDemoFP/Saved/FeelKit/Gallery"
LITE = ROOT / "Build/LiteGallery"
PUBLISH = ROOT / "Publish"

W, H = 1920, 1080
MARGIN = 72
PANEL_TOP = 236
PANEL_BOTTOM = H - 52
BG_TOP, BG_BOTTOM = (23, 26, 30), (13, 15, 18)
TEXT = (243, 245, 247)
MUTED = (163, 172, 182)
ACCENT = (82, 183, 136)
BORDER = (46, 52, 58)
FONTS = Path("C:/Windows/Fonts")


def font(name: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(FONTS / name), size)


SEMIBOLD = "seguisb.ttf"
REGULAR = "segoeui.ttf"


def background() -> Image.Image:
    image = Image.new("RGB", (W, H))
    draw = ImageDraw.Draw(image)
    for y in range(H):
        t = y / (H - 1)
        draw.line([(0, y), (W, y)], fill=tuple(round(a + (b - a) * t) for a, b in zip(BG_TOP, BG_BOTTOM)))
    return image


def spaced(draw: ImageDraw.ImageDraw, xy, text: str, fnt, fill, tracking: int) -> None:
    x, y = xy
    for char in text:
        draw.text((x, y), char, font=fnt, fill=fill)
        x += draw.textlength(char, font=fnt) + tracking


def header(image: Image.Image, edition: str, headline: str, line: str) -> None:
    draw = ImageDraw.Draw(image)
    spaced(draw, (MARGIN, 58), f"FEELKIT {edition.upper()}", font(SEMIBOLD, 21), ACCENT, 4)
    draw.text((MARGIN, 88), headline, font=font(SEMIBOLD, 52), fill=TEXT)
    small = font(REGULAR, 27)
    if draw.textlength(line, font=small) > W - 2 * MARGIN:
        raise SystemExit(f"line too long: {line}")
    draw.text((MARGIN, 164), line, font=small, fill=MUTED)


def rounded(picture: Image.Image, radius: int, corners=(True, True, True, True)) -> Image.Image:
    mask = Image.new("L", picture.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, picture.width - 1, picture.height - 1], radius, fill=255, corners=corners)
    out = picture.convert("RGBA")
    out.putalpha(mask)
    return out


def place(image: Image.Image, picture: Image.Image, box, radius: int = 12) -> None:
    """Pastes picture at box (x, y) with a border, rounded corners and a soft shadow."""
    x, y = box
    shadow = Image.new("RGBA", (picture.width + 80, picture.height + 80), (0, 0, 0, 0))
    ImageDraw.Draw(shadow).rounded_rectangle([40, 48, 40 + picture.width, 48 + picture.height], radius, fill=(0, 0, 0, 150))
    shadow = shadow.filter(ImageFilter.GaussianBlur(18))
    image.paste(shadow, (x - 40, y - 40), shadow)
    framed = Image.new("RGB", (picture.width + 4, picture.height + 4), BORDER)
    framed = rounded(framed, radius + 2)
    image.paste(framed, (x - 2, y - 2), framed)
    inner = rounded(picture.convert("RGB"), radius)
    image.paste(inner, (x, y), inner)


def fit(picture: Image.Image, max_w: int, max_h: int) -> Image.Image:
    scale = min(max_w / picture.width, max_h / picture.height)
    return picture.resize((round(picture.width * scale), round(picture.height * scale)), Image.LANCZOS)


def crop(path: Path, box=None) -> Image.Image:
    picture = Image.open(path).convert("RGB")
    return picture.crop(box) if box else picture


def panel(image: Image.Image, picture: Image.Image) -> None:
    """One picture, as large as the panel area allows, centered."""
    picture = fit(picture, W - 2 * MARGIN, PANEL_BOTTOM - PANEL_TOP)
    place(image, picture, ((W - picture.width) // 2, PANEL_TOP))


def panels(image: Image.Image, pictures, gap: int = 40) -> None:
    """Pictures side by side at one height, centered."""
    height = PANEL_BOTTOM - PANEL_TOP
    scaled = [p.resize((round(p.width * height / p.height), height), Image.LANCZOS) for p in pictures]
    total = sum(p.width for p in scaled) + gap * (len(scaled) - 1)
    if total > W - 2 * MARGIN:
        factor = (W - 2 * MARGIN) / total
        scaled = [p.resize((round(p.width * factor), round(p.height * factor)), Image.LANCZOS) for p in scaled]
        total = sum(p.width for p in scaled) + gap * (len(scaled) - 1)
    x = (W - total) // 2
    for p in scaled:
        place(image, p, (x, PANEL_TOP))
        x += p.width + gap


def hero(edition: str, editor: Path) -> Image.Image:
    image = background()
    draw = ImageDraw.Draw(image)
    # The recipe editor, preview and timeline, running off the right edge.
    shot = crop(editor, (16, 256, 2730, 2030))
    shot = fit(shot, 2000, 880)
    x, y = 880, 100
    visible = shot.crop((0, 0, min(shot.width, W - x + 1), shot.height))
    place(image, visible, (x, y), radius=14)
    # Cover the right border and shadow so the picture runs off the edge.
    image.paste(visible.crop((visible.width - 2, 0, visible.width, visible.height)), (W - 2, y))
    word = font(SEMIBOLD, 150)
    draw.text((88, 250), "FeelKit", font=word, fill=TEXT)
    ed_font = font(SEMIBOLD, 64)
    draw.text((96, 440), edition, font=ed_font, fill=ACCENT)
    if edition == "Lite":
        draw.text((96 + draw.textlength("Lite", font=ed_font) + 28, 466), "Free", font=font(REGULAR, 38), fill=MUTED)
    tag = font(REGULAR, 40)
    draw.text((96, 570), "Hitstop, screen shake and", font=tag, fill=(214, 220, 226))
    draw.text((96, 622), "game feel on a timeline", font=tag, fill=(214, 220, 226))
    draw.text((96, 674), "for Unreal Engine", font=tag, fill=(214, 220, 226))
    note = font(REGULAR, 26)
    draw.text((96, 792), "The editor preview runs the game's own code.", font=note, fill=MUTED)
    draw.text((96, 830), "Comfort settings for every player built in.", font=note, fill=MUTED)
    return image


def check(draw: ImageDraw.ImageDraw, x: int, y: int) -> None:
    draw.line([(x, y + 12), (x + 9, y + 21), (x + 26, y + 2)], fill=ACCENT, width=5, joint="curve")


def editions(edition: str) -> Image.Image:
    image = background()
    header(image, edition, "Lite and Pro", "Start free with Lite. Recipes made in Lite open unchanged in Pro.")
    draw = ImageDraw.Draw(image)
    rows = [
        ("Recipe editor: timeline, live preview, curves, scrubbing", True, True),
        ("Effects", "12", "37"),
        ("Library recipes", "11", "38 + Recipe Browser"),
        ("Play Feel from Blueprint and C++", True, True),
        ("Comfort settings, presets and the comfort menu", True, True),
        ("Feel Switch: feel off and on while playing", True, True),
        ("Parameters, accumulators, sustained recipes, Play Feel and Wait", False, True),
        ("Feel Maps and events, anim notifies, Feel Trigger, Enhanced Input", False, True),
        ("GAS add-on, multiplayer replication", False, True),
        ("Debugger, Recent Plays, GIF capture, Comfort Audit", False, True),
        ("Tracks from sound, JSON import and export", False, True),
    ]
    left, col_lite, col_pro, right = 180, 1200, 1440, 1740
    top, row_h = 280, 64
    head = font(SEMIBOLD, 30)
    body = font(REGULAR, 28)
    draw.text((col_lite, top - 58), "Lite", font=head, fill=TEXT)
    draw.text((col_pro, top - 58), "Pro", font=head, fill=TEXT)
    draw.line([(left, top - 8), (right, top - 8)], fill=ACCENT, width=2)
    for index, (label, lite, pro) in enumerate(rows):
        y = top + index * row_h
        draw.text((left, y + 12), label, font=body, fill=(214, 220, 226))
        for value, column in ((lite, col_lite), (pro, col_pro)):
            if value is True:
                check(draw, column + 4, y + 18)
            elif value is False:
                draw.line([(column + 4, y + 32), (column + 30, y + 32)], fill=(90, 98, 106), width=3)
            else:
                draw.text((column, y + 12), value, font=body, fill=TEXT)
        draw.line([(left, y + row_h - 1), (right, y + row_h - 1)], fill=(38, 43, 48), width=1)
    return image


def demos(edition: str) -> Image.Image:
    image = background()
    header(image, edition, "Four playable demos",
           "Action/RPG, platformer, shooter and horror levels built on Unreal's own templates. Links in the description.")
    cells = [
        (GALLERY_TP / "ARPG_On_1.png", "Action/RPG", "Weight Class"),
        (GALLERY_TP / "Platformer_Dash.png", "Platformer", "Bounce Feel"),
        (GALLERY_FP / "Shooter_Weapon1_0.png", "Shooter", "Every Bullet Has an Opinion"),
        (GALLERY_FP / "Horror_Sprint_0.png", "Horror", "Heartbeat"),
    ]
    gap = 28
    cell_h = (PANEL_BOTTOM - PANEL_TOP - gap) // 2
    cell_w = round(cell_h * 16 / 9)
    x0 = (W - (2 * cell_w + gap)) // 2
    for index, (path, genre, title) in enumerate(cells):
        picture = Image.open(path).convert("RGB").resize((cell_w, cell_h), Image.LANCZOS)
        # A dark band under the label, so it reads on any picture.
        band = Image.new("RGBA", (cell_w, 92), (0, 0, 0, 0))
        band_draw = ImageDraw.Draw(band)
        for row in range(92):
            band_draw.line([(0, row), (cell_w, row)], fill=(0, 0, 0, round(170 * row / 91)))
        picture.paste(band, (0, cell_h - 92), band)
        draw = ImageDraw.Draw(picture)
        draw.text((22, cell_h - 70), genre, font=font(SEMIBOLD, 30), fill=TEXT)
        draw.text((22 + draw.textlength(genre, font=font(SEMIBOLD, 30)) + 16, cell_h - 64), title, font=font(REGULAR, 24), fill=(205, 212, 219))
        place(image, picture, (x0 + (index % 2) * (cell_w + gap), PANEL_TOP + (index // 2) * (cell_h + gap)), radius=10)
    return image


def framed(edition: str, headline: str, line: str, pictures) -> Image.Image:
    image = background()
    header(image, edition, headline, line)
    if len(pictures) == 1:
        panel(image, pictures[0])
    else:
        panels(image, pictures)
    return image


def comfort_menu() -> Image.Image:
    # The play window as the screen showed it: the title bar off, the menu and some of the level around it.
    return crop(SAVED / "ComfortMenu_0_open.png", (300, 110, 1607, 960))


def blueprint() -> Image.Image:
    return crop(MANUAL / "QS_Blueprint.png", (592, 330, 2860, 2030))


def save(image: Image.Image, folder: Path, name: str, jpeg: bool = False) -> Path:
    folder.mkdir(parents=True, exist_ok=True)
    path = folder / (name + (".jpg" if jpeg else ".png"))
    if jpeg:
        image.save(path, quality=90, optimize=True, subsampling=0)
    else:
        image.save(path, optimize=True)
    size = path.stat().st_size
    if size >= 3 * 1024 * 1024:
        raise SystemExit(f"{path} is {size / 1024 / 1024:.2f} MB")
    print(f"{path.relative_to(ROOT)}  {image.size[0]}x{image.size[1]}  {size / 1024 / 1024:.2f} MB")
    return path


def pro() -> None:
    out = PUBLISH / "Pro" / "01-Images"
    save(hero("Pro", MANUAL / "Gallery_Editor.png"), out, "01-Main")
    save(framed("Pro", "Build the whole moment on one timeline",
                "Hitstop, shake, camera punch, flash, sound and rumble as tracks. Scrub, loop and tune while the preview plays.",
                [crop(MANUAL / "Gallery_Editor2.png")]), out, "02-Recipe-Editor")
    save(demos("Pro"), out, "03-Demos", jpeg=True)
    save(framed("Pro", "One node plays a recipe",
                "Play Feel on an actor, a component, a widget, a location or the player's camera.",
                [blueprint()]), out, "04-Blueprint")
    save(framed("Pro", "38 recipes to start from",
                "Browse by feeling and genre, hover a tile to preview it, copy it into your project.",
                [crop(MANUAL / "Library_Browser.png")]), out, "05-Library")
    save(framed("Pro", "Scale the response with gameplay values",
                "Map a parameter such as Damage to any track, then try values with the slider in the editor.",
                [crop(MANUAL / "Var_Parameters.png")]), out, "06-Parameters")
    save(framed("Pro", "Hook it up without Blueprint wiring",
                "Feel Maps turn gameplay events into recipes. Feel Trigger plays them on jumps, landings, hits and overlaps.",
                [crop(MANUAL / "Play_FeelMap.png"), crop(MANUAL / "Play_Trigger.png")]), out, "07-Triggers")
    save(framed("Pro", "Comfort settings for every player",
                "A ready menu for shake, camera motion, flashes, hitstop, distortion and vibration, saved per player.",
                [comfort_menu()]), out, "08-Comfort-Menu")
    save(framed("Pro", "Turn an effect off, keep the information",
                "Essential tracks play a substitute, such as a vignette in place of a flash, when a player turns that effect off.",
                [crop(MANUAL / "Gallery_Essential.png")]), out, "09-Essential-Tracks")
    save(editions("Pro"), out, "10-Editions")


def lite() -> None:
    out = PUBLISH / "Lite" / "01-Images"
    save(hero("Lite", LITE / "Gallery_Editor.png"), out, "01-Main")
    save(framed("Lite", "Build the whole moment on one timeline",
                "Hitstop, shake, camera punch, flash, sound and rumble as tracks. Scrub, loop and tune while the preview plays.",
                [crop(LITE / "Gallery_Editor2.png")]), out, "02-Recipe-Editor")
    save(framed("Lite", "One node plays a recipe",
                "Play Feel on an actor, a component, a widget, a location or the player's camera.",
                [blueprint()]), out, "03-Blueprint")
    save(framed("Lite", "11 recipes to start from",
                "Hits, pickups, footsteps, denials and a jump scare, ready to play or to copy into your project.",
                [crop(LITE / "Gallery_ContentBrowser.png", (0, 0, 3212, 1140))]), out, "04-Library")
    save(framed("Lite", "Comfort settings for every player",
                "A ready menu for shake, camera motion, flashes, hitstop, distortion and vibration, saved per player.",
                [comfort_menu()]), out, "05-Comfort-Menu")
    save(framed("Lite", "Turn an effect off, keep the information",
                "Essential tracks play a substitute, such as a vignette in place of a flash, when a player turns that effect off.",
                [crop(LITE / "Gallery_Essential.png")]), out, "06-Essential-Tracks")
    save(editions("Lite"), out, "07-Editions")


if __name__ == "__main__":
    pro()
    lite()
    for edition in ("Pro", "Lite"):
        files = list((PUBLISH / edition / "01-Images").iterdir())
        total = sum(f.stat().st_size for f in files)
        print(f"{edition}: {len(files)} images, {total / 1024 / 1024:.2f} MB in total")
