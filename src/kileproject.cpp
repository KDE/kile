/********************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           (C) 2009-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>
#include <QUrl>

#include "documentinfo.h"
#include "kiledebug.h"
#include "kiledocmanager.h"
#include "kiletoolmanager.h"
#include "kileinfo.h"
#include "kileextensions.h"
#include "livepreview.h"
#include "utilities.h"

/**
 * Since project file version 3, project files 'consist' of two files: one file named '<name>.kilepr' and
 * one file named '<name>.kilepr.gui' located in the '.kile' subdirectory of the project directory.
 * The former files contains the static structure of the project, and the later contains the current gui display settings
 * (like which file is open or on which line and column the cursors are).
 */


/*
 * KileProjectItem
 */
KileProjectItem::KileProjectItem(KileProject *project, const QUrl &url, int type) :
    m_project(project),
    m_url(url),
    m_type(type),
    m_docinfo(Q_NULLPTR),
    m_parent(Q_NULLPTR),
    m_child(Q_NULLPTR),
    m_sibling(Q_NULLPTR),
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
    KConfigGroup projectConfigGroup = m_project->configGroupForItem(this, KileProject::ProjectFile);
    KConfigGroup guiConfigGroup = m_project->configGroupForItem(this, KileProject::GUIFile);
    // project: archive, highlight, mode
    // gui: column, encoding, line, open, order
    setEncoding(projectConfigGroup.readEntry("encoding", QString()));
    setMode(projectConfigGroup.readEntry("mode", QString()));
    setHighlight(projectConfigGroup.readEntry("highlight", QString()));
    setArchive(projectConfigGroup.readEntry("archive", true));
    setOpenState(guiConfigGroup.readEntry("open", true));
    setOrder(guiConfigGroup.readEntry("order", -1));
}

void KileProjectItem::save()
{
    KConfigGroup projectConfigGroup = m_project->configGroupForItem(this, KileProject::ProjectFile);
    KConfigGroup guiConfigGroup = m_project->configGroupForItem(this, KileProject::GUIFile);
    // project: archive, highlight, mode
    // gui: encoding, open, order
    projectConfigGroup.writeEntry("encoding", encoding());
    projectConfigGroup.writeEntry("mode", mode());
    projectConfigGroup.writeEntry("highlight", highlight());
    projectConfigGroup.writeEntry("archive", archive());
    guiConfigGroup.writeEntry("open", isOpen());
    guiConfigGroup.writeEntry("order", order());
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
    KConfigGroup configGroup = m_project->configGroupForItemViewSettings(this, viewIndex);
    view->readSessionConfig(configGroup);
}

void KileProjectItem::saveViewSettings(KTextEditor::View *view, int viewIndex)
{
    KConfigGroup configGroup = m_project->configGroupForItemViewSettings(this, viewIndex);
    view->writeSessionConfig(configGroup);
}

void KileProjectItem::loadDocumentSettings(KTextEditor::Document *document)
{
    KConfigGroup configGroup = m_project->configGroupForItemDocumentSettings(this);
    if(!configGroup.exists()) {
        return;
    }
    document->readSessionConfig(configGroup, QSet<QString>() << "SkipUrl");
}

void KileProjectItem::saveDocumentSettings(KTextEditor::Document *document)
{
    KConfigGroup configGroup = m_project->configGroupForItemDocumentSettings(this);
    document->writeSessionConfig(configGroup, QSet<QString>() << "SkipUrl");
}

void KileProjectItem::print(int level)
{
    QString str;
    str.fill('\t', level);
    KILE_DEBUG_MAIN << str << "+" << url().fileName();

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

// 	KILE_DEBUG_MAIN << "\tKileProjectItem::allChildren(" << list->count() << ")";
    while(item != Q_NULLPTR) {
        list->append(item);
// 		KILE_DEBUG_MAIN << "\t\tappending " << item->url().fileName();
        item->allChildren(list);
        item = item->sibling();
    }
}

