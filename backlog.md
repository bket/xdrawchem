# XDrawChem — Consolidated Backlog
*Compiled from all session history, GitHub issues, Debian tracker, and competitive analysis*
*As of v2.0rc2 — March 2026*

[BryanH - I will work on some or all of these eventually, but if anything here should be a priority, copy the item and create an issue]

---

## Status at v2.0rc2

**Completed this cycle:**
- Qt6 + CMake port (zero warnings/errors)
- 13 test suites, 274 tests, 100% passing
- Bugs fixed: #9 double bond geometry, #10 clean-up orientation, #13 curved arrow tips, #14 Bézier arrows, #15 SMILES `**`, #18 OpenBabel 3, #19 Georgian translation
- Features added: IUPAC naming (PubChem REST), PubChem browser, valence checking, PDF export, Fusion/light-palette startup fix, dative bond (order 9), ACS style preset, SVG export (QSvgGenerator), copy as SVG/PNG, canonical SMILES
- CI/CD: GitHub Actions for CI, DEB (Ubuntu 22.04 + 24.04), RPM (Rocky 9), Release

---

## SECTION 1 — Open GitHub Issues (still unresolved)

These were open on the tracker at the start of the session. Check https://github.com/bryanherger/xdrawchem/issues for any new ones.

| # | Title | Notes |
|---|---|---|
| Open | Any issues filed after rc2 ships | Monitor after release |

---

## SECTION 2 — Code Quality / Technical Debt
*Low risk, relatively small scope — good for a maintenance sprint*

**2.1 Deprecated QMessageBox overloads** — 3 warnings in `application.cpp` at lines 722, 1533, 2155. All use the old multi-string `question()`/`information()` signatures deprecated in Qt5 and producing warnings in Qt6. Should be replaced with the `StandardButtons` overload. Easy, self-contained.

**2.2 Dead `CalcName()` CGI endpoint** — The old name-lookup pathway calls a dead CGI script on woodsidelabs.com. It should be removed or redirected to the new `IUPACName()` PubChem pathway. The menu item may still be wired to the old code.

**2.3 Dead "Find on Internet" endpoint** — `NetDialog` code still exists but the endpoint it calls is gone. Should be replaced by the PubChem browser integration already in the codebase, or removed.

**2.4 Dative bond toolbar icon** — `ring/dativetool.png` is currently a copy of `wavytool.png` used as a placeholder. A proper half-arrow icon (→ with one barb) would make the toolbar unambiguous. Can be drawn as a 22×22 SVG and converted.

**2.5 CHANGELOG.md is sparse** — The file exists but only has stub entries. Should be kept updated going forward; the debian/changelog already has the right content and could be used as a source.

**2.6 Help window HTML docs** — The `doc/` folder contains aging HTML that references Qt3-era UI. Could be modernised in place or linked to a GitHub Wiki.

---

## SECTION 3 — Platform & Distribution
*Enables new user populations; mostly CI/packaging work*

**3.1 Ubuntu 22.04 DEB re-enablement** — The 22.04 DEB build was fixed for the Qt6.2 `addAction` argument order, so it now builds. Consider re-enabling it as an official release artifact (currently only 24.04 Noble is packaged for DEB).

**3.2 Fedora 40 RPM re-enablement** — Disabled in `build-rpm.yml` during initial CI setup. Could be re-added now that the Rocky 9 build is stable.

**3.3 AppStream / metainfo XML** — The Debian tracker flags an AppStream warning. An `xdrawchem.metainfo.xml` file in `/usr/share/metainfo/` would enable proper display in GNOME Software, KDE Discover, and Flathub. Required format: AppStream 0.12+. Content: name, summary, description, screenshots, URL, licence, categories (`Education;Science;Chemistry`), releases block.

**3.4 `debian/watch` file** — The Debian tracker flags a `uscan` error. Adding a `debian/watch` file pointing to the GitHub releases API would let Debian maintainers track upstream automatically.

**3.5 Flathub submission** — A `org.xdrawchem.XDrawChem.yml` Flatpak manifest would allow distribution via Flathub, reaching users on any Linux distro without distro packaging. Dependencies (Qt6, OpenBabel) are available in Flathub's SDK. Requires the AppStream metainfo file (3.3) first.

**3.6 macOS build** — Homebrew has `qt@6` and `open-babel`. No CI runner configured. A `build-mac.yml` using `macos-latest` with `brew install qt@6 open-babel cmake ninja` and a `macdeployqt`-produced `.dmg` would cover macOS users.

**3.7 Windows native build** — MinGW cross-compile or MSVC. Qt6 for Windows can be installed via `aqtinstall` in GitHub Actions. OpenBabel Windows builds are available. More involved than macOS but achievable with a dedicated workflow.

