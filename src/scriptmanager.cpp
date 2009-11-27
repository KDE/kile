/**************************************************************************
*   Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net)  *
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
#include "kiledebug.h"
#include <KInputDialog>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>

#include <signal.h>
#include <sys/time.h>

#include <QEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QList>

#include "scripting/bindings.h"

#include "kileconfig.h"
#include "editorextension.h"
#include "kileinfo.h"
#include "kileversion.h"
#include "kileviewmanager.h"
#include "editorkeysequencemanager.h"

#include <kross/core/interpreter.h>
#include <kross/core/action.h>
#include <kross/core/manager.h>


/*
// Modified declaration from <khtml/ecma/kjs_proxy.h>
// Acknowledgements go to:
//  Copyright (C) 1999 Harri Porten (porten@kde.org)
//  Copyright (C) 2001 Peter Kelly (pmk@post.com)

class KJSCPUGuard {
public:
  KJSCPUGuard() {}
  void start(unsigned int msec=5000, unsigned int i_msec=0);
  void stop();
private:
  void (*oldAlarmHandler)(int);
  static void alarmHandler(int);
  itimerval oldtv;
};

// Modified implementation originating from <khtml/ecma/kjs_proxy.cpp>
// Acknowledgements go to:
// Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
// Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
// Copyright (C) 2001-2003 David Faure (faure@kde.org)
void KJSCPUGuard::start(unsigned int ms, unsigned int i_ms)
{
  oldAlarmHandler = signal(SIGVTALRM, alarmHandler);
  itimerval tv = {
      { i_ms / 1000, (i_ms % 1000) * 1000 },
      { ms / 1000, (ms % 1000) * 1000 }
  };
  setitimer(ITIMER_VIRTUAL, &tv, &oldtv);
}

void KJSCPUGuard::stop()
{
  setitimer(ITIMER_VIRTUAL, &oldtv, NULL);
  signal(SIGVTALRM, oldAlarmHandler);
}

void KJSCPUGuard::alarmHandler(int) {

//     KJS::ExecState::requestTerminate();
}
*/

#ifdef __GNUC__
#warning "Fix time limit functionality!"
#endif

namespace KileScript {
////////////////////////////// Script //////////////////////////////

/* The IDs of the scripts are used to maintain correct bindings with KAction objects, i.e. for example, we
 * want to make sure the action script_execution_0 always refers to same script (the script with id 0 !), even
 * after reloading all the scripts.
 */

	Script::Script(unsigned int id, const QString& file) : m_id(id), m_file(file), m_action(NULL) {
		m_name = KGlobal::dirs()->relativeLocation("appdata", file);
		if(m_name.startsWith("scripts")) {
			m_name = m_name.mid(8); // remove "scripts" + path separator
		}
		if(m_name.endsWith(".js")) { // remove the extension
			m_name = m_name.left(m_name.length() - 3);
		}
	}
	
	QString Script::getName() const {
		return m_name;
	}

	QString Script::getCode() const {
		QFile qFile(m_file);
		if(qFile.open(QIODevice::ReadOnly)) {
			QTextStream inputStream(&qFile);
// 			inputStream.setEncoding(QTextStream::UnicodeUTF8);
			QString code = inputStream.readAll();
			qFile.close();
			return code;
		}
		else {
			return QString();
		}
	}

	QString Script::getFileName() const {
		return m_file;
	}

	unsigned int Script::getID() const {
		return m_id;
	}

	void Script::setID(unsigned int id) {
		m_id = id;
	}

	void Script::setActionObject(KAction* action) {
		m_action = action;
	}

	const KAction* Script::getActionObject() const {
		return m_action;
	}

	KAction* Script::getActionObject() {
		return m_action;
	}

	void Script::setKeySequence(const QString& str) {
		m_keySequence = str;
	}

	QString Script::getKeySequence() const {
		return m_keySequence;
	}

////////////////////////////// ScriptEnvironment //////////////////////////////


/**
 * This class represents the environment that is used to execute Kile's scripts
 * in.
 **/
class ScriptEnvironment {
	public:
		/**
		 * Constructs a new environment.
		 **/
		ScriptEnvironment(KileInfo *kileInfo) : m_kileInfo(kileInfo), m_kileScriptObject(new KileScriptObject(NULL, m_kileInfo))
		{
		}

		~ScriptEnvironment()
		{
			delete m_kileScriptObject;
		}

