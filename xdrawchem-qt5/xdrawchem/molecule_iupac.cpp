// molecule_iupac.cpp — IUPAC name lookup via PubChem REST API
//
// Uses PubChem's free, unauthenticated REST endpoint:
//   GET https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/smiles/{smiles}/property/IUPACName/TXT
//
// Falls back gracefully if the network is unavailable or the molecule
// is not in PubChem (e.g. non-standard structures).

#include <QNetworkAccessManager>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QEventLoop>
#include <QUrl>
#include <QDebug>

#include "molecule.h"
#include "defs.h"

// Synchronous helper: POST SMILES to PubChem and return the IUPAC name,
// or an empty string on failure.  Uses a local QEventLoop so the Qt
// event loop continues running while we wait (no spin-wait freeze).
QString Molecule::IUPACName()
{
    QString smiles = ToSMILES().trimmed();
    if ( smiles.isEmpty() )
        return QString();

    // PubChem REST: POST to /compound/smiles/property/IUPACName/TXT
    // We use POST so that long SMILES strings don't break URL length limits.
    QNetworkAccessManager nam;
    QEventLoop loop;
    QString result;

    QUrl url( QStringLiteral(
        "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/smiles/property/IUPACName/TXT"
    ) );
    QNetworkRequest req( url );
    req.setHeader( QNetworkRequest::ContentTypeHeader,
                   QStringLiteral("application/x-www-form-urlencoded") );

    QUrlQuery postData;
    postData.addQueryItem( QStringLiteral("smiles"), smiles );

    QNetworkReply *reply = nam.post( req, postData.toString(QUrl::FullyEncoded).toUtf8() );

    QObject::connect( reply, &QNetworkReply::finished, &loop, [&]() {
        if ( reply->error() == QNetworkReply::NoError ) {
            result = QString::fromUtf8( reply->readAll() ).trimmed();
        } else {
            qWarning() << "IUPACName: network error:" << reply->errorString();
        }
        loop.quit();
    });

    // 10-second timeout guard
    QTimer::singleShot( 10000, &loop, &QEventLoop::quit );
    loop.exec();

    reply->deleteLater();
    return result;
}
