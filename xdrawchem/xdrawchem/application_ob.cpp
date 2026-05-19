#include "ob_compat.h"
OB_COMPAT_BEGIN
#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <openbabel/math/vector3.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
OB_COMPAT_END
using namespace OpenBabel;

#include <cstdio>
#include <fstream>
#include <cstring>

#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>

#include "application.h"
#include "sdfbrowser.h"
#include "chemdata.h"
#include "molecule.h"
#include "dpoint.h"
#include "bond.h"

#include "ioiface.h"

using namespace OpenBabel;

// OpenBabel functions.
void ApplicationWindow::OBGetFilters()
{
    OBConversion Conv;

    //io_type inFileType = UNDEFINED;
    //int i;
    //OBFileFormat fileFormat;
    QString str1, myext;

    str1 = tr( "--Select a filter-- (*)" );
    readFilters.append( str1 );
    writeFilters.append( str1 );
    str1 = tr( "CDXML - ChemDraw text/XML format (*)" );
    readFilters.append( str1 );
    writeFilters.append( str1 );
    str1 = tr( "CDX - ChemDraw binary format (*)" );
    readFilters.append( str1 );
    writeFilters.append( str1 );
    str1 = tr( "XDC - XDrawChem native format (*)" );
    readFilters.append( str1 );
    writeFilters.append( str1 );

    Formatpos pos;
    OBFormat *pFormat;
    const char *str = 0;

    while ( OBConversion::GetNextFormat( pos, str, pFormat ) ) {
        //qDebug() << "  " << str;
        if ( ( pFormat->Flags() & NOTWRITABLE ) && ( pFormat->Flags() & NOTREADABLE ) )
            continue;
        str1 = str;
        myext = str1.left( str1.indexOf( " -- " ) );
        str1.append( "(*." );
        str1.append( myext );
        str1.append( ")" );
        if ( ( pFormat->Flags() & NOTREADABLE ) == 0 )
            readFilters.append( str1 );
        if ( ( pFormat->Flags() & NOTWRITABLE ) == 0 )
            writeFilters.append( str1 );
    }

    filters.sort();
    readFilters.sort();
    writeFilters.sort();
}

void ApplicationWindow::OBImport()
{
    QFileDialog fd( this, tr( "Open file..." ) );

//  fd.setWindowTitle();
    fd.setFileMode( QFileDialog::ExistingFile );
    fd.setNameFilters( readFilters );
    if ( fd.exec() == QDialog::Accepted ) {
        QString selectedFile = fd.selectedFiles()[0];
        QString selectedFilter = fd.selectedNameFilter();

        // For SDF files, show the multi-record browser instead of
        // loading only the first molecule through OpenBabel.
        if ( selectedFile.endsWith(".sdf", Qt::CaseInsensitive) ||
             selectedFile.endsWith(".sd", Qt::CaseInsensitive) ) {
            SDFBrowser browser( this, selectedFile, m_chemData, m_renderer );
            if ( browser.exec() == QDialog::Accepted ) {
                if ( browser.imported() ) {
                    setWindowTitle( QString( XDC_VERSION ) + QString( " - " ) + selectedFile );
                    statusBar()->showMessage( tr( "Loaded SDF: " ) + selectedFile );
                    filename = selectedFile;
                    m_chemData->saved();
                    updatePropertyPanel();
                }
            }
            return;
        }

        OBNewLoad( selectedFile, selectedFilter );
    }
}

