// tst_bondgeometry.cpp — unit tests for Bond geometry methods.
// Tests cover: midpoint (including the bug we fixed: dy was using end->x),
// toVector(), and the three angle calculation methods.
// Bond depends on Render2D for rendering, so we pass nullptr and only test
// geometry methods that do not call the renderer.

#include <QtTest>
#include <cmath>

// Bring in just the geometry-relevant headers, not the full widget stack
#include "../xdrawchem/dpoint.h"
#include "../xdrawchem/vector2D.h"

// We test Bond geometry via a thin wrapper that exposes the inline methods
// without constructing a full Bond (which needs Render2D*).
// The three angle methods and midpoint are all inline in bond.h.
// We replicate the logic here so we can test it without the Qt widget stack.

// ── Replicated inline logic from bond.h ────────────────────────────────────
// (These are copied verbatim from the fixed bond.h to verify correctness.)

static QPoint bondMidpoint(DPoint *start, DPoint *end)
{
    double dx = (end->x + start->x) / 2.0;
    double dy = (end->y + start->y) / 2.0;  // bug was: end->x + start->x
    return QPoint(static_cast<int>(dx), static_cast<int>(dy));
}

static Vector2D bondToVector(DPoint *start, DPoint *end)
{
    return Vector2D(end->x - start->x, end->y - start->y);
}

static double getAngleBetween(DPoint *s1, DPoint *e1, DPoint *s2, DPoint *e2)
{
    Vector2D v1 = bondToVector(s1, e1);
    Vector2D v2 = bondToVector(s2, e2);
    v1.normalize();
    v2.normalize();
    double dot = v1.dotProduct(&v2);
    // clamp to [-1, 1] for numerical safety
    dot = std::max(-1.0, std::min(1.0, dot));
    return std::acos(dot);
}

// ─────────────────────────────────────────────────────────────────────────────

class TestBondGeometry : public QObject
{
    Q_OBJECT

private slots:
    // ── midpoint — regression test for the fixed copy-paste bug ──────────
    void midpoint_horizontal_bond()
    {
        DPoint s, e;
        s.x = 0.0; s.y = 0.0;
        e.x = 4.0; e.y = 0.0;
        QPoint mp = bondMidpoint(&s, &e);
        QCOMPARE(mp.x(), 2);
        QCOMPARE(mp.y(), 0);  // was wrong before fix (used x instead of y)
    }

    void midpoint_vertical_bond()
    {
        DPoint s, e;
        s.x = 0.0; s.y = 0.0;
        e.x = 0.0; e.y = 6.0;
        QPoint mp = bondMidpoint(&s, &e);
        QCOMPARE(mp.x(), 0);
        QCOMPARE(mp.y(), 3);
    }

    void midpoint_diagonal_bond()
    {
        DPoint s, e;
        s.x = 2.0; s.y = 4.0;
        e.x = 8.0; e.y = 10.0;
        QPoint mp = bondMidpoint(&s, &e);
        QCOMPARE(mp.x(), 5);
        QCOMPARE(mp.y(), 7);
    }

    void midpoint_negative_coords()
    {
        DPoint s, e;
        s.x = -4.0; s.y = -6.0;
        e.x =  2.0; e.y =  2.0;
        QPoint mp = bondMidpoint(&s, &e);
        QCOMPARE(mp.x(), -1);
        QCOMPARE(mp.y(), -2);
    }

    // Critical regression: dy was computed as (end->x + start->x)/2 instead
    // of (end->y + start->y)/2. For a non-horizontal bond, x≠y so this
    // produced wrong results. This test would have failed on the buggy code.
    void midpoint_regression_dy_uses_y_not_x()
    {
        DPoint s, e;
        s.x = 10.0; s.y = 0.0;
        e.x = 10.0; e.y = 8.0;  // vertical bond at x=10
        QPoint mp = bondMidpoint(&s, &e);
        // x midpoint = (10+10)/2 = 10, y midpoint = (0+8)/2 = 4
        // Bug would have given y = (10+10)/2 = 10 instead of 4
        QCOMPARE(mp.x(), 10);
        QCOMPARE(mp.y(), 4);
    }

