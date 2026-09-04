# Kuubik Draw Native — verified test checkpoint

Checkpoint: `0.2.0-preview.2`

The published-release evidence below applies only to source commit
`171d95915f6f5a34b8d9fcb487dd3429de8cda74`. It does not by itself prove later
UI work and must not be reused as a release claim for the integration branch.

## Source and artifact

- executable source commit:
  `171d95915f6f5a34b8d9fcb487dd3429de8cda74`
- LibreCAD base:
  `7ebab007d9eb4c68609388b835a2487648f0877b`
- successful Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33510020896>
- portable ZIP: `KuubikDraw-0.2.0-preview.2-win64.zip`
- ZIP size: `43,438,760` bytes
- ZIP SHA-256:
  `6af290c178dbd9cd21ef3d9968c6972430cf031a2b4ad4262281b2715c280492`

## Passed CI gates

- pinned LibreCAD ancestry;
- MSVC x64 / Qt 5.15.2 release compilation;
- portable runtime packaging;
- required Qt/DLL/resource presence;
- forbidden build/cache payload absence;
- isolated Qt plugin lookup from the copied package;
- UI contract with at least 40 bound real actions;
- right Layers/Blocks and bottom command dock layout;
- ribbon LINE real mouse event and action activation;
- first canvas click leaves zero committed objects;
- pointer move creates the visible preview;
- second canvas click creates exactly one native LINE;
- DXF save and independent `ezdxf` read-back;
- vector PDF export and independent `pypdf` read-back;
- SVG export and XML read-back;
- copied path containing spaces;
- startup smoke and archive SHA-256.

## Independent LINE output

- entity count: `1`
- type: `LINE`
- start: `(122.0, 115.25, 0.0)`
- end: `(256.5, 50.0, 0.0)`
- length: `149.49184760380749`

Public files and their individual hashes are under
`evidence/releases/v0.2.0-preview.2`.

## Local Windows replay

The downloaded ZIP hash matched GitHub. It was extracted to a fresh folder and
the complete portable smoke passed locally, including offscreen UI contract,
LINE mouse workflow, DXF/PDF/SVG generation, independent parsers and normal
Windows GUI startup. A second Windows-platform run captured 1920×1080, 96-DPI
screenshots with fully rendered UI text.

## Corrected false positive

Run `33507858173` passed because the hosted runner found `qoffscreen.dll` in its
installed Qt tree. The downloaded package failed the same isolated local test.
Packaging was corrected to include the plugin and the test now sets package-only
plugin paths. Only corrected run `33510020896` is the release gate.

## Security and publication

- pinned Gitleaks 8.30.1: zero findings for the implementation wave;
- public evidence path/user/client keyword scan: zero findings;
- PNG metadata contains only DPI;
- evidence is synthetic and contains no client drawing or private AutoCAD image.

## Not run / not certified

- full manual owner matrix across every inherited LibreCAD command;
- DWG roundtrip, DWT and XREF parity;
- AutoCAD 2024 live paired workflow for this native fork;
- long-duration autosave/crash recovery;
- installer, code signing and production rollout;
- full 133-row AutoCAD audit against the native fork.

## Development integration checkpoint — 2026-09-02

This section is branch evidence, not a release and not a replacement for the
immutable public `v0.2.0-preview.2` artifact.

- branch: `codex/autocad-visual-integration-root`
- tested source: `d17e8b23bb702a7df8c4c106783b75fcd0ba9ea2`
- Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33665520217>
- tested portable ZIP SHA-256:
  `127b0558ab12a285a5abe639053b16418f5ba8f502789f4e46596bd0c0730365`
- GUI evidence artifact SHA-256:
  `c7c00f50d77907f2ccbffff3ee972e1857a993acf9c1c55cd8d8892e030d8e78`
- portable artifact wrapper SHA-256:
  `cf18225989c4f65abb36b817b397bc1f89eaf3098a87410cbafdd364370b1379`
- GUI evidence artifact ID: `9861131852`
- portable artifact ID: `9861134306`

