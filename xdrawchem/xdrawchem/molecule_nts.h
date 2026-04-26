#ifndef MOLECULE_NTS_H
#define MOLECULE_NTS_H

#include <QString>

// Resolve a chemical name (IUPAC, common, CAS) to an isomeric SMILES string
// via the PubChem REST API.  Returns an empty string if the name is not found
// or network is unavailable.  Implemented in molecule_nts.cpp.
QString nameToSMILES( const QString &name );

#endif

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
