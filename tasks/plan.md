# Five-hour native paperspace foundation — 2026-09-05

Window: 2026-09-05 16:10:46–21:10:46 UTC (19:10–00:10 EEST).
Baseline: clean `3cefc819620335852f3fc807d5ee2c60bb0e3033`, existing integration branch.
User authorized five hours of continued development using installed plugins.
Reio explicitly approved work-branch push and the existing Windows MSVC CI in
the asynchronous reply. No release, remote merge or user-file overwrite.

## Contract

Use the existing LibreCAD engine and libdxfrw. The first file-contract slice is
ASCII DXF 2018, one shared model and an A3 landscape TEST layout with two rectangular
160 mm viewports at 1:50 and 1:100. Persist names, IDs, owners, view centers,
view height, twist and zoom lock. Scope claims to an explicit synthetic corpus.
Never infer UI or arbitrary-file support from a standalone adapter's pass.

Task tracking stays in the repository's designated `docs/ROADMAP.md` and
`NEXT_TASKS.md`, not a second todo file. Update the roadmap during work.

## Ordered slices

1. **P1-01a: viewport camera records.** Initialize all existing numeric fields;
   preserve view direction/target, height, twist and status flags through the real
   reader/writer. Reuse the test adapter's no-op interface once a second test needs
   it. Three acceptance checks: independent known-input values, native reopen,
   independent written values including 0°/30° and locked/unlocked. This slice
   does not yet establish layout ownership. Expected production files: existing
   drw_entities.h/.cpp and libdxfrw.cpp; test files remain separate.
2. **P1-01b: layout and block-record read contract.** Add the minimum initialized
   records and compatible callbacks. Distinguish repeated group codes by subclass
   and reactor context. Acceptance: existing synthetic Model/TEST objects and
   block/viewport owner links are read without confusing either owner 330.
3. **P1-01c: complete bounded layout write contract.** Reserve dictionary/layout/
   block/entity handles before writing references; preserve imported identity
   where supported and prevent collisions. Acceptance: independently audited
   create/read/write/read preserves the two viewports, dimensions, owners and IDs;
   0 errors and 0 repairs; modelspace P0 corpus still passes.
4. **P1-02 preparation / integration only after file proof.** Define native
   ownership and safe unsupported-layout behavior against RS_Graphic and its Undo
   cleanup. Implement only a independently reviewable slice that fits the remaining
   window; do not rush tabs/rendering before record ownership and file safety pass.
5. **Checkpoint.** Focused native build and regression suite, diff review, scoped
   local commits and evidence/docs. Run the authorized exact-source MSVC CI and
   verify its artifact. At the timebox end record completed vs remaining work.

## Review and risks

The stage-0 proposal gets a fresh-context adversarial read-only review before
architecture changes. Behavioral changes first receive failing native tests.
Herdr is unavailable (`HERDR_ENV` unset); do not pretend to use it. The installed
doubt skill permits a generic fresh-context reviewer; editing stays with the lead.
External cross-model CLI review is never invoked without separate authorization.

- Existing VIEWPORT DXF code omits camera height/twist/flags and leaves numeric
  members uninitialized; the DWG parser is not proof of DXF support.
- LAYOUT is currently skipped; BLOCK_RECORD callbacks and layout references are
  absent; the existing block writer allocates new handles.
- Native import discards paper-space containers. Test-level support cannot be
  advertised as safe application write-back until the native adapter is integrated.
- The binary header failure remains F-01, independent of this ASCII contract.
- Unimplemented, negative or unknown input cases must remain explicit; no repair
  suppression, copied model, speculative framework or new product runtime.

## Validation tools

Existing Qt 5.15.2 / MinGW 8.1 developer toolchain under ignored `.artifacts`;
Boost 1.87 headers. Independent ezdxf 1.4.4 is test-only. Use fresh evidence paths.
Existing P0 runner, full native GUI/Properties and PDF/SVG checks stay required
when their code paths change. Windows MSVC/Qt packaging remains the artifact
authority. Read-only registry equality is checked in memory, never published.

## Stage-0 decision record

