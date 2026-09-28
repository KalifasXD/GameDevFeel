"""Draws the manual's full-page and banner artwork at print resolution (300 dpi) into Docs/Manual/design/:
cover.png (A4 cover), part_<n>.png (part divider pages) and chapter_band.png (the dark band behind chapter titles).
The cover shows a real recipe editor picture (Saved/FeelKit/Manual/Editor_HeavyHit.png, taken by DiagFeel.ManualShots).
Run: python make_design_assets.py
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path("B:/NewUE5Project")
OUT = ROOT / "Docs" / "Manual" / "design"
HERO = ROOT / "GameFeelDev" / "Saved" / "FeelKit" / "Manual" / "Editor_HeavyHit.png"
FONTS = Path("C:/Windows/Fonts")

DPI = 300
A4 = (2480, 3508)
INK = (14, 17, 22)
INK_2 = (22, 27, 34)
WHITE = (245, 247, 250)
SOFT = (201, 209, 217)
DIM = (139, 148, 158)
GREEN = (63, 185, 115)
ORANGE = (230, 126, 34)


def font(name: str, pt: float) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(FONTS / name), int(pt * DPI / 72))


def mm(v: float) -> int:
    return int(v / 25.4 * DPI)


def gradient(size, top, bottom) -> Image.Image:
    w, h = size
    img = Image.new("RGB", size, top)
    d = ImageDraw.Draw(img)
    for y in range(h):
        t = y / max(h - 1, 1)
        d.line([(0, y), (w, y)], fill=tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)))
    return img


def tracked(d: ImageDraw.ImageDraw, xy, text: str, fnt, fill, spacing: float) -> None:
    """Letter-spaced text (for small labels)."""
    x, y = xy
    for ch in text:
        d.text((x, y), ch, font=fnt, fill=fill)
        x += d.textlength(ch, font=fnt) + spacing


def framed(shot: Image.Image, width: int, radius: int = 18) -> Image.Image:
    """The screenshot scaled to width, with rounded corners, a hairline edge and a soft shadow (RGBA)."""
    scale = width / shot.width
    shot = shot.resize((width, int(shot.height * scale)), Image.LANCZOS)
    mask = Image.new("L", shot.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, shot.width - 1, shot.height - 1], radius, fill=255)
    pad = 60
    out = Image.new("RGBA", (shot.width + pad * 2, shot.height + pad * 2), (0, 0, 0, 0))
    shadow = Image.new("L", out.size, 0)
    ImageDraw.Draw(shadow).rounded_rectangle([pad, pad + 16, pad + shot.width, pad + shot.height + 16], radius, fill=150)
    shadow = shadow.filter(ImageFilter.GaussianBlur(26))
    out.paste(Image.new("RGBA", out.size, (0, 0, 0, 255)), (0, 0), shadow)
    out.paste(shot.convert("RGBA"), (pad, pad), mask)
    edge = ImageDraw.Draw(out)
    edge.rounded_rectangle([pad, pad, pad + shot.width - 1, pad + shot.height - 1], radius, outline=(60, 68, 78), width=2)
    return out


def cover() -> None:
    img = gradient(A4, INK, INK_2)
    d = ImageDraw.Draw(img)
    left = mm(22)
    # Faint accent glow behind the product picture.
    glow = Image.new("L", A4, 0)
    ImageDraw.Draw(glow).ellipse([mm(-20), mm(95), mm(230), mm(230)], fill=60)
    glow = glow.filter(ImageFilter.GaussianBlur(220))
    img.paste(Image.new("RGB", A4, (24, 70, 48)), (0, 0), glow)
    d = ImageDraw.Draw(img)

    tracked(d, (left, mm(26)), "UNREAL ENGINE PLUGIN", font("seguisb.ttf", 10), GREEN, 9)
    d.text((left - 8, mm(33)), "FeelKit", font=font("seguisb.ttf", 76), fill=WHITE)
    d.text((left, mm(66)), "Manual", font=font("segoeuil.ttf", 30), fill=GREEN)
    d.text((left, mm(84)), "Build game feel on a timeline. Play it with one node.", font=font("segoeui.ttf", 15), fill=WHITE)
    d.text((left, mm(92)), "Let every player choose how much they feel.", font=font("segoeui.ttf", 15), fill=SOFT)

    if HERO.exists():
        shot = Image.open(HERO).convert("RGB").crop((8, 125, 1372, 1028))
        hero = framed(shot, A4[0] - left * 2 + 40)
        img.paste(hero, (left - 60 - 20, mm(108)), hero)
        d = ImageDraw.Draw(img)

    y = mm(232)
    col = (A4[0] - left * 2) // 3
    pillars = [
        ("TIMELINE", "Build and preview a hit", "without pressing Play."),
        ("ONE NODE", "Play recipes from Blueprint", "or C++, or send an event."),
        ("COMFORT", "Every player chooses how much", "shake, flash and rumble they get."),
    ]
    for i, (title, line1, line2) in enumerate(pillars):
        x = left + i * col
        d.rectangle([x, y, x + mm(10), y + 6], fill=GREEN)
        tracked(d, (x, y + mm(5)), title, font("seguisb.ttf", 9.5), WHITE, 6)
        d.text((x, y + mm(12)), line1, font=font("segoeui.ttf", 10.5), fill=SOFT)
        d.text((x, y + mm(17.5)), line2, font=font("segoeui.ttf", 10.5), fill=SOFT)

    d.line([(left, mm(270)), (A4[0] - left, mm(270))], fill=(48, 54, 61), width=3)
    meta = "Version 1.0.0        Unreal Engine 5.6, 5.7 and 5.8        Billo"
    d.text((left, mm(275)), meta, font=font("segoeui.ttf", 9), fill=DIM)
    img.save(OUT / "cover.png", dpi=(DPI, DPI), optimize=True)


def part(number: str, title: str, chapters: list[str], name: str) -> None:
    img = gradient(A4, INK, INK_2)
    d = ImageDraw.Draw(img)
    left = mm(22)
    tracked(d, (left, mm(90)), f"PART {number}", font("seguisb.ttf", 11), GREEN, 10)
    d.text((left - 6, mm(98)), title, font=font("seguisb.ttf", 44), fill=WHITE)
    d.rectangle([left, mm(125), left + mm(18), mm(125) + 8], fill=GREEN)
    y = mm(137)
    for entry in chapters:
        num, text = entry.split(" ", 1)
        d.text((left, y), num, font=font("seguisb.ttf", 12), fill=GREEN)
        d.text((left + mm(12), y), text, font=font("segoeui.ttf", 12), fill=SOFT)
        y += mm(8.2)
    img.save(OUT / name, dpi=(DPI, DPI), optimize=True)


def chapter_band() -> None:
    size = (A4[0], mm(66))
    img = gradient(size, INK, INK_2)
    d = ImageDraw.Draw(img)
    d.rectangle([0, size[1] - 10, size[0], size[1]], fill=GREEN)
    img.save(OUT / "chapter_band.png", dpi=(DPI, DPI), optimize=True)


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    cover()
    from manual_tables import load_table
    chapters = [(r["number"], r["title"]) for r in load_table("outline") if "." not in r["number"] and r["number"].isdigit()]
    part("ONE", "Using FeelKit", [f"{n} {t}" for n, t in chapters if int(n) <= 15], "part_1.png")
    part("TWO", "Reference", [f"{n} {t}" for n, t in chapters if int(n) > 15], "part_2.png")
    chapter_band()
    print("wrote", sorted(p.name for p in OUT.iterdir()))
