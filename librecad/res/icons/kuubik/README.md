# Kuubik technical-line icon system

These 66 SVG icons were drawn from scratch for the Kuubik Draw Native
AutoCAD-familiar visual wave. They depict generic drafting concepts only; no
Autodesk logos, icon files, path data, reference images, or other proprietary
artwork were used.

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
