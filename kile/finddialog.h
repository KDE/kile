#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include "latexeditor.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QRadioButton;

class FindDialog : public QDialog
{ 
    Q_OBJECT

public:
    FindDialog(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FindDialog();

    QLabel* TextLabel1;
    QComboBox* comboFind;
    QPushButton* PushButton1;
    QPushButton* PushButton2;
    QButtonGroup* ButtonGroup2;
    QRadioButton* radioForward;
    QRadioButton* radioBackward;
    QButtonGroup* ButtonGroup1;
    QCheckBox* checkWords;
    QCheckBox* checkCase;
    QCheckBox* checkBegin;


public slots:
    virtual void doFind();
    void SetEditor(LatexEditor *ed);

protected:
    QGridLayout* FindDialogLayout;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout2;
    QVBoxLayout* ButtonGroup2Layout;
    QVBoxLayout* ButtonGroup1Layout;

    LatexEditor *editor;
};

#endif // FINDDIALOG_H
