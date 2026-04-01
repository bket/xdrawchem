/****************************************************************************
** $Id: helpwindow.cpp,v 1.2 2004/06/08 20:39:37 bherger Exp $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <cctype>

#include <QTextBrowser>
#include <QComboBox>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QUrl>

#include "helpwindow.h"
#include "defs.h"
#include "defs.h"

#include "back.xpm"
#include "forward.xpm"
#include "home.xpm"

HelpWindow::HelpWindow( const QString & home_, const QString & _path, QWidget * parent )
    : QMainWindow( parent ), pathCombo( 0 ), selectedURL()
{
    setWindowIcon( QPixmap( RingDir + "xdrawchem-icon.png" ) );

    browser = new QTextBrowser( this );

//    browser->setSearchPaths( _path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, &QTextBrowser::sourceChanged, this, &HelpWindow::sourceChanged );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
        browser->setSource( home_ );

    connect( browser, &QTextBrowser::highlighted, statusBar(),
        [this](const QUrl &url){ statusBar()->showMessage( url.toString() ); } );

    resize( 640, 700 );

    QMenu *file = new QMenu( tr( "&File" ), this );

    { auto *_a = new QAction( tr( "&Print" ), this ); _a->setShortcut( Qt::CTRL | Qt::Key_P ); connect( _a, &QAction::triggered, this, &HelpWindow::print ); file->addAction( _a ); }
    file->addSeparator();
    { auto *_a = new QAction( tr( "&Close" ), this ); _a->setShortcut( Qt::CTRL | Qt::Key_W ); connect( _a, &QAction::triggered, this, &QWidget::close ); file->addAction( _a ); }

    // The same three icons are used twice each.
    QIcon icon_back( QPixmap( RingDir + "back.xpm" ) );
    QIcon icon_forward( QPixmap( RingDir + "forward.xpm" ) );
    QIcon icon_home( QPixmap( RingDir + "home.xpm" ) );

    QMenu *go = new QMenu( tr( "&Go" ), this );

    { auto *_a = new QAction( icon_back, tr( "&Backward" ), this ); _a->setShortcut( Qt::CTRL | Qt::Key_Left ); connect( _a, &QAction::triggered, browser, &QTextBrowser::backward ); go->addAction( _a ); backwardAction = _a; }
    { auto *_a = new QAction( icon_forward, tr( "&Forward" ), this ); _a->setShortcut( Qt::CTRL | Qt::Key_Right ); connect( _a, &QAction::triggered, browser, &QTextBrowser::forward ); go->addAction( _a ); forwardAction = _a; }
    homeAction = go->addAction( icon_home, tr( "&Home" ), browser, &QTextBrowser::home );

    QMenu *help = new QMenu( tr( "&Help" ), this );

    help->addAction( tr( "&About ..." ), this, &HelpWindow::about );

    menuBar()->addMenu( file );
    menuBar()->addMenu( go );
    menuBar()->addMenu( help );

    forwardAction->setEnabled( false );
    backwardAction->setEnabled( false );

    connect( browser, &QTextBrowser::backwardAvailable, this, &HelpWindow::setBackwardAvailable );
    connect( browser, &QTextBrowser::forwardAvailable, this, &HelpWindow::setForwardAvailable );


    QToolBar *toolbar = new QToolBar( this );

    addToolBar( Qt::TopToolBarArea, toolbar );
//    QToolButton *button;

    toolbar->addAction( backwardAction );
    toolbar->addAction( forwardAction );
    toolbar->addAction( homeAction );

    toolbar->addSeparator();

    pathCombo = new QComboBox( toolbar );
    pathCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    connect( pathCombo, &QComboBox::textActivated, this, &HelpWindow::pathSelected );

    toolbar->addWidget( pathCombo );
    pathCombo->addItem( home_ );
    browser->setFocus();

}


void HelpWindow::setBackwardAvailable( bool b )
{
    backwardAction->setEnabled( b );
}

void HelpWindow::setForwardAvailable( bool b )
{
    forwardAction->setEnabled( b );
}


void HelpWindow::sourceChanged( const QUrl & url )
{
    if ( browser->documentTitle().isNull() )
        setWindowTitle( QString( XDC_VERSION ) + " - Help viewer" );
    else
        setWindowTitle( QString( XDC_VERSION ) + " - Help viewer - " + browser->documentTitle() );

//    selectedURL = browser->context();

    if ( !url.isEmpty() && pathCombo ) {
        int i = pathCombo->findText( url.path() );
        if (  i == -1 ) {
            pathCombo->addItem( url.path(), 0 );
            pathCombo->setCurrentIndex( 0 );
        } else
            pathCombo->setCurrentIndex( i );
//      selectedURL = QString();
    }
}

HelpWindow::~HelpWindow()
{

}

void HelpWindow::about()
{
    QMessageBox::about( this, "HelpViewer Example", "<p>This example implements a simple HTML help viewer " "using Qt's rich text capabilities</p>" "<p>It's just about 100 lines of C++ code, so don't expect too much :-)</p>" );
}

void HelpWindow::print()
{
#ifndef QT_NO_PRINTER
/*    QPrinter printer;
    printer.setFullPage(TRUE);
    if ( printer.setup( this ) ) {
	QPainter p( &printer );
	Q3PaintDeviceMetrics metrics(p.device());
	int dpix = metrics.logicalDpiX();
	int dpiy = metrics.logicalDpiY();
	const int margin = 72; // pt
	QRect body(margin*dpix/72, margin*dpiy/72,
		   metrics.width()-margin*dpix/72*2,
		   metrics.height()-margin*dpiy/72*2 );
	Q3SimpleRichText richText( browser->text(), QFont(), browser->context(), browser->styleSheet(),
				  browser->mimeSourceFactory(), body.height() );
	richText.setWidth( &p, body.width() );
	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);
    }*/
#endif
}

void HelpWindow::pathSelected( const QString & _path )
{
    browser->setSource( _path );
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
