# P0 correction record — 2026-09-05

Baseline: `8a5f7ae0d707de0a78bc9c62352221c0e8ccb479`, clean worktree on
`codex/autocad-visual-integration-root`; `origin/kuubik/visual-v0.2` is an ancestor.
Last verified MSVC executable remains `d35ec354` / run `33966232573`.
No push, remote CI, release, merge or paperspace implementation in this session.

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

**Open.** Separate clean fixture and independent reproduction are being prepared.
No zero-repair output claim until the writer fix passes the bounded corpus.
