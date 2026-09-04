# Kuubik Draw Native — next tasks

The first owner-reported reproducible problem always outranks this list.

## P0 — next executable workflow wave

- [x] Make canvas and blank-command-line `Enter` finish native LINE and PLINE;
  keep committed segments, discard only the pending preview and finalize PLINE
  as one native Undo/Redo unit. Local Qt 5 GUI smoke passed on 2026-09-03.
- [ ] Replay LINE/PLINE Enter completion in the packaged Windows qwindows
  workflow before calling the behavior Windows-verified.
- [x] Make canvas `Esc` globally clear active commands and add cursor-adjacent
  LINE/PLINE length-angle dynamic input with numeric entry and `Tab` switching.
  Local Qt 5 GUI smoke passed on 2026-09-03.
- [x] Replace separate END/MID/CEN/INT status buttons with an AutoCAD-familiar
  OSNAP popup backed by the seven available native snap modes; add distinct
  endpoint, midpoint, center, intersection and nearest overlay markers.
- [ ] Implement and certify missing higher-order snaps/tracking (perpendicular,
  tangent, quadrant, extension and acquisition tracking) before exposing them.
- [ ] Test `0.2.0-preview.2` with Reio and record exact failures for LINE,
  PLINE, MOVE, COPY, OFFSET, TRIM, layers, DXF save/reopen and PDF export.
- [ ] Convert each confirmed failure into a synthetic native regression fixture.
- [x] Add a native GUI smoke for PLINE from a physically visible responsive
  ribbon surface to committed entity, quick-access Undo/Redo and three-state
  independent DXF read-back (`aaff14484`, run `33660926998`).
- [x] Add a native GUI smoke for COPY from a physically visible responsive
  ribbon surface through canvas source selection, atomic quick-access
  Undo/Redo and three-state independent DXF read-back (`d17e8b23b`, run
  `33665520217`).
- [ ] Add a separate native GUI smoke for MOVE including its native modal
  workflow and atomic Undo/Redo.
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

- [x] Replace the compact drafting row with the AutoCAD-familiar grey/blue
  status bar and persistent far-right customization menu; prove all ten status
  buttons have functional native/settings/dock bindings, all twelve supported
  controls have show/hide entries, four coordinate formats are available and
  hide/restore works in the local UI contract (2026-09-04).
- [ ] Reio accept/deny the current 66 mapped Kuubik technical-line SVGs and
  recorded provenance; keep inherited LibreCAD fallbacks where unmapped;
- [ ] validate the existing native `ModifyEntity` edit dialog on Reio's real
  selections; keep the right Properties summary read-only;
- [ ] ribbon keyboard/focus/disabled/checked states;
- [x] qwindows + Qt scale-factor smoke at 100%, 125%, and 150%, including a
  narrow 1280-logical-pixel case (`aaff14484`, run `33660926998`);
- [x] verify native LINE and DIMLINEAR Tool Options containment and exact
  active widget sets at 1280×600 (`f1c6733e`, runs `33654660495` and
  `33660926998`);
- [ ] real Windows Settings display-scale checks at 100%, 125%, and 150%;
- [x] Reio decision: reject the generic `More`-first Home presentation and keep
  direct `Line`, `Polyline`, `Circle` and `Arc` buttons visible; implemented
  and locally rendered at 1280x600 on 2026-09-03.
- [ ] Run the Windows qwindows/native workflow for the direct Home Draw layout
  and confirm 100%, 125% and 150% rendering before treating it as verified.
- [ ] Reio accept/deny: keep the now-validated native Tool Options toolbar
  inline or move it to a separate row;
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