The MSVC x64 / Qt 5.15.2 build, package isolation, payload allowlist, Gitleaks,
UI contract v2, native LINE canvas flow, native layer selector, Properties
callbacks and `ModifyEntity` delegation all passed. The test opened a synthetic
DXF, changed the active layer, committed a new LINE, saved it, closed/reopened
it through the native adapter, and independently read back the result. It then
created a native open PLINE with three canvas clicks, committed that command's
single undo cycle, clicked the visible quick-access Undo and Redo buttons, and
saved a DXF after each state.

The same smoke then cleared native selection, activated the visible responsive
Modify `More` route for the exact `ModifyDuplicate` QAction, and clicked the
earlier LINE at graph point `(184.375, 82.25)`. The automation fixture pins the
native action to in-place mode and reports that fact explicitly; this is not a
claim that the Tool Options checkbox itself was mouse-tested. One distinct
native LINE was created on `KUUBIK-SMOKE-LAYER`, then removed and restored by
the visible quick-access Undo and Redo buttons.

All three Draw invocations in the smoke—initial LINE, PLINE and the later
Properties LINE—used the visible `collapsedPanelOverflow` route. The test
physically clicked the visible `More` tool button, required the popup menu to be
visible, matched the exact native QAction at `QMenu::actionGeometry()`, clicked
that row with a mouse event and required the menu to close. No event was sent to
the hidden source button.

Independent `ezdxf` read-back verified:

- `pline-before-undo.dxf`: one open smoke PLINE with points
  `(66.25, 131.0)`, `(184.25, 140.75)`, `(282.75, 101.75)`;
- `pline-after-undo.dxf`: no smoke PLINE;
- `pline-after-redo.dxf`: the same open three-point smoke PLINE;
- the earlier smoke LINE remained exactly
  `(118.75, 114.75, 0.0)` → `(250.0, 49.75, 0.0)` in all three files;
- the original fixture LINE, circle and closed polyline also remained intact.
- `copy-before-undo.dxf` and `copy-after-redo.dxf` contain two identical smoke
  LINE entities with geometry `(118.75, 114.75, 0.0)` →
  `(250.0, 49.75, 0.0)`; `copy-after-undo.dxf` contains only the source smoke
  LINE;
- the smoke LINE counts are therefore `2 → 1 → 2` and total LINE counts,
  including the original fixture, are `3 → 2 → 3`;
- the earlier open smoke PLINE remains unchanged in all three COPY DXFs.

The independent verifier also found a vector A4 PDF and valid SVG vectors.

### Focused native Tool Options checkpoint

Source `f1c6733eb4c58c455132f00d845003acf93b1682` first passed the
1280×600 qwindows Tool Options proof in run
<https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33654660495>.
Its tested portable ZIP SHA-256 was
`7f623a64af40d5b46c9b2639e2c6f6884f04cfb598fec646a55f5c7da9cbb9af`;
the GUI evidence and portable wrapper hashes were respectively
`89bd3edb5301fea7b3ac649bbdfbff8a7f73715af1315133dd186f5a6ebd28d8`
and `f21de3e7c9f43e2a44cd2fe79433799bd877d20cfca3e226c63bd939e9a22acc`.

The report measured a 299-pixel LINE options host with exactly one native
`QG_LineOptions`. DIMLINEAR used a 621-pixel host containing one
420-pixel `QG_DimOptions` and one 200-pixel `QG_DimLinearOptions`, with no
clipping, stale widget or duplicate widget. Runs `33660926998` and
`33665520217` repeated the same focused test successfully; run `33665520217`'s
two Tool Options PNGs are byte-identical to the visually reviewed focused run.

### Corrected responsive-ribbon false interaction

Run `33657916794` on source
`35b1be45ff8921d3f1c4cabbaae1440974574c44` correctly failed because the first
PLINE smoke sent a synthetic event directly to a hidden Draw button. Qt emitted
the QAction signal, but that was not a user-clickable path. Source `aaff14484`
replaced that invalid interaction with the visible overflow button and popup
menu-row mouse route described above. Run `33660926998` proved the correction;
successful run `33665520217` is the current development gate and extends it
with the visible native COPY route.

