"""Raw DXF ownership checks: run before a repairing/normalizing high-level reader."""

from dxf_paperspace import read_records


def scopes(tags):
    result = {"": {}}
    subclass, depth = "", 0
    for code, value in tags:
        if code == 102:
            depth += 1 if value.startswith("{") else -1
            assert depth >= 0
        elif depth:
            continue
        elif code == 100:
            subclass = value
            result.setdefault(subclass, {})
        else:
            result[subclass].setdefault(code, []).append(value)
    assert depth == 0
    return result


def one(tags, code, default=None):
    values = tags.get(code, [])
    assert len(values) <= 1, (code, values)
    return values[0] if values else default


def check_layout_graph(path):
    records = read_records(path)
    for _, tags in records:
        for index, (code, value) in enumerate(tags):
            if code in (5, 105, 480, 481, 1005) or 320 <= code <= 369 or 390 <= code <= 399:
                tags[index] = code, f"{int(value, 16):X}"
    objects = {}
    for kind, tags in records:
        if kind in ("SECTION", "ENDSEC", "ENDTAB", "EOF"):
            continue
        scoped = scopes(tags)
        handle = one(scoped[""], 5, one(scoped[""], 105))
        if handle is not None:
            assert 0 < int(handle, 16) <= 0x7FFFFFFF and handle not in objects, (kind, handle)
            objects[handle] = (kind, scoped)
    dictionaries = {}
    for handle, (kind, scoped) in objects.items():
        if kind == "DICTIONARY":
            data = scoped["AcDbDictionary"]
            names, targets = data.get(3, []), data.get(350, [])
            assert len(names) == len(targets) and len(set(names)) == len(names)
            dictionaries[handle] = dict(zip(names, targets))
    roots = [handle for handle in dictionaries if one(objects[handle][1][""], 330, "0") == "0"]
    root, = roots
    assert "ACAD_LAYOUT" in dictionaries[root], "missing ACAD_LAYOUT dictionary"
    layout_dictionary = dictionaries[root]["ACAD_LAYOUT"]
    assert objects[layout_dictionary][0] == "DICTIONARY"
    assert one(objects[layout_dictionary][1][""], 330) == root
    layouts = {handle: scoped for handle, (kind, scoped) in objects.items() if kind == "LAYOUT"}
    assert len(dictionaries[layout_dictionary]) == len(layouts)
    assert set(dictionaries[layout_dictionary].values()) == set(layouts), "layout dictionary membership"
    block_records = {handle: scoped for handle, (kind, scoped) in objects.items() if kind == "BLOCK_RECORD"}
    for handle, scoped in layouts.items():
        data = scoped["AcDbLayout"]
        assert one(scoped[""], 330) == layout_dictionary
        assert dictionaries[layout_dictionary][one(data, 1)] == handle
        block = one(data, 330)
        assert block in block_records, "missing layout BLOCK_RECORD"
        assert one(block_records[block]["AcDbBlockTableRecord"], 340) == handle, "wrong block backlink"
        viewport = one(data, 331, "0")
        if viewport != "0":
            assert objects[viewport][0] == "VIEWPORT" and one(objects[viewport][1][""], 330) == block
    for kind, tags in records:
        if kind != "LAYOUT":
            continue
        opening = tags.index((102, "{ACAD_REACTORS"))
        closing = tags.index((102, "}"), opening)
        assert tags[opening + 1:closing] == [(330, layout_dictionary)], "wrong layout reactor"
    table, block = None, None
    for kind, tags in records:
        scoped = scopes(tags)
        if kind == "TABLE":
            table = one(scoped[""], 5)
        elif kind == "ENDTAB":
            table = None
        elif table:
            assert one(scoped[""], 330) == table, (kind, "wrong table owner")
        elif kind == "BLOCK":
            name = one(scoped["AcDbBlockBegin"], 2)
            block = one(scoped[""], 330)
            assert block in block_records and one(block_records[block]["AcDbBlockTableRecord"], 2) == name
        elif kind == "ENDBLK":
            assert block and one(scoped[""], 330) == block, "wrong ENDBLK owner"
            block = None
        elif kind in ("LINE", "VIEWPORT"):
            owner = one(scoped[""], 330)
            assert owner in block_records, (kind, "missing entity owner")
            paper = one(scoped["AcDbEntity"], 67, "0") == "1"
            assert paper == (one(block_records[owner]["AcDbBlockTableRecord"], 2) != "*Model_Space")
    for kind, scoped in objects.values():
        owner = one(scoped[""], 330, "0")
        assert owner == "0" or owner in objects, (kind, "dangling owner", owner)
        for data in scoped.values():
            for target in data.get(348, []):
                assert target == "0" or (target in objects and objects[target][0] == "VISUALSTYLE"), (kind, "invalid visual style", target)
            for target in data.get(390, []):
                assert target == "0" or target in objects, (kind, "dangling plot style", target)
    header = next(tags for kind, tags in records if kind == "SECTION" and (2, "HEADER") in tags)
    assert header[header.index((9, "$ACADVER")) + 1] == (1, "AC1032")
    seed = int(header[header.index((9, "$HANDSEED")) + 1][1], 16)
    assert seed > max(int(handle, 16) for handle in objects), "HANDSEED does not exceed all handles"
    return objects
