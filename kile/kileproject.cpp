/***************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                         : (C) 2007  by Holger Danielsson
    email                : Jeroen.Wijnhout@kdemail.net
                           holger.danielsson@versanet.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-03-12 dani
//  - use KileDocument::Extensions
//  - allowed extensions are always defined as list, f.e.: .tex .ltx .latex 

#include "kileproject.h"
#include "kileversion.h"

#include <qstringlist.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include "kiledebug.h"
#include <kmessagebox.h>

#include "kiledocumentinfo.h"
#include "kiledocmanager.h"
#include "kiletoolmanager.h"
#include "kileinfo.h"
#include "kileextensions.h"
#include <krun.h>
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
	m_nLine(0),
	m_order(-1)
{
	m_highlight=m_encoding=QString::null;
	m_bOpen = m_archive = true;

	if (project)
		project->add(this);
}

void KileProjectItem::setOrder(int i)
{
	m_order = i;
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
	KILE_DEBUG() << str << "+" << url().fileName() << endl;

	if (firstChild())
		firstChild()->print(++level);

	if (sibling())
		sibling()->print(level);
}

void KileProjectItem::allChildren(QPtrList<KileProjectItem> *list) const
{
	KileProjectItem *item = firstChild();

// 	KILE_DEBUG() << "\tKileProjectItem::allChildren(" << list->count() << ")" << endl;
	while(item != 0)
	{
		list->append(item);
// 		KILE_DEBUG() << "\t\tappending " << item->url().fileName() << endl;
		item->allChildren(list);
		item = item->sibling();
	}
}

void KileProjectItem::setInfo(KileDocument::TextInfo *docinfo)
{
	m_docinfo = docinfo;
	if(docinfo)
	{
	connect(docinfo,SIGNAL(urlChanged(KileDocument::Info*, const KURL &)), this, SLOT(slotChangeURL(KileDocument::Info*, const KURL &)));
	connect(docinfo,SIGNAL(depChanged()), m_project, SLOT(buildProjectTree()));
	}
}

void KileProjectItem::changeURL(const KURL &url)
{
	// don't allow empty URLs
	if(!url.isEmpty() && m_url != url) 
	{
		m_url = url;
		emit(urlChanged(this));
	}
}

void KileProjectItem::slotChangeURL(KileDocument::Info*, const KURL &url)
{
	changeURL(url);
}

/*
 * KileProject
 */
KileProject::KileProject(const QString& name, const KURL& url, KileDocument::Extensions *extensions) : QObject(0,name.ascii()), m_invalid(false), m_masterDocument(QString::null), m_useMakeIndexOptions(false)
{
	init(name, url, extensions);
}

KileProject::KileProject(const KURL& url, KileDocument::Extensions *extensions) : QObject(0,url.fileName().ascii()), m_invalid(false), m_masterDocument(QString::null), m_useMakeIndexOptions(false)
{
	init(url.fileName(), url, extensions);
}	

KileProject::~KileProject()
{
	KILE_DEBUG() << "DELETING KILEPROJECT " <<  m_projecturl.url() << endl;
	delete m_config;
}

void KileProject::init(const QString& name, const KURL& url, KileDocument::Extensions *extensions)
{
	m_name = name;
	m_projecturl = KileDocument::Manager::symlinkFreeURL( url);;

	m_projectitems.setAutoDelete(true);

	m_config = new KSimpleConfig(m_projecturl.path());
	m_extmanager = extensions;

	m_baseurl = m_projecturl.directory();
	m_baseurl.cleanPath(true);

	KILE_DEBUG() << "KileProject m_baseurl = " << m_baseurl.path() << endl;

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
        m_lastDocument = KileDocument::Manager::symlinkFreeURL(url);
}

