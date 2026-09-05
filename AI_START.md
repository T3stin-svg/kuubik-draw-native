# AI START — Kuubik Draw Native

If Reio says **"LibreCAD GitHub"**, **"Kuubik Draw GitHub"**, or asks another
computer to continue this project, this is the canonical repository:

<https://github.com/T3stin-svg/kuubik-draw-native>

Do not start from the official LibreCAD repository and do not start from the old
React `kuubik-draw` repository. Kuubik Draw Native is a GPLv2 LibreCAD fork with
its own product branch, release, design layer, tests and roadmap.

## One-sentence continuation prompt

> Open `T3stin-svg/kuubik-draw-native` from GitHub, read `AI_START.md` and the
> linked handoff files completely, then continue the first unfinished phase in
> `docs/CURRENT_GOAL.md` and `NEXT_TASKS.md` without restarting the project or
> changing the release claims.

## New computer bootstrap

```powershell
git clone https://github.com/T3stin-svg/kuubik-draw-native.git
Set-Location kuubik-draw-native
git status --short --branch
git remote -v
git rev-parse HEAD
```

The GitHub default branch must be `kuubik/visual-v0.2`. If a pre-existing clone
opens another branch, run:

```powershell
git fetch origin --tags
git switch kuubik/visual-v0.2
git pull --ff-only origin kuubik/visual-v0.2
```

Never use `git reset --hard`, `git clean`, `git add -A`, force-push, or merge to
`master` to solve a setup problem.

## Required reading order

Read these files completely before editing:

1. `AGENTS.md` — repository rules and safety boundaries;
2. `docs/PROJECT_STATE.md` — what exists now;
3. `docs/DECISIONS.md` — decisions that must not be silently reversed;
4. `docs/CURRENT_GOAL.md` — the owner-approved active objective and evidence
   required for completion;
5. `docs/CHAT_PLAN_SUMMARY.md` — sanitized history of the plans from the working
   conversation;
6. `docs/ROADMAP.md` — product direction;
7. `NEXT_TASKS.md` — exact next work order;
8. `docs/TEST_REPORT.md` and `README_TEST.md` — proof and manual checks;
9. `FORK_NOTICE.md`, `LICENSE`, and `THIRD_PARTY_NOTICES.md` — provenance and
   licensing.

## Current certified checkpoint

- Product: `Kuubik Draw Native 0.2.0-preview.2`
- Release tag: `v0.2.0-preview.2`
- Executable source commit:
  `171d95915f6f5a34b8d9fcb487dd3429de8cda74`
- LibreCAD base: `v2.2.1.5`, commit
  `7ebab007d9eb4c68609388b835a2487648f0877b`
- Verified Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33510020896>
- Release:
  <https://github.com/T3stin-svg/kuubik-draw-native/releases/tag/v0.2.0-preview.2>

The release proves a native ribbon LINE mouse workflow, DXF open/save, vector
PDF export, portable runtime completeness and independent output read-back. It
does not certify DWG/DWT/XREF or full AutoCAD parity.

## Latest development checkpoint — not a release

- Integration branch: `codex/autocad-visual-integration-root`
- Tested source commit:
  `9968198bbe72165e28c48f8e37109fa3eb103212`
- Successful Windows MSVC x64 / Qt 5.15 run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33919335101>
- Tested portable ZIP SHA-256:
  `60bf9445fdc6206e8f1b21a392d72dece93714a56feabc435bf7cb66cca27550`

This development artifact verifies the native current-layer selector,
Properties callbacks, Classic restoration, visible responsive-ribbon LINE and
PLINE activation, visible responsive-ribbon native COPY with quick-access
Undo/Redo, native Tool Options at 1280 logical pixels, and independent
DXF/PDF/SVG read-back. It is an expiring workflow artifact and
must not replace the immutable release without Reio's explicit approval.

This newer baseline additionally verifies the native MOVE selection, modal
workflow and atomic Undo/Redo, LINE/PLINE Enter completion, direct Home Draw,
expanded precision/status contracts and packaged DPI captures. The original
command smoke uses offscreen Qt; DPI screenshots use qwindows. Real Windows
Settings scaling and full AutoCAD command equivalence remain unproved.

## Current work — approved 2026-09-05 UI milestone

Read `docs/DEVELOPMENT_PLAN.md`, `docs/RESEARCH_NOTES.md` and
`docs/PAPERSPACE_PLAN.md` alongside the required files. Reio selected SARibbon
UI integration first and true native paperspace next, retaining a free/open-source
LibreCAD base. The five-hour milestone permits work-branch pushes and existing
Windows CI only; no release, remote merge or paid service. Check PROJECT_STATE
and the actual workflow before treating any SARibbon source as Windows-verified.

## How to continue

1. Inspect the real branch and dirty worktree before making assumptions.
2. If Reio reports a bug from the preview, that bug becomes the first P0.
3. Otherwise take the first unchecked phase item shared by
   `docs/CURRENT_GOAL.md` and `NEXT_TASKS.md`.
4. For the active AutoCAD-familiar integration wave, continue the existing
   `codex/autocad-visual-integration-root` branch in one lead-owned integration
   checkout. The lead is the only integrator. Genuinely independent work may
   use named Codex workers managed through Herdr: read-only workers may share a
   checkout, while every editing worker uses a dedicated task branch and
   separate worktree from the intended baseline. Task worktrees are not
   competing integration checkouts, and editing workers must never own
   overlapping files. The lead reviews and integrates their work.
5. Reuse LibreCAD's real `QAction` and native entity/document model; do not
   duplicate CAD behavior in a second UI engine.
6. Add a targeted native test and independent DXF/PDF read-back where the
   change affects a file workflow.
7. Do not push without Reio's explicit approval. If a push is authorized, push
   only the work branch. Do not remotely merge, change the default branch,
   publish a release, or alter certification claims without Reio's explicit
   approval. Approval of a bounded milestone authorizes its local worker
   commits and lead-owned local integration only.

If any file conflicts with a live repository check, the live repository wins;
update the handoff documentation in the same change.
