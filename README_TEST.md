# Kuubik Draw 0.2.0-preview.2 test guide

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

## Kuubik workspace

- The default workspace uses the built-in `Kuubik Dark` theme and compact
  ribbon. Ribbon buttons call LibreCAD's real actions; the original commands
  remain available through the menus.
- Quick Access contains New, Open, Save, Undo, Redo, and Print. The ribbon has
  Home, Annotate, Insert, View, and Output tabs.
- Layers and Blocks are tabbed on the right. Use `View -> Workspace -> Palette
  Left/Right` to move them, `Reset Kuubik Workspace` to restore the default, or
  `Classic workspace` to show the familiar LibreCAD toolbars.
- The command line is full-width at the bottom. GRID, ORTHO, END, MID, CEN, and
  INT status controls use the same checked/enabled state as their real actions.

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

The Windows build also runs a native GUI smoke at 1920×1080. It sends a real
mouse event to the ribbon `LINE` button, clicks two canvas points, requires
exactly one new native LINE, saves the DXF and reads it through an independent
parser, and publishes active/committed PNG evidence with the CI run.

## Scope and known limitations

- DXF open/save and vector PDF export are the promised v0.2 file workflows.
- DWG support inherited from upstream is experimental and is not a guaranteed
  roundtrip workflow.
- `.kdraw` files from the old web application are not imported; use DXF for
  exchange.
- LibreCAD 2.2.1.5 exposes entity Properties as a real command/dialog, not as a
  permanent right-side Properties palette.
- The compact ribbon covers the everyday tools only. Every other inherited
  LibreCAD function remains available from the menus or Classic workspace.
- The v0.2 ribbon uses the existing GPL LibreCAD icons. A complete custom icon
  redraw and a live Properties palette are intentionally deferred.

Report a problem with the exact command, input values, smallest non-confidential
DXF that reproduces it, and a screenshot. Do not upload client drawings.
