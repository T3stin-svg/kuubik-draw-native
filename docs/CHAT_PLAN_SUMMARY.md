# Sanitized conversation and plan summary

This file gives another AI the relevant context from the long Kuubik Draw
working conversation without publishing the verbatim chat or private project
material.

## 1. Original goal

Reio initially wanted a browser-based Kuubik Draw to become extremely similar
to AutoCAD 2024 for 2D work. A 133-row audit and a long functional/visual parity
roadmap were created. The web application accumulated many tests and partial
features, but the most basic user path — clicking LINE and then the canvas — was
not adequately proven in the packaged app.

## 2. Open-source investigation

LibreCAD and FreeCAD were analyzed as possible accelerators:

- LibreCAD was judged useful for mature 2D commands, selection, snapping and
  DXF behavior;
- FreeCAD was judged more useful as a developer-side geometry oracle than as a
  2D application base;
- neither open-source project was accepted as proof of AutoCAD-native DWG
  parity;
- copying small algorithms into the closed web engine was considered, but the
  licensing and integration cost did not solve the immediate usability problem.

## 3. Scope reduction attempts

Reio selected a 94-row personal scope, followed by a 20-row Lite vertical
workflow. The web Lite EXE was packaged and then failed the owner's elementary
LINE click test. The immediate bug was fixed, but Reio rejected further effort
on the reduced web engine because it still omitted the mature behavior already
available in LibreCAD.

The old 133-row and Lite plans remain historical context only. They do not
govern the current native fork unless Reio explicitly reactivates them.

## 4. Active direction: native LibreCAD fork

Reio chose **Kuubik Draw Native as a LibreCAD fork**:

- public repository `T3stin-svg/kuubik-draw-native`;
- GPLv2 with full upstream history;
- stable base LibreCAD `v2.2.1.5`;
- native portable Windows EXE;
- retain all inherited 2D functions;
- first guarantee DXF open/save and vector PDF export;
- treat DWG as experimental/not certified;
- do not mix the old React CAD engine into the fork.

## 5. Native preview sequence

### 0.1 preview

Established the branded portable Windows build and upstream LibreCAD function
base.

### 0.2.0-preview.1

Added Kuubik Dark, a compact QAction-bound ribbon, right Layers/Blocks, bottom
command line, compact status controls and Classic fallback. Automated static UI
contracts passed, but a physical ribbon LINE click was still not proven.

### 0.2.0-preview.2

Closed the missing proof with a real Qt mouse event to `DrawLine`, two native
canvas clicks, native entity counts, screenshots, DXF save and independent
`ezdxf` read-back. A CI plugin-path false positive was discovered locally and
fixed by shipping `qoffscreen.dll` and isolating plugin lookup to the portable
package. The corrected package passed CI and local Windows replay.

## 6. Current governing product goal

Build a reliable, visually clean, AutoCAD-familiar **native 2D Kuubik Draw** by
starting from LibreCAD's working functions and improving only what Reio's real
workflow requires.

Priority order:

1. owner-reported broken everyday workflows;
2. Draw/Modify/layer/DXF/PDF end-to-end reliability;
3. annotation, blocks and recovery needed by Reio;
4. visual polish, custom icons and a better Properties experience;
5. licensed native DWG work only if Reio confirms the business need.

No percentage may be claimed merely because LibreCAD contains a command. The
native fork needs its own scope and evidence if percentage scoring is restarted.

## 7. Collaboration preferences

- Show Reio a working executable early.
- Do not spend weeks building features he does not use.
- Use at most two independent implementation sessions plus an integrator when
  parallel work is explicitly requested.
- Run targeted tests during a wave and the full Windows/package/read-back gate
  at a release checkpoint.
- Preserve the dirty worktree and never overwrite user work.
- Be explicit about what is verified, partial, experimental or not run.
