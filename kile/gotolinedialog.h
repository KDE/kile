#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include "latexeditor.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;
class QSpinBox;

class GotoLineDialog : public QDialog
{ 
    Q_OBJECT

public:
    GotoLineDialog(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GotoLineDialog();

    QLabel* TextLabel1;
    QSpinBox* spinLine;
    QPushButton* PushButton2;
    QPushButton* PushButton1;


public slots:
    virtual void gotoLine();
    void SetEditor(LatexEditor *ed);

protected:
    QGridLayout* GotoLineDialogLayout;
    QHBoxLayout* Layout1;

    LatexEditor *editor;
};

#endif // GOTOLINEDIALOG_H
