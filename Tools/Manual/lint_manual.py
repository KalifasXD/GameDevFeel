"""Checks the FeelKit Manual sources and the built document against the manual's writing rules.

Checks the Markdown in src/ (notes included), overrides.yaml, outline.csv, videos.csv and shots/shots.csv,
then the text and the properties of the newest .docx in out/ and of the PDF next to it, if there is one.

    python lint_manual.py              errors fail the run; draft markers and references to sections
                                       not written yet are warnings
    python lint_manual.py --release    draft markers and references to unwritten sections are errors too
    python lint_manual.py --sources    sources only, without the built files

Exit code 0 when there are no errors.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
import zipfile
from pathlib import Path

from manual_tables import MANUAL, PLAN, load_table, table_lines

HERE = Path(__file__).resolve().parent
SRC = MANUAL / "src"
OUT = MANUAL / "out"


def rel_path(path: Path) -> Path:
    for base in (MANUAL, HERE):
        try:
            return path.relative_to(base)
        except ValueError:
            pass
    return path

TITLE = "FeelKit Manual"
AUTHOR = "Billo"

# Words and phrases that make text read as marketing or as machine-written.
BANNED = [
    "seamless", "seamlessly", "robust", "leverage", "leverages", "leveraging", "comprehensive", "delve", "delves",
    "whether you're", "whether you are", "unlock", "unlocks", "unlocking", "elevate", "elevates", "empower",
    "empowers", "game-changing", "game changer", "game-changer", "ultimate", "revolutionary", "the best",
    "cutting-edge", "cutting edge", "state-of-the-art", "effortless", "effortlessly", "streamline", "streamlined",
    "harness", "supercharge", "unleash", "dive into", "deep dive", "next level", "powerful", "stunning",
    "world-class", "best-in-class", "tapestry", "plethora", "myriad", "embark", "look no further", "rest assured",
    "it's worth noting", "it is worth noting", "worth mentioning", "in conclusion", "in summary", "to summarise",
    "to summarize", "in this chapter", "in this section", "this chapter explains", "this chapter covers",
    "this chapter describes", "this section explains", "this section covers", "we will", "we'll", "let's",
    "as mentioned", "needless to say", "simply put", "at the end of the day", "a breeze", "out of the box",
    "take your game", "bring your game", "juicy", "magic", "magical",
]

UK_SPELLINGS = [
    "analyse", "analysed", "analyses", "authorise", "authorised", "behaviour", "behaviours", "cancelled",
    "cancelling", "catalogue", "catalogues", "categorise", "categorised", "centimetre", "centimetres", "centre",
    "centred", "centres", "centring", "colour", "coloured", "colouring", "colours", "customise", "customised",
    "customises", "emphasise", "emphasised", "favourite", "favourites", "finalise", "finalised", "grey", "greyed",
    "greys", "greyscale", "initialise", "initialised", "initialises", "judgement", "labelled", "labelling", "licence",
    "licences", "localisation", "localise", "localised", "maximise", "maximised", "metre", "metres", "millimetre",
    "millimetres", "minimise", "minimised", "modelled", "modelling", "normalise", "normalised", "normalises",
    "optimise", "optimised", "optimises", "organise", "organised", "organises", "prioritise", "prioritised",
    "realise", "realised", "realises", "recognise", "recognised", "recognises", "serialise", "serialised",
    "specialised", "stabilise", "stabilised", "standardised", "summarise", "summarised", "synchronise",
    "synchronised", "travelled", "travelling", "utilise", "visualise", "visualised", "visualises", "whilst",
]

CODE_RE = re.compile(r"\b(?:[A-Z]{1,4}-\d{3}|D-0\d\d|I-0\d\d)\b")
PHASE_RE = re.compile(r"\bPhase \d|\bPhase [1-9][A-E]\b")
EMOJI_RE = re.compile("[" + "".join([
    chr(0x1F000), "-", chr(0x1FAFF), chr(0x2600), "-", chr(0x27BF), chr(0xFE0F), chr(0x200D),
    chr(0x2B50), chr(0x2B55), chr(0x231A), chr(0x231B), chr(0x23E9), "-", chr(0x23FA),
]) + "]")
EM_DASH = chr(0x2014)
EN_DASH = chr(0x2013)
WRONG_PATH_RE = re.compile(r"Project Settings > FeelKit\b")


class Report:
    def __init__(self, release: bool):
        self.release = release
        self.errors: list[str] = []
        self.warnings: list[str] = []

    def error(self, where: str, rule: str, text: str) -> None:
        self.errors.append(f"{where}: {rule}: {text}")

    def warn(self, where: str, rule: str, text: str) -> None:
        if self.release:
            self.error(where, rule, text)
        else:
            self.warnings.append(f"{where}: {rule}: {text}")


def excerpt(line: str, start: int, end: int) -> str:
    a, b = max(0, start - 30), min(len(line), end + 30)
    return ("..." if a else "") + line[a:b].strip() + ("..." if b < len(line) else "")


def strip_code(line: str) -> str:
    return re.sub(r"`[^`]*`", lambda m: " " * len(m.group(0)), line)


def strip_bold(line: str) -> str:
    return re.sub(r"\*\*[^*]+\*\*", lambda m: " " * len(m.group(0)), line)


def check_text_line(report: Report, where: str, line: str, prose: bool) -> None:
    """Rules for any text that reaches the reader."""
    for m in CODE_RE.finditer(line):
        report.error(where, "internal code", excerpt(line, m.start(), m.end()))
    for m in PHASE_RE.finditer(line):
        report.error(where, "project phase number", excerpt(line, m.start(), m.end()))
    if EM_DASH in line:
        i = line.index(EM_DASH)
        report.error(where, "em dash", excerpt(line, i, i + 1))
    for m in re.finditer(r"\s" + EN_DASH + r"\s|\w" + EN_DASH + r"\s|\s" + EN_DASH + r"\w", line):
        report.error(where, "en dash used as a pause", excerpt(line, m.start(), m.end()))
    for m in EMOJI_RE.finditer(line):
        report.error(where, "emoji or pictograph", excerpt(line, m.start(), m.end()))
    if not prose:
        return
    plain = strip_code(line)
    low = plain.lower()
    for phrase in BANNED:
        for m in re.finditer(r"(?<![\w-])" + re.escape(phrase) + r"(?![\w-])", low):
            report.error(where, f"banned phrase '{phrase}'", excerpt(line, m.start(), m.end()))
    for m in re.finditer(r"(?<=\S)\s-\s", plain):
        report.error(where, "hyphen used as a pause", excerpt(line, m.start(), m.end()))
    for m in re.finditer(r"!(?=\s|$)", plain):
        report.error(where, "exclamation mark", excerpt(line, m.start(), m.end()))
    for m in WRONG_PATH_RE.finditer(plain):
        report.error(where, "settings path (use Project Settings > Plugins > FeelKit)", excerpt(line, m.start(), m.end()))
    words_only = strip_bold(plain)
    for word in UK_SPELLINGS:
        for m in re.finditer(r"(?<![\w/.-])" + re.escape(word) + r"(?![\w-])", words_only):
            report.error(where, f"British spelling '{word}' in prose (the manual uses American spelling)", excerpt(line, m.start(), m.end()))
    for m in re.finditer(r"\b(e\.g\.|i\.e\.|etc\.)", plain):
        report.warn(where, "abbreviation, write it out", excerpt(line, m.start(), m.end()))


def lint_markdown(report: Report, path: Path, anchors: set[str], planned: set[str]) -> None:
    rel = rel_path(path)
    in_code = False
    in_comment = False
    for n, line in enumerate(path.read_text(encoding="utf-8").split("\n"), 1):
        where = f"{rel}:{n}"
        stripped = line.strip()
        if stripped.startswith("```"):
            in_code = not in_code
            continue
        if "<!--" in line:
            in_comment = "-->" not in line
            continue
        if in_comment:
            in_comment = "-->" not in line
            continue
        if path.parent.name == "notes" and n <= 2:
            continue
        check_text_line(report, where, line, prose=not in_code)
        if in_code:
            continue
        for m in re.finditer(r"\[draft:[^\]]*\]", line):
            report.warn(where, "draft marker", m.group(0))
        for m in re.finditer(r"\[(?:see|See|ref|Ref|name):\s*(\w+)\]", line):
            if m.group(1) not in anchors:
                if m.group(1) in planned:
                    report.warn(where, "reference to a section not written yet", m.group(0))
                else:
                    report.error(where, "reference to an unknown anchor", m.group(0))
        for m in re.finditer(r"\[(?!see:|See:|ref:|Ref:|name:|draft:|shot:|video:|edition:)[a-z]+:\s*[^\]]*\]", line):
            report.error(where, "unknown marker", m.group(0))


def lint_plain_file(report: Report, path: Path) -> None:
    rel = rel_path(path)
    for n, line in enumerate(path.read_text(encoding="utf-8").split("\n"), 1):
        if path.suffix == ".yaml" and line.lstrip().startswith("#"):
            check_text_line(report, f"{rel}:{n}", line, prose=False)
            continue
        check_text_line(report, f"{rel}:{n}", line, prose=path.suffix in (".csv", ".yaml"))


def collect_anchors() -> tuple[set[str], set[str]]:
    anchors = set()
    for path in SRC.glob("*.md"):
        for m in re.finditer(r"^#{1,4} .*?\{#(\w+)\}\s*$", path.read_text(encoding="utf-8"), re.M):
            anchors.add(m.group(1))
    planned = set()
    planned = {row["anchor"] for row in load_table("outline")}
    return anchors, planned


def docx_text(path: Path) -> list[str]:
    with zipfile.ZipFile(path) as z:
        xml = z.read("word/document.xml").decode("utf-8")
    paragraphs = []
    for p in re.findall(r"<w:p[ >].*?</w:p>", xml, re.S):
        texts = re.findall(r"<w:t(?: [^>]*)?>([^<]*)</w:t>", p)
        line = "".join(texts)
        if line.strip():
            paragraphs.append(line.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", '"').replace("&apos;", "'"))
    return paragraphs


def lint_docx(report: Report, path: Path) -> None:
    rel = rel_path(path)
    with zipfile.ZipFile(path) as z:
        names = z.namelist()
        core = z.read("docProps/core.xml").decode("utf-8") if "docProps/core.xml" in names else ""
        for name in names:
            data = z.read(name)
            if b"python-docx" in data.lower():
                report.error(f"{rel}:{name}", "metadata", "contains 'python-docx'")
        if "docProps/thumbnail.jpeg" in names:
            report.error(f"{rel}", "metadata", "template thumbnail still in the package")
        app = z.read("docProps/app.xml").decode("utf-8") if "docProps/app.xml" in names else ""
        if "Macintosh" in app:
            report.error(f"{rel}:docProps/app.xml", "metadata", "template application name left in")

    def prop(tag: str) -> str | None:
        m = re.search(r"<" + tag + r"(?: [^>]*)?>(.*?)</" + tag + ">", core, re.S)
        if m:
            return m.group(1)
        if re.search(r"<" + tag + r"(?: [^>]*)?/>", core):
            return ""
        return None

    checks = [("dc:title", TITLE), ("dc:creator", AUTHOR), ("cp:lastModifiedBy", AUTHOR), ("dc:description", "")]
    for tag, expected in checks:
        value = prop(tag)
        label = {"dc:title": "Title", "dc:creator": "Author", "cp:lastModifiedBy": "Last Modified By", "dc:description": "Comments"}[tag]
        if value is None and expected == "":
            continue
        if value != expected:
            report.error(f"{rel}:docProps/core.xml", "metadata", f"{label} is '{value}', must be '{expected}'")
    for n, line in enumerate(docx_text(path), 1):
        if line.strip() == "IN THIS CHAPTER":
            continue  # the label of a chapter's contents box, a design element rather than prose
        check_text_line(report, f"{rel} paragraph {n}", line, prose=True)


def lint_pdf(report: Report, path: Path) -> None:
    rel = rel_path(path)
    try:
        from pypdf import PdfReader
    except ImportError:
        report.warnings.append(f"{rel}: pypdf is not installed, PDF properties not checked")
        return
    info = PdfReader(str(path)).metadata or {}
    if (info.get("/Title") or "") != TITLE:
        report.error(str(rel), "PDF metadata", f"Title is '{info.get('/Title')}'")
    if (info.get("/Author") or "") != AUTHOR:
        report.error(str(rel), "PDF metadata", f"Author is '{info.get('/Author')}'")
    for key, value in info.items():
        if "python-docx" in str(value).lower():
            report.error(str(rel), "PDF metadata", f"{key} contains 'python-docx'")


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    parser = argparse.ArgumentParser(description="Check the FeelKit Manual against its writing rules.")
    parser.add_argument("--release", action="store_true", help="treat draft markers and unwritten references as errors")
    parser.add_argument("--sources", action="store_true", help="check the sources only")
    args = parser.parse_args()
    report = Report(args.release)

    anchors, planned = collect_anchors()
    sources = sorted(SRC.rglob("*.md"))
    for path in sources:
        lint_markdown(report, path, anchors, planned)
    if (HERE / "overrides.yaml").exists():
        lint_plain_file(report, HERE / "overrides.yaml")
    for name in ("outline", "shots", "videos"):
        for n, line in table_lines(name):
            check_text_line(report, f"{PLAN.name}:{n}", line, prose=True)

    checked = [f"{len(sources)} Markdown files", "overrides.yaml", "the three Manual Plan tables"]
    if not args.sources:
        built = sorted(OUT.glob("FeelKit_Manual_*.docx"), key=lambda p: p.stat().st_mtime)
        if not built:
            report.error("out", "build", "no FeelKit_Manual_*.docx found; run build_manual.py first")
        else:
            docx = built[-1]
            newest_source = max(p.stat().st_mtime for p in sources)
            if docx.stat().st_mtime < newest_source:
                report.warnings.append(f"{docx.name} is older than the sources; build again before release")
            lint_docx(report, docx)
            checked.append(docx.name)
            pdf = docx.with_suffix(".pdf")
            if pdf.exists():
                lint_pdf(report, pdf)
                checked.append(pdf.name)

    for w in report.warnings:
        print("warning:", w)
    for e in report.errors:
        print("error:", e)
    print(f"Checked {', '.join(checked)}: {len(report.errors)} errors, {len(report.warnings)} warnings.")
    return 1 if report.errors else 0


if __name__ == "__main__":
    sys.exit(main())
