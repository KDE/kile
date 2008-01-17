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

#include "widgets/projectview.h"

#include <q3header.h>
#include <QList>

//Added by qt3to4:
#include <QDropEvent>

#include <k3urldrag.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kurl.h>
#include <krun.h>
#include <kmimetype.h>

#include "kileinfo.h"
#include "documentinfo.h"
#include "kiledocmanager.h"

const int KPV_ID_OPEN = 0, KPV_ID_SAVE = 1, KPV_ID_CLOSE = 2,
	KPV_ID_OPTIONS = 3, KPV_ID_ADD = 4, KPV_ID_REMOVE = 5,
	KPV_ID_BUILDTREE = 6, KPV_ID_ARCHIVE = 7, KPV_ID_ADDFILES = 8,
	KPV_ID_INCLUDE = 9, KPV_ID_OPENWITH = 10, KPV_ID_OPENALLFILES = 11;

namespace KileWidget {

/*
 * ProjectViewItem
 */
void ProjectViewItem::urlChanged(const KUrl &url)
{
	// don't allow empty URLs
	if(!url.isEmpty()) 
	{
		setURL(url);
		setText(0, url.fileName());
	}
}

void ProjectViewItem::nameChanged(const QString & name)
{
	setText(0,name);
}

void ProjectViewItem::isrootChanged(bool isroot)
{
	KILE_DEBUG() << "SLOT isrootChanged " << text(0) << " to " << isroot << endl;
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

void ProjectViewItem::slotURLChanged(KileDocument::Info*, const KUrl & url)
{
	urlChanged(url);
}

int ProjectViewItem::compare(Q3ListViewItem * i, int col, bool ascending) const
{
	ProjectViewItem *item = dynamic_cast<ProjectViewItem*>(i);

	// sort: 
	//  - first:  container items in fixed order (projectfile, packages, images, other)
	//  - second: root items without container (sorted in ascending order)
	if ( item->type() == KileType::Folder )
	{
		if ( type() != KileType::Folder ) 
			return 1;
		else 
			return ( m_folder < item->folder() ) ? -1 : 1;
	}
	else if ( type() == KileType::Folder ) 
		return -1;
	else 
		return Q3ListViewItem::compare(i, col, ascending);
}

/*
 * ProjectView
 */
ProjectView::ProjectView(QWidget *parent, KileInfo *ki) : K3ListView(parent), m_ki(ki), m_nProjects(0), m_toggle(0)
{
	addColumn(i18n("Files & Projects"),-1);
	addColumn(i18n("Include in Archive"),10);
 	setSorting(-1);
	setFocusPolicy(Qt::ClickFocus);
	header()->hide();
	setRootIsDecorated(true);
	setAllColumnsShowFocus(true);
	setFullWidth(true);
	setSelectionModeExt(K3ListView::NoSelection);

	m_popup = new KMenu(this);

	connect(this, SIGNAL(contextMenu(K3ListView *, Q3ListViewItem *, const QPoint & )), this,SLOT(popup(K3ListView *, Q3ListViewItem * , const QPoint & )));

	connect(this, SIGNAL(executed(Q3ListViewItem*)), this, SLOT(slotClicked(Q3ListViewItem*)));
	setAcceptDrops(true);
	connect(this, SIGNAL(dropped(QDropEvent *, Q3ListViewItem *)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent *)));
}

void ProjectView::slotClicked(Q3ListViewItem *item)
{
	if (item == 0)
		item = currentItem();

	ProjectViewItem *itm = static_cast<ProjectViewItem*>(item);
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
				KMimeType::Ptr pMime = KMimeType::findByUrl(itm->url());
				if ( pMime->name().startsWith("text/") )
					emit(fileSelected(itm->url()));
				else
					KRun::runUrl(itm->url(), pMime->name(), this);
			}
		}
		clearSelection();
	}
}

void ProjectView::slotFile(int id)
{
	ProjectViewItem *item = static_cast<ProjectViewItem*>(currentItem());
	if (item )
	{
		if ( item->type() == KileType::File )
		{
			switch (id)
			{
				case KPV_ID_OPEN : emit(fileSelected(item->url())); break;
				case KPV_ID_SAVE : emit(saveURL(item->url())); break;
				case KPV_ID_ADD : emit(addToProject(item->url())); break;
				case KPV_ID_CLOSE : emit(closeURL(item->url())); return; //don't access "item" later on
				default : break;
			}
		}
	}
}

