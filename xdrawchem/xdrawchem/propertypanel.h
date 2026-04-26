// propertypanel.h — Live molecule property panel (QDockWidget)
//
// Shows MW, empirical formula, and exact mass for the first molecule
// on the canvas, updating whenever the molecule changes.
// Wired via ChemData::SignalMoleculeChanged → ApplicationWindow::updatePropertyPanel().

#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include <QDockWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QFont>

class PropertyPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel( QWidget *parent = nullptr )
        : QDockWidget( tr( "Properties" ), parent )
    {
        setObjectName( QStringLiteral("PropertyPanel") );
        setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
                       | Qt::BottomDockWidgetArea );
        setFeatures( QDockWidget::DockWidgetMovable
                   | QDockWidget::DockWidgetFloatable
                   | QDockWidget::DockWidgetClosable );

        QWidget *inner = new QWidget( this );
        QVBoxLayout *layout = new QVBoxLayout( inner );
        layout->setContentsMargins( 6, 6, 6, 6 );
        layout->setSpacing( 4 );

        QFont labelFont = inner->font();
        labelFont.setPointSize( labelFont.pointSize() - 1 );

        auto makeLabel = [&]( const QString &placeholder ) -> QLabel * {
            QLabel *l = new QLabel( placeholder, inner );
            l->setFont( labelFont );
            l->setTextInteractionFlags( Qt::TextSelectableByMouse );
            l->setWordWrap( true );
            layout->addWidget( l );
            return l;
        };

        m_mwLabel      = makeLabel( tr( "MW: —" ) );
        m_formulaLabel = makeLabel( tr( "Formula: —" ) );

        layout->addStretch();
        setWidget( inner );
    }

    void setMW( const QString &s )      { m_mwLabel->setText( s ); }
    void setFormula( const QString &s ) { m_formulaLabel->setText( s ); }
    void clear()
    {
        m_mwLabel->setText( tr( "MW: —" ) );
        m_formulaLabel->setText( tr( "Formula: —" ) );
    }

private:
    QLabel *m_mwLabel;
    QLabel *m_formulaLabel;
};

#endif // PROPERTYPANEL_H

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
