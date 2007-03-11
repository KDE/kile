/***************************************************************************
    begin                : Tue Aug 12 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                           (C) 2006 by Michel Ludwig
    email                : Jeroen.Wijnhout@kdemail.net
                           michel.ludwig@kdemail.net
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
#include <kurldrag.h>

#include "kileinfo.h"
#include "kiledocumentinfo.h"
#include "kiledocmanager.h"
#include "kileprojectview.h"

const int KPV_ID_OPEN = 0, KPV_ID_SAVE = 1, KPV_ID_CLOSE = 2,
	KPV_ID_OPTIONS = 3, KPV_ID_ADD = 4, KPV_ID_REMOVE = 5,
	KPV_ID_BUILDTREE = 6, KPV_ID_ARCHIVE = 7, KPV_ID_ADDFILES = 8,
	KPV_ID_INCLUDE = 9, KPV_ID_OPENWITH = 10, KPV_ID_OPENALLFILES = 11;

/*
 * KileProjectViewItem
 */
void KileProjectViewItem::urlChanged(const KURL &url)
{
	// don't allow empty URLs
	if(!url.isEmpty()) 
	{
		setURL(url);
		setText(0, url.fileName());
	}
}

void KileProjectViewItem::nameChanged(const QString & name)
{
	setText(0,name);
}

void KileProjectViewItem::isrootChanged(bool isroot)
{
	kdDebug() << "SLOT isrootChanged " << text(0) << " to " << isroot << endl;
	if (isroot)
	{
		setPixmap(0,SmallIcon("masteritem"));
	}
	else
	{
		if ( text(0).right(7) == ".kilepr" )
			setPixmap(0,SmallIcon("kile"));
		else if (type() == KileType::ProjectItem)
			setPixmap(0,SmallIcon("projectitem"));
		else
			setPixmap(0,SmallIcon("file"));
	}
}

void KileProjectViewItem::slotURLChanged(KileDocument::Info*, const KURL & url)
{
	urlChanged(url);
}

int KileProjectViewItem::compare(QListViewItem * i, int col, bool ascending) const
{
	KileProjectViewItem *item = dynamic_cast<KileProjectViewItem*>(i);
	if ( (item->type() == KileType::Folder) &&  (type() != KileType::Folder) ) return 1;
	else if ( (item->type() != KileType::Folder) &&  (type() == KileType::Folder) ) return -1;
	else return QListViewItem::compare(i, col, ascending);
}

/*
 * KileProjectView
 */
KileProjectView::KileProjectView(QWidget *parent, KileInfo *ki) : KListView(parent), m_ki(ki), m_nProjects(0), m_toggle(0)
{
	addColumn(i18n("Files & Projects"),-1);
	addColumn(i18n("Include in Archive"),10);
 	setSorting(-1);
	setFocusPolicy(QWidget::ClickFocus);
	header()->hide();
	setRootIsDecorated(true);

	m_popup = new KPopupMenu(this, "projectview_popup");

	connect(this, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint & )), this,SLOT(popup(KListView *, QListViewItem * , const QPoint & )));

	connect(this, SIGNAL(executed(QListViewItem*)), this, SLOT(slotClicked(QListViewItem*)));
	setAcceptDrops(true);
	connect(this, SIGNAL(dropped(QDropEvent *, QListViewItem *)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent *)));
}

