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
#include <kiconloader.h>
#include <kurl.h>

#include "kileinfo.h"
#include "kiledocumentinfo.h"
#include "kilestructurewidget.h"

namespace KileWidget
{
	Structure::Structure(KileInfo *ki, QWidget * parent, const char * name) : 
		KListView(parent,name),
		m_ki(ki),
		m_docinfo(0L),
		m_label(0L),
		m_current(0L),
		m_root(0L), 
		m_child(0L), 
		m_lastChild(0L)
	{
		kdDebug() << "==KileWidget::Structure::Structure()===========" << endl;
		kdDebug() << "\tm_root " << m_root << endl;
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
		{
			if ( KMessageBox::warningYesNo(this, i18n("Cannot find the included file. The file does not exists, is not readable or Kile is unable to determine the correct path to this file. The filename leading to this error was: %1.\nDo you want to create this file?").arg(fname), i18n("Cannot find file!"))
			== KMessageBox::Yes)
			{
				emit(fileNew(KURL::fromPathOrURL(fname)));
			}
		}
	}

	void Structure::init()
	{
		if ( ! m_docinfo) return;
		if ( ! m_docinfo->getDoc() ) return;

		if (firstChild()) takeItem(firstChild());

		kdDebug() << "ALIVE!" << endl;
		QString shortName = m_docinfo->getDoc()->url().fileName();
		kdDebug() << "ALIVE! (2)" << endl;
		m_root =  new KileListViewItem(this, shortName);
		m_root->setOpen(true);
		m_root->setPixmap(0,SmallIcon("contents"));

		m_child=m_lastChild=m_parent[0]=m_parent[1]=m_parent[2]=m_parent[3]=m_parent[4]=m_root;

		m_label=  new KileListViewItem(m_root,"LABELS");
		m_label->setOpen(false);

		m_map.insert(m_docinfo, m_root, true); //FIXME: when closing a document, item should be removed from map, tree should be deleted
	}

	void Structure::closeDocument(KileDocumentInfo *docinfo)
	{
		m_docinfo = 0L;
		if ( m_map.contains(docinfo) )
		{
			QListViewItem *item = m_map[docinfo];
			delete item;
			m_map.remove(docinfo);
		}
	}
	
	void Structure::update(KileDocumentInfo *docinfo, bool parse)
	{
		kdDebug() << "==KileWidget::Structure::update()=============" << endl;
		if (m_docinfo)
		{
			kdDebug() << "\tdisconnecting " << m_docinfo->url().fileName() << endl;
			disconnect(m_docinfo, 0, this, 0);
			m_docinfo->stopUpdate();
		}

		m_docinfo = docinfo;

		//find structview-item for this docinfo
		QListViewItem *item;
		if ( m_map.contains(m_docinfo) )
		{
			kdDebug() << "\tfound in map" << endl;
			item =  m_map[m_docinfo];
		}
		else
			item = 0;

		if ((item == 0) || parse) //need to reparse the doc
		{
			kdDebug() << "calling init" << endl;
			init();
			connect(m_docinfo, SIGNAL(foundItem(const QString&, uint, uint, int, int, const QString& )), this, SLOT(addItem(const QString&, uint, uint, int, int, const QString & )));
			m_docinfo->updateStruct();
		}
		else if (item) //no need to update, restore stored items
		{
			if ( firstChild() ) //not the first time update is called, we can safely remove the firstChild
			{
				kdDebug() << "\ttaking item" << endl;
				takeItem(firstChild());
			}

			kdDebug() << "\tinserting item" << endl;
			insertItem(item);
		}
	}
	
	void Structure::addItem(const QString &title, uint line, uint column, int type, int lev, const QString & pix)
	{
		if ( lev > -2)
		{
			//find the parent for the new element
			switch (lev)
			{
			case -1 : m_current = m_label;break;
			case 0 : case 1 : m_current= m_root; break;
			default : m_current = m_parent[lev-2]; break;
			}

			//find last element at this level
			m_child = m_current->firstChild();
			while (m_child)
			{
				m_lastChild = m_child;
				m_child = m_child->nextSibling();
			}

			m_child=new KileListViewItem(m_current, m_lastChild, title, line, column, type);
			
			if (! pix.isNull()) m_child->setPixmap(0,SmallIcon(pix));

			//if the level is not greater than the defaultLevel
			//open the parent to make this item visible
			if ( ( m_current != m_label ) && lev <= level() )
				m_current->setOpen(true);

			//update the parent levels, such that section etc. get inserted at the correct level
			if ( lev > 0)
			{
				m_parent[lev-1] = m_child;
				for (int l = lev; l < 5; l++)
					m_parent[l] = m_child;
			}
		}
	}
}

#include "kilestructurewidget.moc"
