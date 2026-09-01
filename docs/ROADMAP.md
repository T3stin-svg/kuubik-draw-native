# Kuubik Draw Native roadmap

The roadmap is workflow-first. It does not attempt to reproduce all of AutoCAD
before Reio can use the program.

## Now — owner acceptance and P0 closure

- Reio tests the latest portable preview on everyday 2D actions.
- Every reproducible owner-found bug becomes the first P0.
- Keep the smallest non-confidential DXF and exact command/input sequence as a
  regression fixture.
- Do not add unrelated tools while a basic Draw/Modify/file workflow is broken.

## Next — primary 2D workflow proof

Create native GUI and file read-back coverage for:

- Draw: LINE, PLINE, CIRCLE, ARC and RECTANGLE;
- Modify: ERASE, MOVE, COPY, ROTATE, OFFSET, TRIM, EXTEND and FILLET;
- precision: ORTHO, endpoint, midpoint, center and intersection snap, exact
  coordinates and distances;
- layers: create/current, color, visibility, lock and lineweight;
- annotation: text, linear dimension and hatch;
- blocks: create, insert and explode;
- files: open DXF, edit, save a new DXF, close/reopen, vector PDF export;
- undo/redo across each command group.

The outcome is a short owner checklist and automated native smoke suite, not a
new percentage claim.

## Then — visual/product refinement

- replace inherited ribbon icons with original Kuubik SVG icons and provenance;
- refine ribbon density, labels, active/disabled states and keyboard access;
- add a real dockable Properties experience only if it edits native entities;
- improve command history/status clarity;
- preserve Classic workspace and all inherited menu commands;
- verify 1920×1080 at 96 DPI plus common high-DPI scaling.

## Reliability wave

- autosave and recovery behavior;
- crash/reopen tests with a synthetic drawing;
- missing font/linetype handling;
- larger DXF performance and memory checks;
- signed installer only after the portable product is accepted.

## Optional native compatibility investment

Only after Reio confirms the requirement:

- evaluate licensed ODA Drawings SDK or RealDWG;
- define DWG/DWT/XREF acceptance files and legal boundaries;
- require AutoCAD read-back for native claims;
- never certify the upstream experimental converter as full roundtrip parity.

## Release gates

Every public preview requires:

- exact source commit and immutable tag;
- Windows MSVC/Qt build;
- portable dependency/plugin isolation;
- targeted native GUI proof for changed workflows;
- independent file read-back where applicable;
- ZIP payload inspection and SHA-256;
- sanitized public evidence;
- explicit known limitations;
- no production claim without Reio's approval.
