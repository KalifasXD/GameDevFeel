"""Builds the FeelKit Manual as a Word document, and as a PDF with --pdf.

Sources are the Markdown files in src/ (one per chapter, in file name order). The file name gives the
chapter number: 02_concepts.md is chapter 2, A_glossary.md is Appendix A. Files in src/notes/ are not
chapters; gen_reference.py reads them.

Markdown accepted here:

    # Chapter title {#anchor}         ## Section {#anchor}      ### Subsection {#anchor}      #### Unnumbered
    paragraphs, "- " bullet lists (two spaces more for a second level), "1. " numbered lists
    | pipe | tables |   with an optional line {widths: 30,70} before them; a first cell ">> text" spans the row
    ``` code blocks ```
    > note box
    **bold**   *italic*   `code`   [link text](https://...)
    [see: anchor] -> "see 2.3 Title"      [See: anchor] -> "See 2.3 Title"
    [ref: anchor] -> "2.3 Title"          [Ref: anchor] -> same, capitalised       [name: anchor] -> "Title"
    [shot: S05-02 | caption]   on its own line: shots/S05-02.png, or a grey box while the file is missing
    [video: V2]                on its own line: a box with the video's title and link from videos.csv
    [edition: Pro]             on its own line: the edition label of a section
    [draft: text]              a highlighted note that must be gone before release
    <!-- comment -->           left out of the document

Run:

    python build_manual.py                 writes out/FeelKit_Manual_<version>.docx
    python build_manual.py --pdf           also updates the fields in Word and exports the PDF
    python build_manual.py --release       leaves out the draft line on the title page
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
import re

from lxml import etree
import sys
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

from docx import Document
from docx.enum.section import WD_ORIENT
from docx.enum.style import WD_STYLE_TYPE
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_COLOR_INDEX, WD_TAB_ALIGNMENT
from docx.opc.constants import RELATIONSHIP_TYPE as RT
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls
from docx.oxml.ns import qn
from docx.enum.section import WD_SECTION
from docx.shared import Cm, Inches, Pt, RGBColor

from manual_tables import MANUAL, load_table

HERE = Path(__file__).resolve().parent
SRC = MANUAL / "src"
OUT = MANUAL / "out"
SHOTS = MANUAL / "Screenshots"
PLUGIN = Path("B:/NewUE5Project/GameFeelDev/Plugins/FeelKit/FeelKit.uplugin")

TEXT_WIDTH_CM = 15.6
AUTHOR = "Billo"
TITLE = "FeelKit Manual"

FONT_BODY = "Segoe UI"
FONT_HEADING = "Segoe UI Semibold"
FONT_CODE = "Consolas"

COLOUR_TEXT = RGBColor(0x2E, 0x34, 0x3B)
COLOUR_HEADING = RGBColor(0x16, 0x1B, 0x22)
COLOUR_ACCENT = RGBColor(0x2E, 0x8B, 0x57)
COLOUR_LINK = RGBColor(0x1F, 0x6F, 0xB2)
COLOUR_MUTED = RGBColor(0x6B, 0x72, 0x80)
HEX_ACCENT = "2E8B57"
HEX_LABEL_TINT = "EAF5EF"
NBSP = "\u00a0"
EN_SPACE = "\u2002"
# PRO label size and raise (half points) by where it sits: heading level 2, 3, 4, contents key, table cell.
LABEL_SIZES = {2: (7.5, 5), 3: (7.0, 4), 4: (6.5, 3), "key": (7.0, 1), "cell": (6.5, 1)}
# Letter spacing of the Pro label, in twentieths of a point.
LABEL_TRACKING = 36
COLOUR_PRO = RGBColor(0x2A, 0x7A, 0x50)
# Pro heading band (D4, chosen 2026-09-28): pale green fill, a green bar on the left, PRO at the right end. Padding of
# the band above and below the text (points) by heading level; the values differ per level so that Word never joins
# two banded headings that follow each other into one box.
BAND_PADDING = {2: 4, 3: 3, 4: 2, "key": 2}
BAND_BAR = 18
BAND_BAR_GAP = 6
HEX_SPEC = "F1F7F3"
HEX_TABLE_HEAD = "DCEEE3"
HEX_TABLE_SECTION = "EEF6F1"
HEX_TABLE_RULE = "2F6B4B"
COLOUR_TABLE_HEAD = RGBColor(0x1C, 0x55, 0x36)
COLOUR_LABEL = RGBColor(0x2F, 0x6B, 0x4B)
HEX_CODE_TINT = "E4F1F2"
COLOUR_CODE = RGBColor(0x0B, 0x5F, 0x68)
HEX_TABLE_LINE = "DDE2E7"
HEX_NUMERAL = "EDF0F3"
DESIGN = MANUAL / "design"
FIGS = MANUAL / "out" / "_figures"
# Screenshots are placed at this many pixels per inch at most, so interface text stays readable in print.
FIGURE_PPI = 165
HEX_RULE = "D5D9DF"
HEX_INK = "161B22"
HEX_ZEBRA = "F2F7F8"
HEX_NOTE = "EEF6F1"

# Sample build: which parts of which chapters go into the look sample (from anchor, up to but not including anchor).
SAMPLE = {
    "2": [(None, "ch02_overlap_inside")],
    "4": [(None, "no_such_anchor")],
    "16": [(None, "ref_recipe_fields"), ("ref_steps_camera", "step_camera_zoom")],
}


class BuildError(Exception):
    pass


# ---------------------------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------------------------

@dataclass
class Block:
    kind: str
    text: str = ""
    level: int = 0
    anchor: str = ""
    number: str = ""
    items: list = field(default_factory=list)
    rows: list = field(default_factory=list)
    widths: list = field(default_factory=list)
    ordered: bool = False
    extra: str = ""


@dataclass
class Chapter:
    path: Path
    label: str          # "2" or "A"
    appendix: bool
    blocks: list[Block] = field(default_factory=list)
    title: str = ""
    anchor: str = ""


HEADING_RE = re.compile(r"^(#{1,4})\s+(.*?)(?:\s*\{#(\w+)\})?\s*$")
LIST_RE = re.compile(r"^(\s*)(-|\d+\.)\s+(.*)$")
SHOT_RE = re.compile(r"^\[shot:\s*([\w-]+)\s*\|\s*(.*)\]$")
VIDEO_RE = re.compile(r"^\[video:\s*(\w+)\s*\]$")
EDITION_RE = re.compile(r"^\[edition:\s*(.*?)\]$")
DRAFT_BLOCK_RE = re.compile(r"^\[draft:\s*(.*)\]$")
WIDTHS_RE = re.compile(r"^\{widths:\s*([\d,\s]+)\}$")


def split_row(line: str) -> list[str]:
    line = line.strip()
    if line.startswith("|"):
        line = line[1:]
    if line.endswith("|") and not line.endswith("\\|"):
        line = line[:-1]
    cells, cur, i = [], [], 0
    while i < len(line):
        if line[i] == "\\" and i + 1 < len(line) and line[i + 1] == "|":
            cur.append("|")
            i += 2
            continue
        if line[i] == "|":
            cells.append("".join(cur).strip())
            cur = []
        else:
            cur.append(line[i])
        i += 1
    cells.append("".join(cur).strip())
    return cells


def parse_markdown(text: str, where: str) -> list[Block]:
    text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    lines = text.split("\n")
    blocks: list[Block] = []
    widths: list[int] = []
    i = 0

    def starts_block(line: str) -> bool:
        s = line.strip()
        return (not s or HEADING_RE.match(line) is not None or s.startswith("```") or s.startswith("|")
                or LIST_RE.match(line) is not None or SHOT_RE.match(s) is not None or VIDEO_RE.match(s) is not None
                or EDITION_RE.match(s) is not None or WIDTHS_RE.match(s) is not None or s.startswith("> ")
                or DRAFT_BLOCK_RE.match(s) is not None)

    while i < len(lines):
        line = lines[i]
        s = line.strip()
        if not s:
            i += 1
            continue
        m = HEADING_RE.match(line)
        if m:
            blocks.append(Block("heading", m.group(2).strip(), level=len(m.group(1)), anchor=m.group(3) or ""))
            i += 1
            continue
        if s.startswith("```"):
            code = []
            i += 1
            while i < len(lines) and not lines[i].strip().startswith("```"):
                code.append(lines[i].rstrip())
                i += 1
            i += 1
            blocks.append(Block("code", "\n".join(code)))
            continue
        m = WIDTHS_RE.match(s)
        if m:
            widths = [int(x) for x in m.group(1).split(",") if x.strip()]
            i += 1
            continue
        if s.startswith("|"):
            rows = []
            while i < len(lines) and lines[i].strip().startswith("|"):
                rows.append(split_row(lines[i]))
                i += 1
            if len(rows) < 2 or not all(re.match(r"^:?-{3,}:?$", c) for c in rows[1]):
                raise BuildError(f"{where}: table without a header separator near '{s[:60]}'")
            header, body = rows[0], rows[2:]
            for r in body:
                if len(r) != len(header):
                    raise BuildError(f"{where}: table row has {len(r)} cells, header has {len(header)}: {r[:2]}")
            if widths and len(widths) != len(header):
                raise BuildError(f"{where}: widths {widths} do not match {len(header)} columns")
            blocks.append(Block("table", rows=[header] + body, widths=widths))
            widths = []
            continue
        m = SHOT_RE.match(s)
        if m:
            blocks.append(Block("shot", m.group(2).strip(), anchor=m.group(1)))
            i += 1
            continue
        m = VIDEO_RE.match(s)
        if m:
            blocks.append(Block("video", anchor=m.group(1)))
            i += 1
            continue
        m = EDITION_RE.match(s)
        if m:
            if m.group(1) not in ("Lite and Pro", "Pro"):
                raise BuildError(f"{where}: edition label must be 'Lite and Pro' or 'Pro', not '{m.group(1)}'")
            blocks.append(Block("edition", m.group(1)))
            i += 1
            continue
        m = DRAFT_BLOCK_RE.match(s)
        if m:
            blocks.append(Block("draft", m.group(1)))
            i += 1
            continue
        if s.startswith("> "):
            note = []
            while i < len(lines) and lines[i].strip().startswith(">"):
                note.append(lines[i].strip()[1:].strip())
                i += 1
            blocks.append(Block("note", " ".join(note)))
            continue
        m = LIST_RE.match(line)
        if m:
            ordered = m.group(2) != "-"
            items = []
            while i < len(lines):
                lm = LIST_RE.match(lines[i])
                if lm and (lm.group(2) != "-") == ordered or (lm and len(lm.group(1)) >= 2):
                    level = 1 if len(lm.group(1)) >= 2 else 0
                    items.append([level, lm.group(3).strip(), lm.group(2) != "-"])
                    i += 1
                elif lines[i].strip() and lines[i].startswith("  ") and items and not starts_block(lines[i].strip()):
                    items[-1][1] += " " + lines[i].strip()
                    i += 1
                else:
                    break
            blocks.append(Block("list", items=items, ordered=ordered, extra=m.group(2).rstrip(".") if ordered else ""))
            continue
        para = [s]
        i += 1
        while i < len(lines) and not starts_block(lines[i]):
            para.append(lines[i].strip())
            i += 1
        blocks.append(Block("para", " ".join(para)))
    return blocks


def load_chapters() -> list[Chapter]:
    chapters = []
    for path in sorted(SRC.glob("*.md")):
        m = re.match(r"^(\d+)_", path.name) or re.match(r"^([A-Z])_", path.name)
        if not m:
            continue
        label = m.group(1).lstrip("0") if m.group(1).isdigit() else m.group(1)
        chapter = Chapter(path, label, not m.group(1).isdigit())
        chapter.blocks = parse_markdown(path.read_text(encoding="utf-8"), path.name)
        headings = [b for b in chapter.blocks if b.kind == "heading"]
        if not headings or headings[0].level != 1 or sum(1 for b in headings if b.level == 1) != 1:
            raise BuildError(f"{path.name}: needs exactly one # heading, at the top")
        chapter.title = headings[0].text
        chapter.anchor = headings[0].anchor
        chapters.append(chapter)
    return chapters


# ---------------------------------------------------------------------------------------------
# Numbering and anchors
# ---------------------------------------------------------------------------------------------

@dataclass
class Target:
    number: str
    title: str
    level: int
    appendix: bool
    planned: bool = False


def number_headings(chapters: list[Chapter]) -> dict[str, Target]:
    targets: dict[str, Target] = {}
    for ch in chapters:
        h2 = h3 = 0
        for b in ch.blocks:
            if b.kind != "heading":
                continue
            if b.level == 1:
                b.number = ch.label
            elif b.level == 2:
                h2 += 1
                h3 = 0
                b.number = f"{ch.label}.{h2}"
            elif b.level == 3:
                h3 += 1
                b.number = f"{ch.label}.{h2}.{h3}"
            else:
                b.number = ""
            if not b.anchor:
                b.anchor = "h" + (b.number.replace(".", "_") if b.number else f"{ch.label}_{id(b)}")
            if not re.match(r"^[A-Za-z]\w{0,39}$", b.anchor):
                raise BuildError(f"{ch.path.name}: anchor '{b.anchor}' is not a valid Word bookmark name (a letter, then letters, digits or _, at most 40)")
            if b.anchor in targets:
                raise BuildError(f"{ch.path.name}: anchor '{b.anchor}' is used twice")
            targets[b.anchor] = Target(b.number, b.text, b.level, ch.appendix)
    return targets


def load_outline() -> dict[str, Target]:
    planned = {}
    for row in load_table("outline"):
        number = row["number"]
        level = 1 if "." not in number else number.count(".") + 1
        planned[row["anchor"]] = Target(number, row["title"], level, number.isalpha(), planned=True)
    return planned


def ref_text(t: Target, kind: str) -> str:
    if kind == "name":
        return t.title
    if t.level == 1:
        text = f"Appendix {t.number}, {t.title}" if t.appendix else f"chapter {t.number}, {t.title}"
    elif t.number:
        text = f"{t.number} {t.title}"
    else:
        text = t.title
    if kind in ("see", "See"):
        text = "see " + text
    if kind in ("See", "Ref"):
        text = text[0].upper() + text[1:]
    return text


# ---------------------------------------------------------------------------------------------
# Word helpers
# ---------------------------------------------------------------------------------------------

def set_font(style_or_run_font_element, name: str) -> None:
    rpr = style_or_run_font_element
    fonts = rpr.find(qn("w:rFonts"))
    if fonts is None:
        fonts = OxmlElement("w:rFonts")
        rpr.insert(0, fonts)
    for attr in ("w:asciiTheme", "w:hAnsiTheme", "w:eastAsiaTheme", "w:cstheme"):
        if fonts.get(qn(attr)) is not None:
            del fonts.attrib[qn(attr)]
    for attr in ("w:ascii", "w:hAnsi", "w:cs", "w:eastAsia"):
        fonts.set(qn(attr), name)


def style_rpr(style):
    return style.element.get_or_add_rPr()


def shade(element_pr, fill: str) -> None:
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), fill)
    element_pr.append(shd)


def add_field(paragraph, instruction: str, cached: str, bold: bool = False):
    def run_with(child):
        r = OxmlElement("w:r")
        if bold:
            rpr = OxmlElement("w:rPr")
            rpr.append(OxmlElement("w:b"))
            r.append(rpr)
        r.append(child)
        paragraph._p.append(r)
        return r

    begin = OxmlElement("w:fldChar")
    begin.set(qn("w:fldCharType"), "begin")
    run_with(begin)
    instr = OxmlElement("w:instrText")
    instr.set(qn("xml:space"), "preserve")
    instr.text = f" {instruction} "
    run_with(instr)
    sep = OxmlElement("w:fldChar")
    sep.set(qn("w:fldCharType"), "separate")
    run_with(sep)
    t = OxmlElement("w:t")
    t.set(qn("xml:space"), "preserve")
    t.text = cached
    run_with(t)
    end = OxmlElement("w:fldChar")
    end.set(qn("w:fldCharType"), "end")
    run_with(end)


class Builder:
    def __init__(self, version: str, release: bool, sample: bool = False):
        self.version = version
        self.release = release
        self.sample = sample
        self.last_heading = None
        self.last_heading_level = 0
        self.current_edition = ""
        self.last_num_id = None
        self.header_ready = False
        self.part_pending = False
        self.doc = Document()
        self.bookmark_id = 0
        self.figure_in_chapter = 0
        self.chapter: Chapter | None = None
        self.targets: dict[str, Target] = {}
        self.planned: dict[str, Target] = {}
        self.shots = {row["id"]: row for row in load_table("shots")}
        self.videos = {row["id"]: row for row in load_table("videos")}
        self.warnings: list[str] = []
        self.used_shots: list[str] = []
        self.list_num_ids: list[str] = []
        self.in_table = False
        self.setup_document()

    @staticmethod
    def load_csv(path: Path, key: str) -> dict[str, dict]:
        if not path.exists():
            return {}
        with path.open(encoding="utf-8", newline="") as f:
            return {row[key]: row for row in csv.DictReader(f)}

    # --- document setup ---------------------------------------------------------------------

    def setup_document(self) -> None:
        doc = self.doc
        section = doc.sections[0]
        section.orientation = WD_ORIENT.PORTRAIT
        section.page_width = Cm(21.0)
        section.page_height = Cm(29.7)
        section.left_margin = Cm(2.7)
        section.right_margin = Cm(2.7)
        section.top_margin = Cm(2.6)
        section.bottom_margin = Cm(2.4)
        section.header_distance = Cm(1.2)
        section.footer_distance = Cm(1.1)
        section.different_first_page_header_footer = True

        styles = doc.styles
        defaults = styles.element.find(qn("w:docDefaults"))
        rpr_default = defaults.find(qn("w:rPrDefault")).find(qn("w:rPr"))
        lang = rpr_default.find(qn("w:lang"))
        if lang is None:
            lang = OxmlElement("w:lang")
            rpr_default.append(lang)
        lang.set(qn("w:val"), "en-US")
        lang.set(qn("w:eastAsia"), "en-US")
        lang.set(qn("w:bidi"), "ar-SA")
        set_font(rpr_default, FONT_BODY)

        normal = styles["Normal"]
        set_font(style_rpr(normal), FONT_BODY)
        normal.font.size = Pt(10)
        normal.font.color.rgb = COLOUR_TEXT
        normal.paragraph_format.space_after = Pt(8)
        normal.paragraph_format.line_spacing = 1.3

        sizes = {1: 30, 2: 15, 3: 12, 4: 10}
        for level in range(1, 5):
            st = styles[f"Heading {level}"]
            set_font(style_rpr(st), FONT_HEADING)
            st.font.size = Pt(sizes[level])
            st.font.bold = False
            st.font.italic = False
            st.font.color.rgb = COLOUR_HEADING
            pf = st.paragraph_format
            pf.space_before = Pt({1: 120, 2: 26, 3: 18, 4: 10}[level])
            pf.space_after = Pt({1: 8, 2: 8, 3: 5, 4: 3}[level])
            pf.keep_with_next = True
            pf.page_break_before = False
            pf.line_spacing = 1.05
        # Chapter titles sit low on a white opener page, under a large pale chapter number.
        # Word must not downsample or recompress screenshots when it saves or exports the PDF.
        settings = doc.settings.element
        if settings.find(qn("w:doNotAutoCompressPictures")) is None:
            settings.append(OxmlElement("w:doNotAutoCompressPictures"))
        # "High fidelity" image resolution (0): without it the PDF export resamples every picture to about 200 ppi JPEG.
        w14 = "http://schemas.microsoft.com/office/word/2010/wordml"
        if settings.find(f"{{{w14}}}defaultImageDpi") is None:
            dpi = etree.SubElement(settings, f"{{{w14}}}defaultImageDpi")
            dpi.set(f"{{{w14}}}val", "0")

        title = styles["Title"]
        set_font(style_rpr(title), FONT_HEADING)
        title.font.size = Pt(54)
        title.font.color.rgb = COLOUR_HEADING
        title.paragraph_format.space_after = Pt(0)
        for bdr in title.element.xpath(".//w:pBdr"):
            bdr.getparent().remove(bdr)
        subtitle = styles["Subtitle"]
        set_font(style_rpr(subtitle), FONT_BODY)
        subtitle.font.size = Pt(13)
        subtitle.font.italic = False
        subtitle.font.color.rgb = COLOUR_MUTED

        part = styles.add_style("Part Title", WD_STYLE_TYPE.PARAGRAPH)
        part.base_style = normal
        set_font(style_rpr(part), FONT_HEADING)
        part.font.size = Pt(30)
        part.font.color.rgb = COLOUR_HEADING
        part.paragraph_format.page_break_before = True
        part.paragraph_format.space_before = Pt(220)

        contents = styles.add_style("Contents Title", WD_STYLE_TYPE.PARAGRAPH)
        contents.base_style = normal
        set_font(style_rpr(contents), FONT_HEADING)
        contents.font.size = Pt(24)
        contents.font.color.rgb = COLOUR_HEADING
        contents.paragraph_format.space_before = Pt(40)
        contents.paragraph_format.space_after = Pt(18)

        for level, size, bold, before in ((1, 10.5, True, 10), (2, 9.5, False, 2), (3, 9, False, 0)):
            toc = styles.add_style(f"TOC {level}", WD_STYLE_TYPE.PARAGRAPH)
            toc.base_style = normal
            set_font(style_rpr(toc), FONT_HEADING if bold else FONT_BODY)
            toc.font.size = Pt(size)
            toc.font.color.rgb = COLOUR_HEADING if bold else COLOUR_TEXT
            toc.paragraph_format.space_before = Pt(before)
            toc.paragraph_format.space_after = Pt(2)
            toc.paragraph_format.left_indent = Cm(0.6 * (level - 1))

        code = styles.add_style("Code Block", WD_STYLE_TYPE.PARAGRAPH)
        code.base_style = normal
        set_font(style_rpr(code), FONT_CODE)
        code.font.size = Pt(8.5)
        code.paragraph_format.space_after = Pt(0)
        code.paragraph_format.line_spacing = 1.0
        code.paragraph_format.left_indent = Cm(0.3)
        shade(code.element.get_or_add_pPr(), HEX_ZEBRA)

        inline = styles.add_style("Inline Code", WD_STYLE_TYPE.CHARACTER)
        set_font(style_rpr(inline), FONT_CODE)
        inline.font.size = Pt(8.5)
        inline.font.color.rgb = COLOUR_CODE
        ishd = OxmlElement("w:shd")
        ishd.set(qn("w:val"), "clear")
        ishd.set(qn("w:color"), "auto")
        ishd.set(qn("w:fill"), HEX_CODE_TINT)
        style_rpr(inline).append(ishd)

        link = styles.add_style("Internal Link", WD_STYLE_TYPE.CHARACTER)
        link.font.color.rgb = COLOUR_LINK
        ext = styles.add_style("Web Link", WD_STYLE_TYPE.CHARACTER)
        ext.font.color.rgb = COLOUR_LINK
        ext.font.underline = True

        table_text = styles.add_style("Table Text", WD_STYLE_TYPE.PARAGRAPH)
        table_text.base_style = normal
        table_text.font.size = Pt(8.5)
        table_text.paragraph_format.space_after = Pt(0)
        table_text.paragraph_format.line_spacing = 1.05

        caption = styles["Caption"]
        set_font(style_rpr(caption), FONT_BODY)
        caption.font.size = Pt(8.5)
        caption.font.italic = False
        caption.font.bold = False
        caption.font.color.rgb = COLOUR_MUTED
        caption.paragraph_format.space_before = Pt(4)
        caption.paragraph_format.space_after = Pt(14)

        note = styles.add_style("Note Box", WD_STYLE_TYPE.PARAGRAPH)
        note.base_style = normal
        ppr = note.element.get_or_add_pPr()
        shade(ppr, HEX_NOTE)
        bdr = OxmlElement("w:pBdr")
        left = OxmlElement("w:left")
        left.set(qn("w:val"), "single")
        left.set(qn("w:sz"), "24")
        left.set(qn("w:space"), "8")
        left.set(qn("w:color"), HEX_ACCENT)
        bdr.append(left)
        ppr.insert(0, bdr)
        note.paragraph_format.left_indent = Cm(0.35)
        note.paragraph_format.space_before = Pt(4)
        note.paragraph_format.space_after = Pt(10)

        box_title = styles.add_style("Box Title", WD_STYLE_TYPE.PARAGRAPH)
        box_title.base_style = normal
        set_font(style_rpr(box_title), FONT_HEADING)
        box_title.font.size = Pt(8)
        box_title.font.color.rgb = COLOUR_ACCENT
        box_title.paragraph_format.space_after = Pt(3)

        self.page_numbers(section.footer.paragraphs[0])

    def page_numbers(self, footer) -> None:
        footer.alignment = WD_ALIGN_PARAGRAPH.RIGHT
        add_field(footer, "PAGE", "1")
        for r in footer.runs:
            r.font.size = Pt(8.5)
            r.font.color.rgb = COLOUR_MUTED

    # --- inline text ------------------------------------------------------------------------

    INLINE_RE = re.compile(
        r"\*\*(?P<bold>.+?)\*\*"
        r"|`(?P<code>[^`]+)`"
        r"|\[(?P<xkind>see|See|ref|Ref|name):\s*(?P<xanchor>\w+)\]"
        r"|\[draft:\s*(?P<draft>[^\]]+)\]"
        r"|\[(?P<ltext>[^\]]+)\]\((?P<lurl>https?://[^)\s]+)\)"
        r"|(?<![\w*])\*(?P<ital>[^*\s][^*]*?)\*(?![\w*])"
    )

    # Names that are code rather than English (gameplay tags, asset names, C++ types) are set as code even when the
    # source text does not mark them.
    AUTO_CODE_RE = re.compile(r"(?<![\w`/.])(Feel(?:\.[A-Z][A-Za-z0-9]*)+|(?:FR|WBP|BP|FM|AM|ATT|NS|S|M)_[A-Za-z0-9_]+|"
                              r"[UAFIE][A-Z][a-z]+(?:[A-Z][a-z0-9]*)+(?:::\w+)?)(?![\w])")

    def add_plain(self, paragraph, text: str, bold: bool = False, size: float | None = None) -> None:
        pos = 0
        for m in self.AUTO_CODE_RE.finditer(text):
            if m.start() > pos:
                self.add_run(paragraph, text[pos:m.start()], bold=bold, size=size)
            self.add_run(paragraph, m.group(1), bold=bold, style="Inline Code", size=size or (8.5 if self.in_table else None))
            pos = m.end()
        if pos < len(text):
            self.add_run(paragraph, text[pos:], bold=bold, size=size)

    def add_inline(self, paragraph, text: str, bold: bool = False, size: float | None = None) -> None:
        pos = 0
        for m in self.INLINE_RE.finditer(text):
            if m.start() > pos:
                self.add_plain(paragraph, text[pos:m.start()], bold=bold, size=size)
            if m.group("bold") is not None:
                self.add_inline(paragraph, m.group("bold"), bold=True, size=size)
            elif m.group("code") is not None:
                self.add_run(paragraph, m.group("code"), bold=bold, style="Inline Code", size=size or (8.5 if self.in_table else None))
            elif m.group("xkind") is not None:
                self.add_reference(paragraph, m.group("xkind"), m.group("xanchor"), bold, size)
            elif m.group("draft") is not None:
                if not self.sample:
                    r = self.add_run(paragraph, "(" + m.group("draft") + ")", bold=bold, size=size)
                    r.italic = True
                    r.font.color.rgb = COLOUR_MUTED
            elif m.group("ltext") is not None:
                self.add_external_link(paragraph, m.group("ltext"), m.group("lurl"), size)
            elif m.group("ital") is not None:
                r = self.add_run(paragraph, m.group("ital"), bold=bold, size=size)
                r.italic = True
            pos = m.end()
        if pos < len(text):
            self.add_plain(paragraph, text[pos:], bold=bold, size=size)

    @staticmethod
    def add_run(paragraph, text: str, bold: bool = False, style: str | None = None, size: float | None = None):
        run = paragraph.add_run(text.replace("\\*", "*"), style=style)
        if bold:
            run.bold = True
        if size:
            run.font.size = Pt(size)
        return run

    def add_reference(self, paragraph, kind: str, anchor: str, bold: bool, size: float | None) -> None:
        target = self.targets.get(anchor)
        if target is None:
            planned = self.planned.get(anchor)
            if planned is None:
                raise BuildError(f"{self.chapter.path.name}: [{kind}: {anchor}] points to no heading and no entry in the outline table of the Manual Plan")
            self.warnings.append(f"{self.chapter.path.name}: reference to {anchor} ({planned.number}), a section not written yet")
            self.add_run(paragraph, ref_text(planned, kind), bold=bold, size=size)
            return
        text = ref_text(target, kind)
        link = OxmlElement("w:hyperlink")
        link.set(qn("w:anchor"), anchor)
        link.set(qn("w:history"), "1")
        run = OxmlElement("w:r")
        rpr = OxmlElement("w:rPr")
        rstyle = OxmlElement("w:rStyle")
        rstyle.set(qn("w:val"), self.doc.styles["Internal Link"].style_id)
        rpr.append(rstyle)
        if bold:
            rpr.append(OxmlElement("w:b"))
        if size:
            sz = OxmlElement("w:sz")
            sz.set(qn("w:val"), str(int(size * 2)))
            rpr.append(sz)
        run.append(rpr)
        t = OxmlElement("w:t")
        t.set(qn("xml:space"), "preserve")
        t.text = text
        run.append(t)
        link.append(run)
        paragraph._p.append(link)

    def add_external_link(self, paragraph, text: str, url: str, size: float | None) -> None:
        rel = paragraph.part.relate_to(url, RT.HYPERLINK, is_external=True)
        link = OxmlElement("w:hyperlink")
        link.set(qn("r:id"), rel)
        run = OxmlElement("w:r")
        rpr = OxmlElement("w:rPr")
        rstyle = OxmlElement("w:rStyle")
        rstyle.set(qn("w:val"), self.doc.styles["Web Link"].style_id)
        rpr.append(rstyle)
        if size:
            sz = OxmlElement("w:sz")
            sz.set(qn("w:val"), str(int(size * 2)))
            rpr.append(sz)
        run.append(rpr)
        t = OxmlElement("w:t")
        t.set(qn("xml:space"), "preserve")
        t.text = text
        run.append(t)
        link.append(run)
        paragraph._p.append(link)

    def add_bookmark(self, paragraph, name: str) -> None:
        self.bookmark_id += 1
        start = OxmlElement("w:bookmarkStart")
        start.set(qn("w:id"), str(self.bookmark_id))
        start.set(qn("w:name"), name)
        end = OxmlElement("w:bookmarkEnd")
        end.set(qn("w:id"), str(self.bookmark_id))
        ppr = paragraph._p.find(qn("w:pPr"))
        index = 1 if ppr is not None else 0
        paragraph._p.insert(index, start)
        paragraph._p.append(end)

    # --- blocks -----------------------------------------------------------------------------

    def float_picture(self, path: Path, width_cm: float, x_cm: float = 0.0, y_cm: float = 0.0) -> None:
        """A picture placed behind the text at a fixed spot on the page (full-page cover, chapter band)."""
        p = self.doc.add_paragraph()
        p.paragraph_format.space_after = Pt(0)
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.line_spacing = Pt(1)
        run = p.add_run()
        run.add_picture(str(path), width=Cm(width_cm))
        inline = run._r.xpath(".//wp:inline")[0]
        extent = inline.find(qn("wp:extent"))
        doc_pr = inline.find(qn("wp:docPr"))
        graphic = inline.find(qn("a:graphic"))
        anchor = parse_xml(
            f'<wp:anchor {nsdecls("wp", "a", "pic", "r")} distT="0" distB="0" distL="0" distR="0" simplePos="0" '
            f'relativeHeight="0" behindDoc="1" locked="1" layoutInCell="1" allowOverlap="1">'
            f'<wp:simplePos x="0" y="0"/>'
            f'<wp:positionH relativeFrom="page"><wp:posOffset>{int(Cm(x_cm))}</wp:posOffset></wp:positionH>'
            f'<wp:positionV relativeFrom="page"><wp:posOffset>{int(Cm(y_cm))}</wp:posOffset></wp:positionV>'
            f'</wp:anchor>')
        anchor.append(extent)
        effect = OxmlElement("wp:effectExtent")
        for side in ("l", "t", "r", "b"):
            effect.set(side, "0")
        anchor.append(effect)
        anchor.append(OxmlElement("wp:wrapNone"))
        anchor.append(doc_pr)
        anchor.append(OxmlElement("wp:cNvGraphicFramePr"))
        anchor.append(graphic)
        inline.getparent().replace(inline, anchor)

    def new_section(self) -> None:
        """Every part and chapter starts a new page and section: no running header on its first page."""
        section = self.doc.add_section(WD_SECTION.NEW_PAGE)
        section.different_first_page_header_footer = True
        if not self.header_ready:
            self.header_ready = True
            section.header.is_linked_to_previous = False
            section.footer.is_linked_to_previous = False
            header = section.header.paragraphs[0]
            # The Header style comes with centre and right tabs; clear them so the chapter sits at the right margin.
            header.paragraph_format.tab_stops.add_tab_stop(Inches(3.25), WD_TAB_ALIGNMENT.CLEAR)
            header.paragraph_format.tab_stops.add_tab_stop(Inches(6.5), WD_TAB_ALIGNMENT.CLEAR)
            header.paragraph_format.tab_stops.add_tab_stop(Cm(TEXT_WIDTH_CM), WD_TAB_ALIGNMENT.RIGHT)
            header.add_run(TITLE)
            header.add_run("\t")
            add_field(header, 'STYLEREF "Heading 1"', "")
            for r in header.runs:
                r.font.size = Pt(8)
                r.font.color.rgb = COLOUR_MUTED
            hppr = header._p.get_or_add_pPr()
            hbdr = OxmlElement("w:pBdr")
            hbottom = OxmlElement("w:bottom")
            hbottom.set(qn("w:val"), "single")
            hbottom.set(qn("w:sz"), "4")
            hbottom.set(qn("w:space"), "6")
            hbottom.set(qn("w:color"), HEX_RULE)
            hbdr.append(hbottom)
            hppr.insert(0, hbdr)
            self.page_numbers(section.footer.paragraphs[0])

    def title_page(self) -> None:
        doc = self.doc
        cover = DESIGN / "cover.png"
        if cover.exists():
            self.float_picture(cover, 21.0)
        else:
            doc.add_paragraph("FeelKit Manual", style="Title")
        contents = doc.add_paragraph("Contents", style="Contents Title")
        contents.paragraph_format.page_break_before = True
        toc = doc.add_paragraph()
        add_field(toc, 'TOC \\o "1-2" \\h \\z \\u', "Update fields (F9) to fill in the contents.")

        # The key shows the band itself, so it looks exactly like the Pro headings it explains.
        key = doc.add_paragraph()
        key.paragraph_format.space_before = Pt(20)
        key.paragraph_format.space_after = Pt(4)
        k = key.add_run("A section in FeelKit Pro only")
        k.font.size = Pt(8.5)
        k.font.color.rgb = COLOUR_HEADING
        self.pro_band(key, "key", width_cm=7.5)
        note = doc.add_paragraph()
        self.add_pro_label(note, *LABEL_SIZES["key"], gap=False)
        n = note.add_run("\u2003In a table or in the contents: in FeelKit Pro only. Everything without the band or the label is "
                         "in FeelKit Lite and FeelKit Pro.")
        n.font.size = Pt(8.5)
        n.font.color.rgb = COLOUR_MUTED
        spacer = doc.add_paragraph()
        spacer.paragraph_format.space_before = Pt(0)
        if not self.release:
            d = doc.add_paragraph().add_run(f"Draft, {dt.date.today().strftime('%d %B %Y').lstrip('0')}")
            d.font.size = Pt(8)
            d.font.color.rgb = COLOUR_MUTED

    def add_pro_label(self, paragraph, size: float, raise_hp: int, gap: bool = True) -> None:
        """The Pro label: the word PRO in small, widely spaced green capitals, raised a little, with no box. Typeset like
        a small-caps marker, so it reads as a quiet note next to the heading rather than a badge."""
        if gap:
            # Non-breaking spaces keep the label on the line of the heading's last word.
            paragraph.add_run(NBSP * 3)
        r = paragraph.add_run("PRO")
        rpr = r._r.get_or_add_rPr()
        set_font(rpr, FONT_HEADING)
        r.font.bold = False
        r.font.size = Pt(size)
        r.font.color.rgb = COLOUR_PRO
        sp = OxmlElement("w:spacing")
        sp.set(qn("w:val"), str(LABEL_TRACKING))
        rpr.append(sp)
        if raise_hp:
            pos = OxmlElement("w:position")
            pos.set(qn("w:val"), str(raise_hp))
            rpr.append(pos)

    def pro_band(self, paragraph, level, width_cm: float = TEXT_WIDTH_CM) -> None:
        """A Pro heading (D4): the paragraph sits on a pale green band with a green bar on its left, and the PRO label
        at the band's right end. Plain paragraph formatting on the heading's own style, so the heading keeps its
        number, its place in the contents and its PDF bookmark. The tab before the label becomes spaces in the
        contents (fix_toc_labels)."""
        pf = paragraph.paragraph_format
        if width_cm < TEXT_WIDTH_CM:
            pf.right_indent = Cm(TEXT_WIDTH_CM - width_cm)
        pf.tab_stops.add_tab_stop(Cm(width_cm), WD_TAB_ALIGNMENT.RIGHT)
        paragraph.add_run("\t")
        self.add_pro_label(paragraph, *LABEL_SIZES[level if level == "key" else min(level, 4)], gap=False)
        pad = str(BAND_PADDING[level if level == "key" else min(level, 4)])
        ppr = paragraph._p.get_or_add_pPr()
        bdr = OxmlElement("w:pBdr")
        for side, size, space, colour in (("top", 4, pad, HEX_LABEL_TINT), ("left", BAND_BAR, str(BAND_BAR_GAP), HEX_ACCENT),
                                          ("bottom", 4, pad, HEX_LABEL_TINT), ("right", 4, str(BAND_BAR_GAP), HEX_LABEL_TINT)):
            el = OxmlElement(f"w:{side}")
            el.set(qn("w:val"), "single")
            el.set(qn("w:sz"), str(size))
            el.set(qn("w:space"), space)
            el.set(qn("w:color"), colour)
            bdr.append(el)
        shd = OxmlElement("w:shd")
        shd.set(qn("w:val"), "clear")
        shd.set(qn("w:color"), "auto")
        shd.set(qn("w:fill"), HEX_LABEL_TINT)
        later = ("w:tabs", "w:suppressAutoHyphens", "w:kinsoku", "w:wordWrap", "w:overflowPunct", "w:topLinePunct",
                 "w:autoSpaceDE", "w:autoSpaceDN", "w:bidi", "w:adjustRightInd", "w:snapToGrid", "w:spacing", "w:ind",
                 "w:contextualSpacing", "w:mirrorIndents", "w:suppressOverlap", "w:jc", "w:textDirection",
                 "w:textAlignment", "w:textboxTightWrap", "w:outlineLvl", "w:divId", "w:cnfStyle", "w:rPr", "w:sectPr",
                 "w:pPrChange")
        ppr.insert_element_before(bdr, *later)
        ppr.insert_element_before(shd, *later)

    @staticmethod
    def frame_picture(run) -> None:
        """A hairline frame around a picture, so light screenshots do not melt into the page."""
        for sppr in run._r.xpath(".//pic:spPr"):
            ln = OxmlElement("a:ln")
            ln.set("w", "9525")
            fill = OxmlElement("a:solidFill")
            clr = OxmlElement("a:srgbClr")
            clr.set("val", HEX_RULE)
            fill.append(clr)
            ln.append(fill)
            sppr.append(ln)

    def part_page(self, text: str) -> None:
        self.new_section()
        art = DESIGN / {"Part One: Using FeelKit": "part_1.png", "Part Two: Reference": "part_2.png"}.get(text, "none.png")
        if art.exists():
            self.float_picture(art, 21.0)
        else:
            self.doc.add_paragraph(text, style="Part Title")
        self.part_pending = True

    def heading(self, b: Block) -> None:
        if b.level == 1:
            text = f"Appendix {b.number}: {b.text}" if self.chapter.appendix else f"{b.number}  {b.text}"
        elif b.number:
            text = f"{b.number}  {b.text}"
        else:
            text = b.text
        if b.level == 1:
            # A chapter starts on its own page (and section), under the dark band. A part page before it already
            # started a section, so the chapter needs one more.
            self.new_section()
            numeral = chapter_numeral(self.chapter.label)
            self.float_picture(numeral, 5.4, 21.0 - 2.7 - 5.4 + 0.4, 2.1)
        p = self.doc.add_paragraph(style=f"Heading {b.level}")
        number = f"Appendix {b.number}" if (b.level == 1 and self.chapter.appendix) else b.number
        if number and text.startswith(number):
            n = p.add_run(number)
            n.font.color.rgb = COLOUR_ACCENT
            self.add_inline(p, text[len(number):])
        else:
            self.add_inline(p, text)
        self.add_bookmark(p, b.anchor)
        self.last_heading = p
        self.last_heading_level = b.level
        self.current_edition = ""
        if b.level == 1:
            rule = self.doc.add_paragraph()
            rppr = rule._p.get_or_add_pPr()
            rbdr = OxmlElement("w:pBdr")
            rtop = OxmlElement("w:top")
            rtop.set(qn("w:val"), "single")
            rtop.set(qn("w:sz"), "18")
            rtop.set(qn("w:space"), "1")
            rtop.set(qn("w:color"), HEX_ACCENT)
            rbdr.append(rtop)
            rppr.insert(0, rbdr)
            rule.paragraph_format.right_indent = Cm(TEXT_WIDTH_CM - 2.2)
            rule.paragraph_format.space_after = Pt(22)
            self.chapter_contents()
        planned = self.planned.get(b.anchor)
        if planned and b.level <= 2 and planned.number != b.number:
            self.warnings.append(f"{self.chapter.path.name}: {b.anchor} is {b.number} here but {planned.number} in the outline table of the Manual Plan")

    def paragraph(self, text: str, style: str | None = None):
        p = self.doc.add_paragraph(style=style)
        self.add_inline(p, text)
        if text.rstrip().endswith(":"):
            p.paragraph_format.keep_with_next = True
        return p

    def edition(self, text: str) -> None:
        """Only Pro is labelled, on its heading (never on a chapter title, which the running header copies). Reference
        entries also state their edition in words in the first row of their key facts card."""
        self.current_edition = text
        if text == "Pro" and self.last_heading is not None and self.last_heading_level >= 2:
            self.pro_band(self.last_heading, self.last_heading_level)

    def reference_chapter(self) -> bool:
        return self.chapter is not None and not self.chapter.appendix and int(self.chapter.label) >= 16

    def chapter_contents(self) -> None:
        """An "In this chapter" box under the chapter title, listing its sections."""
        blocks = sample_blocks(self.chapter) if self.sample else self.chapter.blocks
        sections = [b for b in blocks if b.kind == "heading" and b.level == 2]
        if len(sections) < 2:
            return
        table = self.doc.add_table(rows=1, cols=1)
        self.borders(table, vertical=False, size=0, colour="FFFFFF")
        self.cell_margins(table, 140, 140, 200, 200)
        self.fixed_layout(table, [TEXT_WIDTH_CM])
        cell = table.rows[0].cells[0]
        self.shade_cell(cell, HEX_SPEC)
        self.left_rule(cell)
        title = cell.paragraphs[0]
        title.style = self.doc.styles["Box Title"]
        spaced = title.add_run("IN THIS CHAPTER")
        spacing = OxmlElement("w:spacing")
        spacing.set(qn("w:val"), "20")
        spaced._r.get_or_add_rPr().append(spacing)
        for b in sections:
            p = cell.add_paragraph(style="Table Text")
            p.paragraph_format.space_after = Pt(2)
            n = p.add_run(f"{self.targets[b.anchor].number}   ")
            n.font.color.rgb = COLOUR_ACCENT
            n.bold = True
            p.add_run(b.text).font.size = Pt(9)
        spacer = self.doc.add_paragraph()
        spacer.paragraph_format.space_after = Pt(6)

    def new_list_num(self, style_name: str) -> str:
        numbering = self.doc.part.numbering_part.element
        style = self.doc.styles[style_name]
        num_pr = style.element.pPr.find(qn("w:numPr")) if style.element.pPr is not None else None
        base_num_id = num_pr.find(qn("w:numId")).get(qn("w:val"))
        abstract_id = None
        for num in numbering.findall(qn("w:num")):
            if num.get(qn("w:numId")) == base_num_id:
                abstract_id = num.find(qn("w:abstractNumId")).get(qn("w:val"))
        new_id = str(max(int(n.get(qn("w:numId"))) for n in numbering.findall(qn("w:num"))) + 1)
        num = OxmlElement("w:num")
        num.set(qn("w:numId"), new_id)
        abstract = OxmlElement("w:abstractNumId")
        abstract.set(qn("w:val"), abstract_id)
        num.append(abstract)
        override = OxmlElement("w:lvlOverride")
        override.set(qn("w:ilvl"), "0")
        start = OxmlElement("w:startOverride")
        start.set(qn("w:val"), "1")
        override.append(start)
        num.append(override)
        numbering.append(num)
        return new_id

    SPEC_RE = re.compile(r"^\*\*(.+?):\*\*\s*(.*)$")

    def spec_card(self, b: Block) -> bool:
        """A reference entry's key facts ("**Class:** ...") as a two-column card instead of bullets."""
        if not self.reference_chapter() or b.ordered or any(level != 0 for level, _, _ in b.items):
            return False
        pairs = [self.SPEC_RE.match(text) for _, text, _ in b.items]
        if not pairs or not all(pairs):
            return False
        if self.current_edition:
            edition = "Pro only" if self.current_edition == "Pro" else "Lite and Pro"
            pairs.insert(0, self.SPEC_RE.match(f"**Edition:** {edition}"))
        self.in_table = True
        # Four or more facts sit two to a row (label, value, label, value) to keep reference entries short.
        per_row = 2 if len(pairs) >= 4 else 1
        # Long values (C++ names, sentences) get a row of their own, after the short facts.
        short = [m for m in pairs if len(m.group(2)) <= 34] if per_row == 2 else pairs
        long = [m for m in pairs if m not in short]
        rows = [short[i:i + per_row] for i in range(0, len(short), per_row)] + [[m] for m in long]
        table = self.doc.add_table(rows=len(rows), cols=2 * per_row)
        self.borders(table, vertical=False, size=0, colour="FFFFFF")
        self.cell_margins(table, 40, 40, 120, 120)
        widths = [3.1, TEXT_WIDTH_CM / 2 - 3.1] * 2 if per_row == 2 else [4.2, TEXT_WIDTH_CM - 4.2]
        self.fixed_layout(table, widths)
        for row, facts in zip(table.rows, rows):
            for cell in row.cells:
                self.shade_cell(cell, HEX_SPEC)
                cell.paragraphs[0].style = self.doc.styles["Table Text"]
            if per_row == 2 and len(facts) == 1 and facts[0] in long:
                merged = row.cells[1].merge(row.cells[3])
                for extra in merged.paragraphs[1:]:
                    extra._p.getparent().remove(extra._p)
            for index, m in enumerate(facts):
                label = row.cells[2 * index].paragraphs[0]
                label.style = self.doc.styles["Table Text"]
                lr = label.add_run(m.group(1))
                lr.font.color.rgb = COLOUR_LABEL
                value = row.cells[2 * index + 1].paragraphs[0]
                value.style = self.doc.styles["Table Text"]
                text = m.group(2).strip()
                if text[:1].islower():
                    text = text[0].upper() + text[1:]
                if text.startswith("`") and text.endswith("`."):
                    text = text[:-1]
                self.add_inline(value, text)
            # The card stays on one page and on the same page as the property table after it.
            row._tr.get_or_add_trPr().append(OxmlElement("w:cantSplit"))
            for cell in row.cells:
                cell.paragraphs[0].paragraph_format.keep_with_next = True
        self.left_rule(table.rows[0].cells[0])
        for row in table.rows[1:]:
            self.left_rule(row.cells[0])
        self.in_table = False
        spacer = self.doc.add_paragraph()
        spacer.paragraph_format.space_after = Pt(4)
        spacer.paragraph_format.line_spacing = 0.8
        return True

    def list_block(self, b: Block) -> None:
        if self.spec_card(b):
            return
        continues = b.ordered and b.extra.isdigit() and int(b.extra) > 1 and self.last_num_id
        num_id = (self.last_num_id if continues else self.new_list_num("List Number")) if b.ordered else None
        if b.ordered:
            self.last_num_id = num_id
        for level, text, ordered in b.items:
            if ordered:
                style = "List Number" if level == 0 else "List Number 2"
            else:
                style = "List Bullet" if level == 0 else "List Bullet 2"
            p = self.doc.add_paragraph(style=style)
            p.paragraph_format.space_after = Pt(3)
            if ordered and level == 0 and num_id:
                ppr = p._p.get_or_add_pPr()
                num_pr = OxmlElement("w:numPr")
                ilvl = OxmlElement("w:ilvl")
                ilvl.set(qn("w:val"), "0")
                nid = OxmlElement("w:numId")
                nid.set(qn("w:val"), num_id)
                num_pr.append(ilvl)
                num_pr.append(nid)
                ppr.append(num_pr)
            self.add_inline(p, text)
        self.doc.paragraphs[-1].paragraph_format.space_after = Pt(8)

    def code(self, text: str) -> None:
        lines = text.split("\n")
        for idx, line in enumerate(lines):
            p = self.doc.add_paragraph(style="Code Block")
            p.add_run(line if line else " ")
            if idx == 0:
                p.paragraph_format.space_before = Pt(2)
            if idx == len(lines) - 1:
                p.paragraph_format.space_after = Pt(8)
            else:
                p.paragraph_format.keep_with_next = True

    def note(self, text: str) -> None:
        p = self.doc.add_paragraph(style="Note Box")
        label = p.add_run("Note   ")
        label.bold = True
        label.font.color.rgb = COLOUR_ACCENT
        self.add_inline(p, text)

    def draft(self, text: str) -> None:
        if self.sample:
            return
        p = self.doc.add_paragraph()
        r = p.add_run("Draft note: " + text)
        r.italic = True
        r.font.color.rgb = COLOUR_MUTED

    @staticmethod
    def cell_margins(table, top=50, bottom=50, left=90, right=90) -> None:
        tblpr = table._tbl.tblPr
        mar = OxmlElement("w:tblCellMar")
        for side, value in (("top", top), ("left", left), ("bottom", bottom), ("right", right)):
            el = OxmlElement(f"w:{side}")
            el.set(qn("w:w"), str(value))
            el.set(qn("w:type"), "dxa")
            mar.append(el)
        tblpr.append(mar)
        # Line the table's edge up with the text column (Word otherwise pulls it left by the cell margin).
        ind = OxmlElement("w:tblInd")
        ind.set(qn("w:w"), str(left))
        ind.set(qn("w:type"), "dxa")
        tblpr.append(ind)

    @staticmethod
    def borders(table, colour=HEX_RULE, size=4, vertical=True) -> None:
        tblpr = table._tbl.tblPr
        existing = tblpr.find(qn("w:tblBorders"))
        if existing is not None:
            tblpr.remove(existing)
        b = OxmlElement("w:tblBorders")
        for side in ("top", "left", "bottom", "right", "insideH", "insideV"):
            el = OxmlElement(f"w:{side}")
            if size == 0 or (not vertical and side in ("left", "right", "insideV")):
                el.set(qn("w:val"), "nil")
            else:
                el.set(qn("w:val"), "single")
                el.set(qn("w:sz"), str(size))
                el.set(qn("w:space"), "0")
                el.set(qn("w:color"), colour)
            b.append(el)
        tblpr.append(b)

    @staticmethod
    def fixed_layout(table, widths_cm: list[float]) -> None:
        table.autofit = False
        tblpr = table._tbl.tblPr
        layout = OxmlElement("w:tblLayout")
        layout.set(qn("w:type"), "fixed")
        tblpr.append(layout)
        grid = table._tbl.tblGrid
        for col, width in zip(grid.findall(qn("w:gridCol")), widths_cm):
            col.set(qn("w:w"), str(int(width / 2.54 * 1440)))
        for row in table.rows:
            for cell, width in zip(row.cells, widths_cm):
                cell.width = Cm(width)

    @staticmethod
    def cell_rule(cell, side: str, colour: str, size: int) -> None:
        """One border of a cell; size 0 removes it."""
        tcpr = cell._tc.get_or_add_tcPr()
        borders = tcpr.find(qn("w:tcBorders"))
        if borders is None:
            borders = OxmlElement("w:tcBorders")
            tcpr.append(borders)
        el = OxmlElement(f"w:{side}")
        if size == 0:
            el.set(qn("w:val"), "nil")
        else:
            el.set(qn("w:val"), "single")
            el.set(qn("w:sz"), str(size))
            el.set(qn("w:space"), "0")
            el.set(qn("w:color"), colour)
        borders.append(el)

    @staticmethod
    def left_rule(cell, colour: str = HEX_ACCENT, size: int = 24) -> None:
        """A colored bar down the left edge of a cell."""
        tcpr = cell._tc.get_or_add_tcPr()
        borders = OxmlElement("w:tcBorders")
        left = OxmlElement("w:left")
        left.set(qn("w:val"), "single")
        left.set(qn("w:sz"), str(size))
        left.set(qn("w:space"), "0")
        left.set(qn("w:color"), colour)
        borders.append(left)
        tcpr.append(borders)

    @staticmethod
    def shade_cell(cell, fill: str) -> None:
        shade(cell._tc.get_or_add_tcPr(), fill)

    @staticmethod
    def auto_widths(header: list[str], body: list[list[str]]) -> list[float]:
        """Column widths from the length of the text in each column, so a column of short names does not take half the
        page. Each column gets at least 14 percent and at most 60 percent."""
        def visible(text: str) -> int:
            return len(re.sub(r"\*\*|`|\[(?:see|See|ref|Ref):\s*\w+\]", "", text))
        cols = len(header)
        rows = [r for r in body if not r[0].startswith(">> ")]
        weights = []
        for c in range(cols):
            lengths = sorted(visible(r[c]) for r in rows if c < len(r)) or [visible(header[c])]
            typical = lengths[int(len(lengths) * 0.8)] if len(lengths) > 1 else lengths[0]
            weights.append(max(typical, visible(header[c]), 6) ** 0.8)
        total = sum(weights)
        widths = [min(max(w / total * 100, 14.0), 60.0) for w in weights]
        scale = 100 / sum(widths)
        return [w * scale for w in widths]

    def table(self, b: Block) -> None:
        header, body = b.rows[0], b.rows[1:]
        widths = b.widths or self.auto_widths(header, body)
        # A column that is empty in every row (such as Default for pins without defaults) is left out.
        keep = [i for i in range(len(header)) if i == 0 or any(not r[0].startswith(">> ") and i < len(r) and r[i].strip() for r in body)]
        if len(keep) < len(header):
            header = [header[i] for i in keep]
            widths = [widths[i] for i in keep]
            body = [r if r[0].startswith(">> ") else [r[i] for i in keep] for r in body]
        cols = len(header)
        total = sum(widths)
        widths_cm = [TEXT_WIDTH_CM * w / total for w in widths]
        self.in_table = True
        table = self.doc.add_table(rows=1 + len(body), cols=cols)
        table.style = self.doc.styles["Table Grid"]
        table.alignment = WD_TABLE_ALIGNMENT.LEFT
        self.borders(table, colour=HEX_TABLE_LINE, vertical=False)
        self.cell_margins(table, 80, 80, 100, 100)
        self.fixed_layout(table, widths_cm)
        hdr = table.rows[0]
        trpr = hdr._tr.get_or_add_trPr()
        repeat = OxmlElement("w:tblHeader")
        trpr.append(repeat)
        for cell, text in zip(hdr.cells, header):
            cell.paragraphs[0].style = self.doc.styles["Table Text"]
            self.add_inline(cell.paragraphs[0], text, bold=True)
            for r in cell.paragraphs[0].runs:
                r.font.color.rgb = COLOUR_HEADING
            cell.paragraphs[0].paragraph_format.keep_with_next = True
            self.cell_rule(cell, "bottom", HEX_TABLE_RULE, 12)
            self.cell_rule(cell, "top", "FFFFFF", 0)
        for r_index, row_cells in enumerate(body, start=1):
            row = table.rows[r_index]
            cant_split = OxmlElement("w:cantSplit")
            row._tr.get_or_add_trPr().append(cant_split)
            if row_cells[0].startswith(">> "):
                merged = row.cells[0].merge(row.cells[-1])
                merged.paragraphs[0].style = self.doc.styles["Table Text"]
                for extra in merged.paragraphs[1:]:
                    extra._p.getparent().remove(extra._p)
                self.add_inline(merged.paragraphs[0], row_cells[0][3:], bold=True)
                merged.paragraphs[0].paragraph_format.keep_with_next = True
                for r in merged.paragraphs[0].runs:
                    r.font.color.rgb = COLOUR_TABLE_HEAD
                    r.font.size = Pt(8.5)
                continue
            for cell, text in zip(row.cells, row_cells):
                cell.paragraphs[0].style = self.doc.styles["Table Text"]
                if text.startswith("**Pro.** "):
                    self.add_pro_label(cell.paragraphs[0], *LABEL_SIZES["cell"], gap=False)
                    cell.paragraphs[0].add_run(" ")
                    text = text[len("**Pro.** "):]
                self.add_inline(cell.paragraphs[0], text)
        self.in_table = False
        spacer = self.doc.add_paragraph()
        spacer.paragraph_format.space_after = Pt(4)
        spacer.paragraph_format.line_spacing = 0.8

    def box(self, lines: list[tuple[str, bool]], fill: str, border: str) -> None:
        table = self.doc.add_table(rows=1, cols=1)
        table.style = self.doc.styles["Table Grid"]
        self.borders(table, border, 8)
        self.cell_margins(table, 120, 120, 160, 160)
        self.fixed_layout(table, [TEXT_WIDTH_CM])
        cell = table.rows[0].cells[0]
        self.shade_cell(cell, fill)
        first = True
        for text, bold in lines:
            p = cell.paragraphs[0] if first else cell.add_paragraph()
            first = False
            p.style = self.doc.styles["Table Text"]
            p.paragraph_format.space_after = Pt(3)
            self.add_inline(p, text, bold=bold)

    def figure(self, b: Block) -> None:
        shot_id = b.anchor
        info = self.shots.get(shot_id)
        if info is None:
            raise BuildError(f"{self.chapter.path.name}: shot {shot_id} is not in the screenshot table of the Manual Plan")
        self.used_shots.append(shot_id)
        self.figure_in_chapter += 1
        png = SHOTS / f"{shot_id}.png"
        if png.exists():
            prepared, px_w = prepare_figure(png)
            width = min(Cm(TEXT_WIDTH_CM + 0.9), Cm(px_w * 2.54 / FIGURE_PPI))
            p = self.doc.add_paragraph()
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.paragraph_format.keep_with_next = True
            p.paragraph_format.space_before = Pt(6)
            p.paragraph_format.space_after = Pt(0)
            run = p.add_run()
            run.add_picture(str(prepared), width=width)
        else:
            kind = "Diagram" if shot_id.startswith("D") else "Screenshot"
            lines = [(f"{kind} {shot_id}, not captured yet", True), (info.get("on_screen", ""), False)]
            if info.get("state"):
                lines.append((f"State: {info['state']}", False))
            lines.append((f"Window: {info.get('window', '')}. Taken by: {info.get('method', '')}. Project: {info.get('project', '')}.", False))
            self.box(lines, HEX_ZEBRA, HEX_RULE)
        cap = self.doc.add_paragraph(style="Caption")
        cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        prefix = self.chapter.label
        lead = cap.add_run(f"Figure {prefix}.")
        lead.bold = True
        switch = " \\r 1" if self.figure_in_chapter == 1 else ""
        add_field(cap, f"SEQ Figure{switch} \\* ARABIC", str(self.figure_in_chapter), bold=True)
        cap.add_run("   ")
        self.add_inline(cap, b.text)

    def video(self, b: Block) -> None:
        info = self.videos.get(b.anchor)
        if info is None:
            raise BuildError(f"{self.chapter.path.name}: video {b.anchor} is not in the video table of the Manual Plan")
        lines = [(f"\u25B6  Video: {info['title']} (YouTube, about {info['minutes']} minutes)", True)]
        if info.get("url"):
            lines.append((f"[{info['url']}]({info['url']})", False))
        else:
            lines.append(("*The video will be added after publishing.*", False))
        self.box(lines, HEX_NOTE, HEX_ACCENT)
        self.doc.add_paragraph().paragraph_format.space_after = Pt(2)

    # --- whole document ----------------------------------------------------------------------

    def build(self, chapters: list[Chapter], all_chapters: list[Chapter] | None = None) -> None:
        self.targets = number_headings(all_chapters or chapters)
        self.planned = load_outline()
        self.title_page()
        part = None
        for ch in chapters:
            this_part = "Appendices" if ch.appendix else ("Part One: Using FeelKit" if int(ch.label) <= 15 else "Part Two: Reference")
            if this_part != part:
                self.part_page(this_part)
                part = this_part
            self.chapter = ch
            self.figure_in_chapter = 0
            self.last_heading = None
            for b in sample_blocks(ch) if self.sample else ch.blocks:
                if b.kind == "heading":
                    self.heading(b)
                elif b.kind == "para":
                    self.paragraph(b.text)
                elif b.kind == "list":
                    self.list_block(b)
                elif b.kind == "table":
                    self.table(b)
                elif b.kind == "code":
                    self.code(b.text)
                elif b.kind == "note":
                    self.note(b.text)
                elif b.kind == "shot":
                    self.figure(b)
                elif b.kind == "video":
                    self.video(b)
                elif b.kind == "edition":
                    self.edition(b.text)
                elif b.kind == "draft":
                    self.draft(b.text)


