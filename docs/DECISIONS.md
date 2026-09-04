# Kuubik Draw Native — decisions

These are current product decisions. A later AI may recommend changes, but must
not silently reverse them.

## D-001 — Native LibreCAD fork is the active product

The earlier React/Canvas Kuubik Draw direction exposed critical wiring gaps in
basic CAD interaction. Reio chose a native LibreCAD fork so a mature 2D engine,
selection model, file workflow and established commands are available from the
start. The old web repo remains separate and is not a source to merge wholesale.

## D-002 — GPLv2 and upstream history stay intact

The fork keeps the LibreCAD history, GPLv2 licence, authors and third-party
notices. Kuubik changes remain compatible with GPLv2. Upstream is pinned to
LibreCAD `v2.2.1.5` for the preview line.

## D-003 — Keep inherited functions available

The product does not use the former Lite disabled-command model. The compact
ribbon exposes common commands; all other inherited LibreCAD commands remain
available through menus or Classic workspace until Reio explicitly removes
them.

## D-004 — Reuse real QAction and native CAD behavior

The Kuubik ribbon is a view over LibreCAD's existing `QAction` objects. It must
not implement parallel command logic. The native document/entity/layer/undo
model remains the source of truth.

## D-005 — 2D-only product

Kuubik Draw Native is a 2D drafting application. A 3D view cube and 3D mode are
out of scope. AutoCAD familiarity applies to layout, density and interaction,
not copying Autodesk trademarks or proprietary assets.

## D-006 — File claims remain evidence-bounded

DXF open/save and vector PDF export are the guaranteed preview workflows. The
presence of upstream experimental DWG code does not certify DWG roundtrip,
DWT, XREF or AutoCAD-native fidelity. A future native compatibility investment
requires a separate legal/technical decision such as licensed ODA/RealDWG.

## D-007 — Owner-reproduced functional failures remain the absolute P0

A reproducible owner-found failure in drawing, Modify, layers, annotation,
blocks or file workflows interrupts visual work and becomes the first P0.
When no such failure is open, D-011 makes the AutoCAD-familiar visual and
workflow integration the active primary wave rather than deferred polish.

## D-008 — Evidence must exercise the real GUI path

Action-registry inspection alone was insufficient. The preview now sends a real
mouse event to the ribbon button, clicks the native canvas, inspects the native
document and reads back the saved DXF. New command certification should follow
that pattern.

## D-009 — Public evidence is synthetic and sanitized

Public GitHub content may include synthetic drawings, screenshots, hashes and
reports. It must not contain client drawings, private AutoCAD reference images,
credentials, user settings or local user-profile paths.

## D-010 — Conversation history is summarized, not copied verbatim

The full chat contains obsolete plans, repetition and potentially private
context. `docs/CHAT_PLAN_SUMMARY.md` is the canonical sanitized record of the
relevant decisions and plan evolution.

## D-011 — AutoCAD-familiar native workspace is the active primary wave

Reio explicitly prioritized an AutoCAD 2024 Drafting & Annotation-familiar
visual hierarchy, command placement and workflow for building and civil
engineering work. This includes the ribbon, current-layer and pen controls,
annotation, blocks, right-side Properties/Layers/Blocks, command line and
status controls.

Familiarity does not authorize copying Autodesk assets or making parity claims.
Every command and state remains backed by LibreCAD's existing QAction,
document, entity, layer, block, selection, undo and file behavior.

## D-012 — One primary window with internal Terra High agents

The active integration is coordinated in one primary Codex conversation and
one active codex integration branch. Parallel subtasks may use only internal
gpt-5.6-terra agents at High reasoning inside that conversation. Separate
sidebar tasks and parallel integration sessions are not part of this workflow.

The primary agent owns Git state, reviews all internal-agent evidence,
integrates changes through explicit allowlists and checks progress at least
once per hour.

## D-013 — Everyday Draw commands stay directly visible

Reio rejected the generic `More` tiles as the primary Home-ribbon presentation.
The Home Draw panel keeps `Line`, `Polyline`, `Circle` and `Arc` visible as
large icon-and-label commands, with `Rectangle` and `Hatch` alongside them, in
an AutoCAD 2024-familiar hierarchy. Secondary panels may still collapse when
space is constrained, but their collapsed tile uses a representative Kuubik
icon rather than the word `More`.

This is a layout decision only. Every button remains bound to its existing
LibreCAD QAction and no Autodesk artwork or proprietary behavior is copied.

## D-014 — Enter finishes native LINE and PLINE

For the everyday LINE and PLINE workflows, `Enter` on the drawing canvas or an
empty command-line submission finishes the active native action. Already
committed LINE segments remain in the document. A live PLINE is finalized as
one native undoable entity; only its uncommitted preview disappears.

This behavior is implemented inside the existing `RS_ActionDrawLine` and
`RS_ActionDrawPolyline` actions. It does not add a parallel command or document
path, and Escape/right-click behavior remains available.

## D-015 — Dynamic length/angle input and global Escape