Claim under review: adding bounded DXF record support before native UI work can
expose the required ownership/camera contract without changing the CAD model.
Fresh reviewer receives the proposal and user contract, not that conclusion.
Findings and resulting changes are recorded here as they arrive.

Review 1 (fresh-context read-only agent) found five actionable gaps; all accepted:
1. Check raw tags before ezdxf loading, including default BLOCK/ENDBLK owners.
   High-level loading normalizes these and can create missing layouts before audit.
2. Specify identity preservation against legacy reserved handles and nonconsecutive
   imported IDs; test collisions and HANDSEED. Existing signed-int handle parser
   limits must be explicit; do not claim arbitrary 64-bit DXF handle support.
3. Require embedded page width/height, mm units, rotation and 1:1 plot scale,
   independently yielding a physical 420 x 297 mm A3 page.
4. Preserve optional main viewport ID 1 separately from the two floating views,
   which have distinct IDs above 1; verify the last-active viewport reference.
5. Fix both frame dimensions to 160 mm with model/paper mm units; view heights
   8000 and 16000 establish 1:50 and 1:100 via paper height / model view height.

Reio explicitly chose to continue with this review and tests, without an external
cross-model CLI. Re-review the bounded ownership proposal after these constraints
are made concrete; camera field behavior first receives its own red/green test.

## P1-01c reviewed export contract

The second writer review accepted the bounded explicit ASCII2018 path, with six
corrections incorporated before implementation:

- Pass retained BLOCK_RECORD records as well as layouts and the complete supported
  LINE/VIEWPORT list. Validate backlinks, names, IDs, units, finite camera/page
  values, separate viewport IDs and last-active references before touching a target.
  The source adapter must establish raw ACAD_LAYOUT dictionary membership.
- Serialize that validated entity list directly with existing native writers;
  do not invoke a second writeEntities callback for the same explicit export.
- Preserve layout/dictionary/block-record/entity handles. Relocate generated
  system handles above them, keep null references zero, check arithmetic headroom
  and every allocation, and correct inherited STYLE/VPORT table owners and dangling
  plot-style references. Patch HANDSEED only after allocation finishes.
- Obtain and validate the millimeter header once; header getters consume values,
  so validation inspects vars. AC1032's missing header case was separately fixed
  in `4a1c4e42`, with raw red/green proof.
- Write an exclusively created sibling temporary file, check serialization and
  flush/close, then perform native replacement. Never remove the destination first.
  Failed preflight, write or replacement must preserve an existing sentinel file.
- Keep export state scoped to one call. Test failure → explicit success → legacy
  export reuse, raw target types/owners before independent loading, nonconsecutive
  and conflicting imported handles, near-limit rejection and zero audit repairs.

This remains a bounded file contract. Unknown source records, arbitrary DXF
losslessness, native layout ownership/Undo and UI stay separate gates.

Implementation review caught three shared-path defects: inverted entity visibility,
an optional VPORT style pointer to absent/wrong-type handle 10020, and stale owned
image definitions after an interrupted legacy write. All are corrected; raw
visibility/style RED checks were recorded before fixes. A Windows CRLF stream
position failure also received a failing raw test and a flush-before-tellp fix.
The final local suite passes four inputs/two saves plus image-recovery output,
raw ownership and independent audit 0/0, twenty-one Windows failure/retry cases,
camera/read/PLOTSETTINGS regressions. Full native build and GUI checks passed,
as did Windows MSVC run 33983851259 for source 6e49a93a.

## P1-02a save guard checkpoint

The guard and both import routes pass 22 isolated native cases, with six ordinary
model saves and sixteen protected paperspace cases, each checking 15 outcomes.
Fresh review exposed skipped geometry, 102 scoping, null/block owner context,
POLYLINE/SEQEND attribution, compatibility-buffer length and UI failure branches.
All were addressed; the positive compatibility case also exposed an inherited
empty-string early exit. Full native/independent GUI regression passes. The common
parser check includes malformed/wide input rejection and nested scalar write-back.
This guard is deliberately conservative and remains active until native preservation
is actually connected; file-API success does not remove the restriction.

