/****************************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           2008 by Michel Ludwig (michel.ludwig@kdemail.net)
*****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgets/abbreviationview.h"

#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QTextStream>

#include <KLocale>
#include <KMenu>
#include <KMessageBox>

#include "dialogs/abbreviationinputdialog.h"

#include "kiledebug.h"

namespace KileWidget {

AbbreviationView::AbbreviationView(QWidget *parent, const char *name)
	: QTreeWidget(parent), m_changes(false)
{
	setObjectName(name);
	setColumnCount(2);
	QStringList headerLabelList;
	headerLabelList << i18n("Short") << QString() << i18n("Expanded Text");
	setHeaderLabels(headerLabelList);
	setAllColumnsShowFocus(true);

	header()->setMovable(false);      // default: true
	header()->setResizeMode(QHeaderView::ResizeToContents);

	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotCustomContextMenuRequested(const QPoint&)));
}

AbbreviationView::~AbbreviationView()
{
}
//////////////////// init abbreviation view with wordlists ////////////////////


void AbbreviationView::init(const QStringList *globallist, const QStringList *locallist)
{
	setUpdatesEnabled(false);
	clear();
	addWordlist(globallist,true);
	addWordlist(locallist,false);
	setUpdatesEnabled(true);

	m_changes = false;
}

void AbbreviationView::addWordlist(const QStringList *wordlist, bool global)
{
	QString type = (global) ? QString() : "*";

	QStringList::ConstIterator it;
	for(it = wordlist->begin(); it != wordlist->end(); ++it) {
		int index = (*it).indexOf('=');
		if(index >= 0) {
			QTreeWidgetItem *item = new QTreeWidgetItem(this);
			item->setText(0, (*it).left(index));
			item->setText(1, type);
			item->setText(2, (*it).right((*it).length() - index - 1));
			addTopLevelItem(item);
		}
	}
}

//////////////////// save local abbreviation list ////////////////////

void AbbreviationView::saveLocalAbbreviation(const QString &filename)
{
	if(!m_changes) {
		return;
	}

	KILE_DEBUG() << "=== AbbreviationView::saveLocalAbbreviation ===================";
	// create the file 
	QFile abbrevfile(filename);
	if(!abbrevfile.open( QIODevice::WriteOnly)) {
		return;
	}

	QTextStream stream(&abbrevfile);
	stream << "# abbreviation mode: editable abbreviations\n";
	stream << "# dani/2007\n";

	//QTextCodec *codec = QTextCodec::codecForName(m_ki->activeTextDocument()->encoding().ascii());
	// stream.setCodec(codec);

	QTreeWidgetItemIterator it(this);
	while(*it) {
		QTreeWidgetItem *current = *it;
		if(current->text(AbbreviationView::ALVlocal) == "*") {
			stream << current->text(AbbreviationView::ALVabbrev)
			       << "=" 
			       << current->text(AbbreviationView::ALVexpansion)
			       << "\n";
		}
		++it;
	}
	abbrevfile.close();

	m_changes = false;
}

//////////////////// find abbreviation ////////////////////

bool AbbreviationView::findAbbreviation(const QString &abbrev)
{
	QTreeWidgetItemIterator it(this);
	while(*it) {
		QTreeWidgetItem *current = *it;
		if(current->text(AbbreviationView::ALVabbrev) == abbrev) {
			return true;
		}

		++it;
	}
	return false;
}

//////////////////// item clicked ////////////////////

void AbbreviationView::slotItemClicked(QTreeWidgetItem *item, int /* column */)
{
	if(item) {
		emit(sendText(item->text(AbbreviationView::ALVexpansion)));
	}
}

//////////////////// context menu ////////////////////

void AbbreviationView::slotCustomContextMenuRequested(const QPoint& p)
{
	KMenu popupMenu;
	QAction *action = new QAction(i18n("&Add"), &popupMenu);
	connect(action, SIGNAL(triggered()), this, SLOT(slotAddAbbreviation()));
	popupMenu.addAction(action);

	QList<QTreeWidgetItem*> selectedList = selectedItems();
	if(selectedList.count() > 0) {
		QTreeWidgetItem *selectedItem = selectedList.first();
		if(!selectedItem->text(ALVlocal).isEmpty()) {
			popupMenu.addSeparator();
			action = new QAction(i18n("&Edit"), &popupMenu);
			connect(action, SIGNAL(triggered()), this, SLOT(slotChangeAbbreviation()));
			popupMenu.addAction(action);
			popupMenu.addSeparator();
			action = new QAction(i18n("&Delete"), &popupMenu);
			connect(action, SIGNAL(triggered()), this, SLOT(slotDeleteAbbreviation()));
			popupMenu.addAction(action);
		}
	}


	popupMenu.exec(mapToGlobal(p));
}

void AbbreviationView::slotAddAbbreviation()
{
	KileDialog::AbbreviationInputDialog dialog(this, NULL, ALVadd);
	if(dialog.exec() == QDialog::Accepted) {
		QString abbrev, expansion;
		dialog.abbreviation(abbrev, expansion);
		addAbbreviation(abbrev, expansion);
	}
}

void AbbreviationView::slotChangeAbbreviation()
{
	QList<QTreeWidgetItem*> selectedList = selectedItems();
	if(selectedList.count() == 0) {
		return;
	}
	QTreeWidgetItem *selectedItem = selectedList.first();
	KileDialog::AbbreviationInputDialog dialog(this, selectedItem, ALVedit);
	if(dialog.exec() == QDialog::Accepted) {
		QString abbrev, expansion;
		dialog.abbreviation(abbrev, expansion);
		changeAbbreviation(selectedItem, abbrev, expansion);
	}
}

void AbbreviationView::slotDeleteAbbreviation()
{
	QList<QTreeWidgetItem*> selectedList = selectedItems();
	if(selectedList.count() == 0) {
		return;
	}
	deleteAbbreviation(selectedList.first());
}

void AbbreviationView::addAbbreviation(const QString &abbrev, const QString &expansion)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(this);
	item->setText(0, abbrev);
	item->setText(1, "*");
	item->setText(2, expansion);
	addTopLevelItem(item);
	QString newAbbrev = abbrev + '=' + expansion;

	emit(updateAbbrevList(QString(), newAbbrev));
	m_changes = true;
}

void AbbreviationView::changeAbbreviation(QTreeWidgetItem *item, const QString &abbrev, const QString &expansion)
{
	if(item) {
		QString oldAbbrev = item->text(ALVabbrev) + '=' + item->text(ALVexpansion);
		QString newAbbrev = abbrev + '=' + expansion;
		item->setText(ALVabbrev,abbrev);
		item->setText(ALVexpansion,expansion);

		emit(updateAbbrevList(oldAbbrev,newAbbrev));
		m_changes = true;
	}
}

void AbbreviationView::deleteAbbreviation(QTreeWidgetItem *item)
{
	QString abbrev = item->text(ALVabbrev);
	QString message = i18n("Delete the abbreviation '%1'?", abbrev);
	if(KMessageBox::questionYesNo(this,
		       "<center>" + message + "</center>",
		       i18n("Delete Abbreviation") ) == KMessageBox::Yes) {
		QString s = item->text(ALVabbrev) + '=' + item->text(ALVexpansion);
		delete item;

		emit(updateAbbrevList(s, QString()));
		m_changes = true;
	}
}

}

#include "abbreviationview.moc"
