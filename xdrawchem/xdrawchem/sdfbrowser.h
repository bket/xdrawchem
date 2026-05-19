// sdfbrowser.h — Multi-record SDF browser dialog
//
// Provides Prev/Next navigation through SDF records and a list view
// of all molecules in the file.  Each record is parsed independently via
// OpenBabel so the existing Molecule::convertFromOBMol() path is reused.

#ifndef SDFBROWSER_H
#define SDFBROWSER_H

#include <QDialog>
#include <QList>
#include <QString>

class QListWidget;
class QPushButton;
class QLabel;
class QTableWidget;
class ChemData;
class Render2D;

struct SDFRecord {
    QString name;      // from the header line (first line of MOL block)
    int startOffset;   // byte offset in file where this record begins
    int length;        // bytes in this record (including $$$$)
};

class SDFBrowser : public QDialog
{
    Q_OBJECT

public:
    // parent: parent widget
    // fileName: path to the SDF file
    // chemData / render2D: target document and renderer for import
    SDFBrowser( QWidget *parent, const QString &fileName,
                ChemData *chemData, Render2D *render2D );

    // Returns true if the user imported at least one record.
    bool imported() const { return m_imported; }

public slots:
    void importCurrent();
    void importAll();
    void skipCurrent();
    void goPrevious();
    void goNext();
    void rowClicked( int row );
    void rowDoubleClicked( int row );

private:
    void parseRecords();
    void updateUI();
    bool loadRecord( int index );
    void previewRecord( int index );

    QString m_fileName;
    ChemData *m_chemData;
    Render2D *m_render2D;
    QList<SDFRecord> m_records;
    int m_currentIndex = 0;
    bool m_imported = false;

    QTableWidget *m_table;
    QLabel *m_statusLabel;
    QPushButton *m_importBtn;
    QPushButton *m_importAllBtn;
    QPushButton *m_skipBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_doneBtn;
};

#endif // SDFBROWSER_H
