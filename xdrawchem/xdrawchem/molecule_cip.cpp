// molecule_cip.cpp — CIP R/S and E/Z stereochemistry label calculation and rendering
//
// Uses OpenBabel's stereochemistry perception (OBStereoFacade) to compute
// CIP descriptors for:
//   - Tetrahedral (R/S) stereocenters
//   - Cis/trans (E/Z) double bonds
//
// Labels are computed on demand and cached in QMaps keyed by DPoint/Bond
// pointers so repeated rendering is cheap.

#include "ob_compat.h"
OB_COMPAT_BEGIN
#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <openbabel/stereo/stereo.h>
#include <openbabel/stereo/tetrahedral.h>
#include <openbabel/stereo/cistrans.h>
OB_COMPAT_END

#include <QMap>
#include <QPainter>

#include "molecule.h"
#include "render2d.h"
#include "bond.h"
#include "dpoint.h"
#include "xdc_logging.h"

// Static caches so labels persist across render passes without recomputation.
// Cleared whenever the molecule structure changes (see Molecule::Changed()).
static QMap<DPoint*, QString> s_cipPointLabels;
static QMap<Bond*, QString>   s_cipBondLabels;
static bool s_cipLabelsValid = false;

void Molecule::ClearCIPCache()
{
    s_cipPointLabels.clear();
    s_cipBondLabels.clear();
    s_cipLabelsValid = false;
}

void Molecule::CalcCIPLabels()
{
    if ( s_cipLabelsValid )
        return;

    s_cipPointLabels.clear();
    s_cipBondLabels.clear();

    // Need an OBMol to run stereo perception.  convertToOBMol() builds one
    // from the current molecule geometry.  This is a relatively expensive
    // call, which is why we cache the results.
    OpenBabel::OBMol *obmol = convertToOBMol();
    if ( !obmol || obmol->NumAtoms() == 0 ) {
        delete obmol;
        s_cipLabelsValid = true;
        return;
    }

    // Build a map from OB atom index (1-based) → DPoint* so we can route
    // results back to our internal data structures.
    QList<DPoint*> allPoints = AllPoints();
    QMap<unsigned long, DPoint*> obAtomToPoint;

    // convertToOBMol() adds atoms in the same order as AllPoints().
    for ( int i = 0; i < allPoints.count() && i < (int)obmol->NumAtoms(); ++i ) {
        OpenBabel::OBAtom *obAtom = obmol->GetAtom( i + 1 ); // OB is 1-based
        if ( obAtom )
            obAtomToPoint.insert( obAtom->GetId(), allPoints[i] );
    }

    // Run stereo perception
    OpenBabel::OBStereoFacade facade( obmol, true ); // true = call PerceiveStereo

    // --- Tetrahedral (R/S) ---
    std::vector<OpenBabel::OBTetrahedralStereo*> tetraList = facade.GetAllTetrahedralStereo();
    for ( OpenBabel::OBTetrahedralStereo *tet : tetraList ) {
        if ( !tet )
            continue;

        OpenBabel::OBTetrahedralStereo::Config cfg = tet->GetConfig();
        if ( !cfg.specified )
            continue;

        DPoint *centerPt = obAtomToPoint.value( cfg.center, nullptr );
        if ( !centerPt )
            continue;

        // Determine R/S from winding and view direction.
        // Clockwise + ViewFrom  → R
        // AntiClockwise + ViewFrom → S
        // The inverse is true for ViewTowards, but OB normalizes these
        // for us in GetConfig().  We just use the returned winding.
        QString label;
        if ( cfg.winding == OpenBabel::OBStereo::Clockwise )
            label = QStringLiteral( "R" );
        else if ( cfg.winding == OpenBabel::OBStereo::AntiClockwise )
            label = QStringLiteral( "S" );
        else
            label = QString(); // UnknownWinding

        if ( !label.isEmpty() )
            s_cipPointLabels.insert( centerPt, label );
    }

    // --- Cis/Trans (E/Z) ---
    std::vector<OpenBabel::OBCisTransStereo*> ctList = facade.GetAllCisTransStereo();
    for ( OpenBabel::OBCisTransStereo *ct : ctList ) {
        if ( !ct )
            continue;

        OpenBabel::OBCisTransStereo::Config cfg = ct->GetConfig();
        if ( !cfg.specified )
            continue;

        // cfg.begin / cfg.end are the atom IDs of the double-bond atoms.
        DPoint *beginPt = obAtomToPoint.value( cfg.begin, nullptr );
        DPoint *endPt   = obAtomToPoint.value( cfg.end,   nullptr );
        if ( !beginPt || !endPt )
            continue;

        // Find the bond connecting these two points.
        for ( Bond *tmp_bond : bonds ) {
            if ( ( tmp_bond->Start() == beginPt && tmp_bond->End() == endPt ) ||
                 ( tmp_bond->Start() == endPt   && tmp_bond->End() == beginPt ) ) {
                // E = trans, Z = cis
                // OpenBabel uses ShapeU/ShapeZ to encode this.
                QString label;
                if ( cfg.shape == OpenBabel::OBStereo::ShapeZ )
                    label = QStringLiteral( "Z" );
                else
                    label = QStringLiteral( "E" );

                s_cipBondLabels.insert( tmp_bond, label );
                break;
            }
        }
    }

    delete obmol;
    s_cipLabelsValid = true;
}

void Molecule::DrawCIPLabels()
{
    if ( !s_cipLabelsValid )
        CalcCIPLabels();

    if ( s_cipPointLabels.isEmpty() && s_cipBondLabels.isEmpty() )
        return;

    QFont labelFont( QStringLiteral( "Helvetica" ), 10, QFont::Bold );
    QColor labelColor( 0, 100, 200 ); // distinct blue

    // Draw R/S next to chiral centers
    for ( auto it = s_cipPointLabels.constBegin(); it != s_cipPointLabels.constEnd(); ++it ) {
        DPoint *pt = it.key();
        QString label = it.value();

        // Place label slightly above and to the right of the atom.
        QPoint drawPos( (int)pt->x + 8, (int)pt->y - 8 );
        r->drawString( label, drawPos, labelColor, labelFont );
    }

    // Draw E/Z at midpoint of double bonds
    for ( auto it = s_cipBondLabels.constBegin(); it != s_cipBondLabels.constEnd(); ++it ) {
        Bond *b = it.key();
        QString label = it.value();

        // Midpoint of bond, offset slightly to avoid overlapping the bond line.
        QPoint mid = b->midpoint().toPointF().toPoint();
        mid.rx() += 6;
        mid.ry() -= 6;
        r->drawString( label, mid, labelColor, labelFont );
    }
}

QString Molecule::GetCIPLabelForPoint( DPoint *dp )
{
    if ( !s_cipLabelsValid )
        CalcCIPLabels();
    return s_cipPointLabels.value( dp, QString() );
}

QString Molecule::GetCIPLabelForBond( Bond *b )
{
    if ( !s_cipLabelsValid )
        CalcCIPLabels();
    return s_cipBondLabels.value( b, QString() );
}
