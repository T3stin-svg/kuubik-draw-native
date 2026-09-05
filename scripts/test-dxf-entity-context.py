"""Check entity observation and 102 scope without involving application geometry."""
import argparse
from pathlib import Path
import struct
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)

    def run(name, records, expected, block=False):
        tags = [(0, "SECTION"), (2, "BLOCKS" if block else "ENTITIES")]
        if block:
            tags += [(0, "BLOCK"), (2, "TEST"), (70, 0)]
        tags += records
        if block:
            tags += [(0, "ENDBLK")]
        tags += [(0, "ENDSEC"), (0, "EOF")]
        path = args.output / f"{name}.dxf"
        path.write_text("".join(f"{code}\n{value}\n" for code, value in tags))
        result = subprocess.run([str(args.executable.resolve()), str(path.resolve())], capture_output=True, text=True)
        if expected is None:
            assert result.returncode != 0, name
        else:
            assert result.returncode == 0 and result.stdout.splitlines() == expected, (name, result)

    for kind in ("LINE", "POLYLINE", "DIMENSION", "SPLINE", "KUUBIK_UNKNOWN"):
        tags = [(0, kind), (5, "AB"), (330, "CD"), (67, 1),
                (102, "{OUTER"), (330, "11"), (102, "{INNER"), (67, 0), (1040, 2.5), (290, 1), (102, "}"), (102, "}")]
        run(kind, tags, ["171 205 1"])
        run(kind + "-block", tags, ["171 205 1"], block=True)
    polyline = [(0, "POLYLINE"), (5, "AB"), (330, "CD"), (67, 1), (70, 16),
                (0, "VERTEX"), (5, "AC"), (330, "AB"), (10, 1), (20, 2),
                (0, "SEQEND"), (5, "EF"), (330, "AB"), (8, "SEQEND-LAYER"),
                (0, "LINE"), (5, "FA"), (330, "FB")]
    run("polyline-sequence", polyline, ["171 205 1", "250 251 0"])
    run("polyline-sequence-block", polyline, ["171 205 1", "250 251 0"], block=True)
    for name, groups in {
        "unclosed": [(102, "{A")],
        "underflow": [(102, "}")],
        "invalid": [(102, "bad")],
        "deep": [(102, "{A")] * 65 + [(102, "}")] * 65,
        "wide-integer": [(102, "{A"), (160, 4294967296), (102, "}")],
        "invalid-integer": [(102, "{A"), (160, "invalid"), (102, "}")],
    }.items():
        for block in (False, True):
            run(name + str(block), [(0, "KUUBIK_UNKNOWN"), (5, "AB")] + groups, None, block=block)
    # The generic app-data writer must retain nested scope and hexadecimal handles.
    output = args.output / "roundtrip.dxf"
    subprocess.run([str(args.executable.resolve()), str((args.output / "LINE.dxf").resolve()), str(output.resolve())], check=True)
    lines = output.read_text().splitlines()
    tags = [(int(lines[i]), lines[i + 1].strip()) for i in range(0, len(lines), 2)]
    start = tags.index((102, "{OUTER"))
    assert tags[start:start + 9] == [(102, "{OUTER"), (330, "11"), (102, "{INNER"), (67, "0"), (1040, "2.5"), (290, "1"), (102, "}"), (102, "}"), (100, "AcDbLine")]
    binary = args.output / "appdata.bdxf"
    subprocess.run([str(args.executable.resolve()), str((args.output / "LINE.dxf").resolve()), str(binary.resolve())], check=True)
    payload = (struct.pack("<H", 102) + b"{INNER\0" + struct.pack("<Hh", 67, 0)
               + struct.pack("<Hd", 1040, 2.5) + struct.pack("<HB", 290, 1)
               + struct.pack("<H", 102) + b"}\0")
    assert binary.read_bytes().count(payload) == 1
    print("PASS 12 entity/unknown/block contexts, 12 rejected groups, nested ASCII and binary scalar payloads")


if __name__ == "__main__":
    main()
