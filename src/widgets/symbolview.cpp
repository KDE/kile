/****************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2006 - 2007 by Thomas Braun
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
dani 2005-11-22
  - add some new symbols
  - rearranged source

tbraun 2006-07-01
   - added tooltips which show the keys, copied from kfileiconview
   - reorganized the hole thing, more flexible png loading, removing the old big code_array, more groups

tbraun 2007-06-04
    - Send a warning in the logwidget if needed packages are not included for the command
tbraun 2007-06-13
    - Added Most frequently used symbolview, including remembering icons upon restart, removing of least popular item and configurable max item count
*/

#include "widgets/symbolview.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPixmap>
#include <QRegExp>
#include <QStringList>

#include <KConfig>
#include <KLocale>
#include <KStandardDirs>

#include "kileconfig.h"
#include "kiledebug.h"

#define MFUS_GROUP "MostFrequentlyUsedSymbols"
#define MFUS_PREFIX "MFUS"


namespace KileWidget {

SymbolView::SymbolView(QWidget *parent, int type, const char *name)
		: QListWidget(parent)
{
	setObjectName(name);
	setViewMode(IconMode);
	setGridSize(QSize(36, 36));
	setSpacing(5);
	setWordWrap(false);
	setResizeMode(Adjust);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setMovement(Static);
	setSortingEnabled(false);
	setFlow(LeftToRight);
	setDragDropMode(NoDragDrop);
	initPage(type);
}

SymbolView::~SymbolView()
{
}

void SymbolView::extract(const QString& key, int& refCnt)
{
	if (!key.isEmpty()) {
		refCnt = key.section('%', 0, 0).toInt();
	}

	return;
}

void SymbolView::extract(const QString& key, int& refCnt, QString &cmd, QStringList &args, QStringList &pkgs)
{
	if (key.isEmpty()) {
		return;
	}

	extract(key, refCnt);

	QRegExp rePkgs("(?:\\[(.*)\\])?\\{(.*)\\}");

	args.clear();
	pkgs.clear();

	cmd = key.section('%', 1, 1);
	QString text = key.section('%', 2, 2);

	if (text.indexOf(rePkgs) != -1) {
		args = rePkgs.cap(1).split(",");
		pkgs = rePkgs.cap(2).split(",");
	}
}

void SymbolView::initPage(int page)
{
	switch(page) {
		case MFUS:
			fillWidget(MFUS_PREFIX);
			break;
	
		case Relation:
			fillWidget("relation");
			break;
	
		case Operator:
			fillWidget("operators");
			break;
	
		case Arrow:
			fillWidget("arrows");
			break;
	
		case MiscMath:
			fillWidget("misc-math");
			break;
	
		case MiscText:
			fillWidget("misc-text");
			break;
	
		case Delimiters:
			fillWidget("delimiters");
			break;
	
		case Greek:
			fillWidget("greek");
			break;
	
		case Special:
			fillWidget("special");
			break;
	
		case Cyrillic:
			fillWidget("cyrillic");
			break;
	
		case User:
			fillWidget("user");
			break;
	
		default:
			kWarning() << "wrong argument in initPage()";
			break;
	}
}

QString SymbolView::getToolTip(const QString &key)
{
	QString cmd, label;
	QStringList pkgs, args;
	int refCnt;

	extract(key, refCnt, cmd, args, pkgs);

	label = i18n("Command: ") + cmd + "\n";

	if(pkgs.count() > 0) {
		if(pkgs.count() == 1) {
			label += i18n("Package: ");
		}
		else {
			label += i18n("Packages: ");
		}

		for (int i = 0; i < pkgs.count() ; i++) {
			if(i < args.count()) {
				label = label + "[" + args[i] + "]" + pkgs[i] + "\n";
			}
			else {
				label = label + pkgs[i] + "\n";
			}
		}
	}

	return label;
}

void SymbolView::mousePressEvent(QMouseEvent *event)
{
	QString code_symbol;
	QStringList args, pkgs;
	QListWidgetItem *item = NULL;
	int count;
	bool math = false, bracket = false;

	if(event->button() == Qt::LeftButton && (item = itemAt(event->pos()))) {
		bracket = event->modifiers() & Qt::ControlModifier;
		math = event->modifiers() & Qt::ShiftModifier;

		extract(item->data(Qt::UserRole).toString(), count, code_symbol, args, pkgs);

		if(math != bracket) {
			if(math) {
				code_symbol = '$' + code_symbol + '$';
			}
			else {
				if(bracket) {
					code_symbol = '{' + code_symbol + '}';
				}
			}
		}
		emit(insertText(code_symbol, pkgs));
		emit(addToList(item));
	}

	KILE_DEBUG() << "math is " << math << ", bracket is " << bracket << " and item->data(Qt::UserRole).toString() is " << (item ? item->data(Qt::UserRole).toString() : "");
}

void SymbolView::fillWidget(const QString& prefix)
{
	KILE_DEBUG() << "===SymbolView::fillWidget(const QString& " << prefix <<  " )===";
	QImage image;
	QListWidgetItem* item;
	QStringList refCnts, paths;

	if (prefix == MFUS_PREFIX) {
		KConfigGroup config = KGlobal::config()->group(MFUS_GROUP);
		QString configPaths = config.readEntry("paths");
		QString configrefCnts = config.readEntry("counts");
		paths = configPaths.split(',');
		refCnts = configrefCnts.split(',');
		KILE_DEBUG() << "Read " << paths.count() << " paths and " << refCnts.count() << " refCnts";
		if(paths.count() != refCnts.count()) {
			KILE_DEBUG() << "error in saved LRU list";
			paths.clear();
			refCnts.clear();
		}
	}
	else {
		paths = KGlobal::dirs()->findAllResources("app_symbols", prefix + "/*.png", KStandardDirs::NoDuplicates);
		paths.sort();
		for(int i = 0 ; i < paths.count() ; i++) {
			refCnts.append("1");
		}
	}
	for(int i = 0; i < paths.count(); i++) {
		if(image.load(paths[i])) {
//      KILE_DEBUG() << "path is " << paths[i];
			item = new QListWidgetItem(this);
			item->setIcon(QPixmap::fromImage(image));
			QString key = refCnts[i] + '%' + image.text("Command") + '%' + image.text("Packages") + '%' + paths[i];
			item->setData(Qt::UserRole, key);
			item->setToolTip(getToolTip(key));
//    image = KImageEffect::blend(colorGroup().text(), image, 1); // destroys our png comments, so we do it after reading the comments
		}
		else {
			KILE_DEBUG() << "Loading file " << paths[i] << " failed";
		}
	}
}

void SymbolView::writeConfig()
{
	QListWidgetItem *item;
	QStringList paths, refCnts;


	KConfigGroup grp = KGlobal::config()->group(MFUS_GROUP);

	if (KileConfig::clearMFUS()) {
		grp.deleteEntry("paths");
		grp.deleteEntry("counts");
	}
	else {
		for(int i = 0; i < count(); ++i) {
			item = this->item(i);
			refCnts.append(item->data(Qt::UserRole).toString().section('%', 0, 0));
			paths.append(item->data(Qt::UserRole).toString().section('%', 3, 3));
			KILE_DEBUG() << "path=" << paths.last() << ", count is " << refCnts.last();
		}
		grp.writeEntry("paths", paths);
		grp.writeEntry("counts", refCnts);
	}
}

void SymbolView::slotAddToList(const QListWidgetItem *item)
{
	if(!item || item->icon().isNull()) {
		return;
	}

	QListWidgetItem *tmpItem = NULL;
	bool found = false;
	const QRegExp reCnt("^\\d+");

	KILE_DEBUG() << "===void SymbolView::slotAddToList(const QIconViewItem *" << item << " )===";

	for(int i = 0; i < count(); ++i) {
		tmpItem = this->item(i);
		if (item->data(Qt::UserRole).toString().section('%', 1) == tmpItem->data(Qt::UserRole).toString().section('%', 1)) {
			found = true;
			break;
		}
	}

	if(!found
	   && static_cast<unsigned int>(this->count() + 1) > KileConfig::numSymbolsMFUS()) {   // we check before adding the symbol
		int refCnt, minRefCnt = 10000;
		QListWidgetItem *unpopularItem = NULL;

		KILE_DEBUG() << "Removing most unpopular item";

		for (int i = 0; i < count(); ++i) {
			tmpItem = this->item(i);
			extract(tmpItem->data(Qt::UserRole).toString(), refCnt);

			if (refCnt < minRefCnt) {
				refCnt = minRefCnt;
				unpopularItem = tmpItem;
			}
		}
		KILE_DEBUG() << " minRefCnt is " << minRefCnt;
		delete unpopularItem;
	}

	if(found) {
		KILE_DEBUG() << "item is already in the iconview";

		int refCnt;
		extract(tmpItem->data(Qt::UserRole).toString(), refCnt);

		QString key = tmpItem->data(Qt::UserRole).toString();
		key.replace(reCnt, QString::number(refCnt + 1));
		tmpItem->setData(Qt::UserRole, key);
		tmpItem->setToolTip(getToolTip(key));
	}
	else {
		tmpItem = new QListWidgetItem(this);
		tmpItem->setIcon(item->icon());
		QString key = item->data(Qt::UserRole).toString();
		tmpItem->setData(Qt::UserRole, key);
		tmpItem->setToolTip(getToolTip(key));
	}
}

}

#include "symbolview.moc"
