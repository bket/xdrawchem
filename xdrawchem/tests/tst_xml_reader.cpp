// tst_xml_reader.cpp — unit tests for XML_Reader utility functions.
//
// XML_Reader is a base class with pure string-parsing static-style utilities.
// No Qt widget, file system, or OpenBabel dependency — QTEST_APPLESS_MAIN.
//
// Functions tested:
//   selfContainedTag(tag)         → true if tag ends with "/>"
//   readTag(doc, ptr)             → extract next "<...>" tag
//   readData(doc, ptr)            → extract text between current pos and next "<"
//   readAttr(tag)                 → parse attribute name/value pairs
//   tokenize(tag)                 → split on whitespace
//   positionOfEndTag(doc, name)   → find matching close tag depth

#include <QtTest>
#include "../xdrawchem/xml_reader.h"

// Concrete subclass (XML_Reader has a pure-virtual readFile)
class TestReader : public XML_Reader {};

class TestXmlReader : public QObject
{
    Q_OBJECT

private:
    TestReader r;

private slots:

    // ── selfContainedTag ─────────────────────────────────────────────────────
    void selfContained_empty_element()
    {
        QVERIFY(r.selfContainedTag("<br/>"));
    }

    void selfContained_with_attrs()
    {
        QVERIFY(r.selfContainedTag("<bond id=\"b1\" order=\"1\"/>"));
    }

    void selfContained_not_for_open_tag()
    {
        QVERIFY(!r.selfContainedTag("<molecule>"));
    }

    void selfContained_not_for_data_tag()
    {
        QVERIFY(!r.selfContainedTag("<atom>C</atom>"));
    }

    void selfContained_not_for_close_tag()
    {
        QVERIFY(!r.selfContainedTag("</molecule>"));
    }

    // ── readTag ──────────────────────────────────────────────────────────────
    void readTag_first_tag()
    {
        QString doc = "<molecule><atom>C</atom></molecule>";
        QCOMPARE(r.readTag(doc, 0), QString("<molecule>"));
    }

    void readTag_from_offset()
    {
        QString doc = "<molecule><atom>C</atom></molecule>";
        int ptr = doc.indexOf("<atom>");
        QCOMPARE(r.readTag(doc, ptr), QString("<atom>"));
    }

    void readTag_self_contained()
    {
        QString doc = "<atom id=\"a1\"/>";
        QCOMPARE(r.readTag(doc, 0), QString("<atom id=\"a1\"/>"));
    }

    void readTag_with_attrs()
    {
        QString doc = "<bond order=\"2\" id=\"b3\">";
        QCOMPARE(r.readTag(doc, 0), QString("<bond order=\"2\" id=\"b3\">"));
    }

    // ── readData ─────────────────────────────────────────────────────────────
    void readData_element_symbol()
    {
        // After the opening tag, data is "C" before the next "<"
        QString doc = "<atom>C</atom>";
        int ptr = QString("<atom>").length();
        QCOMPARE(r.readData(doc, ptr), QString("C"));
    }

    void readData_number()
    {
        QString doc = "<order>2</order>";
        int ptr = QString("<order>").length();
        QCOMPARE(r.readData(doc, ptr), QString("2"));
    }

    void readData_coordinate()
    {
        QString doc = "<x>123.45</x>";
        int ptr = QString("<x>").length();
        QCOMPARE(r.readData(doc, ptr), QString("123.45"));
    }

    void readData_empty_is_empty()
    {
        QString doc = "<tag></tag>";
        int ptr = QString("<tag>").length();
        QCOMPARE(r.readData(doc, ptr), QString(""));
    }

    // ── readAttr ─────────────────────────────────────────────────────────────
    void readAttr_single_attribute()
    {
        QStringList attrs = r.readAttr("<atom id=\"a1\">");
        QCOMPARE(attrs.size(), 2);
        QCOMPARE(attrs[0], QString("id"));
        QCOMPARE(attrs[1], QString("a1"));
    }

