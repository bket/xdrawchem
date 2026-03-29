cmake_minimum_required(VERSION 3.19)
project(xdrawchem VERSION 2.0 LANGUAGES CXX)

# ── C++ standard ───────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type default ─────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ── Qt6 ───────────────────────────────────────────────────────────────────────
# Tells Qt's CMake integration to run moc/uic/rcc automatically on all targets
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    PrintSupport
    Xml
    LinguistTools
)

# ── OpenBabel ─────────────────────────────────────────────────────────────────
# Try pkg-config first (most reliable on Linux), then fall back to manual search
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(OpenBabel QUIET openbabel-3)
endif()

if(NOT OpenBabel_FOUND)
    # Manual search across common install prefixes
    find_path(OpenBabel_INCLUDE_DIR
        NAMES openbabel/mol.h
        PATHS
            /usr/include/openbabel3
            /usr/local/include/openbabel3
            /usr/local/include/openbabel-2.0
            /opt/homebrew/include/openbabel3
        REQUIRED
    )
    find_library(OpenBabel_LIBRARY
        NAMES openbabel openbabel-3
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
        REQUIRED
    )
    set(OpenBabel_INCLUDE_DIRS ${OpenBabel_INCLUDE_DIR})
    set(OpenBabel_LIBRARIES    ${OpenBabel_LIBRARY})
endif()

message(STATUS "OpenBabel include: ${OpenBabel_INCLUDE_DIRS}")
message(STATUS "OpenBabel libs:    ${OpenBabel_LIBRARIES}")

# ── Install prefix ────────────────────────────────────────────────────────────
if(NOT DEFINED RINGHOME)
    set(RINGHOME "${CMAKE_INSTALL_PREFIX}/share/xdrawchem")
endif()

# ── Source files ──────────────────────────────────────────────────────────────
set(XDC_SOURCES
    xdrawchem/application.cpp
    xdrawchem/application_ob.cpp
    xdrawchem/application_ring.cpp
    xdrawchem/arrow.cpp
    xdrawchem/biotools.cpp
    xdrawchem/bond.cpp
    xdrawchem/bondedit.cpp
    xdrawchem/boshcp.cpp
    xdrawchem/bracket.cpp
    xdrawchem/cdx2cdxml.cpp
    xdrawchem/cdxml_reader.cpp
    xdrawchem/charsel.cpp
    xdrawchem/chemdata_cdx.cpp
    xdrawchem/chemdata_cdxml.cpp
    xdrawchem/chemdata_cml.cpp
    xdrawchem/chemdata.cpp
    xdrawchem/chemdata_edit.cpp
    xdrawchem/chemdata_event.cpp
    xdrawchem/chemdata_mdl.cpp
    xdrawchem/chemdata_rw.cpp
    xdrawchem/chemdata_rxn.cpp
    xdrawchem/chemdata_tools.cpp
    xdrawchem/chemdata_xdc.cpp
    xdrawchem/chemdata_xml.cpp
    xdrawchem/colorbutton.cpp
    xdrawchem/crings_dialog.cpp
    xdrawchem/curvearrow.cpp
    xdrawchem/drawable.cpp
    xdrawchem/dyk.cpp
    xdrawchem/fixeddialog.cpp
    xdrawchem/gobject.cpp
    xdrawchem/graphdialog.cpp
    xdrawchem/graphwidget.cpp
    xdrawchem/helpwindow.cpp
    xdrawchem/http.cpp
    xdrawchem/ioiface.cpp
    # main.cpp excluded from object lib — linked only into the executable
    xdrawchem/molecule_1h_nmr.cpp
    xdrawchem/molecule.cpp
    xdrawchem/molecule_obmol.cpp
    xdrawchem/molecule_smiles.cpp
    xdrawchem/molecule_sssr.cpp
    xdrawchem/molecule_tools_1.cpp
    xdrawchem/molecule_tools_2.cpp
    xdrawchem/molinfodialog.cpp
    xdrawchem/myfiledialog.cpp
    xdrawchem/netaccess.cpp
    xdrawchem/netchoosedialog.cpp
    xdrawchem/netdialog.cpp
    xdrawchem/ngw.cpp
    xdrawchem/pagesetupdialog.cpp
    xdrawchem/peptidebuilder.cpp
    xdrawchem/previewwidget.cpp
    xdrawchem/render2d.cpp
    xdrawchem/render2d_draw.cpp
    xdrawchem/render2d_event.cpp
    xdrawchem/render2d_print.cpp
    xdrawchem/render2d_select.cpp
    xdrawchem/render2d_text.cpp
    xdrawchem/renderarea.cpp
    xdrawchem/retro.cpp
    xdrawchem/ringdialog.cpp
    xdrawchem/smilesdialog.cpp
    xdrawchem/symbol.cpp
    xdrawchem/text.cpp
    xdrawchem/textshapedialog.cpp
    xdrawchem/to3d.cpp
    xdrawchem/tool_13c_nmr.cpp
    xdrawchem/tool_1h_nmr.cpp
    xdrawchem/tool_2d3d.cpp
    xdrawchem/tooldialog.cpp
    xdrawchem/tool_ir.cpp
    xdrawchem/xdc_toolbutton.cpp
    xdrawchem/xml_cml.cpp
    xdrawchem/xml_reader.cpp
    xdrawchem/xruler.cpp
    xdrawchem/vector2D.cpp
)

