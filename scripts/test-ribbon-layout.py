"""Negative tests keep ribbon checks from accepting a hidden/non-native mockup."""

import copy
import importlib.util
from pathlib import Path
import unittest

SPEC = importlib.util.spec_from_file_location("ribbon_verifier", Path(__file__).with_name("verify-ribbon-layout.py"))
VERIFIER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERIFIER)


def fixture():
    panels, left = [], 0
    for title, width in VERIFIER.HOME.items():
        keys = sorted(VERIFIER.DIRECT_DRAW) if title == "Draw" else [title + "Native"]
        if title == "Groups":
            keys = []
        elif title == "Properties":
            keys = ["ModifyEntity", "PenSyncFromLayer", "PenPick", "PenPickResolved", "PenApply", "PenCopy"]
        controls = [{"key": key, "visible": True, "contained": True,
                     "nativeIdentity": True, "enabledMatchesNative": True,
                     "x": i * 40, "y": 2, "width": 24, "height": 22}
                    for i, key in enumerate(keys)]
        panels.append({"tab": "Home", "title": title, "referenceWidth": width,
                       "x": left, "y": 0, "width": width, "height": 96,
                       "pageWidth": 1920, "visible": True, "contained": True,
                       "titleVisible": True, "titleFits": True, "collapsed": False,
                       "unavailable": title == "Groups", "unavailableEnabled": False,
                       "unavailableTooltip": "Groups are unavailable", "controls": controls,
                       "embeddedControls": [{"visible": True, "contained": True, "width": 100, "height": 20}
                                            for _ in range(1 if title == "Layers" else 3 if title == "Properties" else 0)]})
        left += width
    interaction = {field: True for field in (
        "tabsPassed", "gridAvailable", "keyboardFocusReached", "spaceToggledNativeGrid",
        "spaceRestoredNativeGrid", "nativePresentationChangesPreserveKuubik",
        "nativeEnabledMirroredAndRestored", "nativeVisibilityPreserved", "tabRestored")}
    interaction.update(status="PASS", gridTriggerCount=2, tabs=[
        {"name": name, "activatedByMouse": True}
        for name in ("Home", "Insert", "Annotate", "View", "Manage", "Output")])
    return {"classicNativeToolbarsRestored": True, "classicPenActionsRestored": True,
            "ribbonInteraction": interaction,
            "ribbonLayout": {"implementation": "SARibbon", "version": "2.9.0",
                            "frameless": False, "barContained": True, "currentLayerInLayers": True,
                            "penInProperties": True,
                            "tabs": ["Home", "Insert", "Annotate", "View", "Manage", "Output"],
                            "panels": panels}}


class RibbonVerifierTests(unittest.TestCase):
    def test_valid_reference(self):
        self.assertEqual(VERIFIER.verify(fixture(), True)["status"], "PASS")

    def test_reject_false_positive_evidence(self):
        for change in (
            lambda p: p[0]["controls"][0].update(visible=False),
            lambda p: p[1]["controls"][0].update(visible=False),
            lambda p: p[0]["controls"][0].update(nativeIdentity=False),
            lambda p: p[0]["controls"][0].update(enabledMatchesNative=False),
            lambda p: p[0]["controls"][0].update(x=999),
            lambda p: p[1].update(x=0),
            lambda p: p[1].update(titleFits=False),
            lambda p: p[6].update(unavailableEnabled=True),
            lambda p: p[0].update(pageWidth=100),
            lambda p: p[0].update(collapsed=True),
            lambda p: p[5]["embeddedControls"][0].update(contained=False),
            lambda p: p[5].update(collapsed=True),
        ):
            data = copy.deepcopy(fixture())
            change(data["ribbonLayout"]["panels"])
            with self.subTest(change=change), self.assertRaises(ValueError):
                VERIFIER.verify(data, True)

    def test_reject_reference_boundary_drift(self):
        data = fixture()
        data["ribbonLayout"]["panels"][0]["width"] += 5
        with self.assertRaises(ValueError):
            VERIFIER.verify(data, True)

    def test_reject_failed_interactions(self):
        for field in ("keyboardFocusReached", "spaceToggledNativeGrid", "spaceRestoredNativeGrid",
                      "nativeEnabledMirroredAndRestored", "nativePresentationChangesPreserveKuubik"):
            data = fixture()
            data["ribbonInteraction"][field] = False
            with self.subTest(field=field), self.assertRaises(ValueError):
                VERIFIER.verify(data)
        data = fixture()
        data["classicNativeToolbarsRestored"] = False
        with self.assertRaises(ValueError):
            VERIFIER.verify(data)


if __name__ == "__main__":
    unittest.main()