void ProjectView::slotProjectItem(int id)
{
	ProjectViewItem *item = static_cast<ProjectViewItem*>(currentItem());
	if ( item )
	{
		if ( item->type()==KileType::ProjectItem || item->type()==KileType::ProjectExtra )
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
				case KPV_ID_CLOSE : emit(closeURL(item->url())); break; //we can access "item" later as it isn't deleted
				case KPV_ID_OPENWITH :
					KRun::displayOpenWithDialog(item->url(), this);
					break;
				default : break;
			}
		}
	}
}

void ProjectView::slotProject(int id)
{
	ProjectViewItem *item = static_cast<ProjectViewItem*>(currentItem());
	if ( item )
	{
		if ( item->type() == KileType::Project )
		{
			switch (id)
			{
				case KPV_ID_BUILDTREE : emit(buildProjectTree(item->url())); break;
				case KPV_ID_OPTIONS : emit(projectOptions(item->url())); break;
				case KPV_ID_CLOSE : emit(closeProject(item->url())); return; //don't access "item" later on
				case KPV_ID_ARCHIVE : emit(projectArchive(item->url())); break;
				case KPV_ID_ADDFILES : emit(addFiles(item->url())); break;
				case KPV_ID_OPENALLFILES : emit(openAllFiles(item->url())); break;
				default : break;
			}
		}
	}
}

void ProjectView::slotRun(int id)
{
	ProjectViewItem *itm = static_cast<ProjectViewItem*>(currentItem());

	if (id == 0)
		KRun::displayOpenWithDialog(itm->url(), this);
	else
		KRun::run(*m_offerList[id-1], itm->url(), this);

	itm->setSelected(false);
}

//FIXME clean this mess up
void ProjectView::popup(K3ListView *, Q3ListViewItem *  item, const QPoint &  point)
{
#ifdef __GNUC__
#warning The popup menu still needs to be ported!
#endif
//FIXME: port for KDE4
/*
	if (item != 0)
	{
		ProjectViewItem *itm = static_cast<ProjectViewItem*>(item);
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
				KMenu *apps = new KMenu( m_popup);
				m_offerList = KMimeTypeTrader::self()->query(KMimeType::findByUrl(itm->url())->name(), "Type == 'Application'");
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
*/
}

void ProjectView::makeTheConnection(ProjectViewItem *item)
{
	KILE_DEBUG() << "\tmakeTheConnection " << item->text(0) << endl;

	if (item->type() == KileType::Project) {
		KileProject *project = m_ki->docManager()->projectFor(item->url());
		if (!project) {
			kWarning() << "makeTheConnection COULD NOT FIND AN PROJECT OBJECT FOR " << item->url().path() << endl;
		}
		else {
			connect(project, SIGNAL(nameChanged(const QString &)), item, SLOT(nameChanged(const QString &)));
		}
	}
	else {
		KileDocument::TextInfo *docinfo = m_ki->docManager()->textInfoFor(item->url().path());
		item->setInfo(docinfo);
		if (docinfo ==0 ) {KILE_DEBUG() << "\tmakeTheConnection COULD NOT FIND A DOCINFO" << endl; return;}
		connect(docinfo, SIGNAL(urlChanged(KileDocument::Info*, const KUrl&)),  item, SLOT(slotURLChanged(KileDocument::Info*, const KUrl&)));
		connect(docinfo, SIGNAL(isrootChanged(bool)), item, SLOT(isrootChanged(bool)));
		//set the pixmap
		item->isrootChanged(docinfo->isLaTeXRoot());
	}
}

