/***************************************************************************
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

	layout->addWidget(new QLabel(select, plainPage()));

	m_listbox = new KListBox(plainPage());
	m_listbox->insertStringList(list);
	m_listbox->setCurrentItem(0);
	layout->addWidget(m_listbox);

	setMinimumWidth(275);
	resize(sizeHint().width(),sizeHint().height()+4);
	connect(m_listbox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(accept()));
}

KileListSelectorMultiple::KileListSelectorMultiple(const QStringList & list, const QString &caption, const QString &select, QWidget *parent, const char * name) :
	KDialogBase( KDialogBase::Plain, caption, Ok|Cancel,Ok, parent, name, true, true )
{
	QVBoxLayout *layout = new QVBoxLayout(plainPage());

	layout->addWidget(new QLabel(select, plainPage()));

	m_listbox = new KListBox(plainPage());
	m_listbox->insertStringList(list);
	m_listbox->setSelectionMode(QListBox::Multi);
	layout->addWidget(m_listbox);

	setMinimumWidth(275);
	resize(sizeHint().width(),sizeHint().height()+4);
	connect(m_listbox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(accept()));
}

const QStringList& KileListSelectorMultiple::selected(){

m_selectedfiles.clear();

for(uint i=0;i < m_listbox->count(); i++){
	if( m_listbox->isSelected( i ) ){
	m_selectedfiles.append(m_listbox->item(i)->text());
	}
}
return m_selectedfiles;
}



