/********************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           (C) 2009-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
*********************************************************************************************/

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

#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include <kdeversion.h>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KRun>
#include <KTextEditor/SessionConfigInterface>
#if KDE_IS_VERSION(4,3,75)
#include <KTextEditor/ParameterizedSessionConfigInterface>
#endif
#include <KUrl>

#include "documentinfo.h"
#include "kiledebug.h"
#include "kiledocmanager.h"
#include "kiletoolmanager.h"
#include "kileinfo.h"
#include "kileextensions.h"

/*
 01/24/06 tbraun
	Added the logic to get versioned kilepr files
	We also warn if the user wants to open a project file with a different kileprversion
*/


/*
 * KileProjectItem
 */
KileProjectItem::KileProjectItem(KileProject *project, const KUrl & url, int type) :
	m_project(project),
	m_url(url),
	m_type(type),
	m_docinfo(NULL),
	m_parent(NULL),
	m_child(NULL),
	m_sibling(NULL),
	m_nLine(0),
	m_order(-1)
{
	m_bOpen = m_archive = true;

	if (project) {
		project->add(this);
	}
}

void KileProjectItem::setOrder(int i)
{
	m_order = i;
}

void KileProjectItem::setParent(KileProjectItem * item)
{
	m_parent = item;

	//update parent info
	if (m_parent) {
		if (m_parent->firstChild()) {
			//get last child
			KileProjectItem *sib = m_parent->firstChild();
			while (sib->sibling()) {
				sib = sib->sibling();
			}

			sib->setSibling(this);
		}
		else {
			m_parent->setChild(this);
		}
	}
	else {
		setChild(0);
		setSibling(0);
	}
}

void KileProjectItem::load()
{
	KConfigGroup configGroup = m_project->configGroupForItem(this);
	setOpenState(configGroup.readEntry("open", true));
	setEncoding(configGroup.readEntry("encoding", QString()));
	setMode(configGroup.readEntry("mode", QString()));
	setHighlight(configGroup.readEntry("highlight", QString()));
	setArchive(configGroup.readEntry("archive", true));
	setLineNumber(configGroup.readEntry("line", 0));
	setColumnNumber(configGroup.readEntry("column", 0));
	setOrder(configGroup.readEntry("order", -1));
}

void KileProjectItem::save()
{
	KConfigGroup configGroup = m_project->configGroupForItem(this);
	configGroup.writeEntry("open", isOpen());
	configGroup.writeEntry("encoding", encoding());
	configGroup.writeEntry("mode", mode());
	configGroup.writeEntry("highlight", highlight());
	configGroup.writeEntry("archive", archive());
	configGroup.writeEntry("line", lineNumber());
	configGroup.writeEntry("column", columnNumber());
	configGroup.writeEntry("order", order());
}

void KileProjectItem::loadDocumentAndViewSettings()
{
	if(!m_docinfo) {
		return;
	}
	KTextEditor::Document *document = m_docinfo->getDocument();
	if(!document) {
		return;
	}
	QList<KTextEditor::View*> viewList = document->views();
	loadDocumentSettings(document);
	int i = 0;
	for(QList<KTextEditor::View*>::iterator it = viewList.begin(); it != viewList.end(); ++it) {
		loadViewSettings(*it, i);
		++i;
	}
}

void KileProjectItem::saveDocumentAndViewSettings()
{
	if(!m_docinfo) {
		return;
	}
	KTextEditor::Document *document = m_docinfo->getDocument();
	if(!document) {
		return;
	}
	QList<KTextEditor::View*> viewList = document->views();
	saveDocumentSettings(document);
	int i = 0;
	for(QList<KTextEditor::View*>::iterator it = viewList.begin(); it != viewList.end(); ++it) {
		saveViewSettings(*it, i);
		++i;
	}
}

void KileProjectItem::loadViewSettings(KTextEditor::View *view, int viewIndex)
{
	KTextEditor::SessionConfigInterface *interface = qobject_cast<KTextEditor::SessionConfigInterface*>(view);
	if(!interface) {
		return;
	}
	interface->readSessionConfig(m_project->configGroupForItemViewSettings(this, viewIndex));
}

void KileProjectItem::saveViewSettings(KTextEditor::View *view, int viewIndex)
{
	KTextEditor::SessionConfigInterface *interface = qobject_cast<KTextEditor::SessionConfigInterface*>(view);
	if(!interface) {
		return;
	}
	KConfigGroup configGroup = m_project->configGroupForItemViewSettings(this, viewIndex);
	interface->writeSessionConfig(configGroup);
}