The render smoke first verified the hosted Windows desktop changed from
1024×768 to 1920×1080, then recorded:

- 100%: 1280×600 logical and 1280×600 PNG;
- 125%: 1280×600 logical and 1600×750 PNG;
- 150%: 1200×600 logical and 1800×900 PNG.

All three qwindows screenshots were visually inspected with no clipping or
overlap. These 125%/150% cases use `QT_SCALE_FACTOR`; they are not evidence that
Windows Settings OS display scaling was changed. Real Windows 100%/125%/150%
scale validation and Reio's layout acceptance remain open.

The automated qwindows screenshots show Draw and Modify through responsive
`More` panels at the tested 1200/1280 logical widths. The 1920×1080 offscreen
workflow also used the Draw overflow route. That route is functional and now
mouse-tested, but keeping it as the preferred layout still requires Reio's
accept/deny decision.

The offscreen LINE active/committed PNGs are functional canvas evidence only;
the qwindows screenshots, where text is rendered, remain the visual-layout
gate. The goal also still requires real Windows Settings 100%/125%/150% checks
on controlled hardware and remaining native Modify/Annotation/Blocks workflow
evidence.

## Local direct-Draw layout check — 2026-09-03

The owner rejected the generic `More`-first Home ribbon shown by the preceding
Windows checkpoint. A local Arch Linux Qt 5 release build now keeps Home Draw
expanded at 1280x600 and renders direct `Line`, `Polyline`, `Circle` and `Arc`
buttons, plus `Rectangle` and `Hatch`. Secondary collapsed panels render a
representative Kuubik icon instead of `More`. The build completed successfully
and the offscreen UI contract/screenshot writer completed successfully.

This is local visual evidence only. The Windows MSVC/Qt 5.15 portable workflow,
qwindows DPI captures and native QAction-to-canvas smoke have not yet been run
for this uncommitted layout wave.

## Local LINE/PLINE Enter workflow check — 2026-09-03

The Arch Linux Qt 5 release build completed successfully after adding native
`Enter` completion to LINE and PLINE. The full offscreen GUI smoke exited 0 and
reported `PASS`. It physically invoked both ribbon actions, drew through canvas
clicks and sent `Qt::Key_Return` to the canvas.

The report recorded `accepted: true` and `finishedAction: true` for both LINE
and PLINE. LINE retained its committed segment. PLINE retained two segments as
one open entity, and its existing Undo/Redo roundtrip passed. The packaged
Windows qwindows run remains required before this local result is described as
Windows-verified.

The same GUI smoke now also captures each LINE/PLINE ribbon button before
activation and verifies that its visible text and Kuubik icon remain unchanged
after the native action starts. Both presentation-stability checks passed.

The extended smoke additionally found a visible cursor-adjacent dynamic input
containing both `L` and `A`, and verified that a canvas Escape event was
accepted and left no active action. The full smoke remained `PASS`, including
LINE/PLINE creation and PLINE atomic Undo/Redo.

After an owner screenshot showed numeric input landing in the bottom command
edit, the command-widget filter was corrected so active LINE/PLINE dynamic
input owns numeric keys, Backspace and Tab. The rebuilt full GUI smoke remained
`PASS`.

The bottom-status refinement hides the legacy snap toolbar and diagnostic
status widgets in Kuubik mode and restores them in Classic mode. The full local
GUI smoke remained `PASS` after the MODEL/GRID/ORTHO/OSNAP single-row change.

The OSNAP status/menu and type-specific overlay marker wave compiled in the
same Qt 5 release build. The full offscreen GUI smoke remained `PASS`; a native
Windows qwindows snap-interaction matrix is still required.

The expanded drafting-status/OSNAP wave on 2026-09-03 compiled with Qt 5 and
the full offscreen native GUI smoke reported `PASS`. Its report retained
`dynamicInputVisible: true`, `escapeCancelsAll: true`, successful LINE/PLINE
Enter completion, Undo/Redo, document lifecycle and DXF saves. The new snap
candidate matrix still requires dedicated geometry fixtures and the Windows
MSVC/qwindows workflow before release certification.

