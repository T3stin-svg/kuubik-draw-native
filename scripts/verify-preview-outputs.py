#!/usr/bin/env python3
"""Independent parser checks for the CI-generated DXF/PDF/SVG smoke files."""

import sys
import xml.etree.ElementTree as ET
import json
import re
from pathlib import Path

import ezdxf
from PIL import Image, ImageStat
from pypdf import PdfReader


def require(condition: bool, message: object) -> None:
    if not condition:
        raise RuntimeError(message)


def main() -> None:
    smoke = Path(sys.argv[1])
    dxf_path = smoke / "preview-smoke.dxf"
    pdf_path = smoke / "preview-smoke.pdf"
    svg_path = smoke / "preview-smoke.svg"
    gui_dxf_path = smoke / "gui-evidence" / "line-gui-smoke.dxf"
    gui_report_path = smoke / "gui-evidence" / "line-gui-smoke.json"
    pline_before_path = smoke / "gui-evidence" / "pline-before-undo.dxf"
    pline_undo_path = smoke / "gui-evidence" / "pline-after-undo.dxf"
    pline_redo_path = smoke / "gui-evidence" / "pline-after-redo.dxf"
    tool_options_root = smoke / "tool-options-evidence"
    tool_options_report_path = tool_options_root / "tool-options-1280.json"

    doc = ezdxf.readfile(dxf_path)
    modelspace = list(doc.modelspace())
    types = [entity.dxftype() for entity in modelspace]
    require(types.count("LWPOLYLINE") == 1, types)
    require(types.count("CIRCLE") == 1, types)
    require(types.count("LINE") == 1, types)
    require("KUUBIK_TEST" in doc.layers, list(doc.layers))
    polyline = next(entity for entity in modelspace if entity.dxftype() == "LWPOLYLINE")
    require(polyline.closed, "Fixture polyline is no longer closed")
    require(
        [(round(point[0], 6), round(point[1], 6)) for point in polyline.get_points("xy")]
        == [(0.0, 0.0), (120.0, 0.0), (120.0, 80.0), (0.0, 80.0)],
        list(polyline.get_points("xy")),
    )
    circle = next(entity for entity in modelspace if entity.dxftype() == "CIRCLE")
    require(circle.dxf.center.isclose((60.0, 40.0, 0.0)), circle.dxf.center)
    require(abs(circle.dxf.radius - 20.0) < 1e-9, circle.dxf.radius)
    fixture_line = next(entity for entity in modelspace if entity.dxftype() == "LINE")
    require(fixture_line.dxf.start.isclose((0.0, 0.0, 0.0)), fixture_line.dxf.start)
    require(fixture_line.dxf.end.isclose((120.0, 80.0, 0.0)), fixture_line.dxf.end)

    pdf = PdfReader(str(pdf_path))
    require(len(pdf.pages) == 1, f"Unexpected PDF page count: {len(pdf.pages)}")
    page = pdf.pages[0]
    require(float(page.mediabox.width) > 500, page.mediabox)
    require(float(page.mediabox.height) > 800, page.mediabox)
    contents = page.get_contents()
    require(contents is not None, "PDF has no page content stream")
    pdf_stream = contents.get_data()
    require(len(pdf_stream) > 100, len(pdf_stream))
    vector_operators = re.findall(rb"(?:^|\s)(?:m|l|c|re|S|s)(?:\s|$)", pdf_stream)
    require(len(vector_operators) >= 4, pdf_stream[:500])

    root = ET.parse(svg_path).getroot()
    require(root.tag.endswith("svg"), root.tag)
    svg_elements = list(root.iter())
    require(len(svg_elements) > 3, [element.tag for element in svg_elements])
    vector_tags = {
        element.tag.rsplit("}", 1)[-1]
        for element in svg_elements
    } & {"path", "line", "polyline", "polygon", "circle", "ellipse", "rect"}
    require(vector_tags, [element.tag for element in svg_elements])

    gui_doc = ezdxf.readfile(gui_dxf_path)
    gui_entities = list(gui_doc.modelspace())
    gui_types = [entity.dxftype() for entity in gui_entities]
    require(gui_types.count("LWPOLYLINE") == 1, gui_types)
    require(gui_types.count("CIRCLE") == 1, gui_types)
    require(gui_types.count("LINE") == 2, gui_types)
    require("KUUBIK_TEST" in gui_doc.layers, list(gui_doc.layers))
    gui_polyline = next(entity for entity in gui_entities if entity.dxftype() == "LWPOLYLINE")
    require(gui_polyline.dxf.layer == "KUUBIK_TEST", gui_polyline.dxf.layer)
    require(gui_polyline.closed, "Saved GUI polyline is no longer closed")
    require(
        [(round(point[0], 6), round(point[1], 6)) for point in gui_polyline.get_points("xy")]
        == [(0.0, 0.0), (120.0, 0.0), (120.0, 80.0), (0.0, 80.0)],
        list(gui_polyline.get_points("xy")),
    )
    gui_circle = next(entity for entity in gui_entities if entity.dxftype() == "CIRCLE")
    require(gui_circle.dxf.layer == "KUUBIK_TEST", gui_circle.dxf.layer)
    require(gui_circle.dxf.center.isclose((60.0, 40.0, 0.0)), gui_circle.dxf.center)
    require(abs(gui_circle.dxf.radius - 20.0) < 1e-9, gui_circle.dxf.radius)
    gui_lines = [entity for entity in gui_entities if entity.dxftype() == "LINE"]
    original_lines = [line for line in gui_lines if line.dxf.layer == "0"]
    require(len(original_lines) == 1, [(line.dxf.layer, line.dxf.start, line.dxf.end) for line in gui_lines])
    require(original_lines[0].dxf.start.isclose((0.0, 0.0, 0.0)), original_lines[0].dxf.start)
    require(original_lines[0].dxf.end.isclose((120.0, 80.0, 0.0)), original_lines[0].dxf.end)

    with gui_report_path.open(encoding="utf-8") as report_file:
        gui_report = json.load(report_file)
    require(gui_report["schemaVersion"] == 3, gui_report.get("schemaVersion"))
    require(gui_report["sourceDxfLoaded"] is True, gui_report.get("sourceDxfLoaded"))
    layer_selector = gui_report["layerSelector"]
    require(layer_selector["present"] is True, layer_selector)
    require(layer_selector["enabled"] is True, layer_selector)
    selected_layer = layer_selector["selectedLayer"]
    require(selected_layer == layer_selector["nativeCurrentLayer"], layer_selector)
    require(selected_layer == layer_selector["createdLineLayer"], layer_selector)
    created_lines = [line for line in gui_lines if line.dxf.layer == selected_layer]
    require(len(created_lines) == 1, [(line.dxf.layer, line.dxf.start, line.dxf.end) for line in gui_lines])
    start = created_lines[0].dxf.start
    end = created_lines[0].dxf.end
    require(not start.isclose(end), (start, end))
    require(gui_report["documentLifecycle"]["passed"] is True, gui_report["documentLifecycle"])
    full_properties = gui_report["fullPropertiesAction"]
    require(full_properties["actionKey"] == "ModifyEntity", full_properties)
    require(full_properties["nativeIdentity"] is True, full_properties)
    require(full_properties["nativeActionActive"] is True, full_properties)

    properties_states = gui_report["propertiesStates"]
    expected_states = (("document", "document", 0), ("single", "single", 1), ("multiple", "multiple", 2))
    for name, mode, minimum_count in expected_states:
        state = properties_states[name]
        require(state["nativeCallback"] is True, (name, state))
        require(state["mode"] == mode, (name, state))
        require(state["count"] >= minimum_count, (name, state))
        require("summary" in state, (name, state))
    require(properties_states["document"]["count"] == 0, properties_states["document"])
    single_summary = properties_states["single"]["summary"]
    require(single_summary["type"], single_summary)
    require(single_summary["layer"], single_summary)

    def read_pline_state(path: Path, expect_smoke_polyline: bool):
        state_doc = ezdxf.readfile(path)
        state_entities = list(state_doc.modelspace())
        state_types = [entity.dxftype() for entity in state_entities]
        require(state_types.count("CIRCLE") == 1, (path.name, state_types))
        require(state_types.count("LINE") == 2, (path.name, state_types))
        require(
            state_types.count("LWPOLYLINE") == (2 if expect_smoke_polyline else 1),
            (path.name, state_types),
        )
        require("KUUBIK_TEST" in state_doc.layers, (path.name, list(state_doc.layers)))
        require(
            "KUUBIK-SMOKE-LAYER" in state_doc.layers,
            (path.name, list(state_doc.layers)),
        )
        state_circle = next(
            entity for entity in state_entities if entity.dxftype() == "CIRCLE"
        )
        require(
            state_circle.dxf.center.isclose((60.0, 40.0, 0.0)),
            (path.name, state_circle.dxf.center),
        )
        require(
            abs(state_circle.dxf.radius - 20.0) < 1e-9,
            (path.name, state_circle.dxf.radius),
        )
        fixture_polylines = [
            entity for entity in state_entities
            if entity.dxftype() == "LWPOLYLINE" and entity.dxf.layer == "KUUBIK_TEST"
        ]
        require(len(fixture_polylines) == 1, (path.name, fixture_polylines))
        require(fixture_polylines[0].closed, path.name)
        fixture_points = [
            (round(point[0], 6), round(point[1], 6))
            for point in fixture_polylines[0].get_points("xy")
        ]
        require(
            fixture_points
            == [(0.0, 0.0), (120.0, 0.0), (120.0, 80.0), (0.0, 80.0)],
            (path.name, fixture_points),
        )
        smoke_lines = [
            entity for entity in state_entities
            if entity.dxftype() == "LINE" and entity.dxf.layer == "KUUBIK-SMOKE-LAYER"
        ]
        require(len(smoke_lines) == 1, (path.name, smoke_lines))
        require(not smoke_lines[0].dxf.start.isclose(smoke_lines[0].dxf.end), path.name)
        smoke_line_geometry = (
            tuple(round(value, 6) for value in smoke_lines[0].dxf.start),
            tuple(round(value, 6) for value in smoke_lines[0].dxf.end),
        )
        original_lines = [
            entity for entity in state_entities
            if entity.dxftype() == "LINE" and entity.dxf.layer == "0"
        ]
        require(len(original_lines) == 1, (path.name, original_lines))
        require(
            original_lines[0].dxf.start.isclose((0.0, 0.0, 0.0)),
            (path.name, original_lines[0].dxf.start),
        )
        require(
            original_lines[0].dxf.end.isclose((120.0, 80.0, 0.0)),
            (path.name, original_lines[0].dxf.end),
        )
        smoke_polylines = [
            entity for entity in state_entities
            if entity.dxftype() == "LWPOLYLINE"
            and entity.dxf.layer == "KUUBIK-SMOKE-LAYER"
        ]
        require(
            len(smoke_polylines) == (1 if expect_smoke_polyline else 0),
            (path.name, smoke_polylines),
        )
        if not smoke_polylines:
            return None, smoke_line_geometry
        smoke_polyline = smoke_polylines[0]
        require(not smoke_polyline.closed, path.name)
        points = [
            (round(point[0], 6), round(point[1], 6))
            for point in smoke_polyline.get_points("xy")
        ]
        require(len(points) == 3, (path.name, points))
        require(len(set(points)) == 3, (path.name, points))
        cross = (
            (points[1][0] - points[0][0]) * (points[2][1] - points[0][1])
            - (points[1][1] - points[0][1]) * (points[2][0] - points[0][0])
        )
        require(abs(cross) > 1e-6, (path.name, points, cross))
        return points, smoke_line_geometry

    before_points, before_smoke_line = read_pline_state(pline_before_path, True)
    undo_points, undo_smoke_line = read_pline_state(pline_undo_path, False)
    redo_points, redo_smoke_line = read_pline_state(pline_redo_path, True)
    require(undo_points is None, pline_undo_path)
    require(before_points == redo_points, (before_points, redo_points))
    require(
        before_smoke_line == undo_smoke_line == redo_smoke_line,
        (before_smoke_line, undo_smoke_line, redo_smoke_line),
    )
    polyline_undo_redo = gui_report["polylineUndoRedo"]
    require(polyline_undo_redo["passed"] is True, polyline_undo_redo)
    require(
        polyline_undo_redo["ribbon"]["actionTriggeredByMouse"] is True,
        polyline_undo_redo["ribbon"],
    )
    require(
        polyline_undo_redo["undo"]["actionTriggeredByMouse"] is True,
        polyline_undo_redo["undo"],
    )
    require(
        polyline_undo_redo["redo"]["actionTriggeredByMouse"] is True,
        polyline_undo_redo["redo"],
    )

    dpi_root = smoke / "dpi-evidence"
    for percent, factor, width, height in (
        ("100", 1.0, 1280, 600),
        ("125", 1.25, 1280, 600),
        ("150", 1.5, 1200, 600),
    ):
        dpi_directory = dpi_root / percent
        with (dpi_directory / "kuubik-ui-contract.json").open(encoding="utf-8") as contract_file:
            dpi_contract = json.load(contract_file)
        dpi = dpi_contract["dpi"]
        require(abs(float(dpi["devicePixelRatio"]) - factor) <= 0.06, dpi)
        require(int(dpi["windowLogicalWidth"]) == width, dpi)
        require(int(dpi["windowLogicalHeight"]) == height, dpi)
        screenshot_path = dpi_directory / "workspace.png"
        with Image.open(screenshot_path) as screenshot:
            require(screenshot.format == "PNG", screenshot.format)
            require(screenshot.size == (round(width * factor), round(height * factor)), screenshot.size)
            rgb = screenshot.convert("RGB")
            deviation = ImageStat.Stat(rgb).stddev
            require(max(deviation) > 5.0, (percent, deviation))

    with tool_options_report_path.open(encoding="utf-8") as report_file:
        tool_options_report = json.load(report_file)
    require(tool_options_report["schemaVersion"] == 1, tool_options_report)
    require(tool_options_report["status"] == "PASS", tool_options_report)
    require(tool_options_report["platform"] == "windows", tool_options_report)
    require(tool_options_report["windowWidth"] == 1280, tool_options_report)
    require(tool_options_report["windowHeight"] == 600, tool_options_report)
    toolbar = tool_options_report["optionsToolbar"]
    require(toolbar["objectName"] == "options_toolbar", toolbar)
    require(toolbar["hostObjectName"] == "kuubikOptionToolbarHost", toolbar)
    for flag in (
        "present", "visible", "hostPresent", "hostVisible",
        "nativeToolbarInRibbon", "directChildOfHost", "containedByHost",
        "containedThroughWindowAncestors",
    ):
        require(toolbar[flag] is True, (flag, toolbar))

    expected_tool_options = {
        "DrawLine": (["Ui_LineOptions"], "tool-options-line-1280.png"),
        "DimLinear": (
            ["Ui_DimOptions", "Ui_DimLinearOptions"],
            "tool-options-dimlinear-1280.png",
        ),
    }
    states = {state["actionKey"]: state for state in tool_options_report["states"]}
    require(set(states) == set(expected_tool_options), states)
    for action_key, (expected_widgets, screenshot_name) in expected_tool_options.items():
        state = states[action_key]
        for flag in (
            "actionPresent", "actionEnabled", "ribbonIdentity",
            "nativeActionActive", "optionsToolbarPositiveSize",
            "optionsHostPositiveSize", "optionsToolbarContainedByHost",
            "optionsToolbarContainedThroughWindowAncestors",
            "settledWidgetCounts", "screenshotSaved", "passed",
        ):
            require(state[flag] is True, (action_key, flag, state))
        require(state["activeActionType"] == state["expectedActionType"], state)
        require(abs(float(state["screenshotDevicePixelRatio"]) - 1.0) <= 0.06, state)
        expected_counts = {
            "line": 1 if action_key == "DrawLine" else 0,
            "dimension": 1 if action_key == "DimLinear" else 0,
            "dimLinear": 1 if action_key == "DimLinear" else 0,
        }
        require(state["visibleNativeWidgetCounts"] == expected_counts, state)
        require(state["optionsToolbarGeometry"]["width"] > 0, state)
        require(state["optionsToolbarGeometry"]["height"] > 0, state)
        require(state["optionsHostGeometry"]["width"] > 0, state)
        require(state["optionsHostGeometry"]["height"] > 0, state)
        widgets = {widget["objectName"]: widget for widget in state["widgets"]}
        require(set(widgets) == set(expected_widgets), widgets)
        for widget_name in expected_widgets:
            widget = widgets[widget_name]
            for flag in (
                "present", "nativeType", "visible", "containedByToolbar",
                "containedByHost", "containedByWindow",
                "containedThroughWindowAncestors", "positiveSize", "passed",
            ):
                require(widget[flag] is True, (action_key, widget_name, flag, widget))
            require(widget["geometry"]["width"] > 0, widget)
            require(widget["geometry"]["height"] > 0, widget)
        screenshot_path = tool_options_root / screenshot_name
        with Image.open(screenshot_path) as screenshot:
            require(screenshot.format == "PNG", screenshot.format)
            require(screenshot.size == (1280, 600), screenshot.size)
            rgb = screenshot.convert("RGB")
            deviation = ImageStat.Stat(rgb).stddev
            require(max(deviation) > 5.0, (action_key, deviation))

    print(
        "Independent read-back passed: "
        f"{types}, one A4 PDF page with vector operators, valid SVG vectors, "
        f"one new non-zero GUI LINE on {selected_layer} after native reopen, "
        "one open three-vertex GUI PLINE removed/restored by native QAT Undo/Redo, "
        "and visible, geometrically contained native LINE/DIMLINEAR Tool Options "
        "at 1280x600"
    )


if __name__ == "__main__":
    main()
