// tst_dpoint.cpp — unit tests for DPoint geometry and element methods.
// DPoint is the shared atom/vertex type used throughout the drawing model.

#include <QtTest>
#include <cmath>
#include "../xdrawchem/dpoint.h"

class TestDPoint : public QObject
{
    Q_OBJECT

private slots:
    // ── Construction ───────────────────────────────────────────────────────
    void defaultConstructor_zeroCoords()
    {
        DPoint p;
        QCOMPARE(p.x, 0.0);
        QCOMPARE(p.y, 0.0);
    }

    void defaultElement_isCarbon()
    {
        DPoint p;
        QCOMPARE(p.element, QString("C"));
    }

    // ── Distance ───────────────────────────────────────────────────────────
    void distanceTo_self_is_zero()
    {
        DPoint p;
        p.x = 3.0; p.y = 4.0;
        QCOMPARE(p.distanceTo(&p), 0.0);
    }

    void distanceTo_3_4_triangle()
    {
        DPoint a, b;
        a.x = 0.0; a.y = 0.0;
        b.x = 3.0; b.y = 4.0;
        QCOMPARE(a.distanceTo(&b), 5.0);
    }

    void distanceTo_is_symmetric()
    {
        DPoint a, b;
        a.x = 1.0; a.y = 2.0;
        b.x = 4.0; b.y = 6.0;
        QCOMPARE(a.distanceTo(&b), b.distanceTo(&a));
    }

    void distanceTo_horizontal()
    {
        DPoint a, b;
        a.x = 0.0; a.y = 0.0;
        b.x = 7.0; b.y = 0.0;
        QCOMPARE(a.distanceTo(&b), 7.0);
    }

    void distanceTo_vertical()
    {
        DPoint a, b;
        a.x = 0.0; a.y = 0.0;
        b.x = 0.0; b.y = 5.0;
        QCOMPARE(a.distanceTo(&b), 5.0);
    }

    void distanceTo_negative_coords()
    {
        DPoint a, b;
        a.x = -3.0; a.y = -4.0;
        b.x =  0.0; b.y =  0.0;
        QCOMPARE(a.distanceTo(&b), 5.0);
    }

    // ── Atomic number ──────────────────────────────────────────────────────
    void atomicNumber_carbon()
    {
        DPoint p;
        p.element = "C";
        QCOMPARE(p.getAtomicNumber(), 6);
    }

    void atomicNumber_hydrogen()
    {
        DPoint p;
        p.element = "H";
        QCOMPARE(p.getAtomicNumber(), 1);
    }

    void atomicNumber_oxygen()
    {
        DPoint p;
        p.element = "O";
        QCOMPARE(p.getAtomicNumber(), 8);
    }

    void atomicNumber_nitrogen()
    {
        DPoint p;
        p.element = "N";
        QCOMPARE(p.getAtomicNumber(), 7);
    }

    void atomicNumber_sulfur()
    {
        DPoint p;
        p.element = "S";
        QCOMPARE(p.getAtomicNumber(), 16);
    }

    void atomicNumber_chlorine()
    {
        DPoint p;
        p.element = "Cl";
        QCOMPARE(p.getAtomicNumber(), 17);
    }

    void atomicNumber_unknown_returns_999()
    {
        DPoint p;
        p.element = "Xx";
        QCOMPARE(p.getAtomicNumber(), 999);
    }

    // ── Base element (strips charge/count suffixes) ────────────────────────
    void baseElement_plain_carbon()
    {
        DPoint p;
        p.element = "C";
        QCOMPARE(p.baseElement(), QString("C"));
    }

    void baseElement_strips_digits()
    {
        // e.g. "CH3" → "CH" after digit removal
        DPoint p;
        p.element = "C1";
        // baseElement removes digits via QRegularExpression("\\d+")
        QCOMPARE(p.baseElement(), QString("C"));
    }

    void baseElement_oxygen()
    {
        DPoint p;
        p.element = "O";
        QCOMPARE(p.baseElement(), QString("O"));
    }

    // ── Aromatic flag ──────────────────────────────────────────────────────
    void aromatic_defaults_false()
    {
        DPoint p;
        QVERIFY(!p.aromatic);
    }

    void aromatic_can_be_set()
    {
        DPoint p;
        p.aromatic = true;
        QVERIFY(p.aromatic);
    }

    // ── In-ring flag ───────────────────────────────────────────────────────
    void inring_defaults_false()
    {
        DPoint p;
        QVERIFY(!p.inring);
    }
};

QTEST_APPLESS_MAIN(TestDPoint)
#include "tst_dpoint.moc"
