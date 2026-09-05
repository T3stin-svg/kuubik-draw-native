# Kuubik Draw Native — next tasks

Updated 2026-09-06. Reproducible owner-found failures always take priority.

## P0 — local and Windows verification complete; owner gate open

- [ ] Reio reviews the delivered SARibbon checkpoint using `docs/OWNER_REVIEW.md`.
- [x] Locally verify native document-change notification for the Properties entity/Modified
  summary after drawing, Undo/Redo and save; count active entities, not undone
  retained objects. Keep selection callbacks and no polling/duplicate model.
- [x] Fix the inherited PLOTSETTINGS ownership/dictionary writer and add an
  audit-clean modelspace fixture. Reproduce repair 202 before the fix; require
  zero errors/repairs for four standalone and ten GUI ASCII outputs after it.
- [x] Verify both P0 corrections with exact-source MSVC CI `33977714231`, source
  `3cefc819`, downloaded package hash/manifest and full local portable replay.
- [ ] Obtain owner acceptance. Reio approved work-branch pushes and Windows CI
  for the current five-hour development wave; release/remote merge remain separate.
- [x] F-01 local: correct all 17 two-byte Boolean writes; fifteen binary outputs
  cover AC1015/1018/1021/1027/1032, defaults and explicit false/true, native reread
  and independent audit 0/0. Existing ASCII/file contracts pass again.
- [x] F-01: verify source `81103460`, MSVC run `33990304131`. This does not certify arbitrary binary DXF
  or add binary Save As to the native application (its exporter remains ASCII).
- [x] Diagnose the isolated-settings startup fatal from the local MSVC replay;
  retain per-sync status and native stderr, test locked INI/report/combined failures.
- [x] Verify that diagnostic change in the same exact-source MSVC CI; original environmental
  cause remains unspecified, with initial failure and successful full retry both retained.
- [x] Verify the final `81103460` ZIP/source/SHA and full local portable replay:
  ten isolated processes, unchanged registry, independent outputs/ribbon/negative
  PDF checks and the three actual settings failure cases all pass.

- [x] Rebuild corrected packaged test guide: `7b897ea8`, MSVC `33991361757`;
  downloaded SHA/source and full local replay pass. Application code unchanged.

## Completed — approved SARibbon UI milestone

- [x] Verify clean baseline 9968198b and product-branch ancestry.
- [x] Persist DEVELOPMENT_PLAN, RESEARCH_NOTES and PAPERSPACE_PLAN.
- [x] Pin SARibbon v2.9.0 and MIT attribution; qmake integration, no QWindowKit.
- [x] Implement shared-QAction ribbon and Home Layers/Properties consolidation.
- [x] Pass work-branch Windows CI 33966232573 / source d35ec354.
- [x] Pass MSVC/Qt build and existing native GUI/file/package regressions.
- [x] Pass all ten Home panels, measured full-width boundaries and label containment.
- [x] Pass four directly visible Draw buttons at 1200/1280 logical widths.
- [x] Pass focused Space activation, enabled/checked states and stable presentation;
  full physical Tab-order audit remains distinct.
- [x] Verify native current-layer and pen/options ownership through Classic roundtrip.
- [x] Review qwindows screenshots and run the delivered EXE locally.
- [x] Record exact source/run/ZIP hash and give Reio a short owner checklist.
- [x] Fix native/CLI settings isolation; ten process checks and unchanged registry.

[DEVELOPMENT_PLAN](docs/DEVELOPMENT_PLAN.md) is the five-hour execution log.
Executable source is d35ec354; later handoff/docs/evidence commits are not separate
binary claims. Full local independent replay passed. No release or merge is authorized.

## Existing verified Windows baseline, not new SARibbon evidence

Run 33919335101 / source 9968198b supersedes d17e8b2. The following no longer need
to be rediscovered as missing implementations:

- [x] LINE/PLINE Enter completion, dynamic input, global Escape in packaged Windows
  native automation; distinguish offscreen command smoke from qwindows visual captures.
