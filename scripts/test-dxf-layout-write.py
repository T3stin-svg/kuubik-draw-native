"""Prove the bounded 2D layout exporter on synthetic drawings, before native integration."""

import argparse
import os
from pathlib import Path
import subprocess

import ezdxf
from dxf_layout_audit import check_layout_graph, one
from dxf_paperspace import make_fixture, read_records


def write_records(path, records):
    path.write_text("".join(f"0\n{kind}\n" + "".join(f"{code}\n{value}\n" for code, value in tags)
                            for kind, tags in records), encoding="utf-8")


def compare(source, target):
    expected, actual = check_layout_graph(source), check_layout_graph(target)
    assert not any(kind.startswith("IMAGE") for kind, _ in actual.values()), "stale legacy image state"
    for handle, (kind, scoped) in expected.items():
        if kind not in ("LAYOUT", "BLOCK_RECORD", "VIEWPORT", "LINE"):
            continue
        assert handle in actual and actual[handle][0] == kind, (kind, "identity changed", handle)
        written = actual[handle][1]
        if kind in ("LINE", "VIEWPORT"):
            for code, default in ((60, "0"), (48, "1")):
                assert float(one(scoped["AcDbEntity"], code, default)) == float(one(written["AcDbEntity"], code, default)), (kind, "entity state changed", code)
        if kind != "BLOCK_RECORD":
            assert one(scoped[""], 330) == one(written[""], 330), (kind, "owner changed")
        if kind == "LAYOUT":
            groups = {"AcDbPlotSettings": (1, 2, 4, 6, *range(40, 48), 142, 143, 70, 72, 73, 74, 75, 147, 148, 149),
                      "AcDbLayout": (1, 70, 71, 10, 20, 11, 21, 12, 22, 32, 14, 24, 34,
                                     15, 25, 35, 146, 13, 23, 33, 16, 26, 36, 17, 27, 37, 76, 330, 331)}
        elif kind == "BLOCK_RECORD":
            groups = {"AcDbBlockTableRecord": (2, 70, 340)}
        elif kind == "VIEWPORT":
            groups = {"AcDbViewport": (10, 20, 40, 41, 68, 69, 12, 22, 16, 26, 36, 17, 27, 37, 45, 51, 90)}
        else:
            groups = {"AcDbLine": (10, 20, 30, 11, 21, 31)}
        for subclass, codes in groups.items():
            for code in codes:
                default = "" if code in (1, 2, 4, 6) else "0"
                wanted = one(scoped[subclass], code, default)
                found = one(written[subclass], code, default)
                if code in (1, 2, 4, 6, 330, 331, 340):
                    assert found == wanted, (kind, code, found, wanted)
                else:
                    assert abs(float(found) - float(wanted)) <= 1e-8, (kind, code, found, wanted)
    document = ezdxf.readfile(target)
    audit = document.audit()
    assert not audit.errors and not audit.fixes, (target.name, audit.errors, audit.fixes)
    source_document = ezdxf.readfile(source)
    assert document.units == 4 and set(document.layouts.names()) == set(source_document.layouts.names())
    line, = document.modelspace().query("LINE")
    assert line.dxf.start.isclose((0, 0, 0)) and line.dxf.end.isclose((5000, 0, 0))
    paper, = [layout for layout in document.layouts if layout.name != "Model"]
    viewports = {viewport.dxf.id: viewport for viewport in paper.query("VIEWPORT")}
    assert set(viewports) == {1, 2, 3}
    assert viewports[2].dxf.view_height / viewports[2].dxf.height == 50
    assert viewports[3].dxf.view_height / viewports[3].dxf.height == 100


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writer", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    source = args.output / "input-a3-two-views.dxf"
    make_fixture(source)
    check_layout_graph(source)
    inputs = [source]
    customized = args.output / "custom-page-utf8.dxf"
    document = make_fixture(customized)
    settings = document.layouts.get("TEST").dxf_layout.dxf
    settings.update({"page_setup_name": "A3-rotated", "plot_configuration_file": "Test printer",
                     "plot_view_name": "Test view", "paper_width": 297, "paper_height": 420,
                     "left_margin": 1, "bottom_margin": 2, "right_margin": 3, "top_margin": 4,
                     "plot_rotation": 1, "scale_numerator": 2, "scale_denominator": 3,
                     "plot_origin_x_offset": 5, "plot_origin_y_offset": 6,
                     "standard_scale_type": 25, "unit_factor": 1 / 25.4,
                     "paper_image_origin_x": 7, "paper_image_origin_y": 8,
                     "limmin": (-10, -20), "limmax": (300, 400), "insert_base": (1, 2, 0),
                     "extmin": (-1, -2, 0), "extmax": (400, 500, 0), "ucs_origin": (4, 5, 0),
                     "ucs_xaxis": (0, 1, 0), "ucs_yaxis": (-1, 0, 0), "ucs_type": 0})
    document.layouts.rename("TEST", "TEST-õ")
    line, = document.modelspace().query("LINE")
    line.dxf.invisible = 1
    line.dxf.ltscale = 2.5
    list(document.layouts.get("TEST-õ").query("VIEWPORT"))[-1].dxf.invisible = 1
    document.saveas(customized)
    inputs.append(customized)
    sparse = args.output / "sparse-high-handles.dxf"
    records = read_records(source)
    for _, tags in records:
        for index, (code, value) in enumerate(tags):
            if code in (5, 105, 480, 481, 1005) or 320 <= code <= 369 or 390 <= code <= 399:
                if int(value, 16):
                    tags[index] = code, f"{0x100000 + 7 * int(value, 16):X}"
    write_records(sparse, records)
    inputs.append(sparse)
    collision = args.output / "retained-10020.dxf"
    records = read_records(source)
    for kind, tags in records:
        if kind == "LINE":
            tags[:] = [(code, "10020" if code == 5 else value) for code, value in tags]
        elif kind == "SECTION" and (2, "HEADER") in tags:
            position = tags.index((9, "$HANDSEED"))
            tags[position + 1] = 5, "10021"
    write_records(collision, records)
    inputs.append(collision)
    for original in inputs:
        check_layout_graph(original)
        previous = original
        for generation in (1, 2):
            target = args.output / f"{original.stem}-save-{generation}.dxf"
            target.write_bytes(b"sentinel drawing: replace only after successful serialization\n")
            subprocess.run([str(args.writer.resolve()), str(previous.resolve()), str(target.resolve())], check=True)
            compare(original, target)
            previous = target
    modes = ["duplicate-handle", "duplicate-viewport-id", "wrong-owner", "wrong-backlink", "wrong-last-viewport",
                 "missing-units", "inches", "nan-camera", "negative-camera", "perspective", "nonrectangular",
                 "nonplanar", "handle-overflow", "handle-exhaustion", "angle-overflow", "newline-name", "wrong-type", "unsupported-entity",
             "header-exception", "write-exception"]
    if os.name == "nt":
        modes.append("locked-target")
    for mode in modes:
        target = args.output / f"reject-{mode}.dxf"
        target.write_bytes(b"SENTINEL: a rejected export must leave these bytes intact\n")
        subprocess.run([str(args.writer.resolve()), str(source.resolve()), str(target.resolve()), mode], check=True)
    recovered = args.output / "legacy-image-retry.dxf"
    subprocess.run([str(args.writer.resolve()), str(source.resolve()), str(recovered.resolve()), "legacy-image-retry"], check=True)
    compare(source, recovered)
    # An existing directory cannot be replaced by a DXF file; preserve its contents.
    directory = args.output / "existing-directory.dxf"
    directory.mkdir()
    sentinel = directory / "keep.txt"
    sentinel.write_bytes(b"keep")
    result = subprocess.run([str(args.writer.resolve()), str(source.resolve()), str(directory.resolve()), "replace-failure"])
    assert result.returncode == 3 and sentinel.read_bytes() == b"keep"
    assert not list(args.output.glob("kdx*.tmp")) and not list(args.output.glob(".kuubik-*")), "temporary export leaked"
    print("PASS layout export: two saves, preserved identities/owners/cameras, sparse handles, raw graph + audit 0/0")
    print("PASS rejected inputs preserve destination; successful retry and legacy reuse; failed replacement cleans temporary")


if __name__ == "__main__":
    main()
