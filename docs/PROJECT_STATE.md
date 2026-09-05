# Kuubik Draw Native — project state

Status date: **2026-09-05**. Current work: **SARibbon integration in progress**.

## Product and authority

Native portable Windows x64 2D CAD based on LibreCAD v2.2.1.5, GPLv2.
The application remains offline and does not require Node.js, Python or .NET.
The single native document/entity/layer/selection/undo engine is retained.

- Repository: <https://github.com/T3stin-svg/kuubik-draw-native>
- Product branch: `kuubik/visual-v0.2`.
- Active integration: `codex/autocad-visual-integration-root`.
- Upstream preservation: `master`; LibreCAD base `7ebab007d9eb4c68609388b835a2487648f0877b`.
- The old web product remains separate. Its requirements/measurements may inform
  this fork; its implementation and old PASS scores are not merged.
- Reio approved work-branch pushes and Windows CI for the current UI milestone.
  No remote merge, release, default-branch change, paid SDK or paid service is approved.

## Last verified native Windows checkpoint — before SARibbon

- Source: `9968198bbe72165e28c48f8e37109fa3eb103212`.
- [Windows run 33919335101](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33919335101).
- Portable ZIP SHA-256:
  `60bf9445fdc6206e8f1b21a392d72dece93714a56feabc435bf7cb66cca27550`.

This exact-source development run supersedes the older d17e8b2 checkpoint.
It proves MSVC/Qt packaging, native LINE/PLINE completion, COPY and MOVE with
Undo/Redo, current-layer and Properties integration, status/precision contracts,
direct Home Draw layout, Qt-scale DPI captures and independent DXF/PDF/SVG read-back.
COPY remains native in-place duplication; MOVE still includes its native dialog.
These results are not a certificate of AutoCAD command equivalence.

The historical public release is still `v0.2.0-preview.2`, executable source
`171d95915f6f5a34b8d9fcb487dd3429de8cda74`, ZIP SHA-256
`6af290c178dbd9cd21ef3d9968c6972430cf031a2b4ad4262281b2715c280492`.
Do not replace its tag or conflate newer CI artifacts with a release.

## Current implementation wave — not yet verified

SARibbon v2.9.0 MIT source is pinned, compiled through qmake, and hosted inside
the existing KuubikRibbon QWidget/QMainWindow. No QWindowKit or frameless helper
is enabled. The existing native QActions remain bound directly to the buttons;
presentation-only QWidgetActions control panel collapse without hiding shared
commands in other tabs or Classic menus.

Home now targets ten measured panels: Draw, Modify, Annotation, Layers, Block,
Properties, Groups, Utilities, Clipboard, View. Layers contains the native
current-layer selector; Properties hosts the native pen toolbar vertically.
Groups is explicitly unavailable, not an alias for blocks.
The existing Application/Quick Access, six tabs, Tool Options, right palettes,
command line, status controls and Classic fallback remain in scope.

First integration source: `b694b5d8d9385b8b51d487d8c16138a0e0278024`.
[Initial CI](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33960243802)
is pending. Do not use the preceding checkpoint as proof of this new ribbon.

## Current limitations and next work

- Exact AutoCAD 2024.1.2 visual and interaction equivalence remains unproved.
- Real paperspace, Model/Layout tabs, viewport cameras/locks and layout DXF
  persistence are not implemented; see [PAPERSPACE_PLAN](PAPERSPACE_PLAN.md).
- The Properties palette is read-only and delegates editing to ModifyEntity.
- DWG/DWT/XREF and lossless handling of arbitrary unknown objects are not certified.
- Automated 125/150% captures use QT_SCALE_FACTOR, not Windows Settings changes.
- Broader Modify/annotation/blocks, large drawings, recovery and real owner
  workflows still need dedicated evidence.
- Existing original Kuubik icons retain inherited LibreCAD fallbacks. No
  Autodesk assets are added; owner acceptance of the icon system is still pending.

## Continuation

[DEVELOPMENT_PLAN](DEVELOPMENT_PLAN.md) owns this five-hour milestone and progress.
[RESEARCH_NOTES](RESEARCH_NOTES.md) records the Open CAD Studio/SARibbon/file-library
findings. [TEST_REPORT](TEST_REPORT.md) keeps exact-source test evidence and history.
The immediate task list is [NEXT_TASKS](../NEXT_TASKS.md).

The Windows CI is the build authority here: local Qt/MSVC development tools are
not installed. Only a verified new artifact can be offered as a working preview.