ProjectViewItem* ProjectView::folder(const KileProjectItem *pi, ProjectViewItem *item)
{
	ProjectViewItem *parent = parentFor(pi, item);

	if (parent == 0)
	{
		kError() << "no parent for " << pi->url().path() << endl;
		return 0;
	}

	// we have already found the parent folder
	if ( parent->type() == KileType::Folder )
		return parent;

	// we are looking at the children, if there is an existing folder for this type
	ProjectViewItem *folder;

	// determine the foldername for this type
	QString foldername;
	switch(pi->type()) {
		case (KileProjectItem::ProjectFile):
			foldername = i18n("projectfile");
		break;
		case (KileProjectItem::Package):
			foldername = i18n("packages");
		break;
		case (KileProjectItem::Image):
			foldername = i18n("images");
		break;
		case (KileProjectItem::Other): 
		default :
			foldername = i18n("other");
		break;
	}

	// if there already a folder for this type on this level?
	bool found = false;
	folder = parent->firstChild();
	while ( folder )
	{
		if ( folder->text(0) == foldername )
		{
			found = true;
			break;
		}
		folder = folder->nextSibling();
	}
	
	// if no folder was found, we must create a new one
	if ( ! found )
	{
		folder = new ProjectViewItem(parent,foldername);
		KILE_DEBUG() << "new folder: " << parent->url().url() << endl;

		folder->setFolder(pi->type());
		folder->setType(KileType::Folder);
	}

	return folder;
}

void ProjectView::add(const KileProject *project)
{
	ProjectViewItem *parent = new ProjectViewItem(this, project);
	
	parent->setType(KileType::Project);
	parent->setURL(project->url());
	parent->setOpen(true);
	parent->setPixmap(0,SmallIcon("relation"));
	makeTheConnection(parent);

	//ProjectViewItem *nonsrc = new ProjectViewItem(parent, i18n("non-source"));
	//parent->setNonSrc(nonsrc);

	refreshProjectTree(project);

	++m_nProjects;
}

ProjectViewItem * ProjectView::projectViewItemFor(const KUrl & url)
{
	ProjectViewItem *item = 0;

	//find project view item
	item = static_cast<ProjectViewItem*>(firstChild());

	while ( item)
	{
		if ( (item->type() == KileType::Project) && (item->url() == url) )
			break;
		item = static_cast<ProjectViewItem*>(item->nextSibling());
	}

	return item;
}

ProjectViewItem * ProjectView::itemFor(const KUrl & url)
{
	ProjectViewItem *item=0;

	Q3ListViewItemIterator it(this);
	while (it.current())
	{
		item = static_cast<ProjectViewItem*>(*it);
		if (item->url() == url)
			break;
		++it;
	}

	return item;
}

ProjectViewItem* ProjectView::parentFor(const KileProjectItem *projitem, ProjectViewItem *projvi)
{
	//find parent projectviewitem of projitem
	KileProjectItem *parpi = projitem->parent();
	ProjectViewItem *parpvi = projvi, *vi;

	if (parpi) {
		//find parent viewitem that has an URL parpi->url()
		Q3ListViewItemIterator it( projvi );
		KILE_DEBUG() << "\tlooking for " << parpi->url().path() << endl;
		while (it.current())
		{
			vi = static_cast<ProjectViewItem*>(*it);
			KILE_DEBUG() << "\t\t" << vi->url().path() << endl;
			if (vi->url() == parpi->url())
			{
				parpvi = vi;
				KILE_DEBUG() << "\t\tfound" <<endl;
				break;
			}
			++it;
		}

		KILE_DEBUG() << "\t\tnot found" << endl;
	}
	else {
		KILE_DEBUG() << "\tlooking for folder type " << projitem->type() << endl;
		for (parpvi = parpvi->firstChild(); parpvi; parpvi = parpvi->nextSibling())
		{
			if ( (parpvi->type() == KileType::Folder) && (parpvi->folder() == projitem->type()) )
			{
				KILE_DEBUG() << "\t\tfound" << endl;
				break;
			}
		}
	}

	return (parpvi == 0) ? projvi : parpvi;
}

