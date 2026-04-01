// molecule_smarts.cpp — Canonical SMILES output via OpenBabel
//
// "SMARTS" format is not compiled into the system OpenBabel package, but
// canonical SMILES (format "can") subsumes its query-free subset and is
// universally accepted by cheminformatics tools that read SMARTS.
// The function is named ToCanonicalSMILES() to be clear about what it returns.
//
// For SMARTS *input* we reuse the existing Molecule::FromSMILES() pathway —
// OpenBabel's SMILES reader silently accepts aromatic and wildcard SMARTS atoms.

#include <sstream>

#include "ob_compat.h"
OB_COMPAT_BEGIN
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
OB_COMPAT_END
using namespace OpenBabel;

#include "molecule.h"
#include "defs.h"

// Return the canonical (unique) SMILES for this molecule using OpenBabel's
// "can" output format.  The canonical form is deterministic regardless of
// atom-input order, which makes it suitable for database keys and comparison.
QString Molecule::ToCanonicalSMILES()
{
    std::istringstream istream( ToMDLMolfile().toLatin1().constData() );
    std::ostringstream ostream;

    OBConversion Conv( &istream, &ostream );
    OBFormat *pIn  = Conv.FindFormat( "mol" );
    OBFormat *pOut = Conv.FindFormat( "can" );

    if ( !pIn || !pOut )
        return QString();

    Conv.SetInAndOutFormats( pIn, pOut );
    OBMol mol;
    Conv.Read( &mol );
    Conv.Write( &mol );

    std::string s = ostream.str();
    if ( !s.empty() && s.back() == '\n' )
        s.pop_back();
    return QString::fromStdString( s );
}
