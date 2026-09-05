# Kuubik Draw Native roadmap

Updated 2026-09-05. Direction: free/open-source LibreCAD-based Windows 2D CAD,
with AutoCAD 2024.1.2 visual placement and workflow as the target, including real
paperspace. A staged delivery is not a reduction of that final goal.

## Now — SARibbon checkpoint review and bounded corrections

Owner: lead integrator. Five-hour capacity, approximately four planned hours and
one hour of correction/handoff buffer; see [DEVELOPMENT_PLAN](DEVELOPMENT_PLAN.md).

The UI checkpoint is implemented and tested: source d35ec354, Windows run
33966232573 and full local replay. Reio's acceptance remains open. Before the
paperspace slice, address the document-summary refresh gap and orphan PLOTSETTINGS
audit repair documented in NEXT_TASKS; the passing UI suite is not lossless-DXF proof.

Delivered scope:

- Integrate pinned MIT SARibbon into the existing native action-bound workspace.
- Use measured Home panel order/widths, a single Layers panel and native pen
  controls in Properties; retain four direct Draw commands at narrow widths.
- Preserve Classic, real current-layer selection, Tool Options and native actions.
- Require Windows build, visible GUI interaction, DPI/read-back and portable checks.
- Record evidence and give Reio a runnable checkpoint, not an untested mockup.

Any reproducible owner-found Draw/Modify/file regression interrupts UI work as P0.

## Next — true native paperspace vertical slice

Owner: lead integrator. Start after the UI checkpoint; duration is not yet committed.
[PAPERSPACE_PLAN](PAPERSPACE_PLAN.md) defines document ownership, contexts,
transforms, undo and persistence.

- A3 layout, two views of one model at 1:50 and 1:100, independent cameras and locks.
- Enter/exit ModelThroughViewport; one edit updates both views.
- Shared native Undo/Redo, DXF save/reopen, zero structural repair errors.
- Scale-accurate vector PDF and unknown-object safety before compatibility claims.

SARibbon does not provide any of this CAD-engine behavior.

## Next — everyday AutoCAD-like command lifecycles

Retain native geometry/undo while matching command prompts, base/target points,
selection, options, Enter/Esc and dynamic input. Prioritize MOVE, COPY, OFFSET,
TRIM, EXTEND, FILLET, dimensions and editable Properties. The existing tests for
in-place COPY and modal MOVE are regressions, not AutoCAD parity certificates.

Expand native GUI/file evidence for CIRCLE/ARC/RECTANGLE, layers, text, hatch and
blocks. Keep all inherited commands available.

## Later — reliability and free file compatibility

- Autosave/recovery, crash/reopen, missing fonts/linetypes, large DXFs and memory.
- Real Windows display scaling and multiple monitors; same-state AutoCAD reference
  comparison with private images kept out of the public repository.
- Continue libdxfrw for DXF and ezdxf as an independent test oracle.
- ACadSharp remains a bounded MIT file-adapter candidate, not a promised lossless
  DWG runtime. Broader corpus, object preservation, packaging and license review
  precede shipping. No additional .NET dependency in the current UI milestone.
- Signing/installer only after explicit owner approval.

## Reprioritization record

This roadmap supersedes the older "visual refinement later / optional licensed
ODA or RealDWG" order. Reio rejected license fees, kept LibreCAD and selected the
UI milestone before the native paperspace prototype. Open CAD Studio's tested
workflow is a useful reference; its observed DXF structural errors and different
stack rule it out as the selected replacement.

The [research notes](RESEARCH_NOTES.md) distinguish tested behavior, source-based
assessment and unverified assumptions. Existing gaps remain on the roadmap.

## Release gates

An exact-source Windows artifact, package-only Qt loading, targeted native GUI
proof, independent file read-back, payload/license inspection, Gitleaks, SHA-256
and honest limitations are mandatory. Work-branch CI approval does not authorize
a release, remote merge, tag replacement or a production/parity claim.
