# Kuubik Draw 0.2.0-preview.2 test guide

This is an experimental Windows x64 preview of Kuubik Draw, based on LibreCAD
2.2.1.5. It is a native, offline 2D CAD application. It does not require
Node.js, Python, internet access, installation, or administrator rights.

The public `v0.2.0-preview.2` release remains fixed to its published source
commit. A later CI artifact carrying this guide is a development checkpoint,
not a replacement release; use its `build-manifest.json` for the exact source
commit and workflow run.

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
  Home, Insert, Annotate, View, Manage, and Output tabs. Its controls reuse
  LibreCAD's existing native actions.
- The Home ribbon has a real current-layer selector. Changing it changes the
  native document's current layer; the selector is not a display-only label.
- Properties, Layers, and Blocks are tabbed on the right. Properties is a
  read-only native selection/document summary; `Open Full Properties` delegates
  editing to LibreCAD's existing `ModifyEntity` action.
- Use `View -> Workspace -> Palette Left/Right` to move the right work area,
  `Reset Kuubik Workspace` to restore the default, or `Classic workspace` to
  show the familiar LibreCAD toolbars and menus.
- The command line is full-width at the bottom. The compact status bar starts
  with live `X, Y, Z` coordinates, MODEL and GRID, followed by SNAP, ORTHO,
  POLAR, OSNAP, OTRACK, dynamic input, lineweight, Quick Properties and clean
  screen. Blue means enabled. OSNAP's arrow opens the native snap-mode list;
  right-click GRID or SNAP for settings. Clicked controls retain their Kuubik
  symbol while the native action state changes.
- Click the three horizontal lines at the far right to show or hide status
  controls. Choices persist, and `Reset Status Bar` restores the default row.
  Coordinates are visible by default; right-click the readout to select
  absolute/relative Cartesian or polar display.

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
exactly one new native LINE on the selected layer, saves and natively reopens
the DXF, and reads it through an independent parser. Separate qwindows + Qt
scale-factor screenshots cover 100%, 125%, and 150% rendering, including a
1280-logical-pixel narrow case. They do not replace a real Windows Settings
display-scale check on owner-controlled hardware.

## Scope and known limitations

- DXF open/save and vector PDF export are the promised v0.2 file workflows.
- DWG support inherited from upstream is experimental and is not a guaranteed
  roundtrip workflow.
- `.kdraw` files from the old web application are not imported; use DXF for
  exchange.
- The right Properties palette is intentionally read-only. It shows native
  document and selection state; entity editing remains in LibreCAD's existing
  Properties/`ModifyEntity` workflow rather than a second editor.
- The compact ribbon covers the everyday tools only. Every other inherited
  LibreCAD function remains available from the menus or Classic workspace.
- 66 direct ribbon/Quick Access action keys use the repository's mapped Kuubik
  technical-line SVGs. Five direct Annotate/Lines keys and all menus, Classic,
  and native status/pen/options toolbars retain inherited LibreCAD fallback
  icons under their existing repository licences. Kuubik icon provenance and
  licence are recorded under `librecad/res/icons/kuubik`; Reio's visual
  acceptance remains open.

Report a problem with the exact command, input values, smallest non-confidential
DXF that reproduces it, and a screenshot. Do not upload client drawings.