		/**
		 * Executes script code in this environment.
		 * @param s the script that should be executed
		 **/
		void execute(const Script *script)
		{
			Kross::Action* action = new Kross::Action(NULL, "KileScript");
			action->addObject(m_kileScriptObject, "kile");
			action->setInterpreter(Kross::Manager::self().interpreternameForFile(script->getFileName()));
			action->setFile(script->getFileName());
			action->trigger();
//FIXME: add time execution limit once it becomes available
/*
			bool useGuard = KileConfig::timeLimitEnabled();
			uint timeLimit = (uint)KileConfig::timeLimit();
			KJSCPUGuard guard;
			if(useGuard) {
				guard.start(timeLimit*1000);
			}
			KJS::Completion completion = m_interpreter->evaluate(QString(), 0, s);
			if(useGuard) {
				guard.stop();
			}
*/
			if(action->hadError()) {
				const int errorLine = action->errorLineNo();
				QString visibleErrorMessage;
				if(errorLine >= 0) {
					visibleErrorMessage = i18n("An error has occurred at line %1 during the execution of the script \"%2\":\n%3", errorLine, script->getName(), action->errorMessage());
				}
				else {
					visibleErrorMessage = i18n("An error has occurred during the execution of the script \"%1\":\n%2", script->getName(), action->errorMessage());
				}
				KMessageBox::sorry(m_kileInfo->mainWindow(), visibleErrorMessage, i18n("Error"));
			}
			delete action;
		}

	protected:
		KileInfo *m_kileInfo;
		KileScriptObject *m_kileScriptObject;
};

////////////////////////////// Manager //////////////////////////////

	Manager::Manager(KileInfo *kileInfo, KConfig *config, KActionCollection *actionCollection, QObject *parent, const char *name)
	: QObject(parent), m_jScriptDirWatch(NULL), m_kileInfo(kileInfo), m_config(config), m_actionCollection(actionCollection)
	{
		setObjectName(name);
		// create a local scripts directory if it doesn't exist yet
		m_localScriptDir = KStandardDirs::locateLocal("appdata", "scripts/", true);
		m_jScriptDirWatch = new KDirWatch(this);
		m_jScriptDirWatch->setObjectName("KileScript::Manager::ScriptDirWatch");
		connect(m_jScriptDirWatch, SIGNAL(dirty(const QString&)), this, SLOT(scanScriptDirectories()));
		connect(m_jScriptDirWatch, SIGNAL(created(const QString&)), this, SLOT(scanScriptDirectories()));
		connect(m_jScriptDirWatch, SIGNAL(deleted(const QString&)), this, SLOT(scanScriptDirectories()));
 		m_jScriptDirWatch->startScan();
	}

	Manager::~Manager() {
		delete m_jScriptDirWatch;

		//still need to delete the scripts
		for(QList<Script*>::iterator it = m_jScriptList.begin(); it != m_jScriptList.end(); ++it) {
			delete *it;
		}
	}

	void Manager::executeScript(const Script *script) {
		ScriptEnvironment env(m_kileInfo);
// demonstrate repainting bug:
/*KMessageBox::information(m_kileInfo->mainWindow(), "works!!!");
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();
m_kileInfo->viewManager()->currentView()->down();*/
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
		env.execute(script);
	}

	void Manager::executeScript(unsigned int id) {
		QMap<unsigned int, Script*>::iterator i = m_idScriptMap.find(id);
		if(i != m_idScriptMap.end()) {
			executeScript(*i);
		}
	}

	const Script* Manager::getScript(unsigned int id) {
		QMap<unsigned int, Script*>::iterator i = m_idScriptMap.find(id);
		return ((i != m_idScriptMap.end()) ? (*i) : NULL);
	}

	void Manager::scanScriptDirectories() {
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

		QStringList scriptFileNamesList = KGlobal::dirs()->findAllResources("appdata", "scripts/*.js", KStandardDirs::Recursive | KStandardDirs::NoDuplicates);
		for(QStringList::iterator i = scriptFileNamesList.begin(); i != scriptFileNamesList.end(); ++i) {
			registerScript(*i, pathIDMap, takenIDMap, maxID);
		}
		//rewrite the IDs that are currently in use
		writeIDs();
		m_actionCollection->readSettings();
		emit scriptsChanged();
	}