- [x] Native PLINE and COPY Undo/Redo with independent three-state DXF read-back.
- [x] Native modal MOVE and Undo/Redo.
- [x] Layer create/current and DXF open/edit/save/reopen.
- [x] Native snap/tracking precision contracts, direct Draw layout and status row.
- [x] Package-only Qt loading, Qt-scale 100/125/150% captures, Tool Options containment.

These checks protect inherited behavior; COPY's in-place duplication and MOVE's
native dialog are not identical AutoCAD command lifecycles.

## Next — native paperspace vertical slice

- [x] P1-01a: camera fields and native/raw-tag regression; MSVC run `33981465387`.
- [x] P1-01b: read LAYOUT/BLOCK_RECORD and distinguish subclass/reactor owners;
  local full build and bounded native/independent tests pass; same reader MSVC run.
- [x] P1-01c local: bounded layout identity/page settings, two saves, raw links and
  audit 0/0; failure-safe replacement and writer reuse including interrupted writes.
- [x] P1-01c: full application regression and exact-source MSVC CI `33983851259`,
  source `6e49a93a`.
- [x] P1-02a local: implemented native paperspace save
  guard; include unknown/discarded geometry, paper blocks, 102 scope, compatibility
  import, ordinary model saves and actual UI autosave/retry behavior.
- [x] P1-02a: exact-source MSVC `33986140500` at `fcad4372`.
- [x] P1-02a: downloaded portable hash/source and full local replay; registry unchanged.
- [x] P1-02b metadata core: RS_Graphic value registry, same-kind live IDs/new IDs,
  shared cycle-owned Undo, newDoc/close cleanup; 54 local checks and full native
  GUI/file/guard regressions pass.
- [x] P1-02b: exact-source MSVC `33987457900` at `a601e807`.
- [ ] P1-02c: validate staged DXF membership and identity bindings before native
  import commit; prove DCS/WCS conversion with nonzero targets and both twist signs.
  Keep save protection until native export/reopen passes. See PAPERSPACE_PLAN.
- [x] P1-03a: Qt camera/clip/vector probe; 36 native checks, independent vector
  measurements and six negative PDF oracles; MSVC `33988991011` at `15eed2e2`.
- [ ] P1-03: connect the camera to native render, hit/snap and command contexts.

- [ ] Implement the single-document architecture in PAPERSPACE_PLAN.
- [ ] A3 layout and two shared-model viewports at 1:50 and 1:100, independent locks.
- [ ] Model/Paper/ModelThroughViewport coordinate, selection and snap context.
- [ ] Model editing through either viewport; native Undo/Redo updates both views.
- [ ] DXF owner/handle/layout identity, reopen and zero independent repair errors.
- [ ] Vector PDF: 5000 mm model line becomes 100 and 50 mm on paper.
- [ ] Fail safely on unsupported objects; no silent destructive write-back.

## Later — broader owner workflow acceptance

- [ ] Pointer workflows for CIRCLE, ARC and RECTANGLE.
- [ ] ERASE, ROTATE, OFFSET, TRIM, EXTEND and FILLET.
- [ ] AutoCAD-like MOVE/COPY prompts and selection/base/target behavior.
- [ ] Layer color/visibility/lock/lineweight, text, dimensions, hatch and blocks.
- [ ] Editable Properties using the same native document and Undo.
- [ ] Owner acceptance of Kuubik icons and native fallback artwork.
- [ ] Actual Windows Settings DPI and multi-monitor checks.
- [ ] Larger DXFs, autosave/recovery, missing fonts/linetypes and crash fixtures.
- [ ] Fix inherited SVG CLI absolute `--outfile` handling and failure exit status;
  retain independent file-presence/content checks (RESEARCH_NOTES).
- [ ] Broader free file-adapter evaluation before any DWG compatibility promise.

No web-engine restart, 3D, paid CAD SDK, client-data publication, release or
upstream/master merge. Keep every inherited LibreCAD command reachable.