void KileProjectView::slotClicked(QListViewItem *item)
{
	if (item == 0)
		item = currentItem();

	KileProjectViewItem *itm = static_cast<KileProjectViewItem*>(item);
	if (itm)
	{
		if (itm->type() == KileType::File )
			emit(fileSelected(itm->url()));
		else if ( itm->type() == KileType::ProjectItem )
			emit(fileSelected(itm->projectItem()));
		else if ( itm->type() != KileType::Folder )
		{
			// don't open project configuration files (*.kilepr)
			if ( itm->url().path().right(7) != ".kilepr" )
			{
				//determine mimeType and open file with preferred application
				KMimeType::Ptr pMime = KMimeType::findByURL(itm->url());
				if ( pMime->name().startsWith("text/") )
					emit(fileSelected(itm->url()));
				else
					KRun::runURL(itm->url(), pMime->name());
			}
		}
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
	KileProjectViewItem *item = static_cast<KileProjectViewItem*>(currentItem());
	if (item && (item->type() == KileType::ProjectItem || item->type() == KileType::ProjectExtra))
	{
		switch (id)
		{
			case KPV_ID_OPEN : emit(fileSelected(item->projectItem())); break;
			case KPV_ID_SAVE : emit(saveURL(item->url())); break;
			case KPV_ID_REMOVE :
				emit(removeFromProject(item->projectItem()));
				break;
			case KPV_ID_INCLUDE :
				if (item->text(1) == "*") item->setText(1,"");
				else item->setText(1,"*");

				emit(toggleArchive(item->projectItem()));
				break;
			case KPV_ID_CLOSE : emit(closeURL(item->url())); break;
			case KPV_ID_OPENWITH :
				KRun::displayOpenWithDialog(item->url());
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
			case KPV_ID_OPENALLFILES : emit(openAllFiles(item->url())); break;
			default : break;
		}
	}
}

void KileProjectView::slotRun(int id)
{
	KileProjectViewItem *itm = static_cast<KileProjectViewItem*>(currentItem());

	if (id == 0)
		KRun::displayOpenWithDialog(itm->url());
	else
		KRun::run(*m_offerList[id-1], itm->url());
}

//FIXME clean this mess up
void KileProjectView::popup(KListView *, QListViewItem *  item, const QPoint &  point)
{
	if (item != 0)
	{
		KileProjectViewItem *itm = static_cast<KileProjectViewItem*>(item);
		if ( itm->type() == KileType::Folder )
			return;
		 
		m_popup->clear();
		m_popup->disconnect();

		bool isKilePrFile = false;
		if (itm->type() != KileType::Project && itm->projectItem() && itm->projectItem()->project())
			isKilePrFile = itm->projectItem()->project()->url() == itm->url();

		bool insertsep = false; 
		if (itm->type() == KileType::ProjectExtra)
		{
			if ( ! isKilePrFile )
			{
				KPopupMenu *apps = new KPopupMenu( m_popup);
				m_offerList = KTrader::self()->query(KMimeType::findByURL(itm->url())->name(), "Type == 'Application'");
				for (uint i=0; i < m_offerList.count(); ++i)
					apps->insertItem(SmallIcon(m_offerList[i]->icon()), m_offerList[i]->name(), i+1);

				apps->insertSeparator();
				apps->insertItem(i18n("Other..."), 0);
				connect(apps, SIGNAL(activated(int)), this, SLOT(slotRun(int)));
				m_popup->insertItem(SmallIcon("fork"), i18n("&Open With"),apps);
				insertsep = true;
			}
		}

		if (itm->type() == KileType::File || itm->type() == KileType::ProjectItem)
		{
			if ( ! m_ki->isOpen(itm->url()) )
				m_popup->insertItem(SmallIcon("fileopen"), i18n("&Open"), KPV_ID_OPEN);
			else
				m_popup->insertItem(SmallIcon("filesave"), i18n("&Save"), KPV_ID_SAVE);
			insertsep = true;
		}

		if (itm->type() == KileType::File)
		{
			if ( m_nProjects > 0)
			{
				if ( insertsep )
					m_popup->insertSeparator();
			   m_popup->insertItem(SmallIcon("project_add"),i18n("&Add to Project"), KPV_ID_ADD);
			   insertsep = true;
			}
			connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotFile(int)));
		}
		else if (itm->type() == KileType::ProjectItem || itm->type() == KileType::ProjectExtra)
		{
			KileProjectItem *pi  = itm->projectItem();
			if (pi)
			{
				if ( insertsep )
					m_popup->insertSeparator();
				m_popup->insertItem(i18n("&Include in Archive"), KPV_ID_INCLUDE);
				m_popup->setItemChecked(KPV_ID_INCLUDE, pi->archive());
				insertsep = true;
			}
			if ( !isKilePrFile ) 
			{
				if ( insertsep )
					m_popup->insertSeparator();
				m_popup->insertItem(SmallIcon("project_remove"),i18n("&Remove From Project"), KPV_ID_REMOVE);
				insertsep = true;
			}
   			connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotProjectItem(int)));
		}
		else if (itm->type() == KileType::Project)
		{
			if ( insertsep )
				m_popup->insertSeparator();
			m_popup->insertItem(i18n("A&dd Files..."), KPV_ID_ADDFILES);
			m_popup->insertSeparator();
			m_popup->insertItem(i18n("Open All &Project Files"), KPV_ID_OPENALLFILES);
			m_popup->insertSeparator();
			m_popup->insertItem(SmallIcon("relation"),i18n("Refresh Project &Tree"), KPV_ID_BUILDTREE);
			m_popup->insertItem(SmallIcon("configure"),i18n("Project &Options"), KPV_ID_OPTIONS);
			m_popup->insertItem(SmallIcon("package"),i18n("&Archive"), KPV_ID_ARCHIVE);
			connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotProject(int)));
			insertsep = true;
		}

		if ( (itm->type() == KileType::File) || (itm->type() == KileType::ProjectItem) || (itm->type()== KileType::Project))
		{
			if ( insertsep )
				m_popup->insertSeparator();
			m_popup->insertItem(SmallIcon("fileclose"), i18n("&Close"), KPV_ID_CLOSE);
		}

		m_popup->exec(point);
	}
}

