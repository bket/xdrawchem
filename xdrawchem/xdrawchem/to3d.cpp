// to3d.cpp — Generate a 3D model of the current 2D structure.
//
// History
// =======
// The original implementation (Thomas Shattuck's BUILD3D, called via a
// SourceForge CGI endpoint) is preserved in git history.  That service
// went away when xdrawchem.sourceforge.net was decommissioned, leaving
// a stub that only popped up "feature unavailable".
//
// Restored in 2.1 using a local OpenBabel-based generator:
//   1.  Convert the 2D Molecule to an OBMol.
//   2.  Add explicit hydrogens.
//   3.  OBBuilder::Build() generates initial 3D coordinates from the
//       2D connectivity (using the built-in fragment library).
//   4.  Set up an MMFF94 force field (falls back to UFF for elements
//       MMFF94 cannot parameterise, e.g. some transition metals).
//   5.  Conjugate-gradients minimization for ~250 steps to relieve
//       any strain left after the initial build.
//   6.  Write the resulting OBMol to an MDL .mol file (3D atom block).
//
// All of this happens in-process — no network call, no temp Fortran
// program, no CGI.

#include "ob_compat.h"
OB_COMPAT_BEGIN
#include <openbabel/mol.h>
#include <openbabel/builder.h>
#include <openbabel/forcefield.h>
#include <openbabel/obconversion.h>
OB_COMPAT_END

#include <sstream>

#include <QFileDialog>
#include <QMessageBox>
#include <QString>

#include "molinfodialog.h"
#include "moldata.h"
#include "render2d.h"
#include "drawable.h"
#include "molecule.h"
#include "dpoint.h"
#include "defs.h"

extern Preferences preferences;

// invoked from tool_2d3d.cpp via Tool_2D3D::process(), and from
// chemdata_tools.cpp / application.cpp for headless export.
//
// fn3d: target filename.  Empty/length<=1 triggers a save dialog.
void Molecule::Make3DVersion( QString fn3d )
{
    using namespace OpenBabel;

    // 1. Convert to OBMol.  convertToOBMol() copies atoms/bonds/charges
    //    but leaves z=0 for every atom (since we are 2D internally).
    OBMol *obmol = convertToOBMol();
    if ( obmol == nullptr || obmol->NumAtoms() == 0 ) {
        QMessageBox::warning( getRender2D(),
                              tr( "3D model" ),
                              tr( "Nothing to convert: the molecule is empty." ) );
        delete obmol;
        return;
    }

    // 2. Add hydrogens explicitly.  OBBuilder::Build needs them for
    //    correct geometry, and the user expects the saved 3D file to
    //    contain Hs anyway.
    obmol->AddHydrogens();

    // 3. Build initial 3D coordinates from connectivity.
    OBBuilder builder;
    if ( !builder.Build( *obmol ) ) {
        QMessageBox::warning( getRender2D(),
                              tr( "3D model" ),
                              tr( "OpenBabel could not generate initial 3D "
                                  "coordinates for this structure.\n"
                                  "(Check for valence errors or unusual "
                                  "fragments.)" ) );
        delete obmol;
        return;
    }
    obmol->SetDimension( 3 );

    // 4. Set up a force field for cleanup.  Try MMFF94 first; if it
    //    cannot parameterise the molecule (rare elements, exotic
    //    valence states), fall back to UFF which covers the periodic
    //    table almost entirely.
    OBForceField *ff = OBForceField::FindForceField( "MMFF94" );
    if ( ff == nullptr || !ff->Setup( *obmol ) ) {
        ff = OBForceField::FindForceField( "UFF" );
        if ( ff == nullptr || !ff->Setup( *obmol ) ) {
            // No force field available — keep builder coords as-is.
            // Better than nothing; user gets a workable 3D model even
            // if it is slightly strained.
            qDebug() << "Make3DVersion: no force field available, "
                        "skipping minimization";
            ff = nullptr;
        }
    }

    // 5. Energy minimization.  250 conjugate-gradients steps is the
    //    OpenBabel obabel(1) default for `--gen3d` and gives a good
    //    balance between quality and runtime (sub-second for typical
    //    drug-sized molecules).
    if ( ff != nullptr ) {
        ff->ConjugateGradients( 250 );
        ff->GetCoordinates( *obmol );
    }

    // 6. Resolve target filename — caller may have passed one in
    //    (headless export from chemdata_tools), otherwise prompt.
    if ( fn3d.length() > 1 ) {
        if ( fn3d.right( 4 ).toLower() != ".mol" )
            fn3d.append( ".mol" );
    } else {
        QFileDialog fd( getRender2D() );
        fd.setWindowTitle( tr( "Save 3D file as..." ) );
        fd.setFileMode( QFileDialog::AnyFile );
        fd.setAcceptMode( QFileDialog::AcceptSave );
        fd.setNameFilters( QStringList( tr( "MDL molfile (*.mol)" ) ) );
        if ( fd.exec() != QDialog::Accepted ) {
            qDebug() << "To3D cancelled by user";
            delete obmol;
            return;
        }
        QStringList sel = fd.selectedFiles();
        if ( sel.isEmpty() ) {
            delete obmol;
            return;
        }
        fn3d = sel[0];
        if ( fn3d.right( 4 ).toLower() != ".mol" )
            fn3d.append( ".mol" );
    }

    // 7. Write OBMol to MDL .mol format (3D atom block preserved).
    OBConversion conv;
    if ( !conv.SetOutFormat( "mol" ) ) {
        QMessageBox::warning( getRender2D(),
                              tr( "3D model" ),
                              tr( "OpenBabel does not have an MDL "
                                  "molfile writer registered." ) );
        delete obmol;
        return;
    }

    std::ostringstream oss;
    if ( !conv.Write( obmol, &oss ) ) {
        QMessageBox::warning( getRender2D(),
                              tr( "3D model" ),
                              tr( "OpenBabel failed to serialise the "
                                  "3D model." ) );
        delete obmol;
        return;
    }

    QFile fout( fn3d );
    if ( !fout.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        QMessageBox::warning( getRender2D(),
                              tr( "3D model" ),
                              tr( "Could not open '%1' for writing." )
                                  .arg( fn3d ) );
        delete obmol;
        return;
    }
    fout.write( oss.str().c_str(), static_cast<qint64>( oss.str().size() ) );
    fout.close();

    qDebug() << "3D file written:" << fn3d;
    delete obmol;
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
