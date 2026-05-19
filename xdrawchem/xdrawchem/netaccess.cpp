// netaccess.cpp — PubChem REST backend (replaces defunct woodsidelabs.com)
//
// PubChem PUG REST API:
//   Name/CAS search:  GET /rest/pug/compound/name/{name}/property/.../JSON
//   Formula search:   GET /rest/pug/compound/fastformula/{formula}/property/.../JSON
//   InChI lookup:     POST /rest/pug/compound/inchi/property/.../JSON
//                       body: inchi=InChI%3D1S%2F...
//   Synonyms:         GET /rest/pug/compound/cid/{cid}/synonyms/JSON
//
// Internal list format passed to NetChooseDialog (pipe-separated):
//   cas|iupacname|formula|synonyms|smiles

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QDebug>

#include "xdc_logging.h"

#include "netaccess.h"

static const QString PUBCHEM = QStringLiteral("https://pubchem.ncbi.nlm.nih.gov/rest/pug");

// ── Constructor ───────────────────────────────────────────────────────────────
NetAccess::NetAccess(QObject *parent)
    : QObject(parent)
    , status(true)
    , spccompound(QStringLiteral("unknown"))
    , scas(QStringLiteral("unknown"))
    , siupacname(QStringLiteral("unknown"))
    , sname(QStringLiteral("unknown"))
{}

// ── Private helpers ───────────────────────────────────────────────────────────

QString NetAccess::httpGet(const QString &url)
{
    QNetworkAccessManager mgr;
    QEventLoop loop;
    QString result;

    QUrl reqUrl(url);
    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("XDrawChem/2.0 (https://github.com/bryanherger/xdrawchem)"));

    QNetworkReply *reply = mgr.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError)
        result = QString::fromUtf8(reply->readAll());
    else
        qCWarning(lcNetwork) << "NetAccess GET error:" << reply->errorString() << url;

    reply->deleteLater();
    return result;
}

QString NetAccess::httpPost(const QString &url, const QString &body)
{
    QNetworkAccessManager mgr;
    QEventLoop loop;
    QString result;

    QUrl reqUrl(url);
    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  QStringLiteral("application/x-www-form-urlencoded"));
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("XDrawChem/2.0 (https://github.com/bryanherger/xdrawchem)"));

    QNetworkReply *reply = mgr.post(req, body.toUtf8());
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError)
        result = QString::fromUtf8(reply->readAll());
    else
        qCWarning(lcNetwork) << "NetAccess POST error:" << reply->errorString() << url;

    reply->deleteLater();
    return result;
}

// Returns the first synonym that matches a CAS registry number pattern
// (digits, exactly one dash after 2-7 digits, another dash, 1 check digit).
QString NetAccess::extractCAS(const QStringList &synonyms)
{
    static const QRegularExpression cas(QStringLiteral("^\\d{1,7}-\\d{2}-\\d$"));
    for (const QString &s : synonyms)
        if (cas.match(s.trimmed()).hasMatch())
            return s.trimmed();
    return QStringLiteral("N/A");
}

// ── getChoices: search PubChem by name, CAS, or formula ──────────────────────
//
// Returns a QStringList; each entry is pipe-separated:
//   cas|iupacname|formula|synonyms|smiles
// NetChooseDialog expects exactly this format (see netchoosedialog.cpp).
// The 'server' and 'exact' parameters are kept for API compatibility;
// PubChem's /compound/name/ endpoint already resolves synonyms and CAS
// numbers automatically without a separate exact/fuzzy mode.

QStringList NetAccess::getChoices(QString /*server*/, QString key,
                                  QString value, bool /*exact*/)
{
    QStringList results;

    if (value.trimmed().isEmpty())
        return results;

    // Build the property URL depending on search type.
    QString propUrl;
    if (key == QStringLiteral("formula")) {
        value = Rearrange(value);
        propUrl = PUBCHEM + QStringLiteral("/compound/fastformula/")
                  + QUrl::toPercentEncoding(value)
                  + QStringLiteral("/property/IsomericSMILES,IUPACName,MolecularFormula/JSON");
    } else {
        // key == "name" or "cas" — PubChem /name/ handles both transparently
        propUrl = PUBCHEM + QStringLiteral("/compound/name/")
                  + QUrl::toPercentEncoding(value)
                  + QStringLiteral("/property/IsomericSMILES,IUPACName,MolecularFormula/JSON");
    }

    qCDebug(lcNetwork) << "NetAccess::getChoices GET" << propUrl;
    QString json = httpGet(propUrl);
    if (json.isEmpty())
        return results;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull() || !doc.object().contains(QStringLiteral("PropertyTable")))
        return results;

    QJsonArray props = doc.object()
                          .value(QStringLiteral("PropertyTable")).toObject()
                          .value(QStringLiteral("Properties")).toArray();

    // Limit to first 50 hits to avoid overwhelming the dialog.
    int limit = qMin(props.size(), 50);
    for (int i = 0; i < limit; ++i) {
        QJsonObject p = props[i].toObject();
        QString cid     = QString::number(p.value(QStringLiteral("CID")).toInt());
        QString smiles  = p.value(QStringLiteral("IsomericSMILES")).toString();
        QString iupac   = p.value(QStringLiteral("IUPACName")).toString();
        QString formula = p.value(QStringLiteral("MolecularFormula")).toString();

        // Fetch synonyms for CAS number and alternate names.
        QString synUrl  = PUBCHEM + QStringLiteral("/compound/cid/") + cid
                          + QStringLiteral("/synonyms/JSON");
        QString synJson = httpGet(synUrl);
        QStringList synList;
        QString casNum = QStringLiteral("N/A");

        if (!synJson.isEmpty()) {
            QJsonDocument sd = QJsonDocument::fromJson(synJson.toUtf8());
            QJsonArray syns  = sd.object()
                                  .value(QStringLiteral("InformationList")).toObject()
                                  .value(QStringLiteral("Information")).toArray();
            if (!syns.isEmpty()) {
                QJsonArray sa = syns[0].toObject()
                                       .value(QStringLiteral("Synonym")).toArray();
                for (const QJsonValue &sv : sa)
                    synList << sv.toString();
                casNum = extractCAS(synList);
            }
        }

        // Join first 5 synonyms as the "other names" column.
        QString altNames = synList.mid(0, 5).join(QStringLiteral("; "));

        // Row format: cas|iupacname|formula|synonyms|smiles
        results << casNum + QLatin1Char('|')
                   + iupac + QLatin1Char('|')
                   + formula + QLatin1Char('|')
                   + altNames + QLatin1Char('|')
                   + smiles;
    }

    return results;
}

