/***************************************************************************
                          tabdialog.h  -  description
                             -------------------
    begin                : Mon Apr 30 2001
    copyright            : (C) 2001 by Brachet Pascal
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

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include "kilewizard.h"

class QTable;
class QSpinBox;
class QCheckBox;

class KComboBox;
class KConfig;

namespace KileDialog
{
	class QuickTabular : public Wizard
	{
		Q_OBJECT

	public:
		QuickTabular(KConfig *, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
		~QuickTabular();

	public slots:
		void slotOk();

	private:
		KComboBox	*m_cbAlign, *m_cbSeparator;
		QSpinBox		*m_spRows, *m_spCols;
		QCheckBox	*m_ckHSeparator;
		QTable		*m_table;
	};
}

#endif


