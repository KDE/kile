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

#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QList>
#include <QMap>
#include <QObject>

#include <KAction>
#include <KActionCollection>
#include <KConfig>
#include <KDirWatch>

#include <KTextEditor/Range>
#include <KTextEditor/Cursor>

#include "scripting/kilescriptobject.h"
#include "scripting/kilescriptview.h"
#include "scripting/kilescriptdocument.h"

class KileInfo;

namespace KileScript {

class Script;

/**
 * This class handles the scripting functionality in Kile.
 **/
class Manager : public QObject {
	Q_OBJECT

	public:
		/**
		 * Constructs a new Manager object.
		 **/
		Manager(KileInfo *info, KConfig *config, KActionCollection *actionCollection, QObject *parent = 0, const char *name = 0);
		virtual ~Manager();

		/**
		 * Executes a script in Kile's scripting environment.
		 * @param script the script that should be executed
		 **/
		void executeScript(const Script *script);

		/**
		 * Executes a script in Kile's scripting environment.
		 * @param id the id of the script that should be executed
		 **/
		void executeScript(unsigned int id);

		/**
		 * Retrieves a list of all the scripts that are currently available.
		 **/
		QList<Script*> getScripts();

		/**
		 * Writes the key sequence-to-script bindings to the KConfig object that has
		 * passed in the constructor.
		 **/
		void writeConfig();

		/**
		 * Assigns a key sequence to a script. If the parameter "keySequence" is empty,
		 * then nothing is done.
		 * @param script the script that is considered
		 * @param keySequence the key sequence that is assigned
		 **/
		void setEditorKeySequence(Script* script, int type, const QString& keySequence);

		/**
		 * Removes an assigned key sequence from a script.
		 * @param script the script that is considered
		 **/
		void removeEditorKeySequence(Script* script);

		/**
		 * Returns the directory that can be used by the used to store Kile's scripts.
		 * Usually $HOME/.kde/share/apps/kile/scripts
		 **/
		QString getLocalScriptDirectory() const;

		/**
		 * Returns the script object that corresponds to a script id.
		 * @param id the id of the script
		 **/
		const Script* getScript(unsigned int id);

		void initScriptActions();

	Q_SIGNALS:
		/**
		 * Signal emitted whenever the managed scripts haved changed, for example if the
		 * watched directories have been scanned for scripts and thus, the potentially
		 * available scripts (could) have changed.
		 * The signal is also emitted when the currently available scripts have been
		 * deleted internally in Kile (for example, after disabling the scripting feature).
		 **/
		void scriptsChanged();

	public Q_SLOTS:
		/**
		 * Does nothing if scripting has been disabled.
		 **/
		void scanScriptDirectories();

		/**
		 * Reads and assigns the key sequence-to-script bindings from the KConfig
		 * object that has been passed in the constructor.
		 **/
		void readConfig();

	protected:
		QString m_localScriptDir;
		QList<Script*> m_jScriptList;
 		QMap<unsigned int, Script*> m_idScriptMap;
		KDirWatch *m_jScriptDirWatch;

		KileInfo *m_kileInfo;
		KConfig *m_config;
		KActionCollection *m_actionCollection;

		/**
		 * Registers the script contained in a file.
		 * @param fileName the file that contains the script
		 **/
		void registerScript(const QString& fileName, QMap<QString, unsigned int>& pathIDMap, QMap<unsigned int, bool>& takenIDMap, unsigned int &maxID);

		/**
		 * (Re-)Creates and initialises the KDirWatch object.
		 **/
		void populateDirWatch();

		/**
		 * Deletes all the scripts that are handled by this manager.
		 **/
		void deleteScripts();

		/**
		 * Finds the next free ID.
		 * @param takenIDMap map describing which IDs are already in use
		 * @param maxID the maximum ID that is currently in use (if there is no ID assigned, then
		 *              any value can be passed here, but typically '0')
		 **/
		unsigned int findFreeID(const QMap<unsigned int, bool>& takenIDMap, unsigned int maxID);

		/**
		 * Writes the ID to file name mappings that are currently in use to the local
		 * KConfig object.
		 **/
		void writeIDs();

		/**
		 * read the plugin to initialize Cursor and Range objects
		 **/
		void readEnginePlugin();

	private:
		/**
		 * Recursively adds a directory to a KDirWatch object.
		 * @param dir the directory that should be added
		 **/
		void addDirectoryToDirWatch(const QString& dir);

		KileScriptObject *m_kileScriptObject;
		KileScriptView *m_kileScriptView;
		KileScriptDocument *m_kileScriptDocument;

		QString m_enginePlugin;
		QMap<QString,QAction *> *m_scriptActionMap;
};

class ScriptExecutionAction : public KAction {
	Q_OBJECT

	public:
		ScriptExecutionAction(unsigned int scriptID, Manager *manager, QObject* parent = 0);

		virtual ~ScriptExecutionAction();

	protected Q_SLOTS:
		void executeScript();

	protected:
		KileScript::Manager *m_manager;
		unsigned int m_id;
};

}




#endif

