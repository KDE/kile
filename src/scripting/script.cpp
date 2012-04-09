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

#include <QFile>
#include <QTextStream>
#include <QScriptValue>
#include <QTimer>

#include <KActionCollection>
#include <KTextEditor/Range>
#include <KTextEditor/Cursor>
#include <KStandardDirs>
#include <KLocale>
#include <KMessageBox>

#include <iostream>

#include "kileinfo.h"
#include "kiledebug.h"
#include "scripting/script.h"
#include "scripting/kilescriptobject.h"
#include "scripting/kilescriptview.h"
#include "scripting/kilescriptdocument.h"

namespace KileScript {

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


//BEGIN QtScript conversion functions for Cursors and Ranges
/** Conversion function from KTextEditor::Cursor to QtScript cursor */
static QScriptValue cursorToScriptValue(QScriptEngine *engine, const KTextEditor::Cursor &cursor)
{
  QString code = QString("new Cursor(%1, %2);").arg(cursor.line())
                                               .arg(cursor.column());
  return engine->evaluate(code);
}

/** Conversion function from QtScript cursor to KTextEditor::Cursor */
static void cursorFromScriptValue(const QScriptValue &obj, KTextEditor::Cursor &cursor)
{
  cursor.setPosition(obj.property("line").toInt32(),
                     obj.property("column").toInt32());
}

/** Conversion function from QtScript range to KTextEditor::Range */
static QScriptValue rangeToScriptValue(QScriptEngine *engine, const KTextEditor::Range &range)
{
  QString code = QString("new Range(%1, %2, %3, %4);").arg(range.start().line())
                                                      .arg(range.start().column())
                                                      .arg(range.end().line())
                                                      .arg(range.end().column());
  return engine->evaluate(code);
}

/** Conversion function from QtScript range to KTextEditor::Range */
static void rangeFromScriptValue(const QScriptValue &obj, KTextEditor::Range &range)
{
  range.start().setPosition(obj.property("start").property("line").toInt32(),
                            obj.property("start").property("column").toInt32());
  range.end().setPosition(obj.property("end").property("line").toInt32(),
                          obj.property("end").property("column").toInt32());
}
//END

////////////////////////////// Script //////////////////////////////

/* The IDs of the scripts are used to maintain correct bindings with KAction objects, i.e. for example, we
 * want to make sure the action script_execution_0 always refers to same script (the script with id 0 !), even
 * after reloading all the scripts.
 */

Script::Script(unsigned int id, const QString& file)
   : m_id(id), m_file(file), m_action(NULL), m_sequencetype(KEY_SEQUENCE)
{
	m_name = KGlobal::dirs()->relativeLocation("appdata", file);
	if(m_name.startsWith("scripts")) {
		m_name = m_name.mid(8); // remove "scripts" + path separator
	}
	if(m_name.endsWith(".js")) { // remove the extension
		m_name = m_name.left(m_name.length() - 3);
	}
}

QString Script::getCode() const
{
	return readFile(m_file);
}

QString Script::getName() const
{
	return m_name;
}

QString Script::getFileName() const
{
	return m_file;
}

unsigned int Script::getID() const
{
	return m_id;
}

void Script::setID(unsigned int id)
{
	m_id = id;
}

void Script::setActionObject(KAction* action)
{
	m_action = action;
}

// const KAction* Script::getActionObject() const
// {
// 	return m_action;
// }

KAction* Script::getActionObject() const
{
	return m_action;
}

void Script::setKeySequence(const QString& str)
{
	m_keySequence = str;
}

QString Script::getKeySequence() const
{
	return m_keySequence;
}


int Script::getSequenceType() const
{
	return m_sequencetype;
}

void Script::setSequenceType(int type)
{
	m_sequencetype = type;
}

QString Script::readFile(const QString &filename) {
	QFile file(filename);
	if ( !file.open(QIODevice::ReadOnly) ) {
		KILE_DEBUG() << i18n("Unable to find '%1'", filename);
		return QString();
	} else {
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		QString text = stream.readAll();
		file.close();
		return text;
	}
}

////////////////////////////// ScriptEnvironment //////////////////////////////

ScriptEnvironment::ScriptEnvironment(KileInfo *kileInfo,
   KileScriptView *scriptView, KileScriptDocument *scriptDocument,
   KileScriptObject *scriptObject, const QString &pluginCode)
   : m_kileInfo(kileInfo), m_scriptView(scriptView), m_scriptDocument(scriptDocument),
     m_kileScriptObject(scriptObject), m_enginePluginCode(pluginCode)
{

	KILE_DEBUG() << "create ScriptEnvironment";
	m_engine = new QScriptEngine();
	qScriptRegisterMetaType(m_engine, cursorToScriptValue, cursorFromScriptValue);
	qScriptRegisterMetaType(m_engine, rangeToScriptValue, rangeFromScriptValue);
}

ScriptEnvironment::~ScriptEnvironment()
{
	delete m_engine;
}

// Executes script code in this environment.
void ScriptEnvironment::execute(const Script *script)
{
	// initialize engine to work with Cursor and Range objects
	m_engine->evaluate(m_enginePluginCode,i18n("Cursor/Range plugin"));

	if ( m_engine->hasUncaughtException() ) {
		scriptError(i18n("Cursor/Range plugin"));
		return;
	}
	else {
		KILE_DEBUG() << "Cursor/Range plugin successfully installed ";
	}

	// set global objects
	if ( m_scriptView->view() ) {
		m_engine->globalObject().setProperty("view", m_engine->newQObject(m_scriptView));
		m_engine->globalObject().setProperty("document", m_engine->newQObject(m_scriptDocument));
	}
	m_engine->globalObject().setProperty("kile", m_engine->newQObject(m_kileScriptObject));

  // export debug function
  m_engine->globalObject().setProperty("debug", m_engine->newFunction(KileScript::debug));

	// start engine
	m_engine->evaluate(script->getCode());

	// success or error
	if ( m_engine->hasUncaughtException() ) {
		scriptError(script->getName());
	}
	else {
		KILE_DEBUG() << "script finished without errors";
	}

 //FIXME: add time execution limit once it becomes available
// 			bool useGuard = KileConfig::timeLimitEnabled();
// 			uint timeLimit = (uint)KileConfig::timeLimit();
// 			KJSCPUGuard guard;
// 			if(useGuard) {
// 				guard.start(timeLimit*1000);
// 			}
// 			KJS::Completion completion = m_interpreter->evaluate(QString(), 0, s);
// 			if(useGuard) {
// 				guard.stop();
// 			}
	QTimer::singleShot(0, m_scriptView->view(), SLOT(setFocus()));

	// remove global objects
	m_engine->globalObject().setProperty("view", QScriptValue());
	m_engine->globalObject().setProperty("document", QScriptValue());
	m_engine->globalObject().setProperty("kile", QScriptValue());
}

// Executes script code in this environment.
void ScriptEnvironment::scriptError(const QString &name)
{
	int errorline = m_engine->uncaughtExceptionLineNumber();
	QScriptValue exception = m_engine->uncaughtException();
	QString errormessage = ( exception.isError() ) ? exception.toString() : QString();
	QString message = i18n("An error has occurred at line %1 during the execution of the script \"%2\":\n%3", errorline, name, errormessage);
	KMessageBox::sorry(m_kileInfo->mainWindow(), message, i18n("Error"));
}

////////////////////////////// ScriptHelpers //////////////////////////////

QScriptValue debug(QScriptContext *context, QScriptEngine *engine)
{
	QStringList message;
	for(int i = 0; i < context->argumentCount(); ++i) {
		message << context->argument(i).toString();
	}
	// debug in blue to distance from other debug output if necessary
	std::cerr << "\033[31m" << qPrintable(message.join(" ")) << "\033[0m\n";
	return engine->nullValue();
}

}
