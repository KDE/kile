/***************************************************************************
                          kileproject.cpp -  description
                             -------------------
    begin                : Tue Aug 12 2003
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

#include <qheader.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kurl.h>
#include <krun.h>
#include <kmimetype.h>

#include "kileinfo.h"
#include "kiledocumentinfo.h"
#include "kileproject.h"
#include "kileprojectview.h"

const int KPV_ID_OPEN = 0, KPV_ID_SAVE = 1, KPV_ID_CLOSE = 2, KPV_ID_OPTIONS = 3, KPV_ID_ADD = 4,	KPV_ID_REMOVE = 5,
	KPV_ID_BUILDTREE = 6, KPV_ID_ARCHIVE = 7, KPV_ID_ADDFILES = 8, KPV_ID_INCLUDE = 9, KPV_ID_OPENWITH = 10;

/*
 * KileProjectViewItem
 */
void KileProjectViewItem::urlChanged(const KURL &url)
{
	setURL(url); setText(0,url.fileName()); kdDebug() << "SLOT KileProjectViewItem(" << text(0) << ")::urlChanged" << endl;
}

void KileProjectViewItem::nameChanged(const QString & name)
{
	setText(0,name); kdDebug() << "SLOT KileProjectViewItem(" << text(0) << ")::nameChanged" << endl;
}

void KileProjectViewItem::isrootChanged(bool isroot)
{
	kdDebug() << "SLOT isrootChanged " << text(0) << " to " << isroot << endl;
	if (isroot)
	{
		setPixmap(0,UserIcon("masteritem"));
	}
	else
	{
		if (type() == KileType::ProjectItem)
		{
			if ( text(0).right(7) == ".kilepr" )
				setPixmap(0,SmallIcon("kile"));
			else
				setPixmap(0,UserIcon("projectitem"));
		}
		else
			setPixmap(0,UserIcon("file"));
	}
}

/*
 * KileProjectView
 */
KileProjectView::KileProjectView(QWidget *parent, KileInfo *ki) : KListView(parent), m_ki(ki), m_nProjects(0), m_toggle(0)
{
	addColumn(i18n("Files and Projects"),-1);
	addColumn(i18n("Include in Archive"),10);
	setSorting(-1);
	setFocusPolicy(QWidget::ClickFocus);
	header()->hide();
	setRootIsDecorated(true);

	m_popup = new KPopupMenu(this, "projectview_popup");

	connect(this, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint & )),
		this,SLOT(popup(KListView *, QListViewItem * , const QPoint & )));

	connect(this, SIGNAL(executed(QListViewItem*)), this, SLOT(slotClicked(QListViewItem*)));
}

void KileProjectView::slotClicked(QListViewItem *item)
{
	if (item == 0)
		item = currentItem();

	KileProjectViewItem *itm = static_cast<KileProjectViewItem*>(item);
	kdDebug() << "KileProjectView:: clicked(" << itm->url().fileName() << ")" << endl;
	if (itm && (itm->type() == KileType::File || itm->type() == KileType::ProjectItem ))
	{
		kdDebug() << "KileProjectView:: emit fileSelected" << endl;
		emit fileSelected(itm->url());
	}
}

void KileProjectView::slotFile(int id)
{
	KileProjectViewItem *item = static_cast<KileProjectViewItem*>(currentItem());
	if (item && item->type() == KileType::File)
	{
		switch (id)
		{
			case KPV_ID_OPEN : emit(fileSelected(item->url())); break;
			case KPV_ID_SAVE : emit(saveURL(item->url())); break;
			case KPV_ID_ADD : emit(addToProject(item->url())); break;
			case KPV_ID_CLOSE : emit(closeURL(item->url())); break;
			default : break;
		}
	}
}

void KileProjectView::slotProjectItem(int id)
{
	kdDebug() << "===KileProjectView::slotProjectItem()===============" << endl;
	kdDebug() << "\tid: " << id << endl;
	KileProjectViewItem *item = static_cast<KileProjectViewItem*>(currentItem());
	if (item && (item->type() == KileType::ProjectItem || item->type() == KileType::ProjectExtra))
	{
		KURL projecturl,url;
		KRun *run;
		KileProjectItem *itm;
		switch (id)
		{
			case KPV_ID_OPEN : emit(fileSelected(item->url())); break;
			case KPV_ID_SAVE : emit(saveURL(item->url())); break;
			case KPV_ID_REMOVE :
				url = item->url();
				itm = m_ki->itemFor(url);
				projecturl = itm->project()->url();
				emit(removeFromProject(projecturl , url));
				break;
			case KPV_ID_INCLUDE :
				if (item->text(1) == "*") item->setText(1,"");
				else item->setText(1,"*");

				emit(toggleArchive(item->url()));
				break;
			case KPV_ID_CLOSE : emit(closeURL(item->url())); break;
			case KPV_ID_OPENWITH :
				{run = new KRun(item->url());}
				break;
			default : break;
		}
	}
}

