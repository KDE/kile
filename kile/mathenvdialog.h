/***************************************************************************
                           mathenvdialog.h
----------------------------------------------------------------------------
    date                 : Jul 23 2005
    version              : 0.20
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MATHENVDIALOG_H
#define MATHENVDIALOG_H

#include "kilewizard.h"
#include "latexcmd.h"

#include <klineedit.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qmap.h>
#include <qvaluelist.h>
 
namespace KileDialog
{

class MathEnvironmentDialog : public Wizard  
{
	Q_OBJECT

public:
	MathEnvironmentDialog(QWidget *parent, KConfig *config, KileDocument::LatexCommands *commands);
	~MathEnvironmentDialog() {}
	
public slots:
	void slotOk();

private slots:
	void slotEnvironmentChanged(int index);  
	void slotSpinboxValueChanged(int index);  

private:
	KileDocument::LatexCommands *m_latexCommands;
	
	QComboBox *m_coEnvironment, *m_coTabulator, *m_coDisplaymath;
	QCheckBox *m_cbStarred, *m_cbBullets;
	QSpinBox *m_spRows, *m_spCols;
	QLabel *m_lbRows, *m_lbCols, *m_lbSpace ;
	QLabel *m_lbTabulator, *m_lbDisplaymath, *m_lbStarred;
	QLabel *m_lbEnvironment, *m_lbBullets;
	KLineEdit *m_edSpace;
	
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
