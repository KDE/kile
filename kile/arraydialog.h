/***************************************************************************
                          arraydialog.h  -  description
                             -------------------
    begin                : ven sep 27 2002
    copyright            : (C) 2002 by Pascal Brachet
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

	public:
		QTable *m_table;
		QSpinBox *m_spRows, *m_spCols;
		KComboBox *m_cbAlign, *m_cbEnv;
	};
}

#endif
