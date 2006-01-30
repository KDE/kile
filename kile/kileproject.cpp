/***************************************************************************
    begin                : Fri Aug 1 2003
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
#include "kileproject.h"
#include "kileversion.h"

#include <qstringlist.h>
#include <qfileinfo.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kiledocumentinfo.h"
#include "kiletoolmanager.h"

/*
 01/24/06 tbraun
	Added the logic to get versioned kilepr files
	We also warn if the user wants to open a project file with a different kileprversion
*/


/*
 * KileProjectItem
 */
KileProjectItem::KileProjectItem(KileProject *project, const KURL & url, int type) :
	m_project(project),
	m_url(url),
	m_type(type),
	m_docinfo(0L),
	m_parent(0L),
	m_child(0L),
	m_sibling(0L),
	m_nLine(0)
{
	m_highlight=m_encoding=QString::null; m_bOpen = m_archive = true;

	if (project)
		project->add(this);
}

void KileProjectItem::setParent(KileProjectItem * item)
{
	m_parent = item;

	//update parent info
	if (m_parent)
	{
		if (m_parent->firstChild())
		{
			//get last child
			KileProjectItem *sib = m_parent->firstChild();
			while (sib->sibling())
				sib = sib->sibling();

			sib->setSibling(this);
		}
		else
			m_parent->setChild(this);
	}
	else
	{
		setChild(0);
		setSibling(0);
	}
}

void KileProjectItem::print(int level)
{
	QString str;
	str.fill('\t', level);
	kdDebug() << str << "+" << url().fileName() << endl;

	if (firstChild())
		firstChild()->print(++level);

	if (sibling())
		sibling()->print(level);
}

void KileProjectItem::allChildren(QPtrList<KileProjectItem> *list) const
{
	KileProjectItem *item = firstChild();

// 	kdDebug() << "\tKileProjectItem::allChildren(" << list->count() << ")" << endl;
	while(item != 0)
	{
		list->append(item);
// 		kdDebug() << "\t\tappending " << item->url().fileName() << endl;
		item->allChildren(list);
		item = item->sibling();
	}
}

void KileProjectItem::setInfo(KileDocument::Info *docinfo)
{
	m_docinfo = docinfo;
	if(docinfo)
	{
	connect(docinfo,SIGNAL(nameChanged(const KURL &)), this, SLOT(changeURL(const KURL &)));
	connect(docinfo,SIGNAL(depChanged()), m_project, SLOT(buildProjectTree()));
	}
}

/*
 * KileProject
 */
KileProject::KileProject(const QString& name, const KURL& url) : QObject(0,name.ascii()), m_invalid(false), m_masterDocument(QString::null), m_useMakeIndexOptions(false)
{
	init(name, url);
}

KileProject::KileProject(const KURL& url) : QObject(0,url.fileName().ascii()), m_invalid(false), m_masterDocument(QString::null), m_useMakeIndexOptions(false)
{
	init(url.fileName(), url);
}	

KileProject::~KileProject()
{
	kdDebug() << "DELETING KILEPROJECT " <<  m_projecturl.url() << endl;
	delete m_config;
}

void KileProject::init(const QString& name, const KURL& url)
{
	m_name = name;
	m_projecturl = url;
	m_projectitems.setAutoDelete(true);

	m_config = new KSimpleConfig(m_projecturl.path());

	m_baseurl = m_projecturl.directory();
	m_baseurl.cleanPath(true);

	kdDebug() << "KileProject m_baseurl = " << m_baseurl.path() << endl;

	if (QFileInfo(url.path()).exists())
	{
		load();
	}
	else
	{
		//create the project file
		m_config->setGroup("General");
		m_config->writeEntry("name", m_name);
		m_config->writeEntry("kileprversion", kilePrVersion);
		m_config->writeEntry("kileversion", kileVersion);
		m_config->sync();
	}
}

void KileProject::setLastDocument(const KURL &url)
{
    if ( item(url) != 0 )
        m_lastDocument = url;
}

void KileProject::setExtensions(KileProjectItem::Type type, const QString & ext)
{
	QString pattern = ext;

	if (type < 1) 
	{
		kdWarning() << "ERROR: TYPE < 1" << endl;
		return;
	}

	if (ext.stripWhiteSpace().length() == 0) pattern = "";

	if ( (!pattern.isEmpty()) && !extIsRegExp(type))
	{
		QStringList lst = QStringList::split(" ", ext);
		pattern = lst.join("|");
		pattern.replace(".","\\.");
		pattern ="("+ pattern +")$";
	}

	if ( extIsRegExp(type) )
		pattern = ext.stripWhiteSpace();

	m_reExtensions[type-1].setPattern(pattern);

	if (m_extensions[type-1] != ext)
	{
		m_extensions[type-1] = ext.stripWhiteSpace();
		buildProjectTree();
	}
}

