# Kuubik Draw Native repository rules

## Owner and purpose

Reio leads Kuubik Projekt OÜ in Estonia and uses CAD/BIM software for practical
structural and architectural design work. Communicate directly in Estonian when
working with Reio. Prefer working, checkable outcomes over speculative feature
lists.

This repository is the native Windows 2D Kuubik Draw product. It is a GPLv2
fork of LibreCAD. The old React repository at `T3stin-svg/kuubik-draw` is a
separate archived direction and must not be mixed into this native codebase.

## Mandatory start

Before editing:

1. read `AI_START.md`, this file, `docs/PROJECT_STATE.md`,
   `docs/DECISIONS.md`, `docs/ROADMAP.md`, `NEXT_TASKS.md`, and
   `docs/TEST_REPORT.md` completely;
2. run `git status --short --branch`, `git remote -v`, and `git rev-parse HEAD`;
3. preserve every pre-existing modification and untracked user file;
4. confirm that the work is based on `kuubik/visual-v0.2`, not upstream
   `master`.

## Git and release safety

- Use a `codex/` branch or isolated worktree for new implementation waves.
- Never use `git reset --hard`, `git clean`, `git add -A`, or force-push.
- Stage an explicit allowlist only.
- Keep `origin` as `T3stin-svg/kuubik-draw-native` and `upstream` as
  `LibreCAD/LibreCAD`.
- Do not merge to `master`, publish a production release, or replace an
  existing release without explicit Reio approval.
- Preview releases must identify the exact source commit and artifact SHA-256.
- Do not change a parity/certification statement because a unit test is green;
  require the documented native workflow and independent read-back.

## Product and licensing boundaries

- Preserve GPLv2, LibreCAD history, author notices and third-party licences.
- Do not copy Autodesk logos, proprietary icons, binaries or private reference
  images.
- LibreCAD's existing GPL icons may remain until Kuubik originals replace them
  with recorded provenance.
- Keep the product 2D. Do not introduce a 3D view cube or 3D workflow without a
  new explicit product decision.
- Keep all inherited LibreCAD functions reachable through menus or Classic
  workspace unless Reio explicitly approves removal.
- Guaranteed preview workflows are DXF open/save and vector PDF export.
- DWG, DWT and XREF are not certified. Do not represent upstream experimental
  conversion as native AutoCAD parity.
- Never put a client drawing, private AutoCAD screenshot, `.env`, credential,
  local application settings or user profile path in the public repository.

## Architecture rules

- The compact Kuubik ribbon must bind to existing LibreCAD `QAction` objects.
- Do not duplicate a CAD command in a second command engine merely for the UI.
- Keep native `RS_Graphic`, entity, layer, undo and file-adapter behavior as the
  source of truth.
- Preview and commit must use the same native geometry behavior.
- A command-changing wave needs a real action-to-canvas-to-document wiring
  proof, not only action registry inspection.
- A file-changing wave needs an independent parser/read-back when available.

## Test frequency

During implementation, run focused compile/tests for the changed component.
Before a preview checkpoint, run the Windows workflow that:

- builds MSVC x64 / Qt 5.15;
- packages the portable runtime;
- isolates Qt plugin loading to the package;
- runs native GUI smoke workflows;
- independently reads DXF/PDF/SVG outputs;
- checks payload allowlists and SHA-256.

Run Gitleaks and `git diff --check` before every public push containing new
handoff or evidence files. Do not weaken, skip or delete a failing test merely
to make a release green.

## Handoff maintenance

When a material wave finishes, update `docs/PROJECT_STATE.md`,
`NEXT_TASKS.md`, `docs/TEST_REPORT.md`, and `docs/DECISIONS.md` if a decision
changed. Rebuild the AI handoff archive with
`scripts/build-ai-handoff.ps1` for the next public preview.

## Herdr orchestration

You are the lead agent for this repository.

When a task has genuinely independent workstreams, use the installed Herdr
skill and Herdr CLI to create named Codex worker agents in sibling panes. Give
each worker one bounded task, monitor its state, wait for completion, read its
result, and review the work before integration.

Read-only workers may share the current checkout. Every worker that edits code
must use its own git worktree and dedicated branch created from the intended
baseline commit. Never allow two workers to edit the same files. A worker must
report its changed files, tests, result, and commit hash.

The lead owns task decomposition, worker coordination, review, integration into
the lead branch, final testing, and the report to the user.

Approval of a bounded implementation milestone authorizes local worker commits
on its dedicated task branches and lead-owned local integration for that
milestone. Do not create workers for small sequential tasks. Pushing,
publishing, remote merging, releases, deletion, discarding existing changes,
and destructive actions still require explicit authorization.

Before starting workers, confirm HERDR_ENV=1, inspect current agent and Git
state, preserve all existing commits and user changes, and read the Herdr skill
instructions.

Preserve all existing commits and user changes. Treat running previews and
background processes as user-owned; do not close or interrupt them unless
explicitly requested.
