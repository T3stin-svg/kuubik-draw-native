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
    parser.add_argument("--binary", action="store_true", help="Bounded modern binary bool/geometry corpus")
    args = parser.parse_args()
    fixture = Path(__file__).resolve().parents[1] / "tests/fixtures/modelspace-audit-clean.dxf"
    source = require_audit_clean(fixture, 0)
    args.output.mkdir(parents=True, exist_ok=False)
    command = [str(args.writer.resolve()), str(fixture), str(args.output.resolve())]
    if args.binary:
        command.append("binary")
    subprocess.run(command, check=True)
    cases = ((f"{version}-{value}", 1) for version in ("AC1015", "AC1018", "AC1021", "AC1027", "AC1032") for value in ("default", 0, 1)) if args.binary else (
        ("one", 1), ("empty", 0), ("two", 2), ("reused", 1))
    for name, count in cases:
        document = require_audit_clean(args.output / f"{name}.dxf", count)
        assert geometry(document) == geometry(source), name
        assert document.units == source.units == 4, name
        if args.binary:
            from ezdxf.lldxf.tagger import binary_tags_loader
            raw = (args.output / f"{name}.dxf").read_bytes()
            assert raw.startswith(b"AutoCAD Binary DXF\r\n\x1a\x00"), name
            tags = list(binary_tags_loader(raw))
            assert tags[-1] == (0, "EOF"), name
            version, value = name.split("-")
            assert document.dxfversion == version, name
            flags = {"$LWDISPLAY": 0, "$XEDIT": 1, "$PSTYLEMODE": 1, "$EXTNAMES": 1, "$OLESTARTUP": 0}
            if version != "AC1015":
                flags["$XCLIPFRAME"] = 0
            if version not in ("AC1015", "AC1018"):
                flags.update({"$CAMERADISPLAY": 0, "$REALWORLDSCALE": 1})
            assert all(document.header[key] == (default if value == "default" else int(value)) for key, default in flags.items()), name
            if value == "default":
                assert "KUUBIK_BINARY" not in document.dimstyles, name
            else:
                assert document.dimstyles.get("KUUBIK_BINARY").dxf.dimfxlon == (0 if version in ("AC1015", "AC1018") else int(value)), name
        for plot in document.objects.query("PLOTSETTINGS"):
            assert (plot.dxf.left_margin, plot.dxf.bottom_margin,
                    plot.dxf.right_margin, plot.dxf.top_margin) == (1, 2, 3, 4), name
        print(f"PASS {name}: geometry/units/native reopen; audit 0 errors, 0 repairs; {count} plot settings")
    print("PASS clean input + " + ("15 binary bool outputs" if args.binary else "4 ASCII libdxfrw outputs"))


if __name__ == "__main__":
    main()
