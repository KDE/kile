/***************************************************************************
                          bibtexdialog.h  -  description
                             -------------------
    begin                : Tue Sep 2 2003
    copyright            : (C) 2003 by Rob Lensen
    email                : rob@bsdfreaks.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BIBTEXDIALOG_H
#define BIBTEXDIALOG_H
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qstringlist.h>
#include "kileinfo.h"
#include <kdialogbase.h>
/**
  *@author Rob Lensen
  */

class KListBox;
class QStringList;

class bibtexdialog : public KDialogBase
{
public:
	bibtexdialog(const QStringList &filesbib, const QString& caption, const QString& select ,
								QWidget *parent = 0, const char * name = 0);
	~bibtexdialog() {}

	int currentItem() { return m_listbox->currentItem();}

	private:
			KListBox	*m_listbox;

};
#endif
