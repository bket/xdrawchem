// chemdata_mdl.cpp — MDL Mol/SDF file I/O via OpenBabel.
//
// History
// =======
// Previously stubbed out (returned false for all three entry points).
// Implemented in 2.1 using OpenBabel's mol format reader/writer,
// leveraging the existing Molecule::convertFromOBMol() and
// Molecule::convertToOBMol() pathways.

#include "ob_compat.h"
OB_COMPAT_BEGIN
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
OB_COMPAT_END

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "xdc_logging.h"

#include "chemdata.h"
#include "molecule.h"
#include "render2d.h"

bool ChemData::load_mdl( QString fn )
{
    using namespace OpenBabel;

    QFile f( fn );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        QMessageBox::warning( r, tr( "Open MDL" ),
                              tr( "Could not open file %1 for reading." ).arg( fn ) );
        return false;
    }

    QByteArray data = f.readAll();
    f.close();

    // Determine format: SDF if "$$$$" present, otherwise standard MOL
    QString content( data );
    QString fmt = content.contains( QLatin1String( "$$$$" ) ) ? QStringLiteral( "sdf" )
                                                             : QStringLiteral( "mol" );
    qCDebug(lcMDL) << "load_mdl: detected format" << fmt << "for" << fn;

    OBConversion conv;
    if ( !conv.SetInFormat( fmt.toLatin1().constData() ) ) {
        QMessageBox::warning( r, tr( "Open MDL" ),
                              tr( "OpenBabel does not support the '%1' format." ).arg( fmt ) );
        return false;
    }

    std::istringstream iss( data.constData() );
    OBMol obmol;
    bool ok = conv.Read( &obmol, &iss );
    if ( !ok || obmol.NumAtoms() == 0 ) {
        QMessageBox::warning( r, tr( "Open MDL" ),
                              tr( "OpenBabel could not parse the MDL file." ) );
        return false;
    }

    qCDebug(lcMDL) << "load_mdl: read" << obmol.NumAtoms() << "atoms," << obmol.NumBonds() << "bonds";

    // Convert OBMol -> XDrawChem Molecule and add to document
    Molecule *mol = new Molecule( r, this );
    if ( !mol->convertFromOBMol( &obmol ) ) {
        delete mol;
        QMessageBox::warning( r, tr( "Open MDL" ),
                              tr( "Failed to convert the MDL structure to internal format." ) );
        return false;
    }

    addMolecule( mol );
    qCDebug(lcMDL) << "load_mdl: added molecule to drawlist";
    return true;
}

// ProcessMDL is used for headless / programmatic import (e.g. drag-drop
// from external sources or temporary files).  For MDL it behaves the same
// as load_mdl.
bool ChemData::ProcessMDL( QString wf )
{
    return load_mdl( wf );
}

bool ChemData::save_mdl( QString fn )
{
    using namespace OpenBabel;

    // Collect all molecules from the drawlist
    QList<OBMol *> obMols;
    for ( Drawable *tmp_draw : drawlist ) {
        if ( tmp_draw->Type() != TYPE_MOLECULE )
            continue;
        Molecule *mol = static_cast<Molecule *>( tmp_draw );
        OBMol *obmol = mol->convertToOBMol();
        if ( obmol != nullptr && obmol->NumAtoms() > 0 ) {
            obMols.append( obmol );
        } else {
            delete obmol;
        }
    }

    if ( obMols.isEmpty() ) {
        QMessageBox::warning( r, tr( "Save MDL" ),
                              tr( "Nothing to save: no molecules in the document." ) );
        return false;
    }

    qCDebug(lcMDL) << "save_mdl: writing" << obMols.count() << "molecule(s) to" << fn;

    // Determine format from extension
    QString fmt = fn.right( 4 ).toLower() == QLatin1String( ".sdf" )
                      ? QStringLiteral( "sdf" )
                      : QStringLiteral( "mol" );

    OBConversion conv;
    if ( !conv.SetOutFormat( fmt.toLatin1().constData() ) ) {
        for ( OBMol *m : obMols )
            delete m;
        QMessageBox::warning( r, tr( "Save MDL" ),
                              tr( "OpenBabel does not support the '%1' format." ).arg( fmt ) );
        return false;
    }

    // If multiple molecules and we're writing MOL (not SDF), concatenate
    // them with $$$$ separators ourselves.  SDF writer handles this natively.
    std::ostringstream oss;
    bool first = true;
    for ( OBMol *obmol : obMols ) {
        if ( !first )
            oss << "$$$$" << Qt::endl;
        first = false;

        if ( !conv.Write( obmol, &oss ) ) {
            qCWarning(lcMDL) << "save_mdl: OpenBabel failed to write a molecule block";
        }
        delete obmol;
    }

    QFile fout( fn );
    if ( !fout.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        QMessageBox::warning( r, tr( "Save MDL" ),
                              tr( "Could not open '%1' for writing." ).arg( fn ) );
        return false;
    }

    QByteArray outData( oss.str().c_str(), static_cast<int>( oss.str().size() ) );
    fout.write( outData );
    fout.close();

    qCDebug(lcMDL) << "save_mdl: wrote" << outData.size() << "bytes to" << fn;
    return true;
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