# ---------------------------------------------------------------------------------------------
# Package properties
# ---------------------------------------------------------------------------------------------

CORE_XML = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><dc:title>{title}</dc:title><dc:subject>{subject}</dc:subject><dc:creator>{author}</dc:creator><cp:keywords></cp:keywords><dc:description></dc:description><cp:lastModifiedBy>{author}</cp:lastModifiedBy><cp:revision>1</cp:revision><dcterms:created xsi:type="dcterms:W3CDTF">{now}</dcterms:created><dcterms:modified xsi:type="dcterms:W3CDTF">{now}</dcterms:modified></cp:coreProperties>"""

APP_XML = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"><Template>Normal.dotm</Template><TotalTime>0</TotalTime><Application>Microsoft Office Word</Application><DocSecurity>0</DocSecurity><ScaleCrop>false</ScaleCrop><Company></Company><LinksUpToDate>false</LinksUpToDate><SharedDoc>false</SharedDoc><HyperlinksChanged>false</HyperlinksChanged><AppVersion>16.0000</AppVersion></Properties>"""


def finalize_package(path: Path, version: str, keep_app: bool) -> None:
    """Writes the document properties and removes the leftovers of the python-docx template."""
    now = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).strftime("%Y-%m-%dT%H:%M:%SZ")
    tmp = path.with_suffix(".tmp")
    with zipfile.ZipFile(path) as zin, zipfile.ZipFile(tmp, "w", zipfile.ZIP_DEFLATED) as zout:
        names = zin.namelist()
        for name in names:
            data = zin.read(name)
            if name == "docProps/thumbnail.jpeg":
                continue
            if name == "docProps/core.xml":
                data = CORE_XML.format(title=TITLE, subject=f"Version {version}", author=AUTHOR, now=now).encode("utf-8")
            elif name == "docProps/app.xml":
                if keep_app:
                    text = data.decode("utf-8")
                    text = re.sub(r"<Company>.*?</Company>", "<Company></Company>", text)
                    text = re.sub(r"<Manager>.*?</Manager>", "", text)
                    data = text.encode("utf-8")
                else:
                    data = APP_XML.encode("utf-8")
            elif name == "_rels/.rels":
                text = data.decode("utf-8")
                text = re.sub(r'<Relationship [^>]*Target="docProps/thumbnail\.jpeg"[^>]*/>', "", text)
                data = text.encode("utf-8")
            elif name == "[Content_Types].xml":
                if not any(n.lower().endswith((".jpeg", ".jpg")) and n != "docProps/thumbnail.jpeg" for n in names):
                    text = data.decode("utf-8")
                    text = re.sub(r'<Default Extension="jpeg"[^>]*/>', "", text)
                    data = text.encode("utf-8")
            zout.writestr(zin.getinfo(name), data)
    # A sync client or virus scanner can hold a freshly written file for a moment; try again before giving up.
    import time
    for attempt in range(20):
        try:
            tmp.replace(path)
            return
        except PermissionError:
            if attempt == 19:
                raise
            time.sleep(0.5)


