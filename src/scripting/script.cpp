/******************************************************************************
  Copyright (C) 2006-2017 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include "scripting/script.h"

#include <KTextEditor/Cursor>
#include <QFile>
#include <QTextStream>
#include <QJSValue>
#include <QJSEngine>
#include <QTimer>

#include <KActionCollection>
#include <KTextEditor/Range>
#include <KTextEditor/Cursor>
#include <KLocalizedString>
#include <KMessageBox>

#include <iostream>

#include "kileinfo.h"
#include "kiledebug.h"
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
  setitimer(ITIMER_VIRTUAL, &oldtv, Q_NULLPTR);
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
static QJSValue cursorToScriptValue(QJSEngine *engine, const KTextEditor::Cursor &cursor)
{
    QString code = QStringLiteral("new Cursor(%1, %2);").arg(cursor.line())
                   .arg(cursor.column());
    return engine->evaluate(code);
}

/** Conversion function from QtScript cursor to KTextEditor::Cursor */
static void cursorFromScriptValue(const QJSValue &obj, KTextEditor::Cursor &cursor)
{
    cursor.setPosition(obj.property(QStringLiteral("line")).toInt(),
                       obj.property(QStringLiteral("column")).toInt());
}

/** Conversion function from QtScript range to KTextEditor::Range */
static QJSValue rangeToScriptValue(QJSEngine *engine, const KTextEditor::Range &range)
{
    QString code = QString("new Range(%1, %2, %3, %4);").arg(range.start().line())
                   .arg(range.start().column())
                   .arg(range.end().line())
                   .arg(range.end().column());
    return engine->evaluate(code);
}

/** Conversion function from QtScript range to KTextEditor::Range */
static void rangeFromScriptValue(const QJSValue &obj, KTextEditor::Range &range)
{
    range.setStart(KTextEditor::Cursor(obj.property(QStringLiteral("start")).property(QStringLiteral("line")).toInt(),
                                       obj.property(QStringLiteral("start")).property(QStringLiteral("column")).toInt()));
    range.setEnd(KTextEditor::Cursor(obj.property(QStringLiteral("end")).property(QStringLiteral("line")).toInt(),
                                     obj.property(QStringLiteral("end")).property(QStringLiteral("column")).toInt()));
}
//END

////////////////////////////// Script //////////////////////////////

/* The IDs of the scripts are used to maintain correct bindings with QAction objects, i.e. for example, we
 * want to make sure the action script_execution_0 always refers to same script (the script with id 0 !), even
 * after reloading all the scripts.
 */

Script::Script(unsigned int id, const QString& file)
    : m_id(id), m_file(file), m_action(Q_NULLPTR), m_sequencetype(KEY_SEQUENCE)
{
    m_name = QFileInfo(file).fileName();

    if(m_name.endsWith(QLatin1String(".js"))) { // remove the extension
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

void Script::setActionObject(QAction * action)
{
    m_action = action;
}

// const QAction * Script::getActionObject() const
// {
// 	return m_action;
// }

QAction * Script::getActionObject() const
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
        KILE_DEBUG_MAIN << i18n("Unable to find '%1'", filename);
        return QString();
    } else {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
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

    KILE_DEBUG_MAIN << "create ScriptEnvironment";
    m_engine = new QJSEngine();
}

ScriptEnvironment::~ScriptEnvironment()
{
    delete m_engine;
}

// Executes script code in this environment.
void ScriptEnvironment::execute(const Script *script)
{
    // initialize engine to work with Cursor and Range objects
    m_engine->evaluate(m_enginePluginCode, i18n("Cursor/Range plugin"));

    if(m_engine->hasError()) {
        scriptError(i18n("Cursor/Range plugin"));
        return;
    }
    else {
        KILE_DEBUG_MAIN << "Cursor/Range plugin successfully installed ";
    }

    // set global objects
    if(m_scriptView->view()) {
        m_engine->globalObject().setProperty("view", m_engine->newQObject(m_scriptView));
        m_engine->globalObject().setProperty("document", m_engine->newQObject(m_scriptDocument));
    }
    m_engine->globalObject().setProperty("kile", m_engine->newQObject(m_kileScriptObject));

    // start engine
    m_engine->evaluate(script->getCode());

    // success or error
    if(m_engine->hasError()) {
        scriptError(script->getName());
    }
    else {
        KILE_DEBUG_MAIN << "script finished without errors";
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
    m_engine->globalObject().setProperty("view", QJSValue());
    m_engine->globalObject().setProperty("document", QJSValue());
    m_engine->globalObject().setProperty("kile", QJSValue());
}

// Executes script code in this environment.
void ScriptEnvironment::scriptError(const QString &name)
{
    int errorline = 0; // m_engine->uncaughtExceptionLineNumber();
    QJSValue exception = m_engine->catchError();
    QString errormessage = ( exception.isError() ) ? exception.toString() : QString();
    QString message = i18n("An error has occurred at line %1 during the execution of the script \"%2\":\n%3", errorline, name, errormessage);
    KMessageBox::error(m_kileInfo->mainWindow(), message, i18n("Error"));
}
}