// ── getNameCAS: look up name/CAS for a drawn molecule via its InChI ───────────
//
// Submits the InChI string to PubChem and populates:
//   spccompound  — PubChem CID (as string)
//   scas         — CAS registry number
//   siupacname   — IUPAC preferred name
//   sname        — first few synonyms joined by "; "

bool NetAccess::getNameCAS(QString /*server*/, QString sinchi)
{
    sinchi = sinchi.trimmed();
    if (sinchi.length() < 2)
        return false;

    qCDebug(lcNetwork) << "NetAccess::getNameCAS InChI:" << sinchi.left(40) << "...";

    // POST the InChI string to PubChem.
    QString propUrl = PUBCHEM
                      + QStringLiteral("/compound/inchi/property/IUPACName,MolecularFormula/JSON");
    QString body = QStringLiteral("inchi=")
                   + QUrl::toPercentEncoding(sinchi);

    QString json = httpPost(propUrl, body);
    if (json.isEmpty())
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull() || !doc.object().contains(QStringLiteral("PropertyTable")))
        return false;

    QJsonArray props = doc.object()
                          .value(QStringLiteral("PropertyTable")).toObject()
                          .value(QStringLiteral("Properties")).toArray();
    if (props.isEmpty())
        return false;

    QJsonObject p = props[0].toObject();
    QString cid   = QString::number(p.value(QStringLiteral("CID")).toInt());
    siupacname    = p.value(QStringLiteral("IUPACName")).toString();
    spccompound   = cid;

    // Fetch synonyms for CAS and alternate names.
    QString synUrl  = PUBCHEM + QStringLiteral("/compound/cid/") + cid
                      + QStringLiteral("/synonyms/JSON");
    QString synJson = httpGet(synUrl);

    QStringList synList;
    if (!synJson.isEmpty()) {
        QJsonDocument sd = QJsonDocument::fromJson(synJson.toUtf8());
        QJsonArray syns  = sd.object()
                              .value(QStringLiteral("InformationList")).toObject()
                              .value(QStringLiteral("Information")).toArray();
        if (!syns.isEmpty()) {
            QJsonArray sa = syns[0].toObject()
                                   .value(QStringLiteral("Synonym")).toArray();
            for (const QJsonValue &sv : sa)
                synList << sv.toString();
        }
    }

    scas  = extractCAS(synList);
    sname = synList.mid(0, 8).join(QStringLiteral("; "));

    qCDebug(lcNetwork) << "getNameCAS result: CID" << spccompound
            << "CAS" << scas << "Name" << siupacname;
    return true;
}

// ── Rearrange: put formula atoms in Hill order (C, H, then alpha) ─────────────
QString NetAccess::Rearrange(QString key)
{
    QString key1, subkey;
    bool addflag;
    QStringList allatoms;

    do {
        subkey   = QString(key[0]);
        addflag  = false;
        for (int cc = 1; cc < key.length(); ++cc) {
            if (key[cc].category() == QChar::Letter_Uppercase) {
                allatoms.append(subkey);
                key.remove(0, cc);
                addflag = true;
                break;
            } else {
                subkey.append(key[cc]);
            }
        }
        if (!addflag) {
            allatoms.append(subkey);
            key.remove(0, key.length());
        }
    } while (key.length() > 0);

    allatoms.sort();
    for (const QString &n1 : allatoms)
        if (n1.contains(QLatin1Char('C'))) key1.append(n1);
    for (const QString &n1 : allatoms)
        if (n1.contains(QLatin1Char('H'))) key1.append(n1);
    for (const QString &n1 : allatoms)
        if (!n1.contains(QLatin1Char('C')) && !n1.contains(QLatin1Char('H')))
            key1.append(n1);

    return key1;
}