void KileProjectItem::setInfo(KileDocument::TextInfo *docinfo)
{
    m_docinfo = docinfo;
    if(docinfo)
    {
        connect(docinfo,SIGNAL(urlChanged(KileDocument::Info*, const QUrl &)), this, SLOT(slotChangeURL(KileDocument::Info*, const QUrl &)));
        connect(docinfo,SIGNAL(depChanged()), m_project, SLOT(buildProjectTree()));
    }
}

void KileProjectItem::changeURL(const QUrl &url)
{
    // don't allow empty URLs
    if(!url.isEmpty() && m_url != url)
    {
        m_url = url;
        emit(urlChanged(this));
    }
}

void KileProjectItem::slotChangeURL(KileDocument::Info*, const QUrl &url)
{
    changeURL(url);
}

/*
 * KileProject
 */

// for creating an empty project
KileProject::KileProject(const QString& name, const QUrl &url, KileDocument::Extensions *extensions)
    : QObject(Q_NULLPTR), m_invalid(false), m_masterDocument(QString()), m_useMakeIndexOptions(false),
      m_config(Q_NULLPTR), m_guiConfig(Q_NULLPTR), m_extmanager(extensions)
{
    m_name = name;
    init(url);

    //create the project file
    KConfigGroup configGroup = m_config->group("General");
    configGroup.writeEntry("name", m_name);
    configGroup.writeEntry("kileprversion", KILE_PROJECTFILE_VERSION);
    configGroup.writeEntry("kileversion", kileFullVersion);

    load();
}

// for opening an existing project, 'load()' still has to be called separately!
KileProject::KileProject(const QUrl &url, KileDocument::Extensions *extensions)
    : QObject(Q_NULLPTR), m_invalid(false), m_masterDocument(QString()), m_useMakeIndexOptions(false),
      m_config(Q_NULLPTR), m_guiConfig(Q_NULLPTR), m_extmanager(extensions)
{
    init(url);
}

KileProject::~KileProject()
{
    KILE_DEBUG_MAIN << "DELETING KILEPROJECT " <<  m_projecturl.url();
    emit(aboutToBeDestroyed(this));
    delete m_guiConfig;
    delete m_config;

    for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
        delete *it;
    }
}

void KileProject::init(const QUrl &url)
{
    m_projecturl = KileUtilities::canonicalUrl(url);

    m_baseurl = m_projecturl.adjusted(QUrl::RemoveFilename);

    KILE_DEBUG_MAIN << "KileProject m_baseurl = " << m_baseurl.toLocalFile();

    m_config = new KConfig(m_projecturl.toLocalFile(), KConfig::SimpleConfig);
}

void KileProject::setLastDocument(const QUrl &url)
{
    if (item(url) != 0) {
        m_lastDocument = KileUtilities::canonicalUrl(url);
    }
}