## P1-02b ownership and Undo review

The existing read-only reviewer traced RS_Graphic/newDoc/import and RS_UndoCycle.
The next bounded design uses one value-owned layout/page/viewport registry in
RS_Graphic, with stable IDs independent of editable names. Model entities stay
in the existing graphic. Import commits validated staged values without creating
Undo, while edits use the existing native history and Modified notifications.

Before metadata edits, add a narrow history reset and cycle-owned Undo payload
path. Existing entity Undo references remain borrowed. Reset history before
newDoc clears entities, and before graphic members die on close. One metadata
snapshot per current cycle aggregates first-before/final-after states: the current
pointer-ordered set cannot safely toggle multiple whole-registry snapshots.
Payload destructors must not call the document or UI. Cover nested cycles, mixed
LINE/metadata, obsolete redo, save/undo, newDoc and close in applied/undone states.

The native value registry/history checkpoint now passes 54 local checks and full
GUI/independent file/save-guard regression. Review corrections are incorporated:
validate/no-op before starting Undo; idempotent state callbacks; zero-ID native
creation versus retained-ID baseline import; same-kind live-ID enforcement;
const payload observations; successive A/B cycles and replacement C after Undo B.
Native nonempty metadata is still save-protected. Adapter and renderer/UI wiring
remain separate. The earlier guard source fcad4372 passes MSVC run 33986140500.

## P1-03a shared Qt camera checkpoint

RS_PaperViewport now supplies the native QTransform and paper rectangle; native
registry acceptance reuses it. The three numeric REDs are fixed, and 36 native
checks plus existing54 metadata checks and full GUI/file regression pass locally.
Independent Qt PDF probes measure100/50 mm and signed direction with effective
clip/default-page-unit checks; six misleading PDF mutations are rejected. No
renderer/command/plot integration is implied. D-030 records spaces and precision.

Model source a601e807 passed MSVC33987457900. Its ZIP/source/checksum match; the
first local portable replay stopped before UI creation in isolated-profile
read/write verification (fatal message confirmed from its dump). Two focused
fresh-profile125% replays pass. Diagnose that failure without weakening isolation
or declaring the incomplete full replay green. Transform MSVC is the next gate.

The model package's full diagnostic retry now passes10 isolated processes and
independent read-back. Dump evidence narrows the original to a sticky non-NoError
QSettings status while both read comparisons passed, not an evidence-file failure
or DPI rendering issue. The source now retains per-sync status and native stderr.
Three actual locked/report/combined failure cases pass without weakening isolation;
the qCritical suppression RED required including status in each fatal message.

F-01 has been reproduced with the current library: a two-byte boolean payload
after $LWDISPLAY shifts the next group code to2304. Reuse writeBool at all17
identified header/DIMSTYLE sites and verify binary false/true across supported
modern DXF versions, while retaining ASCII and file-ownership regression gates.

The reviewed F-01 matrix now includes defaults and the first affected version:
15 outputs across AC1015/1018/1021/1027/1032, all native reread and independent
audit 0/0. Four ASCII outputs and recompiled camera/layout/owner/context regressions
pass. Full GUI, 36 transform/54 metadata checks, independent files, six PDF negative
probes and four ribbon checks pass. Native application Save As remains ASCII;
this is a bounded codec repair, not a new binary save UI or compatibility claim.

## Final checkpoint and next integration contract

Combined source `81103460` was committed and pushed after Gitleaks/diff checks;
MSVC run `33990304131` passes. Final artifact checksum/manifest and local portable
replay pass, including independent outputs, ten isolated processes, unchanged
registry and the three forced settings failures. Development handoff is being assembled.

The existing read-only reviewer traced the next adapter boundary. Native geometry
currently changes during import, DICTIONARY membership is skipped and entity DXF
handles are not retained. The next task is bounded validated import/identity
bindings and a geometrically proven DCS/WCS codec, followed by native export/reopen.
Save protection remains active until that later gate passes. PAPERSPACE_PLAN lists
concrete call sites, unsupported-input limits and five acceptance checks. No second
geometry model, external review CLI or new product runtime was introduced.
