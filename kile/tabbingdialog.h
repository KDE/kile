/***************************************************************************
    begin                : dim jui 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, (C) 2003 Jeroen Wijnhout
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

#ifndef TABBINGDIALOG_H
#define TABBINGDIALOG_H

#include "kilewizard.h"

#include <qdialog.h>

class QSpinBox;
class KLineEdit;

/**
  *@author Pascal Brachet
  *@author Jeroen Wijnhout
  */

namespace KileDialog
{
	class QuickTabbing : public Wizard  
	{
		Q_OBJECT

	public:
		QuickTabbing(KConfig *config, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
		~QuickTabbing();

	public slots:
		void slotOk();
	
	public:
		QSpinBox		*m_spCols, *m_spRows;
		KLineEdit		*m_leSpacing;
	};
}

#endif
