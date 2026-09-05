# Kuubik Draw Native — next tasks

Updated 2026-09-05. Reproducible owner-found failures always take priority.

## P0 — local corrections complete; MSVC and owner gates open

- [ ] Reio reviews the delivered SARibbon checkpoint using `docs/OWNER_REVIEW.md`.
- [x] Locally verify native document-change notification for the Properties entity/Modified
  summary after drawing, Undo/Redo and save; count active entities, not undone
  retained objects. Keep selection callbacks and no polling/duplicate model.
- [x] Fix the inherited PLOTSETTINGS ownership/dictionary writer and add an
  audit-clean modelspace fixture. Reproduce repair 202 before the fix; require
  zero errors/repairs for four standalone and ten GUI ASCII outputs after it.
- [ ] Verify both P0 corrections with the exact-source MSVC x64 / Qt 5.15 portable
  CI artifact, then obtain owner acceptance. Local Qt/MinGW checks passed;
  push and remote CI require Reio's new explicit approval.
- [ ] Investigate the inherited binary-DXF header failure reproduced by the
  optional standalone binary probe. No binary compatibility claim from P0-B.

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
