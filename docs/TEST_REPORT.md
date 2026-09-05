# Kuubik Draw Native — verified test checkpoint

## F-01 — binary Boolean payload width (local)

Both the original optional binary probe and the expanded corpus failed native
reread before the fix. Independent parsing found header tag 2304 immediately after
$LWDISPLAY: its group 290 payload incorrectly occupied two bytes, shifting the
following group 9. All 16 header calls and the DIMSTYLE DIMFXLON call now use the
existing writeBool; group 280 behavior and the byte writer itself are unchanged.

The final corpus has 15 binary files: AC1015/1018/1021/1027/1032, each with defaults,
false and true. Native reread checks header flags, DIMFXLON and entity counts;
independent parsing checks full tags/EOF, exact version, flags, geometry, units,
PLOTSETTINGS ownership and audit 0/0. Four ASCII regressions pass, as do recompiled
camera/layout/read/write/common-entity tests. Full native GUI passes with 8 isolated
profiles and unchanged registry. CI now invokes `test-dxf-plotsettings.py --binary`
and uploads the binary corpus; exact-source MSVC is pending.
The same final application also passes all 22 native save-guard cases, independent
DXF/PDF/SVG, six negative PDF oracles and four ribbon geometry checks.

This closes the reproduced Boolean-width defect only. The native application's
Save As still uses ASCII; arbitrary binary compatibility, application chunks,
legacy pre-R13 encoding and full paperspace roundtrip are not certified.
Evidence: `binary-header-red`, `binary-matrix-red`, `binary-matrix-reviewed`,
`binary-ascii-regression`, `native-binary-final` and `*-binary-regression` under the
ignored wave root. Format rule: [Autodesk binary DXF](https://help.autodesk.com/cloudhelp/2023/ENU/AutoCAD-DXF/files/GUID-FC1C3C69-DBC2-49E4-893A-000D6538C0FE.htm).

## P1-03a — Qt camera, exact rectangle clip and vector probe

The minimal Qt implementation first failed three real numeric cases: collapsed
paper frame, lost paper-center translation at WCS 1e20, and determinant overflow
with a finite but incorrect inverse. The hardened factory rejects those before
native metadata edits. The final native report passes 36 checks covering both
axes at 0/30 degrees and 1:50/1:100, a non-square frame, inverse, fractional WCS
coordinates around 1e9, inch/meter drawing units, flags, twelve invalid inputs,
exact rotated point membership and both raster clips with separate ink/endpoints.
The existing 54 metadata checks and full GUI/file regression still pass; eight
isolated profiles leave the registry unchanged. Four ribbon geometry checks pass.

The QPdfWriter probe contains two vector LINE strokes. Independent pypdf graphics
state, matrix and rectangle-clip checks measure 100/50 mm, centers and signed
0/-30-degree directions within 0.05 mm. Six serialized in-memory corruptions must
fail: empty clip, doubled UserUnit, page rotation/crop, doubled or mirrored geometry.
The first page-size oracle incorrectly imposed geometry tolerance on Qt 5's
integer-point MediaBox. Source inspection established 1191×842 pt for A3; only
page extent allows half-point rounding. Geometry tolerance was not relaxed.

Evidence: ignored `native-transform-red`, `native-transform-green`,
`native-transform-reviewed`, `transform-probe-verifier-negatives.log` under the
wave root. These are camera/Qt painter probes, not native layout-renderer or plot
acceptance. Transform source `15eed2e2` passes MSVC run `33988991011`, including
the native probe and independent negative oracle checks. Later settings/F-01
source changes need their own MSVC checkpoint.

## P1-02b — native layout metadata and shared Undo

The new native check first proved that newDoc left an Undo cycle pointing to
already deleted LINE data. History now clears before entity deletion and before
RS_Graphic members die. Cycle-owned metadata is distinct from borrowed entities.
The value registry holds paper layouts and rectangular camera metadata; there
is no second model geometry or separate history.

`layout-model-smoke.json` passes 54 native checks: two edits inside a mixed LINE/
metadata cycle; idempotent state notifications; successive A/B cycles, both Undo/
Redo directions and obsolete B replaced by C; close with applied/undone history and reset an open cycle;
newDoc; Modified; no-op/invalid changes preserving redo; live-kind and retired-ID
rejection; zero-ID creation, imported IDs and exhaustion; copied input ownership;
empty-history import policy; refusal to save nonempty native metadata. Twelve
invalid page/camera/identity/name cases leave state/history/Modified unchanged.

The full GUI suite and 22-case native save-guard suite pass again with the final
local model code. Independent DXF/PDF/SVG validation passes and checks the saved
5000 mm baseline remains intact. Eight GUI and 22 guard profiles are isolated;
native registry equality is unchanged. Evidence: `native-layout-model-reviewed`,
`native-layout-model-guard-regression` and their logs under the ignored wave root.
The original failure is `native-layout-model-red/gui-evidence/layout-model-smoke.json`.

The Windows workflow executes this model check as part of the existing native GUI
smoke and uploads its report/baseline. Source `a601e807` passes MSVC `33987457900`.
The downloaded ZIP is 43,805,010 bytes with matching source/sidecar and SHA-256
`9ae9675a920d9a80266ee13f6fb7ccd7d1ab78f2c8501611a6fed5f15b00f783`.
Its first local packaged replay aborted at125% before UI construction, code
0xC0000409. The isolated settings INI contains both probe keys but no completed
isolation report; diagnosis is in progress. Two focused125% runs pass using the
extracted and original copied executables with fresh profiles and unchanged registry.
The subsequent full replay of the unchanged package passes10 isolated processes,
unchanged registry and exact-model-source independent DXF/PDF/SVG verification.
That replay uses the same portable assertions with stderr capture added. Original
failure and crash dump remain local; the dump confirms both read directions were
true but QSettings status was non-NoError before opening the evidence report.
Its specific filesystem cause is not established. Staged adapter import,
paper entities, rendering and UI remain unconnected.

### Isolated settings failure diagnostics

The check now records status after each sync, writes a failed probe report when
possible, and reports distinct settings-sync/report-open/write/commit failures.
The portable harness asynchronously drains stderr and preserves a unique failure
log, including on timeout. No retry, reduced assertion or fallback to user settings
was added. The three reviewed failure cases are a directory blocking the report,
a Windows share-locked INI, and both faults together. All stop before UI. The
locked case retains AccessError at all three sync observations with both read
directions true; a simultaneous report failure cannot hide that primary diagnostic.
Registry unchanged. The earlier full GUI and independent regression also pass
with diagnostic source (`native-settings-diagnostic-green`). The last review adds
only the combined failure log and assertion, covered by `settings-failures-verified`.
An intermediate regression proved that the installed Qt handler suppresses the
separate qCritical message; all fatal report stages now include probe status directly.

## P1-02a — native paperspace save protection (local)

The bounded writer source `6e49a93a` passed Windows MSVC run `33983851259`.
The subsequent native guard passed Windows MSVC run `33986140500` at `fcad4372`.
The original native RED reproduced 11 failed protection assertions; the later
mesh RED remained after removing the null-owner false positive. Review also found
102 scope, block context, POLYLINE/SEQEND attribution, compatibility import and UI
retry/autosave gaps. Each is covered by the final corpus.

- `test-native-paperspace.py`: 22 isolated native processes, both default and QCad 1
  import. Six ordinary model saves retain a 5000 mm LINE and pass audit 0/0,
  including CRLF and nested application data. Sixteen protected cases each pass
  15 checks covering Save/Save As/autosave/direct export, UI refusal and retained
  autosave timer, unchanged original/backup/autosave/target bytes, and fresh-doc
  export. Fixtures include A3 layouts, paper LINE/VIEWPORT/mesh/ordinate dimension,
  unknown entity, nested102 and paper BLOCK content without redundant owners.
- `test-dxf-entity-context.py`: 12 entity/unknown/block/real-POLYLINE contexts,
  12 rejected application groups and nested ASCII/binary scalar payload checks.
  Binary payload width is not a binary-DXF interoperability claim; F-01 is open.
  Wider application integers are rejected because DRW_Variant cannot retain them.
- All prior camera, layout-reader, bounded writer and PLOTSETTINGS tests pass.
  Full native GUI and independent DXF/PDF/SVG regression pass, including nine
  Properties states, four ribbon geometry checks and eight isolated profiles.
  All process runs leave the native settings registry unchanged.

The compatibility positive test exposed two inherited bugs: CRLF normalization
left a stale buffer length, and an empty string value ended parsing before LINE.
Both fixes are required for ordinary model save to remain usable.
The first extended test setup misspelled `$Paper_Space`; no behavior claim was
made until that fixture error was fixed and the complete corpus passed.

Run the Python scripts with the assertion-enabled adapter/KuubikDraw executable
and fresh output directories; the existing MSVC workflow now runs both checks.
Evidence stays under ignored `.artifacts/paperspace-wave/`, notably
`native-guard-verified`, `native-guard-gui-final` and `*-guard-complete`.
Native layout ownership/editing/rendering remain separate unfinished gates.

## Five-hour wave — P0 Windows proof and P1 camera records

P0 source `3cefc819620335852f3fc807d5ee2c60bb0e3033` passed all gates in
[Windows MSVC run 33977714231](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33977714231).
The downloaded portable archive is 43,776,151 bytes, SHA-256
`a38bfa6291f5bed5776f724cad6868b80c135e872cb2d30cca2717cb065f28be`.
Checksum sidecar and build-manifest source match. Full local packaged replay
passes: nine Properties states, ten isolated processes, unchanged native registry
and independent DXF/PDF/SVG read-back. The qwindows reference capture was inspected.
Owner acceptance is separate.

P1-01a is locally green with MinGW 8.1 and ezdxf 1.4.4. Before the fix the raw-tag
check fails on missing VIEWPORT group 45. Afterward it preserves main ID 1,
floating IDs 2/3, 160 mm frames, scale ratios 1:50/1:100, centers, targets,
direction, 0°/30° twist, lock/status flags and the 5000 mm model LINE.
The non-default direction vector is a parser probe, not a 3D application claim.
Four PLOTSETTINGS regressions still pass with audit 0 errors/0 repairs.
Camera-record evidence does not establish raw layout ownership, whole-layout
audit or native paperspace support. High-level ezdxf loading can normalize block
owners before audit; the following P1 slice requires raw-tag checks too.

Compile `tests/dxf-viewport.cpp` and `tests/dxf-plotsettings.cpp` against libdxfrw
with assertions enabled, then run the corresponding `scripts/test-dxf-*.py`
with the adapter and a fresh output directory. The MSVC workflow includes the
new camera check for later commits; run 33977714231 predates it.
The fresh review required complete native camera assertions on both reads and
a non-default direction; these checks were added before the camera checkpoint.

Local setup fixes: regenerate qmake's missing response file and relink its empty
archive; use ezdxf's public layout-rename API and explicit viewport IDs. The true
RED above was obtained after the fixture setup was corrected.

### P1-01c prerequisite — actual DXF 2018 header

The write-contract review found that `write(..., AC1032, ...)` fell through to
the AC1021 header. A new raw `$ACADVER` assertion reproduced that failure. Adding
the missing AC1032 switch case makes the camera roundtrip pass with an actual
2018 header. Earlier camera results prove field values only, not a 2018 output
header. This narrow correction is locally verified in commit `4a1c4e42`; the passed
reader checkpoint CI at `f2879c5b` predates it.

### P1-01b — layout and block-record reading

The native red test failed because neither record reached the adapter. The new
additive callbacks and scoped reader pass four positive inputs: A3 TEST, misleading
nested application groups, an additional ordinary block, and alternative page/UCS
settings with a UTF-8 layout name. Seven malformed application-group cases return
a read error. Layout/page names, plot/layout flags, dictionary owner/block owner,
last-active viewport, paper dimensions/units/rotation, custom scale, margins,
origins, limits, extents and UCS coordinates are independently compared.
Copy construction retains owned XDATA after the parser record resets.

The fresh review found uninitialized BLOCK/ENDBLK members, permissive invalid 102
strings and ambiguous copy guidance; all three were corrected. The inherited
TableEntry copy-assignment ownership defect is outside the used path: retain
records via copy construction, never `saved = callbackData`.

Full local Qt/MinGW application build passes. The native GUI/file replay passes
with eight isolated process profiles, unchanged registry, ten independently clean
DXFs, vector PDF/SVG and the prior Properties/Undo/Redo workflows. Camera and four
PLOTSETTINGS regressions also pass against the new reader. Exact-source MSVC CI
for the reader passed at `f2879c5b86f04354ecf1cde0a76c134568299168` in
[run 33981465387](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33981465387),
including the new camera/read adapters, native GUI and independent file checks.
It predates the AC1032 header fix and layout writer below.

Record fields follow Autodesk's [LAYOUT](https://help.autodesk.com/cloudhelp/2024/ENU/AutoCAD-DXF/files/GUID-433D25BF-655D-4697-834E-C666EDFD956D.htm)
and [PLOTSETTINGS](https://help.autodesk.com/cloudhelp/2024/ENU/AutoCAD-DXF/files/GUID-1113675E-AB07-4567-801A-310CDE0D56E9.htm)
references. Group 147 is preserved numerically: ezdxf names it `unit_factor`,
whereas the Autodesk text calls it a standard-scale factor. Plotting semantics
are not inferred from this read-only check.

### P1-01c — bounded layout export, local evidence

The explicit ASCII2018 exporter preserves the layout dictionary, LAYOUT,
BLOCK_RECORD and LINE/VIEWPORT identities, entity owners, last viewport and
backlinks. Generated table/block/object handles move above retained identities;
the final HANDSEED exceeds all emitted handles. This typed API supports one Model
and one paper layout, mm model/paper units, planar LINE and rectangular VIEWPORT.
The native application does not use it yet; a caller still must reject unsupported
source records before selecting this bounded export. No whole-file losslessness
or arbitrary DXF conversion is claimed.

Compile `tests/dxf-layout-write.cpp` with assertions enabled and run
`scripts/test-dxf-layout-write.py ADAPTER FRESH_OUTPUT_DIRECTORY`. Local MinGW 8.1
checks pass four synthetic inputs through two saves: A3 TEST, custom page/UCS
metadata and UTF-8 name, sparse high handles, and retained handle 10020. Raw checks
precede ezdxf loading and cover uniqueness, typed table/block owners, dictionaries,
reactors, backlinks, viewport owner/ID, camera, visibility and line-type scale.
Nine successful outputs (including recovery from an interrupted legacy IMAGE
write) have zero independent audit errors and fixes.

Twenty-one Windows rejection/failure cases preserve the destination bytes,
including duplicate IDs, wrong links, units, non-finite/unsupported camera data,
handle headroom/exhaustion, header/write exceptions and a real locked destination.
Each failed operation can be followed by a valid explicit save and legacy reuse.
Replacement of an existing directory fails without touching its contents, and
owned temporary files are removed. Files are flushed/closed before final native
replacement; power-loss durability and unsupported source fidelity are not tested.

The fresh review found and the checks reproduced: inverted DXF visibility on the
second save, a dangling optional VPORT visual-style pointer, and stale owned image
definitions after an interrupted legacy write. All are corrected in their shared
paths. A first write check also exposed a MinGW Windows text-stream position error;
flushing before recording HANDSEED's byte offset fixes the CRLF translation offset.
Camera, layout-read and four standalone PLOTSETTINGS regressions pass after these
changes. Full Qt/MinGW application build and native GUI replay pass: eight isolated
profiles, unchanged registry, previous Properties/Undo/Redo workflows, independent
DXF/PDF/SVG outputs and four ribbon geometry captures. This writer's MSVC run is
pending; the reader CI does not cover this code.

The common visibility flag follows Autodesk's
[entity group codes](https://help.autodesk.com/cloudhelp/2024/ENU/AutoCAD-DXF/files/GUID-3610039E-27D1-4E23-B6D3-7E60B22BB5BD.htm);
the unimplemented optional style pointer is omitted as permitted by the
[VPORT record](https://help.autodesk.com/cloudhelp/2023/ENU/AutoCAD-DXF/files/GUID-8CE7CC87-27BD-4490-89DA-C21F516415A9.htm).

## Historical local P0 corrections — 2026-09-05 (before MSVC CI)

Baseline: clean `8a5f7ae0`, product-branch ancestry confirmed. Local developer
toolchain: Qt 5.15.2 / MinGW 8.1 x64 with Boost 1.87 headers, all outside the
tracked source in `.artifacts`. The authoritative MSVC artifact below is unchanged.

The new native GUI test failed before the fix (process exit 4): eight of nine
document summary snapshots were stale. After LINE it displayed 3 entities instead
of 4; after Undo it included the retained polyline; save left Modified stale.
After the fix, the rebuilt native GUI smoke exited 0 with PASS for all nine states
and existing LINE/PLINE/COPY/MOVE, Undo/Redo, selection, layer and MDI checks.
The independent summary verifier passed and rejected a retained-entity count even
when its producer `passed` field was true. Native/Qt isolated INI backend and both
sentinel read directions passed. No visual/DPI acceptance is inferred from offscreen.

Initial build invocation used unordered make targets and failed because the app
started before libmuparser existed. The corrected ordered target compiled
successfully. This was a local test setup failure, not an application regression.
See [P0_CORRECTIONS](P0_CORRECTIONS.md) for mechanism, commands and remaining gate.

P0-B first failed the independent clean-input roundtrip with exit 1 and repair
202 (orphan PLOTSETTINGS deleted). After the ownership fix, all four standalone
ASCII cases pass: one object, no objects, two objects, and the same writer reused
with two then one. Exact geometry/layers/units, four margins, native reopen and
dictionary/owner/reactor/name links pass with ezdxf 1.4.4: **0 errors, 0 repairs**.
The new input has explicit table handles and passes 0/0; the old fixture is retained
and the strict checker still rejects its two repair 110 findings.

Combined final-source local developer build: eight native process runs exit 0;
all nine Properties states and existing LINE/PLINE/COPY/MOVE, Undo/Redo,
selection/layer/MDI operations pass. `verify-preview-outputs.py` passes in full:
ten GUI DXFs audit 0/0, expected geometry survives, A4 PDF contains vectors and
no images, SVG contains vectors and no raster image elements. Four existing
ribbon geometry checks pass, including 0 px reference panel deltas. qwindows
captures use 100/125/150% Qt scale factors; the reference capture was inspected.
Eight isolated profile checks pass and the in-memory registry comparison is equal.

The optional binary probe failed before the ownership fix: native reopen exits 3,
and ezdxf reports an invalid header tag (code 2304). No existing test was disabled;
the new passing contract is explicitly ASCII (input AC1015, GUI outputs AC1021,
standalone outputs AC1027). This inherited binary issue is tracked in NEXT_TASKS.
Python syntax and strict audit negative checks pass. An optional PyYAML syntax
check could not run because PyYAML is absent; the workflow was reviewed statically.
No test-only dependency was added to the product to resolve that optional check.

Local logs and generated files: `.artifacts/p0-corrections/` (ignored).
Developer EXE SHA-256: `035142f0001211d4b3dd58420e4ec624dc04bacc18794c4267f4655e95bc1f60`.
This is MinGW development evidence, not an approved portable build. The updated
MSVC workflow includes the strict audits and standalone ownership/reuse adapter,
but has not run for these corrections. Owner acceptance and Windows Settings DPI
checks remain open. No push, remote CI, release or paperspace implementation occurred.

## Last Windows MSVC checkpoint summary — 2026-09-05 (before P0 corrections)

Latest tested SARibbon development source:
`d35ec35486912e4bca1fdc2a7125ecc6d53580eb`.
[Windows CI 33966232573](https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33966232573)
passed all gates, 12:30:33–12:44:15 UTC (13m42s). Portable ZIP
`KuubikDraw-0.2.0-preview.2-win64.zip`: **43,773,937 bytes**, SHA-256
`4c3a2d8c2e36918e3fa77b0c106a174d1ea01bf27b6ecb0ea8d3400fa1c382d0`.
The downloaded manifest/source/run and sidecar checksum were independently checked.
Later documentation/evidence commits do not change this executable source.

### Passed gates for this exact binary

- MSVC x64, Qt 5.15.2, qmake and pinned Qt jom 1.1.7; portable runtime/license/payload checks.
- Byte-exact MIT SARibbon v2.9.0, no QWindowKit/frameless dependency.
- 71 original/native icon mappings and 105 referenced SVGs; vendor checks and
  independent ribbon-verifier negative tests; handoff positive/negative tests.
- All six visible tab hit targets; native QAction identity, presentation,
  enabled/checked state preservation; focused GRID Space activation and restoration.
  The latter sets focus programmatically, not a complete physical Tab-order audit.
- All ten Home panels have **0 px boundary delta** at the 1920x1080 reference.
  Current layer and three native pen selectors are contained with their full
  ancestor chain. Four direct Draw buttons remain visible at narrow widths.
- Classic restores native toolbar ownership/orientation and all five Pen actions.
- Native LINE/PLINE Enter, COPY/MOVE and quick-access Undo/Redo, layer and selection
  Properties callbacks, status/precision contracts, open/save/native reopen.
- Independent ezdxf geometry/undo read-back, one A4 PDF page with vector operators
  and zero images, valid SVG vectors without raster image elements.
- Ten test processes verify an isolated shared Qt/native INI backend; native
  registry contents are unchanged. Public report exposes booleans/count only.
- Local full portable smoke from a path with spaces, independent DXF/PDF/SVG,
  reference/narrow geometry and all ten profile checks repeated successfully.

qwindows captures: reference 1920x1080 logical/physical; 100% 1280x600;
125% 1280x600 logical / 1600x750 physical; 150% 1200x600 logical / 1800x900 physical.
The reference test bounds Qt's maximum client height to avoid Windows' default
1920x1061 clamp; it retains native qwindows fonts/frame and does not crop/resize
an image to manufacture a pass. Normal application geometry is unchanged.
These are widget captures and Qt scale-factor tests, not a whole-desktop AutoCAD
pixel comparison or proof of Windows Settings DPI changes.

The local PDF was rendered with Poppler and visually inspected: the synthetic
rectangle, circle and diagonal line are present as thin vectors on one A4 page.
Native qwindows reference and narrow idle screenshots were also visually reviewed.
[Persistent CI evidence](../evidence/development/2026-09-05-saribbon/README.md)
contains 37 byte-exact CI files and their SHA-256 list. Local desktop captures
with profile paths or unrelated overlays are not published.

### Additional Windows pointer review and honest limits

A separate normal qwindows process used a fresh developer INI profile and a copy
of the synthetic fixture. First-run Millimeter/English choices, Home/Annotate/View,
native LINE via the visible button, two canvas points, Enter, Ctrl+S and Auto Zoom
were exercised through Windows input. Independent read-back found the expected
four entities: one polyline, one circle and two LINEs. The added LINE is
`(174.75,126) -> (224.75,88.5)`, length **62.5 mm**. The app remains available for
Reio's review. The full Classic roundtrip is automated proof, not a claimed
completed separate physical-pointer review.

Two additional findings are **not fixed in this UI checkpoint**:

1. A stricter `ezdxf.audit()` gives zero errors but one repair 202: an orphan
   PLOTSETTINGS object is discarded. This reproduces in both the manual saved DXF
   and the CI LINE output. `dxfRW::writePlotSettings` writes no owner 330;
   `writeObjects` creates no ACAD_PLOTSETTINGS dictionary. The source fixture itself
   has two older table-handle repairs (110), so it is not an audit-clean oracle.
   Existing geometry tests pass; **zero-repair/lossless whole-file DXF is not claimed**.
2. The read-only Properties document summary can lag entity/Modified changes
   after drawing and saving. Its tested native selection/layer callbacks work,
   but there is no complete document-change notification path yet. Do not equate
   the stale summary label with lost geometry or a failed save.

These are explicit next tasks before the broader paperspace/file-integrity gate.
No test was weakened to suppress either observation. True paperspace, arbitrary
DXF preservation, DWG/DWT/XREF, editable Properties and full AutoCAD parity remain open.

### Correction history

The pre-SARibbon baseline was source `9968198bbe72165e28c48f8e37109fa3eb103212`,
run 33919335101, ZIP SHA `60bf9445fdc6206e8f1b21a392d72dece93714a56feabc435bf7cb66cca27550`.
SARibbon run 33960243802 failed an unused minimum-mode null QAction after building;
33961402868 exposed client-height clipping, the reserved title row and Fusion
button styling. Runs 33962558802 and 33963838013 rejected unsupported CI display
modes. The runner supports at most 1920x1080, so inventing a larger desktop was
not a solution. Adapter/style/geometry fixes and exact client capture are recorded
in DEVELOPMENT_PLAN.

Source `18d3f734` passed all CI gates in run 33964068198, but local independent
read-back rejected duplicate PLINE vertices because native QSettings bypassed the
test INI format and read registry snap preferences. Source `d35ec354` corrected
all native/selection/CLI settings paths and added backend, registry and
nondegenerate-PLINE gates. This is why the preceding green CI ZIP was not delivered.

The first local d35 replay printed complete native PASS, then its ad-hoc outer
command incorrectly checked stale/unset LASTEXITCODE from a PowerShell script.
The suite's ten-process report and separately executed independent parsers all
passed; that wrapper error was not an application failure.

All sections below retain historical evidence for their stated source/date.

Checkpoint: `0.2.0-preview.2`

The published-release evidence below applies only to source commit
`171d95915f6f5a34b8d9fcb487dd3429de8cda74`. It does not by itself prove later
UI work and must not be reused as a release claim for the integration branch.

## Source and artifact

- executable source commit:
  `171d95915f6f5a34b8d9fcb487dd3429de8cda74`
- LibreCAD base:
  `7ebab007d9eb4c68609388b835a2487648f0877b`
- successful Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33510020896>
- portable ZIP: `KuubikDraw-0.2.0-preview.2-win64.zip`
- ZIP size: `43,438,760` bytes
- ZIP SHA-256:
  `6af290c178dbd9cd21ef3d9968c6972430cf031a2b4ad4262281b2715c280492`

## Passed CI gates

- pinned LibreCAD ancestry;
- MSVC x64 / Qt 5.15.2 release compilation;
- portable runtime packaging;
- required Qt/DLL/resource presence;
- forbidden build/cache payload absence;
- isolated Qt plugin lookup from the copied package;
- UI contract with at least 40 bound real actions;
- right Layers/Blocks and bottom command dock layout;
- ribbon LINE real mouse event and action activation;
- first canvas click leaves zero committed objects;
- pointer move creates the visible preview;
- second canvas click creates exactly one native LINE;
- DXF save and independent `ezdxf` read-back;
- vector PDF export and independent `pypdf` read-back;
- SVG export and XML read-back;
- copied path containing spaces;
- startup smoke and archive SHA-256.

## Independent LINE output

- entity count: `1`
- type: `LINE`
- start: `(122.0, 115.25, 0.0)`
- end: `(256.5, 50.0, 0.0)`
- length: `149.49184760380749`

Public files and their individual hashes are under
`evidence/releases/v0.2.0-preview.2`.

## Local Windows replay

The downloaded ZIP hash matched GitHub. It was extracted to a fresh folder and
the complete portable smoke passed locally, including offscreen UI contract,
LINE mouse workflow, DXF/PDF/SVG generation, independent parsers and normal
Windows GUI startup. A second Windows-platform run captured 1920×1080, 96-DPI
screenshots with fully rendered UI text.

## Corrected false positive

Run `33507858173` passed because the hosted runner found `qoffscreen.dll` in its
installed Qt tree. The downloaded package failed the same isolated local test.
Packaging was corrected to include the plugin and the test now sets package-only
plugin paths. Only corrected run `33510020896` is the release gate.

## Security and publication

- pinned Gitleaks 8.30.1: zero findings for the implementation wave;
- public evidence path/user/client keyword scan: zero findings;
- PNG metadata contains only DPI;
- evidence is synthetic and contains no client drawing or private AutoCAD image.

## Not run / not certified

- full manual owner matrix across every inherited LibreCAD command;
- DWG roundtrip, DWT and XREF parity;
- AutoCAD 2024 live paired workflow for this native fork;
- long-duration autosave/crash recovery;
- installer, code signing and production rollout;
- full 133-row AutoCAD audit against the native fork.

## Development integration checkpoint — 2026-09-02

This section is branch evidence, not a release and not a replacement for the
immutable public `v0.2.0-preview.2` artifact.

- branch: `codex/autocad-visual-integration-root`
- tested source: `d17e8b23bb702a7df8c4c106783b75fcd0ba9ea2`
- Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33665520217>
- tested portable ZIP SHA-256:
  `127b0558ab12a285a5abe639053b16418f5ba8f502789f4e46596bd0c0730365`
- GUI evidence artifact SHA-256:
  `c7c00f50d77907f2ccbffff3ee972e1857a993acf9c1c55cd8d8892e030d8e78`
- portable artifact wrapper SHA-256:
  `cf18225989c4f65abb36b817b397bc1f89eaf3098a87410cbafdd364370b1379`
- GUI evidence artifact ID: `9861131852`
- portable artifact ID: `9861134306`

The MSVC x64 / Qt 5.15.2 build, package isolation, payload allowlist, Gitleaks,
UI contract v2, native LINE canvas flow, native layer selector, Properties
callbacks and `ModifyEntity` delegation all passed. The test opened a synthetic
DXF, changed the active layer, committed a new LINE, saved it, closed/reopened
it through the native adapter, and independently read back the result. It then
created a native open PLINE with three canvas clicks, committed that command's
single undo cycle, clicked the visible quick-access Undo and Redo buttons, and
saved a DXF after each state.

The same smoke then cleared native selection, activated the visible responsive
Modify `More` route for the exact `ModifyDuplicate` QAction, and clicked the
earlier LINE at graph point `(184.375, 82.25)`. The automation fixture pins the
native action to in-place mode and reports that fact explicitly; this is not a
claim that the Tool Options checkbox itself was mouse-tested. One distinct
native LINE was created on `KUUBIK-SMOKE-LAYER`, then removed and restored by
the visible quick-access Undo and Redo buttons.

All three Draw invocations in the smoke—initial LINE, PLINE and the later
Properties LINE—used the visible `collapsedPanelOverflow` route. The test
physically clicked the visible `More` tool button, required the popup menu to be
visible, matched the exact native QAction at `QMenu::actionGeometry()`, clicked
that row with a mouse event and required the menu to close. No event was sent to
the hidden source button.

Independent `ezdxf` read-back verified:

- `pline-before-undo.dxf`: one open smoke PLINE with points
  `(66.25, 131.0)`, `(184.25, 140.75)`, `(282.75, 101.75)`;
- `pline-after-undo.dxf`: no smoke PLINE;
- `pline-after-redo.dxf`: the same open three-point smoke PLINE;
- the earlier smoke LINE remained exactly
  `(118.75, 114.75, 0.0)` → `(250.0, 49.75, 0.0)` in all three files;
- the original fixture LINE, circle and closed polyline also remained intact.
- `copy-before-undo.dxf` and `copy-after-redo.dxf` contain two identical smoke
  LINE entities with geometry `(118.75, 114.75, 0.0)` →
  `(250.0, 49.75, 0.0)`; `copy-after-undo.dxf` contains only the source smoke
  LINE;
- the smoke LINE counts are therefore `2 → 1 → 2` and total LINE counts,
  including the original fixture, are `3 → 2 → 3`;
- the earlier open smoke PLINE remains unchanged in all three COPY DXFs.

The independent verifier also found a vector A4 PDF and valid SVG vectors.

### Focused native Tool Options checkpoint

Source `f1c6733eb4c58c455132f00d845003acf93b1682` first passed the
1280×600 qwindows Tool Options proof in run
<https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33654660495>.
Its tested portable ZIP SHA-256 was
`7f623a64af40d5b46c9b2639e2c6f6884f04cfb598fec646a55f5c7da9cbb9af`;
the GUI evidence and portable wrapper hashes were respectively
`89bd3edb5301fea7b3ac649bbdfbff8a7f73715af1315133dd186f5a6ebd28d8`
and `f21de3e7c9f43e2a44cd2fe79433799bd877d20cfca3e226c63bd939e9a22acc`.

The report measured a 299-pixel LINE options host with exactly one native
`QG_LineOptions`. DIMLINEAR used a 621-pixel host containing one
420-pixel `QG_DimOptions` and one 200-pixel `QG_DimLinearOptions`, with no
clipping, stale widget or duplicate widget. Runs `33660926998` and
`33665520217` repeated the same focused test successfully; run `33665520217`'s
two Tool Options PNGs are byte-identical to the visually reviewed focused run.

### Corrected responsive-ribbon false interaction

Run `33657916794` on source
`35b1be45ff8921d3f1c4cabbaae1440974574c44` correctly failed because the first
PLINE smoke sent a synthetic event directly to a hidden Draw button. Qt emitted
the QAction signal, but that was not a user-clickable path. Source `aaff14484`
replaced that invalid interaction with the visible overflow button and popup
menu-row mouse route described above. Run `33660926998` proved the correction;
successful run `33665520217` is the current development gate and extends it
with the visible native COPY route.

The render smoke first verified the hosted Windows desktop changed from
1024×768 to 1920×1080, then recorded:

- 100%: 1280×600 logical and 1280×600 PNG;
- 125%: 1280×600 logical and 1600×750 PNG;
- 150%: 1200×600 logical and 1800×900 PNG.

All three qwindows screenshots were visually inspected with no clipping or
overlap. These 125%/150% cases use `QT_SCALE_FACTOR`; they are not evidence that
Windows Settings OS display scaling was changed. Real Windows 100%/125%/150%
scale validation and Reio's layout acceptance remain open.

The automated qwindows screenshots show Draw and Modify through responsive
`More` panels at the tested 1200/1280 logical widths. The 1920×1080 offscreen
workflow also used the Draw overflow route. That route is functional and now
mouse-tested, but keeping it as the preferred layout still requires Reio's
accept/deny decision.

The offscreen LINE active/committed PNGs are functional canvas evidence only;
the qwindows screenshots, where text is rendered, remain the visual-layout
gate. The goal also still requires real Windows Settings 100%/125%/150% checks
on controlled hardware and remaining native Modify/Annotation/Blocks workflow
evidence.

## Local direct-Draw layout check — 2026-09-03

The owner rejected the generic `More`-first Home ribbon shown by the preceding
Windows checkpoint. A local Arch Linux Qt 5 release build now keeps Home Draw
expanded at 1280x600 and renders direct `Line`, `Polyline`, `Circle` and `Arc`
buttons, plus `Rectangle` and `Hatch`. Secondary collapsed panels render a
representative Kuubik icon instead of `More`. The build completed successfully
and the offscreen UI contract/screenshot writer completed successfully.

This is local visual evidence only. The Windows MSVC/Qt 5.15 portable workflow,
qwindows DPI captures and native QAction-to-canvas smoke have not yet been run
for this uncommitted layout wave.

## Local LINE/PLINE Enter workflow check — 2026-09-03

The Arch Linux Qt 5 release build completed successfully after adding native
`Enter` completion to LINE and PLINE. The full offscreen GUI smoke exited 0 and
reported `PASS`. It physically invoked both ribbon actions, drew through canvas
clicks and sent `Qt::Key_Return` to the canvas.

The report recorded `accepted: true` and `finishedAction: true` for both LINE
and PLINE. LINE retained its committed segment. PLINE retained two segments as
one open entity, and its existing Undo/Redo roundtrip passed. The packaged
Windows qwindows run remains required before this local result is described as
Windows-verified.

The same GUI smoke now also captures each LINE/PLINE ribbon button before
activation and verifies that its visible text and Kuubik icon remain unchanged
after the native action starts. Both presentation-stability checks passed.

The extended smoke additionally found a visible cursor-adjacent dynamic input
containing both `L` and `A`, and verified that a canvas Escape event was
accepted and left no active action. The full smoke remained `PASS`, including
LINE/PLINE creation and PLINE atomic Undo/Redo.

After an owner screenshot showed numeric input landing in the bottom command
edit, the command-widget filter was corrected so active LINE/PLINE dynamic
input owns numeric keys, Backspace and Tab. The rebuilt full GUI smoke remained
`PASS`.

The bottom-status refinement hides the legacy snap toolbar and diagnostic
status widgets in Kuubik mode and restores them in Classic mode. The full local
GUI smoke remained `PASS` after the MODEL/GRID/ORTHO/OSNAP single-row change.

The OSNAP status/menu and type-specific overlay marker wave compiled in the
same Qt 5 release build. The full offscreen GUI smoke remained `PASS`; a native
Windows qwindows snap-interaction matrix is still required.

The expanded drafting-status/OSNAP wave on 2026-09-03 compiled with Qt 5 and
the full offscreen native GUI smoke reported `PASS`. Its report retained
`dynamicInputVisible: true`, `escapeCancelsAll: true`, successful LINE/PLINE
Enter completion, Undo/Redo, document lifecycle and DXF saves. The new snap
candidate matrix still requires dedicated geometry fixtures and the Windows
MSVC/qwindows workflow before release certification.

The status-symbol refinement on 2026-09-03 added original Kuubik SVG symbols
for the ten drafting controls and the extended OSNAP menu. The Qt 5 build and
full offscreen GUI smoke both passed after the resource and layout change;
`git diff --check` also passed. A live Wayland capture confirmed the compact
single-row rendering at a narrow half-screen window width.

## Local AutoCAD-familiar status-bar check — 2026-09-04

The status bar was rebuilt around the Autodesk-documented interaction pattern
using original Kuubik assets. The Qt 5 release build completed successfully.
The offscreen UI contract completed with exit code 0 and recorded:

- ten functional status-button bindings;
- twelve persistent customization entries with matching controls;
- a successful GRID hide/restore round trip through the customization action;
- fourteen OSNAP modes and a synchronized split-button state;
- four live coordinate display formats;
- no bottom-right resize grip;
- a rendered 222×374 customization-menu image and 1024×1024 application image.

The full local native GUI smoke also exited 0 with `PASS`. It retained LINE and
PLINE Enter behavior, visible dynamic input, global Escape cancellation, native
document lifecycle, and COPY and MOVE atomic Undo/Redo. `git diff --check`
passed. This is local Linux evidence only; the packaged Windows MSVC/qwindows
workflow and real Windows display-scale checks remain pending.

After live review found inherited LibreCAD symbols replacing Kuubik status
icons on click, the direct status bindings were separated from their visual
buttons. The rebuilt UI contract click-tested GRID, SNAP and ORTHO, restored
each native checked state, and recorded `customIconsStableAfterClick: true`.
All four direct-action controls recorded `customIconOwned: true`. The complete
native GUI smoke remained `PASS` for drawing, dynamic input, document lifecycle,
COPY and MOVE Undo/Redo.

The first-five-reference-pages pass on 2026-09-04 rebuilt the row with the
approved blue-grey palette and placed live coordinates, MODEL and GRID in one
ordered cluster. The coordinate readout is 184 logical pixels wide and displays
three Cartesian fields. A Kuubik/Classic/Kuubik workspace round trip restores
the original Classic coordinate slot and then reconstructs the Kuubik cluster.

The rebuilt UI contract exited 0 and recorded all first-phase checks as true:
coordinates visible by default, X/Y/Z output, coordinate/MODEL/GRID ordering,
single-row alignment, MODEL text, native Grid settings menu, synchronized Grid
state and matching Grid tooltip. The status bar measured 27 logical pixels.
The full offscreen native GUI smoke also exited 0 with `status: PASS`, including
LINE, PLINE, COPY, MOVE, Undo/Redo, Properties and document lifecycle. Autodesk
screenshots were not added to application resources.

## Reference pages 6–11 implementation check — 2026-09-04

The Qt 5 release build completed after adding the six precision-drafting
controls. The offscreen UI contract exited 0 and reported every page 6–11 gate
true: the controls were visible and ordered; SNAP exposed Grid/Polar choices,
right-click settings and F9; Dynamic Input exposed distance/angle settings and
F12; native ORTHO retained F8; POLAR exposed eight presets, F10 and a verified
15-degree snapping-engine quantization; and Isometric Drafting round-tripped
Left, Top and Right planes with F5/Ctrl+E registered.

The Infer control successfully enabled and restored the native endpoint,
perpendicular, tangent and parallel snap bundle. The contract also confirmed
that its presentation states the non-persistent limitation. The full native
GUI smoke exited 0 with `status: PASS`, retained visible dynamic input, global
Escape cancellation, exact LINE creation, PLINE/COPY/MOVE Undo/Redo and native
DXF save. Icon validation passed with 66 action mappings and 88 referenced
Kuubik SVGs, and `git diff --check` passed. This remains local Linux evidence;
Windows MSVC/qwindows packaging and real display-scale checks are pending.
