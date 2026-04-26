#ifndef TOOL_IR_H
#define TOOL_IR_H

#include <QTextEdit>

#include "molecule.h"
#include "tooldialog.h"
#include "ngw.h"


class Tool_IR_Dialog : public ToolDialog
{
public:
    Tool_IR_Dialog( QWidget *parent = nullptr );
    void process();

private:
    NewGraphWidget *ngw;
    QTextEdit *peaktext;
};

#endif

// kate: tab-width 4; indent-width 4; space-indent on; replace-trailing-space-save on;
