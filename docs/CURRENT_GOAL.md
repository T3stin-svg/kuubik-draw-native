# CURRENT GOAL — Kuubik Draw AutoCAD-familiar native workspace

## Objective

Build Kuubik Draw Native into a practical Windows 2D CAD workspace whose
visual hierarchy, command placement and everyday workflow are familiar to an
AutoCAD 2024 Drafting & Annotation user, while retaining LibreCAD's proven
native behavior.

The target user is a building or civil engineer. The primary information
architecture therefore prioritizes:

- lines, polylines, rectangles, circles, arcs, cuts and everyday Modify tools;
- current layer, layer visibility, locking, color, linetype and lineweight;
- text, dimensions, leaders, hatch and other annotation;
- block creation, insertion, editing and explode;
- a right-side Properties, Layers and Blocks workflow;
- a full-width command line and compact drafting status controls;
- reliable DXF open/edit/save/reopen and vector PDF export.

This goal is not a claim of certified AutoCAD parity. Familiarity applies to
layout, density, terminology and workflow, not to Autodesk assets or
proprietary behavior.

## Native architecture invariant

Every Kuubik command and state view must use LibreCAD's existing native
QAction, QG_ActionHandler, RS_Document, RS_Graphic, entity, layer, block,
selection, undo and file-adapter behavior.

The Kuubik UI must not introduce a second CAD command engine, parallel document
model, polling-based selection model or preview-only geometry path.

## Product and licensing boundaries

- Keep the product 2D. No 3D view cube, 3D modeler or BIM-authoring workflow is
  part of this goal.
- Preserve GPLv2, LibreCAD history, author notices and third-party licences.
- Do not copy Autodesk logos, icons, screenshots, fonts, binaries, path data or
  other proprietary assets.
- Original Kuubik technical-line icons require recorded provenance. Existing
  LibreCAD icons may remain as a documented fallback.
- Preserve Classic workspace and keep every inherited LibreCAD command
  reachable through Classic or native menus.
- Hiding the menu bar in Kuubik workspace is allowed only when Application or
  workspace controls retain a visible route back to Classic and native menus.
- DXF open/save and vector PDF export remain the guaranteed preview workflows.
  DWG, DWT and XREF are not certified.
- Base work on kuubik/visual-v0.2; implementation stays on the active
  codex/autocad-visual-integration-root integration branch until Reio explicitly
  approves another branch or release action.

## Coordination

This Codex conversation is the single primary work window. The primary agent is
the only integrator, documentation owner and Git-state controller.

Parallel analysis or implementation may use only internal gpt-5.6-terra agents
at High reasoning inside this conversation. Do not create sidebar tasks,
separate Codex sessions or parallel integration worktrees for this goal.

The primary agent must:

- inspect the real branch, commit and dirty state before each integration;
- preserve all pre-existing user changes;
- give each internal agent a bounded task and review its evidence;
- integrate and commit through explicit file allowlists;
- check agent and branch progress at least once per hour;
- keep the hourly heartbeat attached to this conversation and branch;
- never merge to master, replace a release, publish production or force-push
  without explicit Reio approval.

## Active deliverables

### Phase 0 — Audited integration baseline

- Keep the original icon, ribbon and QSS history on one reviewable codex branch.
- Record the exact active commit and distinguish committed work from dirty or
  unverified work.
- Make CURRENT_GOAL, PROJECT_STATE, DECISIONS, ROADMAP and NEXT_TASKS describe
  the same current direction.

### Phase 1 — AutoCAD-familiar visual workspace

- Application button and Quick Access area with native file/undo actions.
- Ribbon tabs and panels ordered for Home, Insert, Annotate, View, Manage and
  Output workflows.
- Clear large/medium/small command hierarchy, active/hover/checked/disabled and
  keyboard-focus states.
- Original Kuubik icons mapped to existing QAction keys.
- Native option and pen controls in the ribbon.
- A real native current-layer selector in the ribbon. A read-only layer label
  does not satisfy this deliverable.
- A right-side read-only Properties dock of about 320 logical pixels, above or
  coherently grouped with Layers and Blocks.
- Bottom native command line and AutoCAD-familiar status controls.
- Kuubik menu-bar behavior with a tested Classic restore path.
- Usable layouts at 100%, 125% and 150% DPI, including a 1280-logical-pixel
  narrow desktop case.

### Phase 2 — Native civil-engineering workflow integrity

