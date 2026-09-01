# Kuubik Draw Native

Kuubik Draw is an experimental native Windows 2D CAD application by Kuubik
Projekt OÜ. This repository is a GPLv2 fork of LibreCAD `v2.2.1.5`, pinned to
upstream commit `7ebab007d9eb4c68609388b835a2487648f0877b`.

The current target is the portable prerelease `0.2.0-preview.2`: extract the ZIP
and run `KuubikDraw.exe` without an installer, Node.js, Python, or internet.
It adds a compact, action-bound Kuubik ribbon, dark 2D workspace, right-side
Layers/Blocks, bottom command line, and classic LibreCAD fallback. DXF
open/save and vector PDF export are the guaranteed v0.2 file workflows;
DWG roundtrip is explicitly not certified.

- [Fork and license notice](FORK_NOTICE.md)
- [AI start and cross-computer handoff](AI_START.md)
- [Copy-paste prompt for another AI](PROMPT_FOR_NEXT_AI.md)
- [Current project state](docs/PROJECT_STATE.md)
- [Roadmap and next tasks](docs/ROADMAP.md)
- [Preview test guide](README_TEST.md)
- [0.2.0-preview.2 release report](docs/releases/v0.2.0-preview.2.md)
- [0.2.0-preview.2 public GUI evidence](evidence/releases/v0.2.0-preview.2/README.md)
- [LibreCAD upstream](https://github.com/LibreCAD/LibreCAD)
- [Kuubik Draw releases](https://github.com/T3stin-svg/kuubik-draw-native/releases)

This project is not affiliated with Autodesk. Kuubik Draw keeps LibreCAD's
full source history, author notices, GPLv2 license, and upstream attribution.

---

# LibreCAD upstream README [![Build Status](https://travis-ci.org/LibreCAD/LibreCAD.svg?branch=master)](https://travis-ci.org/LibreCAD/LibreCAD)

[→ Download ←](https://github.com/LibreCAD/LibreCAD/wiki/Download)

[LibreCAD](https://www.librecad.org) is a 2D CAD drawing tool
based on the community edition of [QCAD](https://www.qcad.org).
LibreCAD uses the cross-platform framework [Qt](https://www.qt.io/download-open-source/),
which means it works with most operating systems.  
The user interface is translated in over 30 languages.  https://translate.librecad.org

LibreCAD is free software; you can redistribute it and/or modify  
it under the terms of the [GNU General Public License version 2](https://www.gnu.org/licenses/gpl-2.0.html) (GPLv2)  
as published by the Free Software Foundation.  
Please read the [LICENSE](LICENSE) file for additional information.

The master branch represents the latest pre-release code,  
and now requires Qt 5.15.4 or newer.  
The 2.2 branch requires Qt 5.2.1 or newer
The 2.1 branch will be the last to support Qt4.  
The 2.0 branch will be the last to support the QCAD toolbar. [![Build Status](https://travis-ci.org/LibreCAD/LibreCAD.svg?branch=2.0)](https://travis-ci.org/LibreCAD/LibreCAD) 

## DXF Converter
LibreCAD can be used as dxf to a pdf, png or svg converter. For example, to convert a foo.dxf to foo.pdf, foo.png or foo.svg:
```bash
$ librecad dxf2pdf foo.dxf
$ librecad dxf2png foo.dxf
$ librecad dxf2svg foo.dxf
```
## Releases and Milestones

- [Releases and Prereleases](https://github.com/LibreCAD/LibreCAD/releases)
- [Milestones](https://github.com/LibreCAD/LibreCAD/milestones)

## libdxfrw
[libdxfrw](https://sourceforge.net/projects/libdxfrw/) is an associated project that allows LibreCAD to read DXF and DWG files.

#
**Requests and Bug reports**

- [GitHub issues (preferred)](https://github.com/LibreCAD/LibreCAD/issues)
- [SourceForge tickets (disabled)](https://sourceforge.net/p/librecad/_list/tickets?source=navbar)

**Users Documentation**

- [Users Manual](https://librecad.readthedocs.io/)
- [Wiki Main Page](https://dokuwiki.librecad.org/)

**Questions or Comments**

- [LibreCAD's Forum](https://forum.librecad.org/)
- IRC: [#librecad](https://web.libera.chat/#librecad) at libera.chat

**Building**

Requirements:

- [Qt](https://www.qt.io/download-open-source/) 5.2.1+ (MinGW version on Windows)
- [Boost](https://www.boost.org/)

More information: [Build from source](https://github.com/LibreCAD/LibreCAD/wiki/Build-from-source)

**Contributing**

[Git and GitHub](https://github.com/LibreCAD/LibreCAD/wiki/Git-and-GitHub)

[Becoming a developer](https://github.com/LibreCAD/LibreCAD/wiki/Becoming-a-developer)

There is a [resources repository](https://github.com/LibreCAD/Resources) for people that want to indirectly  
contribute to the project by supplying icons, stylesheets, documentation, templates...

Associated downloads: <https://sourceforge.net/projects/librecad/files/Resources/>