set(XDC_HEADERS
    xdrawchem/aa_xpm.h
    xdrawchem/application.h
    xdrawchem/arrow.h
    xdrawchem/arrows.h
    xdrawchem/atom.h
    xdrawchem/biotools.h
    xdrawchem/bond.h
    xdrawchem/bondedit.h
    xdrawchem/boshcp.h
    xdrawchem/bracket.h
    xdrawchem/brackets.h
    xdrawchem/CDXConstants.h
    xdrawchem/cdxml_reader.h
    xdrawchem/charsel.h
    xdrawchem/chemdata.h
    xdrawchem/clipboard.h
    xdrawchem/cml.h
    xdrawchem/colorbutton.h
    xdrawchem/crings_dialog.h
    xdrawchem/cubicbezier.h
    xdrawchem/curvearrow.h
    xdrawchem/defs.h
    xdrawchem/dpoint.h
    xdrawchem/drawable.h
    xdrawchem/dyk.h
    xdrawchem/fixeddialog.h
    xdrawchem/gobject.h
    xdrawchem/graphdata.h
    xdrawchem/graphdialog.h
    xdrawchem/graphwidget.h
    xdrawchem/helpwindow.h
    xdrawchem/http.h
    xdrawchem/ioiface.h
    xdrawchem/justifytools.h
    xdrawchem/lines.h
    xdrawchem/magnifytools.h
    xdrawchem/moldata.h
    xdrawchem/molecule.h
    xdrawchem/molecule_sssr.h
    xdrawchem/molinfodialog.h
    xdrawchem/myfiledialog.h
    xdrawchem/na_xpm.h
    xdrawchem/netaccess.h
    xdrawchem/netchoosedialog.h
    xdrawchem/netdialog.h
    xdrawchem/ngw.h
    xdrawchem/paintable.h
    xdrawchem/peak.h
    xdrawchem/peptidebuilder.h
    xdrawchem/prefs.h
    xdrawchem/previewwidget.h
    xdrawchem/render2d.h
    xdrawchem/renderarea.h
    xdrawchem/ring.h
    xdrawchem/ringdialog.h
    xdrawchem/rings.h
    xdrawchem/ring_xpm.h
    xdrawchem/sdg.h
    xdrawchem/setofrings.h
    xdrawchem/smilesdialog.h
    xdrawchem/sorf.h
    xdrawchem/sugar_xpm.h
    xdrawchem/symbol.h
    xdrawchem/symbol_xpm.h
    xdrawchem/text.h
    xdrawchem/textshapedialog.h
    xdrawchem/tool_13c_nmr.h
    xdrawchem/tool_1h_nmr.h
    xdrawchem/tool_2d3d.h
    xdrawchem/tool_ir.h
    xdrawchem/tooldialog.h
    xdrawchem/vector2D.h
    xdrawchem/xdc_event.h
    xdrawchem/xdc_textedit.h
    xdrawchem/xdc_toolbutton.h
    xdrawchem/xml_cml.h
    xdrawchem/xml_reader.h
    xdrawchem/xruler.h
)

