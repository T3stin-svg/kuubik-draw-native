# Kuubik Draw Native — next tasks

The first owner-reported reproducible problem always outranks this list.

## P0 — next executable workflow wave

- [ ] Test `0.2.0-preview.2` with Reio and record exact failures for LINE,
  PLINE, MOVE, COPY, OFFSET, TRIM, layers, DXF save/reopen and PDF export.
- [ ] Convert each confirmed failure into a synthetic native regression fixture.
- [ ] Add a native GUI smoke for PLINE from ribbon to committed entity and DXF
  read-back.
- [ ] Add a native GUI smoke for MOVE or COPY including atomic Undo/Redo.
- [ ] Add a layer create/current workflow and verify the saved DXF layer.
- [ ] Add DXF open → edit → save-as → close/reopen read-back in the packaged EXE.

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

- [ ] original Kuubik icon set with provenance;
- [ ] better Properties workflow backed by native entity editing;
- [ ] ribbon keyboard/focus/disabled/checked states;
- [ ] high-DPI and narrower desktop layouts;
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