def fix_toc_labels(doc, toc_range) -> int:
    """Pro labels in the contents: Word switches them to the contents font. Restores the label's font, size, color,
    letter spacing and raise. A Pro heading has a tab before its label (it sits at the right end of the band); in the
    contents that tab would jump to the page number column, so it becomes three non-breaking spaces, as before D4.
    Works from the last label back, so replacing text does not move the labels still to do."""
    start, end = toc_range.Start, toc_range.End
    found = []
    r = doc.Range(start, end)
    find = r.Find
    find.ClearFormatting()
    find.Text = "PRO"
    find.MatchCase = True
    find.MatchWholeWord = True
    while find.Execute() and r.End <= end:
        found.append((r.Start, r.End))
        r.Collapse(0)
    green = COLOUR_PRO[2] * 65536 + COLOUR_PRO[1] * 256 + COLOUR_PRO[0]
    for a, b in reversed(found):
        before = doc.Range(a - 1, a)
        if before.Text == "\t":
            before.Text = NBSP * 3
            a, b = a + 2, b + 2
        label = doc.Range(a, b)
        label.Font.Name = FONT_HEADING
        label.Font.Size = 6.5
        label.Font.Color = green
        label.Font.Spacing = LABEL_TRACKING / 20
        label.Font.Position = 1
    return len(found)