void KileProject::setType(KileProjectItem *item)
{
	if ( item->path().right(7) == ".kilepr" )
	{
		item->setType(KileProjectItem::ProjectFile);
		return;
	}

	bool unknown = true;
	for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i)
		if ( (!extensions((KileProjectItem::Type) i).isEmpty()) &&
			m_reExtensions[i-1].search(item->url().fileName()) != -1)
		{
			item->setType(i);
			unknown = false;
			break;
		}

	if (unknown) item->setType(KileProjectItem::Other);
}

void KileProject::readMakeIndexOptions()
{
	QString grp = KileTool::groupFor("MakeIndex", m_config);

	//get the default value
	KConfig *cfg = KGlobal::config();
	cfg->setGroup(KileTool::groupFor("MakeIndex", KileTool::configName("MakeIndex", cfg)));
	QString deflt = cfg->readEntry("options", "'%S'.idx");

	if ( useMakeIndexOptions() && !grp.isEmpty() )
	{
		m_config->setGroup(grp);
		QString val = m_config->readEntry("options", deflt);
		if ( val.isEmpty() ) val = deflt;
		setMakeIndexOptions(val);
	}
	else //use default value
		setMakeIndexOptions(deflt);
}

void KileProject::writeUseMakeIndexOptions()
{
	if ( useMakeIndexOptions() )
		KileTool::setConfigName("MakeIndex", "Default", m_config);
	else 
		KileTool::setConfigName("MakeIndex", "", m_config);
}

QString KileProject::addBaseURL(const QString &path)
{
  if ( path.isEmpty() || path.startsWith("/") )
  {
    return path;
  }
  else
  {
    return m_baseurl.path() + "/" + path;
  }
}

QString KileProject::removeBaseURL(const QString &path)
{

  if ( path.startsWith("/") )
  {
    QFileInfo info(path);
    QString relPath = findRelativePath(path);
    kdDebug() << "removeBaseURL path is" << path << " , relPath is " << relPath << endl;
    return relPath;
  }
  else
  {
    return path;
  }
}

bool KileProject::load()
{
	kdDebug() << "KileProject: loading..." <<endl;

	//load general settings/options
	m_config->setGroup("General");
	m_name = m_config->readEntry("name", i18n("Project"));
	m_kileversion = m_config->readEntry("kileversion",QString::null);
	m_kileprversion = m_config->readEntry("kileprversion",QString::null);

	if(!m_kileprversion.isNull() && m_kileprversion.toInt() > kilePrVersion.toInt())
	{
		if(KMessageBox::warningYesNo(0L,i18n("The project file of %1 was created by a newer version of kile.\
				Opening it can lead to unexpected results.\n\
				Do you really want to continue (not recommended)?").arg(m_name),
				 QString::null, KStdGuiItem::yes(), KStdGuiItem::no(),QString::null,KMessageBox::Dangerous) == KMessageBox::No)
		{
			m_invalid=true;
			return false;
		}
	}
	
	QString master = addBaseURL(m_config->readEntry("masterDocument", QString::null));
  	kdDebug() << "LOADED MASTER = " << master << endl;
	setMasterDocument(master);

	// IsRegExp has to be loaded _before_ the Extensions
	setExtIsRegExp(KileProjectItem::Source, m_config->readBoolEntry("src_extIsRegExp", false));
	setExtensions(KileProjectItem::Source, m_config->readEntry("src_extensions", SOURCE_EXTENSIONS));
	setExtIsRegExp(KileProjectItem::Package, m_config->readBoolEntry("pkg_extIsRegExp", false));
	setExtensions(KileProjectItem::Package, m_config->readEntry("pkg_extensions", PACKAGE_EXTENSIONS));
	setExtIsRegExp(KileProjectItem::Image, m_config->readBoolEntry("img_extIsRegExp", false));
	setExtensions(KileProjectItem::Image, m_config->readEntry("img_extensions", IMAGE_EXTENSIONS));

	setQuickBuildConfig(KileTool::configName("QuickBuild", m_config));
	readMakeIndexOptions();

	KURL url;
	KileProjectItem *item;
	QStringList groups = m_config->groupList();

	//retrieve all the project files and create and initialize project items for them
	for (uint i=0; i < groups.count(); ++i)
	{
		if (groups[i].left(5) == "item:")
		{
			QString path = groups[i].mid(5);
			if (path[0] == '/' )
			{
				url = KURL::fromPathOrURL(path);
			}
			else
			{
				url = m_baseurl;
				url.addPath(path);
				url.cleanPath(true);
			}
			item = new KileProjectItem(this, url);
			setType(item);

			m_config->setGroup(groups[i]);
			item->setOpenState(m_config->readBoolEntry("open", true));
			item->setEncoding(m_config->readEntry("encoding", QString::null));
			item->setHighlight(m_config->readEntry("highlight",QString::null));
			item->setArchive(m_config->readBoolEntry("archive", true));
			item->setLineNumber(m_config->readNumEntry("line", 0));
			item->setColumnNumber(m_config->readNumEntry("column", 0));
			item->changePath(groups[i].mid(5));

			connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );
		}
	}

    // only call this after all items are created, otherwise setLastDocument doesn't accept the url
    m_config->setGroup("General");
    setLastDocument(KURL::fromPathOrURL(addBaseURL(m_config->readEntry("lastDocument", QString::null))));

	// dump();

	return true;
}

