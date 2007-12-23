/***************************************************************************
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                           (C) 2007 by Holger Danielsson
    email                : Jeroen.Wijnhout@kdemail.net
                           holger.danielsson@versanet.de
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

#include <QStringList>
#include <QLabel>
#include <QLayout>
#include <q3header.h>

#include <Q3VBoxLayout>
#include <QList>

#include <kapplication.h>
#include "kiledebug.h"
#include <klocale.h>

//////////////////// KileListSelectorBase ////////////////////

KileListSelectorBase::KileListSelectorBase(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) :
	KDialog(parent)
{
	setObjectName(name);
	setCaption(caption);
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	Q3VBoxLayout *layout = new Q3VBoxLayout(this);

	layout->addWidget(new QLabel(select, this));
	layout->addSpacing(8);

	m_listview = new K3ListView(this);
	m_listview->addColumn(i18n("Files"));
	m_listview->setSorting(-1);
	m_listview->setAllColumnsShowFocus(true);
	m_listview->setFullWidth(true);
	m_listview->setItemsMovable(false);                 // default: true
	//setAcceptDrops(false);                            // default: false
	//setDragEnabled(false);                            // default: false
	//setShadeSortColumn(true);                         // default: true
	m_listview->header()->setMovingEnabled(false);      // default: true

	m_listview->setShadeSortColumn(false);

	layout->addWidget(m_listview);

	insertStringList(list);

	int w = m_listview->columnWidth(0) + 32;
	w = ( w > 275 ) ? w : 275;
	int h = ( list.count() > 0 ) ? m_listview->header()->height()+12*m_listview->firstChild()->height() : 224;
	m_listview->setMinimumSize(w,h);

	resize(sizeHint().width(),sizeHint().height()+4);
	connect(m_listview, SIGNAL(doubleClicked(Q3ListViewItem*,const QPoint &,int)), this, SLOT(accept()));
}

int KileListSelectorBase::currentItem()
{
	Q3ListViewItem *item = m_listview->currentItem();
	return ( item ) ? m_listview->itemIndex(item) : -1; 
}

void KileListSelectorBase::insertStringList(const QStringList &list)
{
	QStringList::ConstIterator it;
	K3ListViewItem *item = 0L;
	for ( it=list.begin(); it!=list.end(); ++it )
	{
		item = new K3ListViewItem(m_listview,item,*it);
		m_listview->insertItem(item);
	}
}

//////////////////// with single selection ////////////////////

KileListSelector::KileListSelector(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) : KileListSelectorBase(list,caption,select,parent,name)
{
	m_listview->setSelectionMode(Q3ListView::Single);

	if ( list.count() > 0 )
		m_listview->setSelected(m_listview->firstChild(),true);
}

//////////////////// with multi selection ////////////////////

KileListSelectorMultiple::KileListSelectorMultiple(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) : KileListSelectorBase(list,caption,select,parent,name)
{
	m_listview->setSelectionMode(Q3ListView::Extended);     // default: Single
}

const QStringList& KileListSelectorMultiple::selected()
{
	m_selectedfiles.clear();

	QList<Q3ListViewItem*> list = m_listview->selectedItems();
	QList<Q3ListViewItem*>::iterator it;
	
	for (it = list.begin(); it != list.end(); ++it)
		m_selectedfiles.append((*it)->text(0));
	
	return m_selectedfiles;
}


