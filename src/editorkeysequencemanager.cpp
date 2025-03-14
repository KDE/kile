/**************************************************************************
*   Copyright (C) 2006 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "editorkeysequencemanager.h"

#include "kileinfo.h"
#include "scriptmanager.h"
#include "scripting/script.h"

#include <QEvent>
#include <QKeyEvent>

#include <KTextEditor/Document>
#include <KLocalizedString>

namespace KileEditorKeySequence {

Manager::Manager(KileInfo* kileInfo, QObject *parent, const char *name) : QObject(parent), m_kileInfo(kileInfo)
{
    setObjectName(name);
}

Manager::~Manager()
{
}

void Manager::addAction(const QString& seq, Action *action)
{
    if(seq.isEmpty()) {
        return;
    }
    if(m_actionMap.find(seq) == m_actionMap.end()) {
        m_actionMap[seq] = action;
        m_watchedKeySequencesList.push_back(seq);
        Q_EMIT watchedKeySequencesChanged();
    }
}

void Manager::removeKeySequence(const QString& seq)
{
    if(seq.isEmpty()) {
        return;
    }
    QMap<QString, Action*>::iterator it = m_actionMap.find(seq);
    if(it != m_actionMap.end()) {
        delete (it.value());
        m_actionMap.erase(it);
        m_watchedKeySequencesList.removeAll(seq);
        Q_EMIT watchedKeySequencesChanged();
    }
}

void Manager::removeKeySequence(const QStringList& l)
{
    bool changed = false;
    for(const QString& entry : l) {
        if(entry.isEmpty()) {
            continue;
        }
        QMap<QString, Action*>::iterator it = m_actionMap.find(entry);
        if(it != m_actionMap.end()) {
            delete (it.value());
            m_actionMap.erase(it);
            m_watchedKeySequencesList.removeAll(entry);
            changed = true;
        }
    }
    if(changed) {
        Q_EMIT watchedKeySequencesChanged();
    }
}

void Manager::addActionMap(const QMap<QString, Action*>& map)
{
    bool changed = false;
    for(QMap<QString, Action*>::const_iterator i = map.begin(); i != map.end(); ++i) {
        if(!i.key().isEmpty() && (m_actionMap[i.key()] != i.value())) {
            m_actionMap[i.key()] = i.value();
            changed = true;
        }
    }
    if(changed) {
        Q_EMIT watchedKeySequencesChanged();
    }
}

QString Manager::getKeySequence(const Action* a)
{
    for(QMap<QString, Action*>::const_iterator i = m_actionMap.constBegin(); i != m_actionMap.constEnd(); ++i) {
        if(i.value() == a) {
            return i.key();
        }
    }
    return QString();
}

Action* Manager::getAction(const QString& seq)
{
    QMap<QString, Action*>::iterator i = m_actionMap.find(seq);
    return (i == m_actionMap.end()) ? nullptr : (*i);
}

void Manager::setEditorKeySequence(const QString& /* seq */, Action* /* action */)
{
}

void Manager::keySequenceTyped(const QString& seq)
{
    m_actionMap[seq]->execute();
}

void Manager::clear()
{
    m_watchedKeySequencesList.clear();
    m_actionMap.clear();
    Q_EMIT watchedKeySequencesChanged();
}


const QStringList& Manager::getWatchedKeySequences()
{
    return m_watchedKeySequencesList;
}

bool Manager::isSequenceAssigned(const QString& seq) const {
    for(const QString& entry : std::as_const(m_watchedKeySequencesList)) {
        if(entry.startsWith(seq)) {
            return true;
        }
    }
    return false;
}

std::pair<int, QString> Manager::checkSequence(const QString& seq, const QString& skip)
{
    for(const QString& entry : std::as_const(m_watchedKeySequencesList)) {
        if (entry == skip) {
            continue;
        }
        if(entry.startsWith(seq)) {
            return (entry == seq) ? std::pair<int, QString>(1, seq) : std::pair<int, QString>(2, entry);
        }
        if(!entry.isEmpty() && seq.startsWith(entry)) {
            return std::pair<int, QString>(3, entry);
        }
    }
    return qMakePair<int, QString>(0, QString());
}

Recorder::Recorder(KTextEditor::View *view, Manager *manager) : QObject(view), m_manager(manager), m_view(view)
{
    connect(m_manager, SIGNAL(watchedKeySequencesChanged()), this, SLOT(reloadWatchedKeySequences()));
    connect(this, SIGNAL(detectedTypedKeySequence(QString)), m_manager, SLOT(keySequenceTyped(QString)));
    KTextEditor::Cursor cursor = m_view->cursorPosition();
    m_oldLine = cursor.line();
    m_oldCol = cursor.column();

    reloadWatchedKeySequences();
}

Recorder::~Recorder()
{
}

bool Recorder::eventFilter(QObject* /* o */, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        int curLine, curCol;
        KTextEditor::Cursor cursor = m_view->cursorPosition();
        curLine = cursor.line();
        curCol = cursor.column();
        if(curLine != m_oldLine || m_oldCol+1 != curCol) {
            m_typedSequence.clear();
            m_oldLine = curLine;
            m_oldCol = curCol;
        }
        else {
            ++m_oldCol;
        }
        m_typedSequence += keyEvent->text();
        if(m_typedSequence.length() == m_maxSequenceLength + 1) {
            m_typedSequence = m_typedSequence.mid(1, m_typedSequence.length() - 1);
        }
        return seekForKeySequence(m_typedSequence);
    }
    return false;
}

bool Recorder::seekForKeySequence(const QString& s)
{
    for(int i = 0; i < s.length(); ++i) {
        QString toCheck = s.right(s.length() - i);
        if(m_watchedKeySequencesList.contains(toCheck)) {
            m_view->document()->removeText(KTextEditor::Range(m_oldLine, m_oldCol - (s.length() - i - 1), m_oldLine, m_oldCol));
            m_typedSequence.clear(); // clean m_typedSequence to avoid wrong action triggering if one presses keys without printable character
            Q_EMIT detectedTypedKeySequence(toCheck);
            return true;
        }
    }
    return false;
}

void Recorder::reloadWatchedKeySequences()
{
    m_watchedKeySequencesList = m_manager->getWatchedKeySequences();
    m_maxSequenceLength = 0;
    for(QStringList::iterator i = m_watchedKeySequencesList.begin(); i != m_watchedKeySequencesList.end(); ++i) {
        m_maxSequenceLength = qMax(m_maxSequenceLength, (*i).length());
    }
    if(m_maxSequenceLength < m_typedSequence.length()) {
        m_typedSequence = m_typedSequence.right(m_maxSequenceLength);
    }
}

Action::Action() {
}

Action::~Action() {
}

QString Action::getDescription() const {
    return QString();
}

ExecuteScriptAction::ExecuteScriptAction(KileScript::Script *script, KileScript::Manager *scriptManager) : m_script(script), m_scriptManager(scriptManager)
{
}

ExecuteScriptAction::~ExecuteScriptAction()
{
}

void ExecuteScriptAction::execute()
{
    m_scriptManager->executeScript(m_script);
}

QString ExecuteScriptAction::getDescription() const
{
    return i18n("Script execution of %1", m_script->getFileName());
}


}

