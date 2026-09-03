# Kuubik Draw Native — project state

Status date: **2026-09-03**

## Product

Kuubik Draw Native is a portable, offline Windows x64 2D CAD application based
on LibreCAD `v2.2.1.5`. The current product branch is
`kuubik/visual-v0.2`. It uses the full LibreCAD 2D engine and adds a Kuubik Dark
workspace, compact AutoCAD-familiar ribbon and Kuubik product identity.

The application does not need Node.js, Python, internet, installation or
administrator rights at runtime.

## Repository map

- Canonical public repo: `T3stin-svg/kuubik-draw-native`
- Product/default branch: `kuubik/visual-v0.2`
- Upstream preservation branch: `master`
- LibreCAD remote: `https://github.com/LibreCAD/LibreCAD.git`
- Old web experiment: `T3stin-svg/kuubik-draw` — separate and not the native
  product

## Last published release (immutable historical checkpoint)

- Version: `0.2.0-preview.2`
- Tag: `v0.2.0-preview.2`
- Executable source commit:
  `171d95915f6f5a34b8d9fcb487dd3429de8cda74`
- ZIP SHA-256:
  `6af290c178dbd9cd21ef3d9968c6972430cf031a2b4ad4262281b2715c280492`
- Release URL:
  <https://github.com/T3stin-svg/kuubik-draw-native/releases/tag/v0.2.0-preview.2>

The product branch also contains later documentation and public handoff files.
Do not move the release tag: its exact executable source is intentionally
immutable.

## Verified development checkpoint (not a release)

- Integration branch: `codex/autocad-visual-integration-root`
- Tested source commit:
  `d17e8b23bb702a7df8c4c106783b75fcd0ba9ea2`
- Successful Windows MSVC x64 / Qt 5.15 run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33665520217>
- Tested portable ZIP SHA-256:
  `127b0558ab12a285a5abe639053b16418f5ba8f502789f4e46596bd0c0730365`
- GUI evidence artifact SHA-256:
  `c7c00f50d77907f2ccbffff3ee972e1857a993acf9c1c55cd8d8892e030d8e78`
- Portable artifact wrapper SHA-256:
  `cf18225989c4f65abb36b817b397bc1f89eaf3098a87410cbafdd364370b1379`

A later documentation-only handoff commit may follow on the branch without
changing the tested executable source identified above.

These are expiring workflow artifacts, not a GitHub release. They must not
replace `v0.2.0-preview.2` or be represented as a new public preview without
Reio's explicit approval.

## Implemented Kuubik layer

- executable and product identity `KuubikDraw.exe` / `Kuubik Draw`;
- isolated Qt settings namespace `Kuubik Projekt OÜ / Kuubik Draw`;
- Fusion/QSS `Kuubik Dark` theme;
- compact Home, Annotate, Insert, View and Output ribbon;
- ribbon buttons bound to real LibreCAD `QAction` objects;
- dark 2D model space with blue active/selection accents;
- right-side Layers and Blocks palettes;
- full-width bottom command line;
- compact coordinate/snap/status controls;
- Kuubik and Classic workspace modes;
- palette left/right and workspace reset commands;
- portable MSVC/Qt Windows packaging and prerelease automation;
- native GUI proof for ribbon LINE and two canvas clicks;
- public CI and local Windows screenshot/JSON/DXF evidence.

The development checkpoint additionally provides:

- Home, Insert, Annotate, View, Manage, and Output ribbon organization using
  existing LibreCAD `QAction` identities;
- responsive Home panels that collapse to their same-action overflow menus at
  narrow widths;
- 66 mapped Kuubik technical-line SVGs covering 66 direct ribbon/Quick Access
  action keys, with recorded repository provenance and automated validation;
- a real native current-layer selector embedded in the ribbon;
- a right-side read-only Properties dock tabbed with Layers and Blocks;
- native no/single/multiple selection summaries and `ModifyEntity` delegation;
- a tested Kuubik-to-Classic workspace restore path;
- native LINE and DIMLINEAR Tool Options widgets contained in the inline ribbon
  host at 1280 logical pixels without stale or duplicate widgets;
- a native three-vertex open PLINE plus quick-access Undo/Redo workflow, with
  three independently read-back DXF states;
- native COPY through the existing `ModifyDuplicate` QAction, a real canvas
  source click and quick-access Undo/Redo, with three independently read-back
  DXF states.

The current uncommitted owner-review wave additionally keeps the Home `Draw`
panel expanded at narrow widths and presents `Line`, `Polyline`, `Circle` and
`Arc` as four direct large commands, with `Rectangle` and `Hatch` beside them.
Collapsed secondary panels use their first Kuubik command icon instead of a
generic `More` tile. This layout has compiled and rendered locally with Qt 5 at
1280x600. The same wave makes `Enter` finish native LINE and PLINE from either
the canvas or an empty command-line submission while retaining committed
segments and PLINE's atomic native Undo/Redo behavior. It is not yet a Windows
development checkpoint. Ribbon command text and Kuubik icons are also pinned
to their stable command identity while LibreCAD changes the underlying native
action's active construction mode. LINE and PLINE now share a cursor-adjacent
length/angle dynamic input with numeric entry and `Tab` switching; canvas
`Esc` globally clears the active command stack. While either drawing action is
active, the global command-line key filter routes numeric keys, Backspace and
Tab to this dynamic input instead of stealing canvas focus.

