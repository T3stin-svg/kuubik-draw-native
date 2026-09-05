"""Check native LAYOUT/BLOCK_RECORD reading against independent known records."""

import argparse
from pathlib import Path
import subprocess

from dxf_paperspace import make_fixture, read_records


def expected_rows(document):
    result = []
    for record in document.block_records:
        dxf = record.dxf
        result.append(["BLOCK_RECORD", dxf.name, int(dxf.handle, 16), int(dxf.owner, 16),
                       int(dxf.get("layout", "0"), 16), dxf.units])
    for layout in document.layouts:
        entity = layout.dxf_layout
        dxf = entity.dxf
        reactors = [int(handle, 16) for handle in entity.get_reactors()]
        row = ["LAYOUT", dxf.name, int(dxf.handle, 16), int(dxf.owner, 16),
               int(dxf.block_record_handle, 16), int(dxf.get("viewport_handle", "0"), 16),
               dxf.taborder, dxf.page_setup_name, dxf.paper_width, dxf.paper_height,
               dxf.plot_paper_units, dxf.plot_rotation, dxf.scale_numerator,
               dxf.scale_denominator, dxf.plot_layout_flags, dxf.layout_flags, len(reactors), *reactors]
        row.extend([dxf.plot_configuration_file, dxf.paper_size, dxf.plot_view_name,
                    dxf.left_margin, dxf.bottom_margin, dxf.right_margin, dxf.top_margin,
                    dxf.plot_origin_x_offset, dxf.plot_origin_y_offset, dxf.plot_type,
                    dxf.standard_scale_type, dxf.unit_factor, dxf.paper_image_origin_x,
                    dxf.paper_image_origin_y])
        for name in ("limmin", "limmax", "insert_base", "extmin", "extmax", "ucs_origin", "ucs_xaxis", "ucs_yaxis"):
            point = tuple(getattr(dxf, name))
            row.extend(point if len(point) == 3 else (*point, 0))
        row.extend([dxf.elevation, dxf.ucs_type, int(dxf.get("ucs_handle", "0"), 16),
                    int(dxf.get("base_ucs_handle", "0"), 16)])
        result.append(row)
    return result


def check_native(executable, path, expected):
    run = subprocess.run([str(executable.resolve()), str(path.resolve())], capture_output=True, text=True, encoding="utf-8")
    assert run.returncode == 0, (path.name, run.returncode, run.stderr)
    rows = [row.split("\t") for row in run.stdout.splitlines()]
    assert len(rows) == len(expected), (rows, expected)
    actual = {(row[0], row[1]): row for row in rows}
    assert len(actual) == len(rows), "duplicate callback record"
    for wanted in expected:
        row = actual[(wanted[0], wanted[1])]
        assert len(row) == len(wanted), (row, wanted)
        for found, value in zip(row, wanted):
            matches = found == value if isinstance(value, str) else float(found) == value
            assert matches, (row, wanted)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reader", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    source = args.output / "input-a3-two-views.dxf"
    document = make_fixture(source)
    expected = expected_rows(document)
    check_native(args.reader, source, expected)
    # Put unknown application data AFTER the real owner and before the subclasses.
    # Its repeated codes must not overwrite either owner, layout name or plot name.
    records = read_records(source)
    for kind, tags in records:
        if kind in ("LAYOUT", "BLOCK_RECORD"):
            position = next(i for i, tag in enumerate(tags) if tag[0] == 100)
            tags[position:position] = [(102, "{TEST_APP"), (330, "ABCD"),
                                      (100, "AcDbLayout"), (1, "wrong-name"),
                                      (102, "{NESTED"), (330, "BCDE"), (102, "}"), (102, "}")]
    guarded = args.output / "application-groups.dxf"
    guarded.write_text("".join(f"0\n{kind}\n" + "".join(f"{code}\n{value}\n" for code, value in tags)
                               for kind, tags in records), encoding="utf-8")
    check_native(args.reader, guarded, expected)
    ordinary = document.blocks.new("SYMBOL")
    ordinary.add_line((1, 2), (3, 4))
    document.saveas(args.output / "ordinary-block.dxf")
    check_native(args.reader, args.output / "ordinary-block.dxf", expected_rows(document))
    settings = document.layouts.get("TEST").dxf_layout.dxf
    settings.update({"page_setup_name": "A3-rotated", "plot_configuration_file": "Test printer",
                     "plot_view_name": "Test view", "paper_width": 297, "paper_height": 420,
                     "left_margin": 1, "bottom_margin": 2, "right_margin": 3, "top_margin": 4,
                     "plot_rotation": 1, "plot_paper_units": 0, "scale_numerator": 2,
                     "scale_denominator": 3, "plot_origin_x_offset": 5, "plot_origin_y_offset": 6,
                     "standard_scale_type": 25, "unit_factor": 1 / 25.4,
                     "paper_image_origin_x": 7, "paper_image_origin_y": 8,
                     "limmin": (-10, -20), "limmax": (300, 400), "insert_base": (1, 2, 3),
                     "extmin": (-1, -2, -3), "extmax": (400, 500, 600), "ucs_origin": (4, 5, 6),
                     "ucs_xaxis": (0, 1, 0), "ucs_yaxis": (-1, 0, 0), "elevation": 7, "ucs_type": 0})
    document.layouts.rename("TEST", "TEST-õ")
    document.saveas(args.output / "non-default-settings.dxf")
    check_native(args.reader, args.output / "non-default-settings.dxf", expected_rows(document))
    for label, group in (("unclosed", "102\n{BROKEN\n"), ("orphan-close", "102\n}\n"),
                         ("too-deep", "102\n{NESTED\n" * 65)):
        malformed = args.output / f"{label}.dxf"
        content = source.read_text(encoding="utf-8")
        content = content.replace("100\nAcDbLayout\n", group + "100\nAcDbLayout\n", 1)
        malformed.write_text(content, encoding="utf-8")
        failed = subprocess.run([str(args.reader.resolve()), str(malformed.resolve())], capture_output=True)
        assert failed.returncode == 2, (label, failed.returncode)
    for subclass in ("AcDbLayout", "AcDbBlockTableRecord"):
        for control in ("BROKEN", ""):
            malformed = args.output / f"invalid-{subclass}-{len(control)}.dxf"
            content = source.read_text(encoding="utf-8").replace(
                f"100\n{subclass}\n", f"102\n{control}\n330\nABCD\n100\n{subclass}\n", 1)
            malformed.write_text(content, encoding="utf-8")
            failed = subprocess.run([str(args.reader.resolve()), str(malformed.resolve())], capture_output=True)
            assert failed.returncode == 2, (subclass, control, failed.returncode)
    print("PASS native layout read: Model/TEST IDs, owners, block links, last viewport, A3/mm/1:1, independent flags/names")
    print("PASS owner/subclass guard: trailing unknown application group and nested misleading codes")
    print("PASS ordinary block reset and rejection of unclosed/orphan/excessive application groups")
    print("SCOPE read contract only; write ownership and native application paperspace are separate gates")


if __name__ == "__main__":
    main()
