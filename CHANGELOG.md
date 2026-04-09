# XDrawChem Changelog

All notable changes to XDrawChem are documented here.
Entries are derived from the Debian changelog and commit history.

---

## 2.0 — 2026-04-09

### Infrastructure / CI
- Flatpak: fix RapidJSON 1.1.0 const-member build error on GCC 14 (`-fpermissive`)
- Flatpak: pre-install RapidJSON as a module; add `--disable-rofiles-fuse`
- Release: fix DEB filename collision when noble and jammy extracted together
- Windows: replace Chocolatey/SourceForge NSIS fallback with `winget install NSIS.NSIS`

*(All rc4 changes below)*

## 2.0rc4 — 2026-04

### Bug fixes (UAT)
- Dative bond toolbar button now shows a distinct half-arrow icon (was blank)
- Dative bond rubber-band preview now draws correctly while dragging
- Arrow tool: clicking the button face now activates the last-chosen arrow type
- Arrow tool: `bracket_type` properly initialised so first use draws the correct arrow

### Infrastructure
- Dead woodsidelabs.com endpoints replaced with PubChem REST API
  - "Find on PubChem…" (Ctrl+F) now searches PubChem by name, CAS, or formula
  - Molecule Information dialog (Ctrl+I) now populates CAS, IUPAC name, synonyms
- `netaccess.cpp` rewritten: `NetAccess` is now a plain `QObject` (not a `QDialog`)
- "Build 3D model" menu item shows informative dialog (server was defunct)
- CMake: `generated/defs.h` output directory avoids name collision with executable
- All deprecation warnings cleared on Linux (GCC 13), macOS (Xcode 16), Windows (MSVC)

---

## 2.0rc3 — 2026-03-31

### New features
- Name-to-structure: Tools → "Name to structure…" accepts IUPAC, common, and CAS names
  via PubChem REST API (backlog 5.7)
- InChI input: SMILES dialog detects `InChI=` prefix and routes through OpenBabel (5.8)

### Infrastructure
- Single `VERSION` file at repository root; all four platform builds read from it
- `defs.h.in` template with `@APP_VERSION@` placeholder; CMake stamps it at build time
- About/Support dialogs: replaced SourceForge URLs with GitHub repo and issues links
- All 13 translation files updated to match new C++ source strings
- RPM spec: `%{xdcver}` macro; `-DAPP_VERSION` passed through Docker via environment
- RPM `%files`: added `xdrawchem.metainfo.xml`
- macOS: `ob_compat.h` force-included for Xcode 16 / libc++19 (removes `std::binary_function`)
- Windows: NSIS `makensis.exe` found dynamically; `ilammy/msvc-dev-cmd` replaced with
  inline `vcvarsall.bat x64`

---

## 2.0rc2 — 2026-03-30

### New features
- Dative (coordinate covalent) bond type (bond order 9, half-arrowhead rendering)
- ACS publication style preset
- Improved SVG export via Qt6 `QSvgGenerator`
- Copy as SVG or 300 dpi PNG to clipboard
- Canonical SMILES output via OpenBabel
- IUPAC name lookup via PubChem REST API
- PubChem structure browser integration
- Valence checking
- PDF export
- Georgian translation (contributed by EkaterinePapava)

### Bug fixes
- Qt 6.2 compatibility: `QMenu::addAction` argument order fixed in `application.cpp`
  and `helpwindow.cpp` (keysequence must be last argument)

### Testing
- 13 test suites, 274 tests, all passing

---

## 2.0rc1 — 2026-03-26

### Port to Qt6 + CMake
- Zero build warnings or errors against Qt 6.2+
- `CMakeLists.txt` replaces legacy `qmake` `.pro` files
- `QXmlSimpleReader` → `QXmlStreamReader` (removed in Qt6)
- `QHttp` usage removed (removed in Qt5)
- 266 Qt3-era `foreach` → C++11 range-for loops
- Old-style `SIGNAL/SLOT` macros → pointer-to-member syntax throughout

### Bug fixes
- Fix #9: double bond inner-line geometry at non-linear angles
- Fix #10: "Clean up molecule" preserves molecule orientation (Procrustes alignment)
- Fix #13: curved arrow tips aligned using arc tangent direction
- Fix #14: Bézier arrow drawing (was entirely non-functional)
- Fix #15: SMILES output no longer produces `**` (MDL Molfile column fix)
- Fix #18: OpenBabel 3 builds without manual `.pro` file editing

### Infrastructure
- GitHub Actions CI for DEB (Ubuntu 24.04), RPM (Rocky 9), Windows (MSVC), macOS
- AppStream `xdrawchem.metainfo.xml` for GNOME Software / KDE Discover
- `.desktop` file and application icon for desktop integration

### Testing
- 261-test unit suite (10 suites), executed during every build

---

## 1.10.2 — 2016-12-11

- Initial Debian packaging
