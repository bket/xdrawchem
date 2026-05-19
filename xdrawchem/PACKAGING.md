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
sudo dpkg -i xdrawchem_2.0~rc1-1_amd64.deb
sudo apt-get install -f    # resolve any missing runtime deps
```

### Runtime dependencies installed by the DEB
- `libqt6core6`, `libqt6gui6`, `libqt6widgets6`, `libqt6network6`, `libqt6printsupport6`
- `libopenbabel7 >= 3.0`

---

## Distribution compatibility matrix

| Distro                | Qt6 source   | OpenBabel source | Notes                          |
|-----------------------|-------------|-----------------|--------------------------------|
| RHEL 8 / Rocky 8      | EPEL 8      | EPEL 8          | Enable EPEL before building    |
| RHEL 9 / Rocky 9      | AppStream   | EPEL 9          | Enable EPEL for openbabel      |
| Fedora 38+            | Default     | Default         | No extra repos needed          |
| Debian 12 (Bookworm)  | Default     | Default         | No extra repos needed          |
| Ubuntu 22.04 LTS      | Default     | Default         | No extra repos needed          |
| Ubuntu 24.04 LTS      | Default     | Default         | No extra repos needed          |