void ApplicationWindow::OBNewLoad( QString infile, QString infilter )
{
    //io_type inFileType = UNDEFINED;
    ///OBFileFormat fileFormat;
    filename = infile;
    filefilter = infilter;

    /* handled internally */
    if ( ( infilter.left( 3 ) == "XDC" ) || ( infilter.left( 3 ) == "CDX" ) || ( infile.endsWith(".XDC", Qt::CaseInsensitive) ) ) {
        QString realFileName = filename;

        QFile f( filename );

        if ( !f.open( QIODevice::ReadOnly ) ) {
            QMessageBox::warning( 0, tr( "Couldn't open file" ), tr( "Could not open the file: " ) + filename );
            statusBar()->showMessage( tr( "Unable to load " ) + filename );
            return;
        }
        f.close();
        SelectAll();
        Clear();

        if ( !m_chemData->load( filename ) ) {
            statusBar()->showMessage( tr( "Unable to load " ) + filename );
            return;
        }

        filename = realFileName;
        setWindowTitle( QString( XDC_VERSION ) + QString( " - " ) + filename );
        statusBar()->showMessage( tr( "Loaded document " ) + filename );
        m_chemData->DeselectAll();
        m_renderer->update();
        m_chemData->saved();
        updatePropertyPanel();
        return;
    }
    /* end XDC/CDX code */

    OBConversion Conv;
    OBFormat *inFormat = 0;

    if ( infilter.left( 2 ) == QString::fromLatin1( "--" ) ) {
        inFormat = Conv.FormatFromExt( infile.toLatin1().data() );
        if ( inFormat == 0 ) {
            QMessageBox::warning( 0, tr( "Could not determine file type" ), tr( "Please select a file type from the list." ) );
            return;
        }
    }

    /* rest done by OpenBabel */

    int cutpt = infilter.indexOf( QString::fromLatin1( " -- " ) );

    infilter.truncate( cutpt );
    // Keep the QByteArray alive for the duration of the FindFormat call.
    // Previously: const char* pointed into a temporary that was destroyed
    // before use, plus malloc(length) was one byte short of null terminator.
    QByteArray infilterBytes = infilter.toLatin1();
    if ( inFormat == 0 )
        inFormat = Conv.FindFormat( infilterBytes.constData() );
    OBMol *mol = new OBMol;
    std::ifstream inFileStream( infile.toLatin1() );

    if ( !inFileStream ) {
        QMessageBox::warning( 0, tr( "Problem while opening the file" ), tr( "Cannot open the specified file." ) );
        delete mol;
        return;
    }

    qInfo() << "OB loading " << inFormat;
    Conv.SetInAndOutFormats( inFormat, inFormat );
    Conv.Read( mol, &inFileStream );
    inFileStream.close();
    SelectAll();
    Clear();
    IOIface converter( m_chemData, mol );

    converter.convertToChemData();
    m_chemData->ScaleAll();
    m_chemData->DeselectAll();
    m_renderer->update();
    m_renderer->setMode_Select();
    m_chemData->saved();
    updatePropertyPanel();
    setWindowTitle( QString( XDC_VERSION ) + QString( " - " ) + filename );
    statusBar()->showMessage( tr( "Loaded document " ) + filename );
    delete mol;
}

void ApplicationWindow::OBExport()
{
    qDebug() << "OBExport()";
    //io_type inFileType = UNDEFINED;
    int i;

    //OBFileFormat fileFormat;
    //char *formatOptions;
    QFile ftest;

    QFileDialog fd( this, tr( "Save file as..." ) );
    QString str1;

    fd.setFileMode( QFileDialog::AnyFile );
    fd.setNameFilters( writeFilters );
    fd.setAcceptMode( QFileDialog::AcceptSave );
    fd.selectNameFilter("XDC - XDrawChem native format (*)");
    if ( fd.exec() == QDialog::Accepted ) {
        if ( ftest.exists( fd.selectedFiles()[0] ) ) {
            i = QMessageBox::warning( this, tr( "Overwrite file?" ), tr( "Overwrite existing file: %1 ?" ).arg( fd.selectedFiles()[0] ), QMessageBox::Ok, QMessageBox::Cancel );
            if ( i == QMessageBox::Cancel )
                return;
        }
        str1 = fd.selectedNameFilter();
        filefilter = str1;      // so we know what file type to save as
        filename = fd.selectedFiles()[0];
        OBNewSave();
    }
}

