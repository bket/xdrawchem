// sdfbrowser.cpp — Multi-record SDF browser dialog
//
// Parses an SDF file into individual records, displays a list with
// molecule names, and lets the user preview, import single records,
// or import all records into the current document.

#include <QFile>
#include <QByteArray>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

#include "sdfbrowser.h"
#include "chemdata.h"
#include "render2d.h"
#include "molecule.h"
#include "xdc_logging.h"

#include "ob_compat.h"
OB_COMPAT_BEGIN
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
OB_COMPAT_END

SDFBrowser::SDFBrowser( QWidget *parent, const QString &fileName,
                        ChemData *chemData, Render2D *render2D )
    : QDialog( parent )
    , m_fileName( fileName )
    , m_chemData( chemData )
    , m_render2D( render2D )
{
    setWindowTitle( tr( "SDF Browser — %1" ).arg( fileName ) );
    setMinimumSize( 600, 400 );

    QGridLayout *grid = new QGridLayout( this );

    // Status label
    m_statusLabel = new QLabel( tr( "Reading file…" ) );
    grid->addWidget( m_statusLabel, 0, 0, 1, 4 );

    // Table of records
    m_table = new QTableWidget( this );
    m_table->setColumnCount( 3 );
    QStringList headers;
    headers << tr( "#" ) << tr( "Name" ) << tr( "Size (bytes)" );
    m_table->setHorizontalHeaderLabels( headers );
    m_table->horizontalHeader()->setStretchLastSection( true );
    m_table->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
    m_table->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_table->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_table->setSelectionMode( QAbstractItemView::SingleSelection );
    m_table->setAlternatingRowColors( true );
    connect( m_table, &QTableWidget::cellClicked, this, &SDFBrowser::rowClicked );
    connect( m_table, &QTableWidget::cellDoubleClicked, this, &SDFBrowser::rowDoubleClicked );
    grid->addWidget( m_table, 1, 0, 1, 4 );

    // Button row
    QHBoxLayout *btnLayout = new QHBoxLayout();

    m_importBtn = new QPushButton( tr( "&Import This Record" ) );
    connect( m_importBtn, &QPushButton::clicked, this, &SDFBrowser::importCurrent );
    btnLayout->addWidget( m_importBtn );

    m_importAllBtn = new QPushButton( tr( "Import &All" ) );
    connect( m_importAllBtn, &QPushButton::clicked, this, &SDFBrowser::importAll );
    btnLayout->addWidget( m_importAllBtn );

    m_skipBtn = new QPushButton( tr( "&Skip" ) );
    connect( m_skipBtn, &QPushButton::clicked, this, &SDFBrowser::skipCurrent );
    btnLayout->addWidget( m_skipBtn );

    btnLayout->addStretch();

    m_prevBtn = new QPushButton( tr( "<< &Previous" ) );
    connect( m_prevBtn, &QPushButton::clicked, this, &SDFBrowser::goPrevious );
    btnLayout->addWidget( m_prevBtn );

    m_nextBtn = new QPushButton( tr( "&Next >>" ) );
    connect( m_nextBtn, &QPushButton::clicked, this, &SDFBrowser::goNext );
    btnLayout->addWidget( m_nextBtn );

    m_doneBtn = new QPushButton( tr( "&Done" ) );
    connect( m_doneBtn, &QPushButton::clicked, this, &QDialog::accept );
    btnLayout->addWidget( m_doneBtn );

    grid->addLayout( btnLayout, 2, 0, 1, 4 );

    setLayout( grid );

    // Parse the file
    parseRecords();
    updateUI();
}

void SDFBrowser::parseRecords()
{
    QFile f( m_fileName );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        m_statusLabel->setText( tr( "Error: could not open file." ) );
        return;
    }

    QByteArray data = f.readAll();
    f.close();

    // Split into records at "$$$$" lines.
    // The $$$$ delimiter may have trailing whitespace; we strip it.
    QList<QByteArray> rawRecords = data.split( '$' );
    // Re-assemble: split on '$' gives us fragments; we look for "$$$\n" sequences
    // Simpler approach: scan line by line for lines that are exactly "$$$$"
    QByteArray currentRecord;
    int recordStart = 0;
    int lineStart = 0;
    int recordCount = 0;

    m_records.clear();

    for ( int i = 0; i <= data.size(); ++i ) {
        bool atEnd = ( i == data.size() );
        bool atLineEnd = atEnd || ( data[i] == '\n' );

        if ( atLineEnd ) {
            int lineLen = i - lineStart;
            if ( lineLen >= 4 ) {
                QByteArray line = data.mid( lineStart, lineLen ).trimmed();
                if ( line == "$$$$" ) {
                    // End of current record
                    int recLen = lineStart - recordStart;
                    if ( recLen > 0 ) {
                        QByteArray recData = data.mid( recordStart, recLen );
                        // Extract name from first line of MOL block
                        int nameEnd = recData.indexOf('\n');
                        QString name;
                        if ( nameEnd > 0 )
                            name = QString::fromLatin1( recData.left( nameEnd ).trimmed() );
                        if ( name.isEmpty() )
                            name = tr( "Record %1" ).arg( recordCount + 1 );

                        SDFRecord rec;
                        rec.name = name;
                        rec.startOffset = recordStart;
                        rec.length = recLen;
                        m_records.append( rec );
                        ++recordCount;
                    }
                    recordStart = i + 1; // start next record after newline
                }
            }
            lineStart = i + 1;
        }
    }

    // Handle last record if file doesn't end with $$$$
    if ( recordStart < data.size() ) {
        int recLen = data.size() - recordStart;
        QByteArray recData = data.mid( recordStart, recLen );
        int nameEnd = recData.indexOf('\n');
        QString name;
        if ( nameEnd > 0 )
            name = QString::fromLatin1( recData.left( nameEnd ).trimmed() );
        if ( name.isEmpty() )
            name = tr( "Record %1" ).arg( recordCount + 1 );

        SDFRecord rec;
        rec.name = name;
        rec.startOffset = recordStart;
        rec.length = recLen;
        m_records.append( rec );
        ++recordCount;
    }

    // Populate table
    m_table->setRowCount( m_records.count() );
    for ( int i = 0; i < m_records.count(); ++i ) {
        m_table->setItem( i, 0, new QTableWidgetItem( QString::number( i + 1 ) ) );
        m_table->setItem( i, 1, new QTableWidgetItem( m_records[i].name ) );
        m_table->setItem( i, 2, new QTableWidgetItem( QString::number( m_records[i].length ) ) );
    }

    if ( m_records.isEmpty() ) {
        m_statusLabel->setText( tr( "No records found in file." ) );
    } else {
        m_statusLabel->setText( tr( "%1 record(s) found." ).arg( m_records.count() ) );
    }
}