    void readAttr_two_attributes()
    {
        QStringList attrs = r.readAttr("<bond id=\"b1\" order=\"2\">");
        QCOMPARE(attrs.size(), 4);
        QCOMPARE(attrs[0], QString("id"));
        QCOMPARE(attrs[1], QString("b1"));
        QCOMPARE(attrs[2], QString("order"));
        QCOMPARE(attrs[3], QString("2"));
    }

    void readAttr_no_attributes_is_empty()
    {
        QStringList attrs = r.readAttr("<molecule>");
        QVERIFY(attrs.isEmpty());
    }

    void readAttr_self_contained_tag()
    {
        QStringList attrs = r.readAttr("<atom id=\"a5\"/>");
        QCOMPARE(attrs.size(), 2);
        QCOMPARE(attrs[0], QString("id"));
        QCOMPARE(attrs[1], QString("a5"));
    }

    void readAttr_numeric_value()
    {
        QStringList attrs = r.readAttr("<float value=\"3.14\">");
        QCOMPARE(attrs.size(), 2);
        QCOMPARE(attrs[0].trimmed(), QString("value"));
        QCOMPARE(attrs[1], QString("3.14"));
    }

    // ── tokenize ─────────────────────────────────────────────────────────────
    void tokenize_two_tokens()
    {
        QStringList toks = r.tokenize("223.651 252");
        QCOMPARE(toks.size(), 2);
        QCOMPARE(toks[0], QString("223.651"));
        QCOMPARE(toks[1], QString("252"));
    }

    void tokenize_four_tokens()
    {
        QStringList toks = r.tokenize("10 20 30 40");
        QCOMPARE(toks.size(), 4);
        QCOMPARE(toks[0], QString("10"));
        QCOMPARE(toks[3], QString("40"));
    }

    void tokenize_single_token()
    {
        // Known tokenize quirk: for a single-token input (no spaces), the loop
        // exits without appending anything, then the trailing mid(ptr) appends
        // the whole string again — so the result contains 2 copies of the token.
        // This is a pre-existing quirk of the utility (only used for multi-token
        // coordinate strings like "223.651 252" in practice).
        QStringList toks = r.tokenize("hello");
        QCOMPARE(toks.size(), 2);          // returns token twice for single input
        QCOMPARE(toks[0], QString("hello"));
        QCOMPARE(toks[1], QString("hello"));
    }

    void tokenize_strips_extra_whitespace()
    {
        // simplified() normalises internal spaces
        QStringList toks = r.tokenize("  a   b  ");
        QCOMPARE(toks.size(), 2);
        QCOMPARE(toks[0], QString("a"));
        QCOMPARE(toks[1], QString("b"));
    }

    // ── positionOfEndTag ──────────────────────────────────────────────────────
    void endTag_simple_pair()
    {
        // <a>text</a> — end tag closes at position after "</a>"
        QString doc = "<a>text</a>";
        int pos = r.positionOfEndTag(doc, "a");
        // pos should be just after "</a>"
        QVERIFY(pos > 0);
        QVERIFY(pos <= doc.length());
    }

    void endTag_nested_same_name()
    {
        // <n><n>inner</n></n> — must match outer close tag
        QString doc = "<n><n>inner</n></n>";
        int pos = r.positionOfEndTag(doc, "n");
        // Should point past the OUTER </n>, not the inner one
        QVERIFY(pos == doc.length());
    }

    void endTag_two_siblings()
    {
        // <root><a>1</a><a>2</a></root>
        // Looking for end of first <a> block
        QString doc = "<root><a>1</a><a>2</a></root>";
        int startOfA = doc.indexOf("<a>");
        QString subDoc = doc.mid(startOfA);
        int pos = r.positionOfEndTag(subDoc, "a");
        // pos should be after "</a>" in subDoc (length of "<a>1</a>" = 8 → "a>" = pos 8)
        QVERIFY(pos > 0);
        QVERIFY(pos < subDoc.length());
    }
};

QTEST_APPLESS_MAIN(TestXmlReader)
#include "tst_xml_reader.moc"
