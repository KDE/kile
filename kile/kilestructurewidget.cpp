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
#include <qregexp.h>
 
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurl.h>

#include "kileinfo.h"
#include "kiledocmanager.h"
#include "kiledocumentinfo.h"
#include "kilestructurewidget.h"
#include "kileconfig.h"

KileListViewItem::KileListViewItem(QListViewItem * parent, QListViewItem * after, const QString &title, const KURL &url, uint line, uint column, int type, int level) : 
	KListViewItem(parent,after),
	m_title(title), m_url(url), m_line(line), m_column(column), m_type(type), m_level(level)
{
	this->setText(0, m_title+" (line "+QString::number(m_line)+")");
}

KileListViewItem::KileListViewItem(QListView * parent, QString label) : 
	KListViewItem(parent,label),
	m_title(label), m_url(KURL()), m_line(0),  m_column(0), m_type(KileStruct::None), m_level(0) 
{}

KileListViewItem::KileListViewItem(QListViewItem * parent, QString label) :
	KListViewItem(parent,label),
	m_title(label), m_url(KURL()), m_line(0),  m_column(0), m_type(KileStruct::None), m_level(0) 
{}

namespace KileWidget
{
	StructureList::StructureList(Structure *stack, KileDocument::Info *docinfo) : 
		KListView(stack),
		m_stack(stack), m_docinfo(docinfo)
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

	StructureList::~StructureList()
	{
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
// 			connect(m_docinfo, SIGNAL(doneUpdating()), this, SLOT(insertInMasterList()));
		}

		m_current=m_child=m_lastChild=m_parent[0]=m_parent[1]=m_parent[2]=m_parent[3]=m_parent[4]=m_parent[5]=m_parent[7]=m_root;

