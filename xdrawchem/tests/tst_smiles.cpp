// tst_smiles.cpp — SMILES round-trip tests for Molecule::ToSMILES / FromSMILES.
//
// Strategy: build Molecule objects by manually constructing DPoint atoms and
// Bond objects (with Render2D* = nullptr — safe since Render2D is never
// dereferenced in setPoints/setOrder/addBond), then call ToSMILES() and compare
// against canonical SMILES produced by OpenBabel for the same connectivity.
//
// ToSMILES() converts via MDL Molfile → OpenBabel → SMILES string.
// FromSMILES() converts SMILES → OpenBabel → DPoint/Bond graph.
//
// Canonical SMILES verified against OBConversion("mol","smi"):
//   ethane    → "CC"
//   propane   → "CCC"
//   benzene   → "c1ccccc1"       (aromatic kekulé)
//   cyclohexane → "C1CCCCC1"

#include <QtTest>
#include <QColor>

#include "../xdrawchem/molecule.h"
#include "../xdrawchem/dpoint.h"
#include "../xdrawchem/bond.h"

// ── Molecule builder helper ──────────────────────────────────────────────────

struct MolBuilder {
    Molecule *mol;
    QList<DPoint *> atoms;

    explicit MolBuilder() : mol(new Molecule(nullptr)) {}

    // Add a carbon atom at (x, y) — returns the new DPoint
    DPoint *addCarbon(double x, double y) {
        return addAtom("C", x, y);
    }

    DPoint *addAtom(const QString &elem, double x, double y) {
        DPoint *pt = new DPoint;
        pt->element = elem;
        pt->x = x;
        pt->y = y;
        atoms.append(pt);
        return pt;
    }

    // Add a bond between two atoms
    void bond(DPoint *a, DPoint *b, int order = 1) {
        mol->addBond(a, b, 1, order, Qt::black, false);
    }

    QString smiles() { return mol->ToSMILES().trimmed(); }

    ~MolBuilder() { delete mol; }
};

// ── Test class ───────────────────────────────────────────────────────────────

class TestSmiles : public QObject
{
    Q_OBJECT

private slots:
    // ── Methane: single atom, no bonds ──────────────────────────────────────
    // OB produces "C" for a single carbon
    void methane_single_carbon()
    {
        MolBuilder mb;
        // No bonds — molecule has no atoms via AllPoints() (bond-based).
        // Single isolated atom won't appear in MDL molfile since AllPoints
        // only iterates bonds. Verify ToSMILES at least doesn't crash.
        mb.addCarbon(50.0, 100.0);
        QString s = mb.mol->ToSMILES().trimmed();
        // Empty or no-atom result — main thing is no crash
        QVERIFY2(!s.isNull(), "ToSMILES should not return null");
    }

    // ── Ethane: C–C ─────────────────────────────────────────────────────────
    void ethane_smiles_is_CC()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(100.0, 200.0);
        DPoint *c2 = mb.addCarbon(125.0, 200.0);
        mb.bond(c1, c2, 1);

