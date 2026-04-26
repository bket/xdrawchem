// tst_bond_enthalpy.cpp — unit tests for Bond::Enthalpy(), Bond::Length(),
// Molecule::OrderOfBond(), and Molecule::SumBondEnthalpy().
//
// Bond::Enthalpy() returns standard bond dissociation energy (kJ/mol) for
// common element-element bond pairs. Bond::Length() returns the Euclidean
// distance between start and end DPoint coordinates.
//
// These use Bond(nullptr) — Render2D* is stored but never dereferenced
// in Enthalpy(), Length(), setPoints(), or setOrder().

#include <QtTest>
#include <cmath>
#include <QColor>

#include "../xdrawchem/bond.h"
#include "../xdrawchem/dpoint.h"
#include "../xdrawchem/molecule.h"

// ── Helper ────────────────────────────────────────────────────────────────────
static Bond *makeBond(const QString &e1, const QString &e2, int order,
                      double x1=0, double y1=0, double x2=50, double y2=0)
{
    DPoint *s = new DPoint; s->element = e1; s->x = x1; s->y = y1;
    DPoint *e = new DPoint; e->element = e2; e->x = x2; e->y = y2;
    Bond *b = new Bond(nullptr);
    b->setPoints(s, e);
    b->setOrder(order);
    return b;
}

// ── Test class ───────────────────────────────────────────────────────────────

class TestBondEnthalpy : public QObject
{
    Q_OBJECT

private slots:

    // ── Single bonds — carbon family ─────────────────────────────────────────
    void cc_single_is_348()
    {
        QCOMPARE(makeBond("C","C",1)->Enthalpy(), 348.0);
    }

    void ch_single_is_413()
    {
        QCOMPARE(makeBond("C","H",1)->Enthalpy(), 413.0);
    }

    void cn_single_is_292()
    {
        QCOMPARE(makeBond("C","N",1)->Enthalpy(), 292.0);
    }

    void co_single_is_351()
    {
        QCOMPARE(makeBond("C","O",1)->Enthalpy(), 351.0);
    }

    void cf_single_is_441()
    {
        QCOMPARE(makeBond("C","F",1)->Enthalpy(), 441.0);
    }

    void ccl_single_is_328()
    {
        QCOMPARE(makeBond("C","Cl",1)->Enthalpy(), 328.0);
    }

    void cbr_single_is_276()
    {
        QCOMPARE(makeBond("C","Br",1)->Enthalpy(), 276.0);
    }

    void ci_single_is_240()
    {
        QCOMPARE(makeBond("C","I",1)->Enthalpy(), 240.0);
    }

    void cs_single_is_259()
    {
        QCOMPARE(makeBond("C","S",1)->Enthalpy(), 259.0);
    }

    // ── H–X single bonds ─────────────────────────────────────────────────────
    void hh_single_is_436()
    {
        QCOMPARE(makeBond("H","H",1)->Enthalpy(), 436.0);
    }

    void hn_single_is_391()
    {
        QCOMPARE(makeBond("H","N",1)->Enthalpy(), 391.0);
    }

    void ho_single_is_463()
    {
        QCOMPARE(makeBond("H","O",1)->Enthalpy(), 463.0);
    }

    void hf_single_is_563()
    {
        QCOMPARE(makeBond("H","F",1)->Enthalpy(), 563.0);
    }

    void hcl_single_is_432()
    {
        QCOMPARE(makeBond("H","Cl",1)->Enthalpy(), 432.0);
    }

    void hbr_single_is_366()
    {
        QCOMPARE(makeBond("H","Br",1)->Enthalpy(), 366.0);
    }

    void hi_single_is_299()
    {
        QCOMPARE(makeBond("H","I",1)->Enthalpy(), 299.0);
    }

    void hs_single_is_339()
    {
        QCOMPARE(makeBond("H","S",1)->Enthalpy(), 339.0);
    }

    void oo_single_is_139()
    {
        QCOMPARE(makeBond("O","O",1)->Enthalpy(), 139.0);
    }

    // ── Double bonds ──────────────────────────────────────────────────────────
    void cc_double_is_615()
    {
        QCOMPARE(makeBond("C","C",2)->Enthalpy(), 615.0);
    }

    void co_double_is_728()
    {
        QCOMPARE(makeBond("C","O",2)->Enthalpy(), 728.0);
    }

    void cn_double_is_615()
    {
        QCOMPARE(makeBond("C","N",2)->Enthalpy(), 615.0);
    }

    void cs_double_is_477()
    {
        QCOMPARE(makeBond("C","S",2)->Enthalpy(), 477.0);
    }

    void nn_double_is_418()
    {
        QCOMPARE(makeBond("N","N",2)->Enthalpy(), 418.0);
    }

    void oo_double_is_498()
    {
        QCOMPARE(makeBond("O","O",2)->Enthalpy(), 498.0);
    }

    // ── Triple bonds ──────────────────────────────────────────────────────────
    void cc_triple_is_812()
    {
        QCOMPARE(makeBond("C","C",3)->Enthalpy(), 812.0);
    }

    void cn_triple_is_891()
    {
        QCOMPARE(makeBond("C","N",3)->Enthalpy(), 891.0);
    }

    void nn_triple_is_945()
    {
        QCOMPARE(makeBond("N","N",3)->Enthalpy(), 945.0);
    }

    // ── Enthalpy is symmetric: element order doesn't matter ──────────────────
    void enthalpy_symmetric_CH()
    {
        QCOMPARE(makeBond("C","H",1)->Enthalpy(), makeBond("H","C",1)->Enthalpy());
    }

    void enthalpy_symmetric_CO()
    {
        QCOMPARE(makeBond("C","O",2)->Enthalpy(), makeBond("O","C",2)->Enthalpy());
    }

