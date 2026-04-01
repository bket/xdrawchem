#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>

#include "netchoosedialog.h"
#include "defs.h"

NetChooseDialog::NetChooseDialog( QWidget * parent, QStringList r1 )
    : QDialog( parent )
{
    results = r1;

    QPushButton *ok, *dismiss;
    QLabel *cap1;

    setWindowTitle( tr("Choose Structure") );

    QGridLayout *mygrid = new QGridLayout( this );

    cap1 = new QLabel( tr("Select a molecule from list and click OK:"), this );
    mygrid->addWidget( cap1, 0, 0, 1, 4 );

    tw = new QTableWidget( this );
    connect( tw, &QTableWidget::cellDoubleClicked, this, &NetChooseDialog::cellDoubleClicked );
    tw->setRowCount( r1.size() );
    tw->setColumnCount(5);
    QStringList m_TableHeader;
    m_TableHeader << "CAS" << "IUPAC Name" << "Formula" << "Other names" << "SMILES";
    tw->setHorizontalHeaderLabels(m_TableHeader);
    tw->verticalHeader()->setVisible(false);
    tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw->setSelectionBehavior(QAbstractItemView::SelectRows);
    tw->setSelectionMode(QAbstractItemView::SingleSelection);
    tw->setShowGrid(false);
//  lv->setSorting(0); ///TODO; this should be set after loading data anyways.
    mygrid->addWidget( tw, 1, 0, 3, 4 );

    // Parse pipe-separated rows produced by NetAccess::getChoices():
    //   cas|iupacname|formula|synonyms|smiles
    int iRow = 0;
    for ( const QString &row : r1 ) {
        QStringList fields = row.split( QLatin1Char('|') );
        while (fields.size() < 5) fields << QString();
        tw->setItem(iRow, 0, new QTableWidgetItem( fields[0] ));  // CAS
        tw->setItem(iRow, 1, new QTableWidgetItem( fields[1] ));  // IUPAC name
        tw->setItem(iRow, 2, new QTableWidgetItem( fields[2] ));  // formula
        tw->setItem(iRow, 3, new QTableWidgetItem( fields[3] ));  // synonyms
        tw->setItem(iRow, 4, new QTableWidgetItem( fields[4] ));  // SMILES
        ++iRow;
        /*lvi = new QListWidgetItem( lv );
        lvi->setText( tcas );
        lvi->setText( tname );
        lvi->setText( tformula );
        lvi->setText( taltname );
        lvi->setText( tformat );*/
        //lv->addItem(lvi);
    }

    QHBoxLayout *buttonHBox = new QHBoxLayout( this );
    QSpacerItem *spacer = new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );

    buttonHBox->addItem( spacer );

    ok = new QPushButton( tr( "Select" ), this );
    connect( ok, &QAbstractButton::clicked, this, &NetChooseDialog::OK );
    buttonHBox->addWidget( ok );

    dismiss = new QPushButton( tr( "Cancel" ), this );
    connect( dismiss, &QAbstractButton::clicked, this, &QDialog::reject );
    buttonHBox->addWidget( dismiss );

    mygrid->addLayout( buttonHBox, 4, 0, 1, 4 );
}

void NetChooseDialog::OK()
{
    // save SMILES string of selected item
    int iRow = tw->currentItem()->row();
    fn = tw->item(iRow, 4)->text();
    fn.replace( "\"", "" );     // strip quotes
	qInfo() << "OK: fn = " << fn << ", row = " << iRow;
    accept();
}

void NetChooseDialog::cellDoubleClicked(int row, int col)
{
    // save SMILES string of selected item
    fn = tw->item(row, 4)->text();
    fn.replace( "\"", "" );     // strip quotes
	qInfo() << "dblClick: fn = " << fn << ", row = " << row;
    accept();
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
