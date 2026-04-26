// globals_stub.cpp — provides the global variables defined in main.cpp.
// Required when linking tests against xdrawchem_obj (which excludes main.cpp
// to avoid a main() collision, but whose translation units reference these globals).

#include "../xdrawchem/prefs.h"
#include <QString>

// Global preferences object (main.cpp: "Preferences preferences;")
Preferences preferences;

// Ring template directory and home directory (main.cpp: "QString RingDir, HomeDir;")
QString RingDir;
QString HomeDir;
