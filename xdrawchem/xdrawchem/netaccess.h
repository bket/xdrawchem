#ifndef NETACCESS_H
#define NETACCESS_H

// netaccess.h — PubChem REST backend for structure search and name lookup.
//
// Replaces the defunct woodsidelabs.com/newdb.php endpoint.
// All network calls use the HTTP helper class (QNetworkAccessManager +
// QEventLoop — no spin-wait, no QDialog base class required).
//
// Public API is unchanged so callers (chemdata_tools.cpp, application.cpp)
// need no modification.

#include <QObject>
#include <QStringList>
#include <QString>

class NetAccess : public QObject
{
    Q_OBJECT

public:
    explicit NetAccess(QObject *parent = nullptr);

    // Search PubChem by name, CAS number, or molecular formula.
    // Returns a QStringList; each entry is pipe-separated:
    //   cas|iupacname|formula|synonyms|smiles
    // The 'server' parameter is ignored (kept for API compatibility).
    // 'exact' is forwarded to PubChem as-is; name searches use the
    // /compound/name/ endpoint which handles synonyms automatically.
    QStringList getChoices( QString server, QString key, QString value, bool exact );

    // Look up CAS number, IUPAC name, and synonyms for a drawn molecule
    // by submitting its InChI string to PubChem.
    // Populates: spccompound (CID), scas, siupacname, sname.
    // Returns true on success, false if PubChem returned no match.
    bool getNameCAS( QString server, QString sinchi );

    // Reorder formula atoms into Hill order (C first, H second, rest alpha).
    QString Rearrange( QString key );

    // Results populated by getNameCAS():
    QString spccompound;   // PubChem CID (as string)
    QString scas;          // CAS registry number
    QString siupacname;    // IUPAC preferred name
    QString sname;         // pipe-separated synonyms

    bool status;

signals:
    void choicesFinished( const QStringList & );

private:
    // Synchronous GET — blocks via QEventLoop until reply arrives.
    QString httpGet( const QString &url );
    // Synchronous POST (application/x-www-form-urlencoded body).
    QString httpPost( const QString &url, const QString &body );
    // Extract first CAS-looking synonym from a PubChem synonyms array.
    QString extractCAS( const QStringList &synonyms );
};

#endif

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
