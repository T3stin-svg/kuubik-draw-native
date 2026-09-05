"""Verify the bounded ASCII VIEWPORT camera contract, not complete LAYOUT support."""

import argparse
from pathlib import Path
import subprocess

import ezdxf
from dxf_paperspace import make_fixture, read_records


def viewport_records(path):
    records = []
    for kind, tags in read_records(path):
        if kind != "VIEWPORT":
            continue
        current = {}
        for code, value in tags:
            current.setdefault(code, []).append(value)
        records.append(current)
    by_id = {int(record[69][0]): record for record in records}
    assert len(by_id) == len(records), "duplicate viewport IDs"
    return by_id


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writer", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    source = args.output / "input-a3-two-views.dxf"
    target = args.output / "viewport-roundtrip.dxf"
    make_fixture(source, direction_probe=True)
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