# ── Target ────────────────────────────────────────────────────────────────────
# OBJECT library — lets tests link the same compiled units without relinking
add_library(xdrawchem_obj OBJECT
    ${XDC_SOURCES}
    ${XDC_HEADERS}
)
target_include_directories(xdrawchem_obj PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/xdrawchem
    ${OpenBabel_INCLUDE_DIRS}
)
target_compile_definitions(xdrawchem_obj PUBLIC
    UNIX
    RINGHOME="${RINGHOME}"
    XDC_SERVER="https://www.woodsidelabs.com/chemistry"
)
target_link_libraries(xdrawchem_obj PUBLIC
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::PrintSupport Qt6::Xml
    ${OpenBabel_LIBRARIES}
)

add_executable(xdrawchem
    $<TARGET_OBJECTS:xdrawchem_obj>
    xdrawchem/main.cpp
    ${XDC_HEADERS}
)
set_target_properties(xdrawchem PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

# ── Include paths ─────────────────────────────────────────────────────────────
target_include_directories(xdrawchem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/xdrawchem
    ${OpenBabel_INCLUDE_DIRS}
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(xdrawchem PRIVATE
    UNIX
    RINGHOME="${RINGHOME}"
    XDC_SERVER="https://www.woodsidelabs.com/chemistry"
)

# ── Compiler warnings ─────────────────────────────────────────────────────────
target_compile_options(xdrawchem PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:Debug>>:-O0 -g3 -rdynamic>
)

# ── Link libraries ────────────────────────────────────────────────────────────
target_link_libraries(xdrawchem PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::PrintSupport
    Qt6::Xml
    ${OpenBabel_LIBRARIES}
)

# ── Translations ──────────────────────────────────────────────────────────────
# qt_add_translations compiles .ts -> .qm and embeds them;
# list the .ts files that exist and are non-trivially translated
set(XDC_TS_FILES
    translation/xdrawchem_da.ts
    translation/xdrawchem_de.ts
    translation/xdrawchem_dk.ts
    translation/xdrawchem_en.ts
    translation/xdrawchem_es.ts
    translation/xdrawchem_fr.ts
    translation/xdrawchem_it.ts
    translation/xdrawchem_jp.ts
    translation/xdrawchem_ka.ts
    translation/xdrawchem_nl.ts
    translation/xdrawchem_pl.ts
    translation/xdrawchem_pt.ts
    translation/xdrawchem_pt_BR.ts
)
qt_add_translations(xdrawchem
    TS_FILES ${XDC_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE XDC_QM_FILES
)

# ── Install rules ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS xdrawchem
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Ring templates and data files
install(DIRECTORY ring/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xdrawchem
    FILES_MATCHING
        PATTERN "*.cml"
        PATTERN "*.png"
        PATTERN "*.xpm"
        PATTERN "*.txt"
        PATTERN "*.qm"
        PATTERN "xdrawchemrc"
)

# HTML documentation
install(DIRECTORY doc/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xdrawchem/doc
    FILES_MATCHING
        PATTERN "*.html"
        PATTERN "*.png"
)

# .desktop file for application menu
install(FILES xdrawchem.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

# Application icon
install(FILES ring/xdrawchem-icon.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps
    RENAME xdrawchem.png
)

# ── Tests ───────────────────────────────────────────────────────────────
include(CTest)
add_subdirectory(tests)
