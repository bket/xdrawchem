#ifndef NETDIALOG_H
#define NETDIALOG_H

// netdialog.h — "Find structure on PubChem" dialog.
//
// The server URL field has been removed: PubChem is the fixed backend
// and is not user-configurable.  The getServer() method is retained as
// a no-op so callers compile without changes.

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>

class QLineEdit;

class NetDialog : public QDialog
{
public:
    NetDialog( QWidget *parent );

    QString getKey() const
    {
        switch ( keylist->currentIndex() ) {
        case 1:  return QStringLiteral("cas");
        case 2:  return QStringLiteral("formula");
        default: return QStringLiteral("name");
        }
    }

    QString getValue() const { return searchkey->text().trimmed(); }
    // Server is always PubChem — kept for API compatibility.
    QString getServer() const { return QString(); }
    bool    getExact()  const { return emcheck->isChecked(); }

private:
    QComboBox *keylist;
    QCheckBox *emcheck;
    QLineEdit *searchkey;
};

#endif

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
