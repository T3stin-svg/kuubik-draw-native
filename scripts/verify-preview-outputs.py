#!/usr/bin/env python3
"""Independent parser checks for the CI-generated DXF/PDF/SVG smoke files."""

import sys
import xml.etree.ElementTree as ET
import json
from pathlib import Path

import ezdxf
from pypdf import PdfReader


def main() -> None:
    smoke = Path(sys.argv[1])
    dxf_path = smoke / "preview-smoke.dxf"
    pdf_path = smoke / "preview-smoke.pdf"
    svg_path = smoke / "preview-smoke.svg"
    gui_dxf_path = smoke / "gui-evidence" / "line-gui-smoke.dxf"
    gui_report_path = smoke / "gui-evidence" / "line-gui-smoke.json"

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

    gui_doc = ezdxf.readfile(gui_dxf_path)
    gui_lines = [entity for entity in gui_doc.modelspace() if entity.dxftype() == "LINE"]
    assert len(gui_lines) == 1, [entity.dxftype() for entity in gui_doc.modelspace()]
    start = gui_lines[0].dxf.start
    end = gui_lines[0].dxf.end
    assert not start.isclose(end), (start, end)

    with gui_report_path.open(encoding="utf-8") as report_file:
        gui_report = json.load(report_file)
    assert gui_report["schemaVersion"] == 2, gui_report.get("schemaVersion")
    layer_selector = gui_report["layerSelector"]
    assert layer_selector["present"] is True
    assert layer_selector["enabled"] is True
    selected_layer = layer_selector["selectedLayer"]
    assert selected_layer == layer_selector["nativeCurrentLayer"]
    assert selected_layer == layer_selector["createdLineLayer"]
    assert gui_lines[0].dxf.layer == selected_layer, (
        gui_lines[0].dxf.layer,
        selected_layer,
    )

    properties_states = gui_report["propertiesStates"]
    expected_states = (("none", "none", 0), ("single", "single", 1), ("multiple", "multiple", 2))
    for name, mode, minimum_count in expected_states:
        state = properties_states[name]
        assert state["nativeCallback"] is True, (name, state)
        assert state["mode"] == mode, (name, state)
        assert state["count"] >= minimum_count, (name, state)
        assert "summary" in state, (name, state)
    assert properties_states["none"]["count"] == 0, properties_states["none"]
    single_summary = properties_states["single"]["summary"]
    assert single_summary["type"], single_summary
    assert single_summary["layer"], single_summary

    print(
        "Independent read-back passed: "
        f"{types}, one A4 PDF page, valid SVG, one non-zero GUI LINE on {selected_layer}"
    )


if __name__ == "__main__":
    main()