void KileProjectView::makeTheConnection(KileProjectViewItem *item)
{
	kdDebug() << "\tmakeTheConnection " << item->text(0) << endl;

	if (item->type() == KileType::Project)
	{
		KileProject *project = m_ki->docManager()->projectFor(item->url());
		if (project==0)
			kdWarning() << "makeTheConnection COULD NOT FIND AN PROJECT OBJECT FOR " << item->url().path() << endl;
		else
			connect(project, SIGNAL(nameChanged(const QString &)), item, SLOT(nameChanged(const QString &)));
	}
	else
	{
		KileDocument::TextInfo *docinfo = m_ki->docManager()->textInfoFor(item->url().path());
		item->setInfo(docinfo);
		if (docinfo ==0 ) {kdDebug() << "\tmakeTheConnection COULD NOT FIND A DOCINFO" << endl; return;}
		connect(docinfo, SIGNAL(urlChanged(KileDocument::Info*, const KURL&)),  item, SLOT(slotURLChanged(KileDocument::Info*, const KURL&)));
		connect(docinfo, SIGNAL(isrootChanged(bool)), item, SLOT(isrootChanged(bool)));
		//set the pixmap
		item->isrootChanged(docinfo->isLaTeXRoot());
	}
}

KileProjectViewItem* KileProjectView::folder(const KileProjectItem *pi, KileProjectViewItem *item)
{
	KileProjectViewItem *parent = parentFor(pi, item);

	if (parent == 0)
	{
		kdError() << "no parent for " << pi->url().path() << endl;
		return 0;
	}

	if (parent->type() == KileType::Folder)
		return parent;

	//create the folder item
	KileProjectViewItem *folder;

	switch (pi->type()) {
	case (KileProjectItem::Package):
		folder = new KileProjectViewItem(parent, i18n("packages"));
		break;
	case (KileProjectItem::Image):
		folder = new KileProjectViewItem(parent, i18n("images"));
		break;
	case (KileProjectItem::Other): default :
		folder = new KileProjectViewItem(parent, i18n("other"));
		break;
	}

	kdDebug() << "new folder: " << parent->url().url() << endl;

	folder->setFolder(pi->type());
	folder->setType(KileType::Folder);

	return folder;
}

