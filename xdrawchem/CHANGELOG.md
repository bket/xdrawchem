# Changelog

## [2.1.0] — 2026-05-18

### Summary
XDrawChem 2.1 is the first stable release of the Qt6-based series, building on
the 2.0 groundwork with restored 3D generation, MDL Molfile support, and
significant codebase modernization.

---

### New Features

- **3D structure generation restored** (`to3d.cpp`) using local OpenBabel
  (`OBBuilder` + MMFF94/UFF force field minimization). No network call required.
- **MDL Mol/SDF I/O implemented** via OpenBabel. `chemdata_mdl.cpp` now reads
  and writes standard MDL MOL and SDF files, replacing the previous no-op stubs.

### Bug Fixes

- **Memory leaks in `application_ob.cpp`** — `OBNewLoad()` and `OBNewSave()`
  now properly delete temporary `OBMol` objects.
- **Memory leak in `molecule_obmol.cpp`** — `convertToOBMol()` no longer leaks
  the returned `OBMol*` on error paths.

### Qt / C++ Modernisation

- **Remaining `SIGNAL()`/`SLOT()` string macros** converted to
  pointer-to-member syntax across `application.cpp`, `render2d_event.cpp`,
  and `main.cpp`. Zero string-based `connect()` calls remain in project code.
- **`qDebug()` runtime noise gated behind `QLoggingCategory`** — all
  `qDebug()` / `qInfo()` / `qWarning()` calls in the chemistry and I/O layers
  now use `lcXDC()` so they can be silenced with `QT_LOGGING_RULES=xdc=false`.

### Code Cleanup

- **`netaccess.cpp`** — removed large commented-out `QHttp`-based dead code
  and unreachable `slotFinished` / `slotData` / `rf` slots.
- **Repository cleanup** — legacy Qt3/Qt4 source trees removed from `master`;
  source directory renamed from `xdrawchem-qt5/` to `xdrawchem/`.

---

## [2.0.1] — 2026-04-23

### Summary
Security, UI, and packaging maintenance release.

### Security
- Fix buffer overflows in OpenBabel format resolution and atom-label `strcpy`
- Replace `system()` with `QProcess::startDetached` for external program launch

### UI / Bug Fixes
- Ring menu: amino acids, nucleic acids, sugars, and useful groups submenus
- Custom ring menu: dispatch restored, save crash fixed
- Property panel refreshes on ring placement, name-to-structure, selection changes

### Packaging
- Flatpak app-id renamed to `io.github.bryanherger.xdrawchem`
- DEB/RPM: include reverse-DNS desktop/metainfo/icon files
- Debian Standards-Version updated to 4.7.2

---

## [2.0.0] — 2026-04-09

### Summary
First stable Qt6 release.

### Highlights
- Full Qt6 + CMake build; requires Qt 6.2 or later
- OpenBabel 3 integration for 20+ chemical file formats
- PubChem REST integration: Find on PubChem, IUPAC name lookup, name-to-structure
- Dative (coordinate covalent) bond, ACS style preset, Bézier arrows
- SVG, PDF, and high-DPI PNG export; copy as SVG/PNG to clipboard
- Valence checking; canonical SMILES and InChI output
- Packages: DEB, RPM, Windows EXE, macOS DMG, Flatpak

---

## [Unreleased] — Qt5 Modernisation (Historical)

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
