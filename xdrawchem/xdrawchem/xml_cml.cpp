// xml_cml.cpp -- CML parser using QXmlStreamReader (Qt5/6 safe)
// Replaces the previous QXmlDefaultHandler / QXmlSimpleReader implementation
// which was deprecated in Qt5 and removed in Qt6.
//
// Behaviour is identical to the original SAX1 parser:
//   <atom id="a1"> <string builtin="elementType">C</string>
//                  <float builtin="x2">1.5</float>
//                  <float builtin="y2">2.3</float> </atom>
//   <bond id="b1"> <string builtin="atomRef">a1</string>
//                  <string builtin="atomRef">a2</string>
//                  <string builtin="order">1</string> </bond>

#include <QList>
#include <QXmlStreamReader>

#include "xml_cml.h"
#include "defs.h"

// ── Public entry point ────────────────────────────────────────────────────────
bool CMLParser::parse(QIODevice *device)
{
    QXmlStreamReader xml(device);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        switch (xml.tokenType()) {
        case QXmlStreamReader::StartElement:
            handleStartElement(xml);
            break;
        case QXmlStreamReader::EndElement:
            handleEndElement(xml);
            break;
        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace())
                handleCharacters(xml);
            break;
        default:
            break;
        }
    }

    if (xml.hasError()) {
        qWarning() << "CMLParser XML error:" << xml.errorString()
                   << "at line" << xml.lineNumber();
        return false;
    }
    return true;
}

// ── Start element ─────────────────────────────────────────────────────────────
void CMLParser::handleStartElement(QXmlStreamReader &xml)
{
    const QString qName = xml.name().toString().toUpper();
    const QXmlStreamAttributes attr = xml.attributes();

    qDebug() << "Start:" << qName;

    if (qName == QLatin1String("ATOM")) {
        states  = CML_ATOM;
        tmp_pt  = new DPoint;
        tmp_pt->id = attr.value(QLatin1String("id")).toString();
        qDebug() << "Atom id=" << tmp_pt->id;
    } else if (qName == QLatin1String("BOND")) {
        states    = CML_BOND;
        tmp_bond  = new Bond(r);
        tmp_bond->setID(attr.value(QLatin1String("id")).toString());
        ep1 = nullptr;
        ep2 = nullptr;
    } else if (qName == QLatin1String("FLOAT")) {
        last_builtin = attr.value(QLatin1String("builtin")).toString().toUpper();
        // normalise 3-D coordinate aliases to 2-D names
        if (last_builtin == QLatin1String("X3")) last_builtin = QLatin1String("X2");
        if (last_builtin == QLatin1String("Y3")) last_builtin = QLatin1String("Y2");
    } else if (qName == QLatin1String("STRING")) {
        last_builtin = attr.value(QLatin1String("builtin")).toString().toUpper();
    }
}

// ── End element ───────────────────────────────────────────────────────────────
void CMLParser::handleEndElement(QXmlStreamReader &xml)
{
    const QString qName = xml.name().toString().toUpper();
    qDebug() << "End:" << qName;

    if (qName == QLatin1String("ATOM")) {
        localPoints.append(tmp_pt);
        tmp_pt = nullptr;
        states = CML_NONE;
        qDebug() << "finished atom";
    } else if (qName == QLatin1String("BOND")) {
        tmp_bond->setPoints(ep1, ep2);
        localBonds.append(tmp_bond);
        tmp_bond = nullptr;
        states   = CML_NONE;
        qDebug() << "finished bond";
    }
}

// ── Characters ────────────────────────────────────────────────────────────────
void CMLParser::handleCharacters(QXmlStreamReader &xml)
{
    const QString ch = xml.text().toString().trimmed();
    if (ch.isEmpty())
        return;

    qDebug() << "char:" << ch << ":";

    if (states == CML_ATOM) {
        if (last_builtin == QLatin1String("ELEMENTTYPE"))
            tmp_pt->element = ch;
        else if (last_builtin == QLatin1String("X2"))
            tmp_pt->x = ch.toDouble();
        else if (last_builtin == QLatin1String("Y2"))
            tmp_pt->y = ch.toDouble();
    } else if (states == CML_BOND) {
        if (last_builtin == QLatin1String("ATOMREF")) {
            // Find the DPoint whose id matches ch
            DPoint *found = nullptr;
            for (DPoint *pt : localPoints) {
                if (pt->id == ch) {
                    found = pt;
                    break;
                }
            }
            if (!ep1)
                ep1 = found;
            else
                ep2 = found;
        } else if (last_builtin == QLatin1String("ORDER")) {
            tmp_bond->setOrder(ch.toInt());
        } else if (last_builtin == QLatin1String("STEREO")) {
            if (ch == QLatin1String("H")) tmp_bond->setOrder(7);  // stereo down
            if (ch == QLatin1String("W")) tmp_bond->setOrder(5);  // stereo up
        }
    }
}

// ── Accessors ─────────────────────────────────────────────────────────────────
QList<DPoint *> CMLParser::getPoints()
{
    return localPoints;
}

QList<Bond *> CMLParser::getBonds()
{
    return localBonds;
}
