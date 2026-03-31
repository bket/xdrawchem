// tst_aromaticity.cpp — unit tests for SSSR::FindAromatic.
//
// FindAromatic marks DPoint::aromatic = true for all atoms in a 6-membered ring
// that has alternating single (order=1) and double (order=2) bonds.
// It takes a QList<Bond*> and uses Bond::Find() / Bond::Order() — no Render2D.
//
// Topologies tested:
//   benzene ring (alternating 1/2) → all 6 atoms aromatic
//   cyclohexane (all single)       → no atoms aromatic
//   naphthalene (two fused 6-rings)→ all 10 ring atoms aromatic
//   5-membered ring                → not marked (FindAromatic only handles 6-rings)
//   isolated chain                 → no atoms aromatic

#include <QtTest>
#include "../xdrawchem/molecule_sssr.h"
#include "../xdrawchem/dpoint.h"
#include "../xdrawchem/bond.h"

// ── Graph builder helpers ─────────────────────────────────────────────────────

static QList<DPoint *> makeAtoms(int n, const QString &elem = "C")
{
    QList<DPoint *> atoms;
    for (int i = 0; i < n; ++i) {
        DPoint *p = new DPoint;
        p->element = elem;
        p->serial  = i;
        atoms.append(p);
    }
    return atoms;
}

static void addEdge(DPoint *a, DPoint *b)
{
    if (!a->neighbors.contains(b)) a->neighbors.append(b);
    if (!b->neighbors.contains(a)) b->neighbors.append(a);
}

// Build a Bond with given order (Render2D* = nullptr — safe, never dereferenced)
static Bond *makeBond(DPoint *s, DPoint *e, int order)
{
    Bond *b = new Bond(nullptr);
    b->setPoints(s, e);
    b->setOrder(order);
    return b;
}

// ── Test class ────────────────────────────────────────────────────────────────

class TestAromaticity : public QObject
{
    Q_OBJECT

private slots:

