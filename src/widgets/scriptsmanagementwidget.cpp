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

#include "widgets/scriptsmanagementwidget.h"

#include <QLayout>
#include <QList>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>

#include <KIconLoader>
#include <KInputDialog>
#include <KLocale>
#include <KMessageBox>

#include "editorkeysequencemanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "scriptmanager.h"

/*
 * We do not use the 'watchedKeySequencesChanged' signal as it creates lots of problems with the setKeySequences... methods.
 * This works fine as this class is the only one modifying the key sequences.
 */

namespace KileWidget {

/**
* This class represents an entry in the scripts tree widget.
**/
class ScriptListItem : public QTreeWidgetItem {
	public:
		ScriptListItem(QTreeWidget *parent, KileScript::Script *script) : QTreeWidgetItem(parent), m_script(script)
		{
		}

		virtual ~ScriptListItem()
		{
		}

		KileScript::Script* getScript()
		{
			return m_script;
		}

	protected:
		KileScript::Script *m_script;
};

ScriptsManagement::ScriptsManagement(KileInfo *kileInfo, QWidget *parent, const char *name, Qt::WFlags f)
 : QWidget(parent, f), m_kileInfo(kileInfo) {
	setObjectName(name);
	QVBoxLayout *baseLayout = new QVBoxLayout(this);
	baseLayout->setMargin(0);
	setLayout(baseLayout);

	m_toolBar = new KToolBar(this, "scriptControlToolBar");
	m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_toolBar->setIconDimensions(KIconLoader::SizeSmall);

	m_runAction = new KAction(this);
	m_runAction->setIcon(SmallIcon("run-build"));
	m_runAction->setText(i18n("Run Selected Script"));
	connect(m_runAction, SIGNAL(triggered()), this, SLOT(executeSelectedScript()));
	m_toolBar->addAction(m_runAction);

	KAction *action = new KAction(this);
	action->setIcon(SmallIcon("scriptnew"));
	action->setText(i18n("Create New Script"));
	connect(action, SIGNAL(triggered()), kileInfo->docManager(), SLOT(fileNewScript()));
	m_toolBar->addAction(action);

	m_scriptOpenAction = new KAction(this);
	m_scriptOpenAction->setIcon(SmallIcon("scriptopen"));
	m_scriptOpenAction->setText(i18n("Open Selected Script in Editor"));
	connect(m_scriptOpenAction, SIGNAL(triggered()), this, SLOT(openSelectedScript()));
	m_toolBar->addAction(m_scriptOpenAction);

	m_configureKeySequenceAction = new KAction(this);
	m_configureKeySequenceAction->setIcon(SmallIcon("configure-shortcuts"));
	m_configureKeySequenceAction->setText(i18n("Configure Key Sequence"));
	connect(m_configureKeySequenceAction, SIGNAL(triggered()), this, SLOT(configureSelectedKeySequence()));
	m_toolBar->addAction(m_configureKeySequenceAction);

	m_removeKeySequenceAction = new KAction(this);
	m_removeKeySequenceAction->setIcon(SmallIcon("edit-delete"));
	m_removeKeySequenceAction->setText(i18n("Remove Key Sequence"));
	connect(m_removeKeySequenceAction, SIGNAL(triggered()), this, SLOT(removeSelectedKeySequence()));
	m_toolBar->addAction(m_removeKeySequenceAction);

	action = new KAction(this);
	action->setIcon(SmallIcon("view-refresh"));
	action->setText(i18n("Refresh List"));
	connect(action, SIGNAL(triggered()), m_kileInfo->scriptManager(), SLOT(scanScriptDirectories()));
	m_toolBar->addAction(action);

	baseLayout->addWidget(m_toolBar);
	m_treeWidget = new QTreeWidget(this);
	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->setColumnCount(2);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);
	QStringList headerLabels;
	headerLabels.push_back(i18n("Script Name"));
	headerLabels.push_back(i18n("Sequence"));
	m_treeWidget->setHeaderLabels(headerLabels);
	m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_treeWidget->setRootIsDecorated(false);
	connect(m_kileInfo->scriptManager(), SIGNAL(scriptsChanged()), this, SLOT(update()));
// 	connect(m_treeWidget, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), this, SLOT(executed(QListViewItem*, const QPoint&, int)));
// 	connect(m_treeWidget, SIGNAL(itemRenamed(QListViewItem*, const QString&, int)), this, SLOT(itemRenamed(QListViewItem*, const QString&, int)));
	connect(m_treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtonPanel()));

	baseLayout->addWidget(m_treeWidget);
	updateButtonPanel();
	update();
}

