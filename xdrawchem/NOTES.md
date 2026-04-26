# Developer Notes — Qt5 Modernisation

## Build instructions

```bash
cd xdrawchem
mkdir build && cd build
qmake ../xdrawchem.pro
make -j$(nproc)
```

**Dependencies**

| Package | Minimum version | Notes |
|---|---|---|
| Qt | 5.9 | Tested on 5.15.13 |
| OpenBabel | 3.x | Header path auto-detected by `xdrawchem.pro` |
| GCC / Clang | C++14 or later | `-std=c++14` minimum |

On Debian/Ubuntu:
```bash
sudo apt install qt5-qmake qtbase5-dev qttools5-dev libopenbabel-dev libssl-dev
```

---

## Qt6 migration readiness

This codebase now passes `-Wall -Wextra` cleanly on Qt 5.15 with zero warnings
from project source files. The following items were the main Qt6 blockers and
are now resolved:

| Removed API | Replacement used |
|---|---|
| `QXmlSimpleReader` / `QXmlInputSource` | `QXmlStreamReader` |
| `QHttp` | `QNetworkAccessManager` |
| `Qt::foreach` macro | C++11 range-for |
| `QTextStreamFunctions::endl` | `Qt::endl` |

**Remaining work before a Qt6 port:**

1. **Old-style `SIGNAL()`/`SLOT()` macros** — 122 string-based `connect()`
   calls remain. The safe built-in pairs (clicked/accept/reject/quit) were
   converted; the application-specific ones need the sender's concrete class
   type at the call site. Work through `application.cpp` (largest file) first.
   The new syntax is:
   ```cpp
   // old
   connect(obj, SIGNAL(mySignal(int)), this, SLOT(mySlot(int)));
   // new
   connect(obj, &MyClass::mySignal, this, &MyOtherClass::mySlot);
   ```

2. **`QTextStream` on `QString`** — Several call sites open a `QTextStream`
   with `QIODevice::ReadOnly` on a `QString` buffer. In Qt6 this constructor
   is removed; use `QTextStream ts(&str)` (no flags) for read-only, or
   `QStringView` / `QLatin1StringView` parsing instead.

3. **`QButtonGroup::buttonClicked(int)`** — The `int`-overload signal is
   removed in Qt6. Replace with `buttonClicked(QAbstractButton*)` and call
   `id()` explicitly, or use `idClicked(int)`.

4. **`QAction` in `QMenu::addAction`** — Several `addAction` overloads taking
   a raw member-function pointer were added in Qt5 and are the preferred form
   in Qt6. The existing calls using `SLOT()` strings still work but should be
   updated as part of the signal/slot pass above.

5. **`QRegExp`** — Not used currently but referenced in commented-out code.
   Ensure any future additions use `QRegularExpression`.

---

## Known remaining issues (not regressions)

- **MDL Mol/SDF I/O is stubbed out** — `chemdata_mdl.cpp` contains three
  functions (`load_mdl`, `ProcessMDL`, `save_mdl`) that return `false` without
  doing anything. This was the case before this modernisation pass. MDL format
  is the standard for structure interchange; implementing it via OpenBabel
  would be the highest-value I/O addition.

- **`netaccess.cpp` dead code** — Large sections of `NetAccess` are commented
  out (the old `QHttp`-based implementation). The active code path uses the
  `HTTP` helper class for synchronous-style requests. The `slotFinished`,
  `slotData`, `rf` slots and the `QBuffer *buffer` member are unreachable;
  they can be removed once the network layer is further rationalised.

- **No test suite** — There are zero unit or integration tests. The chemistry
  algorithms (SSSR, SMILES generation, NMR prediction, partial charge
  calculation) are good candidates for property-based testing. Recommended
  framework: Qt Test (`QTest`) for widget-level tests, Catch2 for pure
  algorithm tests.

- **`qDebug()` noise** — Many functions emit dense `qDebug()` traces at normal
  runtime (CML parser, network access, molecule operations). These should be
  gated behind a `QLoggingCategory` so they can be silenced in release builds
  without code changes.

---

## Architecture notes for new contributors

The codebase is split into three logical layers:

**Data model** — `ChemData` is the central document object. It owns a flat
`drawlist` of `Drawable*` objects (bonds, molecules, arrows, brackets, text,
symbols). All I/O goes through `ChemData`: the `chemdata_*.cpp` files handle
one file format each. `DPoint` is the shared atom/vertex type used by both
the drawing layer and the chemistry layer.

**Rendering** — `Render2D` is a `QWidget` subclass that handles all painting,
mouse events, and tool state. It's large (~5 000 lines across 6 files) and
is the main candidate for a future split into a proper MVC architecture.
`RenderArea` is a thin `QScrollArea` wrapper that emits scroll position changes.

**Chemistry** — `Molecule` (a `Drawable` subclass) owns lists of `Bond*`,
`Text*`, and `Symbol*` and implements all chemistry algorithms. OpenBabel
integration is isolated to `molecule_obmol.cpp` and `molecule_smiles.cpp`.
