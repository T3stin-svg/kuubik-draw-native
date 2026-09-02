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