void KileProject::setExtensions(KileProjectItem::Type type, const QString & ext)
{
	if (type<KileProjectItem::Source || type>KileProjectItem::Image) 
	{
		kdWarning() << "ERROR: TYPE<1 or TYPE>3" << endl;
		return;
	}

	// first we take all standard extensions
	QStringList standardExtList;
	if ( type == KileProjectItem::Source )
		standardExtList = QStringList::split(" ", m_extmanager->latexDocuments() );
	else if ( type == KileProjectItem::Package )
		standardExtList = QStringList::split(" ", m_extmanager->latexPackages() );
	else // if ( type == KileProjectItem::Image )
		standardExtList = QStringList::split(" ", m_extmanager->images() );

	// now we scan user defined list and accept all extension, 
	// except standard extensions of course
	QString userExt;
	if ( ! ext.isEmpty() )
	{
		QStringList userExtList;

		QStringList::ConstIterator it;
		QStringList list = QStringList::split(" ", ext);
		for ( it=list.begin(); it != list.end(); ++it ) 
		{
			// some tiny extension checks
			if ( (*it).length()<2 || (*it)[0]!='.' )
				continue;

			// some of the old definitions are wrong, so we test them all
			if ( type==KileProjectItem::Source || type==KileProjectItem::Package)
			{
				if ( ! (m_extmanager->isLatexDocument(*it) || m_extmanager->isLatexPackage(*it)) )
				{
					standardExtList << (*it);
					userExtList << (*it);
				}
			}
			else // if ( type == KileProjectItem::Image )
			{
				if ( ! m_extmanager->isImage(*it) )
				{
					standardExtList << (*it);
					userExtList << (*it);
				}
			}
		}
		if ( userExtList.count() > 0 )
			userExt = userExtList.join(" ");	
	}

	// now we build a regular expression for all extensions
	// (used to search for a filename with a valid extension)
	QString pattern = standardExtList.join("|");
	pattern.replace(".","\\.");
	pattern = '('+ pattern +")$";

	// and save it
	m_reExtensions[type-1].setPattern(pattern);

	// if the list of user defined extensions has changed
	// we save the new value and (re)build the project tree
	if (m_extensions[type-1] != userExt)
	{
		m_extensions[type-1] = userExt;
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
	{
		if ( m_reExtensions[i-1].search(item->url().fileName()) != -1)
		{
			item->setType(i);
			unknown = false;
			break;
		}
	}

	if (unknown)
		item->setType(KileProjectItem::Other);
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
	KILE_DEBUG() << "===addBaseURL(const QString & " << path << " )" << endl;
	if ( path.isEmpty())
		return path;
  	else if ( path.startsWith("/") )
  		return KileDocument::Manager::symlinkFreeURL(KURL::fromPathOrURL(path)).path();
  	else
    		return  KileDocument::Manager::symlinkFreeURL(KURL::fromPathOrURL(m_baseurl.path() + '/' +path)).path();
 }

QString KileProject::removeBaseURL(const QString &path)
{

  if ( path.startsWith("/") )
  {
    QFileInfo info(path);
    QString relPath = findRelativePath(path);
    KILE_DEBUG() << "removeBaseURL path is" << path << " , relPath is " << relPath << endl;
    return relPath;
  }
  else
  {
    return path;
  }
}

bool KileProject::load()
{
	KILE_DEBUG() << "KileProject: loading..." <<endl;

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
  	KILE_DEBUG() << "masterDoc == " << master << endl;
	setMasterDocument(master);

	setExtensions(KileProjectItem::Source, m_config->readEntry("src_extensions",m_extmanager->latexDocuments()));
	setExtensions(KileProjectItem::Package, m_config->readEntry("pkg_extensions",m_extmanager->latexPackages()));
	setExtensions(KileProjectItem::Image, m_config->readEntry("img_extensions",m_extmanager->images()));

	setQuickBuildConfig(KileTool::configName("QuickBuild", m_config));

	if( KileTool::configName("MakeIndex",m_config).compare("Default") == 0)
		setUseMakeIndexOptions(true);
	else
		setUseMakeIndexOptions(false);

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
			item = new KileProjectItem(this, KileDocument::Manager::symlinkFreeURL(url));
			setType(item);

			m_config->setGroup(groups[i]);
			item->setOpenState(m_config->readBoolEntry("open", true));
			item->setEncoding(m_config->readEntry("encoding", QString::null));
			item->setHighlight(m_config->readEntry("highlight",QString::null));
			item->setArchive(m_config->readBoolEntry("archive", true));
			item->setLineNumber(m_config->readNumEntry("line", 0));
			item->setColumnNumber(m_config->readNumEntry("column", 0));
			item->setOrder(m_config->readNumEntry("order", -1));
			item->changePath(groups[i].mid(5));

			connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );
		}
	}

    // only call this after all items are created, otherwise setLastDocument doesn't accept the url
    m_config->setGroup("General");
    setLastDocument(KURL::fromPathOrURL(addBaseURL(m_config->readEntry("lastDocument", QString::null))));

// 	dump();

	return true;
}

bool KileProject::save()
{
	KILE_DEBUG() << "KileProject: saving..." <<endl;

	m_config->setGroup("General");
	m_config->writeEntry("name", m_name);
	m_config->writeEntry("kileprversion", kilePrVersion);
	m_config->writeEntry("kileversion", kileVersion);

  	KILE_DEBUG() << "KileProject::save() masterDoc = " << removeBaseURL(m_masterDocument) << endl;
	m_config->writeEntry("masterDocument", removeBaseURL(m_masterDocument));
  	m_config->writeEntry("lastDocument", removeBaseURL(m_lastDocument.path()));


	writeConfigEntry("src_extensions",m_extmanager->latexDocuments(),KileProjectItem::Source);
	writeConfigEntry("pkg_extensions",m_extmanager->latexPackages(),KileProjectItem::Package);
	writeConfigEntry("img_extensions",m_extmanager->images(),KileProjectItem::Image);
	// only to avoid problems with older versions
	m_config->writeEntry("src_extIsRegExp", false);
	m_config->writeEntry("pkg_extIsRegExp", false);
	m_config->writeEntry("img_extIsRegExp", false);

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
		m_config->writeEntry("order", item->order());
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

	KILE_DEBUG() << "Check if the config file is writeable: " << m_config->checkConfigFilesWritable(false) << endl;
	m_config->sync();

	// dump();

	return true;
}

