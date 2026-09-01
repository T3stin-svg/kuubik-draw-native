#!/usr/bin/env python3
"""Independent parser checks for the CI-generated DXF/PDF/SVG smoke files."""

import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import ezdxf
from pypdf import PdfReader


def main() -> None:
    smoke = Path(sys.argv[1])
    dxf_path = smoke / "preview-smoke.dxf"
    pdf_path = smoke / "preview-smoke.pdf"
    svg_path = smoke / "preview-smoke.svg"

    doc = ezdxf.readfile(dxf_path)
    types = [entity.dxftype() for entity in doc.modelspace()]
    assert types.count("LWPOLYLINE") == 1, types
    assert types.count("CIRCLE") == 1, types
    assert types.count("LINE") == 1, types
    assert "KUUBIK_TEST" in doc.layers, list(doc.layers)

    pdf = PdfReader(str(pdf_path))
    assert len(pdf.pages) == 1
    page = pdf.pages[0]
    assert float(page.mediabox.width) > 500
    assert float(page.mediabox.height) > 800
    contents = page.get_contents()
    assert contents is not None
    assert len(contents.get_data()) > 100

    root = ET.parse(svg_path).getroot()
    assert root.tag.endswith("svg"), root.tag
    assert len(list(root.iter())) > 3

    print(f"Independent read-back passed: {types}, one A4 PDF page, valid SVG")


if __name__ == "__main__":
    main()