		m_folders.clear();
	}

	void StructureList::cleanUp()
	{
		saveState();
		clear();
		if(NULL != m_docinfo)
			disconnect(m_docinfo, 0, this, 0);
		init();
	}

	void StructureList::saveState()
	{
		m_openByTitle.clear();
		m_openByLine.clear();

		QListViewItemIterator it(this);
		KileListViewItem *item = 0L;
		while ( it.current() ) 
		{
			item = (KileListViewItem*)it.current();
			if ( item->firstChild() )
			{
				//we don't accept duplicate entries in the map, remove this item alltogether
				//and rely on the openByLine map instead
				if ( m_openByTitle.contains ( item->title() ) )
					m_openByTitle.remove ( item->title() );
				else
					m_openByTitle [ item->title() ] = item->isOpen();

				m_openByLine [ item->line() ] = item->isOpen();
			}
			it++;
		}
	}

	bool StructureList::shouldBeOpen(KileListViewItem *item, QString folder, int level)
	{
		if ( item->parent() == 0L ) return true;

		if ( m_openByTitle.contains(item->title()) )
			return m_openByTitle [ item->title() ];
		else if ( m_openByLine.contains(item->line()) )
			return m_openByLine [ item->line() ]; //TODO check surrounding lines as well
		else
			return ((folder == "root") && level <= m_stack->level());
	}

	KileListViewItem* StructureList::createFolder(const QString &folder)
	{
		KileListViewItem *fldr=  new KileListViewItem(m_root, folder);
		fldr->setOpen(false);
		if ( folder == "labels" )
		{
			fldr->setText(0, i18n("Labels"));
			fldr->setPixmap(0, SmallIcon("label"));
		}
		else if ( folder == "refs" )
		{
			fldr->setText(0, i18n("References"));
			fldr->setPixmap(0, SmallIcon("bibtex"));
		}

		m_folders[ folder ] = fldr;

		return m_folders[folder];
	}

	KileListViewItem* StructureList::folder(const QString &folder)
	{
		KileListViewItem *item = m_folders[folder];
		if ( item == 0L ) item = createFolder(folder);
		return item;
	}

	void StructureList::activate()
	{
		m_stack->raiseWidget(this);
	}

	KileListViewItem *StructureList::parentFor(int lev, const QString & fldr)
	{
		KileListViewItem *par = m_current;

		if ( fldr == "root" )
		{
			switch (lev)
			{
				case KileStruct::File :  
					if ( m_current == 0L ) par = m_root;
					else if ( m_current->level() == KileStruct::File )
						par = (KileListViewItem*)m_current->parent();
					else if ( m_current->level() >= 0 )
						par = m_parent[m_current->level()];
				break;

				case 0 : case 1 :
					par = m_root;
				break;

				default:
					par = m_parent[lev - 2];
				break;
			}
		}
		else
			par = folder(fldr);

		return par;
	}

	void StructureList::addItem(const QString &title, uint line, uint column, int type, int lev, const QString & pix, const QString & fldr /* = "root" */)
	{
 		kdDebug() << "==void Structure::addItem(" << title << ")=========" << endl;

		if ( lev == KileStruct::Hidden ) return;

		//find the m_parent for the new element
		m_current = parentFor(lev, fldr);

		//find last element at this level
		m_child = (KileListViewItem*)m_current->firstChild();
		while (m_child)
		{
			m_lastChild = m_child;
			m_child = (KileListViewItem*)m_child->nextSibling();
		}

		m_child = new KileListViewItem(m_current, m_lastChild, title, m_docinfo->url(), line, column, type, lev);

		if (! pix.isNull()) m_child->setPixmap(0,SmallIcon(pix));

		//if the level is not greater than the defaultLevel
		//open the m_parent to make this item visible
		m_current->setOpen(shouldBeOpen((KileListViewItem*)m_current, fldr, lev));

		//update the m_parent levels, such that section etc. get inserted at the correct level
		m_current = m_child;
		if ( lev > 0)
		{
			m_parent[lev-1] = m_child;
			for (int l = lev; l < 7; l++)
				m_parent[l] = m_child;
		}
		else if ( lev == 0 )
		{
			for ( int l = 0; l < 7; l++ ) m_parent[l] = m_root;
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

		m_default = new StructureList(this, 0L);
		m_default->activate();
	}

	int Structure::level()
	{
		return KileConfig::defaultLevel();
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
			emit(setCursor(item->url(), item->line()-1, item->column()));
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
		if ( info == 0L ) return 0L;

		if ( ! viewExistsFor(info) )
			m_map.insert(info, new StructureList(this, info), true);

		return  m_map[info];
	}

	bool Structure::viewExistsFor(KileDocument::Info *info)
	{
		if ( info == 0L ) return false;
		else
			return m_map.contains(info);
	}

	void Structure::closeDocumentInfo(KileDocument::Info *docinfo)
	{
		m_docinfo = 0L;
		if ( m_map.contains(docinfo) )
		{
			StructureList *data = m_map[docinfo];
			m_map.remove(docinfo);
			delete data;
		}
		
		if ( m_map.isEmpty() ) m_default->activate();
	}

	void Structure::clear()
	{
		QMapIterator<KileDocument::Info *, StructureList *> it;
		QMapIterator<KileDocument::Info *, StructureList *> itend(m_map.end());
		for ( it = m_map.begin(); it != itend; it++)
			if ( it.data() != 0L ) delete it.data();

		m_map.clear();
		m_docinfo = 0L;

		m_default->activate();
	}

	void Structure::update(KileDocument::Info *docinfo, bool parse)
	{
		kdDebug() << "==KileWidget::Structure::update(" << docinfo << ")=============" << endl;

		if ( docinfo == 0L ) 
		{
			m_default->activate();
			return;
		}

		m_docinfo = docinfo;

		bool needParsing = parse || ( ! viewExistsFor(m_docinfo) );

		//find structview-item for this docinfo
		StructureList *view = viewFor(m_docinfo);

		if ( needParsing ) //need to reparse the doc
		{
			view->cleanUp();
			m_docinfo->updateStruct();
		}

		view->activate();
	}
}

#include "kilestructurewidget.moc"
