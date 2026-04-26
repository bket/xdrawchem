// tst_valence.cpp — unit tests for Molecule::ValenceErrors()
//
// Builds molecules with known-good and known-bad valences and verifies that
// ValenceErrors() returns the right diagnosis.  No network or display needed.

#include <QtTest>
#include <QColor>

#include "../xdrawchem/molecule.h"
#include "../xdrawchem/dpoint.h"
#include "../xdrawchem/bond.h"

// ── Minimal molecule builder (same pattern as tst_smiles) ────────────────────
struct MB {
    Molecule *mol;
    explicit MB() : mol(new Molecule(nullptr)) {}
    ~MB() { delete mol; }

    DPoint *atom(const QString &el, double x = 0, double y = 0) {
        DPoint *p = new DPoint;
        p->element = el;
        p->x = x;  p->y = y;
        return p;
    }
    void bond(DPoint *a, DPoint *b, int order = 1) {
        mol->addBond(a, b, 1, order, Qt::black, false);
    }
    QStringList errors() { return mol->ValenceErrors(); }
};

class TestValence : public QObject
{
    Q_OBJECT

private slots:

    // ── Valid structures — expect zero errors ────────────────────────────────

    void ethane_valid()
    {
        // C–C: each carbon has 1 bond drawn → well within valence 4
        MB mb;
        DPoint *c1 = mb.atom("C", 10, 10);
        DPoint *c2 = mb.atom("C", 50, 10);
        mb.bond(c1, c2, 1);
        QVERIFY2(mb.errors().isEmpty(),
                 qPrintable("Unexpected errors: " + mb.errors().join("; ")));
    }

    void ethene_double_bond_valid()
    {
        // C=C: each carbon has bond order 2 → still ≤ 4
        MB mb;
        DPoint *c1 = mb.atom("C", 10, 10);
        DPoint *c2 = mb.atom("C", 50, 10);
        mb.bond(c1, c2, 2);
        QVERIFY2(mb.errors().isEmpty(),
                 qPrintable("Unexpected errors: " + mb.errors().join("; ")));
    }

    void ethyne_triple_bond_valid()
    {
        // C≡C: bond order 3, still ≤ 4
        MB mb;
        DPoint *c1 = mb.atom("C", 10, 10);
        DPoint *c2 = mb.atom("C", 50, 10);
        mb.bond(c1, c2, 3);
        QVERIFY2(mb.errors().isEmpty(),
                 qPrintable("Unexpected errors: " + mb.errors().join("; ")));
    }

    void water_valid()
    {
        // O with two single bonds → bond order sum 2 ≤ valence 2
        MB mb;
        DPoint *o  = mb.atom("O", 30, 30);
        DPoint *h1 = mb.atom("H", 10, 10);
        DPoint *h2 = mb.atom("H", 50, 10);
        mb.bond(o, h1, 1);
        mb.bond(o, h2, 1);
        QVERIFY2(mb.errors().isEmpty(),
                 qPrintable("Unexpected errors: " + mb.errors().join("; ")));
    }

    void ammonia_valid()
    {
        // N with three single bonds → 3 ≤ 3
        MB mb;
        DPoint *n  = mb.atom("N", 30, 30);
        DPoint *h1 = mb.atom("H", 10, 10);
        DPoint *h2 = mb.atom("H", 50, 10);
        DPoint *h3 = mb.atom("H", 30, 60);
        mb.bond(n, h1);
        mb.bond(n, h2);
        mb.bond(n, h3);
        QVERIFY2(mb.errors().isEmpty(),
                 qPrintable("Unexpected errors: " + mb.errors().join("; ")));
    }

    // ── Invalid structures — expect at least one error ────────────────────────

    void pentavalent_carbon_detected()
    {
        // Carbon with 5 single bonds → valence sum 5 > max 4
        MB mb;
        DPoint *c  = mb.atom("C", 50, 50);
        for (int i = 0; i < 5; ++i) {
            DPoint *h = mb.atom("H", 10.0 * i, 10);
            mb.bond(c, h, 1);
        }
        QStringList errs = mb.errors();
        QVERIFY2(!errs.isEmpty(), "Pentavalent carbon should be flagged");
        // Error message should mention carbon
        bool mentionsCarbon = errs.join(" ").contains("C", Qt::CaseSensitive);
        QVERIFY2(mentionsCarbon, qPrintable("Error should mention 'C': " + errs.join("; ")));
    }

    void trivalent_oxygen_detected()
    {
        // Oxygen with 3 single bonds → 3 > 2
        MB mb;
        DPoint *o  = mb.atom("O", 50, 50);
        DPoint *h1 = mb.atom("H", 10, 10);
        DPoint *h2 = mb.atom("H", 50, 10);
        DPoint *h3 = mb.atom("H", 90, 10);
        mb.bond(o, h1);
        mb.bond(o, h2);
        mb.bond(o, h3);
        QStringList errs = mb.errors();
        QVERIFY2(!errs.isEmpty(), "Trivalent oxygen should be flagged");
        QVERIFY2(errs.join(" ").contains("O"),
                 qPrintable("Error should mention 'O': " + errs.join("; ")));
    }

    void pentavalent_nitrogen_detected()
    {
        // Nitrogen with 5 single bonds → 5 > 3
        MB mb;
        DPoint *n  = mb.atom("N", 50, 50);
        for (int i = 0; i < 5; ++i) {
            DPoint *h = mb.atom("H", 10.0 * i, 0);
            mb.bond(n, h, 1);
        }
        QStringList errs = mb.errors();
        QVERIFY2(!errs.isEmpty(), "Pentavalent nitrogen should be flagged");
    }

    void double_bond_pushes_over_valence()
    {
        // C with 3 single bonds + 1 double bond = sum 5 > 4
        MB mb;
        DPoint *c  = mb.atom("C", 50, 50);
        DPoint *c2 = mb.atom("C", 90, 50);
        mb.bond(c, c2, 2);  // double bond = 2
        for (int i = 0; i < 3; ++i) {
            DPoint *h = mb.atom("H", 10.0 * i, 0);
            mb.bond(c, h, 1);  // 3 more single bonds
        }
        QStringList errs = mb.errors();
        QVERIFY2(!errs.isEmpty(), "Over-bonded carbon (3+2=5) should be flagged");
    }

    // ── Edge cases ────────────────────────────────────────────────────────────

    void hydrogen_atoms_not_flagged()
    {
        // Explicit H atoms should be skipped by the valence checker
        MB mb;
        DPoint *h  = mb.atom("H", 10, 10);
        DPoint *h2 = mb.atom("H", 50, 10);
        mb.bond(h, h2, 1);
        // H–H is H2, both H atoms have 1 bond — valid
        QVERIFY2(mb.errors().isEmpty(), "H2 should pass valence check");
    }

    void empty_molecule_no_errors()
    {
        MB mb;
        QVERIFY2(mb.errors().isEmpty(), "Empty molecule should have no errors");
    }
};

QTEST_APPLESS_MAIN(TestValence)
#include "tst_valence.moc"
