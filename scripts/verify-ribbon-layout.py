"""Independent SARibbon geometry/read-back checks, not an AutoCAD parity score."""

import argparse
import json
from pathlib import Path

HOME = {
    "Draw": 225, "Modify": 250, "Annotation": 189, "Layers": 273,
    "Block": 161, "Properties": 262, "Groups": 72, "Utilities": 97,
    "Clipboard": 91, "View": 53,
}
DIRECT_DRAW = {"DrawLine", "DrawPolyline", "DrawCircle", "DrawArc"}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def verify(contract, exact_reference=False):
    require(contract["classicNativeToolbarsRestored"] is True,
            "Classic workspace did not regain its native toolbars")
    interaction = contract["ribbonInteraction"]
    for field in ("tabsPassed", "gridAvailable", "keyboardFocusReached",
                  "spaceToggledNativeGrid", "spaceRestoredNativeGrid",
                  "nativePresentationChangesPreserveKuubik",
                  "nativeEnabledMirroredAndRestored", "nativeVisibilityPreserved", "tabRestored"):
        require(interaction[field] is True, f"Native ribbon interaction failed: {field}")
    require(interaction["gridTriggerCount"] == 2, "Space must trigger the native grid toggle exactly twice")
    require(interaction["status"] == "PASS", "Native ribbon interaction failed")
    require([tab["name"] for tab in interaction["tabs"]]
            == ["Home", "Insert", "Annotate", "View", "Manage", "Output"]
            and all(tab["activatedByMouse"] is True for tab in interaction["tabs"]),
            "Every ribbon tab must be activated through its visible mouse target")
    layout = contract["ribbonLayout"]
    require(layout["implementation"] == "SARibbon" and layout["version"] == "2.9.0",
            "Unexpected ribbon implementation/version")
    require(layout["frameless"] is False, "Unexpected frameless dependency")
    require(layout["tabs"] == ["Home", "Insert", "Annotate", "View", "Manage", "Output"],
            "Native ribbon tab order changed")
    require(layout["currentLayerInLayers"] is True, "Native layer selector is outside Layers")
    require(layout["penInProperties"] is True, "Native pen toolbar is outside Properties")
    panels = [p for p in layout["panels"] if p["tab"] == "Home"]
    require([p["title"] for p in panels] == list(HOME), "Home panel order/identity changed")
    expected_right = 0
    actual_right = 0
    direct = set()
    boundary_deltas = {}
    for panel in panels:
        title = panel["title"]
        require(panel["referenceWidth"] == HOME[title], f"{title}: reference data changed")
        require(panel["visible"] is True and panel["contained"] is True,
                f"{title}: panel is hidden or clipped: {panel}")
        require(panel["titleVisible"] is True and panel["titleFits"] is True,
                f"{title}: panel label is hidden or clipped")
        require(panel["width"] > 0 and panel["height"] > 0, f"{title}: empty geometry")
        require(panel["x"] >= 0 and panel["x"] + panel["width"] <= panel["pageWidth"],
                f"{title}: geometry extends beyond its page")
        require(panel["x"] >= actual_right, f"{title}: panel overlaps its predecessor")
        actual_right = panel["x"] + panel["width"]
        expected_right += HOME[title]
        boundary_deltas[title] = actual_right - expected_right
        if exact_reference:
            require(panel["collapsed"] is False, f"{title}: collapsed in full-width reference")
            require(abs(boundary_deltas[title]) <= 2,
                    f"{title}: reference boundary delta {boundary_deltas[title]} > 2 px")
        if title == "Groups":
            require(panel["unavailable"] is True and panel["unavailableEnabled"] is False,
                    "Groups must not pretend to be implemented")
            require("unavailable" in panel["unavailableTooltip"].lower(),
                    "Groups must explain its limitation")
            require(not panel["controls"], "Groups must not alias another native command")
        else:
            require(panel["unavailable"] is False and bool(panel["controls"]),
                    f"{title}: unexpected unavailable panel")
        visible_rects = []
        for control in panel["controls"]:
            require(control["nativeIdentity"] is True and control["enabledMatchesNative"] is True,
                    f"{control['key']}: native action identity/enabled state diverged")
            if control["visible"]:
                require(control["contained"] is True, f"{control['key']}: clipped button")
                x, y, w, h = (control[k] for k in ("x", "y", "width", "height"))
                require(w > 0 and h > 0, f"{control['key']}: empty button")
                require(x >= 0 and y >= 0 and x + w <= panel["width"] and y + h <= panel["height"],
                        f"{control['key']}: measured button extends beyond its panel")
                for ox, oy, ow, oh in visible_rects:
                    require(x + w <= ox or ox + ow <= x or y + h <= oy or oy + oh <= y,
                            f"{title}: overlapping visible controls")
                visible_rects.append((x, y, w, h))
                if title == "Draw":
                    direct.add(control["key"])
        if title == "Draw":
            require(panel["collapsed"] is False and DIRECT_DRAW <= direct,
                    "Line, Polyline, Circle and Arc must all be directly visible")
    return {"status": "PASS", "exactReferenceBoundaries": exact_reference,
            "boundaryDeltas": boundary_deltas,
            "scope": "Native ribbon geometry/actions only; not full AutoCAD parity"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("contract", type=Path)
    parser.add_argument("--exact-reference", action="store_true")
    args = parser.parse_args()
    with args.contract.open(encoding="utf-8-sig") as source:
        contract = json.load(source)
    print(json.dumps(verify(contract, args.exact_reference), indent=2))


if __name__ == "__main__":
    main()