bool KileProject::save()
{
	kdDebug() << "KileProject: saving..." <<endl;

	m_config->setGroup("General");
	m_config->writeEntry("name", m_name);
	m_config->writeEntry("kileprversion", kilePrVersion);
	m_config->writeEntry("kileversion", kileVersion);

  	kdDebug() << "KileProject::save() masterDoc = " << m_masterDocument << endl;
	m_config->writeEntry("masterDocument", removeBaseURL(m_masterDocument));
  	m_config->writeEntry("lastDocument", removeBaseURL(m_lastDocument.path()));

	m_config->writeEntry("src_extensions", extensions(KileProjectItem::Source));
	m_config->writeEntry("src_extIsRegExp", extIsRegExp(KileProjectItem::Source));
	m_config->writeEntry("pkg_extensions", extensions(KileProjectItem::Package));
	m_config->writeEntry("pkg_extIsRegExp", extIsRegExp(KileProjectItem::Package));
	m_config->writeEntry("img_extensions", extensions(KileProjectItem::Image));
	m_config->writeEntry("img_extIsRegExp", extIsRegExp(KileProjectItem::Image));

	KileProjectItem *item;
	for (uint i=0; i < m_projectitems.count(); ++i)
	{
		item = m_projectitems.at(i);
		m_config->setGroup("item:"+item->path());
		m_config->writeEntry("open", item->isOpen());
		m_config->writeEntry("encoding", item->encoding());
		m_config->writeEntry("highlight", item->highlight());
		m_config->writeEntry("archive", item->archive());
		m_config->writeEntry("line", item->lineNumber());
		m_config->writeEntry("column", item->columnNumber());
	}

	KileTool::setConfigName("QuickBuild", quickBuildConfig(), m_config);

	writeUseMakeIndexOptions();
	if ( useMakeIndexOptions() ) 
	{
		
		QString grp = KileTool::groupFor("MakeIndex", m_config);
		if ( grp.isEmpty() ) grp = "Default";
		m_config->setGroup(grp);
		m_config->writeEntry("options", makeIndexOptions() );
	}

	m_config->sync();

	// dump();

	return true;
}

void KileProject::buildProjectTree()
{
	kdDebug() << "==KileProject::buildProjectTree==========================" << endl;

	//determine the parent doc for each item (TODO:an item can only have one parent, not necessarily true for LaTeX docs)

	const QStringList *deps;
	KileProjectItem *itm;
	KURL url;
	QPtrListIterator<KileProjectItem> it(m_projectitems);

	//clean first
	while (it.current())
	{
		(*it)->setParent(0);
		++it;
	}

	//use the dependencies list of the documentinfo object to determine the parent
	it.toFirst();
	while (it.current())
	{
		//set the type correctly (changing m_extensions causes a call to buildProjectTree)
		setType(*it);

		if ( (*it)->getInfo() )
		{
			deps = (*it)->getInfo()->dependencies();
			for (uint i=0; i < deps->count(); ++i)
			{
				url = m_baseurl;
				url.addPath((*deps)[i]);
                url.cleanPath();
				itm = item(url);
				if (itm && (itm->parent() == 0)) itm->setParent(*it);
			}
		}

		++it;
	}

	//make a list of all the root items (items with parent == 0)
	m_rootItems.clear();
	it.toFirst();
	while (it.current())
	{
		if ((*it)->parent() == 0) m_rootItems.append(*it);
		++it;
	}

	emit(projectTreeChanged(this));
}

