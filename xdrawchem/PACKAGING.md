# XDrawChem Packaging Guide

## File layout: bare names vs reverse-DNS

Starting in 2.0.1, CMake installs each user-facing data file twice — once
under its traditional bare name, and once under the reverse-DNS form that
Flathub and AppStream require:

| Bare name (DEB/RPM convention) | Reverse-DNS (Flathub/AppStream) |
|---|---|
| `xdrawchem.desktop` | `io.github.bryanherger.xdrawchem.desktop` |
| `xdrawchem.png` | `io.github.bryanherger.xdrawchem.png` |
| `xdrawchem.metainfo.xml` | `io.github.bryanherger.xdrawchem.metainfo.xml` |

Both are byte-identical copies. Both `debian/xdrawchem.install` and the
RPM `%files` section list both names, so `dpkg-buildpackage` and
`rpmbuild` accept the install tree without "unpackaged file" errors.
Flatpak's manifest cleans up the bare-name copies inside the sandbox
before `appstreamcli compose` runs, since both files declare the same
`<id>` and would otherwise trigger a `duplicate-component` rejection.

## RPM (RHEL 8/9, Fedora, Rocky, AlmaLinux)

### Prerequisites

**RHEL 8 / Rocky 8 / AlmaLinux 8:**
```bash
dnf install epel-release
dnf install rpm-build rpmdevtools cmake ninja-build gcc-c++ \
    qt6-qtbase-devel qt6-qttools-devel \
    openbabel-devel desktop-file-utils libGL-devel libEGL-devel
```

**RHEL 9 / Rocky 9 / AlmaLinux 9:**
```bash
dnf install epel-release
dnf install rpm-build rpmdevtools cmake ninja-build gcc-c++ \
    qt6-qtbase-devel qt6-qttools-devel \
    openbabel-devel desktop-file-utils libGL-devel libEGL-devel
```

**Fedora 38+:**
```bash
dnf install rpm-build rpmdevtools cmake ninja-build gcc-c++ \
    qt6-qtbase-devel qt6-qttools-devel \
    openbabel-devel desktop-file-utils libGL-devel libEGL-devel
```

### Build
```bash
rpmdev-setuptree
# Create source tarball from git:
git archive --prefix=xdrawchem-2.1/ HEAD:xdrawchem | \
    gzip > ~/rpmbuild/SOURCES/xdrawchem-2.1.tar.gz
cp SPECS/xdrawchem.spec ~/rpmbuild/SPECS/
rpmbuild -ba ~/rpmbuild/SPECS/xdrawchem.spec
```

The `%check` section runs all unit tests automatically during the build.

### Runtime dependencies installed by the RPM
- `qt6-qtbase` (provides Qt6 Core, GUI, Widgets, Network, PrintSupport)
- `openbabel-libs >= 3.0`

---

## DEB (Debian 12+, Ubuntu 22.04+, Ubuntu 24.04)

### Prerequisites

**Ubuntu 22.04 LTS:**
```bash
sudo apt install debhelper cmake ninja-build pkg-config \
    qt6-base-dev qt6-tools-dev \
    libopenbabel-dev libgl-dev libegl-dev \
    desktop-file-utils libxkbcommon-dev
```

**Ubuntu 24.04 LTS / Debian 12:**
```bash
sudo apt install debhelper cmake ninja-build pkg-config \
    qt6-base-dev qt6-tools-dev \
    libopenbabel-dev libgl-dev libegl-dev \
    desktop-file-utils libxkbcommon-dev
```

### Build
```bash
cd xdrawchem
dpkg-buildpackage -us -uc -b      # binary-only, no signing
# or for a source+binary build:
dpkg-buildpackage -us -uc
```

Produces:
- `xdrawchem_2.1-1_amd64.deb`       — main application
- `xdrawchem-doc_2.1-1_all.deb`     — HTML documentation

The `override_dh_auto_test` step runs all unit tests automatically.

### Install
```bash
sudo dpkg -i xdrawchem_2.1-1_amd64.deb
sudo apt-get install -f    # resolve any missing runtime deps
```

### Runtime dependencies installed by the DEB
- `libqt6core6`, `libqt6gui6`, `libqt6widgets6`, `libqt6network6`, `libqt6printsupport6`
- `libopenbabel7 >= 3.0`

---

## Windows (MSVC + NSIS)

