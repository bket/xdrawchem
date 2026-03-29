// molecule_valence.cpp — valence validation for drawn molecules
//
// Checks each atom's actual bond order sum against the expected maximum
// valence from MolData::Bonds().  Flags atoms that exceed their normal
// valence as errors (e.g. a carbon with 5 bonds).

#include <QtDebug>

#include "molecule.h"
#include "moldata.h"
#include "defs.h"

// ValenceErrors(): return a list of human-readable error strings for atoms
// whose total bond order exceeds the expected normal valence.
// An empty list means the molecule passes all valence checks.
QStringList Molecule::ValenceErrors()
{
    QStringList errors;

    // Build a map from atom -> sum of bond orders
    QList<DPoint *> pts = AllPoints();

    for ( DPoint *atom : pts ) {
        if ( atom->element.isEmpty() )
            continue;

        // Skip explicit hydrogens (they're drawn separately and always valid)
        if ( atom->element.toUpper() == "H" )
            continue;

        int bondOrderSum = 0;
        for ( Bond *b : bonds ) {
            if ( b->Find( atom ) ) {
                int ord = b->Order();
                // Aromatic bonds count as 1.5; round up to 2 for worst-case check
                if ( ord == 4 )
                    ord = 2;
                bondOrderSum += ord;
            }
        }

        int maxValence = MolData::Hydrogens( atom->element );
        if ( maxValence <= 0 )
            continue;   // unknown element — skip

        if ( bondOrderSum > maxValence ) {
            errors.append( QObject::tr( "%1 atom has %2 bonds (maximum is %3)" )
                           .arg( atom->element )
                           .arg( bondOrderSum )
                           .arg( maxValence ) );
        }
    }
    return errors;
}
