"""Synthetic paperspace records shared by the native DXF regression scripts."""

import ezdxf


def read_records(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    assert len(lines) % 2 == 0, path.name
    records = []
    for code, value in zip(lines[::2], lines[1::2]):
        code, value = int(code.strip()), value.strip()
        if code == 0:
            records.append((value, []))
        elif records:
            records[-1][1].append((code, value))
    return records


def make_fixture(path, direction_probe=False):
    document = ezdxf.new("R2018")
    document.units = 4
    document.layers.new("VIEWPORTS", dxfattribs={"plot": 0})
    document.modelspace().add_line((0, 0), (5000, 0))
    document.layouts.rename("Layout1", "TEST")
    layout = document.layouts.get("TEST")
    layout.page_setup(size=(420, 297), margins=(0, 0, 0, 0), units="mm", scale=(1, 1))
    layout.dxf_layout.dxf.page_setup_name = "A3-preset"
    for index, height in enumerate((8000, 16000)):
        viewport = layout.add_viewport(
            center=(100 + index * 190, 148.5), size=(160, 160),
            view_center_point=(2500 - index * 1250, index * 800), view_height=height,
            dxfattribs={
                "view_twist_angle": index * 30,
                # Non-default XYZ is a parser probe, not a 3D application workflow.
                "view_direction_vector": (index, index * 2, 1 + index * 2) if direction_probe else (0, 0, 1),
                "view_target_point": (index * 100, index * 200, 0),
                "flags": 0x8000 | 0x200 | (0x4000 if index == 0 else 0),
            },
        )
        viewport.dxf.id = index + 2
        assert viewport.dxf.height / viewport.dxf.view_height == 1 / (50 + index * 50)
    layout.dxf_layout.dxf.viewport_handle = viewport.dxf.handle
    for item in document.layouts:
        item.dxf_layout.set_reactors([item.dxf_layout.dxf.owner])
    audit = document.audit()
    assert not audit.errors and not audit.fixes, (audit.errors, audit.fixes)
    document.saveas(path)
    return document
