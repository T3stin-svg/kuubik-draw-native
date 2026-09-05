# Kuubik Draw Native — project state

Status date: **2026-09-05**. **P0-A/B MSVC and local portable passed; P1 records locally verified.**

The current correction session starts at `8a5f7ae0` on the existing integration
branch. P0-A uses native modification notifications and counts active entities.
Its rebuilt Qt 5.15.2 / MinGW 8.1 x64 GUI smoke passes all nine new summary states
and the existing native workflows. P0-B supplies PLOTSETTINGS dictionary/owner/
reactor links; the clean input, four standalone outputs and ten GUI outputs pass
ezdxf 1.4.4 with zero errors and repairs. See [P0_CORRECTIONS](P0_CORRECTIONS.md).
The subsequent five-hour wave has explicit approval for work-branch push and
Windows CI. Source `3cefc819` passed run `33977714231`; its downloaded ZIP SHA-256
is `a38bfa6291f5bed5776f724cad6868b80c135e872cb2d30cca2717cb065f28be`
(43,776,151 bytes). The manifest source matches; full local portable replay and
independent DXF/PDF/SVG verification pass, with unchanged native settings registry.
P1 camera fields pass native/raw-tag read/write checks (commit `04a0f55a`).
LAYOUT/BLOCK_RECORD reading now passes the bounded A3, owner-context and alternate
page/coordinate cases. New callbacks default to no-op for existing adapters.
Layout writing/ownership and application integration remain unfinished.
Owner acceptance, release and remote merge are not implied by these checks.

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
- Reio approved work-branch pushes and Windows CI for this five-hour wave.
  No remote merge, release, default-branch change, paid SDK or paid service is approved.

## Previous Windows development checkpoint — SARibbon

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
- Its document entity/Modified summary refresh is corrected locally in P0-A;
  the last MSVC development artifact still has the old behavior.
- PLOTSETTINGS repair 202 is corrected locally for the bounded ASCII modelspace
  corpus; the last MSVC artifact still contains it. Arbitrary DXF fidelity is unproved.
- A separate exploratory binary-DXF probe fails on an inherited malformed header.
  It is outside the passing ASCII corpus and remains unresolved.
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

MSVC x64 / Qt 5.15 Windows CI remains the build authority. A local isolated
Qt 5.15.2 / MinGW 8.1 toolchain now supports development tests; it does not replace
the required MSVC portable gate. Only a verified new artifact can be offered as
the next portable preview.