void SDFBrowser::updateUI()
{
    bool hasRecords = !m_records.isEmpty();
    m_importBtn->setEnabled( hasRecords );
    m_importAllBtn->setEnabled( hasRecords && m_records.count() > 1 );
    m_skipBtn->setEnabled( hasRecords );
    m_prevBtn->setEnabled( hasRecords && m_currentIndex > 0 );
    m_nextBtn->setEnabled( hasRecords && m_currentIndex < m_records.count() - 1 );

    if ( hasRecords ) {
        m_table->selectRow( m_currentIndex );
        m_statusLabel->setText( tr( "Record %1 of %2: %3" )
                                .arg( m_currentIndex + 1 )
                                .arg( m_records.count() )
                                .arg( m_records[m_currentIndex].name ) );
    }
}

bool SDFBrowser::loadRecord( int index )
{
    if ( index < 0 || index >= m_records.count() )
        return false;

    QFile f( m_fileName );
    if ( !f.open( QIODevice::ReadOnly ) )
        return false;

    f.seek( m_records[index].startOffset );
    QByteArray recData = f.read( m_records[index].length );
    f.close();

    using namespace OpenBabel;

    std::istringstream iss( recData.constData() );
    OBMol obmol;
    OBConversion conv;
    if ( !conv.SetInFormat( "sdf" ) )
        return false;

    bool ok = conv.Read( &obmol, &iss );
    if ( !ok || obmol.NumAtoms() == 0 )
        return false;

    Molecule *mol = new Molecule( m_render2D, m_chemData );
    if ( !mol->convertFromOBMol( &obmol ) ) {
        delete mol;
        return false;
    }

    m_chemData->addMolecule( mol );
    qCDebug(lcMDL) << "SDFBrowser: imported record" << (index + 1) << "with"
                     << obmol.NumAtoms() << "atoms";
    return true;
}

void SDFBrowser::previewRecord( int index )
{
    if ( index < 0 || index >= m_records.count() )
        return;
    m_currentIndex = index;
    updateUI();
}

void SDFBrowser::rowClicked( int row )
{
    previewRecord( row );
}

void SDFBrowser::rowDoubleClicked( int row )
{
    previewRecord( row );
    importCurrent();
}

void SDFBrowser::importCurrent()
{
    if ( m_currentIndex < 0 || m_currentIndex >= m_records.count() )
        return;

    if ( loadRecord( m_currentIndex ) ) {
        m_imported = true;
        m_chemData->DeselectAll();
        m_render2D->update();

        // Move to next record for convenience
        if ( m_currentIndex < m_records.count() - 1 ) {
            ++m_currentIndex;
            updateUI();
        }
    } else {
        QMessageBox::warning( this, tr( "Import failed" ),
                              tr( "Could not import record %1." ).arg( m_currentIndex + 1 ) );
    }
}

void SDFBrowser::importAll()
{
    int imported = 0;
    int failed = 0;

    for ( int i = 0; i < m_records.count(); ++i ) {
        if ( loadRecord( i ) ) {
            ++imported;
        } else {
            ++failed;
        }
    }

    m_imported = ( imported > 0 );
    m_chemData->DeselectAll();
    m_render2D->update();

    m_statusLabel->setText( tr( "Imported %1 of %2 records." )
                            .arg( imported ).arg( m_records.count() ) );

    if ( failed > 0 ) {
        QMessageBox::warning( this, tr( "Import complete" ),
                              tr( "%1 record(s) could not be imported." ).arg( failed ) );
    }
}

void SDFBrowser::skipCurrent()
{
    if ( m_currentIndex < m_records.count() - 1 ) {
        ++m_currentIndex;
        updateUI();
    }
}

void SDFBrowser::goPrevious()
{
    if ( m_currentIndex > 0 ) {
        --m_currentIndex;
        updateUI();
    }
}

void SDFBrowser::goNext()
{
    if ( m_currentIndex < m_records.count() - 1 ) {
        ++m_currentIndex;
        updateUI();
    }
}
