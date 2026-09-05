"""Run the real libdxfrw adapter, then audit every output with ezdxf (test only)."""

import argparse
from pathlib import Path
import subprocess

from dxf_audit import require_audit_clean


def geometry(document):
    result = []
    for entity in document.modelspace():
        kind = entity.dxftype()
        if kind == "LINE":
            data = (tuple(entity.dxf.start), tuple(entity.dxf.end))
        elif kind == "CIRCLE":
            data = (tuple(entity.dxf.center), entity.dxf.radius)
        elif kind == "LWPOLYLINE":
            data = (entity.closed, tuple(entity.get_points("xy")))
        else:
            raise AssertionError(("unexpected geometry", kind))
        result.append((kind, entity.dxf.layer, data))
    return sorted(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writer", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    fixture = Path(__file__).resolve().parents[1] / "tests/fixtures/modelspace-audit-clean.dxf"
    source = require_audit_clean(fixture, 0)
    args.output.mkdir(parents=True, exist_ok=False)
    subprocess.run([str(args.writer.resolve()), str(fixture), str(args.output.resolve())], check=True)
    for name, count in (("one", 1), ("empty", 0), ("two", 2), ("reused", 1)):
        document = require_audit_clean(args.output / f"{name}.dxf", count)
        assert geometry(document) == geometry(source), name
        assert document.units == source.units == 4, name
        for plot in document.objects.query("PLOTSETTINGS"):
            assert (plot.dxf.left_margin, plot.dxf.bottom_margin,
                    plot.dxf.right_margin, plot.dxf.top_margin) == (1, 2, 3, 4), name
        print(f"PASS {name}: geometry/units/native reopen; audit 0 errors, 0 repairs; {count} plot settings")
    print("PASS clean input + 4 ASCII libdxfrw outputs")


if __name__ == "__main__":
    main()
