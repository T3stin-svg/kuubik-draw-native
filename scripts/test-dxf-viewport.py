"""Verify the bounded ASCII VIEWPORT camera contract, not complete LAYOUT support."""

import argparse
from pathlib import Path
import subprocess

import ezdxf


def viewport_records(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    assert len(lines) % 2 == 0, path.name
    records = []
    current = None
    for code, value in zip(lines[::2], lines[1::2]):
        code = int(code.strip())
        if code == 0:
            current = {} if value.strip() == "VIEWPORT" else None
            if current is not None:
                records.append(current)
        elif current is not None:
            current.setdefault(code, []).append(value.strip())
    by_id = {int(record[69][0]): record for record in records}
    assert len(by_id) == len(records), "duplicate viewport IDs"
    return by_id


def make_fixture(path):
    document = ezdxf.new("R2018")
    document.units = 4
    document.layers.new("VIEWPORTS", dxfattribs={"plot": 0})
    document.modelspace().add_line((0, 0), (5000, 0))
    document.layouts.rename("Layout1", "TEST")
    layout = document.layouts.get("TEST")
    layout.page_setup(size=(420, 297), margins=(0, 0, 0, 0), units="mm", scale=(1, 1))
    for index, height in enumerate((8000, 16000)):
        viewport = layout.add_viewport(
            center=(100 + index * 190, 148.5), size=(160, 160),
            view_center_point=(2500 - index * 1250, index * 800), view_height=height,
            dxfattribs={
                "view_twist_angle": index * 30,
                # Non-default XYZ tests the record parser, not a 3D application workflow.
                "view_direction_vector": (index, index * 2, 1 + index * 2),
                "view_target_point": (index * 100, index * 200, 0),
                "flags": 0x8000 | 0x200 | (0x4000 if index == 0 else 0),
            },
        )
        viewport.dxf.id = index + 2
        assert viewport.dxf.height / viewport.dxf.view_height == 1 / (50 + index * 50)
    audit = document.audit()
    assert not audit.errors and not audit.fixes, (audit.errors, audit.fixes)
    document.saveas(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writer", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    source = args.output / "input-a3-two-views.dxf"
    target = args.output / "viewport-roundtrip.dxf"
    make_fixture(source)
    subprocess.run([str(args.writer.resolve()), str(source.resolve()), str(target.resolve())], check=True)
    expected = viewport_records(source)
    actual = viewport_records(target)
    assert set(expected) == set(actual) == {1, 2, 3}, (set(expected), set(actual))
    # Require serialized tags: a high-level reader's defaults cannot mask omitted fields.
    for identifier, original in expected.items():
        written = actual[identifier]
        for code in (10, 20, 40, 41, 12, 22, 45, 51, 90, 68, 69):
            assert code in written, (identifier, "missing serialized group", code)
            wanted = float(original.get(code, [0])[0])
            assert abs(float(written[code][0]) - wanted) < 1e-8, (identifier, code, written[code], wanted)
        for code in (16, 26, 36, 17, 27, 37):
            wanted = float(original.get(code, [1 if code == 36 else 0])[0])
            assert code in written and abs(float(written[code][0]) - wanted) < 1e-8, (identifier, code)
    reopened = ezdxf.readfile(target)
    assert reopened.units == 4
    line, = reopened.modelspace().query("LINE")
    assert line.dxf.start.isclose((0, 0, 0)) and line.dxf.end.isclose((5000, 0, 0))
    print("PASS viewport camera records: main ID 1 + floating IDs 2/3, 1:50/1:100, 0/30 degrees, lock flags, native reopen")
    print("SCOPE camera records only; full layout ownership and application integration remain separate gates")


if __name__ == "__main__":
    main()
