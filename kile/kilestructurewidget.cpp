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
#include <qheader.h>
 
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurl.h>

#include "kileinfo.h"
#include "kiledocmanager.h"

#include "kilestructurewidget.h"

namespace KileWidget
{
	StructureList::StructureList(Structure *stack, KileDocument::Info *docinfo) : KListView(stack), m_stack(stack), m_docinfo(docinfo)
	{
		show();
		header()->hide();
		addColumn(i18n("Structure"),-1);
		setSorting(-1,true);
		setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

		connect(this, SIGNAL(clicked(QListViewItem *)), m_stack, SLOT(slotClicked(QListViewItem *)));
		connect(this, SIGNAL(doubleClicked(QListViewItem *)), m_stack, SLOT(slotDoubleClicked(QListViewItem *)));

		init();
	}

	void StructureList::init()
	{
		QString title = (m_docinfo == 0L ) ? i18n("No \"structure data\" to display.") : m_docinfo->url().fileName();
		m_root =  new KileListViewItem(this, title);
		if ( m_docinfo != 0L )
		{
			m_root->setOpen(true);
			m_root->setPixmap(0, SmallIcon("contents"));
			connect(m_docinfo, SIGNAL(foundItem(const QString&, uint, uint, int, int, const QString &, const QString &)), this, SLOT(addItem(const QString&, uint, uint, int, int, const QString &, const QString &)));
		}

		m_current=m_child=m_lastChild=m_parent[0]=m_parent[1]=m_parent[2]=m_parent[3]=m_parent[4]=m_root;

		m_folders.clear();
	}

	void StructureList::cleanUp()
	{
		saveState();
		clear();
		disconnect(m_docinfo, 0, this, 0);
		init();
	}

	void StructureList::saveState()
	{
		m_openByTitle.clear();
		QListViewItemIterator it(this);
		KileListViewItem *item = 0L;
		while ( it.current() ) 
		{
			item = (KileListViewItem*)it.current();
			if ( item->firstChild() ) m_openByTitle [ item->title() ] = item->isOpen();
			it++;
		}
	}

	bool StructureList::shouldBeOpen(KileListViewItem *item, QString folder, int level)
	{
		//FIXME what do we do when there are multiple section with the same title?
		if ( m_openByTitle.contains(item->title()) )
			return m_openByTitle [ item->title() ];
		else
			return ((folder == "root") && level <= m_stack->level());
	}

	QListViewItem* StructureList::createFolder(const QString &folder)
	{
		QListViewItem *fldr=  new KileListViewItem(m_root, folder.upper());
		fldr->setOpen(false);
		if ( folder == "labels" )
			fldr->setPixmap(0, SmallIcon("label"));

		m_folders[ folder ] = fldr;

		return m_folders[folder];
	}

	QListViewItem* StructureList::folder(const QString &folder)
	{
		QListViewItem *item = m_folders[folder];
		if ( item == 0L ) item = createFolder(folder);
		return item;
	}

	void StructureList::addItem(const QString &title, uint line, uint column, int type, int lev, const QString & pix, const QString & fldr /* = "m_root" */)
	{
 		kdDebug() << "==void Structure::addItem(" << title << ")=========" << endl;

// 		StructureList *data = viewFor(info);

		if ( lev > -2)
		{
			//find the m_parent for the new element
			if ( fldr == "root" )
			{
// 				kdDebug() << "\tadding to m_root folder" << endl;
				switch (lev)
				{
				case -1 : m_current = folder("labels"); break;
				case 0 : case 1 : m_current= m_root; break;
				default : m_current = m_parent[lev-2]; break;
				}
			}
			else
			{
				m_current = folder(fldr);
			}

// 			kdDebug() << "\t\tm_m_current = " << m_current << endl;

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
			//open the m_parent to make this item visible
			m_current->setOpen(shouldBeOpen((KileListViewItem*)m_current, fldr, lev));

			//update the m_parent levels, such that section etc. get inserted at the correct level
			if ( lev > 0)
			{
				m_parent[lev-1] = m_child;
				for (int l = lev; l < 5; l++)
					m_parent[l] = m_child;
			}
		}
	}

