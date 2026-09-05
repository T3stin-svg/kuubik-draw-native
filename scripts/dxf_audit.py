"""Strict independent audit for the bounded synthetic ASCII modelspace corpus."""

from pathlib import Path

import ezdxf


def require_audit_clean(path, expected_plot_count):
    path = Path(path)
    document = ezdxf.readfile(path)
    plots_before = list(document.objects.query("PLOTSETTINGS"))
    audit = document.audit()
    findings = [(entry.code, entry.message) for entry in audit.errors + audit.fixes]
    if findings:
        raise AssertionError((path.name, "audit errors/repairs", findings))
    plots_after = list(document.objects.query("PLOTSETTINGS"))
    assert len(plots_before) == len(plots_after) == expected_plot_count, path.name
    dictionary = document.rootdict.get("ACAD_PLOTSETTINGS")
    if expected_plot_count or dictionary is not None:
        assert dictionary is not None, (path.name, "missing plot settings dictionary")
        assert dictionary.dxf.owner == document.rootdict.dxf.handle, path.name
        assert len(dictionary) == expected_plot_count, path.name
        entries = {entity.dxf.handle for _, entity in dictionary.items()}
        assert entries == {entity.dxf.handle for entity in plots_after}, path.name
        for name, plot in dictionary.items():
            assert name == plot.dxf.page_setup_name, (path.name, name)
        for plot in plots_after:
            assert plot.dxf.owner == dictionary.dxf.handle, (path.name, plot)
            assert dictionary.dxf.handle in plot.get_reactors(), (path.name, plot)
    return document