void KileProjectItem::loadDocumentSettings(KTextEditor::Document *document)
{
#if KDE_IS_VERSION(4,3,75)
	KTextEditor::ParameterizedSessionConfigInterface *interface = qobject_cast<KTextEditor::ParameterizedSessionConfigInterface*>(document);
	if(!interface) {
		return;
	}
	KConfigGroup configGroup = m_project->configGroupForItemDocumentSettings(this);
	if(!configGroup.exists()) {
		return;
	}
	interface->readParameterizedSessionConfig(configGroup, KTextEditor::ParameterizedSessionConfigInterface::SkipUrl);
#endif
}

void KileProjectItem::saveDocumentSettings(KTextEditor::Document *document)
{
#if KDE_IS_VERSION(4,3,75)
	KTextEditor::ParameterizedSessionConfigInterface *interface = qobject_cast<KTextEditor::ParameterizedSessionConfigInterface*>(document);
	if(!interface) {
		return;
	}
	KConfigGroup configGroup = m_project->configGroupForItemDocumentSettings(this);
	interface->writeParameterizedSessionConfig(configGroup, KTextEditor::ParameterizedSessionConfigInterface::SkipUrl);
#endif
}

void KileProjectItem::print(int level)
{
	QString str;
	str.fill('\t', level);
	KILE_DEBUG() << str << "+" << url().fileName();

	if (firstChild()) {
		firstChild()->print(++level);
	}

	if (sibling()) {
		sibling()->print(level);
	}
}

void KileProjectItem::allChildren(QList<KileProjectItem*> *list) const
{
	KileProjectItem *item = firstChild();

// 	KILE_DEBUG() << "\tKileProjectItem::allChildren(" << list->count() << ")";
	while(item != NULL) {
		list->append(item);
// 		KILE_DEBUG() << "\t\tappending " << item->url().fileName();
		item->allChildren(list);
		item = item->sibling();
	}
}

void KileProjectItem::setInfo(KileDocument::TextInfo *docinfo)
{
	m_docinfo = docinfo;
	if(docinfo)
	{
	connect(docinfo,SIGNAL(urlChanged(KileDocument::Info*, const KUrl &)), this, SLOT(slotChangeURL(KileDocument::Info*, const KUrl &)));
	connect(docinfo,SIGNAL(depChanged()), m_project, SLOT(buildProjectTree()));
	}
}

void KileProjectItem::changeURL(const KUrl &url)
{
	// don't allow empty URLs
	if(!url.isEmpty() && m_url != url)
	{
		m_url = url;
		emit(urlChanged(this));
	}
}

void KileProjectItem::slotChangeURL(KileDocument::Info*, const KUrl &url)
{
	changeURL(url);
}

/*
 * KileProject
 */
KileProject::KileProject(const QString& name, const KUrl& url, KileDocument::Extensions *extensions)
: QObject(NULL), m_invalid(false), m_masterDocument(QString()), m_useMakeIndexOptions(false)
{
	setObjectName(name);
	init(name, url, extensions);
}

KileProject::KileProject(const KUrl& url, KileDocument::Extensions *extensions)
: QObject(NULL), m_invalid(false), m_masterDocument(QString()), m_useMakeIndexOptions(false)
{
	setObjectName(url.fileName());
	init(url.fileName(), url, extensions);
}

KileProject::~KileProject()
{
	KILE_DEBUG() << "DELETING KILEPROJECT " <<  m_projecturl.url();
	emit(aboutToBeDestroyed(this));
	delete m_config;

	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		delete *it;
	}
}

void KileProject::init(const QString& name, const KUrl& url, KileDocument::Extensions *extensions)
{
	m_name = name;
	m_projecturl = KileDocument::Manager::symlinkFreeURL( url);;

	m_config = new KConfig(m_projecturl.toLocalFile(), KConfig::SimpleConfig);
	m_extmanager = extensions;

	m_baseurl = m_projecturl.directory();
	m_baseurl.cleanPath(KUrl::SimplifyDirSeparators);

	KILE_DEBUG() << "KileProject m_baseurl = " << m_baseurl.toLocalFile();

	if(QFileInfo(url.toLocalFile()).exists()) {
		load();
	}
	else {
		//create the project file
		KConfigGroup configGroup = m_config->group("General");
		configGroup.writeEntry("name", m_name);
		configGroup.writeEntry("kileprversion", kilePrVersion);
		configGroup.writeEntry("kileversion", kileFullVersion);
		configGroup.sync();
	}
}