def export_with_word(docx: Path, pdf: Path) -> None:
    import pythoncom
    import win32com.client

    pythoncom.CoInitialize()
    word = win32com.client.DispatchEx("Word.Application")
    try:
        word.Visible = False
        word.DisplayAlerts = 0
        doc = word.Documents.Open(str(docx), ConfirmConversions=False, ReadOnly=False, AddToRecentFiles=False, Visible=False)
        try:
            doc.Fields.Update()
            for index in range(1, doc.TablesOfContents.Count + 1):
                doc.TablesOfContents(index).Update()
            doc.Fields.Update()
            for index in range(1, doc.TablesOfContents.Count + 1):
                doc.TablesOfContents(index).UpdatePageNumbers()
                fix_toc_labels(doc, doc.TablesOfContents(index).Range)
            doc.Save()
            doc.ExportAsFixedFormat(OutputFileName=str(pdf), ExportFormat=17, OpenAfterExport=False, OptimizeFor=0,
                                    Range=0, From=1, To=1, Item=0, IncludeDocProps=True, KeepIRM=True,
                                    CreateBookmarks=1, DocStructureTags=True, BitmapMissingFonts=True, UseISO19005_1=False)
        finally:
            doc.Close(SaveChanges=0)
    finally:
        word.Quit()
        pythoncom.CoUninitialize()


