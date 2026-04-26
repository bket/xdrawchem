// tst_cmlparser.cpp — unit tests for CMLParser (QXmlStreamReader-based CML parser).
//
// CMLParser (xml_cml.cpp) handles the "new" XDrawChem CML dialect (1.3+):
//   <atom id="a0">
//     <string builtin="elementType">C</string>
//     <float builtin="x2">57.3</float>
//     <float builtin="y2">40.0</float>
//   </atom>
//   <bond id="b0">
//     <string builtin="atomRef">a0</string>
//     <string builtin="atomRef">a1</string>
//     <string builtin="order">1</string>
//   </bond>
//
// Legacy files (benzene.cml) use <coordinate2> / <integer> tags and are handled
// by the separate LoadCMLFile path in chemdata_cml.cpp — not tested here.
//
// Disk fixtures: anthracene.cml (14C, 16 bonds, new dialect)
//                biotin.cml     (18 atoms incl N, O, S; 19 bonds, new dialect)

#include <QtTest>
#include <QBuffer>
#include <QFile>
#include <cmath>

#include "../xdrawchem/xml_cml.h"
#include "../xdrawchem/dpoint.h"
#include "../xdrawchem/bond.h"

static CMLParser parseCml(const QByteArray &xml)
{
    CMLParser parser(nullptr);
    QBuffer buf;
    buf.setData(xml);
    buf.open(QIODevice::ReadOnly);
    parser.parse(&buf);
    return parser;
}

static CMLParser parseCmlFile(const QString &path)
{
    CMLParser parser(nullptr);
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return parser;
    parser.parse(&f);
    return parser;
}

class TestCmlParser : public QObject
{
    Q_OBJECT

private:
    QString ringDir()
    {
        QString env = qEnvironmentVariable("RINGHOME");
        if (!env.isEmpty()) return env + "/";
        return QString(RING_DIR) + "/";
    }

private slots:

    // ── Empty and degenerate inputs ─────────────────────────────────────────
    void empty_xml_gives_zero_atoms()
    {
        auto p = parseCml("");
        QCOMPARE(p.getPoints().size(), 0);
        QCOMPARE(p.getBonds().size(),  0);
    }

