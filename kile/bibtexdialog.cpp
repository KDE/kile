/***************************************************************************
                          bibtexdialog.cpp -  description
                             -------------------
    begin                : Mon Sept 1 2003
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

#include "kile.h"
#include "kiledocumentinfo.h"
#include "bibtexdialog.h"
#include <klocale.h>
#include <qstringlist.h>

bibtexdialog::bibtexdialog(	const QStringList &filesbib, const QString& caption, const QString& select ,
															QWidget *parent, const char * name )
:KDialogBase( KDialogBase::Plain, caption, Ok|Cancel,Ok, parent, name, true, true )
{
 	QVBoxLayout *layout = new QVBoxLayout(plainPage());
	layout->addWidget(new QLabel(i18n("Please select a %1 from the list:").arg(select), plainPage()));

	m_listbox = new KListBox(plainPage());
	m_listbox->insertStringList(filesbib);
	layout->addWidget(m_listbox);

	connect(m_listbox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(accept()));
}



