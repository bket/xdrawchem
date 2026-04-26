// tst_sssr.cpp — unit tests for SSSR ring-detection algorithm.
//
// SSSR (Smallest Set of Smallest Rings) is a pure graph algorithm that works
// on QList<DPoint*> where each DPoint has a populated `neighbors` list.
// No Render2D or Qt widget dependencies — tests run headless.
//
// Test topologies:
//   Linear chain      → 0 rings
//   Triangle (3-ring) → 1 ring of size 3
//   Square   (4-ring) → 1 ring of size 4
//   Benzene  (6-ring) → 1 ring of size 6
//   Naphthalene       → 2 fused 6-rings (10 atoms, 11 bonds)
//   Anthracene        → 3 fused 6-rings (14 atoms, 16 bonds)
//   Bicyclo[2.2.1]    → bridged bicycle (camphor-like), 2 rings
//   Spiro[4.4]nonane  → spiro junction, 2 rings of size 5

#include <QtTest>
#include "../xdrawchem/molecule_sssr.h"
#include "../xdrawchem/dpoint.h"

// ── Graph builder helpers ────────────────────────────────────────────────────

// Build N atoms with no bonds
static QList<DPoint *> makeAtoms(int n)
{
    QList<DPoint *> atoms;
    for (int i = 0; i < n; ++i) {
        DPoint *p = new DPoint;
        p->element = "C";
        p->serial  = i;
        atoms.append(p);
    }
    return atoms;
}

// Add a bidirectional edge (undirected graph)
static void addEdge(DPoint *a, DPoint *b)
{
    if (!a->neighbors.contains(b)) a->neighbors.append(b);
    if (!b->neighbors.contains(a)) b->neighbors.append(a);
}

// Build a simple ring: 0→1→2→…→(n-1)→0
static QList<DPoint *> makeRing(int n)
{
    QList<DPoint *> atoms = makeAtoms(n);
    for (int i = 0; i < n; ++i)
        addEdge(atoms[i], atoms[(i + 1) % n]);
    return atoms;
}

// Run SSSR and return ring count
static int ringCount(QList<DPoint *> atoms)
{
    SSSR s;
    s.BuildSSSR(atoms);
    return s.sssr.count();
}

// Return sizes of all detected rings (sorted)
static QList<int> ringSizes(QList<DPoint *> atoms)
{
    SSSR s;
    s.BuildSSSR(atoms);
    QList<int> sizes;
    for (auto *ring : s.sssr)
        sizes.append(ring->size());
    std::sort(sizes.begin(), sizes.end());
    return sizes;
}

// ── Test class ───────────────────────────────────────────────────────────────

class TestSSSR : public QObject
{
    Q_OBJECT

private slots:

    // ── Acyclic structures → 0 rings ────────────────────────────────────────
    void single_atom_no_rings()
    {
        auto atoms = makeAtoms(1);
        QCOMPARE(ringCount(atoms), 0);
    }

