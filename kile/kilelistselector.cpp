/***************************************************************************
                          kilelistselector.cpp -  description
                             -------------------
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#include "kilelistselector.h"

#include <qstringlist.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <klistbox.h>

KileListSelector::KileListSelector(const QStringList & list, const QString &caption, const QString &select, QWidget *parent, const char * name) :
	KDialogBase( KDialogBase::Plain, caption, Ok|Cancel,Ok, parent, name, true, true )
{
	QVBoxLayout *layout = new QVBoxLayout(plainPage());

	layout->addWidget(new QLabel(i18n("Please select a %1 from the list:").arg(select), plainPage()));

	m_listbox = new KListBox(plainPage());
	m_listbox->insertStringList(list);
	layout->addWidget(m_listbox);

	connect(m_listbox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(accept()));
}
