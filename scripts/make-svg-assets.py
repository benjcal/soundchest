#!/usr/bin/env python3
"""Generate clean brand SVGs from assets/branding/logo.svg.

logo.svg is an Inkscape document with the wordmark already converted to
outlines. This script strips the editor metadata, rebuilds the lockups with the
right text color for each background, and derives the tight icon and the square
app tile. Inkscape is only used to measure the chest path's bounding box.
"""

from __future__ import annotations

import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

SVG_NS = "http://www.w3.org/2000/svg"
ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets" / "branding" / "logo.svg"
OUT_DIR = ROOT / "assets" / "branding"

CHEST_ID = "path1-6"
WORDMARK_ID = "text4"
SOUND_ID = "path1"
CHEST_WORD_ID = "path2"

ACCENT = "#d79921"
LIGHT_TEXT = "#ebdbb2"
DARK_TEXT = "#282828"

# App tile geometry, in the 1024 canvas.
TILE_MARGIN = 64.0
TILE_RADIUS = 210.0
CHEST_WIDTH = 600.0

ICON_PADDING = 0.5


def element_by_id(root: ET.Element, element_id: str) -> ET.Element:
    for element in root.iter():
        if element.get("id") == element_id:
            return element
    raise SystemExit(f"error: no element with id {element_id!r} in {SOURCE}")


def parent_of(root: ET.Element, child: ET.Element) -> ET.Element | None:
    for element in root.iter():
        if any(candidate is child for candidate in element):
            return element
    return None


def measure_chest() -> tuple[float, float, float, float]:
    """Chest bounding box in SVG user units, via Inkscape."""
    args = [
        "inkscape",
        f"--query-id={CHEST_ID}",
        "--query-x",
        "--query-y",
        "--query-width",
        "--query-height",
        str(SOURCE),
    ]
    try:
        raw = subprocess.run(args, capture_output=True, text=True, check=True).stdout
    except FileNotFoundError:
        raise SystemExit("error: inkscape is required to measure the source logo")

    values = [float(line) for line in raw.splitlines() if line.strip()]
    if len(values) != 4:
        raise SystemExit(f"error: unexpected inkscape query output: {raw!r}")

    tree = ET.parse(SOURCE)
    view_box = tree.getroot().get("viewBox").split()
    view_width = float(view_box[2])
    doc_width = float(
        subprocess.run(
            ["inkscape", "--query-width", str(SOURCE)],
            capture_output=True,
            text=True,
            check=True,
        ).stdout.split()[0]
    )

    scale = view_width / doc_width
    return tuple(value * scale for value in values)


def new_svg(view_box: str) -> ET.Element:
    _, _, width, height = view_box.split()
    return ET.Element(
        "svg",
        {"xmlns": SVG_NS, "viewBox": view_box, "width": width, "height": height, "version": "1.1"},
    )


def add_path(parent: ET.Element, path_id: str, source: ET.Element, fill: str, transform: str | None) -> None:
    attrs = {"id": path_id, "d": source.get("d"), "fill": fill}
    if transform:
        attrs["transform"] = transform
    ET.SubElement(parent, "path", attrs)


def write_svg(root: ET.Element, name: str) -> None:
    ET.indent(root, space="  ")
    target = OUT_DIR / name
    target.write_text(
        '<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root, encoding="unicode") + "\n",
        encoding="utf-8",
    )
    print(f"wrote {target.relative_to(ROOT)}")


def build_lockup(chest: ET.Element, wordmark_paths: list[ET.Element], layer_transform: str, text_fill: str) -> ET.Element:
    root = new_svg("0 0 104.63644 11.980329")
    layer = ET.SubElement(root, "g", {"transform": layer_transform})
    add_path(layer, "chest", chest, ACCENT, None)

    wordmark = ET.SubElement(layer, "g", {"id": "wordmark"})
    for path_id, source in zip(("sound", "chest-word"), wordmark_paths):
        add_path(wordmark, path_id, source, text_fill, None)
    return root


def build_icon(chest: ET.Element, layer_transform: str, box: tuple[float, float, float, float]) -> ET.Element:
    x, y, width, height = box
    root = new_svg(f"0 0 {width + 2 * ICON_PADDING:.4f} {height + 2 * ICON_PADDING:.4f}")
    transform = f"translate({ICON_PADDING - x:.4f} {ICON_PADDING - y:.4f}) {layer_transform}"
    add_path(root, "chest", chest, ACCENT, transform)
    return root


def build_appicon(chest: ET.Element, layer_transform: str, box: tuple[float, float, float, float]) -> ET.Element:
    x, y, width, height = box
    scale = CHEST_WIDTH / width
    target_height = height * scale
    tile_size = 1024.0 - 2 * TILE_MARGIN

    root = new_svg("0 0 1024 1024")
    ET.SubElement(
        root,
        "rect",
        {
            "x": f"{TILE_MARGIN:.1f}",
            "y": f"{TILE_MARGIN:.1f}",
            "width": f"{tile_size:.1f}",
            "height": f"{tile_size:.1f}",
            "rx": f"{TILE_RADIUS:.1f}",
            "fill": DARK_TEXT,
        },
    )

    tx = (1024.0 - CHEST_WIDTH) / 2.0
    ty = (1024.0 - target_height) / 2.0
    transform = f"translate({tx - scale * x:.4f} {ty - scale * y:.4f}) scale({scale:.6f}) {layer_transform}"
    add_path(root, "chest", chest, ACCENT, transform)
    return root


def main() -> None:
    tree = ET.parse(SOURCE)
    source_root = tree.getroot()

    element_by_id(source_root, WORDMARK_ID)  # ensure the wordmark group is present

    chest = element_by_id(source_root, CHEST_ID)
    wordmark_paths = [
        element_by_id(source_root, SOUND_ID),
        element_by_id(source_root, CHEST_WORD_ID),
    ]
    parent = parent_of(source_root, chest)
    layer_transform = parent.get("transform") if parent is not None and parent.get("transform") else "translate(0 0)"

    box = measure_chest()
    print("chest bbox: " + ", ".join(f"{value:.4f}" for value in box))

    write_svg(build_lockup(chest, wordmark_paths, layer_transform, LIGHT_TEXT), "soundchest-lockup-on-dark.svg")
    write_svg(build_lockup(chest, wordmark_paths, layer_transform, DARK_TEXT), "soundchest-lockup-on-light.svg")
    write_svg(build_icon(chest, layer_transform, box), "soundchest-icon.svg")
    write_svg(build_appicon(chest, layer_transform, box), "soundchest-appicon.svg")


if __name__ == "__main__":
    sys.exit(main())
