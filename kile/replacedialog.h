#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

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

class ReplaceDialog : public QDialog
{ 
    Q_OBJECT

public:
    ReplaceDialog(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ReplaceDialog();

    QLabel* TextLabel2;
    QLabel* TextLabel1;
    QComboBox* comboFind;
    QComboBox* comboReplace;
    QPushButton* buttonReplace;
    QPushButton* buttonReplaceAll;
    QPushButton* PushButton2;
    QButtonGroup* ButtonGroup1;
    QCheckBox* checkWords;
    QCheckBox* checkCase;
    QCheckBox* checkBegin;
    QButtonGroup* ButtonGroup2;
    QRadioButton* radioForward;
    QRadioButton* radioBackward;


public slots:
    virtual void doReplace();
    virtual void doReplaceAll();
    void SetEditor(LatexEditor *ed);

protected:
    QGridLayout* ReplaceDialogLayout;
    QGridLayout* Layout3;
    QHBoxLayout* Layout5;
    QVBoxLayout* ButtonGroup1Layout;
    QVBoxLayout* ButtonGroup2Layout;

    LatexEditor *editor;
    QObject *formWindow;
};

#endif // REPLACEDIALOG_H
