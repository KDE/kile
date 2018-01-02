/******************************************************************************************
                           mathenvdialog.h
-------------------------------------------------------------------------------------------
    date                 : Dec 06 2005
    version              : 0.21
    copyright            : (C) 2005 by Holger Danielsson (holger.danielsson@t-online.de)
 ******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MATHENVIRONMENTDIALOG_H
#define MATHENVIRONMENTDIALOG_H

#include "kileinfo.h"
#include "kilewizard.h"
#include "latexcmd.h"

class QLabel;
class QCheckBox;
class QSpinBox;
class QLineEdit;

class KComboBox;

namespace KileDialog
{

class MathEnvironmentDialog : public Wizard
{
    Q_OBJECT

public:
    MathEnvironmentDialog(QWidget *parent, KConfig *config, KileInfo *ki,
                          KileDocument::LatexCommands *commands);
    ~MathEnvironmentDialog() {}

public Q_SLOTS:
    void slotAccepted();

private Q_SLOTS:
    void slotEnvironmentChanged(int index);
    void slotSpinboxValueChanged(int index);

private:
    KileInfo *m_ki;
    KileDocument::LatexCommands *m_latexCommands;

    KComboBox *m_coEnvironment, *m_coTabulator, *m_coDisplaymath;
    QCheckBox *m_cbStarred, *m_cbBullets;
    QSpinBox *m_spRows, *m_spCols;
    QLabel *m_lbRows, *m_lbCols, *m_lbSpace ;
    QLabel *m_lbTabulator, *m_lbDisplaymath, *m_lbStarred;
    QLabel *m_lbEnvironment, *m_lbBullets;
    QLineEdit *m_edSpace;

    QString m_envname;
    bool m_starred;
    bool m_groups;
    bool m_columns;
    bool m_fixedcolumns;
    bool m_mathmode;
    QString m_tabulator;
    QString m_parameter;

    void initEnvironments();
    bool isParameterEnv();
    bool isGroupsParameterEnv();
};

}

#endif