void KileProject::setLastDocument(const KUrl &url)
{
    if ( item(url) != 0 )
        m_lastDocument = KileDocument::Manager::symlinkFreeURL(url);
}

void KileProject::setExtensions(KileProjectItem::Type type, const QString & ext)
{
	if (type<KileProjectItem::Source || type>KileProjectItem::Image)
	{
		kWarning() << "ERROR: TYPE<1 or TYPE>3";
		return;
	}

	// first we take all standard extensions
	QStringList standardExtList;
	if(type == KileProjectItem::Source) {
		standardExtList = (m_extmanager->latexDocuments()).split(' ');
	}
	else if(type == KileProjectItem::Package) {
		standardExtList = (m_extmanager->latexPackages()).split(' ');
	}
	else { // if ( type == KileProjectItem::Image )
		standardExtList = (m_extmanager->images()).split(' ');
	}

	// now we scan user-defined list and accept all extension,
	// except standard extensions of course
	QString userExt;
	if(!ext.isEmpty()) {
		QStringList userExtList;

		QStringList::ConstIterator it;
		QStringList list = ext.split(' ');
		for(it = list.constBegin(); it != list.constEnd(); ++it) {
			// some tiny extension checks
			if((*it).length() < 2 || (*it)[0] != '.') {
				continue;
			}

			// some of the old definitions are wrong, so we test them all
			if(type == KileProjectItem::Source || type == KileProjectItem::Package) {
				if(!(m_extmanager->isLatexDocument(*it) || m_extmanager->isLatexPackage(*it))) {
					standardExtList << (*it);
					userExtList << (*it);
				}
			}
			else { // if ( type == KileProjectItem::Image )
				if(!m_extmanager->isImage(*it)) {
					standardExtList << (*it);
					userExtList << (*it);
				}
			}
		}
		if(userExtList.count() > 0) {
			userExt = userExtList.join(" ");
		}
	}

	// now we build a regular expression for all extensions
	// (used to search for a filename with a valid extension)
	QString pattern = standardExtList.join("|");
	pattern.replace('.', "\\.");
	pattern = '('+ pattern +")$";

	// and save it
	m_reExtensions[type-1].setPattern(pattern);

	// if the list of user-defined extensions has changed
	// we save the new value and (re)build the project tree
	if (m_extensions[type-1] != userExt) {
		m_extensions[type-1] = userExt;
		buildProjectTree();
	}
}

void KileProject::setDefaultGraphicExt(const QString & ext){
	m_defGraphicExt = ext;
}

const QString & KileProject::defaultGraphicExt(){
	return m_defGraphicExt;
}

void KileProject::setType(KileProjectItem *item)
{
	if(item->path().right(7) == ".kilepr") {
		item->setType(KileProjectItem::ProjectFile);
		return;
	}

	bool unknown = true;
	for(int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
		if(m_reExtensions[i-1].indexIn(item->url().fileName()) != -1) {
			item->setType(i);
			unknown = false;
			break;
		}
	}

	if(unknown) {
		item->setType(KileProjectItem::Other);
	}
}

void KileProject::readMakeIndexOptions()
{
	QString grp = KileTool::groupFor("MakeIndex", m_config);

	//get the default value
	KSharedConfig::Ptr cfg = KGlobal::config();
	KConfigGroup configGroup = cfg->group(KileTool::groupFor("MakeIndex", KileTool::configName("MakeIndex", cfg.data())));
	QString deflt = configGroup.readEntry("options", "'%S'.idx");

	if (useMakeIndexOptions() && !grp.isEmpty()) {
		KConfigGroup makeIndexGroup = m_config->group(grp);
		QString val = makeIndexGroup.readEntry("options", deflt);
		if ( val.isEmpty() ) val = deflt;
		setMakeIndexOptions(val);
	}
	else { //use default value
		setMakeIndexOptions(deflt);
	}
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
	KILE_DEBUG() << "===addBaseURL(const QString & " << path << " )";
	if(path.isEmpty()) {
		return path;
	}

	else if(QDir::isAbsolutePath(path)) {
		return KileDocument::Manager::symlinkFreeURL(KUrl(path)).toLocalFile();
	}
	else {
		return  KileDocument::Manager::symlinkFreeURL(KUrl(m_baseurl.toLocalFile(KUrl::AddTrailingSlash) +path)).toLocalFile();
	}
}

