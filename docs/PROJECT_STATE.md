# Kuubik Draw Native — project state

Status date: **2026-09-05**. **SARibbon UI development checkpoint ready for owner review.**

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

## Latest tested Windows development checkpoint

- Source: `d35ec35486912e4bca1fdc2a7125ecc6d53580eb`.
- [Windows run 33966232573](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33966232573), all gates passed.
- Portable ZIP: 43,773,937 bytes; SHA-256
  `4c3a2d8c2e36918e3fa77b0c106a174d1ea01bf27b6ecb0ea8d3400fa1c382d0`.
- Full local portable replay, independent files, strict ribbon geometry and ten
  isolated-profile checks passed; Windows registry unchanged.
- [Persistent CI evidence](../evidence/development/2026-09-05-saribbon/README.md)
  and [owner review](OWNER_REVIEW.md). This is not a release or full parity certificate.

## Historical verified checkpoint — before SARibbon

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

## Implemented SARibbon UI wave

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

Source `18d3f7342f4902ac5d103c06ab661744447fa1c4` passed the complete
[Windows CI 33964068198](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33964068198):
exact 1920x1080 qwindows client, all ten measured Home boundaries, narrow/DPI
containment, native interaction and independent file checks. However, the local
independent parser rejected duplicate PLINE vertices: RS_Settings still read
Windows registry snap preferences despite the intended INI test profile.
That package is not the final handoff.

Source `d35ec35486912e4bca1fdc2a7125ecc6d53580eb` corrects native and CLI profile
isolation, verifies both directions of Qt/native settings access before the UI
starts, and requires an unchanged registry across ten smoke processes. It also
rejects degenerate PLINE geometry in the producer. Its
[Windows CI 33966232573](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33966232573)
and full local replay/read-back passed. A separate Windows pointer LINE/Enter/save
review produced the expected 62.5 mm native LINE in a synthetic DXF.
Normal user preferences are retained, not reset. Failure/correction history is
in DEVELOPMENT_PLAN and TEST_REPORT; no preceding source proves a later binary.

## Current limitations and next work

- Exact AutoCAD 2024.1.2 visual and interaction equivalence remains unproved.
- Real paperspace, Model/Layout tabs, viewport cameras/locks and layout DXF
  persistence are not implemented; see [PAPERSPACE_PLAN](PAPERSPACE_PLAN.md).
- The Properties palette is read-only and delegates editing to ModifyEntity.
- Its document entity/Modified summary can lag drawing/save changes until a
  native selection/layer/activation refresh; add a document-change notification path.
- A stricter ezdxf audit finds one orphan PLOTSETTINGS record (repair 202) in
  saved synthetic DXFs. Geometry passes, but whole-file zero-repair DXF is not claimed.
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