void KileProject::writeConfigEntry(const QString &key, const QString &standardExt, KileProjectItem::Type type)
{
	QString userExt = extensions(type);
	if ( userExt.isEmpty() )
		m_config->writeEntry(key,standardExt);
	else
		m_config->writeEntry(key,standardExt + ' ' + extensions(type));
}

void KileProject::buildProjectTree()
{
	KILE_DEBUG() << "==KileProject::buildProjectTree==========================" << endl;

	//determine the parent doc for each item (TODO:an item can only have one parent, not necessarily true for LaTeX docs)

	const QStringList *deps;
	QString dep;
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
				dep = (*deps)[i];

				if( m_extmanager->isTexFile(dep) )
					url = KileInfo::checkOtherPaths(m_baseurl,dep,KileInfo::texinputs);
				else if( m_extmanager->isBibFile(dep) )
					url = KileInfo::checkOtherPaths(m_baseurl,dep,KileInfo::bibinputs);
				
				itm = item(url);
				if (itm && (itm->parent() == 0))
					itm->setParent(*it);
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

KileProjectItem* KileProject::item(const KileDocument::Info *info)
{
	QPtrListIterator<KileProjectItem> it(m_projectitems);
	KileProjectItem *current;
	while ((current = it.current()) != 0)
	{
		++it;
		if (current->getInfo() == info)
		{
			return current;
		}
	}
	return 0;
}

void KileProject::add(KileProjectItem* item)
{
	KILE_DEBUG() << "KileProject::add projectitem" << item->url().path() << endl;

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

	KILE_DEBUG() << "KileProject::remove" << endl;
	m_projectitems.remove(item);

	// dump();
}

void KileProject::itemRenamed(KileProjectItem *item)
{
	KILE_DEBUG() << "==KileProject::itemRenamed==========================" << endl;
	KILE_DEBUG() << "\t" << item->url().fileName() << endl;
	m_config->deleteGroup("item:"+item->path());
	//config.sync();

	item->changePath(findRelativePath(item->url()));
}

QString KileProject::findRelativePath(const KURL &url)
{
	QString basepath = m_baseurl.path();
	QString path = url.directory();
	QString filename = url.fileName();

 	KILE_DEBUG() <<"===findRelativeURL==================" << endl;
 	KILE_DEBUG() << "\tbasepath : " <<  basepath << " path: " << path << endl;

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
// 		KILE_DEBUG() << "\t\tbasedirs " << i << ": " << basedirs[i] << endl;
// 	}

// 	for (uint i=0; i < dirs.count(); ++i)
// 	{
//  		KILE_DEBUG() << "\t\tdirs " << i << ": " << dirs[i] << endl;
// 	}

	while ( dirs.count() > 0 && basedirs.count() > 0 &&  dirs[0] == basedirs[0] )
	{
		dirs.pop_front();
		basedirs.pop_front();
	}

// 	KILE_DEBUG() << "\tafter" << endl;
// 	for (uint i=0; i < basedirs.count(); ++i)
// 	{
// 		KILE_DEBUG() << "\t\tbasedirs " << i << ": " << basedirs[i] << endl;
// 	}
// 
// 	for (uint i=0; i < dirs.count(); ++i)
// 	{
// 		KILE_DEBUG() << "\t\tdirs " << i << ": " << dirs[i] << endl;
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

		if ( path.length() > 0 && path.right(1) != "/" ) path = path + '/';

		path = path+filename;
	}
	else //assume an absolute path was requested
	{
		path = url.path();
	}

//  	KILE_DEBUG() << "\tpath : " << path << endl;

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

bool KileProject::contains(const KileDocument::Info *info)
{
	QPtrListIterator<KileProjectItem> it(m_projectitems);
	KileProjectItem *current;
	while( (current = it.current()) != 0)
	{
		++it;
		if(current->getInfo() == info)
		{
			return true;
		}
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
	KILE_DEBUG() << "KileProject::dump() " << m_name << endl;
	for ( uint i=0; i < m_projectitems.count(); ++i)
	{
		KileProjectItem *item;
		item = m_projectitems.at(i);
		KILE_DEBUG() << "item " << i << " has path: "  << item->path() << endl;
		KILE_DEBUG() << "item->type() " << item->type() << endl;
		KILE_DEBUG() << "OpenState: " << item->isOpen() << endl;
	}
}

QString KileProject::archiveFileList() const
{
	KILE_DEBUG() << "KileProject::archiveFileList()" << endl;

	QString path,list;
	QPtrListIterator<KileProjectItem> it(m_projectitems);
	
	while (it.current())
	{
		if ((*it)->archive())
		{
			path = (*it)->path();
			KRun::shellQuote(path);
			list.append(path + ' ');
		}
		++it;
	}
	return list;
}

void KileProject::setMasterDocument(const QString & master){
	
	if(!master.isEmpty()){
	
		QFileInfo fi(master);
		if(fi.exists())
			m_masterDocument = master;
		else{
			m_masterDocument = QString::null;
			KILE_DEBUG() << "setMasterDocument: masterDoc=NULL" << endl;	
		}
	
	}
	else
		m_masterDocument = QString::null;
	
	emit (masterDocumentChanged(m_masterDocument));
}

#include "kileproject.moc"
