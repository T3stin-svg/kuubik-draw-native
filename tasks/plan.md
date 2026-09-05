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