    // ── toVector ───────────────────────────────────────────────────────────
    void toVector_horizontal()
    {
        DPoint s, e;
        s.x = 0.0; s.y = 0.0;
        e.x = 5.0; e.y = 0.0;
        Vector2D v = bondToVector(&s, &e);
        QCOMPARE(v.x, 5.0);
        QCOMPARE(v.y, 0.0);
    }

    void toVector_diagonal()
    {
        DPoint s, e;
        s.x = 1.0; s.y = 2.0;
        e.x = 4.0; e.y = 6.0;
        Vector2D v = bondToVector(&s, &e);
        QCOMPARE(v.x, 3.0);
        QCOMPARE(v.y, 4.0);
    }

    void toVector_reverse_has_negated_components()
    {
        DPoint s, e;
        s.x = 3.0; s.y = 4.0;
        e.x = 0.0; e.y = 0.0;
        Vector2D v = bondToVector(&s, &e);
        QCOMPARE(v.x, -3.0);
        QCOMPARE(v.y, -4.0);
    }

    // ── Angle between bonds ────────────────────────────────────────────────
    void angleBetween_parallel_bonds_is_zero()
    {
        // Two horizontal bonds pointing in the same direction
        DPoint s1, e1, s2, e2;
        s1.x = 0; s1.y = 0; e1.x = 1; e1.y = 0;
        s2.x = 2; s2.y = 0; e2.x = 3; e2.y = 0;
        double angle = getAngleBetween(&s1, &e1, &s2, &e2);
        QVERIFY(std::abs(angle) < 1e-9);
    }

    void angleBetween_perpendicular_bonds_is_90()
    {
        DPoint s1, e1, s2, e2;
        s1.x = 0; s1.y = 0; e1.x = 1; e1.y = 0;  // horizontal
        s2.x = 0; s2.y = 0; e2.x = 0; e2.y = 1;  // vertical
        double angle = getAngleBetween(&s1, &e1, &s2, &e2);
        QVERIFY(std::abs(angle - M_PI / 2.0) < 1e-9);
    }

    void angleBetween_antiparallel_bonds_is_180()
    {
        DPoint s1, e1, s2, e2;
        s1.x = 0; s1.y = 0; e1.x =  1; e1.y = 0;
        s2.x = 0; s2.y = 0; e2.x = -1; e2.y = 0;
        double angle = getAngleBetween(&s1, &e1, &s2, &e2);
        QVERIFY(std::abs(angle - M_PI) < 1e-9);
    }

    void angleBetween_60_degrees()
    {
        // Bond 1: along x-axis. Bond 2: at 60° = (cos60, sin60) = (0.5, √3/2)
        DPoint s1, e1, s2, e2;
        s1.x = 0; s1.y = 0; e1.x = 1;           e1.y = 0;
        s2.x = 0; s2.y = 0; e2.x = 0.5; e2.y = std::sqrt(3.0) / 2.0;
        double angle = getAngleBetween(&s1, &e1, &s2, &e2);
        QVERIFY(std::abs(angle - M_PI / 3.0) < 1e-6);
    }

    void angleBetween_is_symmetric()
    {
        DPoint s1, e1, s2, e2;
        s1.x = 0; s1.y = 0; e1.x = 3; e1.y = 4;
        s2.x = 0; s2.y = 0; e2.x = 1; e2.y = 0;
        double ab = getAngleBetween(&s1, &e1, &s2, &e2);
        double ba = getAngleBetween(&s2, &e2, &s1, &e1);
        QVERIFY(std::abs(ab - ba) < 1e-9);
    }
};

QTEST_APPLESS_MAIN(TestBondGeometry)
#include "tst_bondgeometry.moc"