    void linear_chain_no_rings()
    {
        // A–B–C–D — no ring
        auto atoms = makeAtoms(4);
        addEdge(atoms[0], atoms[1]);
        addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]);
        QCOMPARE(ringCount(atoms), 0);
    }

    void branched_chain_no_rings()
    {
        // Star: centre connected to 3 terminal atoms
        auto atoms = makeAtoms(4);
        addEdge(atoms[0], atoms[1]);
        addEdge(atoms[0], atoms[2]);
        addEdge(atoms[0], atoms[3]);
        QCOMPARE(ringCount(atoms), 0);
    }

    // ── Simple rings ─────────────────────────────────────────────────────────
    void triangle_is_one_ring_size_3()
    {
        auto atoms = makeRing(3);
        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 1);
        QCOMPARE(sizes[0],     3);
    }

    void square_is_one_ring_size_4()
    {
        auto atoms = makeRing(4);
        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 1);
        QCOMPARE(sizes[0],     4);
    }

    void pentagon_is_one_ring_size_5()
    {
        auto atoms = makeRing(5);
        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 1);
        QCOMPARE(sizes[0],     5);
    }

    void benzene_ring_is_one_ring_size_6()
    {
        auto atoms = makeRing(6);
        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 1);
        QCOMPARE(sizes[0],     6);
    }

    // ── SSSR counts atoms-in-rings correctly ─────────────────────────────────
    void benzene_all_atoms_flagged_inring()
    {
        auto atoms = makeRing(6);
        SSSR s;
        s.BuildSSSR(atoms);
        for (DPoint *pt : atoms)
            QVERIFY2(pt->inring, "All benzene atoms must be flagged in-ring");
    }

    void linear_chain_atoms_not_inring()
    {
        auto atoms = makeAtoms(4);
        addEdge(atoms[0], atoms[1]);
        addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]);
        SSSR s;
        s.BuildSSSR(atoms);
        for (DPoint *pt : atoms)
            QVERIFY2(!pt->inring, "Chain atoms must NOT be flagged in-ring");
    }

    // ── Fused rings (naphthalene topology) ───────────────────────────────────
    // Naphthalene: two 6-rings sharing one bond (2 shared atoms)
    //   0-1-2-3-4-5-0
    //         3-6-7-8-9-4  ← second ring sharing edge 3–4
    void naphthalene_two_rings()
    {
        // Build naphthalene graph (10 atoms, 11 bonds)
        auto atoms = makeAtoms(10);
        // Ring 1: 0-1-2-3-4-5
        addEdge(atoms[0], atoms[1]);
        addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]);
        addEdge(atoms[3], atoms[4]);
        addEdge(atoms[4], atoms[5]);
        addEdge(atoms[5], atoms[0]);
        // Ring 2: shares 3-4 bridge; adds 4-6-7-8-9-3
        addEdge(atoms[4], atoms[6]);
        addEdge(atoms[6], atoms[7]);
        addEdge(atoms[7], atoms[8]);
        addEdge(atoms[8], atoms[9]);
        addEdge(atoms[9], atoms[3]);

        QCOMPARE(ringCount(atoms), 2);
    }

    void naphthalene_both_rings_size_6()
    {
        auto atoms = makeAtoms(10);
        addEdge(atoms[0], atoms[1]); addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]); addEdge(atoms[3], atoms[4]);
        addEdge(atoms[4], atoms[5]); addEdge(atoms[5], atoms[0]);
        addEdge(atoms[4], atoms[6]); addEdge(atoms[6], atoms[7]);
        addEdge(atoms[7], atoms[8]); addEdge(atoms[8], atoms[9]);
        addEdge(atoms[9], atoms[3]);

        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 2);
        QCOMPARE(sizes[0], 6);
        QCOMPARE(sizes[1], 6);
    }

    void naphthalene_shared_atoms_inring_twice()
    {
        auto atoms = makeAtoms(10);
        addEdge(atoms[0], atoms[1]); addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]); addEdge(atoms[3], atoms[4]);
        addEdge(atoms[4], atoms[5]); addEdge(atoms[5], atoms[0]);
        addEdge(atoms[4], atoms[6]); addEdge(atoms[6], atoms[7]);
        addEdge(atoms[7], atoms[8]); addEdge(atoms[8], atoms[9]);
        addEdge(atoms[9], atoms[3]);

        SSSR s;
        s.BuildSSSR(atoms);
        // atoms[3] and atoms[4] are the shared (junction) atoms
        QCOMPARE(s.IsInRing(atoms[3]), 2);  // in both rings
        QCOMPARE(s.IsInRing(atoms[4]), 2);
        QCOMPARE(s.IsInRing(atoms[0]), 1);  // only in ring 1
        QCOMPARE(s.IsInRing(atoms[6]), 1);  // only in ring 2
    }

    // ── InSameRing predicate ─────────────────────────────────────────────────
    void insameRing_3_adjacent_atoms_in_benzene()
    {
        auto atoms = makeRing(6);
        SSSR s;
        s.BuildSSSR(atoms);
        // Three consecutive atoms 0,1,2 are in the same 6-ring
        int ringSize = s.InSameRing(atoms[0], atoms[1], atoms[2]);
        QCOMPARE(ringSize, 6);
    }

    void insameRing_returns_0_for_non_ring_atoms()
    {
        // Chain: no ring
        auto atoms = makeAtoms(4);
        addEdge(atoms[0], atoms[1]);
        addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]);
        SSSR s;
        s.BuildSSSR(atoms);
        QCOMPARE(s.InSameRing(atoms[0], atoms[1], atoms[2]), 0);
    }

    void insameRing_atoms_from_different_rings_returns_0()
    {
        // Naphthalene: atom 0 (ring1 only) + atom 6 (ring2 only) + atom 3 (shared)
        auto atoms = makeAtoms(10);
        addEdge(atoms[0], atoms[1]); addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]); addEdge(atoms[3], atoms[4]);
        addEdge(atoms[4], atoms[5]); addEdge(atoms[5], atoms[0]);
        addEdge(atoms[4], atoms[6]); addEdge(atoms[6], atoms[7]);
        addEdge(atoms[7], atoms[8]); addEdge(atoms[8], atoms[9]);
        addEdge(atoms[9], atoms[3]);
        SSSR s;
        s.BuildSSSR(atoms);
        // 0 is ring-1-only, 6 is ring-2-only — not in the same ring
        QCOMPARE(s.InSameRing(atoms[0], atoms[3], atoms[6]), 0);
    }

    // ── Spiro junction: two 5-rings sharing one atom ─────────────────────────
    // Spiro[4.4]nonane topology:  ring A (0-1-2-3-4), ring B (4-5-6-7-8)
    void spiro_two_rings_size_5()
    {
        auto atoms = makeAtoms(9);
        // Ring A: 0-1-2-3-4-0
        addEdge(atoms[0], atoms[1]); addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]); addEdge(atoms[3], atoms[4]);
        addEdge(atoms[4], atoms[0]);
        // Ring B: 4-5-6-7-8-4
        addEdge(atoms[4], atoms[5]); addEdge(atoms[5], atoms[6]);
        addEdge(atoms[6], atoms[7]); addEdge(atoms[7], atoms[8]);
        addEdge(atoms[8], atoms[4]);

        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 2);
        QCOMPARE(sizes[0], 5);
        QCOMPARE(sizes[1], 5);
    }

    void spiro_atom_is_in_two_rings()
    {
        auto atoms = makeAtoms(9);
        addEdge(atoms[0], atoms[1]); addEdge(atoms[1], atoms[2]);
        addEdge(atoms[2], atoms[3]); addEdge(atoms[3], atoms[4]);
        addEdge(atoms[4], atoms[0]);
        addEdge(atoms[4], atoms[5]); addEdge(atoms[5], atoms[6]);
        addEdge(atoms[6], atoms[7]); addEdge(atoms[7], atoms[8]);
        addEdge(atoms[8], atoms[4]);

        SSSR s;
        s.BuildSSSR(atoms);
        // atoms[4] is the spiro centre — it's in both rings
        QCOMPARE(s.IsInRing(atoms[4]), 2);
        // atoms[0] is only in ring A
        QCOMPARE(s.IsInRing(atoms[0]), 1);
    }

    // ── Anthracene: 3 fused 6-rings ─────────────────────────────────────────
    void anthracene_three_rings()
    {
        // 14 atoms, 16 bonds: three linearly fused 6-membered rings
        auto atoms = makeAtoms(14);
        // Ring 1: 0-1-2-3-4-5
        addEdge(atoms[0],atoms[1]); addEdge(atoms[1],atoms[2]);
        addEdge(atoms[2],atoms[3]); addEdge(atoms[3],atoms[4]);
        addEdge(atoms[4],atoms[5]); addEdge(atoms[5],atoms[0]);
        // Ring 2: 3-6-7-8-9-4 (shares 3-4)
        addEdge(atoms[3],atoms[6]); addEdge(atoms[6],atoms[7]);
        addEdge(atoms[7],atoms[8]); addEdge(atoms[8],atoms[9]);
        addEdge(atoms[9],atoms[4]);
        // Ring 3: 8-10-11-12-13-9 (shares 8-9)
        addEdge(atoms[8],atoms[10]); addEdge(atoms[10],atoms[11]);
        addEdge(atoms[11],atoms[12]); addEdge(atoms[12],atoms[13]);
        addEdge(atoms[13],atoms[9]);

        QCOMPARE(ringCount(atoms), 3);
    }

    void anthracene_all_rings_size_6()
    {
        auto atoms = makeAtoms(14);
        addEdge(atoms[0],atoms[1]); addEdge(atoms[1],atoms[2]);
        addEdge(atoms[2],atoms[3]); addEdge(atoms[3],atoms[4]);
        addEdge(atoms[4],atoms[5]); addEdge(atoms[5],atoms[0]);
        addEdge(atoms[3],atoms[6]); addEdge(atoms[6],atoms[7]);
        addEdge(atoms[7],atoms[8]); addEdge(atoms[8],atoms[9]);
        addEdge(atoms[9],atoms[4]);
        addEdge(atoms[8],atoms[10]); addEdge(atoms[10],atoms[11]);
        addEdge(atoms[11],atoms[12]); addEdge(atoms[12],atoms[13]);
        addEdge(atoms[13],atoms[9]);

        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 3);
        for (int s : sizes)
            QCOMPARE(s, 6);
    }

    // ── Mixed ring sizes ─────────────────────────────────────────────────────
    void fused_5_and_6_ring()
    {
        // Indene topology: one 5-ring fused to one 6-ring (9 atoms, 10 bonds)
        auto atoms = makeAtoms(9);
        // 6-ring: 0-1-2-3-4-5
        addEdge(atoms[0],atoms[1]); addEdge(atoms[1],atoms[2]);
        addEdge(atoms[2],atoms[3]); addEdge(atoms[3],atoms[4]);
        addEdge(atoms[4],atoms[5]); addEdge(atoms[5],atoms[0]);
        // 5-ring: 3-6-7-8-4 (shares edge 3-4)
        addEdge(atoms[3],atoms[6]); addEdge(atoms[6],atoms[7]);
        addEdge(atoms[7],atoms[8]); addEdge(atoms[8],atoms[4]);

        auto sizes = ringSizes(atoms);
        QCOMPARE(sizes.size(), 2);
        QCOMPARE(sizes[0], 5);  // sorted: 5 first
        QCOMPARE(sizes[1], 6);
    }
};

QTEST_APPLESS_MAIN(TestSSSR)
#include "tst_sssr.moc"