def restore_pdf_pictures(docx: Path, pdf: Path) -> int:
    """Word's PDF export resamples every picture to 200 ppi JPEG, whatever the document's image settings say. This puts
    the document's own pictures (word/media) back in their place: each PDF picture is matched to the media file with the
    same proportions that looks the same when both are shrunk to a thumbnail."""
    import io
    import zipfile

    import fitz
    from PIL import Image, ImageChops

    def thumb(img: Image.Image) -> Image.Image:
        return img.convert("L").resize((48, 48), Image.BILINEAR)

    media = []
    with zipfile.ZipFile(docx) as z:
        for name in z.namelist():
            if name.startswith("word/media/"):
                data = z.read(name)
                img = Image.open(io.BytesIO(data))
                media.append((name, data, img.width / img.height, thumb(img)))
    doc = fitz.open(pdf)
    done: set[int] = set()
    for page in doc:
        for info in page.get_images(full=True):
            xref = info[0]
            if xref in done:
                continue
            done.add(xref)
            pix = fitz.Pixmap(doc, xref)
            if pix.n - pix.alpha >= 3 and pix.colorspace and pix.colorspace.n == 4:
                pix = fitz.Pixmap(fitz.csRGB, pix)
            img = Image.open(io.BytesIO(pix.tobytes("png")))
            ratio, small = img.width / img.height, thumb(img)
            best, best_diff = None, 1e9
            for name, data, media_ratio, media_small in media:
                if abs(media_ratio - ratio) / ratio > 0.02:
                    continue
                diff = sum(ImageChops.difference(small, media_small).getdata()) / (48 * 48)
                if diff < best_diff:
                    best, best_diff = (name, data), diff
            if best is None or best_diff > 12:
                continue
            # JPEG at a quality where screenshots show no loss (full color resolution), written over the picture's own
            # object so nothing else in the page changes.
            original = Image.open(io.BytesIO(best[1])).convert("RGB")
            buf = io.BytesIO()
            original.save(buf, "JPEG", quality=93, subsampling=0)
            doc.update_stream(xref, buf.getvalue(), compress=False)
            for key in ("DecodeParms", "SMask", "Decode", "Mask"):
                if doc.xref_get_key(xref, key)[0] != "null":
                    doc.xref_set_key(xref, key, "null")
            doc.xref_set_key(xref, "Filter", "/DCTDecode")
            doc.xref_set_key(xref, "ColorSpace", "/DeviceRGB")
            doc.xref_set_key(xref, "BitsPerComponent", "8")
            doc.xref_set_key(xref, "Width", str(original.width))
            doc.xref_set_key(xref, "Height", str(original.height))
    tmp = pdf.with_name(pdf.stem + "_full.pdf")
    doc.save(tmp, garbage=4, deflate=True)
    doc.close()
    tmp.replace(pdf)
    return len(done)


