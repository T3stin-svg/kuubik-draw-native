# Kuubik Draw Native — project state

Status date: **2026-09-02**

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
  `aaff1448482b861c10ceb8b8cf47326c956284bc`
- Successful Windows MSVC x64 / Qt 5.15 run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33660926998>
- Tested portable ZIP SHA-256:
  `9361e7f0cb612fb17cd2f4b36630c16bb76c9d7e272545245825dd0193864936`
- GUI evidence artifact SHA-256:
  `18ebe32192445f2c71526fdd08dcbbf896c168f303f610b9065accb9d7e1ee37`
- Portable artifact wrapper SHA-256:
  `1665e1b8082ebebb590bcf74843746182cb97f34b26c4549155cb9a29ef3e94c`

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
  three independently read-back DXF states.

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

For development commit `aaff14484`, run `33660926998` additionally verified:

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
- In the automated 1200/1280 qwindows captures the Home Draw and Modify panels
  are available through their `More` overflow menus, not as always-visible
  direct buttons. The 1920 offscreen native workflow also selected Draw through
  that visible menu. The route is now mouse-tested and functional, but Reio
  must accept this responsive priority or request a separate layout revision.
- Full owner testing of Modify, layers, annotation, blocks and complex file
  workflows is still required on Reio's real workflow.
- No `.kdraw` import from the web experiment exists; use DXF exchange.

## Build authority

The official preview build is GitHub Actions workflow
`.github/workflows/kuubik-preview-win64.yml` on Windows Server 2022 with MSVC
x64 and Qt 5.15. Local machines without that toolchain should not claim a new
binary build. They can edit, inspect, run static checks and consume a verified
release artifact.