void KileProject::setExtensions(KileProjectItem::Type type, const QString & ext)
{
    if (type == KileProjectItem::ProjectFile || type >= KileProjectItem::Other)
    {
        qWarning() << "ERROR: invalid project item type:" << type;
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
    else if(type == KileProjectItem::Image) {
        standardExtList = (m_extmanager->images()).split(' ');
    }
    else if(type == KileProjectItem::Bibliography) {
        standardExtList = (m_extmanager->bibtex()).split(' ');
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
            else if(type == KileProjectItem::Image) {
                if(!m_extmanager->isImage(*it)) {
                    standardExtList << (*it);
                    userExtList << (*it);
                }
            }
            else if(type == KileProjectItem::Bibliography) {
                if(!m_extmanager->isBibFile(*it)) {
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

void KileProject::setDefaultGraphicExt(const QString & ext) {
    m_defGraphicExt = ext;
}

const QString & KileProject::defaultGraphicExt() {
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
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
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
    KILE_DEBUG_MAIN << "===addBaseURL(const QString & " << path << " )";
    if(path.isEmpty()) {
        return path;
    }

    else if(QDir::isAbsolutePath(path)) {
        return KileUtilities::canonicalUrl(QUrl::fromLocalFile(path)).toLocalFile();
    }
    else {
        return  KileUtilities::canonicalUrl(QUrl::fromLocalFile(m_baseurl.adjusted(QUrl::StripTrailingSlash).toLocalFile() + '/' + path)).toLocalFile();
    }
}

QString KileProject::removeBaseURL(const QString &path)
{
    if(QDir::isAbsolutePath(path)) {
        QFileInfo info(path);
        QString relPath = findRelativePath(path);
        KILE_DEBUG_MAIN << "removeBaseURL path is" << path << " , relPath is " << relPath;
        return relPath;
    }
    else {
        return path;
    }
}

bool KileProject::appearsToBeValidProjectFile()
{
    if(!m_config->hasGroup("General")) {
        return false;
    }

    KConfigGroup generalGroup = m_config->group("General");
    return generalGroup.hasKey("name") && generalGroup.hasKey("kileprversion") && generalGroup.hasKey("kileversion");
}

int KileProject::getProjectFileVersion()
{
    KConfigGroup generalGroup = m_config->group("General");

    return generalGroup.readEntry("kileprversion", 0);
}

// WARNING: before calling this method, the project file must be of the current 'kileprversion'!
//          also assumes that 'm_name' has been set correctly already if this is a fresh (empty) project!
bool KileProject::load()
{
    KILE_DEBUG_MAIN << "KileProject: loading..." << endl;

    if(!ensurePrivateKileDirectoryExists(m_projecturl)) {
        return false;
    }

    delete m_guiConfig;
    m_guiConfig = new KConfig(getPathForGUISettingsProjectFile(m_projecturl), KConfig::SimpleConfig);

    //load general settings/options
    KConfigGroup generalGroup = m_config->group("General");
    m_name = generalGroup.readEntry("name", m_name);

    m_defGraphicExt = generalGroup.readEntry("def_graphic_ext", QString());

    QString master = addBaseURL(generalGroup.readEntry("masterDocument", QString()));
    KILE_DEBUG_MAIN << "masterDoc == " << master;
    setMasterDocument(master);

    setExtensions(KileProjectItem::Source, generalGroup.readEntry("src_extensions",m_extmanager->latexDocuments()));
    setExtensions(KileProjectItem::Package, generalGroup.readEntry("pkg_extensions",m_extmanager->latexPackages()));
    setExtensions(KileProjectItem::Image, generalGroup.readEntry("img_extensions",m_extmanager->images()));
    setExtensions(KileProjectItem::Bibliography, generalGroup.readEntry("bib_extensions", m_extmanager->bibtex()));

    setQuickBuildConfig(KileTool::configName("QuickBuild", m_config));

    if( KileTool::configName("MakeIndex",m_config).compare("Default") == 0) {
        setUseMakeIndexOptions(true);
    }
    else {
        setUseMakeIndexOptions(false);
    }

    readMakeIndexOptions();

    QUrl url;
    KileProjectItem *item;
    const QStringList groups = m_config->groupList();

    //retrieve all the project files and create and initialize project items for them
    for (auto group : groups) {
        if(!m_config->hasGroup(group)) { // 'group' might have been deleted
            continue;                // work around bug 384039
        }
        if (group.left(5) == "item:") {
            QString path = group.mid(5);
            if (QDir::isAbsolutePath(path)) {
                url = QUrl::fromLocalFile(path);
            }
            else {
                url = m_baseurl.adjusted(QUrl::StripTrailingSlash);
                url.setPath(url.path() + '/' + path);
            }
            item = new KileProjectItem(this, KileUtilities::canonicalUrl(url));
            setType(item);

            KConfigGroup configGroup = m_config->group(group);
            // path has to be set before we can load it
            item->changePath(group.mid(5));
            item->load();
            connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );
        }
    }

    // only call this after all items are created, otherwise setLastDocument doesn't accept the url
    KConfigGroup guiGeneralGroup = m_guiConfig->group("General");
    setLastDocument(QUrl::fromLocalFile(addBaseURL(guiGeneralGroup.readEntry("lastDocument", QString()))));

    generalGroup = m_config->group("General");

    readBibliographyBackendSettings(generalGroup);

    KileTool::LivePreviewManager::readLivePreviewStatusSettings(guiGeneralGroup, this);

// 	dump();

    return true;
}

bool KileProject::save()
{
    KILE_DEBUG_MAIN << "KileProject: saving..." <<endl;

    KConfigGroup generalGroup = m_config->group("General");
    KConfigGroup guiGeneralGroup = m_guiConfig->group("General");

    generalGroup.writeEntry("name", m_name);
    generalGroup.writeEntry("kileprversion", KILE_PROJECTFILE_VERSION);
    generalGroup.writeEntry("kileversion", kileFullVersion);
    generalGroup.writeEntry("def_graphic_ext", m_defGraphicExt);

    KILE_DEBUG_MAIN << "KileProject::save() masterDoc = " << removeBaseURL(m_masterDocument);
    generalGroup.writeEntry("masterDocument", removeBaseURL(m_masterDocument));
    guiGeneralGroup.writeEntry("lastDocument", removeBaseURL(m_lastDocument.toLocalFile()));

    writeBibliographyBackendSettings(generalGroup);

    KileTool::LivePreviewManager::writeLivePreviewStatusSettings(guiGeneralGroup, this);

    writeConfigEntry("src_extensions",m_extmanager->latexDocuments(),KileProjectItem::Source);
    writeConfigEntry("pkg_extensions",m_extmanager->latexPackages(),KileProjectItem::Package);
    writeConfigEntry("img_extensions",m_extmanager->images(),KileProjectItem::Image);
    writeConfigEntry("bib_extensions", m_extmanager->bibtex(), KileProjectItem::Bibliography);
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
    m_guiConfig->sync();

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

KConfigGroup KileProject::configGroupForItem(KileProjectItem *item, ConfigScope scope) const
{
    KConfig* cfgObject = (scope == GUIFile ? m_guiConfig : m_config);
    return cfgObject->group("item:" + item->path());
}

KConfigGroup KileProject::configGroupForItemDocumentSettings(KileProjectItem *item) const
{
    return m_guiConfig->group("document-settings,item:" + item->path());
}

KConfigGroup KileProject::configGroupForItemViewSettings(KileProjectItem *item, int viewIndex) const
{
    return m_guiConfig->group("view-settings,view=" + QString::number(viewIndex) + ",item:" + item->path());
}

void KileProject::removeConfigGroupsForItem(KileProjectItem *item)
{
    QString itemString = "item:" + item->path();
    const QStringList groupList = m_config->groupList();
    for(auto groupName : groupList) {
        if(!m_config->hasGroup(groupName)) { // 'groupName' might have been deleted
            continue;                    // work around bug 384039
        }
        if(groupName.indexOf(itemString) >= 0) {
            m_config->deleteGroup(groupName);
        }
    }
}

static bool isAncestorOf(KileProjectItem *toBeChecked, KileProjectItem *parent)
{
    KileProjectItem *item = parent;
    while(item != Q_NULLPTR) {
        if(item == toBeChecked) {
            return true;
        }
        item = item->parent();
    }
    return false;
}

void KileProject::buildProjectTree()
{
    KILE_DEBUG_MAIN << "==KileProject::buildProjectTree==========================";

    //determine the parent doc for each item (TODO:an item can only have one parent, not necessarily true for LaTeX docs)

    QStringList deps;
    QString dep;
    KileProjectItem *itm;
    QUrl url;

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
            QUrl parentUrl = docinfo->url();
            if(parentUrl.isLocalFile()) {
                // strip the file name from 'parentUrl'
                parentUrl = QUrl::fromUserInput(QFileInfo(parentUrl.path()).path());
            }
            else {
                parentUrl = m_baseurl;
            }
            deps = docinfo->dependencies();
            for(int i = 0; i < deps.count(); ++i) {
                dep = deps[i];

                if(m_extmanager->isTexFile(dep)) {
                    url = QUrl::fromLocalFile(KileInfo::checkOtherPaths(parentUrl, dep, KileInfo::texinputs));
                }
                else if(m_extmanager->isBibFile(dep)) {
                    url = QUrl::fromLocalFile(KileInfo::checkOtherPaths(parentUrl, dep, KileInfo::bibinputs));
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
        if((*it)->parent() == Q_NULLPTR) {
            m_rootItems.append(*it);
        }
    }

    emit(projectTreeChanged(this));
}

KileProjectItem* KileProject::item(const QUrl &url)
{
    for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
        if((*it)->url() == url) {
            return *it;
        }
    }

    return Q_NULLPTR;
}

KileProjectItem* KileProject::item(const KileDocument::Info *info)
{
    for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
        KileProjectItem *current = *it;

        if (current->getInfo() == info) {
            return current;
        }
    }

    return Q_NULLPTR;
}

void KileProject::add(KileProjectItem* item)
{
    KILE_DEBUG_MAIN << "KileProject::add projectitem" << item->url().toLocalFile();

    setType(item);

    item->changePath(findRelativePath(item->url()));
    connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );

    m_projectItems.append(item);

    emit projectItemAdded(this, item);

    // dump();
}

void KileProject::remove(KileProjectItem* item)
{
    KILE_DEBUG_MAIN << item->path();
    removeConfigGroupsForItem(item);
    m_projectItems.removeAll(item);

    emit projectItemRemoved(this, item);

    // dump();
}

void KileProject::itemRenamed(KileProjectItem *item)
{
    KILE_DEBUG_MAIN << "==KileProject::itemRenamed==========================";
    KILE_DEBUG_MAIN << "\t" << item->url().fileName();
    removeConfigGroupsForItem(item);

    item->changePath(findRelativePath(item->url()));
}

QString KileProject::findRelativePath(const QString &path)
{
    return this->findRelativePath(QUrl::fromLocalFile(path));
}

QString KileProject::findRelativePath(const QUrl &url)
{
    KILE_DEBUG_MAIN << "QString KileProject::findRelativePath(const QUrl " << url.path() << ")";

    if ( m_baseurl.toLocalFile() == url.toLocalFile() ) {
        return "./";
    }
    const QString path = QDir(m_baseurl.path()).relativeFilePath(url.path());
    KILE_DEBUG_MAIN << "relPath is " << path;
    return path;
}

bool KileProject::contains(const QUrl &url)
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
    while(root->parent() != Q_NULLPTR) {
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
    KILE_DEBUG_MAIN << "KileProject::dump() " << m_name;
    for(QList<KileProjectItem*>::iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
        KileProjectItem *item = *it;
        KILE_DEBUG_MAIN << "item " << item << " has path: "  << item->path();
        KILE_DEBUG_MAIN << "item->type() " << item->type();
        KILE_DEBUG_MAIN << "OpenState: " << item->isOpen();
    }
}

QString KileProject::archiveFileList() const
{
    KILE_DEBUG_MAIN << "KileProject::archiveFileList()";

    QString path, list;
    for(QList<KileProjectItem*>::const_iterator it = m_projectItems.begin(); it != m_projectItems.end(); ++it) {
        if ((*it)->archive()) {
            list.append(KShell::quoteArg((*it)->path()) + ' ');
        }
    }
    return list;
}

void KileProject::setMasterDocument(const QString & master) {

    if(!master.isEmpty()) {

        QFileInfo fi(master);
        if(fi.exists())
            m_masterDocument = master;
        else {
            m_masterDocument.clear();
            KILE_DEBUG_MAIN << "setMasterDocument: masterDoc=Q_NULLPTR";
        }

    }
    else {
        m_masterDocument.clear();
    }

    emit (masterDocumentChanged(m_masterDocument));
}

namespace {

void moveConfigGroupKeysAsStrings(KConfig *src, KConfig *dst, const QString& groupName, const QStringList &keysToMove)
{
    KConfigGroup srcGroup(src, groupName);
    KConfigGroup dstGroup(dst, groupName);

    for(const QString& key : keysToMove) {
        if(srcGroup.hasKey(key)) {
            QString value = srcGroup.readEntry(key, QStringLiteral(""));
            dstGroup.writeEntry(key, value);
            srcGroup.deleteEntry(key);
        }
    }
}

void deleteConfigGroupKeys(KConfig *src, const QString& groupName, const QStringList &keysToDelete)
{
    KConfigGroup srcGroup(src, groupName);

    for(const QString& key : keysToDelete) {
        srcGroup.deleteEntry(key);
    }
}
}

bool KileProject::migrateProjectFileToCurrentVersion()
{
    if(getProjectFileVersion() < KILE_PROJECTFILE_VERSION) {
        return migrateProjectFileToVersion3();
    }
    return true;
}

bool KileProject::migrateProjectFileToVersion3()
{
    KILE_DEBUG_MAIN << "Migrating project file" << m_projecturl << "to version 3";

    // (1) Every config group starting with "document-settings," or "view-settings," will be moved to the GUI config file
    // (2) In every group named "item:..." the keys "column" and "line" are deleted
    // (3) In every group named "item:..." the keys "open" and "order" are moved to a new group of the same name
    //     in the GUI project file
    // (4) In the "General" group the keys "lastDocument", "kile_livePreviewEnabled", "kile_livePreviewStatusUserSpecified",
    //     "kile_livePreviewTool" are moved to the "General" group in the GUI project file

    if(!ensurePrivateKileDirectoryExists(m_projecturl)) {
        return false;
    }

    KConfig projectGUIFile(getPathForGUISettingsProjectFile(m_projecturl), KConfig::SimpleConfig);

    QStringList keysToMoveInItemGroups, keysToDeleteInItemGroups, keysToMoveInGeneralGroup;

    keysToMoveInItemGroups
            << QStringLiteral("column")
            << QStringLiteral("line")
            << QStringLiteral("open")
            << QStringLiteral("order");

    keysToDeleteInItemGroups
            << QStringLiteral("column")
            << QStringLiteral("line");

    keysToMoveInGeneralGroup
            << QStringLiteral("lastDocument")
            << QStringLiteral("kile_livePreviewEnabled")
            << QStringLiteral("kile_livePreviewStatusUserSpecified")
            << QStringLiteral("kile_livePreviewTool");

    const QStringList groups = m_config->groupList();
    for(auto groupName : groups) {
        if(!m_config->hasGroup(groupName)) { // 'groupName' might have been deleted
            continue;                    // work around bug 384039
        }

        // these ones we move completely
        if(groupName.startsWith(QLatin1String("document-settings,")) || groupName.startsWith(QLatin1String("view-settings,"))) {
            KConfigGroup oldGroup(m_config, groupName);
            KConfigGroup guiGroup(&projectGUIFile, groupName);
            oldGroup.copyTo(&guiGroup);
            m_config->deleteGroup(groupName);
            continue;
        }

        if(groupName.startsWith(QLatin1String("item:"))) {
            deleteConfigGroupKeys(m_config, groupName, keysToDeleteInItemGroups);
            moveConfigGroupKeysAsStrings(m_config, &projectGUIFile, groupName, keysToMoveInItemGroups);
        }
        else if(groupName == QLatin1String("General")) {
            moveConfigGroupKeysAsStrings(m_config, &projectGUIFile, groupName, keysToMoveInGeneralGroup);
        }
    }

    if(!projectGUIFile.sync()) {
        return false;
    }

    KConfigGroup configGroup = m_config->group("General");
    configGroup.writeEntry("kileprversion", KILE_PROJECTFILE_VERSION);
    configGroup.writeEntry("kileversion", kileFullVersion);

    return m_config->sync();
}
