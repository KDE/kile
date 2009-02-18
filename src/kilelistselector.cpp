/******************************************************************************************
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilelistselector.h"

#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <KApplication>
#include <KLocale>

#include "kiledebug.h"

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

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(KDialog::spacingHint());
	page->setLayout(layout);

	layout->addWidget(new QLabel(select, page));

	m_listview = new QTreeWidget(page);
	m_listview->setHeaderLabel(i18n("Files"));
	m_listview->setSortingEnabled(false);
	m_listview->setAllColumnsShowFocus(true);
	m_listview->setRootIsDecorated(false);

	layout->addWidget(m_listview);
	
	insertStringList(list);

	connect(m_listview, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(accept()));
}

int KileListSelectorBase::currentItem()
{
	QTreeWidgetItem *item = m_listview->currentItem();
	return m_listview->indexOfTopLevelItem(item);
}

void KileListSelectorBase::insertStringList(const QStringList &list)
{
	QStringList::ConstIterator it;
	for (it = list.begin(); it != list.end(); ++it)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(m_listview, QStringList(*it));
		
		if(it == list.begin())
			m_listview->setCurrentItem(item);
	}
}

//////////////////// with single selection ////////////////////

KileListSelector::KileListSelector(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) : KileListSelectorBase(list, caption, select, parent, name)
{
	m_listview->setSelectionMode(QAbstractItemView::SingleSelection);

	if (list.count() > 0)
		m_listview->topLevelItem(0)->setSelected(true);
}

//////////////////// with multi selection ////////////////////

KileListSelectorMultiple::KileListSelectorMultiple(const QStringList &list, const QString &caption, const QString &select, QWidget *parent, const char *name) : KileListSelectorBase(list, caption, select, parent, name)
{
	m_listview->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

const QStringList& KileListSelectorMultiple::selected()
{
	m_selectedfiles.clear();

	QTreeWidgetItemIterator it(m_listview, QTreeWidgetItemIterator::Selected);
	while (*it) {
		m_selectedfiles.append((*it)->text(0));
		++it;
	}

	return m_selectedfiles;
}