        QString s = mb.smiles();
        QCOMPARE(s, QString("CC"));
    }

    void ethane_canonical_not_empty()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        mb.bond(c1, c2);
        QVERIFY(!mb.smiles().isEmpty());
    }

    // ── Propane: C–C–C ──────────────────────────────────────────────────────
    void propane_smiles_is_CCC()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        DPoint *c3 = mb.addCarbon(53.0, 100.0);
        mb.bond(c1, c2); mb.bond(c2, c3);

        QCOMPARE(mb.smiles(), QString("CCC"));
    }

    // ── Benzene: 6-ring alternating single/double ────────────────────────────
    void benzene_smiles_is_aromatic()
    {
        // Regular hexagon coords, alternating bond orders 1/2
        MolBuilder mb;
        const double r = 1.0;
        QList<DPoint *> pts;
        for (int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3.0;
            pts.append(mb.addCarbon(r * cos(angle), r * sin(angle)));
        }
        for (int i = 0; i < 6; ++i)
            mb.bond(pts[i], pts[(i+1)%6], (i % 2 == 0) ? 1 : 2);

        // OB recognises the Kekulé pattern and produces aromatic SMILES
        QString s = mb.smiles();
        QCOMPARE(s, QString("c1ccccc1"));
    }

    // ── Cyclohexane: 6-ring all single bonds ─────────────────────────────────
    void cyclohexane_smiles_is_C1CCCCC1()
    {
        MolBuilder mb;
        const double r = 1.0;
        QList<DPoint *> pts;
        for (int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3.0;
            pts.append(mb.addCarbon(r * cos(angle), r * sin(angle)));
        }
        for (int i = 0; i < 6; ++i)
            mb.bond(pts[i], pts[(i+1)%6], 1);

        QCOMPARE(mb.smiles(), QString("C1CCCCC1"));
    }

    // ── Double bond: ethylene C=C ────────────────────────────────────────────
    void ethylene_has_double_bond_in_smiles()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        mb.bond(c1, c2, 2);

        QString s = mb.smiles();
        QVERIFY2(s.contains("="), "Ethylene SMILES must contain '='");
    }

    void ethylene_smiles_is_C_eq_C()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        mb.bond(c1, c2, 2);

        QCOMPARE(mb.smiles(), QString("C=C"));
    }

    // ── Triple bond: acetylene C≡C ───────────────────────────────────────────
    void acetylene_smiles_has_triple_bond()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        mb.bond(c1, c2, 3);

        QString s = mb.smiles();
        QVERIFY2(s.contains("#"), "Acetylene SMILES must contain '#'");
    }

    void acetylene_smiles_is_C_hash_C()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        mb.bond(c1, c2, 3);

        QCOMPARE(mb.smiles(), QString("C#C"));
    }

    // ── SMILES round-trip: parse SMILES and check atom/bond counts ───────────
    void fromSMILES_ethane_two_atoms()
    {
        Molecule m(nullptr);
        m.FromSMILES("CC");
        // AllPoints() collects atoms from bond endpoints
        QList<DPoint *> pts = m.AllPoints();
        QCOMPARE(pts.size(), 2);
    }

    void fromSMILES_propane_three_atoms()
    {
        Molecule m(nullptr);
        m.FromSMILES("CCC");
        QCOMPARE(m.AllPoints().size(), 3);
    }

    void fromSMILES_benzene_six_atoms()
    {
        Molecule m(nullptr);
        m.FromSMILES("c1ccccc1");
        QCOMPARE(m.AllPoints().size(), 6);
    }

    void fromSMILES_cyclohexane_six_atoms()
    {
        Molecule m(nullptr);
        m.FromSMILES("C1CCCCC1");
        QCOMPARE(m.AllPoints().size(), 6);
    }

    void fromSMILES_benzene_all_carbons()
    {
        Molecule m(nullptr);
        m.FromSMILES("c1ccccc1");
        for (DPoint *pt : m.AllPoints())
            QCOMPARE(pt->baseElement(), QString("C"));
    }

    void fromSMILES_ethane_one_bond()
    {
        Molecule m(nullptr);
        m.FromSMILES("CC");
        // bonds list on Molecule
        QCOMPARE(m.Members(), 1);
    }

    void fromSMILES_benzene_six_bonds()
    {
        Molecule m(nullptr);
        m.FromSMILES("c1ccccc1");
        QCOMPARE(m.Members(), 6);
    }

    // ── Round-trip: build → ToSMILES → FromSMILES → check atom count ─────────
    void roundtrip_ethane_atom_count_preserved()
    {
        // Build ethane, get SMILES, parse back, verify 2 atoms
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        mb.bond(c1, c2);

        QString s = mb.smiles();
        QVERIFY(!s.isEmpty());

        Molecule m2(nullptr);
        m2.FromSMILES(s);
        QCOMPARE(m2.AllPoints().size(), 2);
    }

    void roundtrip_propane_atom_count_preserved()
    {
        MolBuilder mb;
        DPoint *c1 = mb.addCarbon(50.0, 100.0);
        DPoint *c2 = mb.addCarbon(51.5, 100.0);
        DPoint *c3 = mb.addCarbon(53.0, 100.0);
        mb.bond(c1, c2); mb.bond(c2, c3);

        Molecule m2(nullptr);
        m2.FromSMILES(mb.smiles());
        QCOMPARE(m2.AllPoints().size(), 3);
    }

    void roundtrip_benzene_atom_count_preserved()
    {
        MolBuilder mb;
        const double r = 1.0;
        QList<DPoint *> pts;
        for (int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3.0;
            pts.append(mb.addCarbon(r * cos(angle), r * sin(angle)));
        }
        for (int i = 0; i < 6; ++i)
            mb.bond(pts[i], pts[(i+1)%6], (i%2==0) ? 1 : 2);

        Molecule m2(nullptr);
        m2.FromSMILES(mb.smiles());
        QCOMPARE(m2.AllPoints().size(), 6);
    }

    void roundtrip_cyclohexane_bond_count_preserved()
    {
        MolBuilder mb;
        const double r = 1.0;
        QList<DPoint *> pts;
        for (int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3.0;
            pts.append(mb.addCarbon(r * cos(angle), r * sin(angle)));
        }
        for (int i = 0; i < 6; ++i) mb.bond(pts[i], pts[(i+1)%6]);

        Molecule m2(nullptr);
        m2.FromSMILES(mb.smiles());
        QCOMPARE(m2.Members(), 6);
    }
};

QTEST_MAIN(TestSmiles)   // QApplication needed for QTextDocument in ToMDLMolfile
#include "tst_smiles.moc"
