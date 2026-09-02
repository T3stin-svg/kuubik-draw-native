#!/usr/bin/env python3
"""Independent parser checks for the CI-generated DXF/PDF/SVG smoke files."""

import sys
import xml.etree.ElementTree as ET
from collections import Counter
import json
import re
from pathlib import Path

import ezdxf
from PIL import Image, ImageStat
from pypdf import PdfReader


def require(condition: bool, message: object) -> None:
    if not condition:
        raise RuntimeError(message)


def require_ribbon_mouse_invocation(invocation: dict, context: str) -> None:
    for key in (
        "passed",
        "actionTriggeredByMouse",
        "sourceButtonEnabled",
        "sourceButtonIdentity",
    ):
        require(invocation[key] is True, (context, key, invocation))
    surface = invocation["invocationSurface"]
    if surface == "directButton":
        require(invocation["sourceButtonVisible"] is True, (context, invocation))
        require(invocation["panelCollapsed"] is False, (context, invocation))
    elif surface == "collapsedPanelOverflow":
        require(invocation["sourceButtonVisible"] is False, (context, invocation))
        for key in (
            "panelCollapsed",
            "overflowButtonPresent",
            "overflowButtonVisible",
            "overflowButtonEnabled",
            "overflowMenuPresent",
            "overflowActionIdentity",
            "overflowMenuInteractionRan",
            "overflowMenuVisibleAfterOpen",
            "overflowActionGeometryValid",
            "overflowActionAtPoint",
            "overflowMenuClosedAfterSelection",
        ):
            require(invocation[key] is True, (context, key, invocation))
    else:
        raise RuntimeError((context, "unexpected invocation surface", surface))


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
    copy_before_path = smoke / "gui-evidence" / "copy-before-undo.dxf"
    copy_undo_path = smoke / "gui-evidence" / "copy-after-undo.dxf"
    copy_redo_path = smoke / "gui-evidence" / "copy-after-redo.dxf"
    move_before_path = smoke / "gui-evidence" / "move-before-undo.dxf"
    move_undo_path = smoke / "gui-evidence" / "move-after-undo.dxf"
    move_redo_path = smoke / "gui-evidence" / "move-after-redo.dxf"
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
    require(gui_report["schemaVersion"] == 5, gui_report.get("schemaVersion"))
    require(gui_report["sourceDxfLoaded"] is True, gui_report.get("sourceDxfLoaded"))
    require_ribbon_mouse_invocation(
        gui_report["ribbonInvocation"], "ribbonInvocation"
    )
    require_ribbon_mouse_invocation(
        gui_report["propertiesLineRibbonInvocation"],
        "propertiesLineRibbonInvocation",
    )
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
    require_ribbon_mouse_invocation(
        polyline_undo_redo["ribbon"], "polylineUndoRedo.ribbon"
    )
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

    def read_copy_state(path: Path, expect_copy: bool):
        state_doc = ezdxf.readfile(path)
        state_entities = list(state_doc.modelspace())
        state_types = [entity.dxftype() for entity in state_entities]
        require(state_types.count("CIRCLE") == 1, (path.name, state_types))
        require(
            state_types.count("LINE") == (3 if expect_copy else 2),
            (path.name, state_types),
        )
        require(state_types.count("LWPOLYLINE") == 2, (path.name, state_types))
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
        fixture_lines = [
            entity for entity in state_entities
            if entity.dxftype() == "LINE" and entity.dxf.layer == "0"
        ]
        require(len(fixture_lines) == 1, (path.name, fixture_lines))
        require(
            fixture_lines[0].dxf.start.isclose((0.0, 0.0, 0.0)),
            (path.name, fixture_lines[0].dxf.start),
        )
        require(
            fixture_lines[0].dxf.end.isclose((120.0, 80.0, 0.0)),
            (path.name, fixture_lines[0].dxf.end),
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
        smoke_polylines = [
            entity for entity in state_entities
            if entity.dxftype() == "LWPOLYLINE"
            and entity.dxf.layer == "KUUBIK-SMOKE-LAYER"
        ]
        require(len(smoke_polylines) == 1, (path.name, smoke_polylines))
        require(not smoke_polylines[0].closed, path.name)
        smoke_polyline_points = [
            (round(point[0], 6), round(point[1], 6))
            for point in smoke_polylines[0].get_points("xy")
        ]
        require(
            smoke_polyline_points == before_points,
            (path.name, smoke_polyline_points, before_points),
        )
        smoke_lines = [
            entity for entity in state_entities
            if entity.dxftype() == "LINE"
            and entity.dxf.layer == "KUUBIK-SMOKE-LAYER"
        ]
        require(
            len(smoke_lines) == (2 if expect_copy else 1),
            (path.name, smoke_lines),
        )
        smoke_line_geometries = [
            (
                tuple(round(value, 6) for value in line.dxf.start),
                tuple(round(value, 6) for value in line.dxf.end),
            )
            for line in smoke_lines
        ]
        require(
            all(geometry == before_smoke_line for geometry in smoke_line_geometries),
            (path.name, smoke_line_geometries, before_smoke_line),
        )
        return smoke_line_geometries, smoke_polyline_points

    copy_before_lines, copy_before_polyline = read_copy_state(copy_before_path, True)
    copy_undo_lines, copy_undo_polyline = read_copy_state(copy_undo_path, False)
    copy_redo_lines, copy_redo_polyline = read_copy_state(copy_redo_path, True)
    require(copy_before_lines == copy_redo_lines, (copy_before_lines, copy_redo_lines))
    require(len(copy_undo_lines) == 1, copy_undo_lines)
    require(
        copy_before_polyline == copy_undo_polyline == copy_redo_polyline,
        (copy_before_polyline, copy_undo_polyline, copy_redo_polyline),
    )
    copy_undo_redo = gui_report["copyUndoRedo"]
    require(copy_undo_redo["passed"] is True, copy_undo_redo)
    require_ribbon_mouse_invocation(
        copy_undo_redo["ribbon"], "copyUndoRedo.ribbon"
    )
    require(
        copy_undo_redo["ribbon"]["actionKey"] == "ModifyDuplicate",
        copy_undo_redo["ribbon"],
    )
    require(
        copy_undo_redo["ribbon"]["nativeIdentity"] is True,
        copy_undo_redo["ribbon"],
    )
    require(
        copy_undo_redo["ribbon"]["nativeActionActive"] is True,
        copy_undo_redo["ribbon"],
    )
    require(
        copy_undo_redo["ribbon"]["activeActionType"]
        == copy_undo_redo["ribbon"]["expectedActionType"],
        copy_undo_redo["ribbon"],
    )
    copied_line = copy_undo_redo["copy"]
    for flag in (
        "created",
        "duplicateInPlace",
        "inPlaceForcedForSmoke",
        "sourceUnselectedBeforeAction",
        "canvasPointInside",
        "sourceDistinct",
        "startMatches",
        "endMatches",
        "entityUndoneAfterUndo",
    ):
        require(copied_line[flag] is True, (flag, copied_line))
    require(copied_line["candidateCount"] == 1, copied_line)
    require(copied_line["entityUndoneBeforeUndo"] is False, copied_line)
    require(copied_line["entityUndoneAfterRedo"] is False, copied_line)
    require(
        copied_line["activeLayerBeforeAction"]
        == copied_line["sourceLayer"]
        == copied_line["duplicateLayer"]
        == "KUUBIK-SMOKE-LAYER",
        copied_line,
    )
    canvas_point = copied_line["canvasPoint"]
    require(canvas_point["inside"] is True, canvas_point)
    for coordinate in ("graphX", "graphY", "guiX", "guiY"):
        require(isinstance(canvas_point[coordinate], (int, float)), canvas_point)
    for action_name in ("undo", "redo"):
        action_state = copy_undo_redo[action_name]
        require(action_state["actionTriggeredByMouse"] is True, action_state)
        require(action_state["firstLineStillActive"] is True, action_state)
        require(action_state["priorPolylineStillActive"] is True, action_state)

    def line_geometry(line):
        return (
            tuple(round(value, 6) for value in line.dxf.start),
            tuple(round(value, 6) for value in line.dxf.end),
        )

    def read_move_state(path: Path):
        state_doc = ezdxf.readfile(path)
        state_entities = list(state_doc.modelspace())
        state_types = [entity.dxftype() for entity in state_entities]
        require(state_types.count("CIRCLE") == 1, (path.name, state_types))
        require(state_types.count("LINE") == 4, (path.name, state_types))
        require(state_types.count("LWPOLYLINE") == 2, (path.name, state_types))
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
        fixture_lines = [
            entity for entity in state_entities
            if entity.dxftype() == "LINE" and entity.dxf.layer == "0"
        ]
        require(len(fixture_lines) == 1, (path.name, fixture_lines))
        require(
            fixture_lines[0].dxf.start.isclose((0.0, 0.0, 0.0)),
            (path.name, fixture_lines[0].dxf.start),
        )
        require(
            fixture_lines[0].dxf.end.isclose((120.0, 80.0, 0.0)),
            (path.name, fixture_lines[0].dxf.end),
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
        smoke_polylines = [
            entity for entity in state_entities
            if entity.dxftype() == "LWPOLYLINE"
            and entity.dxf.layer == "KUUBIK-SMOKE-LAYER"
        ]
        require(len(smoke_polylines) == 1, (path.name, smoke_polylines))
        smoke_polyline_points = [
            (round(point[0], 6), round(point[1], 6))
            for point in smoke_polylines[0].get_points("xy")
        ]
        require(
            smoke_polyline_points == before_points,
            (path.name, smoke_polyline_points, before_points),
        )
        smoke_lines = [
            entity for entity in state_entities
            if entity.dxftype() == "LINE"
            and entity.dxf.layer == "KUUBIK-SMOKE-LAYER"
        ]
        require(len(smoke_lines) == 3, (path.name, smoke_lines))
        geometries = Counter(line_geometry(line) for line in smoke_lines)
        require(
            geometries[before_smoke_line] == 2,
            (path.name, geometries, before_smoke_line),
        )
        return geometries

    move_before_lines = read_move_state(move_before_path)
    move_undo_lines = read_move_state(move_undo_path)
    move_redo_lines = read_move_state(move_redo_path)
    require(move_before_lines == move_redo_lines, (move_before_lines, move_redo_lines))
    source_only = list((move_undo_lines - move_before_lines).elements())
    moved_only = list((move_before_lines - move_undo_lines).elements())
    require(len(source_only) == 1, (move_undo_lines, move_before_lines, source_only))
    require(len(moved_only) == 1, (move_before_lines, move_undo_lines, moved_only))
    source_geometry = source_only[0]
    moved_geometry = moved_only[0]
    start_offset = tuple(
        round(moved_geometry[0][index] - source_geometry[0][index], 6)
        for index in range(3)
    )
    end_offset = tuple(
        round(moved_geometry[1][index] - source_geometry[1][index], 6)
        for index in range(3)
    )
    require(start_offset == end_offset, (source_geometry, moved_geometry))
    require(any(abs(value) > 1e-6 for value in start_offset[:2]), start_offset)

    move_undo_redo = gui_report["moveUndoRedo"]
    require(move_undo_redo["passed"] is True, move_undo_redo)
    require_ribbon_mouse_invocation(
        move_undo_redo["ribbon"], "moveUndoRedo.ribbon"
    )
    move_ribbon = move_undo_redo["ribbon"]
    require(move_ribbon["actionKey"] == "ModifyMove", move_ribbon)
    for flag in ("nativeIdentity", "selectionActionActive", "nativeActionActive"):
        require(move_ribbon[flag] is True, (flag, move_ribbon))
    require(
        move_ribbon["initialActionType"]
        == move_ribbon["expectedSelectionActionType"],
        move_ribbon,
    )
    require(
        move_ribbon["actionTypeAfterSelection"]
        == move_ribbon["expectedMoveActionType"],
        move_ribbon,
    )

    moved_line = move_undo_redo["move"]
    for flag in (
        "created",
        "sourceUnselectedBeforeAction",
        "sourceSelectedByCanvas",
        "offsetNonZero",
        "offsetMatchesBothEndpoints",
        "sourceUndoneBeforeUndo",
        "sourceActiveAfterUndo",
        "movedUndoneAfterUndo",
        "sourceUndoneAfterRedo",
        "movedActiveAfterRedo",
        "snapModeTemporarilyCleared",
        "snapModeRestored",
    ):
        require(moved_line[flag] is True, (flag, moved_line))
    require(moved_line["candidateCount"] == 1, moved_line)
    require(moved_line["movedUndoneBeforeUndo"] is False, moved_line)
    require(
        moved_line["activeLayerBeforeAction"]
        == moved_line["sourceLayer"]
        == moved_line["movedLayer"]
        == "KUUBIK-SMOKE-LAYER",
        moved_line,
    )
    require(
        moved_line["activeCountBeforeMove"]
        == moved_line["activeCountBeforeUndo"]
        == moved_line["activeCountAfterUndo"]
        == moved_line["activeCountAfterRedo"],
        moved_line,
    )
    report_source_geometry = (
        (
            round(float(moved_line["sourceStart"]["x"]), 6),
            round(float(moved_line["sourceStart"]["y"]), 6),
            0.0,
        ),
        (
            round(float(moved_line["sourceEnd"]["x"]), 6),
            round(float(moved_line["sourceEnd"]["y"]), 6),
            0.0,
        ),
    )
    report_moved_geometry = (
        (
            round(float(moved_line["movedStart"]["x"]), 6),
            round(float(moved_line["movedStart"]["y"]), 6),
            0.0,
        ),
        (
            round(float(moved_line["movedEnd"]["x"]), 6),
            round(float(moved_line["movedEnd"]["y"]), 6),
            0.0,
        ),
    )
    require(report_source_geometry == source_geometry, (report_source_geometry, source_geometry))
    require(report_moved_geometry == moved_geometry, (report_moved_geometry, moved_geometry))
    report_offset = (
        round(float(moved_line["offset"]["x"]), 6),
        round(float(moved_line["offset"]["y"]), 6),
        0.0,
    )
    require(report_offset == start_offset, (report_offset, start_offset))
    for point_name in (
        "selectionCanvasPoint",
        "referenceCanvasPoint",
        "targetCanvasPoint",
    ):
        point = moved_line[point_name]
        require(point["inside"] is True, (point_name, point))
        require(isinstance(point["x"], (int, float)), (point_name, point))
        require(isinstance(point["y"], (int, float)), (point_name, point))

    move_dialog = move_undo_redo["dialog"]
    require(move_dialog["objectName"] == "QG_DlgMove", move_dialog)
    for flag in (
        "timerRan",
        "found",
        "visible",
        "moveModeControlFound",
        "moveModeClickedByMouse",
        "moveModeSelected",
        "okFound",
        "okClickedByMouse",
        "acceptedByMouse",
    ):
        require(move_dialog[flag] is True, (flag, move_dialog))
    require(move_dialog["safetyTriggered"] is False, move_dialog)
    for action_name in ("undo", "redo"):
        action_state = move_undo_redo[action_name]
        for flag in (
            "actionTriggeredByMouse",
            "firstLineStillActive",
            "copyStillActive",
            "priorPolylineStillActive",
        ):
            require(action_state[flag] is True, (action_name, flag, action_state))

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