void KileProjectView::slotProject(int id)
{
	KileProjectViewItem *item = static_cast<KileProjectViewItem*>(currentItem());
	if (item && item->type() == KileType::Project)
	{
		switch (id)
		{
			case KPV_ID_BUILDTREE : emit(buildProjectTree(item->url())); break;
			case KPV_ID_OPTIONS : emit(projectOptions(item->url())); break;
			case KPV_ID_CLOSE : emit(closeProject(item->url())); break;
			case KPV_ID_ARCHIVE : emit(projectArchive(item->url())); break;
			case KPV_ID_ADDFILES : emit(addFiles(item->url())); break;
			default : break;
		}
	}
}

void KileProjectView::slotRun(int id)
{
	KileProjectViewItem *itm = static_cast<KileProjectViewItem*>(currentItem());

	kdDebug() << "===slotRun(" << id << ")=================" << endl;
	if (id == 0)
	{
		KRun::runURL(itm->url(), "");
	}
	else
	{
		KURL::List list;
		list << itm->url();
		KRun::run(*m_offerList[id-1], list);
	}
}

void KileProjectView::popup(KListView *, QListViewItem *  item, const QPoint &  point)
{
	if (item != 0)
	{
		m_popup->clear();
		m_popup->disconnect();

		KileProjectViewItem *itm = static_cast<KileProjectViewItem*>(item);
		kdDebug() << "popup " << itm->url().path() << endl;

		if (itm->type() == KileType::ProjectExtra)
		{
			KPopupMenu *apps = new KPopupMenu( m_popup);
			m_offerList = KTrader::self()->query(KMimeType::findByURL(itm->url())->name(), "Type == 'Application'");
			for (uint i=0; i < m_offerList.count(); i++)
			{
				kdDebug() << "'\tservice: " << m_offerList[i]->name() << endl;
				apps->insertItem(SmallIcon(m_offerList[i]->icon()), m_offerList[i]->name(), i+1);
			}
			apps->insertSeparator();
			apps->insertItem(i18n("Other..."), 0);
			connect(apps, SIGNAL(activated(int)), this, SLOT(slotRun(int)));
			m_popup->insertItem(SmallIcon("fork"), i18n("&Open With"),apps);
		}

		if (itm->type() == KileType::File || itm->type() == KileType::ProjectItem)
		{
			m_popup->insertItem(SmallIcon("fileopen"), i18n("&Open"), KPV_ID_OPEN);
			m_popup->insertItem(SmallIcon("filesave"), i18n("&Save"), KPV_ID_SAVE);
			m_popup->insertSeparator();
		}

		if (itm->type() == KileType::File)
		{
			if (m_nProjects>0) m_popup->insertItem(i18n("&Add To Project"), KPV_ID_ADD);
			m_popup->insertSeparator();
			connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotFile(int)));
		}
		else if (itm->type() == KileType::ProjectItem || itm->type() == KileType::ProjectExtra)
		{
			KileProjectItem *pi  = m_ki->itemFor(itm->url());
			if (pi)
			{
				m_popup->insertItem(i18n("&Include in Archive"), KPV_ID_INCLUDE);
				m_popup->insertSeparator();
				m_popup->setItemChecked(KPV_ID_INCLUDE, pi->archive())	;
			}
			m_popup->insertItem(i18n("&Remove From Project"), KPV_ID_REMOVE);
			m_popup->insertSeparator();
			connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotProjectItem(int)));
		}
		else if (itm->type() == KileType::Project)
		{
			m_popup->insertItem(i18n("A&dd Files..."), KPV_ID_ADDFILES);
			m_popup->insertItem(UserIcon("relation"),i18n("Build Project &Tree"), KPV_ID_BUILDTREE);
   			m_popup->insertItem(SmallIcon("configure"),i18n("Project &Options"), KPV_ID_OPTIONS);
			m_popup->insertItem(SmallIcon("package"),i18n("&Archive"), KPV_ID_ARCHIVE);
			m_popup->insertSeparator();
			connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotProject(int)));
		}

		if ( (itm->type() == KileType::File) || (itm->type() == KileType::ProjectItem) || (itm->type()== KileType::Project)) m_popup->insertItem(SmallIcon("fileclose"), i18n("&Close"), KPV_ID_CLOSE);

		m_popup->exec(point);
	}
}

