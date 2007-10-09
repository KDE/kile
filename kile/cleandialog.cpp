/***************************************************************************
    date                 : Jan 22 2004
    version              : 0.10
    copyright            : (C) 2004 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cleandialog.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <klistview.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfileinfo.h>

#include "kiledebug.h"

namespace KileDialog
{
	Clean::Clean(QWidget *parent, const QString & filename, const QStringList & extlist) : 
		KDialogBase( KDialogBase::Plain, i18n("Delete Files"), Ok | Cancel, Ok, parent, 0, true, true),
		m_extlist(extlist)
	{
		// Layout
		QVBoxLayout *vbox = new QVBoxLayout(plainPage(), 6,6 );
		
		// label widgets
		QWidget *labelwidget = new QWidget(plainPage());
		QHBoxLayout *labellayout = new QHBoxLayout(labelwidget);
		
		// line 1: picture and label
		QLabel *picture =  new QLabel("", labelwidget);
		picture->setPixmap( KGlobal::instance()->iconLoader()->loadIcon("messagebox_warning", KIcon::NoGroup, KIcon::SizeMedium) );
		QLabel *label =  new QLabel(i18n( "Do you really want to delete these files?" ), labelwidget);
		labellayout->addWidget(picture);
		labellayout->addSpacing(20);
		labellayout->addWidget(label);
		
		// line 2: listview
		listview = new KListView(plainPage());
		listview->addColumn(i18n("Files"));
		listview->setSorting(-1);
		
		// insert items into listview
		QString base = QFileInfo(filename).baseName(true);
		for (uint i=0; i <  m_extlist.count(); ++i)
		{
			QCheckListItem *item = new QCheckListItem(listview, base + m_extlist[i], QCheckListItem::CheckBox);
			item->setOn(true);
			listview->insertItem(item);
		}

		vbox->addWidget(labelwidget,0,Qt::AlignHCenter);
		vbox->addWidget(listview);
	}
	
	Clean::~Clean()
	{}
	
	// get all selected items
	
	const QStringList & Clean::getCleanlist()
	{
		QStringList templist;

		QCheckListItem *item = (QCheckListItem *)listview->firstChild();
		int i = m_extlist.count() - 1;
		while ( item )
		{
			if ( item->isOn() && item->text(0).endsWith(m_extlist[i]) )
				templist.append(m_extlist[i]);

			item = (QCheckListItem *)item->nextSibling();
			--i;
		}

		m_extlist = templist;
		return m_extlist;
	}
}

#include "cleandialog.moc"