    // ── Benzene: alternating 1/2 bonds → all atoms aromatic ─────────────────
    void benzene_all_atoms_aromatic()
    {
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;

        // Build 6-ring with alternating bond orders
        for (int i = 0; i < 6; ++i) {
            addEdge(atoms[i], atoms[(i+1)%6]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], (i%2==0) ? 1 : 2));
        }

        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(pt->aromatic, "All benzene atoms must be aromatic");
    }

    void benzene_inring_and_aromatic_both_set()
    {
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;
        for (int i = 0; i < 6; ++i) {
            addEdge(atoms[i], atoms[(i+1)%6]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], (i%2==0) ? 1 : 2));
        }
        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms) {
            QVERIFY2(pt->inring,   "Benzene atom must be inring");
            QVERIFY2(pt->aromatic, "Benzene atom must be aromatic");
        }
    }

    // ── Cyclohexane: all single bonds → no atoms aromatic ────────────────────
    void cyclohexane_no_aromatic_atoms()
    {
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;

        for (int i = 0; i < 6; ++i) {
            addEdge(atoms[i], atoms[(i+1)%6]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], 1));
        }

        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(!pt->aromatic, "Cyclohexane atoms must NOT be aromatic");
    }

    void cyclohexane_still_inring()
    {
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;
        for (int i = 0; i < 6; ++i) {
            addEdge(atoms[i], atoms[(i+1)%6]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], 1));
        }
        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(pt->inring, "Cyclohexane atoms must still be inring");
    }

    // ── All-double-bond ring is NOT aromatic (no single bonds) ───────────────
    void all_double_bonds_not_aromatic()
    {
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;
        for (int i = 0; i < 6; ++i) {
            addEdge(atoms[i], atoms[(i+1)%6]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], 2));
        }
        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        // FindAromatic requires BOTH o1 (single) AND o2 (double) at each atom
        for (DPoint *pt : atoms)
            QVERIFY2(!pt->aromatic, "All-double ring must NOT be marked aromatic");
    }

    // ── 5-membered ring: not touched by FindAromatic (only handles size 6) ───
    void cyclopentadiene_not_aromatic()
    {
        // Even a 5-ring with alternating bonds — FindAromatic skips non-6-rings
        auto atoms = makeAtoms(5);
        QList<Bond *> bonds;
        for (int i = 0; i < 5; ++i) {
            addEdge(atoms[i], atoms[(i+1)%5]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%5], (i%2==0) ? 1 : 2));
        }
        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(!pt->aromatic, "5-ring must NOT be marked aromatic by FindAromatic");
    }

    // ── Linear chain: no ring → no aromatic atoms ────────────────────────────
    void chain_not_aromatic()
    {
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;
        for (int i = 0; i < 5; ++i) {
            addEdge(atoms[i], atoms[i+1]);
            bonds.append(makeBond(atoms[i], atoms[i+1], (i%2==0) ? 1 : 2));
        }
        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(!pt->aromatic, "Chain atoms must NOT be aromatic");
    }

    // ── Naphthalene: two fused aromatic 6-rings → all 10 atoms aromatic ──────
    void naphthalene_all_atoms_aromatic()
    {
        // 10 atoms, 11 bonds: rings 0-1-2-3-4-5 and 3-6-7-8-9-4
        auto atoms = makeAtoms(10);
        QList<Bond *> bonds;

        // Ring 1: 0-1-2-3-4-5-0 with alternating 1/2
        int orders1[] = {1,2,1,2,1,2};
        int a1arr[] = {0,1,2,3,4,5};
        int b1arr[] = {1,2,3,4,5,0};
        for (int i = 0; i < 6; ++i) {
            int a = a1arr[i];
            int b = b1arr[i];
            addEdge(atoms[a], atoms[b]);
            bonds.append(makeBond(atoms[a], atoms[b], orders1[i]));
        }
        // Ring 2: 3-6-7-8-9-4 (shares edge 3-4)
        // Use orders to ensure each atom in ring 2 also has both single+double
        int a2[] = {3,6,7,8,9};
        int b2[] = {6,7,8,9,4};
        int ord2[] = {1,2,1,2,1};
        for (int i = 0; i < 5; ++i) {
            addEdge(atoms[a2[i]], atoms[b2[i]]);
            bonds.append(makeBond(atoms[a2[i]], atoms[b2[i]], ord2[i]));
        }

        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(pt->aromatic, "All naphthalene atoms must be aromatic");
    }

    // ── Aromaticity is atom-level, not ring-level ─────────────────────────────
    void aromatic_flag_set_per_atom()
    {
        // Non-ring atoms attached to aromatic ring are NOT marked aromatic
        // Build benzene (0-5) with an extra chain atom 6 attached to atom 0
        auto atoms = makeAtoms(7);
        QList<Bond *> bonds;

        for (int i = 0; i < 6; ++i) {
            addEdge(atoms[i], atoms[(i+1)%6]);
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], (i%2==0) ? 1 : 2));
        }
        // Pendant atom 6 attached to atom 0 with single bond
        bonds.append(makeBond(atoms[0], atoms[6], 1));

        SSSR s;
        s.BuildSSSR(atoms);
        s.FindAromatic(bonds);

        // Ring atoms should be aromatic
        for (int i = 0; i < 6; ++i)
            QVERIFY2(atoms[i]->aromatic, "Ring atom must be aromatic");

        // Pendant atom should NOT be aromatic
        QVERIFY2(!atoms[6]->aromatic, "Pendant chain atom must NOT be aromatic");
    }

    // ── Empty SSSR → nothing aromatic ────────────────────────────────────────
    void empty_sssr_nothing_aromatic()
    {
        // Call FindAromatic on a fresh SSSR with no rings built
        auto atoms = makeAtoms(6);
        QList<Bond *> bonds;
        for (int i = 0; i < 6; ++i)
            bonds.append(makeBond(atoms[i], atoms[(i+1)%6], (i%2==0) ? 1 : 2));

        SSSR s;  // no BuildSSSR call
        s.FindAromatic(bonds);

        for (DPoint *pt : atoms)
            QVERIFY2(!pt->aromatic, "Atoms with no SSSR must NOT be aromatic");
    }
};

QTEST_APPLESS_MAIN(TestAromaticity)
#include "tst_aromaticity.moc"
