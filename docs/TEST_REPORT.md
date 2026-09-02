# Kuubik Draw Native — verified test checkpoint

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
- tested source: `d7e3b58c4d05e7b0f5d485c7244451c5ee635a8e`
- Windows run:
  <https://github.com/T3stin-svg/kuubik-draw-native/actions/runs/33641126389>
- tested portable ZIP SHA-256:
  `901877c3a403fc181b6088f7aac0e92320c796c1a27eccdc7dfca8d72c4cb2b0`
- GUI evidence artifact SHA-256:
  `1020b07feac5b3b361000d56a857ccbcb5de158e16f26ea0ec847edf97ce1602`
- portable artifact wrapper SHA-256:
  `a5a2271eda798add8cd321d2d6405e0a0bd86c4844e678b07d8c54d243d28020`

The MSVC x64 / Qt 5.15.2 build, package isolation, payload allowlist, Gitleaks,
UI contract v2, native LINE canvas flow, native layer selector, Properties
callbacks and `ModifyEntity` delegation all passed. The test opened a synthetic
DXF, changed the active layer, committed a new LINE, saved it, closed/reopened
it through the native adapter, and independently read back the result. The
independent verifier also found a vector A4 PDF and valid SVG vectors.

The render smoke first verified the hosted Windows desktop changed from
1024×768 to 1920×1080, then recorded:

- 100%: 1280×600 logical and 1280×600 PNG;
- 125%: 1280×600 logical and 1600×750 PNG;
- 150%: 1200×600 logical and 1800×900 PNG.

All three qwindows screenshots were visually inspected with no clipping or
overlap. These 125%/150% cases use `QT_SCALE_FACTOR`; they are not evidence that
Windows Settings OS display scaling was changed. Real Windows 100%/125%/150%
scale validation and Reio's layout acceptance remain open.

The offscreen LINE active/committed PNGs are functional canvas evidence only;
the qwindows screenshots, where text is rendered, are the visual-layout gate.
