/***************************************************************************
                          letterdialog.h  -  description
                             -------------------
    begin                : Tue Oct 30 2001
    copyright            : (C) 2001 by Brachet Pascal, (C) 2003 by Jeroen Wijnhout
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

#ifndef LETTERDIALOG_H
#define LETTERDIALOG_H

#include "quickdocumentdialog.h"

#include <qcheckbox.h>

#include <kcombobox.h>

/**
  *@author Brachet Pascal
  *@author Jeroen Wijnhout
  */

namespace KileDialog
{
	class QuickLetter : public QuickDocument
	{
		Q_OBJECT

	public:
		QuickLetter(KConfig *, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
		~QuickLetter();

	public slots:
		void slotOk();

	public:
		KComboBox *combo2, *combo3, *combo4;
		QCheckBox* checkbox1;
	};
}

#endif

