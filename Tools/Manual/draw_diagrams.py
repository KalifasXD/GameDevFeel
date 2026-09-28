"""Draws the manual's diagrams (the D-numbered figures) into Docs/Manual/Screenshots, in the manual's palette.

    python draw_diagrams.py

D02-01: how a Feel Event becomes feedback: gameplay sends an event, a Feel Map picks the recipe (or gameplay plays a
recipe directly), the player's comfort scales the tracks, and the effects reach six kinds of output.
Drawn at 3000 px wide for a page-wide figure, with text sized to read at about 9 pt on the page.
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

DST = Path("B:/NewUE5Project/Docs/Manual/Screenshots")
FONTS = Path("C:/Windows/Fonts")
TEXT = (46, 52, 59)
HEADING = (22, 27, 34)
MUTED = (107, 114, 128)
ACCENT = (46, 139, 87)
TINT = (241, 247, 243)
LINE = (190, 198, 206)
WHITE = (255, 255, 255)
# Track colors of the recipe editor, by channel.
TRACKS = [(180, 70, 70), (61, 116, 170), (61, 116, 170), (70, 128, 118), (196, 170, 70), (98, 96, 160)]


def font(name: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(FONTS / name), size)


TITLE = font("seguisb.ttf", 66)
BODY = font("segoeui.ttf", 50)
CODE = font("consola.ttf", 40)
PILL = font("segoeui.ttf", 48)


def centered(draw, box, y, text, fnt, fill):
    x0, _, x1, _ = box
    w = draw.textlength(text, font=fnt)
    draw.text((x0 + (x1 - x0 - w) / 2, y), text, font=fnt, fill=fill)


def card(draw, box, title, lines):
    draw.rounded_rectangle(box, 26, fill=TINT, outline=ACCENT, width=5)
    x0, y0, x1, y1 = box
    centered(draw, box, y0 + 44, title, TITLE, HEADING)
    y = y0 + 150
    for text, fnt, fill in lines:
        centered(draw, box, y, text, fnt, fill)
        y += 66


def arrow(draw, start, end, dashed=False, color=ACCENT, width=7):
    (x0, y0), (x1, y1) = start, end
    if dashed:
        length = ((x1 - x0) ** 2 + (y1 - y0) ** 2) ** 0.5
        steps = int(length // 36)
        for i in range(0, steps, 2):
            a, b = i / steps, min((i + 1) / steps, 1.0)
            draw.line([(x0 + (x1 - x0) * a, y0 + (y1 - y0) * a), (x0 + (x1 - x0) * b, y0 + (y1 - y0) * b)], fill=color, width=width)
    else:
        draw.line([start, end], fill=color, width=width)
    # Head pointing along the line.
    import math
    angle = math.atan2(y1 - y0, x1 - x0)
    size = 30
    left = (x1 - size * math.cos(angle - 0.45), y1 - size * math.sin(angle - 0.45))
    right = (x1 - size * math.cos(angle + 0.45), y1 - size * math.sin(angle + 0.45))
    draw.polygon([end, left, right], fill=color)


def d02_01() -> Image.Image:
    image = Image.new("RGB", (3000, 1180), WHITE)
    draw = ImageDraw.Draw(image)
    mid = 470
    gameplay = (30, mid - 230, 640, mid + 230)
    feelmap = (800, mid - 230, 1390, mid + 230)
    recipe = (1550, mid - 230, 2160, mid + 230)
    card(draw, gameplay, "Gameplay", [("sends an event", BODY, TEXT), ("Feel.Event.Hit.Landed", CODE, MUTED), ("with context tags", BODY, TEXT)])
    card(draw, feelmap, "Feel Map", [("the most specific", BODY, TEXT), ("row picks", BODY, TEXT), ("the recipe", BODY, TEXT)])
    draw.rounded_rectangle(recipe, 26, fill=TINT, outline=ACCENT, width=5)
    centered(draw, recipe, recipe[1] + 44, "Recipe", TITLE, HEADING)
    # Tracks as the timeline draws them.
    starts = [0.0, 0.0, 0.0, 0.05, 0.0, 0.0]
    lengths = [0.35, 0.8, 0.95, 0.7, 0.3, 1.0]
    x_left, x_right = recipe[0] + 60, recipe[2] - 60
    for i, (s, l, color) in enumerate(zip(starts, lengths, TRACKS)):
        y = recipe[1] + 160 + i * 44
        x0 = x_left + (x_right - x_left) * s
        draw.rounded_rectangle((x0, y, x0 + (x_right - x_left) * l, y + 30), 8, fill=color)

    arrow(draw, (gameplay[2] + 8, mid), (feelmap[0] - 12, mid))
    arrow(draw, (feelmap[2] + 8, mid), (recipe[0] - 12, mid))
    # Or directly: Play Feel.
    y_direct = gameplay[3] + 150
    draw.line([(335, gameplay[3] + 8), (335, y_direct)], fill=MUTED, width=6)
    draw.line([(335, y_direct), (1855, y_direct)], fill=MUTED, width=6)
    arrow(draw, (1855, y_direct), (1855, recipe[3] + 12), color=MUTED, width=6)
    label = "or plays a recipe directly with Play Feel"
    w = draw.textlength(label, font=BODY)
    draw.rectangle((1095 - w / 2 - 20, y_direct - 40, 1095 + w / 2 + 20, y_direct + 40), fill=WHITE)
    draw.text((1095 - w / 2, y_direct - 34), label, font=BODY, fill=MUTED)

    # Comfort, between the recipe and the outputs.
    comfort = (2250, 150, 2380, 790)
    draw.rounded_rectangle(comfort, 22, fill=ACCENT)
    tag = Image.new("RGBA", (640, 130), (0, 0, 0, 0))
    ImageDraw.Draw(tag).text((320 - draw.textlength("Player comfort", font=TITLE) / 2, 22), "Player comfort", font=TITLE, fill=WHITE)
    tag = tag.rotate(90, expand=True)
    image.paste(tag, (comfort[0] + (comfort[2] - comfort[0] - tag.width) // 2, comfort[1] + (comfort[3] - comfort[1] - tag.height) // 2), tag)
    arrow(draw, (recipe[2] + 8, mid), (comfort[0] - 12, mid))

    outputs = ["Camera", "Screen", "Actor", "Audio", "Controller", "UI"]
    top, step = 120, 116
    for i, name in enumerate(outputs):
        y = top + i * step
        box = (2500, y, 2970, y + 92)
        draw.rounded_rectangle(box, 46, fill=WHITE, outline=LINE, width=4)
        centered(draw, box, y + 14, name, PILL, TEXT)
        arrow(draw, (comfort[2] + 8, mid), (box[0] - 10, y + 46), color=LINE, width=5)
    return image


def trim(image: Image.Image, margin: int = 30) -> Image.Image:
    """Crops the white border to the drawing plus a margin."""
    from PIL import ImageChops
    box = ImageChops.difference(image, Image.new("RGB", image.size, WHITE)).getbbox()
    return image.crop((max(box[0] - margin, 0), max(box[1] - margin, 0), min(box[2] + margin, image.width), min(box[3] + margin, image.height)))


if __name__ == "__main__":
    image = trim(d02_01())
    path = DST / "D02-01.png"
    image.save(path, optimize=True, dpi=(330, 330))
    print(path, image.size)
