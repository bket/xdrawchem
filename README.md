# XDrawChem: a 2D molecule drawing program

XDrawChem is an open-source application for drawing and editing
two-dimensional chemical structures. It supports CML, SMILES, MDL Mol,
CDXML, and its own XDC format, and integrates with OpenBabel for
cheminformatics calculations (SMILES, InChI, NMR prediction, 3D).

![XDrawChem application screenshot showing synthesis of aspirin](xdrawchem_screenshot_0.png)

## Quick start

```bash
git clone https://github.com/bryanherger/xdrawchem
cd xdrawchem/xdrawchem-qt5
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
./xdrawchem
```

Requires **Qt 6.4+**, **OpenBabel 3.x**, and **CMake 3.19+**.
See [xdrawchem-qt5/README.md](xdrawchem-qt5/README.md) for full build instructions and dependencies.

## Repository layout

| Directory | Contents |
|---|---|
| `xdrawchem-qt5/` | **Current source** — Qt5 (qmake) and Qt6 (CMake) |
| `legacy-xdrawchem-qt4/` | Historical Qt4 port (not maintained) |
| `legacy-xdrawchem-qt3/` | Original Qt3 codebase (not maintained) |

## Links

- Releases: here on the right side of the page, and https://sourceforge.net/projects/xdrawchem/files/xdrawchem/
- Issues & PRs: here on GitHub, https://github.com/bryanherger/xdrawchem
- Backlog: See the [backlog](backlog.md) for known issues and possible enhancements.  Open an issue to add to the list.

Bryan Herger — bherger@users.sf.net