QString KileProject::removeBaseURL(const QString &path)
{
	if(QDir::isAbsolutePath(path)) {
		QFileInfo info(path);
		QString relPath = findRelativePath(path);
		KILE_DEBUG() << "removeBaseURL path is" << path << " , relPath is " << relPath;
		return relPath;
	}
	else {
		return path;
	}
}

bool KileProject::load()
{
	KILE_DEBUG() << "KileProject: loading..." <<endl;

	//load general settings/options
	KConfigGroup generalGroup = m_config->group("General");
	m_name = generalGroup.readEntry("name", i18n("Project"));
	m_kileversion = generalGroup.readEntry("kileversion", QString());
	m_kileprversion = generalGroup.readEntry("kileprversion",QString());

	m_defGraphicExt = generalGroup.readEntry("def_graphic_ext", QString());

	if(!m_kileprversion.isNull() && m_kileprversion.toInt() > kilePrVersion.toInt()) {
		if(KMessageBox::warningYesNo(NULL, i18n("The project file of %1 was created by a newer version of kile. "
				"Opening it can lead to unexpected results.\n"
				"Do you really want to continue (not recommended)?", m_name),
				 QString(), KStandardGuiItem::yes(), KStandardGuiItem::no(), QString(), KMessageBox::Dangerous) == KMessageBox::No) {
			m_invalid=true;
			return false;
		}
	}

	QString master = addBaseURL(generalGroup.readEntry("masterDocument", QString()));
	KILE_DEBUG() << "masterDoc == " << master;
	setMasterDocument(master);

	setExtensions(KileProjectItem::Source, generalGroup.readEntry("src_extensions",m_extmanager->latexDocuments()));
	setExtensions(KileProjectItem::Package, generalGroup.readEntry("pkg_extensions",m_extmanager->latexPackages()));
	setExtensions(KileProjectItem::Image, generalGroup.readEntry("img_extensions",m_extmanager->images()));

	setQuickBuildConfig(KileTool::configName("QuickBuild", m_config));

	if( KileTool::configName("MakeIndex",m_config).compare("Default") == 0) {
		setUseMakeIndexOptions(true);
	}
	else {
		setUseMakeIndexOptions(false);
	}

	readMakeIndexOptions();

	KUrl url;
	KileProjectItem *item;
	QStringList groups = m_config->groupList();

	//retrieve all the project files and create and initialize project items for them
	for (int i = 0; i < groups.count(); ++i) {
		if (groups[i].left(5) == "item:") {
			QString path = groups[i].mid(5);
			if (QDir::isAbsolutePath(path)) {
				url = KUrl(path);
			}
			else {
				url = m_baseurl;
				url.addPath(path);
				url.cleanPath(KUrl::SimplifyDirSeparators);
			}
			item = new KileProjectItem(this, KileDocument::Manager::symlinkFreeURL(url));
			setType(item);

			KConfigGroup configGroup = m_config->group(groups[i]);
			// path has to be set before we can load it
			item->changePath(groups[i].mid(5));
			item->load();
			connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );
		}
	}

	// only call this after all items are created, otherwise setLastDocument doesn't accept the url
	generalGroup = m_config->group("General");
	setLastDocument(KUrl(addBaseURL(generalGroup.readEntry("lastDocument", QString()))));

	setLivePreviewTool(generalGroup.readEntry("livePreviewToolName", LIVEPREVIEW_DEFAULT_TOOL_NAME),
	                   generalGroup.readEntry("livePreviewToolConfigName", LIVEPREVIEW_DEFAULT_TOOL_CONFIG_NAME));
	if(generalGroup.readEntry("livePreviewStatusUserSet", false)) {
		setLivePreviewEnabled(generalGroup.readEntry("livePreviewEnabled", true));
	}
// 	dump();

	return true;
}

