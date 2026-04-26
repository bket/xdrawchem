#ifndef XDC_TEXTEDIT_H
#define XDC_TEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>

// Custom text editor widget that emits returnPressed() on Enter key.
class XdcTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit XdcTextEdit( QWidget *parent = nullptr ) : QTextEdit( parent ) {}

    void keyPressEvent( QKeyEvent *event ) override
    {
        if ( event->key() == Qt::Key_Return )
            emit returnPressed();
        else
            QTextEdit::keyPressEvent( event );
    }

signals:
    void returnPressed();
};

#endif // XDC_TEXTEDIT_H