Kuubik workspace now uses one compact AutoCAD-familiar bottom status row. The
legacy LibreCAD snap toolbar and diagnostic selection/layer/grid labels are
hidden in Kuubik mode, while Classic mode restores them. The visible Kuubik row
contains MODEL, GRID, ORTHO and the OSNAP popup.

The status bar now groups the seven supported native snap modes into one
`OSNAP` popup and uses distinct temporary symbols for endpoint, midpoint,
center, intersection and nearest snaps. Higher-order AutoCAD snaps and
acquisition tracking remain explicitly unimplemented rather than appearing as
non-functional menu entries.

## Verified behaviors

- portable startup from a path containing spaces;
- LINE action activation from the Kuubik ribbon;
- LINE and PLINE activation through the responsive panel's physically visible
  `More` menu; the harness rejects interaction through a hidden widget;
- LINE preview after the first click;
- exactly one committed native LINE after the second click;
- DXF open/save and independent `ezdxf` read-back;
- vector PDF export and independent `pypdf` read-back;
- SVG export and XML read-back;
- package-only Qt plugin loading;
- runtime DLL/plugin/resource allowlist;
- 1920×1080 local Windows visual read-back.

Evidence is under `evidence/releases/v0.2.0-preview.2` and summarized in
`docs/TEST_REPORT.md`.

For development commit `d17e8b23b`, run `33665520217` additionally verified:

- 71 distinct ribbon `QAction` bindings/action keys retain their exact native
  identity;
- selecting `KUUBIK-SMOKE-LAYER` changes the native current layer and a newly
  committed LINE remains on that layer after save and native reopen;
- Properties receives native document and selection callbacks and its full
  edit button activates the existing `ModifyEntity` action;
- Draw LINE and PLINE use the visible responsive overflow menu in the 1920×1080
  offscreen workflow and retain the exact native QAction identity;
- an open three-vertex PLINE is present before Undo, absent after quick-access
  Undo, and restored with identical points after quick-access Redo; the earlier
  LINE retains identical start/end coordinates across all three parsed DXFs;
- COPY uses the visible Modify overflow menu and exact native
  `ModifyDuplicate` identity, catches an unselected source LINE with a canvas
  click, adds one distinct in-place LINE, removes only that copy with
  quick-access Undo and restores it with quick-access Redo; the earlier LINE
  and PLINE remain active;
- native LINE and DIMLINEAR Tool Options remain visible and contained at
  1280×600, with exactly one expected widget set per active action;
- DXF open/edit/save/reopen, vector A4 PDF, and SVG pass independent read-back;
- qwindows + Qt scale-factor render smoke at 100%, 125%, and 150% produces
  1280×600, 1600×750, and 1800×900 PNGs without clipping or overlap;
- `build-manifest.json` fixes the source commit, while the generated adjacent
  `.zip.sha256` file records the tested portable ZIP checksum above.

## Honest limits

- This is a prerelease, not production.
- No native 133-row AutoCAD parity percentage has been certified for this fork.
- DWG roundtrip, DWT and XREF parity are not certified.
- The permanent right Properties palette is read-only; full entity editing is
  still delegated to LibreCAD's existing native dialog/action.
- The ribbon covers common tools; inherited commands remain in menus/Classic.
- Of 71 distinct direct ribbon/Quick Access action keys, 66 use the current
  Kuubik technical-line SVG set. Five Annotate/Lines keys plus menus, Classic,
  status, pen, and Tool Options surfaces retain inherited LibreCAD icons; Reio
  has not yet accepted or rejected this mixed visual system.
- The automated 125% and 150% screenshots use qwindows with
  `QT_SCALE_FACTOR`; real Windows Settings display-scale checks at 100%, 125%,
  and 150% remain pending on controlled Windows hardware.
- The last verified Windows checkpoint still shows Home Draw and Modify through
  responsive overflow at 1200/1280 logical widths. Reio has since rejected the
  generic `More`-first presentation and requested an AutoCAD-familiar direct
  Draw layout. The replacement is locally rendered but still needs the Windows
  workflow and real display-scale checks.
- Full owner testing of Modify, layers, annotation, blocks and complex file
  workflows is still required on Reio's real workflow.
- The local Kuubik workspace now has the full compact drafting row and an
  expanded native OSNAP menu. Linux build/smoke is green; dedicated numeric
  fixtures for every newly added snap mode and Windows qwindows verification
  remain pending.
- The drafting row is icon-first rather than text-first. GRID, SNAP, ORTHO,
  POLAR, OSNAP, OTRACK, dynamic input, lineweight, selection cycling and clean
  screen use original Kuubik technical-line SVG symbols; extended OSNAP menu
  entries also have matching symbols.
- No `.kdraw` import from the web experiment exists; use DXF exchange.

## Build authority

The official preview build is GitHub Actions workflow
`.github/workflows/kuubik-preview-win64.yml` on Windows Server 2022 with MSVC
x64 and Qt 5.15. Local machines without that toolchain should not claim a new
binary build. They can edit, inspect, run static checks and consume a verified
release artifact.
