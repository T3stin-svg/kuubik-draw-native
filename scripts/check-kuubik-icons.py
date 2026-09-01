#!/usr/bin/env python3
"""Static contract checks and an HTML contact sheet for Kuubik SVG icons."""

from __future__ import annotations

import argparse
import html
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


REQUIRED_KEYS = {
    "FileNew", "FileOpen", "FileSave", "FileSaveAs", "EditUndo", "EditRedo", "FilePrint",
    "DrawLine", "DrawPolyline", "DrawLineRectangle", "DrawCircle", "DrawArc", "DrawHatch",
    "ModifyMove", "ModifyDuplicate", "ModifyTrim", "ModifyTrim2", "ModifyCut", "ModifyOffset",
    "ModifyRotate", "ModifyMirror", "ModifyScale", "ModifyRound", "ModifyDeleteQuick",
    "DrawMText", "DrawText", "DimLinear", "DimAligned", "DimLinearHor", "DimLinearVer",
    "DimRadial", "DimDiametric", "DimAngular", "DimLeader",
    "LayersAdd", "LayersEdit", "LayersToggleView", "LayersToggleLock",
    "BlocksInsert", "BlocksCreate", "BlocksEdit", "BlocksExplode", "BlocksImport", "ModifyEntity",
    "InfoDist", "InfoAngle", "InfoArea", "InfoTotalLength",
    "EditCut", "EditCopy", "EditPaste", "ViewGrid", "RestrictOrthogonal", "SnapEnd", "SnapMiddle",
    "SnapCenter", "SnapIntersection", "ZoomIn", "ZoomOut", "ZoomPrevious", "ZoomWindow", "ZoomPan",
    "ZoomAuto", "FileExport", "FilePrintPreview", "FilePrintPDF",
}
MAPPING_PATTERN = re.compile(r'\{"([A-Za-z0-9]+)", "(:/icons/kuubik/[^"]+\.svg)"\}')
FORBIDDEN_TAGS = {"image", "script", "text", "tspan", "textPath", "foreignObject", "metadata"}
FORBIDDEN_CONTENT = re.compile(r"data:|<script\\b|<image\\b|<text\\b|<tspan\\b|<foreignObject\\b|<metadata\\b", re.I)


def local_name(tag: str) -> str:
    return tag.rsplit("}", 1)[-1]


def read_mappings(registry: Path) -> list[tuple[str, str]]:
    return MAPPING_PATTERN.findall(registry.read_text(encoding="utf-8"))


