#include <QGridLayout>

#include "tool_ir.h"
#include "render2d.h"
#include "helpwindow.h"
#include "defs.h"

// defined in main.cpp
//extern QString RingDir;

Tool_IR_Dialog::Tool_IR_Dialog( QWidget *parent )
    : ToolDialog( parent )
{
    setWindowTitle( tr( "IR spectrum" ) );
    helpfile = "spectra.html";
    QGridLayout *mygrid;

    mygrid = new QGridLayout( this );
    ngw = new NewGraphWidget( this );
    ngw->setDataType( DATA_IR );
    mygrid->addWidget( ngw, 0, 0 );
    peaktext = new QTextEdit();
    peaktext->setReadOnly( true );
    mygrid->addWidget( peaktext, 1, 1 );
    QPushButton *qhelp = new QPushButton( tr( "Help" ) );

    mygrid->addWidget( qhelp, 2, 0 );
    connect( qhelp, &QAbstractButton::clicked, this, &Tool_IR_Dialog::SendHelp );
    QPushButton *qclose = new QPushButton( tr( "Close" ) );

    mygrid->addWidget( qclose, 2, 1 );
    connect( qclose, &QAbstractButton::clicked, this, &QDialog::accept );
}

void Tool_IR_Dialog::process()
{
    this_mol->CalcIR();
    peaklist = this_mol->peaklist;
    qDebug() << "got peak list";
    if ( product_mol == 0 ) {
        for (Peak *tmp_peak : peaklist) {
            ngw->AddPeak( tmp_peak, QColor( 0, 0, 0 ) );
            peaktext->append( tmp_peak->comment );
            //peaktext->append("\n");
        }
        return;
    }
    peaktext->setTextColor( QColor( 255, 0, 0 ) );
    peaktext->insertPlainText( tr( "Peaks of reactant:\n" ) );
    for (Peak *tmp_peak : peaklist) {
        ngw->AddPeak( tmp_peak, QColor( 255, 0, 0 ) );
        peaktext->insertPlainText( tmp_peak->comment );
        peaktext->insertPlainText( "\n" );
    }
    product_mol->CalcIR();
    peaklist.clear();
    peaklist = product_mol->peaklist;
    peaktext->setTextColor( QColor( 0, 0, 255 ) );
    peaktext->insertPlainText( tr( "Peaks of product:\n" ) );
    for (Peak *tmp_peak : peaklist) {
        ngw->AddPeak( tmp_peak, QColor( 0, 0, 255 ) );
        peaktext->insertPlainText( tmp_peak->comment );
        peaktext->insertPlainText( "\n" );
    }
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
