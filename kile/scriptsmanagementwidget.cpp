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

#include "scriptsmanagementwidget.h"

#include <qlayout.h>
#include <qtooltip.h>
#include <qvbox.h>

#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "editorkeysequencemanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kilejscript.h"

namespace KileWidget {

JScriptListViewItem::JScriptListViewItem(QWidget *managementWidget, KListView *parent, KileJScript::JScript *script, KileInfo *kileInfo) : KListViewItem(parent), m_script(script), m_kileInfo(kileInfo), m_managementWidget(managementWidget) {
}

JScriptListViewItem::~JScriptListViewItem() {
}

KileJScript::JScript* JScriptListViewItem::getScript() {
	return m_script;
}

QString JScriptListViewItem::text(int column) const {
	switch(column) {
		case 0:
 			return m_script->getName();
		case 1:
			return m_script->getKeySequence();
	}
	return QString();
}


void JScriptListViewItem::setText(int column, const QString & str) {
	if(column == 1) {
		QString oldAssignedString = m_script->getKeySequence();
		if(str.isEmpty()) {
			// don't reload the list view here as this may cause a segfault (for example on x86_64)
			QObject::disconnect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), m_managementWidget, SLOT(updateListView()));
			m_kileInfo->scriptManager()->removeEditorKeySequence(m_script);
			QObject::connect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), m_managementWidget, SLOT(updateListView()));
		}
		else if(str == oldAssignedString || (str.isEmpty() && oldAssignedString.isEmpty())) {
			return;
		}
		else {
			QPair<int, QString> pair = m_kileInfo->editorKeySequenceManager()->checkSequence(str, oldAssignedString);
			if(pair.first == 0) {
				// don't reload the list view here as this may cause a segfault (for example on x86_64)
				QObject::disconnect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), m_managementWidget, SLOT(updateListView()));
				m_kileInfo->scriptManager()->setEditorKeySequence(m_script, str);
				QObject::connect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), m_managementWidget, SLOT(updateListView()));
				return; // leaving !
			}
			KileEditorKeySequence::Action *action = m_kileInfo->editorKeySequenceManager()->getAction(pair.second);
			QString description = (action == 0L) ? QString() : action->getDescription();
			switch(pair.first) {
				case 1:
					KMessageBox::sorry(0L, i18n("The sequence \"%1\" is already assigned to the action \"%2\"").arg(str).arg(description), i18n("Sequence Already Assigned"));
					break;
				case 2:
					KMessageBox::sorry(0L, i18n("The sequence \"%1\" is a subsequence of \"%2\", which is already assigned to the action \"%3\"").arg(str).arg(pair.second).arg(description), i18n("Sequence Already Assigned"));
					break;
				case 3:
					KMessageBox::sorry(0L, i18n("The shorter sequence \"%1\" is already assigned to the action \"%2\"").arg(pair.second).arg(description), i18n("Sequence Already Assigned"));
					break;
			}
		}
	}
}

ScriptsManagement::ScriptsManagement(KileInfo *kileInfo, QWidget *parent, const char *name, WFlags f) : QWidget(parent, name, f), m_kileInfo(kileInfo) {
	QVBoxLayout *baseLayout = new QVBoxLayout(this);

	m_toolbar = new KToolBar(this, "scriptControlToolBar");
	m_executeButton = m_toolbar->insertButton(BarIcon("exec"), 0, SIGNAL(clicked(int)), this, SLOT(executeSelectedScript()), true, i18n("Run Selected Script"));
	m_newButton = m_toolbar->insertButton(BarIcon("scriptnew"), 0, SIGNAL(clicked(int)), m_kileInfo->docManager(), SLOT(createNewJScript()), true, i18n("Create New Script"));
	m_openButton = m_toolbar->insertButton(BarIcon("scriptopen"), 0, SIGNAL(clicked(int)), this, SLOT(openSelectedScript()), true, i18n("Open Selected Script in Editor"));
// 	m_toolbar->insertButton(BarIcon("configure_shortcuts"), 0, SIGNAL(clicked(int)), this, SLOT(configureSelectedShortcut()), true, i18n("Configure Shortcut"));
// 	m_toolbar->insertButton(BarIcon("editclear"), 1, SIGNAL(clicked(int)), m_kileInfo->scriptManager(), SLOT(scanJScriptDirectories()), true, i18n("Refresh"));
	m_refreshButton = m_toolbar->insertButton(BarIcon("reload"), 1, SIGNAL(clicked(int)), m_kileInfo->scriptManager(), SLOT(scanJScriptDirectories()), true, i18n("Refresh List"));

	baseLayout->addWidget(m_toolbar);
	m_scriptsListView = new KListView(this);
	m_scriptsListView->addColumn(i18n("Script Name"));
	m_scriptsListView->addColumn(i18n("Sequence"));
	m_scriptsListView->setAllColumnsShowFocus(true);
	m_scriptsListView->setItemsRenameable(true);
	m_scriptsListView->setRenameable(0, false);
	m_scriptsListView->setRenameable(1, true);

	connect(m_kileInfo->scriptManager(), SIGNAL(jScriptsChanged()), this, SLOT(updateListView()));
	connect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), this, SLOT(updateListView()));
// 	connect(m_scriptsListView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), this, SLOT(executed(QListViewItem*, const QPoint&, int)));
// 	connect(m_scriptsListView, SIGNAL(itemRenamed(QListViewItem*, const QString&, int)), this, SLOT(itemRenamed(QListViewItem*, const QString&, int)));
	connect(m_scriptsListView, SIGNAL(selectionChanged()), this, SLOT(updateButtonPanel()));

	baseLayout->addWidget(m_scriptsListView);
	updateButtonPanel();
	updateListView();
}

ScriptsManagement::~ScriptsManagement() {
}


void ScriptsManagement::updateListView() {
	m_scriptsListView->clear();
	const QValueList<KileJScript::JScript*>& scriptList = m_kileInfo->scriptManager()->getJScripts();
	for(QValueList<KileJScript::JScript*>::const_iterator i = scriptList.begin(); i != scriptList.end(); ++i) {
 		new JScriptListViewItem(this, m_scriptsListView, *i, m_kileInfo);
	}
	m_scriptsListView->triggerUpdate();
}

void ScriptsManagement::openSelectedScript() {
	QListViewItem *item = m_scriptsListView->selectedItem();
	if(!item) {
		return;
	}
	JScriptListViewItem *jItem = (JScriptListViewItem*)item;
	m_kileInfo->docManager()->fileOpen(jItem->getScript()->getFileName());
}

void ScriptsManagement::executeSelectedScript() {
	QListViewItem *item = m_scriptsListView->selectedItem();
	if(!item) {
		return;
	}
	JScriptListViewItem *jItem = (JScriptListViewItem*)item;
	m_kileInfo->scriptManager()->executeJScript(jItem->getScript());
}

// void ScriptsManagement::configureSelectedShortcut() {
// 	QListViewItem *item = m_scriptsListView->selectedItem();
// 	if(!item) {
// 		return;
// 	}
// 	JScriptListViewItem *jItem = (JScriptListViewItem*)item;
// 	QString sequence = determineKeySequence();
// 	m_kileInfo->scriptManager()->setEditorKeySequence(jItem->getScript(), sequence);
// }

void ScriptsManagement::updateButtonPanel() {
	bool b = !(m_scriptsListView->selectedItem() == NULL);
	m_toolbar->setItemEnabled(m_executeButton, b);
	m_toolbar->setItemEnabled(m_openButton, b);
}

}

#include "scriptsmanagementwidget.moc"