KileProjectItem* KileProject::item(const KURL & url)
{
	QPtrListIterator<KileProjectItem> it(m_projectitems);
	while (it.current())
	{
		if ((*it)->url() == url)
			return *it;
		++it;
	}

	return 0;
}

void KileProject::add(KileProjectItem* item)
{
	kdDebug() << "KileProject::add projectitem" << item->url().path() << endl;

	setType(item);

	item->changePath(findRelativePath(item->url()));
	connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );

	m_projectitems.append(item);

	// dump();
}

void KileProject::remove(const KileProjectItem* item)
{
	if (m_config->hasGroup("item:"+item->path()))
		m_config->deleteGroup("item:"+item->path());
	else
		kdWarning() << "KileProject::remove() Failed to delete the group corresponding to this item!!!" <<endl;

	kdDebug() << "KileProject::remove" << endl;
	m_projectitems.remove(item);

	// dump();
}

void KileProject::itemRenamed(KileProjectItem *item)
{
	kdDebug() << "==KileProject::itemRenamed==========================" << endl;
	kdDebug() << "\t" << item->url().fileName() << endl;
	m_config->deleteGroup("item:"+item->path());
	//config.sync();

	item->changePath(findRelativePath(item->url()));
}

QString KileProject::findRelativePath(const KURL &url)
{
	QString basepath = m_baseurl.path();
	QString path = url.directory();
	QString filename = url.fileName();

 	kdDebug() <<"===findRelativeURL==================" << endl;
 	kdDebug() << "\tbasepath : " <<  basepath << " path: " << path << endl;

//   if ( basepath == path )
//   {
//     return "./";
//   }

	QStringList basedirs = QStringList::split("/", basepath, false);
	QStringList dirs = QStringList::split("/", path, false);

	uint nDirs = dirs.count();
	//uint nBaseDirs = basedirs.count();

// 	for (uint i=0; i < basedirs.count(); ++i)
// 	{
// 		kdDebug() << "\t\tbasedirs " << i << ": " << basedirs[i] << endl;
// 	}

// 	for (uint i=0; i < dirs.count(); ++i)
// 	{
//  		kdDebug() << "\t\tdirs " << i << ": " << dirs[i] << endl;
// 	}

	while ( dirs.count() > 0 && basedirs.count() > 0 &&  dirs[0] == basedirs[0] )
	{
		dirs.pop_front();
		basedirs.pop_front();
	}

// 	kdDebug() << "\tafter" << endl;
// 	for (uint i=0; i < basedirs.count(); ++i)
// 	{
// 		kdDebug() << "\t\tbasedirs " << i << ": " << basedirs[i] << endl;
// 	}
// 
// 	for (uint i=0; i < dirs.count(); ++i)
// 	{
// 		kdDebug() << "\t\tdirs " << i << ": " << dirs[i] << endl;
// 	}

	if (nDirs != dirs.count() )
	{
		path = dirs.join("/");

		if (basedirs.count() > 0)
		{
			for (uint j=0; j < basedirs.count(); ++j)
			{
				path = "../" + path;
			}
		}

		if ( path.length()>0 && path.right(1) != "/" ) path = path + "/";

		path = path+filename;
	}
	else //assume an absolute path was requested
	{
		path = url.path();
	}

//  	kdDebug() << "\tpath : " << path << endl;

	return path;
}

bool KileProject::contains(const KURL &url)
{
	for (uint i=0; i < m_projectitems.count(); ++i)
	{
		if ( m_projectitems.at(i)->url() == url )
			return true;
	}

	return false;
}

KileProjectItem *KileProject::rootItem(KileProjectItem *item) const
{
	//find the root item (i.e. the eldest parent)
	KileProjectItem *root = item;
	while ( root->parent() != 0)
		root = root->parent();

	//check if this root item is a LaTeX root
	if ( root->getInfo() )
	{
		if (root->getInfo()->isLaTeXRoot())
			return root;
		else
		{
			//if not, see if we can find another root item that is a LaTeX root
			QPtrListIterator<KileProjectItem> it(m_rootItems);
			while ( it.current() )
			{
				if ( it.current()->getInfo() && it.current()->getInfo()->isLaTeXRoot() )
					return it.current();
				++it;
			}
		}

		//no LaTeX root found, return previously found root
		return root;
	}
	
	//root is not a valid item (getInfo() return 0L), return original item	
	return item;
}

void KileProject::dump()
{
	kdDebug() << "KileProject::dump() " << m_name << endl;
	for ( uint i=0; i < m_projectitems.count(); ++i)
		kdDebug() << "\titem " << i << " : "  << m_projectitems.at(i)->path() << endl;
}

#include "kileproject.moc"