void KileProjectView::makeTheConnection(KileProjectViewItem *item)
{
	kdDebug() << "\tmakeTheConnection " << item->text(0) << endl;

	if (item->type() == KileType::Project)
	{
		KileProject *project = m_ki->projectFor(item->url());
		if (project==0)
			kdWarning() << "makeTheConnection COULD NOT FIND AN PROJECT OBJECT FOR " << item->url().path() << endl;
		else
			connect(project, SIGNAL(nameChanged(const QString &)), item, SLOT(nameChanged(const QString &)));
	}
	else
	{
		KileDocumentInfo *docinfo = m_ki->infoFor(item->url().path());
		item->setInfo(docinfo);
		if (docinfo ==0 ) {kdDebug() << "\tmakeTheConnection COULD NOT FIND A DOCINFO" << endl; return;}
		connect(docinfo, SIGNAL(nameChanged(const KURL&)),  item, SLOT(urlChanged(const KURL&)));
		connect(docinfo, SIGNAL(isrootChanged(bool)), item, SLOT(isrootChanged(bool)));
		//set the pixmap
		item->isrootChanged(docinfo->isLaTeXRoot());
	}
}

KileProjectViewItem* KileProjectView::nonSrc(const KileProjectItem *pi, KileProjectViewItem *item)
{
	KileProjectViewItem *parent = parentFor(pi, item);
	
	if (parent == 0)
	{
		kdError() << "no parent for " << pi->url().path() << endl;
		return 0;
	}

	if (parent->nonSrc())
		return parent->nonSrc();

	//create the non-source item
	KileProjectViewItem *nonsrc = new KileProjectViewItem(parent, i18n("non-sources"));
	parent->setNonSrc(nonsrc);
	nonsrc->setType(KileType::Folder);

	return nonsrc;
}

void KileProjectView::add(const KileProject *project)
{
	KileProjectViewItem *parent = new KileProjectViewItem(this, project->name());

	parent->setType(KileType::Project);
	parent->setURL(project->url());
	parent->setOpen(true);
	parent->setPixmap(0,UserIcon("relation"));
	makeTheConnection(parent);

	//KileProjectViewItem *nonsrc = new KileProjectViewItem(parent, i18n("non-source"));
	//parent->setNonSrc(nonsrc);

	refreshProjectTree(project);

	m_nProjects++;
}

KileProjectViewItem * KileProjectView::projectViewItemFor(const KURL & url)
{
	KileProjectViewItem *item = 0;

	//find project view item
	item = static_cast<KileProjectViewItem*>(firstChild());

	while ( item)
	{
		if ( (item->type() == KileType::Project) && (item->url() == url) )
			break;
		item = static_cast<KileProjectViewItem*>(item->nextSibling());
	}

	return item;
}

KileProjectViewItem * KileProjectView::itemFor(const KURL & url)
{
	KileProjectViewItem *item=0;

	QListViewItemIterator it(this);
	while (it.current())
	{
		item = static_cast<KileProjectViewItem*>(*it);
		if (item->url() == url)
			break;
		++it;
	}

	return item;
}

KileProjectViewItem* KileProjectView::parentFor(const KileProjectItem *projitem, KileProjectViewItem *projvi)
{
	//find parent projectviewitem of projitem
	KileProjectItem *parpi = projitem->parent();
	KileProjectViewItem *parpvi = projvi, *vi;

	if (parpi)
	{
		//find parent viewitem that has an URL parpi->url()
		QListViewItemIterator it( projvi );
		kdDebug() << "\tlooking for " << parpi->url().path() << endl;
		while (it.current())
		{
			vi = static_cast<KileProjectViewItem*>(*it);
			kdDebug() << "\t\t" << vi->url().path() << endl;
			if (vi->url() == parpi->url())
			{
				parpvi = vi;
				kdDebug() << "\t\tfound" <<endl;
				break;
			}
			++it;
		}

		kdDebug() << "\t\tnot found" << endl;
	}

	return parpvi;
}