	void Manager::deleteScripts() {
		QList<Script*> scriptList = m_jScriptList;
		m_jScriptList.clear(); // pretend that there are no scripts left
		QStringList keySequenceList;
		for(QList<Script*>::iterator it = scriptList.begin(); it != scriptList.end(); ++it) {
			keySequenceList.push_back((*it)->getKeySequence());
		}
		m_idScriptMap.clear();
		m_kileInfo->editorKeySequenceManager()->removeKeySequence(keySequenceList);
		for(QList<Script*>::iterator it = scriptList.begin(); it != scriptList.end(); ++it) {
			KAction *action = (*it)->getActionObject();
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

	QList<Script*> Manager::getScripts() {
		return m_jScriptList;
	}

	void Manager::registerScript(const QString& fileName, QMap<QString, unsigned int>& pathIDMap, QMap<unsigned int, bool>& takenIDMap, unsigned int &maxID) {
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
		QString editorKeySequence = configGroup.readEntry("Script" + QString::number(id) + "KeySequence");
		if(!editorKeySequence.isEmpty()) {
			script->setKeySequence(editorKeySequence);
			m_kileInfo->editorKeySequenceManager()->addAction(editorKeySequence, new KileEditorKeySequence::ExecuteScriptAction(script, this));
		}
		// now set up a regular action object
		ScriptExecutionAction *action = new ScriptExecutionAction(id, this, m_actionCollection);
		m_actionCollection->addAction("script" + QString::number(id) + "_execution", action);
		script->setActionObject(action);
	}

	void Manager::writeConfig() {
		// don't delete the key sequence settings if scripting has been disabled
		if(!KileConfig::scriptingEnabled()) {
			return;
		}
		m_config->deleteGroup("Scripts");
		writeIDs();
		// write the key sequences
		KConfigGroup configGroup = m_config->group("Scripts");
		for(QList<Script*>::iterator i = m_jScriptList.begin(); i != m_jScriptList.end(); ++i) {
			configGroup.writeEntry("Script" + QString::number((*i)->getID()) + "KeySequence", (*i)->getKeySequence());
		}
	}

	void Manager::setEditorKeySequence(Script* script, const QString& keySequence) {
		if(keySequence.isEmpty()) {
			return;
		}
		if(script) {
			QString oldSequence = script->getKeySequence();
			if(oldSequence == keySequence) {
				return;
			}
			m_kileInfo->editorKeySequenceManager()->removeKeySequence(oldSequence);
			script->setKeySequence(keySequence);
			m_kileInfo->editorKeySequenceManager()->addAction(keySequence, new KileEditorKeySequence::ExecuteScriptAction(script, this));
			writeConfig();
		}
	}

	void Manager::removeEditorKeySequence(Script* script) {
		if(script) {
			QString keySequence = script->getKeySequence();
			if(keySequence.isEmpty()) {
				return;
			}
			script->setKeySequence(QString());
			m_kileInfo->editorKeySequenceManager()->removeKeySequence(keySequence);
			writeConfig();
		}
	}

	void Manager::populateDirWatch() {
		QStringList jScriptDirectories = KGlobal::dirs()->findDirs("appdata", "scripts");
		for(QStringList::iterator i = jScriptDirectories.begin(); i != jScriptDirectories.end(); ++i) {
			// FIXME: future KDE versions could support the recursive
			//        watching of directories out of the box.
			addDirectoryToDirWatch(*i);
		}
		//we do not remove the directories that were once added as this apparently causes some strange
		//bugs (on KDE 3.5.x)
	}

	QString Manager::getLocalScriptDirectory() const {
		return m_localScriptDir;
	}

	void Manager::readConfig() {
		deleteScripts();
		scanScriptDirectories();
	}

	unsigned int Manager::findFreeID(const QMap<unsigned int, bool>& takenIDMap, unsigned int maxID) {
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

	void Manager::writeIDs() {
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

	void Manager::addDirectoryToDirWatch(const QString& dir) {
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
////////////////////////////// ScriptExecutionAction //////////////////////////////

	ScriptExecutionAction::ScriptExecutionAction(unsigned int id, KileScript::Manager *manager, QObject* parent) : KAction(parent), m_manager(manager), m_id(id) {
		const KileScript::Script *script = m_manager->getScript(m_id);
		Q_ASSERT(script);
		setText(i18n("Execution of %1", script->getName()));
		connect(this, SIGNAL(triggered()), this, SLOT(executeScript()));
	}

	ScriptExecutionAction::~ScriptExecutionAction()	{
	}

	void ScriptExecutionAction::executeScript() {
		m_manager->executeScript(m_id);
	}

}

#include "scriptmanager.moc"
