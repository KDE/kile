/***************************************************************************
                          arraydialog.h  -  description
                             -------------------
    begin                : ven sep 27 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// dani 15.09.2004: don't override existing rows or columns,  
//                  when changing the size of the table

#ifndef ARRAYDIALOG_H
#define ARRAYDIALOG_H


/**
  *@author Pascal Brachet
  */

#include "kilewizard.h"

class QTable;
class QSpinBox;

class KComboBox;
class KConfig;

namespace KileDialog
{
	class QuickArray : public Wizard  
	{
		Q_OBJECT

	public:
		QuickArray(KConfig *, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
		~QuickArray();

	public slots:
		void slotOk();

	private:
		QTable *m_table;
		QSpinBox *m_spRows, *m_spCols;
		KComboBox *m_cbAlign, *m_cbEnv;
		
		int m_rows;
		int m_cols;
		bool isTableRowEmpty(int row);
		bool isTableColEmpty(int col);
		
	private slots:
		void slotRowValueChanged(int value);
		void slotColValueChanged(int value);
	};
}

#endif
