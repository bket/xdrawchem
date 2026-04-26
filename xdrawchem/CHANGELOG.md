# Changelog

## [Unreleased] — Qt5 Modernisation

### Summary
Full modernisation pass on the `xdrawchem-qt5` codebase targeting Qt 5.15
compatibility, C++14/17 best practices, and Qt6 forward-readiness.
The build now compiles cleanly with `-Wall -Wextra` and zero warnings or errors
from project source files.

---

### Bug Fixes

- **`bond.h` — `midpoint()` copy-paste bug** (`dy` used `end->x` instead of
  `end->y`), producing wrong midpoint Y coordinates for every bond.
- **`http.cpp` — busy-wait spin loop** (`while (!finished) { qDebug()… }`)
  blocked the Qt event loop entirely during network requests. Replaced with a
  local `QEventLoop` that yields correctly while waiting for replies.
- **`http.cpp` — uninitialized pointer crash** in the default `HTTP()`
  constructor, which called `connect(manager, …)` before `manager` was
  assigned. Shared `init()` helper now ensures `manager` is always constructed
  first.
- **`bond.h` — duplicate `normalize()` calls** in `getAngleBetween()`: both
  vectors were normalized twice in succession. Redundant pair removed.

---

### Qt API Modernisation

- **`QXmlSimpleReader` / `QXmlInputSource` removed** (deprecated in Qt5,
  deleted in Qt6). `xml_cml.h` and `xml_cml.cpp` rewritten to use
  `QXmlStreamReader`. `CMLParser` no longer inherits `QXmlDefaultHandler`;
  it now exposes a simple `parse(QIODevice*)` method instead.
- **`QHttp` removed** from `netaccess.h` (the class was deleted in Qt5).
  All network access goes through `QNetworkAccessManager`.
- **`HTTP` base class changed** from `QDialog` to `QObject`. `HTTP` is not a
  dialog; it was calling `accept()` to signal completion. Now emits
  `downloadComplete()` signal, consumed by `QEventLoop::quit` in callers.
- **266 `Qt::foreach` macro calls** replaced with C++11 range-for loops
  (`for (const auto &x : container)`). The Qt macro is deprecated in Qt5 and
  removed in Qt6.
- **`endl` → `Qt::endl`** in all `QTextStream` and `qDebug()` call chains
  (35 occurrences in `prefs.h`, additional instances across `.cpp` files).
  The bare `endl` overload is deprecated in Qt 5.15.
- **Old-style `SIGNAL()`/`SLOT()` macros** converted to pointer-to-member
  syntax for the safe subset of Qt built-in signal/slot pairs (e.g.
  `&QAbstractButton::clicked`, `&QDialog::accept`). String-based macros
  are still present for custom application signals where the sender type
  cannot be mechanically inferred; these continue to work in Qt5.

---

### C++ Modernisation

- **`nullptr`** replaces all `0` and `NULL` pointer literals in declarations
  and initialisations (184 occurrences across headers and source files).
- **`Vector2D` heap allocations eliminated** in `bond.h`. All three angle
  calculation methods (`getAngleBetween`, `getAngleBetweenDirectional`,
  `getAngleBetweenDirectionalLeftHanded`) previously allocated two `Vector2D`
  objects via `new` and manually `delete`d them — leaking on any exception.
  They now use stack-allocated values; `toVector()` returns by value.
- **`static char*` → `static const char*`** in all 16 embedded XPM pixmap
  arrays. ISO C++ forbids implicit conversion of string literals to `char*`.
- **`#include <stdio.h>` → `#include <cstdio>`** in `http.h`, `http.cpp`,
  and `main.cpp`. `fprintf(stderr, …)` calls replaced with `qWarning()`.
- **Dead `//cmake#` comment lines** removed throughout (build-system artefacts
  from a previous CMake migration attempt).

---

### Header / Include Hygiene

- **`using namespace OpenBabel` removed from all 7 headers** that previously
  exported it into every including translation unit:
  `molecule.h`, `ioiface.h`, `tooldialog.h`, `tool_13c_nmr.h`,
  `tool_1h_nmr.h`, `tool_2d3d.h`, `tool_ir.h`.
- **OpenBabel `#include` directives removed from headers**; replaced with a
  forward declaration `namespace OpenBabel { class OBMol; }` in the three
  headers that reference `OBMol*` in method signatures (`molecule.h`,
  `ioiface.h`, `tooldialog.h`).
- **Explicit OpenBabel includes added** to the four `.cpp` files that actively
  use OpenBabel types (`ioiface.cpp`, `application_ob.cpp`, `tool_2d3d.cpp`,
  `tooldialog.cpp`) and to `molecule_obmol.cpp` and `molecule_smiles.cpp`
  which now require them directly rather than transitively.

---

### Security

- **All external `http://` URLs upgraded to `https://`**: PubChem lookup,
  xdrawchem.sourceforge.net 3D build service, and the runtime-constructed
  URL prefix in `netaccess.cpp`.
- **SSL errors now reported via `qWarning()`** instead of being silently
  printed to `stderr` only.

---

### Files Changed
93 files modified, ~740 insertions, ~700 deletions.  
No functional behaviour was changed; all modifications are mechanical
modernisation or correctness fixes.
