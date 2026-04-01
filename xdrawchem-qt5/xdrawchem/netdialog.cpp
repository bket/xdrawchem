// netdialog.cpp — "Find structure on PubChem" dialog.
//
// Simplified from the original: the "XDC database server" URL field has
// been removed because PubChem is now the fixed backend and is not
// user-configurable.

#include <QLineEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

#include "netdialog.h"

NetDialog::NetDialog( QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( tr( "Find structure on PubChem" ) );

    QGridLayout *mygrid = new QGridLayout( this );

    QLabel *caption = new QLabel( tr( "Search type:" ) );
    mygrid->addWidget( caption, 0, 0, 1, 4 );

    keylist = new QComboBox();
    keylist->addItem( tr( "Chemical name" ) );
    keylist->addItem( tr( "CAS Number" ) );
    keylist->addItem( tr( "Molecular formula" ) );
    mygrid->addWidget( keylist, 0, 4, 1, 4 );

    QLabel *l1 = new QLabel( tr( "Look for:" ), this );
    mygrid->addWidget( l1, 1, 0, 1, 4 );

    searchkey = new QLineEdit();
    mygrid->addWidget( searchkey, 1, 4, 1, 4 );
    connect( searchkey, &QLineEdit::returnPressed, this, &QDialog::accept );
    searchkey->setFocus();

    emcheck = new QCheckBox( tr( "Exact matches only" ) );
    mygrid->addWidget( emcheck, 2, 0, 1, 8 );

    QHBoxLayout *buttonHBox = new QHBoxLayout();
    QSpacerItem *spacer = new QSpacerItem( 1, 1, QSizePolicy::Expanding,
                                           QSizePolicy::Minimum );
    buttonHBox->addItem( spacer );

    QPushButton *ok = new QPushButton( tr( "Search" ) );
    connect( ok, &QAbstractButton::clicked, this, &QDialog::accept );
    buttonHBox->addWidget( ok );

    QPushButton *dismiss = new QPushButton( tr( "Cancel" ) );
    connect( dismiss, &QAbstractButton::clicked, this, &QDialog::reject );
    buttonHBox->addWidget( dismiss );

    mygrid->addLayout( buttonHBox, 3, 0, 1, 8 );
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