ProjectViewItem* ProjectView::add(KileProjectItem *projitem, ProjectViewItem * projvi /* = 0*/)
{
	KILE_DEBUG() << "\tProjectView::adding projectitem " << projitem->path() << endl;

	const KileProject *project = projitem->project();

	if (!projvi) {
		projvi = projectViewItemFor(project->url());
	}

	KILE_DEBUG() << "\tparent projectviewitem " << projvi->url().fileName() << endl;

	ProjectViewItem *item, *parent;

	switch (projitem->type()) {
	case (KileProjectItem::Source):
		item = new ProjectViewItem(projvi, projitem);
		item->setType(KileType::ProjectItem);
		item->setPixmap(0,SmallIcon("projectitem"));
		break;
	case (KileProjectItem::Package):
		parent = folder(projitem, projvi);
		item = new ProjectViewItem(parent, projitem);
		item->setType(KileType::ProjectItem);
		item->setPixmap(0,SmallIcon("projectitem"));
		break;
	case (KileProjectItem::ProjectFile):
	default:
		parent = folder(projitem, projvi);
		item = new ProjectViewItem(parent, projitem);
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

void ProjectView::addTree(KileProjectItem *projitem, ProjectViewItem * projvi)
{
	ProjectViewItem * item = add(projitem, projvi);

	if (projitem->firstChild())
		addTree(projitem->firstChild(), item);

	if (projitem->sibling())
		addTree(projitem->sibling(), projvi);
}

void ProjectView::refreshProjectTree(const KileProject *project)
{
	KILE_DEBUG() << "\tProjectView::refreshProjectTree(" << project->name() << ")" << endl;
	ProjectViewItem *parent= projectViewItemFor(project->url());

	//clean the tree
	if(parent) {
		KILE_DEBUG() << "\tusing parent projectviewitem " << parent->url().fileName() << endl;
		parent->setFolder(-1);
		Q3ListViewItem *vi = parent->firstChild(), *next;
		while (vi) {
			next = vi->nextSibling();
			delete vi;
			vi = next;
		}
	}
	else {
		return;
	}

	//create the non-sources dir
	//ProjectViewItem *nonsrc = new ProjectViewItem(parent, i18n("non-sources"));
	//parent->setNonSrc(nonsrc);



	QList<KileProjectItem*> list = project->rootItems();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		addTree(*it, parent);
	}

	parent->sortChildItems(0, true);
}

void ProjectView::add(const KUrl& url)
{
	KILE_DEBUG() << "\tProjectView::adding item " << url.path() << endl;
	//check if file is already present
	Q3ListViewItemIterator it( this );
	ProjectViewItem *item;
	while ( it.current())
	{
		item = static_cast<ProjectViewItem*>(*it);
		if ( (item->type() != KileType::Project) && (item->url() == url) )
			return;
		++it;
	}

	item = new ProjectViewItem(this, url.fileName());
	item->setType(KileType::File);
	item->setURL(url);
	makeTheConnection(item);
}

void ProjectView::remove(const KileProject *project)
{
	ProjectViewItem *item = static_cast<ProjectViewItem*>(firstChild());

	while ( item)
	{
		if ( item->url() == project->url() )
		{
			takeItem(item);
			delete item;
			--m_nProjects;
			break;
		}
		item = static_cast<ProjectViewItem*>(item->nextSibling());
	}
}

/**
 * Removes a file from the projectview, does not remove project-items. Only files without a project.
 **/
void ProjectView::remove(const KUrl &url)
{
	ProjectViewItem *item = static_cast<ProjectViewItem*>(firstChild());

	while ( item)
	{
		if ( (item->type() == KileType::File) && (item->url() == url) )
		{
			takeItem(item);
			delete item;
			break;
		}
		item = static_cast<ProjectViewItem*>(item->nextSibling());
	}
}

void ProjectView::removeItem(const KileProjectItem *projitem, bool open)
{
	Q3ListViewItemIterator it( this );
	ProjectViewItem *item;
	while ( it.current())
	{
		item = static_cast<ProjectViewItem*>(*it);
		if ( (item->type() == KileType::ProjectItem) && (item->projectItem() == projitem) )
		{
			KILE_DEBUG() << "removing projectviewitem" << endl;
			item->parent()->takeItem(item);
			delete item;
		}
		++it;
	}

	if ( open )
	{
		item = new ProjectViewItem(this, projitem->url().fileName());
		item->setType(KileType::File);
		item->setURL(projitem->url());
		makeTheConnection(item);
	}

}

bool ProjectView::acceptDrag(QDropEvent *e) const
{
	return K3URLDrag::canDecode(e); // only accept URL drops
}

}

#include "projectview.moc"