KileProjectViewItem* KileProjectView::add(const KileProjectItem *projitem, KileProjectViewItem * projvi /* = 0*/)
{
	kdDebug() << "\tKileProjectView::adding projectitem " << projitem->path() << endl;

	const KileProject *project = projitem->project();

	if (projvi == 0 )
	{
		projvi= projectViewItemFor(project->url());
	}

	//KileProjectViewItem *parpvi = parentFor(projitem, projvi);

	kdDebug() << "\tparent projectviewitem " << projvi->url().fileName() << endl;

	KileProjectViewItem *item;

	if (projitem->type() == KileProjectItem::Other)
	{
		KileProjectViewItem *parent= nonSrc(projitem, projvi);
		item =  new KileProjectViewItem(parent,projitem->url().fileName());
		item->setType(KileType::ProjectExtra);
	}
	else
	{
		item =  new KileProjectViewItem(projvi, projitem->url().fileName());
		item->setType(KileType::ProjectItem);
	}

	item->setArchiveState(projitem->archive());
	item->setURL(projitem->url());
	makeTheConnection(item);

	projvi->sortChildItems(0,true);

	return item;
}

const KileProjectViewItem* KileProjectView::addTree(const KileProjectItem *projitem, KileProjectViewItem * projvi)
{
	KileProjectViewItem * item = add(projitem, projvi);

	if (projitem->firstChild())
		return addTree(projitem->firstChild(), item);

	if (projitem->sibling())
		return addTree(projitem->sibling(), projvi);

	if (projitem->parent())
	{
		if (projitem->parent()->sibling())
		{
			return addTree(projitem->parent()->sibling(), projvi->parent());
		}
	}

	return 0;
}

void KileProjectView::refreshProjectTree(const KileProject *project)
{
	kdDebug() << "\tKileProjectView::refreshProjectTree(" << project->name() << ")" << endl;
	KileProjectViewItem *parent= projectViewItemFor(project->url());

	//clean the tree
	if (parent)
	{
		kdDebug() << "\tusing parent projectviewitem " << parent->url().fileName() << endl;
		parent->setNonSrc(0);
		QListViewItem *vi = parent->firstChild(), *next;
		while (vi)
		{
			next = vi->nextSibling();
			delete vi;
			vi = next;
		}
	}
	else
		return;

	//create the non-sources dir
	//KileProjectViewItem *nonsrc = new KileProjectViewItem(parent, i18n("non-source"));
	//parent->setNonSrc(nonsrc);

	QPtrList<KileProjectItem> list = *(project->rootItems());
	QPtrListIterator<KileProjectItem> it(list);
	while (it.current())
	{
		addTree(*it, parent);
		++it;
	}

	parent->sortChildItems(0, true);
}

void KileProjectView::add(const KURL & url)
{
	kdDebug() << "\tKileProjectView::adding item " << url.path() << endl;
	//check if file is already present
	QListViewItemIterator it( this );
	KileProjectViewItem *item;
	while ( it.current())
	{
		item = static_cast<KileProjectViewItem*>(*it);
		if ( (item->type() != KileType::Project) && item->url() == url )
			return;
		it++;
	}

	item = new KileProjectViewItem(this, url.fileName());
	item->setType(KileType::File);
	item->setURL(url);
	makeTheConnection(item);
}

void KileProjectView::remove(const KileProject *project)
{
	KileProjectViewItem *item = static_cast<KileProjectViewItem*>(firstChild());

	while ( item)
	{
		if ( item->url() == project->url() )
		{
			takeItem(item);
			delete item;
			m_nProjects--;
			break;
		}
		item = static_cast<KileProjectViewItem*>(item->nextSibling());
	}
}

/**
 * Removes a file from the projectview, does not remove project-items. Only files without a project.
 **/
void KileProjectView::remove(const KURL &url)
{
	KileProjectViewItem *item = static_cast<KileProjectViewItem*>(firstChild());

	while ( item)
	{
		if ( (item->type() == KileType::File) && (item->url() == url) )
		{
			takeItem(item);
			delete item;
			break;
		}
		item = static_cast<KileProjectViewItem*>(item->nextSibling());
	}
}

void KileProjectView::removeItem(const KURL &url)
{
	QListViewItemIterator it( this );
	KileProjectViewItem *item;

	while ( it.current())
	{
		item = static_cast<KileProjectViewItem*>(*it);
		if ( (item->type() == KileType::ProjectItem) && (item->url() == url) )
		{
			item->parent()->takeItem(item);
			delete item;
			break;
		}
		it++;
	}
}

#include "kileprojectview.moc"