void KileProjectView::add(const KileProject *project)
{
	KileProjectViewItem *parent = new KileProjectViewItem(this, project);
	
	parent->setType(KileType::Project);
	parent->setURL(project->url());
	parent->setOpen(true);
	parent->setPixmap(0,SmallIcon("relation"));
	makeTheConnection(parent);

	//KileProjectViewItem *nonsrc = new KileProjectViewItem(parent, i18n("non-source"));
	//parent->setNonSrc(nonsrc);

	refreshProjectTree(project);

	++m_nProjects;
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

	if (parpi) {
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
	else {
		kdDebug() << "\tlooking for folder type " << projitem->type() << endl;
		for (parpvi = parpvi->firstChild(); parpvi; parpvi = parpvi->nextSibling())
		{
			if ((parpvi->type() == KileType::Folder) &&
//				(parpvi->folder() == projitem->type()))
				((parpvi->folder() == projitem->type()) || (projitem->type()==KileProjectItem::ProjectFile && parpvi->folder()==KileProjectItem::Other) ))
			{
				kdDebug() << "\t\tfound" << endl;
				break;
			}
		}
	}

	return (parpvi == 0) ? projvi : parpvi;
}

KileProjectViewItem* KileProjectView::add(KileProjectItem *projitem, KileProjectViewItem * projvi /* = 0*/)
{
	kdDebug() << "\tKileProjectView::adding projectitem " << projitem->path() << endl;

	const KileProject *project = projitem->project();

	if (projvi == 0 )
	{
		projvi= projectViewItemFor(project->url());
	}

	kdDebug() << "\tparent projectviewitem " << projvi->url().fileName() << endl;

	KileProjectViewItem *item, *parent;

	switch (projitem->type()) {
	case (KileProjectItem::Source):
		item = new KileProjectViewItem(projvi, projitem);
		item->setType(KileType::ProjectItem);
		item->setPixmap(0,SmallIcon("projectitem"));
		break;
	case (KileProjectItem::Package):
		parent = folder(projitem, projvi);
		item = new KileProjectViewItem(parent, projitem);
		item->setType(KileType::ProjectItem);
		item->setPixmap(0,SmallIcon("projectitem"));
		break;
	case (KileProjectItem::ProjectFile):
	default:
		parent = folder(projitem, projvi);
		item = new KileProjectViewItem(parent, projitem);
		item->setType(KileType::ProjectExtra);
		item->setPixmap(0,SmallIcon( (projitem->type()==KileProjectItem::ProjectFile) ? "kile" : "file" ));
		break;
	}

	item->setArchiveState(projitem->archive());
	item->setURL(projitem->url());
	makeTheConnection(item);

	projvi->sortChildItems(0,true);

	return item;
}

void KileProjectView::addTree(KileProjectItem *projitem, KileProjectViewItem * projvi)
{
	KileProjectViewItem * item = add(projitem, projvi);

	if (projitem->firstChild())
		addTree(projitem->firstChild(), item);

	if (projitem->sibling())
		addTree(projitem->sibling(), projvi);
}

void KileProjectView::refreshProjectTree(const KileProject *project)
{
	kdDebug() << "\tKileProjectView::refreshProjectTree(" << project->name() << ")" << endl;
	KileProjectViewItem *parent= projectViewItemFor(project->url());

	//clean the tree
	if (parent)
	{
		kdDebug() << "\tusing parent projectviewitem " << parent->url().fileName() << endl;
		parent->setFolder(-1);
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
	//KileProjectViewItem *nonsrc = new KileProjectViewItem(parent, i18n("non-sources"));
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
		if ( (item->type() != KileType::Project) && (item->url() == url) )
			return;
		++it;
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
			--m_nProjects;
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

void KileProjectView::removeItem(const KileProjectItem *projitem, bool open)
{
	QListViewItemIterator it( this );
	KileProjectViewItem *item;
	while ( it.current())
	{
		item = static_cast<KileProjectViewItem*>(*it);
		if ( (item->type() == KileType::ProjectItem) && (item->projectItem() == projitem) )
		{
			kdDebug() << "removing projectviewitem" << endl;
			item->parent()->takeItem(item);
			delete item;
		}
		++it;
	}

	if ( open )
	{
		item = new KileProjectViewItem(this, projitem->url().fileName());
		item->setType(KileType::File);
		item->setURL(projitem->url());
		makeTheConnection(item);
	}

}

bool KileProjectView::acceptDrag(QDropEvent *e) const
{
	return KURLDrag::canDecode(e); // only accept URL drops
}

#include "kileprojectview.moc"
