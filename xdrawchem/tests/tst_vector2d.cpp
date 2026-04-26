// tst_vector2d.cpp — unit tests for Vector2D
// Tests cover: construction, magnitude, normalize, dot/cross product,
// rotate, reverse, round, and operator arithmetic.

#include <QtTest>
#include <cmath>
#include "../xdrawchem/vector2D.h"

class TestVector2D : public QObject
{
    Q_OBJECT

private slots:
    // ── Construction ───────────────────────────────────────────────────────
    void defaultConstructor()
    {
        Vector2D v;
        QCOMPARE(v.x, 0.0);
        QCOMPARE(v.y, 0.0);
    }

    void valueConstructor()
    {
        Vector2D v(3.0, 4.0);
        QCOMPARE(v.x, 3.0);
        QCOMPARE(v.y, 4.0);
    }

    // ── Magnitude ──────────────────────────────────────────────────────────
    void magnitudeSquared()
    {
        Vector2D v(3.0, 4.0);
        QCOMPARE(v.magnitudeSquared(), 25.0);
    }

    void magnitude_3_4_is_5()
    {
        Vector2D v(3.0, 4.0);
        QCOMPARE(v.magnitude(), 5.0);
    }

    void magnitude_unit_x()
    {
        Vector2D v(1.0, 0.0);
        QCOMPARE(v.magnitude(), 1.0);
    }

    void magnitude_zero()
    {
        Vector2D v(0.0, 0.0);
        QCOMPARE(v.magnitude(), 0.0);
    }

    // ── Normalize ──────────────────────────────────────────────────────────
    void normalize_3_4()
    {
        Vector2D v(3.0, 4.0);
        v.normalize();
        QVERIFY(std::abs(v.magnitude() - 1.0) < 1e-9);
        QVERIFY(std::abs(v.x - 0.6) < 1e-9);
        QVERIFY(std::abs(v.y - 0.8) < 1e-9);
    }

    void normalize_zero_vector_is_safe()
    {
        // Zero vector — must not crash or produce NaN
        Vector2D v(0.0, 0.0);
        v.normalize();
        QCOMPARE(v.x, 0.0);
        QCOMPARE(v.y, 0.0);
    }

    void normalize_already_unit()
    {
        Vector2D v(1.0, 0.0);
        v.normalize();
        QCOMPARE(v.x, 1.0);
        QCOMPARE(v.y, 0.0);
    }

    // ── Dot product ────────────────────────────────────────────────────────
    void dotProduct_parallel()
    {
        Vector2D a(1.0, 0.0);
        Vector2D b(5.0, 0.0);
        QCOMPARE(a.dotProduct(&b), 5.0);
    }

    void dotProduct_perpendicular()
    {
        Vector2D a(1.0, 0.0);
        Vector2D b(0.0, 1.0);
        QCOMPARE(a.dotProduct(&b), 0.0);
    }

    void dotProduct_general()
    {
        Vector2D a(2.0, 3.0);
        Vector2D b(4.0, 5.0);
        // 2*4 + 3*5 = 8+15 = 23
        QCOMPARE(a.dotProduct(&b), 23.0);
    }

    // ── Cross product ──────────────────────────────────────────────────────
    void crossProduct_unit_vectors()
    {
        Vector2D a(1.0, 0.0);
        Vector2D b(0.0, 1.0);
        // 2D cross = x1*y2 - y1*x2 = 1*1 - 0*0 = 1
        QCOMPARE(a.crossProduct(&b), 1.0);
    }

    void crossProduct_parallel_is_zero()
    {
        Vector2D a(2.0, 0.0);
        Vector2D b(5.0, 0.0);
        QCOMPARE(a.crossProduct(&b), 0.0);
    }

    void crossProduct_anticommutative()
    {
        Vector2D a(3.0, 2.0);
        Vector2D b(1.0, 4.0);
        QCOMPARE(a.crossProduct(&b), -b.crossProduct(&a));
    }

    // ── Reverse ────────────────────────────────────────────────────────────
    void reverse()
    {
        Vector2D v(3.0, -4.0);
        v.reverse();
        QCOMPARE(v.x, -3.0);
        QCOMPARE(v.y,  4.0);
    }

    void reverseDouble_is_identity()
    {
        Vector2D v(3.0, 4.0);
        v.reverse();
        v.reverse();
        QCOMPARE(v.x, 3.0);
        QCOMPARE(v.y, 4.0);
    }

    // ── Rotate ─────────────────────────────────────────────────────────────
    void rotate_90_degrees()
    {
        Vector2D v(1.0, 0.0);
        v.rotate(M_PI / 2.0);
        QVERIFY(std::abs(v.x - 0.0) < 1e-6);
        QVERIFY(std::abs(v.y - 1.0) < 1e-6);
    }

    void rotate_180_degrees()
    {
        Vector2D v(1.0, 0.0);
        v.rotate(M_PI);
        QVERIFY(std::abs(v.x - (-1.0)) < 1e-6);
        QVERIFY(std::abs(v.y - 0.0)   < 1e-6);
    }

    void rotate_360_is_identity()
    {
        Vector2D v(3.0, 4.0);
        v.rotate(2.0 * M_PI);
        QVERIFY(std::abs(v.x - 3.0) < 1e-5);
        QVERIFY(std::abs(v.y - 4.0) < 1e-5);
    }

    void rotate_transpose()
    {
        // Transpose rotation is the inverse — rotate +90 then transpose +90 should be identity
        Vector2D v(1.0, 0.0);
        v.rotate(M_PI / 2.0, false);
        v.rotate(M_PI / 2.0, true);
        QVERIFY(std::abs(v.x - 1.0) < 1e-5);
        QVERIFY(std::abs(v.y - 0.0) < 1e-5);
    }

    // ── Equals ─────────────────────────────────────────────────────────────
    void equals_same()
    {
        Vector2D a(1.5, 2.5);
        Vector2D b(1.5, 2.5);
        QVERIFY(a.equals(&b));
    }

    void equals_different()
    {
        Vector2D a(1.0, 2.0);
        Vector2D b(1.0, 3.0);
        QVERIFY(!a.equals(&b));
    }

    // ── Round ──────────────────────────────────────────────────────────────
    void round_to_2_decimals()
    {
        Vector2D v(1.234567, 2.345678);
        v.round(2);
        QCOMPARE(v.x, 1.23);
        QCOMPARE(v.y, 2.35);
    }

    void round_default_is_10_decimals()
    {
        Vector2D v(1.0 / 3.0, 2.0 / 3.0);
        v.round(); // default: 10 decimal places
        // Tolerance is 5e-11 (half of the last rounded place) to account for
        // floating-point representation of 1/3 before rounding
        // Tolerance accounts for the gap between the C++ double literal and
        // the actual rounded binary value (measured: ~1.22e-10)
        QVERIFY(std::abs(v.x - 0.3333333333) < 2e-10);
        QVERIFY(std::abs(v.y - 0.6666666667) < 2e-10);
    }
};

QTEST_APPLESS_MAIN(TestVector2D)
#include "tst_vector2d.moc"
