# P0 correction record — 2026-09-05

Baseline: `8a5f7ae0d707de0a78bc9c62352221c0e8ccb479`, clean worktree on
`codex/autocad-visual-integration-root`; `origin/kuubik/visual-v0.2` is an ancestor.
Last verified MSVC executable remains `d35ec354` / run `33966232573`.
No push, remote CI, release, merge or paperspace implementation in this session.

Local implementation commits: `d4280f9b` (P0-A), `4e40a43c` (P0-B).
The maintained graphical plan and next gate are in [ROADMAP](ROADMAP.md).

## P0-A — Properties summary

**Implemented and locally verified; MSVC CI and owner acceptance pending.**

The base `RS_Document::setModified` was nonvirtual, so `endUndoCycle` bypassed
`RS_Graphic`/`RS_Block` native modification behavior. `isModified` through a
document pointer also ignored the graphic's layer/block dirty flags. Both now
dispatch polymorphically. The existing current-layer listener forwards
`layerListModified` to its existing state signal; the Properties receiver is
queued until layer/block flags are settled. It resolves the active MDI on delivery,
retains no additional document pointer, and keeps selection/layer/MDI refresh paths.
Entity count excludes `isUndone()` objects but includes active hidden/frozen entities.
No timer, polling, second model or new product dependency was added.