	Structure::Structure(KileInfo *ki, QWidget * parent, const char * name) : 
		QWidgetStack(parent,name),
		m_ki(ki),
		m_docinfo(0L)
	{
		kdDebug() << "==KileWidget::Structure::Structure()===========" << endl;
		setLineWidth(0);
		setMidLineWidth(0);
		setMargin(0);
		setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

		StructureList *def = new StructureList(this, 0L);
		raiseWidget(def);

		connect(this, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotClicked(QListViewItem *)));
		connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotDoubleClicked(QListViewItem *)));
	}

	void Structure::addDocumentInfo(KileDocument::Info *docinfo)
	{
		StructureList *view = new StructureList(this, docinfo);
		addWidget(view);
		m_map.insert(docinfo, view, true);
	}

	void Structure::slotClicked(QListViewItem * itm)
	{
		KileListViewItem *item = (KileListViewItem *)itm;
		//return if user didn't click on an item
		if (! item) return;

		if (! (item->type() & KileStruct::None ))
			emit(setCursor(item->line()-1, item->column()));
	}

	void Structure::slotDoubleClicked(QListViewItem * itm)
	{
		KileListViewItem *item = (KileListViewItem*)(itm);
		if (! item) return;
		if (! (item->type() & (KileStruct::Input | KileStruct::Bibliography) ) ) return;

		QString fn = m_ki->getCompileName();
		QString fname = item->title();
		if (fname.right(4) == ".tex")
			fname =QFileInfo(fn).dirPath()+"/" + fname;
		else if ( item->type() == KileStruct::Input )
			fname=QFileInfo(fn).dirPath()+"/" + fname + ".tex";
		else if ( item->type() == KileStruct::Bibliography )
			fname=QFileInfo(fn).dirPath()+"/" + fname + ".bib";
	
		QFileInfo fi(fname);
		if (fi.isReadable())
		{
			emit(fileOpen(KURL::fromPathOrURL(fname), QString::null));
		}
		else
		{
			if ( KMessageBox::warningYesNo(this, i18n("Cannot find the included file. The file does not exist, is not readable or Kile is unable to determine the correct path to it. The filename causing this error was: %1.\nDo you want to create this file?").arg(fname), i18n("Cannot Find File"))
			== KMessageBox::Yes)
			{
				emit(fileNew(KURL::fromPathOrURL(fname)));
			}
		}
	}

	StructureList* Structure::viewFor(KileDocument::Info *info)
	{
		if ( !m_map.contains(info) )
			m_map.insert(info, new StructureList(this, info), true);

		return  m_map[info];
	}

	void Structure::closeDocumentInfo(KileDocument::Info *docinfo)
	{
		kdDebug() << "==void Structure::closeDocumentInfo(KileDocument::Info *docinfo)======" << endl;
		kdDebug() << "\tclosing " << docinfo->url().url() << endl;
		m_docinfo = 0L;
		if ( m_map.contains(docinfo) )
		{
			StructureList *data = m_map[docinfo];
			m_map.remove(docinfo);
			delete data;
		}
	}

	void Structure::clear()
	{
		QMapIterator<KileDocument::Info *, StructureList *> it;
		QMapIterator<KileDocument::Info *, StructureList *> itend(m_map.end());
		for ( it = m_map.begin(); it != itend; it++)
			if ( it.data() != 0L ) delete it.data();

		m_map.clear();
		m_docinfo = 0L;
	}

	void Structure::update(KileDocument::Info *docinfo, bool parse)
	{
		kdDebug() << "==KileWidget::Structure::update(" << docinfo << ")=============" << endl;

		m_docinfo = docinfo;

		bool needParsing = parse || ( ! m_map.contains(m_docinfo) );

		//find structview-item for this docinfo
		StructureList *view = viewFor(m_docinfo);

		if ( needParsing ) //need to reparse the doc
		{
			view->cleanUp();
			m_docinfo->updateStruct();
		}

		raiseWidget(view);
	}
	

}

#include "kilestructurewidget.moc"