def validate(root: Path) -> tuple[list[str], list[tuple[str, str]]]:
    errors: list[str] = []
    icon_root = root / "librecad" / "res" / "icons"
    registry = root / "librecad" / "src" / "ui" / "kuubikiconregistry.cpp"
    qrc = icon_root / "icons.qrc"
    mappings = read_mappings(registry)
    keys = [key for key, _ in mappings]
    resources = [resource for _, resource in mappings]

    duplicate_keys = sorted({key for key in keys if keys.count(key) > 1})
    duplicate_resources = sorted({item for item in resources if resources.count(item) > 1})
    if duplicate_keys:
        errors.append("duplicate action mappings: " + ", ".join(duplicate_keys))
    if duplicate_resources:
        errors.append("duplicate icon mappings: " + ", ".join(duplicate_resources))

    mapped = set(keys)
    missing_keys = sorted(REQUIRED_KEYS - mapped)
    extra_keys = sorted(mapped - REQUIRED_KEYS)
    if missing_keys:
        errors.append("missing required action mappings: " + ", ".join(missing_keys))
    if extra_keys:
        errors.append("unexpected action mappings: " + ", ".join(extra_keys))

    try:
        qrc_files = {element.text for element in ET.parse(qrc).getroot().iter() if local_name(element.tag) == "file"}
    except ET.ParseError as exc:
        errors.append(f"invalid qrc XML: {exc}")
        qrc_files = set()

    expected_files = {resource.removeprefix(":/icons/") for resource in resources}
    missing_qrc = sorted(expected_files - qrc_files)
    if missing_qrc:
        errors.append("mapped resources missing from qrc: " + ", ".join(missing_qrc))

    src_pro = (root / "librecad" / "src" / "src.pro").read_text(encoding="utf-8")
    if "../res/icons/icons.qrc" not in src_pro or "ui/kuubikiconregistry.cpp" not in src_pro or "ui/kuubikiconregistry.h" not in src_pro:
        errors.append("src.pro missing Kuubik registry or icons.qrc reference")

    disk_files = {path.relative_to(icon_root).as_posix() for path in (icon_root / "kuubik").rglob("*.svg")}
    missing_disk = sorted(expected_files - disk_files)
    excess_disk = sorted(disk_files - expected_files)
    if missing_disk:
        errors.append("mapped resources missing on disk: " + ", ".join(missing_disk))
    if excess_disk:
        errors.append("unmapped Kuubik icons: " + ", ".join(excess_disk))

    for relative in sorted(disk_files):
        path = icon_root / relative
        content = path.read_text(encoding="utf-8")
        if FORBIDDEN_CONTENT.search(content):
            errors.append(f"forbidden SVG content: {relative}")
        try:
            svg_root = ET.fromstring(content)
        except ET.ParseError as exc:
            errors.append(f"invalid SVG XML {relative}: {exc}")
            continue
        if local_name(svg_root.tag) != "svg" or svg_root.attrib.get("viewBox") != "0 0 24 24":
            errors.append(f"invalid viewBox: {relative}")
        for element in svg_root.iter():
            if local_name(element.tag) in FORBIDDEN_TAGS:
                errors.append(f"forbidden SVG element: {relative}")
                break
            for attribute, value in element.attrib.items():
                if local_name(attribute) == "href" and ("://" in value or value.startswith("//") or not value.startswith("#")):
                    errors.append(f"external href: {relative}")
                    break

    return errors, mappings


def write_contact_sheet(root: Path, mappings: list[tuple[str, str]]) -> Path:
    output = root / "build" / "kuubik-icon-contact-sheet.html"
    output.parent.mkdir(parents=True, exist_ok=True)
    cards: list[str] = []
    for key, resource in mappings:
        source = "../librecad/res/icons/" + resource.removeprefix(":/icons/")
        sizes = "".join(f'<img src="{html.escape(source)}" width="{size}" height="{size}" alt="{html.escape(key)} {size}px">' for size in (16, 24, 32))
        cards.append(f'<article><strong>{html.escape(key)}</strong><div>{sizes}</div></article>')
    output.write_text("""<!doctype html><html><head><meta charset=\"utf-8\"><title>Kuubik icon contact sheet</title>
<style>body{margin:20px;font:12px system-ui;background:#eef2f5;color:#17212b}h1{font-size:20px}.sheet{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:8px}article{padding:8px;border:1px solid #cbd5df;border-radius:4px;background:#fff}article div{display:flex;align-items:center;gap:12px;min-height:40px;margin-top:6px}.dark{margin-top:28px;padding:20px;background:#1b232b;color:#dde6ed}.dark article{background:#252f38;border-color:#465360;color:#dde6ed}</style></head><body>
<h1>Kuubik technical-line icons — light</h1><section class=\"sheet\">""" + "".join(cards) + """</section><section class=\"dark\"><h1>Kuubik technical-line icons — dark ribbon</h1><div class=\"sheet\">""" + "".join(cards) + "</div></section></body></html>", encoding="utf-8")
    return output


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--contact-sheet", action="store_true", help="write build/kuubik-icon-contact-sheet.html")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    errors, mappings = validate(root)
    if args.contact_sheet:
        print("contact sheet:", write_contact_sheet(root, mappings))
    if errors:
        print("Kuubik icon validation failed:", file=sys.stderr)
        for error in errors:
            print("- " + error, file=sys.stderr)
        return 1
    print(f"Kuubik icon validation passed: {len(mappings)} mappings, {len(mappings)} SVGs.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