### Prerequisites

- **Visual Studio 2022** (Community edition is free) with "Desktop development
  with C++" workload
- **CMake >= 3.19** (bundled with VS or from <https://cmake.org/>)
- **NSIS** (Nullsoft Scriptable Install System) — install via winget:
  ```powershell
  winget install NSIS.NSIS
  ```
- **vcpkg** for dependency management:
  ```powershell
  git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
  C:\vcpkg\bootstrap-vcpkg.bat
  ```
- **Qt6** installed via the online installer or vcpkg
- **OpenBabel 3.x** built or installed

### Build

The CI workflow (`.github/workflows/build-windows.yml`) handles the full build.
For a local build:

```powershell
cd xdrawchem
mkdir build && cd build

# Adjust paths to your Qt6 and OpenBabel installations:
cmake -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_PREFIX_PATH="C:\Qt\6.8.0\msvc2022_64" `
    -DOpenBabel_INCLUDE_DIR="C:\openbabel\include" `
    -DOpenBabel_LIBRARY="C:\openbabel\lib\openbabel.lib" `
    ..

cmake --build . --config Release
```

### Packaging

The `build-windows.yml` workflow produces:
- `xdrawchem-2.1-win64.exe` — NSIS installer
- `xdrawchem-2.1-win64.zip` — Portable zip (no installer)

Both include the Qt6 runtime DLLs, OpenBabel DLLs, and translation files.

### Runtime dependencies
- Qt6 Core, GUI, Widgets, Network, PrintSupport, Svg
- OpenBabel 3.x (`openbabel-3.dll`)
- MSVC Runtime (redistributable included in installer)

---

## macOS (DMG)

### Prerequisites

- **macOS 12+** (Monterey or later)
- **Xcode 14+** with command-line tools:
  ```bash
  xcode-select --install
  ```
- **Homebrew** (<https://brew.sh/>)
- **CMake >= 3.19**
- **Qt6** (via Homebrew or online installer):
  ```bash
  brew install qt@6
  ```
- **OpenBabel 3.x**:
  ```bash
  brew install open-babel
  ```

### Build

```bash
cd xdrawchem
mkdir build && cd build

cmake -G Ninja \
    -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)" \
    -DOpenBabel_INCLUDE_DIR="$(brew --prefix open-babel)/include/openbabel3" \
    -DOpenBabel_LIBRARY="$(brew --prefix open-babel)/lib/libopenbabel.dylib" \
    ..

ninja
```

### Packaging

The CI workflow (`.github/workflows/build-mac.yml`) produces a `.dmg`:

```bash
# Create app bundle and DMG (requires create-dmg from Homebrew)
brew install create-dmg

# The build-mac.yml workflow handles signing, notarization, and DMG creation.
# For local use, the raw app bundle is at:
ls build/xdrawchem.app
```

### Runtime dependencies
- Qt6 frameworks (embedded in `.app` by `macdeployqt`)
- OpenBabel (`libopenbabel.7.dylib`, relinked by CI)
- macOS 12+ (Intel or Apple Silicon, universal binary built by CI)

### Notes
- Apple Silicon (M1/M2/M3) and Intel binaries are built separately and
  lipo'd into a universal binary by the CI workflow.
- Code signing and notarization require an Apple Developer account.
  Unsigned builds work locally with Gatekeeper disabled (`xattr -cr`).

---

## Distribution compatibility matrix

| Distro / Platform     | Qt6 source   | OpenBabel source | Notes                          |
|-----------------------|-------------|-----------------|--------------------------------|
| RHEL 8 / Rocky 8      | EPEL 8      | EPEL 8          | Enable EPEL before building    |
| RHEL 9 / Rocky 9      | AppStream   | EPEL 9          | Enable EPEL for openbabel      |
| Fedora 38+            | Default     | Default         | No extra repos needed          |
| Debian 12 (Bookworm)  | Default     | Default         | No extra repos needed          |
| Ubuntu 22.04 LTS      | Default     | Default         | No extra repos needed          |
| Ubuntu 24.04 LTS      | Default     | Default         | No extra repos needed          |
| Windows 10/11 (MSVC)    | Online installer | vcpkg or self-build | VS 2022 required          |
| macOS 12+ (Intel/Apple) | Homebrew    | Homebrew        | Xcode 14+ required             |