The status-symbol refinement on 2026-09-03 added original Kuubik SVG symbols
for the ten drafting controls and the extended OSNAP menu. The Qt 5 build and
full offscreen GUI smoke both passed after the resource and layout change;
`git diff --check` also passed. A live Wayland capture confirmed the compact
single-row rendering at a narrow half-screen window width.

## Local AutoCAD-familiar status-bar check — 2026-09-04

The status bar was rebuilt around the Autodesk-documented interaction pattern
using original Kuubik assets. The Qt 5 release build completed successfully.
The offscreen UI contract completed with exit code 0 and recorded:

- ten functional status-button bindings;
- twelve persistent customization entries with matching controls;
- a successful GRID hide/restore round trip through the customization action;
- fourteen OSNAP modes and a synchronized split-button state;
- four live coordinate display formats;
- no bottom-right resize grip;
- a rendered 222×374 customization-menu image and 1024×1024 application image.

The full local native GUI smoke also exited 0 with `PASS`. It retained LINE and
PLINE Enter behavior, visible dynamic input, global Escape cancellation, native
document lifecycle, and COPY and MOVE atomic Undo/Redo. `git diff --check`
passed. This is local Linux evidence only; the packaged Windows MSVC/qwindows
workflow and real Windows display-scale checks remain pending.

After live review found inherited LibreCAD symbols replacing Kuubik status
icons on click, the direct status bindings were separated from their visual
buttons. The rebuilt UI contract click-tested GRID, SNAP and ORTHO, restored
each native checked state, and recorded `customIconsStableAfterClick: true`.
All four direct-action controls recorded `customIconOwned: true`. The complete
native GUI smoke remained `PASS` for drawing, dynamic input, document lifecycle,
COPY and MOVE Undo/Redo.

The first-five-reference-pages pass on 2026-09-04 rebuilt the row with the
approved blue-grey palette and placed live coordinates, MODEL and GRID in one
ordered cluster. The coordinate readout is 184 logical pixels wide and displays
three Cartesian fields. A Kuubik/Classic/Kuubik workspace round trip restores
the original Classic coordinate slot and then reconstructs the Kuubik cluster.

The rebuilt UI contract exited 0 and recorded all first-phase checks as true:
coordinates visible by default, X/Y/Z output, coordinate/MODEL/GRID ordering,
single-row alignment, MODEL text, native Grid settings menu, synchronized Grid
state and matching Grid tooltip. The status bar measured 27 logical pixels.
The full offscreen native GUI smoke also exited 0 with `status: PASS`, including
LINE, PLINE, COPY, MOVE, Undo/Redo, Properties and document lifecycle. Autodesk
screenshots were not added to application resources.

## Reference pages 6–11 implementation check — 2026-09-04

The Qt 5 release build completed after adding the six precision-drafting
controls. The offscreen UI contract exited 0 and reported every page 6–11 gate
true: the controls were visible and ordered; SNAP exposed Grid/Polar choices,
right-click settings and F9; Dynamic Input exposed distance/angle settings and
F12; native ORTHO retained F8; POLAR exposed eight presets, F10 and a verified
15-degree snapping-engine quantization; and Isometric Drafting round-tripped
Left, Top and Right planes with F5/Ctrl+E registered.

The Infer control successfully enabled and restored the native endpoint,
perpendicular, tangent and parallel snap bundle. The contract also confirmed
that its presentation states the non-persistent limitation. The full native
GUI smoke exited 0 with `status: PASS`, retained visible dynamic input, global
Escape cancellation, exact LINE creation, PLINE/COPY/MOVE Undo/Redo and native
DXF save. Icon validation passed with 66 action mappings and 88 referenced
Kuubik SVGs, and `git diff --check` passed. This remains local Linux evidence;
Windows MSVC/qwindows packaging and real display-scale checks are pending.
