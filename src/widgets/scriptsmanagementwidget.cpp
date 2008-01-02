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
#include <QToolTip>
#include <QVBoxLayout>

#include <QList>

#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "editorkeysequencemanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "scriptmanager.h"

#include <K3ListView>

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

// void ScriptListItem::setText(int column, const QString & str) {
// 	if(column == 1) {

// 	}
// }



ScriptsManagement::ScriptsManagement(KileInfo *kileInfo, QWidget *parent, const char *name, Qt::WFlags f) : QWidget(parent, name, f), m_kileInfo(kileInfo) {
	QVBoxLayout *baseLayout = new QVBoxLayout(this);
	setLayout(baseLayout);

	m_toolBar = new KToolBar(this, "scriptControlToolBar");
	m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_toolBar->setIconDimensions(KIconLoader::SizeSmall);

	KAction *action = new KAction(this);
	action->setIcon(SmallIcon("run-build"));
	action->setText(i18n("Run Selected Script"));
	connect(action, SIGNAL(triggered()), this, SLOT(executeSelectedScript()));
	m_toolBar->addAction(action);

	action = new KAction(this);
	action->setIcon(SmallIcon("scriptnew"));
	action->setText(i18n("Create New Script"));
	connect(action, SIGNAL(triggered()), m_kileInfo->docManager(), SLOT(createNewScript()));
	m_toolBar->addAction(action);

	action = new KAction(this);
	action->setIcon(SmallIcon("scriptopen"));
	action->setText(i18n("Open Selected Script in Editor"));
	connect(action, SIGNAL(triggered()), this, SLOT(openSelectedScript()));
	m_toolBar->addAction(action);

	action = new KAction(this);
	action->setIcon(SmallIcon("configure-shortcuts"));
	action->setText(i18n("Configure Key Sequence"));
	connect(action, SIGNAL(triggered()), this, SLOT(configureSelectedShortcut()));
	m_toolBar->addAction(action);

	action = new KAction(this);
	action->setIcon(SmallIcon("edit-delete"));
	action->setText(i18n("Remove Key Sequence"));
	connect(action, SIGNAL(triggered()), this, SLOT(removeSelectedShortcut()));
	m_toolBar->addAction(action);

	action = new KAction(this);
	action->setIcon(SmallIcon("view-refresh"));
	action->setText(i18n("Refresh List"));
	connect(action, SIGNAL(triggered()), m_kileInfo->scriptManager(), SLOT(scanScriptDirectories()));
	m_toolBar->addAction(action);

	baseLayout->addWidget(m_toolBar);
	m_treeWidget = new QTreeWidget(this);
	m_treeWidget->setColumnCount(2);
	QStringList headerLabels;
	headerLabels.push_back(i18n("Script Name"));
	headerLabels.push_back(i18n("Sequence"));
	m_treeWidget->setHeaderLabels(headerLabels);
	m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_treeWidget->setRootIsDecorated(false);
	connect(m_kileInfo->scriptManager(), SIGNAL(jScriptsChanged()), this, SLOT(update()));
	connect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), this, SLOT(update()));
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
	ScriptListItem *item = static_cast<ScriptListItem*>(selectedItems.first());
	m_kileInfo->docManager()->fileOpen(item->getScript()->getFileName());
}

void ScriptsManagement::executeSelectedScript() {
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	ScriptListItem *item = static_cast<ScriptListItem*>(selectedItems.first());
	m_kileInfo->scriptManager()->executeScript(item->getScript());
}

void ScriptsManagement::configureSelectedShortcut() {
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	ScriptListItem *item = static_cast<ScriptListItem*>(selectedItems.first());
	QString sequence = determineKeySequence(item->getScript());
	m_kileInfo->scriptManager()->setEditorKeySequence(item->getScript(), sequence);
}

void ScriptsManagement::removeSelectedShortcut()
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if(selectedItems.isEmpty()) {
		return;
	}
	ScriptListItem *item = static_cast<ScriptListItem*>(selectedItems.first());
	m_kileInfo->scriptManager()->removeEditorKeySequence(item->getScript());
}

void ScriptsManagement::updateButtonPanel() {
// 	bool b = !(m_treeWidget->selectedItem() == NULL);
#ifdef __GNUC__
#warning Still things left to be ported!
#endif
/*
	m_toolBar->setItemEnabled(m_executeButton, b);
	m_toolBar->setItemEnabled(m_openButton, b);
*/
}

QString ScriptsManagement::determineKeySequence(KileScript::Script* script)
{
	QString oldSequence = script->getKeySequence();
	bool ok;
	QString value = KInputDialog::getText(i18n("New Key Sequence"), i18n("Please enter a new key sequence:"), oldSequence, &ok, m_kileInfo->mainWindow());
	if(!ok) {
		return oldSequence;
	}

	if(value.isEmpty()) {
#ifdef __GNUC__
#warning Check whether that's still the case!
#endif
		// don't reload the list view here as this may cause a segfault (for example on x86_64)
		QObject::disconnect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), this, SLOT(update()));
		m_kileInfo->scriptManager()->removeEditorKeySequence(script);
		QObject::connect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), this, SLOT(update()));
	}
	else if(value == oldSequence || (value.isEmpty() && oldSequence.isEmpty())) {
		return oldSequence;
	}
	else {
		QPair<int, QString> pair = m_kileInfo->editorKeySequenceManager()->checkSequence(value, oldSequence);
		if(pair.first == 0) {
#ifdef __GNUC__
#warning Check whether that's still the case!
#endif
			// don't reload the list view here as this may cause a segfault (for example on x86_64)
			QObject::disconnect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()),this, SLOT(update()));
			m_kileInfo->scriptManager()->setEditorKeySequence(script, value);
			QObject::connect(m_kileInfo->editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), this, SLOT(update()));
			return oldSequence; // leaving !
		}
		KileEditorKeySequence::Action *action = m_kileInfo->editorKeySequenceManager()->getAction(pair.second);
		QString description = (!action) ? QString() : action->getDescription();
		switch(pair.first) {
			case 1:
				KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The sequence \"%1\" is already assigned to the action \"%2\"", value, description), i18n("Sequence Already Assigned"));
				break;
			case 2:
				KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The sequence \"%1\" is a subsequence of \"%2\", which is already assigned to the action \"%3\"", value, pair.second, description), i18n("Sequence Already Assigned"));
				break;
			case 3:
				KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The shorter sequence \"%1\" is already assigned to the action \"%2\"", pair.second, description), i18n("Sequence Already Assigned"));
				break;
		}
		return oldSequence;
	}

	return value;
}

}

#include "scriptsmanagementwidget.moc"
