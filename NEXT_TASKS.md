# Kuubik Draw Native — next tasks

The first owner-reported reproducible problem always outranks this list.

## P0 — next executable workflow wave

- [ ] Test `0.2.0-preview.2` with Reio and record exact failures for LINE,
  PLINE, MOVE, COPY, OFFSET, TRIM, layers, DXF save/reopen and PDF export.
- [ ] Convert each confirmed failure into a synthetic native regression fixture.
- [ ] Add a native GUI smoke for PLINE from ribbon to committed entity and DXF
  read-back.
- [ ] Add a native GUI smoke for MOVE or COPY including atomic Undo/Redo.
- [x] Add an automated packaged-MSVC layer create/current workflow and verify
  the saved DXF layer (`ee8e29264`, run `33645437662`).
- [x] Add automated DXF open → edit → save-as → close/reopen and independent
  read-back in the packaged EXE (`ee8e29264`, run `33645437662`).
- [ ] Replay both workflows on Reio's owner-controlled Windows system and
  record any discrepancy from the synthetic smoke.

Definition of done: the changed action runs through ribbon/menu → native canvas
or selection → native document → undo/file output, with no silent failure.

## P1 — everyday 2D acceptance matrix

- [ ] ARC, CIRCLE and RECTANGLE pointer workflows.
- [ ] ERASE, ROTATE, OFFSET, TRIM, EXTEND and FILLET.
- [ ] ORTHO and END/MID/CEN/INT snap markers and committed coordinates.
- [ ] layer color, visibility, lock and lineweight.
- [ ] text, DIMLINEAR, hatch and basic block workflows.
- [ ] vector PDF visible-geometry and page-size read-back.

## P1 — Kuubik visual refinement

- [ ] Reio accept/deny the current 66 mapped Kuubik technical-line SVGs and
  recorded provenance; keep inherited LibreCAD fallbacks where unmapped;
- [ ] validate the existing native `ModifyEntity` edit dialog on Reio's real
  selections; keep the right Properties summary read-only;
- [ ] ribbon keyboard/focus/disabled/checked states;
- [x] qwindows + Qt scale-factor smoke at 100%, 125%, and 150%, including a
  narrow 1280-logical-pixel case (`ee8e29264`, run `33645437662`);
- [ ] real Windows Settings display-scale checks at 100%, 125%, and 150%;
- [ ] Reio accept/deny: keep Draw/Modify in `More` at 1280 logical pixels or
  request a separate hybrid direct-button layout;
- [ ] command line history clarity and error states.

## P2 — reliability and distribution

- [ ] autosave/recovery and crash fixture;
- [ ] larger DXF performance test;
- [ ] font/linetype missing-resource behavior;
- [ ] optional Authenticode signing and installer after owner approval.

## Explicitly not next

- do not restart the React/Canvas CAD engine;
- do not remove inherited LibreCAD tools;
- do not begin 3D work;
- do not claim native DWG/DWT/XREF parity;
- do not perform a production release or merge to upstream `master`.

## Required report after each wave

Report the branch, worktree, exact commit, changed commands, targeted tests,
Windows workflow URL, output read-back, artifact SHA-256, honest limitations and
the next blocker. Update this checklist and the handoff docs before stopping.