- Draw: LINE, PLINE, RECTANGLE, CIRCLE and ARC.
- Modify: ERASE, MOVE, COPY, ROTATE, OFFSET, TRIM, EXTEND and FILLET.
- Layers: create/current, color, visibility, lock and lineweight.
- Annotation: text, linear dimension and hatch.
- Blocks: create, insert and explode.
- Properties: no/single/multiple selection summary from the native selection
  update path; Open Full Properties triggers the existing ModifyEntity QAction.
- Undo/redo, DXF and PDF behavior remain native and unchanged by the UI layer.

### Phase 3 — Verification checkpoint

- Kuubik UI contract schema version 2 while preserving all version 1 keys.
- MSVC x64 / Qt 5.15 Windows build.
- Portable runtime packaging and package-only Qt plugin loading.
- Native GUI smoke proving QAction → canvas/selection → document behavior.
- Independent DXF and vector PDF read-back.
- 100%, 125% and 150% DPI screenshot/read-back.
- Payload allowlists, Gitleaks, git diff --check, exact source commit and
  artifact SHA-256.

## Evidence checkpoint — 2026-09-02

Development source `aaff1448482b861c10ceb8b8cf47326c956284bc` passed the
Windows MSVC x64 / Qt 5.15 workflow in run `33660926998`. The run verified the
native current-layer selector, native Properties callbacks and
`ModifyEntity` delegation, Kuubik-to-Classic restoration, native LINE and
PLINE canvas flows, visible responsive-ribbon overflow selection, quick-access
Undo/Redo, DXF save/reopen, independent DXF/PDF/SVG read-back, package
isolation and exact SHA-256 output. The tested portable ZIP SHA-256 is
`9361e7f0cb612fb17cd2f4b36630c16bb76c9d7e272545245825dd0193864936`.
This is a development checkpoint, not a release.

The same run retained the qwindows + Qt scale-factor screenshot checks at
100%, 125% and 150%. Focused run `33654660495` first proved that the native
LINE and DIMLINEAR Tool Options widgets fit their 1280-pixel inline ribbon host
without stale or duplicate option widgets; run `33660926998` repeated that
test successfully. These tests do not prove that Windows Settings OS display
scaling was changed.

The goal remains active. Before completion it still requires:

- real Windows Settings 100%/125%/150% visual checks on controlled hardware;
- Reio's accept/deny decision for the current responsive Home layout: Draw and
  Modify are shown through `More` in the automated 1200/1280 qwindows captures,
  and Draw also used its working `More` route in the 1920 offscreen workflow;
- Reio's accept/deny decision on a separate qwindows LINE visual-capture gate;
- Reio's accept/deny decision on keeping the now-validated native Tool Options
  toolbar inline versus moving it to a separate row;
- explicit owner acceptance of the current 66-icon Kuubik technical-line set
  and completion or owner acceptance of the remaining primary Draw,
  Modify, Annotation and Blocks workflow evidence in Phases 1 and 2; PLINE and
  quick-access Undo/Redo now have automated native evidence;
- a final exact-source artifact and handoff after those decisions are closed.

## Out of scope

- restarting or merging the old React/Canvas CAD project;
- Autodesk assets or proprietary code;
- full DWG/DWT/XREF compatibility claims;
- 3D or BIM authoring;
- production release, release-tag replacement or merge to upstream master;
- an AutoCAD parity or certification percentage inferred from command presence
  or a narrow unit test.

## Definition of done

This goal is complete only when all of the following are proved against the
same integration commit and native Windows artifact:

1. Kuubik workspace has an AutoCAD Drafting & Annotation-familiar layout using
   only Kuubik/LibreCAD-compatible visual assets.
2. Ribbon, current layer, Layers, Blocks, Properties, command line and status
   controls form one coherent native 2D workflow.
3. Classic workspace and every inherited LibreCAD command remain reachable.
4. Changed commands follow the real QAction → canvas/selection → native
   document/entity/layer/block → undo or file-output path.
5. Properties is read-only, receives native selection callbacks without
   polling and delegates full editing to ModifyEntity.
6. The MSVC/Qt portable build, UI contract, GUI smoke, DXF/PDF read-back and
   100/125/150% DPI checks pass.
7. Handoff documents, exact hashes, known limitations and the next task match
   the delivered artifact.
8. No prohibited merge, release, force-push or unsupported parity claim has
   occurred.