**3.8 Repository screenshot** — No screenshot in the repo. GNOME Software, Flathub, and most software directories require at least one screenshot. A 1280×800 PNG of a typical molecule drawing session would suffice.

---

## SECTION 4 — Feature Parity: Medium Priority
*These are present in ChemDraw, ChemDoodle, Ketcher, and MarvinSketch. Medium implementation effort.*

**4.1 CIP R/S and E/Z stereochemistry labels** — Every professional tool displays R/S labels at chiral centres and E/Z at double bonds. XDrawChem draws wedge/hash bonds correctly but never annotates the structure with the descriptor. OpenBabel can calculate CIP assignments from the existing stereo data; the work is mainly overlaying the labels on the canvas and encoding them in CML/Molfile output. This is the single highest-value missing feature for pharmaceutical and organic chemistry users.

**4.2 Atom-to-atom reaction mapping** — Numbers atoms across a reaction arrow so you can track which reactant atom becomes which product atom. Required for reaction databases, retrosynthesis tools, and metabolic pathway analysis. Needs a new "map atoms" UI mode and a way to store/display the mapping numbers; RXN file format (for import/export) then follows naturally.

**4.3 MDL SDF / RXN file support** — SDF (Structure-Data File) is the most widely used format for exchanging collections of structures — every compound database exports SDF. RXN is the reaction equivalent. OpenBabel handles both; it's mostly a UI matter of supporting multi-record files and displaying a structure list/browser. Moderate effort.

**4.4 Formal charges and isotope labels on atoms** — Click an atom to add +, −, 2+, 2−, or a radical dot, and have that encoded in the Molfile `charge` field (not just as cosmetic text). Similarly, isotope labels (¹⁴C, ²H, ³H) should be stored in the Molfile `isotope` field. Currently XDrawChem has charge symbols as insertable graphical objects but they are not chemically meaningful. Needed for NMR labelling, reaction mechanisms, and inorganic chemistry.

**4.5 Live property calculation panel** — A `QDockWidget` that shows MW, molecular formula, exact mass, and elemental composition, updating in real-time as you draw. MarvinSketch has this as a "pinnable calculation box". OpenBabel and the existing `MolData` class provide all the calculations; it's primarily a UI docking panel. Medium effort with high usability payoff.

**4.6 Functional group / fragment template browser** — A categorised template palette for common protecting groups and functional group abbreviations: Boc, Fmoc, Cbz, tosyl (Ts), triflate (OTf), mesylate (OMs), TBS, TMS, Bn, PMB, etc. These are placed constantly in synthetic organic chemistry. The existing CML ring infrastructure would support this; it's mostly a library curation task plus a browseable UI. Ketcher ships 450+ such fragments.

**4.7 S-Groups and polymer notation** — SRU (Structural Repeating Unit) brackets with subscript `n`, used in polymer chemistry and patent applications. Also superatom abbreviations (Ph, Bn, etc.) that expand/collapse. Both Ketcher and ChemDoodle support this. Higher effort — requires a new data model concept alongside the existing bond/atom model.

---

## SECTION 5 — Feature Parity: Lower Priority
*Nice-to-have; present in top-tier tools but not critical for general use*

**5.1 Structure-from-image OCR (OSRA)** — OSRA (Optical Structure Recognition Application) converts scanned or photographed chemical structures into SMILES/SDF. Historically available as a plugin for Accelrys Draw; ChemDoodle has a paid cloud version. An OSRA integration (calling the OSRA binary or library) would be extremely useful for digitising textbook structures but is a significant integration project.

**5.2 Fischer, Haworth, and Newman projection tools** — ChemDoodle 2D automatically recognises and analyses these projection types. XDrawChem has a Newman projection mode but no Fischer or Haworth support. Medium effort for drawing; automatic stereo assignment from projections would be higher effort.

**5.3 Lone pair display on atoms** — Clicking an atom to display lone pairs (horizontal or vertical electron dot pairs) is used heavily in mechanism drawing. ChemSketch added this via the atom properties dialog. The lone pair is cosmetic (does not affect valence counting) so it's a rendering addition only.

**5.4 Electron pushing arrows (mechanism tools)** — ChemDoodle 12.9 added automatic mechanism generation from electron-pushing arrows. At minimum, single-electron (fishhook) and two-electron pushing arrows anchored to bonds or atoms (not just free-floating curved arrows) would improve mechanism drawing. The existing curved arrow infrastructure is a starting point.

**5.5 Superscript/subscript in text labels** — ChemDoodle highlights this as a differentiator. Currently XDrawChem text labels are plain text. Allowing mixed super/subscript in labels (e.g. SO₄²⁻ rendered properly) would improve publication-quality output. Qt's `QTextDocument` rich text engine could handle this.

