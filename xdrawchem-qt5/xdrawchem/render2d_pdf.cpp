// render2d_pdf.cpp — PDF export using Qt's built-in PDF backend
//
// Renders the current canvas to a vector-quality PDF file suitable for
// publication and presentations.  Uses QPrinter::PdfFormat (Qt6 built-in)
// and scales the canvas pixmap to fill the page while preserving aspect ratio.

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPageSize>
#include <QPageLayout>

#include "render2d.h"
#include "chemdata.h"
#include "defs.h"

void Render2D::ExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr( "Export as PDF" ),
        QString(),
        tr( "PDF files (*.pdf)" )
    );
    if ( fileName.isEmpty() )
        return;

    if ( !fileName.endsWith( QLatin1String(".pdf"), Qt::CaseInsensitive ) )
        fileName += QLatin1String(".pdf");

    // Choose page orientation based on canvas aspect ratio
    QPixmap pm = MakeFullPixmap();
    QPageLayout::Orientation orient =
        ( pm.width() >= pm.height() )
        ? QPageLayout::Landscape
        : QPageLayout::Portrait;

    QPrinter pdfPrinter( QPrinter::HighResolution );
    pdfPrinter.setOutputFormat( QPrinter::PdfFormat );
    pdfPrinter.setOutputFileName( fileName );
    pdfPrinter.setPageSize( QPageSize( QPageSize::A4 ) );
    pdfPrinter.setPageOrientation( orient );
    pdfPrinter.setColorMode( QPrinter::Color );
    pdfPrinter.setPageMargins(
        QMarginsF( 10, 10, 10, 10 ),
        QPageLayout::Millimeter
    );

    QPainter painter;
    if ( !painter.begin( &pdfPrinter ) ) {
        QMessageBox::warning(
            this, tr( "PDF Export" ),
            tr( "Could not open file for writing:\n%1" ).arg( fileName )
        );
        return;
    }

    // Scale pixmap to page, preserving aspect ratio
    QRect pageRect = pdfPrinter.pageRect( QPrinter::DevicePixel ).toRect();
    QPixmap scaled = pm.scaled(
        pageRect.size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    // Centre on page
    int x = ( pageRect.width()  - scaled.width()  ) / 2;
    int y = ( pageRect.height() - scaled.height() ) / 2;
    painter.drawPixmap( x, y, scaled );

    painter.end();

    emit SignalSetStatusBar( tr( "Exported PDF: " ) + fileName );
}
