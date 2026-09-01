# Kuubik Draw 0.1.0-preview.1 test guide

This is an experimental Windows x64 preview of Kuubik Draw, based on LibreCAD
2.2.1.5. It is a native, offline 2D CAD application. It does not require
Node.js, Python, internet access, installation, or administrator rights.

## Start

1. Extract the whole ZIP to a writable folder. Do not run the EXE from inside
   the ZIP.
2. Start `KuubikDraw.exe`.
3. On first start select `Millimeter`, `English`, and English command language.
4. Keep all DLL, `platforms`, `imageformats`, `resources`, and `translations`
   folders beside the executable.

Settings are isolated under `Kuubik Projekt OÜ / Kuubik Draw`; they do not use
LibreCAD's settings or the old React Kuubik Draw data.

## Primary preview checks

- Draw: LINE, PLINE, CIRCLE, ARC, RECTANGLE; verify pointer preview, Esc, and
  repeat command.
- Modify: ERASE, MOVE, COPY, ROTATE, OFFSET, TRIM, EXTEND, FILLET; verify one
  Undo/Redo after each group.
- Precision: ORTHO; endpoint, midpoint, center, and intersection snaps; exact
  coordinates and distances.
- Organize: create/current layer, color, visibility, lock, lineweight; text,
  linear dimension, hatch, and a basic block.
- Files: open DXF, edit, save as a new DXF, close and reopen it, then export a
  vector PDF and confirm page size and visible geometry.
- Portable: copy the extracted folder to another location, including a path
  containing spaces, and start it without internet.

## Scope and known limitations

- DXF open/save and vector PDF export are the promised v0.1 file workflows.
- DWG support inherited from upstream is experimental and is not a guaranteed
  roundtrip workflow.
- `.kdraw` files from the old web application are not imported; use DXF for
  exchange.
- LibreCAD 2.2.1.5 exposes entity Properties as a command/dialog, not as a
  permanent right-side Properties palette. Layers and Blocks are docked right;
  the command line is docked at the bottom.
- This preview uses LibreCAD's toolbar/menu interface. An AutoCAD-like ribbon
  is not part of v0.1.

Report a problem with the exact command, input values, smallest non-confidential
DXF that reproduces it, and a screenshot. Do not upload client drawings.
