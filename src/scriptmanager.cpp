/**************************************************************************
*   Copyright (C) 2006-2016 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "scriptmanager.h"

#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QEvent>
#include <QDir>
#include <QDirIterator>
#include <QMap>
#include <QStandardPaths>

#include "kiledebug.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "kileversion.h"
#include "kileviewmanager.h"
#include "editorkeysequencemanager.h"
#include "scripting/script.h"

namespace KileScript {

////////////////////////////// Manager //////////////////////////////

Manager::Manager(KileInfo *kileInfo, KConfig *config, KActionCollection *actionCollection, QObject *parent, const char *name)
    : QObject(parent), m_jScriptDirWatch(Q_NULLPTR), m_kileInfo(kileInfo), m_config(config), m_actionCollection(actionCollection)
{
    setObjectName(name);

    // create a local scripts directory if it doesn't exist yet
    m_localScriptDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/scripts/";
    QDir testDir(m_localScriptDir);
    if (!testDir.exists()) {
        testDir.mkpath(m_localScriptDir);
    }

    m_jScriptDirWatch = new KDirWatch(this);
    m_jScriptDirWatch->setObjectName("KileScript::Manager::ScriptDirWatch");
    connect(m_jScriptDirWatch, SIGNAL(dirty(const QString&)), this, SLOT(scanScriptDirectories()));
    connect(m_jScriptDirWatch, SIGNAL(created(const QString&)), this, SLOT(scanScriptDirectories()));
    connect(m_jScriptDirWatch, SIGNAL(deleted(const QString&)), this, SLOT(scanScriptDirectories()));
    m_jScriptDirWatch->startScan();

    // read plugin code for QScriptEngine
    readEnginePlugin();
    m_scriptActionMap = new QMap<QString,QAction *>;

    // init script objects
    m_kileScriptView = new KileScriptView(this, m_kileInfo->editorExtension());
    m_kileScriptDocument = new KileScriptDocument(this, m_kileInfo, m_kileInfo->editorExtension(), m_scriptActionMap);
    m_kileScriptObject = new KileScriptObject(this, m_kileInfo, m_scriptActionMap);
}

Manager::~Manager()
{
    delete m_jScriptDirWatch;
    delete m_scriptActionMap;

    delete m_kileScriptView;
    delete m_kileScriptDocument;
    delete m_kileScriptObject;

    //still need to delete the scripts
    for(QList<Script*>::iterator it = m_jScriptList.begin(); it != m_jScriptList.end(); ++it) {
        delete *it;
    }
}

void Manager::executeScript(const Script *script)
{
    KILE_DEBUG_MAIN << "execute script: " << script->getName();

    // compatibility check
    QString code = script->getCode();
    QRegExp endOfLineExp("(\r\n)|\n|\r");
    int i = code.indexOf(endOfLineExp);
    QString firstLine = (i >= 0 ? code.left(i) : code);
    QRegExp requiredVersionTagExp("(kile-version:\\s*)(\\d+\\.\\d+(.\\d+)?)");
    if(requiredVersionTagExp.indexIn(firstLine) != -1) {
        QString requiredKileVersion = requiredVersionTagExp.cap(2);
        if(compareVersionStrings(requiredKileVersion, kileFullVersion) > 0) {
            KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("Version %1 of Kile is at least required to execute the script \"%2\". The execution has been aborted.", requiredKileVersion, script->getName()), i18n("Version Error"));
            return;
        }
    }

    // TODO only scripts with a current view can be started at this moment
    KTextEditor::View *view = m_kileInfo->viewManager()->currentTextView();
    if(!view) {
        KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("Cannot start the script: no view available"), i18n("Script Error"));
        return;
    }

    // TODO setup script objects (with existing views at this moment)
    m_kileScriptView->setView(view);
    m_kileScriptDocument->setView(view);
    m_kileScriptObject->setScriptname(script->getName());

    // create environment for QtScript engine
    ScriptEnvironment env(m_kileInfo, m_kileScriptView, m_kileScriptDocument, m_kileScriptObject,m_enginePlugin);

    env.execute(script);
}

void Manager::executeScript(unsigned int id)
{
    QMap<unsigned int, Script*>::iterator i = m_idScriptMap.find(id);
    if(i != m_idScriptMap.end()) {
        executeScript(*i);
    }
}

const Script* Manager::getScript(unsigned int id)
{
    QMap<unsigned int, Script*>::iterator i = m_idScriptMap.find(id);
    return ((i != m_idScriptMap.end()) ? (*i) : Q_NULLPTR);
}

void Manager::scanScriptDirectories()
{
    if(!KileConfig::scriptingEnabled()) {
        return;
    }
    deleteScripts();
    populateDirWatch();

    KConfigGroup configGroup = m_config->group("Scripts");
    QList<unsigned int> idList = configGroup.readEntry("IDs", QList<unsigned int>());
    unsigned int maxID = 0;
    QMap<QString, unsigned int> pathIDMap;
    QMap<unsigned int, bool> takenIDMap;
    for(QList<unsigned int>::iterator i = idList.begin(); i != idList.end(); ++i) {
        QString fileName = configGroup.readPathEntry("Script" + QString::number(*i), QString());
        if(!fileName.isEmpty()) {
            unsigned int id = *i;
            pathIDMap[fileName] = id;
            takenIDMap[id] = true;
            maxID = qMax(maxID, id);
        }
    }

    // scan *.js files
    QSet<QString> scriptFileNamesSet;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "scripts/", QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &dir, dirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.js"), QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            scriptFileNamesSet.insert(it.next());
        }
    }

    Q_FOREACH(const QString &scriptFileName, scriptFileNamesSet) {
        registerScript(scriptFileName, pathIDMap, takenIDMap, maxID);
    }
    //rewrite the IDs that are currently in use
    writeIDs();
    m_actionCollection->readSettings();
    emit scriptsChanged();
}

void Manager::deleteScripts()
{
    QList<Script*> scriptList = m_jScriptList;
    m_jScriptList.clear(); // pretend that there are no scripts left
    QStringList keySequenceList;
    for(QList<Script*>::iterator it = scriptList.begin(); it != scriptList.end(); ++it) {
        keySequenceList.push_back((*it)->getKeySequence());
    }
    m_idScriptMap.clear();
    m_kileInfo->editorKeySequenceManager()->removeKeySequence(keySequenceList);
    for(QList<Script*>::iterator it = scriptList.begin(); it != scriptList.end(); ++it) {
        QAction *action = (*it)->getActionObject();
        if(action) {
            foreach(QWidget *w, action->associatedWidgets()) {
                w->removeAction(action);
            }
            m_actionCollection->takeAction(action);
            delete action;
        }
        delete *it;
    }
    emit scriptsChanged();
}

QList<Script*> Manager::getScripts()
{
    return m_jScriptList;
}

void Manager::registerScript(const QString& fileName, QMap<QString, unsigned int>& pathIDMap, QMap<unsigned int, bool>& takenIDMap, unsigned int &maxID)
{
    unsigned int id;
    QMap<QString, unsigned int>::iterator it = pathIDMap.find(fileName);
    if(it != pathIDMap.end()) {
        id = *it;
    }
    else {
        id = findFreeID(takenIDMap, maxID);
        pathIDMap[fileName] = id;
        takenIDMap[id] = true;
        maxID = qMax(maxID, id);
    }
    Script* script = new Script(id, fileName);
    m_jScriptList.push_back(script);
    m_idScriptMap[id] = script;

    // start with setting up the key sequence
    KConfigGroup configGroup = m_config->group("Scripts");

    int sequenceType = 0;
    QString editorKeySequence = QString();
    QString seq = configGroup.readEntry("Script" + QString::number(id) + "KeySequence");
    if ( !seq.isEmpty() ) {
        QRegExp re("(\\d+)-(.*)");
        if ( re.exactMatch(seq) )  {
            sequenceType = re.cap(1).toInt();
            if ( sequenceType<Script::KEY_SEQUENCE || sequenceType>Script::KEY_SHORTCUT ) {
                sequenceType = Script::KEY_SEQUENCE;
            }
            editorKeySequence = re.cap(2);
        }
        else {
            sequenceType = Script::KEY_SEQUENCE;
            editorKeySequence = re.cap(1);
        }
    }
    KILE_DEBUG_MAIN << "script type=" << sequenceType << " seq=" << editorKeySequence;

    // now set up a regular action object
    ScriptExecutionAction *action = new ScriptExecutionAction(id, this, m_actionCollection);

    // action with shortcut?
    if(!editorKeySequence.isEmpty()) {
        script->setSequenceType(sequenceType);
        script->setKeySequence(editorKeySequence);
        if ( sequenceType == Script::KEY_SEQUENCE )  {
            m_kileInfo->editorKeySequenceManager()->addAction(editorKeySequence, new KileEditorKeySequence::ExecuteScriptAction(script, this));
        }
        else {
            action->setShortcut(editorKeySequence);
        }
    }

    // add to action collection
    m_actionCollection->addAction("script" + QString::number(id) + "_execution", action);
    script->setActionObject(action);
}

void Manager::writeConfig()
{
    // don't delete the key sequence settings if scripting has been disabled
    if(!KileConfig::scriptingEnabled()) {
        return;
    }
    m_config->deleteGroup("Scripts");
    writeIDs();

    // write the key sequences
    KConfigGroup configGroup = m_config->group("Scripts");
    for(QList<Script*>::iterator i = m_jScriptList.begin(); i != m_jScriptList.end(); ++i) {
        QString seq = (*i)->getKeySequence();
        QString sequenceEntry = ( seq.isEmpty() ) ? seq : QString("%1-%2").arg(QString::number((*i)->getSequenceType())).arg(seq);
        configGroup.writeEntry("Script" + QString::number((*i)->getID()) + "KeySequence", sequenceEntry);
    }
}

void Manager::setEditorKeySequence(Script* script, int type, const QString& keySequence)
{
    if(keySequence.isEmpty()) {
        return;
    }
    if(script) {
        int oldType = script->getSequenceType();
        QString oldSequence = script->getKeySequence();
        if( oldType==type && oldSequence==keySequence) {
            return;
        }

        if ( oldType == KileScript::Script::KEY_SEQUENCE ) {
            m_kileInfo->editorKeySequenceManager()->removeKeySequence(oldSequence);
        }
        else {
            script->getActionObject()->setShortcut(QString());
        }
        script->setSequenceType(type);
        script->setKeySequence(keySequence);
        if ( type == KileScript::Script::KEY_SEQUENCE ) {
            m_kileInfo->editorKeySequenceManager()->addAction(keySequence, new KileEditorKeySequence::ExecuteScriptAction(script, this));
        }
        else {
            script->getActionObject()->setShortcut(keySequence);
        }

        writeConfig();
    }
}

void Manager::removeEditorKeySequence(Script* script)
{
    if(script) {
        QString keySequence = script->getKeySequence();
        if(keySequence.isEmpty()) {
            return;
        }
        script->setKeySequence(QString());

        int sequenceType = script->getSequenceType();
        if ( sequenceType == Script::KEY_SEQUENCE ) {
            m_kileInfo->editorKeySequenceManager()->removeKeySequence(keySequence);
        }
        else {
            script->getActionObject()->setShortcut(QString());
        }

        writeConfig();
    }
}

void Manager::populateDirWatch()
{
    QStringList jScriptDirectories = QStandardPaths::locateAll(QStandardPaths::DataLocation, "scripts/", QStandardPaths::LocateDirectory);
    for(QStringList::iterator i = jScriptDirectories.begin(); i != jScriptDirectories.end(); ++i) {
        // FIXME: future KDE versions could support the recursive
        //        watching of directories out of the box.
        addDirectoryToDirWatch(*i);
    }
    //we do not remove the directories that were once added as this apparently causes some strange
    //bugs (on KDE 3.5.x)
}

QString Manager::getLocalScriptDirectory() const
{
    return m_localScriptDir;
}

void Manager::readConfig() {
    deleteScripts();
    scanScriptDirectories();
}

unsigned int Manager::findFreeID(const QMap<unsigned int, bool>& takenIDMap, unsigned int maxID)
{
    if(takenIDMap.size() == 0) {
        return 0;
    }
    // maxID should have a real meaning now
    for(unsigned int i = 0; i < maxID; ++i) {
        if(takenIDMap.find(i) == takenIDMap.end()) {
            return i;
        }
    }
    return (maxID + 1);
}

void Manager::writeIDs()
{
    KConfigGroup configGroup = m_config->group("Scripts");
    //delete old entries
    QList<unsigned int> idList = configGroup.readEntry("IDs", QList<unsigned int>());
    for(QList<unsigned int>::iterator i = idList.begin(); i != idList.end(); ++i) {
        configGroup.deleteEntry("Script" + QString::number(*i));
    }
    //write new ones
    idList.clear();
    for(QMap<unsigned int, Script*>::iterator i = m_idScriptMap.begin(); i != m_idScriptMap.end(); ++i) {
        unsigned int id = i.key();
        idList.push_back(id);
        configGroup.writePathEntry("Script" + QString::number(id), (*i)->getFileName());
    }
    configGroup.writeEntry("IDs", idList);
}

void Manager::addDirectoryToDirWatch(const QString& dir)
{
    //FIXME: no recursive watching and no watching of files as it isn't implemented
    //       yet
    //FIXME: check for KDE4
    if(!m_jScriptDirWatch->contains(dir)) {
        m_jScriptDirWatch->addDir(dir,  KDirWatch::WatchDirOnly);
    }
    QDir qDir(dir);
    QStringList list = qDir.entryList(QDir::Dirs);
    for(QStringList::iterator i = list.begin(); i != list.end(); ++i) {
        QString subdir = *i;
        if(subdir != "." && subdir != "..") {
            addDirectoryToDirWatch(qDir.filePath(subdir));
        }
    }
}

void Manager::readEnginePlugin()
{
    // TODO error message and disable scripting if not found
    QString pluginUrl = QStandardPaths::locate(QStandardPaths::DataLocation, "script-plugins/cursor-range.js");
    m_enginePlugin = Script::readFile(pluginUrl);
}

void Manager::initScriptActions()
{
    QStringList m_scriptActionList = QStringList()
                                     << "tag_chapter" << "tag_section" << "tag_subsection"
                                     << "tag_subsubsection" << "tag_paragraph" << "tag_subparagraph"

                                     << "tag_label" << "tag_ref" << "tag_pageref"
                                     << "tag_index" << "tag_footnote" << "tag_cite"

                                     << "tools_comment" << "tools_uncomment" << "tools_uppercase"
                                     << "tools_lowercase" << "tools_capitalize" << "tools_join_lines"

                                     << "wizard_tabular" << "wizard_array" << "wizard_tabbing"
                                     << "wizard_float" << "wizard_mathenv"
                                     << "wizard_postscript" << "wizard_pdf"
                                     ;


    foreach ( KXMLGUIClient *client, m_kileInfo->mainWindow()->guiFactory()->clients() ) {
        KILE_DEBUG_MAIN << "collection count: " << client->actionCollection()->count() ;

        foreach ( QAction *action, client->actionCollection()->actions() ) {
            QString objectname = action->objectName();
            if ( m_scriptActionList.indexOf(objectname) >= 0  ) {
                m_scriptActionMap->insert(objectname,action);
            }
        }
    }
}



////////////////////////////// ScriptExecutionAction //////////////////////////////

ScriptExecutionAction::ScriptExecutionAction(unsigned int id, KileScript::Manager *manager, QObject* parent) : QAction(parent), m_manager(manager), m_id(id)
{
    const KileScript::Script *script = m_manager->getScript(m_id);
    Q_ASSERT(script);
    setText(i18n("Execution of %1", script->getName()));
    connect(this, SIGNAL(triggered()), this, SLOT(executeScript()));
}

ScriptExecutionAction::~ScriptExecutionAction()
{
}

void ScriptExecutionAction::executeScript()
{
    m_manager->executeScript(m_id);
}



}