LINE and PLINE show live length (`L`) and angle (`A`) next to the cursor after
the first point. Keyboard numbers edit the highlighted value, `Tab` switches
between length and angle, and `Enter` commits the exact endpoint through the
existing native coordinate-event path. Canvas `Esc` lets the current action
finalize any already committed geometry and then guarantees that no command
remains active.

## D-016 — OSNAP exposes only working native modes

The status bar uses one AutoCAD-familiar `OSNAP` popup. Its checked menu items
are the existing native Endpoint, Midpoint, Center, Intersection, Nearest,
Distance and Grid actions. Endpoint, midpoint, center, intersection and nearest
render distinct temporary overlay symbols; configured dashed snap guides remain
overlay-only and never enter the drawing.

Perpendicular, tangent, quadrant, extension and acquisition tracking are not
shown until equivalent native geometry and regression evidence exist.

## D-017 — Expanded native OSNAP state and compact drafting status bar

The Kuubik workspace now uses a single compact drafting row and hides all
legacy top-level toolbars outside the ribbon. Snap flags are appended to the
persisted bit mask, preserving every older flag value. Quadrant, node,
insertion, perpendicular, tangent, geometric center, apparent intersection,
extension and parallel candidates are calculated by the existing native
entity geometry; no display-only snap entries are used. OSNAP is a split
button whose main area disables/restores the selected object snap set.

This decision supersedes D-016's deliberately limited list. Autodesk artwork
is not copied.

## D-018 — AutoCAD-familiar status-bar customization uses working controls only

The Kuubik workspace follows AutoCAD's compact status-bar interaction pattern:
a grey single row, blue checked states, icon-first drafting controls, an OSNAP
split menu, settings on applicable controls and a three-line customization menu
at the far right. The customization menu immediately shows or hides each
control and persists the choice.

The supported row contains optional live coordinates, MODEL, GRID, SNAP,
ORTHO, POLAR, OSNAP, OTRACK, dynamic input, lineweight display, Quick
Properties and clean screen. Coordinates offer absolute/relative Cartesian and
polar formats. Lineweight is the inverse view of LibreCAD's native Draft mode;
Quick Properties controls the existing native Properties dock. Unsupported
AutoCAD-only status features are omitted instead of appearing as display-only
buttons. All symbols are original Kuubik assets; no Autodesk artwork or path
data is copied.

## D-019 — Status actions own behavior; Kuubik buttons own presentation

Status buttons do not use `QToolButton::setDefaultAction()` in the Kuubik
workspace. They trigger and mirror the native QAction explicitly while keeping
the button text, tooltip, checked state and original Kuubik icon under Kuubik
control. This prevents QAction state notifications from copying inherited
LibreCAD toolbar artwork onto a clicked status button. The UI contract changes
and restores GRID, SNAP and ORTHO, and pixel-compares their icons to the owned
Kuubik resources after each click.

## D-020 — The first five reference pages define the initial status-bar phase

The first implementation phase is deliberately limited to pages 1–5 of
`AutoCAD_Status_Bar_Visual_Reference.pdf`: overall visual language, interaction
states, Coordinates, Model/Paper Space and Grid. The Kuubik row therefore uses
the reference's compact blue-grey surface and blue enabled state, with live
`X, Y, Z` coordinates immediately before MODEL and GRID.

Coordinates, MODEL and GRID are enabled once for existing profiles when this
phase is first installed; later customization remains authoritative. MODEL is
an honest model-space indicator because Kuubik Draw has no AutoCAD paper-space
implementation. GRID invokes and mirrors LibreCAD's native `ViewGrid` QAction,
retains F7 behavior and opens the native grid settings on right-click. Classic
workspace moves the coordinate widget back to LibreCAD's original status slot.

Autodesk screenshots remain reference material in the PDF only. No Autodesk
image, icon or path data is embedded in the application.

## D-021 — Pages 6–11 use real native precision behavior

The second status-bar phase implements pages 6–11 of the approved reference:
Snap Mode, Infer Constraints, Dynamic Input, Orthomode, Polar Tracking and
Isometric Drafting. SNAP selects mutually exclusive Grid or Polar snap and
retains native settings access. F9, F12, F8, F10, F5 and Ctrl+E follow the
reference shortcuts. POLAR offers eight standard increments plus a custom
value, and the selected increment quantizes otherwise-free points in
`RS_Snapper` without overriding higher-priority object or orthogonal snaps.

Isometric Drafting uses LibreCAD's real isometric grid and Left/Top/Right
crosshair support. Dynamic Input's distance and angle fields can be configured,
but at least one remains visible. LibreCAD has no AutoCAD-equivalent persistent
parametric constraint solver, so Infer Constraints is explicitly a reversible
native endpoint/coincident, perpendicular, tangent and parallel snap bundle.
Its tooltip and UI contract identify that limitation instead of presenting a
display-only or misleading constraint toggle.

The new status icons are original Kuubik technical-line SVGs. Autodesk
screenshots remain confined to the reference PDF and are not packaged into the
application.
