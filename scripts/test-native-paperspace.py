"""Exercise native refusal of drawing saves that would discard paperspace."""

import argparse
import json
from pathlib import Path
import subprocess

import ezdxf
from dxf_paperspace import make_fixture


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--case", help="Run one named input during focused debugging")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    layout_path = args.output / "layout-a3.dxf"
    make_fixture(layout_path)
    inputs = [layout_path]
    # A valid R12 drawing has no LAYOUT records; paper entities need their own gate.
    for kind in ("line", "viewport", "mesh", "ordinate", "unknown", "nested"):
        document = ezdxf.new("R12")
        document.modelspace().add_line((0, 0), (5000, 0))
        paper = document.layout()
        if kind == "line":
            paper.add_line((1, 2), (101, 2))
        elif kind == "viewport":
            paper.add_viewport(center=(100, 100), size=(160, 160), view_center_point=(2500, 0), view_height=8000)
        elif kind == "mesh":
            paper.add_polymesh((2, 2))
        elif kind == "ordinate":
            paper.add_ordinate_dim(feature_location=(20, 30), offset=(10, 20), dtype=0).render()
        else:
            paper.add_line((1, 2), (101, 2))
        path = args.output / f"r12-paper-{kind}.dxf"
        audit = document.audit()
        assert not audit.errors and not audit.fixes
        document.saveas(path)
        if kind in ("unknown", "nested"):
            content = path.read_text()
            start = content.rfind("\nLINE\n")
            assert start > 0
            if kind == "unknown":
                content = content[:start] + content[start:].replace("\nLINE\n", "\nKUUBIK_UNKNOWN\n", 1)
            else:
                end = content.index("\n  0\n", start)
                content = content[:end] + "\n102\n{KUUBIK\n102\n{NESTED\n 67\n0\n330\n0\n102\n}\n102\n}" + content[end:]
            path.write_text(content)
        inputs.append(path)
    model_path = args.output / "r12-model-only.dxf"
    model = ezdxf.new("R12")
    model.modelspace().add_line((0, 0), (5000, 0))
    model.saveas(model_path)
    inputs.append(model_path)
    model_crlf = args.output / "r12-model-crlf.dxf"
    model_crlf.write_bytes(model_path.read_text().replace("\n", "\r\n").encode())
    inputs.append(model_crlf)
    model_nested = args.output / "r12-model-nested.dxf"
    text = model_path.read_text()
    start = text.rfind("\nLINE\n")
    end = text.index("\n  0\n", start)
    model_nested.write_text(text[:end] + "\n102\n{KUUBIK\n 67\n1\n102\n}" + text[end:])
    inputs.append(model_nested)
    # Paper geometry may be inside BLOCKS without redundant 67/330 attributes.
    block_path = args.output / "r12-paper-block.dxf"
    start = text.lower().index("$paper_space")
    end = text.index("  0\nENDBLK", start)
    block_path.write_text(text[:end] + "  0\nLINE\n  5\nFFF\n  8\n0\n 10\n1\n 20\n2\n 11\n101\n 21\n2\n" + text[end:])
    inputs.append(block_path)
    failures = []
    for source, compatibility in ((source, compat) for source in inputs for compat in (False, True)):
        if args.case and source.stem != args.case:
            continue
        original = source.read_bytes()
        target = args.output / (source.stem + ("-compatibility" if compatibility else ""))
        options = ["-Compatibility"] if compatibility else []
        model_only = source in (model_path, model_crlf, model_nested)
        if model_only:
            options.append("-ModelOnly")
        result = subprocess.run(["pwsh", "-NoProfile", "-File", str(Path(__file__).with_name("test-paperspace-save-guard.ps1")),
                        "-Executable", str(args.executable.resolve()), "-InputDxf", str(source.resolve()),
                        "-OutputDirectory", str(target.resolve()), *options])
        if result.returncode:
            failures.append(target.name)
            continue
        report = json.loads((target / "paperspace-save-guard.json").read_text(encoding="utf-8-sig"))
        if model_only:
            assert report["passed"] and report["modelSaved"]
            output = ezdxf.readfile(target / "protected-source.dxf")
            audit = output.audit()
            assert not audit.errors and not audit.fixes
            line, = output.modelspace().query("LINE")
            assert line.dxf.end.isclose((5000, 0, 0))
            print(f"PASS {target.name}: ordinary native UI save, independent geometry and audit 0/0")
            continue
        assert report["passed"] and len(report["checks"]) == 15 and all(report["checks"].values())
        assert source.read_bytes() == original and (target / "protected-source.dxf").read_bytes() == original
        for name in ("protected-source.dxf~", "#protected-source.dxf", "protected-copy.dxf", "protected-export.dxf"):
            assert (target / name).read_bytes() == b"KEEP EXISTING FILE\n", (source.name, name)
        fresh = ezdxf.readfile(target / "new-model.dxf")
        audit = fresh.audit()
        assert not audit.errors and not audit.fixes
        line, = fresh.modelspace().query("LINE")
        assert line.dxf.start.isclose((0, 0, 0)) and line.dxf.end.isclose((5000, 0, 0))
        print(f"PASS {target.name}: 15 native checks, original/backup/autosave/copy/export bytes, new model audit 0/0")
    assert not failures, failures


if __name__ == "__main__":
    main()
