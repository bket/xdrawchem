// xml_cml.h -- CML SAX parser, modernised to use QXmlStreamReader (Qt5/6 safe)
// Previously used QXmlDefaultHandler / QXmlSimpleReader (deprecated in Qt5,
// removed in Qt6).  Now parses via QXmlStreamReader directly.

#ifndef XML_CML_H
#define XML_CML_H

#include <QList>
#include <QXmlStreamReader>

#include "drawable.h"
#include "dpoint.h"
#include "bond.h"
#include "render2d.h"

class QString;

class CMLParser
{
public:
    // Init order must match member declaration order below
    // (tmp_pt, ep1, ep2, tmp_bond, r, states) to silence -Wreorder.
    explicit CMLParser(Render2D *r1)
        : tmp_pt(nullptr), ep1(nullptr), ep2(nullptr),
          tmp_bond(nullptr), r(r1), states(0) {}

    // Parse CML from an already-open device (e.g. QFile).
    // Returns true on success, false on XML error.
    bool parse(QIODevice *device);

    QList<DPoint *> getPoints();
    QList<Bond *>   getBonds();

private:
    void handleStartElement(QXmlStreamReader &xml);
    void handleEndElement(QXmlStreamReader &xml);
    void handleCharacters(QXmlStreamReader &xml);

    QList<DPoint *> localPoints;
    QList<Bond *>   localBonds;
    DPoint *tmp_pt, *ep1, *ep2;
    Bond   *tmp_bond;
    Render2D *r;
    QString last_builtin;
    int states;
};

#endif // XML_CML_H
