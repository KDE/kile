//Added by qt3to4:
#include <QLabel>
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/




KProgress* ConfigCheckerWidget::progressBar()
{
    return m_progress;
}


QLabel* ConfigCheckerWidget::label()
{
    return m_lbChecking;
}

K3ListBox* ConfigCheckerWidget::listBox()
{
    return m_lstResults;
}
