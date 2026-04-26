#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QString>
#include <QList>

#include "molecule.h"
#include "peak.h"

// Forward-declare OBMol to avoid pulling all OpenBabel headers into every
// translation unit that includes this header.
namespace OpenBabel { class OBMol; }

class ToolDialog : public QDialog
{
    Q_OBJECT

public:
    ToolDialog( QWidget *parent = nullptr );
    void setMolecule( Molecule * );
    void setMolecule( OpenBabel::OBMol * );
    void setProductMolecule( Molecule * );
    void setProductMolecule( OpenBabel::OBMol * );
    virtual void process();

public slots:
    void SendHelp();

protected:
    QString helpfile;
    Molecule *this_mol, *product_mol;
    QList<Peak *> peaklist;
    Peak *tmp_peak;
};

#endif

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