def chapter_numeral(label: str) -> Path:
    """A large pale chapter number (or appendix letter) for the top right of a chapter's first page."""
    from PIL import Image, ImageDraw, ImageFont
    FIGS.mkdir(parents=True, exist_ok=True)
    out = FIGS / f"numeral_{label}.png"
    text = label.zfill(2) if label.isdigit() else label
    font = ImageFont.truetype("C:/Windows/Fonts/seguisb.ttf", 520)
    box = ImageDraw.Draw(Image.new("RGB", (10, 10))).textbbox((0, 0), text, font=font)
    w, h = box[2] - box[0], box[3] - box[1]
    img = Image.new("RGB", (638, 638), (255, 255, 255))
    ImageDraw.Draw(img).text((638 - w - box[0], (638 - h) // 2 - box[1]), text, font=font,
                             fill=tuple(int(HEX_NUMERAL[i:i + 2], 16) for i in (0, 2, 4)))
    img.save(out, dpi=(300, 300))
    return out


def prepare_figure(png: Path) -> tuple[Path, float]:
    """A copy of the screenshot with rounded corners, a hairline edge and a soft shadow on white, for the page.
    Pictures taken by the diagnostics have twice the screen's pixels and say so in their resolution tag (330 instead of
    165); they are printed at the size of a screen picture with the extra detail kept. Large game pictures are stored as
    JPEG to keep the document small. Returns the prepared file and its width in screen pixels (margins included)."""
    from PIL import Image, ImageDraw, ImageFilter
    FIGS.mkdir(parents=True, exist_ok=True)
    source = Image.open(png)
    k = 2 if source.info.get("dpi", (FIGURE_PPI,))[0] >= FIGURE_PPI * 1.5 else 1
    shot = source.convert("RGB")
    radius, pad = 14 * k, 26 * k
    mask = Image.new("L", shot.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, shot.width - 1, shot.height - 1], radius, fill=255)
    canvas = Image.new("RGB", (shot.width + pad * 2, shot.height + pad * 2), (255, 255, 255))
    shadow = Image.new("L", canvas.size, 0)
    ImageDraw.Draw(shadow).rounded_rectangle([pad, pad + 6 * k, pad + shot.width, pad + shot.height + 6 * k], radius, fill=70)
    shadow = shadow.filter(ImageFilter.GaussianBlur(10 * k))
    canvas.paste(Image.new("RGB", canvas.size, (20, 26, 34)), (0, 0), shadow)
    canvas.paste(shot, (pad, pad), mask)
    ImageDraw.Draw(canvas).rounded_rectangle([pad, pad, pad + shot.width - 1, pad + shot.height - 1], radius,
                                             outline=(205, 210, 217), width=k)
    ppi = FIGURE_PPI * k
    # More than 330 pixels per inch on the page adds size, not detail.
    width_cm = min(TEXT_WIDTH_CM + 0.9, canvas.width / k * 2.54 / FIGURE_PPI)
    max_px = round(width_cm / 2.54 * 330)
    if canvas.width > max_px:
        ppi = ppi * max_px / canvas.width
        canvas = canvas.resize((max_px, round(canvas.height * max_px / canvas.width)), Image.LANCZOS)
    if png.stat().st_size > 2_500_000:
        out = FIGS / (png.stem + ".jpg")
        canvas.save(out, quality=92, subsampling=0, dpi=(ppi, ppi))
    else:
        out = FIGS / png.name
        canvas.save(out, dpi=(ppi, ppi))
    return out, (shot.width + pad * 2) / k


