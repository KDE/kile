/***************************************************************************
                          kilestructurewidget.cpp  -  description
                             -------------------
    begin                : Sun Dec 28 2003
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

#include <qfileinfo.h>
 
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>

#include "kileinfo.h"
#include "kiledocumentinfo.h"
#include "kilestructurewidget.h"

namespace KileWidget
{
	Structure::Structure(KileInfo *ki, QWidget * parent, const char * name) : 
		KListView(parent,name),
		m_ki(ki)
	{
		connect(this, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotClicked(QListViewItem *)));
		connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotDoubleClicked(QListViewItem *)));
	}

	void Structure::slotClicked(QListViewItem * itm)
	{
		KileListViewItem *item = (KileListViewItem *)itm;
		//return if user didn't click on an item
		if (! item) return;

		if (! (item->type() & (KileStruct::None | KileStruct::Input)))
			emit(setCursor(item->line()-1, item->column()));
	}

	void Structure::slotDoubleClicked(QListViewItem * itm)
	{
		KileListViewItem *item = (KileListViewItem*)(itm);
		if (! item) return;
		if (! (item->type() & KileStruct::Input)) return;

		QString fn = m_ki->getCompileName();
		QString fname = item->title();
		if (fname.right(4)==".tex")
			fname =QFileInfo(fn).dirPath()+"/" + fname;
		else
			fname=QFileInfo(fn).dirPath()+"/" + fname + ".tex";
	
		QFileInfo fi(fname);
		if (fi.isReadable())
		{
			emit(fileOpen(KURL::fromPathOrURL(fname), QString::null));
		}
		else
			KMessageBox::error(this, i18n("Cannot find the included file. The file does not exists, is not readable or Kile is unable to determine the correct path to this file. The filename leading to this error was: %1").arg(fname), i18n("Cannot find file!"));
	}

	void Structure::update(KileDocumentInfo *docinfo, bool parse)
	{
		takeItem(firstChild());
		QListViewItem *item = (QListViewItem*)docinfo->structViewItem();
		if ((item == 0) || parse)
		{
			docinfo->updateStruct(level());
			item = (QListViewItem*)docinfo->structViewItem();
		}
		if (item) insertItem(item);
	}
}

#include "kilestructurewidget.moc"