bool KileProject::save()
{
	KILE_DEBUG() << "KileProject: saving..." <<endl;

	KConfigGroup generalGroup = m_config->group("General");
	generalGroup.writeEntry("name", m_name);
	generalGroup.writeEntry("kileprversion", kilePrVersion);
	generalGroup.writeEntry("kileversion", kileFullVersion);
	generalGroup.writeEntry("def_graphic_ext", m_defGraphicExt);

	KILE_DEBUG() << "KileProject::save() masterDoc = " << removeBaseURL(m_masterDocument);
	generalGroup.writeEntry("masterDocument", removeBaseURL(m_masterDocument));
	generalGroup.writeEntry("lastDocument", removeBaseURL(m_lastDocument.toLocalFile()));
	generalGroup.writeEntry("livePreviewToolName", livePreviewToolName());
	generalGroup.writeEntry("livePreviewToolConfigName", livePreviewToolConfigName());

	if(userSpecifiedLivePreviewStatus()) {
		generalGroup.writeEntry("livePreviewStatusUserSet", true);
		generalGroup.writeEntry("livePreviewEnabled", isLivePreviewEnabled());
	}

	writeConfigEntry("src_extensions",m_extmanager->latexDocuments(),KileProjectItem::Source);
	writeConfigEntry("pkg_extensions",m_extmanager->latexPackages(),KileProjectItem::Package);
	writeConfigEntry("img_extensions",m_extmanager->images(),KileProjectItem::Image);
	// only to avoid problems with older versions
	generalGroup.writeEntry("src_extIsRegExp", false);
	generalGroup.writeEntry("pkg_extIsRegExp", false);
	generalGroup.writeEntry("img_extIsRegExp", false);

	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		(*it)->save();
	}

	KileTool::setConfigName("QuickBuild", quickBuildConfig(), m_config);

	writeUseMakeIndexOptions();
	if(useMakeIndexOptions()) {
		QString grp = KileTool::groupFor("MakeIndex", m_config);
		if(grp.isEmpty()) {
			grp = "Default";
		}
		KConfigGroup configGroup = m_config->group(grp);
		configGroup.writeEntry("options", makeIndexOptions());
	}

	m_config->sync();

	// dump();

	return true;
}

void KileProject::writeConfigEntry(const QString &key, const QString &standardExt, KileProjectItem::Type type)
{
	KConfigGroup generalGroup = m_config->group("General");
	QString userExt = extensions(type);
	if(userExt.isEmpty()) {
		generalGroup.writeEntry(key, standardExt);
	}
	else {
		generalGroup.writeEntry(key, standardExt + ' ' + extensions(type));
	}
}

KConfigGroup KileProject::configGroupForItem(KileProjectItem *item) const
{
	return m_config->group("item:" + item->path());
}

KConfigGroup KileProject::configGroupForItemDocumentSettings(KileProjectItem *item) const
{
	return m_config->group("document-settings,item:" + item->path());
}

KConfigGroup KileProject::configGroupForItemViewSettings(KileProjectItem *item, int viewIndex) const
{
	return m_config->group("view-settings,view=" + QString::number(viewIndex) + ",item:" + item->path());
}

void KileProject::removeConfigGroupsForItem(KileProjectItem *item)
{
	QString itemString = "item:" + item->path();
	QStringList groupList = m_config->groupList();
	for(QStringList::iterator i = groupList.begin(); i != groupList.end(); ++i) {
		QString groupName = *i;
		if(groupName.indexOf(itemString) >= 0) {
			m_config->deleteGroup(groupName);
		}
	}
}

static bool isAncestorOf(KileProjectItem *toBeChecked, KileProjectItem *parent)
{
	KileProjectItem *item = parent;
	while(item != NULL) {
		if(item == toBeChecked) {
			return true;
		}
		item = item->parent();
	}
	return false;
}

void KileProject::buildProjectTree()
{
	KILE_DEBUG() << "==KileProject::buildProjectTree==========================";

	//determine the parent doc for each item (TODO:an item can only have one parent, not necessarily true for LaTeX docs)

	QStringList deps;
	QString dep;
	KileProjectItem *itm;
	KUrl url;

	//clean first
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		(*it)->setParent(0);
	}

	//use the dependencies list of the documentinfo object to determine the parent
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		//set the type correctly (changing m_extensions causes a call to buildProjectTree)
		setType(*it);
		KileDocument::Info *docinfo = (*it)->getInfo();

		if(docinfo) {
			KUrl parentUrl = docinfo->url();
			if(parentUrl.isLocalFile()) {
				// strip the file name from 'parentUrl'
				parentUrl = KUrl::fromPathOrUrl(QFileInfo(parentUrl.path()).path());
			}
			else {
				parentUrl = m_baseurl;
			}
			deps = docinfo->dependencies();
			for(int i = 0; i < deps.count(); ++i) {
				dep = deps[i];

				if(m_extmanager->isTexFile(dep)) {
					url = KileInfo::checkOtherPaths(parentUrl, dep, KileInfo::texinputs);
				}
				else if(m_extmanager->isBibFile(dep)) {
					url = KileInfo::checkOtherPaths(parentUrl, dep, KileInfo::bibinputs);
				}
				itm = item(url);
				if(itm && (itm->parent() == 0)
				       && !isAncestorOf(itm, *it)) { // avoid circular references if a file should
				                                     // include itself in a circular way
					itm->setParent(*it);
				}
			}
		}
	}

	//make a list of all the root items (items with parent == 0)
	m_rootItems.clear();
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		if((*it)->parent() == NULL) {
			m_rootItems.append(*it);
		}
	}

	emit(projectTreeChanged(this));
}