ScriptsManagement::~ScriptsManagement() {
}


void ScriptsManagement::update() {
	m_treeWidget->clear();
	QList<KileScript::Script*> scriptList = m_kileInfo->scriptManager()->getScripts();
	QList<QTreeWidgetItem*> childrenList;
	for(QList<KileScript::Script*>::iterator i = scriptList.begin(); i != scriptList.end(); ++i) {
		ScriptListItem *item = new ScriptListItem(m_treeWidget, *i);
		item->setText(0, (*i)->getName());
		item->setText(1, (*i)->getKeySequence());
 		childrenList.push_back(item);
	}
	m_treeWidget->addTopLevelItems(childrenList);
}

void ScriptsManagement::openSelectedScript() {
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	KileScript::Script *script = static_cast<ScriptListItem*>(selectedItems.first())->getScript();
	m_kileInfo->docManager()->fileOpen(script->getFileName());
}

void ScriptsManagement::executeSelectedScript() {
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	KileScript::Script *script = static_cast<ScriptListItem*>(selectedItems.first())->getScript();
	m_kileInfo->scriptManager()->executeScript(script);
}

void ScriptsManagement::configureSelectedKeySequence() {
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	KileScript::Script *script = static_cast<ScriptListItem*>(selectedItems.first())->getScript();
	// better not use the QTreeWidgetItem as it can be destroyed as a side effect of the setEditorKeySequence method.
	QString oldSequence = script->getKeySequence();
	bool ok;
	QString value = KInputDialog::getText(i18n("New Key Sequence"), i18n("Please enter a new key sequence:"), oldSequence, &ok, m_kileInfo->mainWindow());
	if(!ok) {
		return;
	}

	if(value.isEmpty()) {
		m_kileInfo->scriptManager()->removeEditorKeySequence(script);
	}
	else if(value == oldSequence || (value.isEmpty() && oldSequence.isEmpty())) {
		return;
	}
	else {
		QPair<int, QString> pair = m_kileInfo->editorKeySequenceManager()->checkSequence(value, oldSequence);
		if(pair.first == 0) {
			m_kileInfo->scriptManager()->setEditorKeySequence(script, value);
		}
		KileEditorKeySequence::Action *action = m_kileInfo->editorKeySequenceManager()->getAction(pair.second);
		QString description = (!action) ? QString() : action->getDescription();
		switch(pair.first) {
			case 1:
				KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The sequence \"%1\" is already assigned to the action \"%2\"", value, description), i18n("Sequence Already Assigned"));
				return;
			case 2:
				KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The sequence \"%1\" is a subsequence of \"%2\", which is already assigned to the action \"%3\"", value, pair.second, description), i18n("Sequence Already Assigned"));
				return;
			case 3:
				KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The shorter sequence \"%1\" is already assigned to the action \"%2\"", pair.second, description), i18n("Sequence Already Assigned"));
				return;
		}
		m_kileInfo->scriptManager()->setEditorKeySequence(script, value);
	}
	QTimer::singleShot(0, this, SLOT(update()));
}

void ScriptsManagement::removeSelectedKeySequence()
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	KileScript::Script *script = static_cast<ScriptListItem*>(selectedItems.first())->getScript();
	// better not use the QTreeWidgetItem as it can be destroyed as a side effect of the removeEditorKeySequence method.
	m_kileInfo->scriptManager()->removeEditorKeySequence(script);
	QTimer::singleShot(0, this, SLOT(update()));
}

void ScriptsManagement::updateButtonPanel() {
	bool b = m_treeWidget->selectionModel()->hasSelection();
	m_runAction->setEnabled(b);
	m_scriptOpenAction->setEnabled(b);
	m_configureKeySequenceAction->setEnabled(b);
	m_removeKeySequenceAction->setEnabled(b);
}

}

#include "scriptsmanagementwidget.moc"