    // ── Stereo bonds (order 5=wedge-up, 7=hatch) treated as single ───────────
    void stereo_up_same_as_single()
    {
        QCOMPARE(makeBond("C","C",5)->Enthalpy(), makeBond("C","C",1)->Enthalpy());
    }

    void stereo_down_same_as_single()
    {
        QCOMPARE(makeBond("C","C",7)->Enthalpy(), makeBond("C","C",1)->Enthalpy());
    }

    // ── Unknown bond type returns 0.0 ────────────────────────────────────────
    void unknown_element_pair_returns_zero()
    {
        // "Xx"–"Xx" has no entry in the table → 0.0
        QCOMPARE(makeBond("Xx","Xx",1)->Enthalpy(), 0.0);
    }

    // ── OH functional group handling ─────────────────────────────────────────
    void oh_adds_463_then_uses_O_enthalpy()
    {
        // OH–H: 463 (OH→O conversion) + 463 (H-O bond) = 926
        // Actually OH is treated as element "OH", converts to O then adds 463
        // so OH-H should give 463 (the OH→O special case adds 463, then H-O = 463)
        Bond *b = makeBond("OH","H",1);
        double dh = b->Enthalpy();
        // OH special case: dh += 463, then use atom1="O"/"H" → H-O = 463 → total 926
        QCOMPARE(dh, 926.0);
    }

    // ── Bond::Length() — Euclidean distance ──────────────────────────────────
    void length_horizontal_50_units()
    {
        Bond *b = makeBond("C","C",1, 0,0, 50,0);
        QCOMPARE(b->Length(), 50.0);
    }

    void length_vertical_30_units()
    {
        Bond *b = makeBond("C","C",1, 0,0, 0,30);
        QCOMPARE(b->Length(), 30.0);
    }

    void length_3_4_5_triangle()
    {
        Bond *b = makeBond("C","C",1, 0,0, 30,40);
        QCOMPARE(b->Length(), 50.0);
    }

    void length_zero_for_coincident_points()
    {
        Bond *b = makeBond("C","C",1, 10,10, 10,10);
        QCOMPARE(b->Length(), 0.0);
    }

    void length_is_positive_always()
    {
        Bond *b = makeBond("C","C",1, 50,0, 0,0);  // reversed direction
        QVERIFY(b->Length() > 0.0);
        QCOMPARE(b->Length(), 50.0);
    }

    // ── Molecule::OrderOfBond ─────────────────────────────────────────────────
    void orderOfBond_single()
    {
        Molecule m(nullptr);
        DPoint *a = new DPoint; a->element = "C"; a->x = 50; a->y = 100;
        DPoint *b = new DPoint; b->element = "C"; b->x = 75; b->y = 100;
        m.addBond(a, b, 1, 1, Qt::black, false);
        QCOMPARE(m.OrderOfBond(a, b), 1);
    }

    void orderOfBond_double()
    {
        Molecule m(nullptr);
        DPoint *a = new DPoint; a->element = "C"; a->x = 50; a->y = 100;
        DPoint *b = new DPoint; b->element = "O"; b->x = 75; b->y = 100;
        m.addBond(a, b, 1, 2, Qt::black, false);
        QCOMPARE(m.OrderOfBond(a, b), 2);
    }

    void orderOfBond_not_bonded_returns_0()
    {
        Molecule m(nullptr);
        DPoint *a = new DPoint; a->element = "C"; a->x = 50; a->y = 100;
        DPoint *b = new DPoint; b->element = "C"; b->x = 75; b->y = 100;
        DPoint *c = new DPoint; c->element = "C"; c->x = 100; c->y = 100;
        m.addBond(a, b, 1, 1, Qt::black, false);
        // a–c not bonded
        QCOMPARE(m.OrderOfBond(a, c), 0);
    }

    // ── Molecule::SumBondEnthalpy ─────────────────────────────────────────────
    void sumEnthalpy_ethane_CC_plus_6CH()
    {
        // Ethane: 1 C-C (348) + 6 C-H (413) = 2826 kJ/mol
        Molecule m(nullptr);
        DPoint *c1 = new DPoint; c1->element = "C"; c1->x = 50; c1->y = 100;
        DPoint *c2 = new DPoint; c2->element = "C"; c2->x = 75; c2->y = 100;
        m.addBond(c1, c2, 1, 1, Qt::black, false);

        // Add 3 H's to each C
        for (int i = 0; i < 3; ++i) {
            DPoint *h1 = new DPoint; h1->element = "H"; h1->x = i*5; h1->y = 50;
            DPoint *h2 = new DPoint; h2->element = "H"; h2->x = i*5; h2->y = 150;
            m.addBond(c1, h1, 1, 1, Qt::black, false);
            m.addBond(c2, h2, 1, 1, Qt::black, false);
        }

        double expected = 348.0 + 6 * 413.0;  // 2826
        QCOMPARE(m.SumBondEnthalpy(), expected);
    }

    void sumEnthalpy_empty_molecule_is_0()
    {
        Molecule m(nullptr);
        QCOMPARE(m.SumBondEnthalpy(), 0.0);
    }

    void sumEnthalpy_single_bond()
    {
        Molecule m(nullptr);
        DPoint *c = new DPoint; c->element = "C"; c->x = 50; c->y = 100;
        DPoint *h = new DPoint; h->element = "H"; h->x = 75; h->y = 100;
        m.addBond(c, h, 1, 1, Qt::black, false);
        QCOMPARE(m.SumBondEnthalpy(), 413.0);  // C-H single
    }
};

QTEST_APPLESS_MAIN(TestBondEnthalpy)
#include "tst_bond_enthalpy.moc"
