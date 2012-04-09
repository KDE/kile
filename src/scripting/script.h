/******************************************************************************
  Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net)
                2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QScriptEngine>
#include <QScriptContext>
#include <QMap>

#include <KAction>
#include <KTextEditor/View>


class KileInfo;

namespace KileScript {

class KileScriptObject;
class KileScriptView;
class KileScriptDocument;

////////////////////////////// Script //////////////////////////////

/**
 * This class represents a script.
 **/
class Script {
	public:
		enum SequenceType {
			KEY_SEQUENCE = 0,
			KEY_SHORTCUT
		};

		/**
		 * Constructs a new JavaScript script.
		 * @param file the file that contains the script
		 **/
		Script(unsigned int id, const QString& file);
		virtual ~Script() {}

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
//		const KAction* getActionObject() const;
		KAction* getActionObject() const;

		void setKeySequence(const QString& str);
		QString getKeySequence() const;

		int getSequenceType() const;
		void setSequenceType(int type);

		static QString readFile(const QString &filename);

private:
		unsigned int m_id;
		QString m_code;
		QString m_file;
		QString m_name;
		KAction *m_action;
		QString m_keySequence;
		int m_sequencetype;
		

};

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
		ScriptEnvironment(KileInfo *kileInfo, KileScriptView *scriptView, KileScriptDocument *scriptDocument,
		                  KileScriptObject *scriptObject, const QString &pluginCode);
		virtual ~ScriptEnvironment();

		/**
		 * Executes script code in this environment.
		 * @param s the script that should be executed
		 **/
		void execute(const Script *script);

	protected:
		KileInfo *m_kileInfo;
		KileScriptView *m_scriptView;
		KileScriptDocument *m_scriptDocument;
		KileScriptObject *m_kileScriptObject;

		QScriptEngine *m_engine;
		QString m_enginePluginCode;

		void scriptError(const QString &name);

};

////////////////////////////// ScriptHelpers //////////////////////////////

QScriptValue debug(QScriptContext *context, QScriptEngine *engine);

}

/**
 * metatype register
 */
Q_DECLARE_METATYPE(KTextEditor::Cursor)
Q_DECLARE_METATYPE(KTextEditor::Range)


#endif
