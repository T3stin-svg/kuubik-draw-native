# Kuubik technical-line icon system

These 66 SVG icons were drawn from scratch for the Kuubik Draw Native
AutoCAD-familiar visual wave. They depict generic drafting concepts only; no
Autodesk logos, icon files, path data, reference images, or other proprietary
artwork were used.

## Provenance and licence

- Introduced in Kuubik Draw Native commit
  `174069898a21ed41ca67c30f822cbd359f60358c` on 2026-09-01.
- Git author recorded on that commit: Olav.
- The SVG files in this directory are distributed under `GPL-2.0-only`; see the
  repository root `LICENSE` file. This explicit icon licence does not change
  the existing licences or notices of inherited LibreCAD assets elsewhere.

## Visual contract

- `viewBox="0 0 24 24"` on every asset;
- primary stroke `#DDE6ED`, accent stroke/fill `#2FA8FF`;
- 1.6 px linework, `stroke-linecap="round"` and
  `stroke-linejoin="round"` throughout;
- simple open geometry, without icon text, raster data, external links, fonts,
  scripts, or metadata.

The round technical stroke keeps corners readable at 16 px while blue marks the
active geometric reference rather than decorating the whole glyph. `icons.qrc`
registers all assets below `:/icons/kuubik/`; `KuubikIconRegistry` maps action
keys without mutating their behaviour. Run
`python scripts/check-kuubik-icons.py` from the repository root to validate the
contract and mapping coverage.

The compact drafting-status symbols added after the original 66-action set,
including the generic three-line customization symbol, follow the same
from-scratch GPL-2.0-only provenance. They are status UI assets rather than
additional `KuubikIconRegistry` action mappings. The geometric-inference and
isometric-grid symbols added for reference pages 7 and 11 are also original
Kuubik linework and do not trace Autodesk's published status icons.