**5.6 SMARTS input** — SMARTS format (`FindFormat("sma")`) is not compiled into the system OpenBabel package on Ubuntu/Rocky. However, OB's SMILES reader silently accepts SMARTS atoms, so reading SMARTS strings through the existing SMILES input dialog already works for most cases. True SMARTS output (for substructure query export) requires OB's SMARTS writer, which may need a custom OB build. Low value unless specifically requested.

**5.7 Name-to-structure (NTS)** — Convert an IUPAC name to a drawn structure. ChemDraw and MarvinSketch both offer this. OPSIN (Open Parser for Systematic IUPAC Nomenclature) is a Java library available under the MIT licence that handles this well and is used by ChemDoodle Web. Integrating it requires either a Java subprocess call or using the OPSIN REST API. Medium effort.

**5.8 InChI input (structure from InChI string)** — Currently XDrawChem can output InChI but not read it back. OpenBabel can convert InChI → Molfile, so this is a small addition to the existing SMILES/InChI input dialog. Low effort.

---

## SECTION 6 — CI/CD & Infrastructure Improvements

**6.1 GPG-signed packages** — Neither the RPM nor the DEB are GPG-signed. Adding a signing step using a repository key stored as a GitHub Actions secret would make the packages installable from a proper apt/dnf repository without manual trust overrides.

**6.2 Apt/DNF repository hosting** — Rather than requiring users to manually download `.deb`/`.rpm` files, host a simple repository on GitHub Pages (`deb.xdrawchem.org`, `rpm.xdrawchem.org`) using `reprepro` (DEB) and `createrepo` (RPM). Users could then do `apt install xdrawchem` after adding the repo.

**6.3 Release workflow robustness** — The current release workflow polls the GitHub API for artifacts with a 20-minute timeout. This is fragile if builds take longer. Switching to `workflow_run` trigger (once GitHub fixes tag support for it) or using a matrix-completion check would be more reliable.

**6.4 Ubuntu 22.04 DEB in release** — Once re-enabled (see 3.1), the release workflow should attach the Jammy `.deb` alongside the Noble one.

**6.5 Code coverage reporting** — Add `gcov`/`lcov` to the CI run and upload a coverage report (e.g. to Codecov). Useful for identifying untested paths as test count grows.

**6.6 Lintian clean** — The Debian tracker reports one lintian warning on the packaged version. Running `lintian --pedantic` on the built DEB and addressing the warning would make the package ready for Debian proper.

---

## SECTION 7 — Documentation

**7.1 GitHub Wiki** — Replace or supplement the aging HTML help docs with a GitHub Wiki covering: installation, first steps, file formats supported, keyboard shortcuts, and the ring library. Would dramatically reduce barrier to entry for new users.

**7.2 Screenshot in repo** — Required for Flathub and most software directories (see 3.8). Needed before any store submission.

**7.3 PACKAGING.md completeness** — The file exists and covers Linux packaging. Adding Windows and macOS sections (even as stubs with "planned") sets expectations clearly.

**7.4 Keyboard shortcuts reference** — No comprehensive shortcut list exists in the docs. A one-page cheat sheet (PDF + Markdown) covering all draw modes, ring shortcuts, and menu accelerators would help power users.

---

## Priority Summary for v2.1 Planning

| Priority | Item | Effort | Impact |
|---|---|---|---|
| 🔴 High | 2.1 Fix deprecated QMessageBox warnings | Low | Eliminates build warnings |
| 🔴 High | 4.1 CIP R/S, E/Z labels | Medium | Major feature gap vs all competitors |
| 🔴 High | 3.3 AppStream metainfo XML | Low | Required for Flathub/GNOME Software |
| 🟡 Medium | 4.3 SDF / RXN file support | Medium | Opens compound database workflows |
| 🟡 Medium | 4.4 Formal charges + isotopes on atoms | Medium | Needed for mechanisms and NMR |
| 🟡 Medium | 4.5 Live property panel | Medium | High usability payoff |
| 🟡 Medium | 3.5 Flathub submission | Low-Med | Reaches all Linux users |
| 🟡 Medium | 5.3 Lone pair display | Low | Mechanism drawing quality |
| 🟡 Medium | 5.8 InChI input | Low | Completes InChI round-trip |
| 🟡 Medium | 2.4 Proper dative bond icon | Low | Polish |
| 🟢 Lower | 4.2 Reaction atom mapping | High | Important for reaction DBs |
| 🟢 Lower | 4.6 Fragment template browser | Medium | Organic chemistry workflows |
| 🟢 Lower | 3.6 macOS build | Medium | New platform |
| 🟢 Lower | 5.7 Name-to-structure (OPSIN) | Medium | Power feature |
| 🟢 Lower | 6.1 GPG-signed packages | Medium | Security/trust |
| 🟢 Lower | 3.7 Windows build | High | New platform |
| 🟢 Lower | 5.1 OSRA image OCR | High | Very useful but large integration |