def sample_blocks(ch: Chapter) -> list:
    """The blocks of a chapter that go into the look sample (SAMPLE), always starting with the chapter heading."""
    ranges = SAMPLE.get(ch.label)
    if not ranges:
        return []
    out = []
    inside = False
    for index, b in enumerate(ch.blocks):
        anchor = b.anchor if b.kind == "heading" else None
        for start, stop in ranges:
            if anchor and anchor == stop:
                inside = False
            if (start is None and index == 0) or (anchor and anchor == start):
                inside = True
        if inside:
            out.append(b)
    return out


def plugin_version() -> str:
    try:
        return json.loads(PLUGIN.read_text(encoding="utf-8-sig"))["VersionName"]
    except (OSError, KeyError, ValueError):
        return "1.0.0"


def word_count(chapter: Chapter) -> int:
    words = 0
    for b in chapter.blocks:
        parts = [b.text]
        if b.kind == "list":
            parts = [item[1] for item in b.items]
        elif b.kind == "table":
            parts = [c for row in b.rows for c in row]
        for text in parts:
            text = re.sub(r"\[(see|See|ref|Ref|name|edition|draft|shot|video):[^\]]*\]", " x ", text or "")
            words += len(re.findall(r"[A-Za-z0-9][\w'.-]*", text))
    return words


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    parser = argparse.ArgumentParser(description="Build the FeelKit Manual.")
    parser.add_argument("--pdf", action="store_true", help="update fields in Word and export a PDF next to the .docx")
    parser.add_argument("--release", action="store_true", help="no draft line on the title page")
    parser.add_argument("--version", default=None, help="version printed in the manual (default: FeelKit.uplugin)")
    parser.add_argument("--sample", action="store_true", help="build the look sample: cover, contents and parts of chapters 2 and 16")
    args = parser.parse_args()
    version = args.version or plugin_version()
    OUT.mkdir(exist_ok=True)
    try:
        all_chapters = load_chapters()
        chapters = [ch for ch in all_chapters if ch.label in SAMPLE] if args.sample else all_chapters
        builder = Builder(version, args.release, args.sample)
        builder.build(chapters, all_chapters)
    except BuildError as error:
        print(f"Build stopped: {error}", file=sys.stderr)
        return 1
    builder.doc.core_properties.author = AUTHOR
    builder.doc.core_properties.title = TITLE
    docx = OUT / ("FeelKit_Manual_Look_v3.docx" if args.sample else f"FeelKit_Manual_{version}.docx")
    builder.doc.save(docx)
    finalize_package(docx, version, keep_app=False)
    print(f"wrote {docx}")
    if args.pdf:
        pdf = docx.with_suffix(".pdf")
        export_with_word(docx.resolve(), pdf.resolve())
        finalize_package(docx, version, keep_app=True)
        restored = restore_pdf_pictures(docx, pdf)
        print(f"wrote {pdf} ({restored} pictures at full resolution)")
    total = 0
    for ch in chapters:
        n = word_count(ch)
        total += n
        name = f"Appendix {ch.label}" if ch.appendix else f"Chapter {ch.label}"
        print(f"  {name:12} {ch.title:45} {n:6} words")
    print(f"  {'Total':58} {total:6} words")
    missing = [s for s in builder.used_shots if not (SHOTS / f"{s}.png").exists()]
    print(f"  figures: {len(builder.used_shots)}, not captured yet: {len(missing)}")
    for w in builder.warnings:
        print("warning:", w)
    return 0


if __name__ == "__main__":
    sys.exit(main())