void ApplicationWindow::OBNewSave()
{
    m_chemData->StartUndo();           // needed in case we scale before saving

    OBConversion Conv;
    OBFormat *outFormat = 0;

    if ( filefilter.left( 2 ) == "--" ) {
        outFormat = Conv.FormatFromExt( filefilter.toLatin1() );
        if ( outFormat == 0 ) {
            QMessageBox::warning( 0, tr( "Could not determine file type" ), tr( "Please select a file type from the list." ) );
            return;
        }
    }

    QString str1;

    //io_type inFileType = UNDEFINED;
    //int i;
    //OBFileFormat fileFormat;
    //char *formatOptions;
    str1 = filefilter;

    if ( str1.left( 3 ) == "XDC" ) {
        if ( filename.right( 4 ).toLower() != ".xdc" )
            filename.append( ".xdc" );
        if ( !m_chemData->save( filename ) ) {
            statusBar()->showMessage( tr( "Could not write to " ) + filename );
            return;
        }
        setWindowTitle( QString( XDC_VERSION ) + QString( " - " ) + filename );
        statusBar()->showMessage( tr( "Saved file " ) + filename );
        return;
    }
    if ( str1.left( 3 ) == "CDX" ) {
        if ( filename.right( 6 ).toLower() != ".cdxml" )
            filename.append( ".cdxml" );
        if ( !m_chemData->save( filename ) ) {
            statusBar()->showMessage( tr( "Could not write to " ) + filename );
            return;
        }
        setWindowTitle( QString( XDC_VERSION ) + QString( " - " ) + filename );
        statusBar()->showMessage( tr( "Saved file " ) + filename );
        return;
    }
    int cutpt = str1.indexOf( " -- " );

    str1.truncate( cutpt );
    QByteArray str1Bytes = str1.toLatin1();
    if ( outFormat == 0 )
        outFormat = Conv.FindFormat( str1Bytes.constData() );
    OBMol *mol = new OBMol;

    Conv.SetInAndOutFormats( outFormat, outFormat );

    // invert so save file has correct coordinates
    //m_renderer->SelectAll();
    //m_renderer->Flip(FLIP_V);
    //m_renderer->DeselectAll();

    IOIface converter( m_chemData, mol );

    if ( !converter.convertToOBMol() ) {
        QMessageBox::warning( 0, tr( "IOIface Data error" ), tr( "Cannot convert the molecule.\nConversion failed in ioiface.cpp" ) );
        return;
    }
    std::ofstream outFileStream( filename.toLatin1() );

    if ( !outFileStream ) {
        QMessageBox::warning( 0, tr( "Problem while writing the file" ), tr( "Cannot write the specified file." ) );
        return;
    }

    Conv.Write( mol, &outFileStream );
    //fileFormat.WriteMolecule(outFileStream, *mol, "3D", formatOptions);
    setWindowTitle( QString( XDC_VERSION ) + QString( " - " ) + filename );

    statusBar()->showMessage( tr( "Saved file " ) + filename );
    //m_chemData->Undo();
    delete mol;
}

QString ApplicationWindow::OBGuessFileType( QString guessname )
{
    QString guesstype = "--Select a filter-- (*)";
    QString guess_lower = guessname.toLower();

    if ( guess_lower.right( 4 ) == ".xdc" ) {
        guesstype = "XDC - XDrawChem native format (*)";
        return guesstype;
    }
    if ( guess_lower.right( 6 ) == ".cdxml" ) {
        guesstype = "CDXML - ChemDraw text/XML format (*)";
        return guesstype;
    }
    if ( guess_lower.right( 4 ) == ".cdx" ) {
        guesstype = "CDX - ChemDraw binary format (*)";
        return guesstype;
    }

    QString dos_ext, ext_test;

    for ( int cc = guess_lower.length() - 1; cc > 0; cc-- ) {
        if ( guess_lower.at( cc ) == '.' )
            break;
        dos_ext.prepend( guess_lower.at( cc ) );
    }

    for ( QStringList::Iterator it = filters.begin(); it != filters.end(); ++it ) {
        ext_test = *it;
        ext_test = ext_test.left( ext_test.indexOf( ":" ) );
        if ( dos_ext == ext_test ) {
            guesstype = *it;
            break;
        }
    }

    return guesstype;
}

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
