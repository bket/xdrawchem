// molecule_nts.cpp — Name-to-Structure via PubChem REST API
//
// Uses PubChem's free, unauthenticated REST endpoint:
//   GET https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/{name}/property/IsomericSMILES/TXT
//
// Accepts IUPAC systematic names, common names, trade names, CAS numbers —
// anything PubChem can resolve.  Returns an isomeric SMILES on success, or
// an empty string if the name is not found or network is unavailable.
//
// The result is passed directly to ChemData::fromSMILES(), which already
// handles SMILES through OpenBabel (including 2-D coordinate generation).

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QUrl>
#include <QDebug>

#include "molecule_nts.h"
#include "molecule.h"

// Synchronous: resolve a chemical name to a SMILES string via PubChem.
// Uses a local QEventLoop (no spin-wait freeze) with a 15-second timeout.
QString nameToSMILES( const QString &name )
{
    if ( name.trimmed().isEmpty() )
        return QString();

    // URL-encode the name and embed it in the PubChem PUG REST path.
    QString encoded = QString::fromUtf8(
        QUrl::toPercentEncoding( name.trimmed() ) );

    QUrl url( QStringLiteral(
        "https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/name/" )
        + encoded
        + QStringLiteral( "/property/IsomericSMILES/TXT" ) );

    QNetworkAccessManager nam;
    QEventLoop loop;
    QString result;

    QNetworkRequest req( url );
    req.setHeader( QNetworkRequest::UserAgentHeader,
                   QStringLiteral( "XDrawChem/2.0 (name-to-structure)" ) );

    QNetworkReply *reply = nam.get( req );

    QObject::connect( reply, &QNetworkReply::finished, &loop, [&]() {
        int status = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute ).toInt();
        if ( reply->error() == QNetworkReply::NoError && status == 200 ) {
            result = QString::fromUtf8( reply->readAll() ).trimmed();
        } else if ( status == 404 ) {
            // PubChem returns 404 for unknown names — not an error worth logging.
            qDebug() << "nameToSMILES: name not found in PubChem:" << name;
        } else {
            qWarning() << "nameToSMILES: HTTP" << status
                       << reply->errorString() << "for name:" << name;
        }
        loop.quit();
    } );

    // 15-second timeout (PubChem can be slow under load)
    QTimer::singleShot( 15000, &loop, &QEventLoop::quit );
    loop.exec();

    reply->deleteLater();
    return result;
}
