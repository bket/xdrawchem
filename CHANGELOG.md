# XDrawChem Changelog

All notable changes to XDrawChem are documented here.
Entries are derived from the Debian changelog and commit history.

---

## 2.1rc0 — UNRELEASED (target: 2026-05-25)

### New Features
- **3D structure generation restored** — Tools → "Build 3D model of molecule" now
  produces a 3D MDL molfile locally using OpenBabel's `OBBuilder` for initial
  coordinate generation followed by 250 steps of MMFF94 (or UFF fallback)
  conjugate-gradients minimization. The original BUILD3D code called a
  Fortran-based CGI service on SourceForge that no longer exists; the feature
  had been disabled with a placeholder dialog. No network call required now.
- **MDL MOL/SDF file I/O** — `chemdata_mdl.cpp` now reads and writes standard MDL
  MOL and SDF files via OpenBabel, replacing the previous no-op stubs.
- **SDF multi-record browser** — When opening an `.sdf` file via File → Open,
  a browser dialog lists all records with molecule names and lets the user
  navigate with Previous/Next, import individual records, or import all at once.
- **CIP R/S and E/Z stereochemistry labels** — Tools → "Show CIP labels" toggles
  display of Cahn-Ingold-Prelog descriptors computed by OpenBabel.
  - **R/S** (tetrahedral): shown in bold blue text next to chiral centers
    (e.g. asymmetric carbon atoms with wedge/hash bonds).
  - **E/Z** (cis/trans): shown at the midpoint of stereogenic double bonds.
  - Labels are cached and only recomputed when the molecule structure changes.

### Bug Fixes
- **Memory leaks in `application_ob.cpp`** — `OBNewLoad()` and `OBNewSave()`
  now properly delete temporary `OBMol` objects.
- **Memory leak in `molecule_obmol.cpp`** — `convertToOBMol()` no longer leaks
  the returned `OBMol*` on error paths.

### Qt / C++ Modernisation
- **Remaining `SIGNAL()`/`SLOT()` string macros** converted to
  pointer-to-member syntax across `application.cpp`, `render2d_event.cpp`,
  and `main.cpp`. Zero string-based `connect()` calls remain in active code.
- **`qDebug()` runtime noise gated behind `QLoggingCategory`** — all
  `qDebug()` / `qInfo()` / `qWarning()` calls in the chemistry and I/O layers
  now use category macros so they can be silenced with `QT_LOGGING_RULES=xdc=false`.

### Code Cleanup
- **`netaccess.cpp`** — removed large commented-out `QHttp`-based dead code
  and unreachable `slotFinished` / `slotData` / `rf` slots.
- **Repository cleanup** — legacy Qt3/Qt4 source trees removed from `master`;
  source directory renamed from `xdrawchem-qt5/` to `xdrawchem/`.
- **Flatpak** — cleaned up bare-name desktop/metainfo/icon copies inside the sandbox
  so `appstreamcli compose` does not see duplicate `<id>` entries and reject the build.

### Documentation
- README: updated screenshot to reflect current UI.
- `backlog.md`: refreshed to reflect completed v2.1 items.

---

## 2.0.1 — 2026-04-23

### Security / correctness
- Fix three instances of malloc/strcpy buffer overflow + use-after-free on QByteArray
  temporary in OpenBabel format resolution (`application_ob.cpp`, `tool_2d3d.cpp`)
- Fix potential buffer overflow in `char type[5]` strcpy on user-defined atom labels
  (`molecule_obmol.cpp`, `ioiface.cpp`)
- Replace `system()` shell invocation for Ghemical/KryoMol send with
  `QProcess::startDetached` to eliminate shell injection risk
- Remove dead `http.cpp`/`http.h` module and dead `XDC_SERVER` / `getenv` code paths
- Fix `-Wreorder` warning in `CMLParser` constructor

### UI fixes
- Ring menu: wire Amino Acids, Nucleic Acids, Sugars, and Useful Groups submenus
  to `setRingAction` (they were silent since the Qt6 port)
- Ring menu: add `setData()` to Useful Groups (FMOC, BOC, DABCYL, DABSYL, DANSYL,
  EDANS, Biotin) so dispatch actually works
- Custom ring submenu: restore dispatch via per-action lambda forwarding to
  `FromRingMenu` (silent since the Qt6 port; user-saved rings can be placed again)
- Custom ring save: fix signal-11 crash on save caused by uninitialised
  `ringmenu` member pointer in `application.h`
- Custom ring menu refresh: swap only the User-defined submenu in place rather
  than rebuilding the whole ring menu (avoids a paint-device warning and stale
  defaultAction pointers in `drawRingButton`)
- Property panel: refresh on ring placement, name-to-structure, selection change,
  clipboard operations, undo, text/label edits, delete (previously only refreshed
  for raw drawing actions)
- Property panel: track currently-selected molecule rather than always showing
  the first molecule
- Amino Acids menu: remove non-natural amino acids (nitrophenylalanine, statine)
  that can still be retrieved via PubChem lookup

### Packaging
- Debian: bump Standards-Version to 4.7.2
- Flatpak: app-id renamed to `io.github.bryanherger.xdrawchem` (Flathub requirement)
- Flatpak: desktop and metainfo files now install with reverse-DNS filenames
- Flatpak: OpenBabel build uses `cmake-ninja` + `builddir:true` for sandbox correctness
- DEB and RPM: include the reverse-DNS desktop / metainfo / icon copies in
  `debian/xdrawchem.install` and `%files` (CMake installs both bare and
  reverse-DNS names; both packaging systems are strict about unpackaged files)
- RPM `%files`: use literal `io.github.bryanherger.*` glob patterns so future
  reverse-DNS additions (e.g. MIME icons) don't need spec changes
- Added `index.html` for GitHub Pages redirect to repo

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