    void molecule_with_no_atoms()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?><molecule id=\"m\"></molecule>"
        );
        QCOMPARE(p.getPoints().size(), 0);
        QCOMPARE(p.getBonds().size(),  0);
    }

    void malformed_xml_does_not_crash()
    {
        // Truncated — parser must not crash or throw
        auto p = parseCml("<molecule><atom id=\"a1\"><string builtin=\"elementType\">C</string>");
        QVERIFY(p.getPoints().size() >= 0);
    }

    // ── Single atom (new dialect) ────────────────────────────────────────────
    void single_carbon_element_parsed()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">100.0</float>\n"
            "    <float builtin=\"y2\">200.0</float>\n"
            "  </atom>\n"
            "</molecule>"
        );
        QCOMPARE(p.getPoints().size(), 1);
        QCOMPARE(p.getBonds().size(),  0);
        DPoint *pt = p.getPoints().first();
        QCOMPARE(pt->element, QString("C"));
    }

    void single_atom_coordinates_parsed()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">N</string>\n"
            "    <float builtin=\"x2\">50.0</float>\n"
            "    <float builtin=\"y2\">75.0</float>\n"
            "  </atom>\n"
            "</molecule>"
        );
        QCOMPARE(p.getPoints().size(), 1);
        DPoint *pt = p.getPoints().first();
        QVERIFY(std::abs(pt->x - 50.0) < 0.01);
        QVERIFY(std::abs(pt->y - 75.0) < 0.01);
    }

    void single_nitrogen_atom()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">N</string>\n"
            "    <float builtin=\"x2\">0.0</float>\n"
            "    <float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "</molecule>"
        );
        QCOMPARE(p.getPoints().size(), 1);
        QCOMPARE(p.getPoints().first()->element, QString("N"));
    }

    // ── Bond parsing (new dialect uses <string builtin="atomRef"> children) ──
    void two_atoms_one_bond_new_dialect()
    {
        // In the new dialect bonds reference atoms via <string builtin="atomRef">
        // children, NOT via atomRefs="a1 a2" attribute
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">0.0</float>\n"
            "    <float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <atom id=\"a2\">\n"
            "    <string builtin=\"elementType\">O</string>\n"
            "    <float builtin=\"x2\">25.0</float>\n"
            "    <float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <bond id=\"b1\">\n"
            "    <string builtin=\"atomRef\">a1</string>\n"
            "    <string builtin=\"atomRef\">a2</string>\n"
            "    <string builtin=\"order\">2</string>\n"
            "  </bond>\n"
            "</molecule>"
        );
        QCOMPARE(p.getPoints().size(), 2);
        QCOMPARE(p.getBonds().size(),  1);
        Bond *b = p.getBonds().first();
        QCOMPARE(b->Order(), 2);
        QVERIFY2(b->Start() != nullptr, "Bond Start should be resolved");
        QVERIFY2(b->End()   != nullptr, "Bond End should be resolved");
        QVERIFY(b->Start() != b->End());
    }

    void bond_order_1_parsed()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">0.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <atom id=\"a2\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">25.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <bond id=\"b1\">\n"
            "    <string builtin=\"atomRef\">a1</string>\n"
            "    <string builtin=\"atomRef\">a2</string>\n"
            "    <string builtin=\"order\">1</string>\n"
            "  </bond>\n"
            "</molecule>"
        );
        QCOMPARE(p.getBonds().size(), 1);
        QCOMPARE(p.getBonds().first()->Order(), 1);
    }

    void bond_order_3_parsed()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">0.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <atom id=\"a2\">\n"
            "    <string builtin=\"elementType\">N</string>\n"
            "    <float builtin=\"x2\">25.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <bond id=\"b1\">\n"
            "    <string builtin=\"atomRef\">a1</string>\n"
            "    <string builtin=\"atomRef\">a2</string>\n"
            "    <string builtin=\"order\">3</string>\n"
            "  </bond>\n"
            "</molecule>"
        );
        QCOMPARE(p.getBonds().size(), 1);
        QCOMPARE(p.getBonds().first()->Order(), 3);
    }

    // ── Stereo bonds ────────────────────────────────────────────────────────
    void stereo_W_gives_order_5()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">0.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <atom id=\"a2\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">25.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <bond id=\"b1\">\n"
            "    <string builtin=\"atomRef\">a1</string>\n"
            "    <string builtin=\"atomRef\">a2</string>\n"
            "    <string builtin=\"order\">1</string>\n"
            "    <string builtin=\"stereo\" convention=\"MDLMol\">W</string>\n"
            "  </bond>\n"
            "</molecule>"
        );
        QCOMPARE(p.getBonds().size(), 1);
        QCOMPARE(p.getBonds().first()->Order(), 5);  // stereo-up = 5
    }

    void stereo_H_gives_order_7()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">0.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <atom id=\"a2\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x2\">25.0</float><float builtin=\"y2\">0.0</float>\n"
            "  </atom>\n"
            "  <bond id=\"b1\">\n"
            "    <string builtin=\"atomRef\">a1</string>\n"
            "    <string builtin=\"atomRef\">a2</string>\n"
            "    <string builtin=\"order\">1</string>\n"
            "    <string builtin=\"stereo\" convention=\"MDLMol\">H</string>\n"
            "  </bond>\n"
            "</molecule>"
        );
        QCOMPARE(p.getBonds().size(), 1);
        QCOMPARE(p.getBonds().first()->Order(), 7);  // stereo-down = 7
    }

    // ── 3D coordinate alias: x3/y3 mapped to x2/y2 ─────────────────────────
    void x3_y3_mapped_to_2d_coordinates()
    {
        auto p = parseCml(
            "<?xml version=\"1.0\"?>\n"
            "<molecule id=\"m1\">\n"
            "  <atom id=\"a1\">\n"
            "    <string builtin=\"elementType\">C</string>\n"
            "    <float builtin=\"x3\">33.0</float>\n"
            "    <float builtin=\"y3\">44.0</float>\n"
            "  </atom>\n"
            "</molecule>"
        );
        QCOMPARE(p.getPoints().size(), 1);
        DPoint *pt = p.getPoints().first();
        QVERIFY(std::abs(pt->x - 33.0) < 0.01);
        QVERIFY(std::abs(pt->y - 44.0) < 0.01);
    }

    // ── Disk fixture: anthracene.cml (14C, 16 bonds, new dialect) ───────────
    void anthracene_atom_count()
    {
        QString path = ringDir() + "anthracene.cml";
        if (!QFile::exists(path)) QSKIP("anthracene.cml not found");
        auto p = parseCmlFile(path);
        QCOMPARE(p.getPoints().size(), 14);
    }

    void anthracene_bond_count()
    {
        QString path = ringDir() + "anthracene.cml";
        if (!QFile::exists(path)) QSKIP("anthracene.cml not found");
        auto p = parseCmlFile(path);
        QCOMPARE(p.getBonds().size(), 16);
    }

    void anthracene_all_atoms_are_carbon()
    {
        QString path = ringDir() + "anthracene.cml";
        if (!QFile::exists(path)) QSKIP("anthracene.cml not found");
        auto p = parseCmlFile(path);
        for (DPoint *pt : p.getPoints())
            QCOMPARE(pt->element, QString("C"));
    }

    void anthracene_coordinates_in_range()
    {
        QString path = ringDir() + "anthracene.cml";
        if (!QFile::exists(path)) QSKIP("anthracene.cml not found");
        auto p = parseCmlFile(path);
        // Anthracene ring coords from file are roughly x: 57–200, y: 40–110
        for (DPoint *pt : p.getPoints()) {
            QVERIFY2(pt->x > 0   && pt->x < 300, "x coordinate out of expected range");
            QVERIFY2(pt->y > 0   && pt->y < 200, "y coordinate out of expected range");
        }
    }

    void anthracene_bond_endpoints_resolved()
    {
        QString path = ringDir() + "anthracene.cml";
        if (!QFile::exists(path)) QSKIP("anthracene.cml not found");
        auto p = parseCmlFile(path);
        for (Bond *b : p.getBonds()) {
            QVERIFY2(b->Start() != nullptr, "Bond has null Start");
            QVERIFY2(b->End()   != nullptr, "Bond has null End");
            QVERIFY2(b->Start() != b->End(), "Bond start == end");
        }
    }

    // ── Disk fixture: biotin.cml (18 atoms incl N, O, S; new dialect) ───────
    void biotin_has_nitrogen()
    {
        QString path = ringDir() + "biotin.cml";
        if (!QFile::exists(path)) QSKIP("biotin.cml not found");
        auto p = parseCmlFile(path);
        bool found = false;
        for (DPoint *pt : p.getPoints())
            if (pt->element == "N") { found = true; break; }
        QVERIFY2(found, "Biotin must contain nitrogen");
    }

    void biotin_has_oxygen()
    {
        QString path = ringDir() + "biotin.cml";
        if (!QFile::exists(path)) QSKIP("biotin.cml not found");
        auto p = parseCmlFile(path);
        bool found = false;
        for (DPoint *pt : p.getPoints())
            if (pt->element == "O" || pt->element == "OH") { found = true; break; }
        QVERIFY2(found, "Biotin must contain oxygen");
    }

    void biotin_has_sulfur()
    {
        QString path = ringDir() + "biotin.cml";
        if (!QFile::exists(path)) QSKIP("biotin.cml not found");
        auto p = parseCmlFile(path);
        bool found = false;
        for (DPoint *pt : p.getPoints())
            if (pt->element == "S") { found = true; break; }
        QVERIFY2(found, "Biotin must contain sulfur");
    }

    void biotin_atom_count()
    {
        QString path = ringDir() + "biotin.cml";
        if (!QFile::exists(path)) QSKIP("biotin.cml not found");
        auto p = parseCmlFile(path);
        QCOMPARE(p.getPoints().size(), 18);
    }

    void biotin_bond_count()
    {
        QString path = ringDir() + "biotin.cml";
        if (!QFile::exists(path)) QSKIP("biotin.cml not found");
        auto p = parseCmlFile(path);
        QCOMPARE(p.getBonds().size(), 19);
    }
};

QTEST_APPLESS_MAIN(TestCmlParser)
#include "tst_cmlparser.moc"
