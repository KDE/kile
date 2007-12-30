/**************************************************************************
*   Copyright (C) 2006, 2007 by Michel Ludwig (michel.ludwig@kdemail.net) *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KILEJSCRIPT_H
#define KILEJSCRIPT_H

#include <qmap.h>
#include <qobject.h>

#include <kjs/function.h>
#include <kjs/interpreter.h>
#include <kjs/object.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kdirwatch.h>

#include <QList>

class KileInfo;

namespace KileJScript {

/**
 * This class represents a JavaScript script.
 **/
class JScript {
	public:
		/**
		 * Constructs a new JavaScript script.
		 * @param file the file that contains the script
		 **/
		JScript(unsigned int id, const QString& file);

		/**
		 * Returns the code of this script, i.e. the file is read and its contents are
		 * returned.
		 **/
		QString getCode() const;
	
		/**
		 * Returns the name of the script (the base name of the file).
		 **/
		QString getName() const;

		/**
		 * Returns the file of the script (the full path, including the base name).
		 **/
		QString getFileName() const;

		/**
		 * Returns the unique identifier of this script.
		 **/
		unsigned int getID() const;

		/**
		 * Sets the unique identifier of this script.
		 **/
		void setID(unsigned int id);


		/**
		 *
		 **/
		void setActionObject(KAction* action);

		const KAction* getActionObject() const;

		KAction* getActionObject();

		void setKeySequence(const QString& str);
		QString getKeySequence() const;

	protected:
		unsigned int m_id;
		QString m_code;
		QString m_file;
		QString m_name;
		KAction *m_action;
		QString m_keySequence;
};

/**
 * This class represents the JavaScript environment that is used to execute Kile's scripts
 * in.
 **/
class JScriptEnvironment {
	public:
		/**
		 * Constructs a new environment.
		 **/
		JScriptEnvironment(KileInfo *kileInfo);
		~JScriptEnvironment();

		/**
		 * Executes JavaScript code in this environment.
		 * @param c the code that should be executed
		 **/
		void execute(const QString& c);

	protected:
		KJS::Interpreter *m_interpreter;
		KJS::JSObject* m_kileJSObject;
		KileInfo *m_kileInfo;
};

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
		void executeJScript(const JScript *script);

		/**
		 * Executes a script in Kile's scripting environment.
		 * @param id the id of the script that should be executed
		 **/
		void executeJScript(unsigned int id);

		/**
		 * Retrieves a list of all the scripts that are currently available.
		 **/
		QList<JScript*> getJScripts();

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
		void setEditorKeySequence(JScript* script, const QString& keySequence);

		/**
		 * Removes an assigned key sequence from a script.
		 * @param script the script that is considered
		 **/
		void removeEditorKeySequence(JScript* script);

		/**
		 * Returns the directory that can be used by the used to store Kile's scripts.
		 * Usually $HOME/.kde/share/apps/kile/scripts
		 **/
		QString getLocalJScriptDirectory() const;

		/**
		 * Returns the script object that corresponds to a script id.
		 * @param id the id of the script
		 **/
		const JScript* getScript(unsigned int id);

	Q_SIGNALS:
		/**
		 * Signal emitted whenever the managed scripts haved changed, for example if the
		 * watched directories have been scanned for scripts and thus, the potentially
		 * available scripts (could) have changed.
		 * The signal is also emitted when the currently available scripts have been
		 * deleted internally in Kile (for example, after disabling the scripting feature).
		 **/
		void jScriptsChanged();

	public Q_SLOTS:
		/**
		 * Does nothing if scripting has been disabled.
		 **/
		void scanJScriptDirectories();

		/**
		 * Reads and assigns the key sequence-to-script bindings from the KConfig
		 * object that has been passed in the constructor.
		 **/
		void readConfig();

	protected:
		QString m_localJScriptDir;
		QList<JScript*> m_jScriptList;
 		QMap<unsigned int, JScript*> m_idScriptMap;
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
		void deleteJScripts();

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

	private:
		/**
		 * Recursively adds a directory to a KDirWatch object.
		 * @param dir the directory that should be added
		 **/
		void addDirectoryToDirWatch(const QString& dir);
};

class ScriptExecutionAction : public KAction {
	Q_OBJECT

	public:
		ScriptExecutionAction(unsigned int scriptID, Manager *manager, QObject* parent = 0);

		virtual ~ScriptExecutionAction();

	protected Q_SLOTS:
		void executeScript();

	protected:
		KileJScript::Manager *m_manager;
		unsigned int m_id;
};


}

#endif

