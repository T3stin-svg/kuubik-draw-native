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
- tested source: `aaff1448482b861c10ceb8b8cf47326c956284bc`
- Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33660926998>
- tested portable ZIP SHA-256:
  `9361e7f0cb612fb17cd2f4b36630c16bb76c9d7e272545245825dd0193864936`
- GUI evidence artifact SHA-256:
  `18ebe32192445f2c71526fdd08dcbbf896c168f303f610b9065accb9d7e1ee37`
- portable artifact wrapper SHA-256:
  `1665e1b8082ebebb590bcf74843746182cb97f34b26c4549155cb9a29ef3e94c`
- GUI evidence artifact ID: `9859219845`
- portable artifact ID: `9859221595`

The MSVC x64 / Qt 5.15.2 build, package isolation, payload allowlist, Gitleaks,
UI contract v2, native LINE canvas flow, native layer selector, Properties
callbacks and `ModifyEntity` delegation all passed. The test opened a synthetic
DXF, changed the active layer, committed a new LINE, saved it, closed/reopened
it through the native adapter, and independently read back the result. It then
created a native open PLINE with three canvas clicks, committed that command's
single undo cycle, clicked the visible quick-access Undo and Redo buttons, and
saved a DXF after each state.

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
clipping, stale widget or duplicate widget. Run `33660926998` repeated the same
focused test successfully; its two Tool Options PNGs are byte-identical to the
visually reviewed run above.

### Corrected responsive-ribbon false interaction

Run `33657916794` on source
`35b1be45ff8921d3f1c4cabbaae1440974574c44` correctly failed because the first
PLINE smoke sent a synthetic event directly to a hidden Draw button. Qt emitted
the QAction signal, but that was not a user-clickable path. Source `aaff14484`
replaced that invalid interaction with the visible overflow button and popup
menu-row mouse route described above. Only successful run `33660926998` is the
current development gate.

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
