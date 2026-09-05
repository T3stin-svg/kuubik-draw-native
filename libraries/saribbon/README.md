# SARibbon vendored source

- Upstream: <https://github.com/czyt1988/SARibbon>
- Version: v2.9.0
- Commit: `806e3e93be4dd7676697d3017282a4359519e053`
- License: MIT, copyright (c) 2020 czyt1988; see LICENSE.
- `SARibbon.h` and `SARibbon.cpp` are unmodified upstream amalgamated artifacts,
  including the resource data. Do not hand-edit them; update from a reviewed pin.
- SHA-256 (imported bytes):
  - SARibbon.cpp: `9c844e16af1d9a7ddbe86d18065923457424ea40c2562abe44fabbd8d4068d14`
  - SARibbon.h: `1e7593ce379a13d4801140fcdf891550aa7e9736d71cd34028457aef71241244`
- `saribbon.pri` is the Kuubik integration: Qt5.15/C++17, QtSvg, no frameless
  helper or QWindowKit, no separate DLL. Upstream MIT notice is also shipped as
  `licenses/SARibbon-MIT.txt` in the portable package.

Native command/document behavior remains outside this library. Kuubik owns
presentation-only QWidgetActions for responsive panel visibility; the buttons
continue to use the existing native QAction as their defaultAction.
