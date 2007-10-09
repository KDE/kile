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

#include <qstringlist.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qheader.h>

#include <kapplication.h>
#include "kiledebug.h"
#include <klocale.h>

//////////////////// KileListSelectorBase ////////////////////

KileListSelectorBase::KileListSelectorBase(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) :
	KDialogBase( KDialogBase::Plain, caption, Ok|Cancel,Ok, parent, name, true, true )
{
	QVBoxLayout *layout = new QVBoxLayout(plainPage());

	layout->addWidget(new QLabel(select, plainPage()));
	layout->addSpacing(8);

	m_listview = new KListView(plainPage());
	m_listview->addColumn(i18n("Files"));
	m_listview->setSorting(-1);
	m_listview->setAllColumnsShowFocus(true);
	m_listview->setFullWidth(true);
	m_listview->setItemsMovable(false);                 // default: true
	//setAcceptDrops(false);                            // default: false
	//setDragEnabled(false);                            // default: false
	//setShadeSortColumn(true);                         // default: true
	m_listview->header()->setMovingEnabled(false);      // default: true

#if KDE_VERSION >= KDE_MAKE_VERSION(3,4,0)
	m_listview->setShadeSortColumn(false);
#endif
	layout->addWidget(m_listview);

	insertStringList(list);

	int w = m_listview->columnWidth(0) + 32;
	w = ( w > 275 ) ? w : 275;
	int h = ( list.count() > 0 ) ? m_listview->header()->height()+12*m_listview->firstChild()->height() : 224;
	m_listview->setMinimumSize(w,h);

	resize(sizeHint().width(),sizeHint().height()+4);
	connect(m_listview, SIGNAL(doubleClicked(QListViewItem*,const QPoint &,int)), this, SLOT(accept()));
}

int KileListSelectorBase::currentItem()
{
	QListViewItem *item = m_listview->currentItem();
	return ( item ) ? m_listview->itemIndex(item) : -1; 
}

void KileListSelectorBase::insertStringList(const QStringList &list)
{
	QStringList::ConstIterator it;
	KListViewItem *item = 0L;
	for ( it=list.begin(); it!=list.end(); ++it )
	{
		item = new KListViewItem(m_listview,item,*it);
		m_listview->insertItem(item);
	}
}

//////////////////// with single selection ////////////////////

KileListSelector::KileListSelector(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) : KileListSelectorBase(list,caption,select,parent,name)
{
	m_listview->setSelectionMode(QListView::Single);

	if ( list.count() > 0 )
		m_listview->setSelected(m_listview->firstChild(),true);
}

//////////////////// with multi selection ////////////////////

KileListSelectorMultiple::KileListSelectorMultiple(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) : KileListSelectorBase(list,caption,select,parent,name)
{
	m_listview->setSelectionMode(QListView::Extended);     // default: Single
}

const QStringList& KileListSelectorMultiple::selected()
{
	m_selectedfiles.clear();

	QPtrList<QListViewItem> list = m_listview->selectedItems();
	QPtrListIterator<QListViewItem> it(list);
	while ( it.current() )
	{
		m_selectedfiles.append((*it)->text(0));
		++it;
	}
	return m_selectedfiles;
}


