"""Reads the outline, screenshot and video tables from the Manual Plan (Docs/FeelKit_3_Manual_Plan.md).

Each table follows a marker line such as <!-- table: shots -->. Columns are matched by their heading, so the tables stay
readable in the plan while the build and lint scripts get fixed keys.
"""
from pathlib import Path

ROOT = Path("B:/NewUE5Project")
PLAN = ROOT / "Docs" / "FeelKit_3_Manual_Plan.md"
MANUAL = ROOT / "Docs" / "Manual"

COLUMNS = {
    "outline": {"Anchor": "anchor", "Number": "number", "Title": "title"},
    "shots": {"ID": "id", "Chapter": "chapter", "Shows": "on_screen", "Window": "window", "State": "state",
              "Taken by": "method", "Project": "project", "Status": "status"},
    "videos": {"ID": "id", "Title": "title", "Minutes": "minutes", "Link": "url", "Linked from sections": "linked_from"},
}


def split_row(line: str) -> list[str]:
    cells = line.strip()
    if cells.startswith("|"):
        cells = cells[1:]
    if cells.endswith("|"):
        cells = cells[:-1]
    return [c.strip() for c in cells.split("|")]


def load_table(name: str) -> list[dict]:
    """Rows of the named table as dicts with the fixed keys from COLUMNS. Empty when the plan or table is missing."""
    if not PLAN.exists():
        return []
    lines = PLAN.read_text(encoding="utf-8").split("\n")
    marker = f"<!-- table: {name} -->"
    try:
        start = lines.index(marker) + 1
    except ValueError:
        return []
    while start < len(lines) and not lines[start].strip().startswith("|"):
        start += 1
    if start >= len(lines):
        return []
    headings = split_row(lines[start])
    keys = [COLUMNS[name].get(h, h) for h in headings]
    rows = []
    for line in lines[start + 2:]:
        if not line.strip().startswith("|"):
            break
        cells = split_row(line)
        rows.append(dict(zip(keys, cells + [""] * (len(keys) - len(cells)))))
    return rows


def table_lines(name: str) -> list[tuple[int, str]]:
    """(line number, text) of every row of the named table, for the lint."""
    if not PLAN.exists():
        return []
    lines = PLAN.read_text(encoding="utf-8").split("\n")
    marker = f"<!-- table: {name} -->"
    if marker not in lines:
        return []
    out = []
    for n in range(lines.index(marker) + 1, len(lines)):
        if lines[n].strip().startswith("|"):
            out.append((n + 1, lines[n]))
        elif out:
            break
    return out