KileProjectItem* KileProject::item(const KUrl& url)
{
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		if((*it)->url() == url) {
			return *it;
		}
	}

	return NULL;
}

KileProjectItem* KileProject::item(const KileDocument::Info *info)
{
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		KileProjectItem *current = *it;

		if (current->getInfo() == info) {
			return current;
		}
	}

	return NULL;
}

void KileProject::add(KileProjectItem* item)
{
	KILE_DEBUG() << "KileProject::add projectitem" << item->url().toLocalFile();

	setType(item);

	item->changePath(findRelativePath(item->url()));
	connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );

	m_projectItems.append(item);

	emit projectItemAdded(this, item);

	// dump();
}

void KileProject::remove(KileProjectItem* item)
{
	KILE_DEBUG() << item->path();
	removeConfigGroupsForItem(item);
	m_projectItems.removeAll(item);

	emit projectItemRemoved(this, item);

	// dump();
}

void KileProject::itemRenamed(KileProjectItem *item)
{
	KILE_DEBUG() << "==KileProject::itemRenamed==========================";
	KILE_DEBUG() << "\t" << item->url().fileName();
	removeConfigGroupsForItem(item);

	item->changePath(findRelativePath(item->url()));
}

QString KileProject::findRelativePath(const QString &path)
{
	return this->findRelativePath(KUrl(path));
}

QString KileProject::findRelativePath(const KUrl &url)
{
	KILE_DEBUG() << "QString KileProject::findRelativePath(const KUrl " << url.path() << ")";

	if ( m_baseurl.toLocalFile() == url.toLocalFile() ) {
		return "./";
	}

	m_baseurl.adjustPath(KUrl::AddTrailingSlash);
	QString path = KUrl::relativeUrl(m_baseurl,url);
	KILE_DEBUG() << "relPath is " << path;
	return path;
}

bool KileProject::contains(const KUrl &url)
{
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		if((*it)->url() == url) {
			return true;
		}
	}

	return false;
}

bool KileProject::contains(const KileDocument::Info *info)
{
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		if((*it)->getInfo() == info) {
			return true;
		}
	}
	return false;
}

KileProjectItem *KileProject::rootItem(KileProjectItem *item) const
{
	//find the root item (i.e. the eldest parent)
	KileProjectItem *root = item;
	while(root->parent() != NULL) {
		root = root->parent();
	}

	//check if this root item is a LaTeX root
	if(root->getInfo()) {
		if (root->getInfo()->isLaTeXRoot()) {
			return root;
		}
		else {
			//if not, see if we can find another root item that is a LaTeX root
			for(QList<KileProjectItem*>::const_iterator it = m_rootItems.begin(); it != m_rootItems.end(); ++it) {
				KileProjectItem *current = *it;
				if(current->getInfo() && current->getInfo()->isLaTeXRoot()) {
					return current;
				}
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
	KILE_DEBUG() << "KileProject::dump() " << m_name;
	for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		KileProjectItem *item = *it;
		KILE_DEBUG() << "item " << item << " has path: "  << item->path();
		KILE_DEBUG() << "item->type() " << item->type();
		KILE_DEBUG() << "OpenState: " << item->isOpen();
	}
}

QString KileProject::archiveFileList() const
{
	KILE_DEBUG() << "KileProject::archiveFileList()";

	QString path, list;
	for(QList<KileProjectItem*>::const_iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
		if ((*it)->archive()) {
			path = (*it)->path();
			KRun::shellQuote(path);
			list.append(path + ' ');
		}
	}
	return list;
}

void KileProject::setMasterDocument(const QString & master){

	if(!master.isEmpty()){

		QFileInfo fi(master);
		if(fi.exists())
			m_masterDocument = master;
		else {
			m_masterDocument.clear();
			KILE_DEBUG() << "setMasterDocument: masterDoc=NULL";
		}

	}
	else {
		m_masterDocument.clear();
	}

	emit (masterDocumentChanged(m_masterDocument));
}

#include "kileproject.moc"
