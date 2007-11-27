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
#include "kilejscript.h"

#include <ktexteditor/document.h>
#include <klocale.h>
//Added by qt3to4:
#include <QEvent>
#include <Q3ValueList>
#include <QKeyEvent>

#define MAX(a,b) (a >= b ? a : b)

namespace KileEditorKeySequence {

	Manager::Manager(KileInfo* kileInfo, QObject *parent, const char *name) : QObject(parent, name), m_kileInfo(kileInfo) {
	}

	Manager::~Manager() {
	}

	void Manager::addAction(const QString& seq, Action *action) {
		if(seq.isEmpty()) {
			return;
		}
		if(m_actionMap.find(seq) == m_actionMap.end()) {
			m_actionMap[seq] = action;
			m_watchedKeySequencesList.push_back(seq);
			emit watchedKeySequencesChanged();
		}
	}


	void Manager::removeKeySequence(const QString& seq) {
		if(seq.isEmpty()) {
			return;
		}
		QMap<QString, Action*>::iterator it = m_actionMap.find(seq);
		if(it != m_actionMap.end()) {
			delete (it.data());
			m_actionMap.remove(it);
			m_watchedKeySequencesList.remove(seq);
			emit watchedKeySequencesChanged();
		}
	}

	void Manager::removeKeySequence(const QStringList& l) {
		bool changed = false;
		for(QStringList::const_iterator i = l.begin(); i != l.end(); ++i) {
			if((*i).isEmpty()) {
				continue;
			}
			QMap<QString, Action*>::iterator it = m_actionMap.find(*i);
			if(it != m_actionMap.end()) {
				delete (it.data());
				m_actionMap.remove(it);
				m_watchedKeySequencesList.remove(*i);
				changed = true;
			}
		}
		if(changed) {
			emit watchedKeySequencesChanged();
		}
	}

	void Manager::addActionMap(const QMap<QString, Action*>& map) {
		bool changed = false;
		for(QMap<QString, Action*>::const_iterator i = map.begin(); i != map.end(); ++i) {
			if(i.key().isEmpty()) {
				continue;
			}
			if(m_actionMap[i.key()] != i.data()) {
				m_actionMap[i.key()] = i.data();
				changed = true;
			}
		}
		if(changed) {
			emit watchedKeySequencesChanged();
		}
	}

	QString Manager::getKeySequence(const Action* a) {
		for(QMap<QString, Action*>::const_iterator i = m_actionMap.begin(); i != m_actionMap.end(); ++i) {
			if(i.data() == a) {
				return i.key();
			}
		}
		return QString();
	}

	Action* Manager::getAction(const QString& seq) {
		QMap<QString, Action*>::iterator i = m_actionMap.find(seq);
		return (i == m_actionMap.end()) ? 0L : (*i);
	}

	void Manager::setEditorKeySequence(const QString& /* seq */, Action* /* action */) {
	}

	void Manager::keySequenceTyped(const QString& seq) {
		m_actionMap[seq]->execute();
	}

	void Manager::clear() {
		m_watchedKeySequencesList.clear();
		m_actionMap.clear();
		emit watchedKeySequencesChanged();
	}


	const QStringList& Manager::getWatchedKeySequences() {
		return m_watchedKeySequencesList;
	}

	bool Manager::isSequenceAssigned(const QString& seq) const {
		for(Q3ValueList<QString>::const_iterator i = m_watchedKeySequencesList.begin(); i != m_watchedKeySequencesList.end(); ++i) {
			if((*i).startsWith(seq)) {
				return true;
			}
		}
		return false;
	}

	QPair<int, QString> Manager::checkSequence(const QString& seq, const QString& skip) {
		for(Q3ValueList<QString>::iterator i = m_watchedKeySequencesList.begin(); i != m_watchedKeySequencesList.end(); ++i) {
			if((*i) == skip) {
				continue;
			}
			if((*i).startsWith(seq)) {
				return (*i == seq) ? qMakePair<int, QString>(1, seq) : qMakePair<int, QString>(2, *i);
			}
 			if(!(*i).isEmpty() && seq.startsWith(*i)) {
				return qMakePair<int, QString>(3, *i);
			}
		}
		return qMakePair<int, QString>(0, QString());
	}

Recorder::Recorder(KTextEditor::View *view, Manager *manager) : QObject(view), m_manager(manager), m_view(view) {
	connect(m_manager, SIGNAL(watchedKeySequencesChanged()), this, SLOT(reloadWatchedKeySequences()));
	connect(this, SIGNAL(detectedTypedKeySequence(const QString&)), m_manager, SLOT(keySequenceTyped(const QString&)));
	m_view->cursorPositionReal(&m_oldLine, &m_oldCol);
	reloadWatchedKeySequences();
}

Recorder::~Recorder() {
}

bool Recorder::eventFilter(QObject* /* o */, QEvent *e) {
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = (QKeyEvent*)(e);
		uint curLine, curCol;
		m_view->cursorPositionReal(&curLine, &curCol);
		if(curLine != m_oldLine || m_oldCol+1 != curCol) {
			m_typedSequence = QString();
			m_oldLine = curLine;
			m_oldCol = curCol;
		}
		else {
			++m_oldCol;
		}
		m_typedSequence += keyEvent->text();
		if(m_typedSequence.length() == m_maxSequenceLength+1) {
			m_typedSequence = m_typedSequence.mid(1, m_typedSequence.length() - 1);
		}
		return seekForKeySequence(m_typedSequence);
	}
	return false;
}

	bool Recorder::seekForKeySequence(const QString& s) {
		for(uint i = 0; i < s.length(); ++i) {
			QString toCheck = s.right(s.length() - i);
			if(m_watchedKeySequencesList.contains(toCheck) > 0) {
 				m_view->getDoc()->removeText(m_oldLine, m_oldCol-(s.length() - i - 1), m_oldLine, m_oldCol);
				m_typedSequence = QString::null; // clean m_typedSequence to avoid wrong action triggering if one presses keys without printable character
				emit detectedTypedKeySequence(toCheck);
				return true;
			}
		}
		return false;
	}

	void Recorder::reloadWatchedKeySequences() {
		m_watchedKeySequencesList = m_manager->getWatchedKeySequences();
		m_maxSequenceLength = 0;
		for(QStringList::iterator i = m_watchedKeySequencesList.begin(); i != m_watchedKeySequencesList.end(); ++i) {
			m_maxSequenceLength = MAX(m_maxSequenceLength, (*i).length());
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

	ExecuteJScriptAction::ExecuteJScriptAction(KileJScript::JScript *jScript, KileJScript::Manager *jScriptManager) : m_jScript(jScript), m_jScriptManager(jScriptManager){
	}

	ExecuteJScriptAction::~ExecuteJScriptAction() {
	}

	void ExecuteJScriptAction::execute() {
		m_jScriptManager->executeJScript(m_jScript);
	}

	QString ExecuteJScriptAction::getDescription() const {
		return i18n("Script execution of %1").arg(m_jScript->getFileName());
	}


}

#include "editorkeysequencemanager.moc"