Qt's queued delivery contract is documented in the pinned
[Qt 5.15.2 source documentation](https://github.com/qt/qtbase/blob/v5.15.2/src/corelib/global/qnamespace.qdoc).

| Native operation | Active entities | Modified | Before fix | After fix |
|---|---:|---|---|---|
| Create/current layer, before Draw | 3 | Yes | FAIL | PASS |
| LINE while already modified | 4 | Yes | FAIL | PASS |
| Save | 4 | No | FAIL | PASS |
| PLINE | 5 | Yes | FAIL | PASS |
| Save PLINE | 5 | No | FAIL | PASS |
| Undo PLINE | 4 | Yes | FAIL | PASS |
| Save after Undo | 4 | No | FAIL | PASS |
| Redo PLINE | 5 | Yes | PASS | PASS |
| Save after Redo | 5 | No | FAIL | PASS |

The test observes the real palette after native action events; it never forces
a refresh. Before-fix native process exit: 4. After-fix native process exit: 0,
full GUI smoke PASS, nine summary states PASS. Existing native selection, layer,
MDI close/reopen, LINE/PLINE/COPY/MOVE and Undo/Redo remain green. The independent
Python summary checks pass and reject a fabricated Undo count with `passed=true`.
Automated INI backend/sentinel checks pass. Offscreen is functional proof only.

Reproduce with the normal `scripts/test-kuubik-portable.ps1` and
`scripts/verify-preview-outputs.py` gates on the next exact-source MSVC package.
For a configured local Qt 5.15.2 / MinGW 8.1 x64 checkout, build with
`qmake librecad.pro -r CONFIG+=release CONFIG-=debug_and_release`, then
`mingw32-make -j4 sub-librecad-make_first-ordered`. Use the existing automation
environment `KUUBIK_GUI_SMOKE_DIR` (fresh absolute output directory),
`KUUBIK_GUI_SMOKE_INPUT_DXF` (synthetic fixture) and `QT_QPA_PLATFORM=offscreen`.
Automation chooses its own isolated INI profile before any native settings access.
Developer Qt/MinGW and Boost headers live in ignored `.artifacts`; MSVC is still
the portable build authority. The initial unordered make invocation failed on
missing libmuparser; the ordered build fixed the setup and completed.

## P0-B — PLOTSETTINGS

**Implemented and locally verified; MSVC CI and owner acceptance pending.**

`writePlotSettings` previously wrote a standalone object without an owner, and
`writeObjects` supplied no ACAD_PLOTSETTINGS dictionary. The writer now reserves
one dictionary handle, links it from the root dictionary, and writes each supplied
PLOTSETTINGS with its dictionary owner and reactor. It writes the dictionary's
entries after the existing callback, once all handles are known. The handle list
is cleared per write, including when the same writer instance is reused.
Generated unique `PlotSettings<handle>` keys match each record's page setup name.
This repairs the existing unnamed standalone settings path; it does not implement
preservation of arbitrary imported named page setups or layout-specific settings.

Primary format references: Autodesk's
[PLOTSETTINGS group codes](https://help.autodesk.com/cloudhelp/2024/ENU/AutoCAD-DXF/files/GUID-1113675E-AB07-4567-801A-310CDE0D56E9.htm)
and [DICTIONARY group codes](https://help.autodesk.com/cloudhelp/2018/ENU/AutoCAD-DXF/files/GUID-40B92C63-26F0-485B-A9C2-B349099B26D0.htm).

The new `tests/fixtures/modelspace-audit-clean.dxf` retains the original synthetic
LINE, CIRCLE, closed LWPOLYLINE and layers; it adds the missing unique LTYPE/LAYER
table handles. The old `preview-smoke.dxf` is unchanged and remains a negative
oracle for two repair 110 findings. The strict helper refuses any audit error or
repair before checking PLOTSETTINGS counts, ownership, reactor and dictionary keys.

| Check | Before fix | After fix |
|---|---|---|
| Clean AC1015 input | 0 errors / 0 repairs | 0 / 0 |
| Native ASCII output, one PLOTSETTINGS | repair 202; Python exit 1 | 0 / 0; retained |
| Empty / two / reused writer (AC1027) | generated for regression | all 0 / 0 |
| Ten GUI save/Undo/Redo outputs (AC1021) | old output could lose plot settings during audit | all 0 / 0 |
| Geometry, units, margins, native reopen in four adapter cases | native geometry alone passed | native + independent checks PASS |

`tests/dxf-plotsettings.cpp` is a test-only DRW_Interface adapter linked to the
real repository library. The Python runner audits all four outputs and checks
exact original geometry/layers, mm units and four margins. No Qt/settings or
additional product runtime is involved. Assertions must be enabled.

After building the repository, run from a configured MinGW shell (fresh output):

```text
g++ -std=c++17 -UNDEBUG -I libraries/libdxfrw/src tests/dxf-plotsettings.cpp generated/lib/libdxfrw.a -o dxf-plotsettings.exe
python scripts/test-dxf-plotsettings.py ./dxf-plotsettings.exe <fresh-output-directory>
```

The existing MSVC workflow compiles the same adapter with `/UNDEBUG`, links
`generated/lib/dxfrw.lib`, and runs the same Python test. Test-only ezdxf is pinned
to 1.4.4. The portable harness now uses the clean fixture; its existing independent
verifier requires 0 errors and 0 repairs on the input and all ten GUI DXFs.

The optional exploratory binary case failed before the fix: native reopen exit 3
and ezdxf invalid header tag 2304. Reproduce by supplying a fourth adapter argument
(`binary`) and an existing fresh output directory. This inherited header defect
is tracked separately; the passing P0 contract is ASCII, not arbitrary DXF fidelity.

## Combined local verification and handoff

- Full GUI smoke and nine Properties states: PASS, native exit 0.
- Clean input + four adapter outputs + ten GUI outputs: audit 0 errors / 0 repairs.
- Complete `scripts/verify-preview-outputs.py`: PASS, including geometry/Undo,
  one vector A4 PDF without images, vector SVG, Tool Options and Qt-scale captures.
- `scripts/verify-ribbon-layout.py`: PASS for reference and 100/125/150% captures;
  all ten reference panel boundary deltas are 0 px. Reference qwindows image inspected.
- Eight native process/profile checks: PASS; user registry unchanged. Registry
  values were compared only in memory and were not exported into evidence.
- Python syntax and rejection of old repair 110 / repair 202 outputs: PASS.
- Review: writer callers and per-write lifetime checked; no second document model,
  polling, runtime dependency, changed license, release or paperspace implementation.

Local evidence is under ignored `.artifacts/p0-corrections/`: `properties-red`,
`properties-green`, `dxf-audit-red`, `dxf-audit-final`, `combined-green` and logs.
The local runner reuses functions from the existing portable harness, but does
not claim its MinGW executable satisfies the MSVC package/manifest gates.
The optional YAML parse check could not run without PyYAML; the workflow was
reviewed statically. No extra dependency was installed for that optional check.

**Next gate:** Reio authorizes a work-branch push and the existing Windows MSVC
workflow on the exact correction source. Then verify its portable artifact and
record owner acceptance. The previous MSVC source/run/ZIP remains authoritative
until that gate passes. No remote action is authorized by this document.
