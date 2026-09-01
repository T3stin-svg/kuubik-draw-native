# Kuubik Draw Native — project state

Status date: **2026-09-01**

## Product

Kuubik Draw Native is a portable, offline Windows x64 2D CAD application based
on LibreCAD `v2.2.1.5`. The current product branch is
`kuubik/visual-v0.2`. It uses the full LibreCAD 2D engine and adds a Kuubik Dark
workspace, compact AutoCAD-familiar ribbon and Kuubik product identity.

The application does not need Node.js, Python, internet, installation or
administrator rights at runtime.

## Repository map

- Canonical public repo: `T3stin-svg/kuubik-draw-native`
- Product/default branch: `kuubik/visual-v0.2`
- Upstream preservation branch: `master`
- LibreCAD remote: `https://github.com/LibreCAD/LibreCAD.git`
- Old web experiment: `T3stin-svg/kuubik-draw` — separate and not the native
  product

## Current release

- Version: `0.2.0-preview.2`
- Tag: `v0.2.0-preview.2`
- Executable source commit:
  `171d95915f6f5a34b8d9fcb487dd3429de8cda74`
- ZIP SHA-256:
  `6af290c178dbd9cd21ef3d9968c6972430cf031a2b4ad4262281b2715c280492`
- Release URL:
  <https://github.com/T3stin-svg/kuubik-draw-native/releases/tag/v0.2.0-preview.2>

The product branch also contains later documentation and public handoff files.
Do not move the release tag: its exact executable source is intentionally
immutable.

## Implemented Kuubik layer

- executable and product identity `KuubikDraw.exe` / `Kuubik Draw`;
- isolated Qt settings namespace `Kuubik Projekt OÜ / Kuubik Draw`;
- Fusion/QSS `Kuubik Dark` theme;
- compact Home, Annotate, Insert, View and Output ribbon;
- ribbon buttons bound to real LibreCAD `QAction` objects;
- dark 2D model space with blue active/selection accents;
- right-side Layers and Blocks palettes;
- full-width bottom command line;
- compact coordinate/snap/status controls;
- Kuubik and Classic workspace modes;
- palette left/right and workspace reset commands;
- portable MSVC/Qt Windows packaging and prerelease automation;
- native GUI proof for ribbon LINE and two canvas clicks;
- public CI and local Windows screenshot/JSON/DXF evidence.

## Verified behaviors

- portable startup from a path containing spaces;
- LINE action activation from the Kuubik ribbon;
- LINE preview after the first click;
- exactly one committed native LINE after the second click;
- DXF open/save and independent `ezdxf` read-back;
- vector PDF export and independent `pypdf` read-back;
- SVG export and XML read-back;
- package-only Qt plugin loading;
- runtime DLL/plugin/resource allowlist;
- 1920×1080 local Windows visual read-back.

Evidence is under `evidence/releases/v0.2.0-preview.2` and summarized in
`docs/TEST_REPORT.md`.

## Honest limits

- This is a prerelease, not production.
- No native 133-row AutoCAD parity percentage has been certified for this fork.
- DWG roundtrip, DWT and XREF parity are not certified.
- Properties opens the real LibreCAD dialog; there is no permanent Properties
  palette yet.
- The ribbon covers common tools; inherited commands remain in menus/Classic.
- The ribbon currently uses inherited GPL LibreCAD icons.
- Full owner testing of Modify, layers, annotation, blocks and complex file
  workflows is still required on Reio's real workflow.
- No `.kdraw` import from the web experiment exists; use DXF exchange.

## Build authority

The official preview build is GitHub Actions workflow
`.github/workflows/kuubik-preview-win64.yml` on Windows Server 2022 with MSVC
x64 and Qt 5.15. Local machines without that toolchain should not claim a new
binary build. They can edit, inspect, run static checks and consume a verified
release artifact.
