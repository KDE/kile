/**************************************************************************
*   Copyright (C) 2006-2014 by Michel Ludwig (michel.ludwig@kdemail.net)  *
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
#include "dialogs/scriptshortcutdialog.h"

#include <QLayout>
#include <QList>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>

#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>

#include "editorkeysequencemanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "scriptmanager.h"
#include "scripting/script.h"

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

ScriptsManagement::ScriptsManagement(KileInfo *kileInfo, QWidget *parent, const char *name, Qt::WindowFlags f)
    : QWidget(parent, f), m_kileInfo(kileInfo) {
    setObjectName(name);
    QVBoxLayout *baseLayout = new QVBoxLayout(this);
    baseLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(baseLayout);

    m_toolBar = new KToolBar(this, "scriptControlToolBar");
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setIconDimensions(KIconLoader::SizeSmall);

    m_runAction = new QAction(this);
    m_runAction->setIcon(QIcon::fromTheme("run-build"));
    m_runAction->setText(i18n("Run Selected Script"));
    connect(m_runAction, SIGNAL(triggered()), this, SLOT(executeSelectedScript()));
    m_toolBar->addAction(m_runAction);

    QAction *action = new QAction(this);
    action->setIcon(QIcon::fromTheme("scriptnew"));
    action->setText(i18n("Create New Script"));
    connect(action, SIGNAL(triggered()), kileInfo->docManager(), SLOT(fileNewScript()));
    m_toolBar->addAction(action);

    m_scriptOpenAction = new QAction(this);
    m_scriptOpenAction->setIcon(QIcon::fromTheme("scriptopen"));
    m_scriptOpenAction->setText(i18n("Open Selected Script in Editor"));
    connect(m_scriptOpenAction, SIGNAL(triggered()), this, SLOT(openSelectedScript()));
    m_toolBar->addAction(m_scriptOpenAction);

    m_configureKeySequenceAction = new QAction(this);
    m_configureKeySequenceAction->setIcon(QIcon::fromTheme("configure-shortcuts"));
    m_configureKeySequenceAction->setText(i18n("Configure Key Sequence"));
    connect(m_configureKeySequenceAction, SIGNAL(triggered()), this, SLOT(configureSelectedKeySequence()));
    m_toolBar->addAction(m_configureKeySequenceAction);

    m_removeKeySequenceAction = new QAction(this);
    m_removeKeySequenceAction->setIcon(QIcon::fromTheme("edit-delete"));
    m_removeKeySequenceAction->setText(i18n("Remove Key Sequence"));
    connect(m_removeKeySequenceAction, SIGNAL(triggered()), this, SLOT(removeSelectedKeySequence()));
    m_toolBar->addAction(m_removeKeySequenceAction);

    action = new QAction(this);
    action->setIcon(QIcon::fromTheme("view-refresh"));
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
// 	connect(m_treeWidget, SIGNAL(doubleClicked(QListViewItem*,QPoint,int)), this, SLOT(executed(QListViewItem*,QPoint,int)));
// 	connect(m_treeWidget, SIGNAL(itemRenamed(QListViewItem*,QString,int)), this, SLOT(itemRenamed(QListViewItem*,QString,int)));
    connect(m_treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtonPanel()));

    baseLayout->addWidget(m_treeWidget);
    updateButtonPanel();
    update();
}

ScriptsManagement::~ScriptsManagement() {
}

int ScriptsManagement::scriptNameColumnWidth() {
    return m_treeWidget->columnWidth(0);
}

void ScriptsManagement::setScriptNameColumnWidth(int width) {
    if(width > 0) {
        m_treeWidget->setColumnWidth(0, width);
    }
}

void ScriptsManagement::update() {
    m_treeWidget->clear();
    QList<KileScript::Script*> scriptList = m_kileInfo->scriptManager()->getScripts();
    QList<QTreeWidgetItem*> childrenList;
    for(QList<KileScript::Script*>::iterator i = scriptList.begin(); i != scriptList.end(); ++i) {
        int sequenceType = (*i)->getSequenceType();
        QString sequence = (*i)->getKeySequence();
        ScriptListItem *item = new ScriptListItem(m_treeWidget, *i);
        item->setText(0, (*i)->getName());
        item->setText(1, sequence);
        if ( !sequence.isEmpty() ) {
            QString icon = ( sequenceType == KileScript::Script::KEY_SHORTCUT ) ? "script-key-shortcut" : "script-key-sequence";
            item->setIcon(1, QIcon::fromTheme(icon));
        }
        else {
            item->setIcon(1, QIcon());
        }
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
    int oldType = script->getSequenceType();
    QString oldSequence = script->getKeySequence();

    // execute ScriptShortcutDialog
    KileDialog::ScriptShortcutDialog *dialog = new KileDialog::ScriptShortcutDialog(this, m_kileInfo, oldType, oldSequence);
    int result = dialog->exec();
    int newType = dialog->sequenceType();
    QString newSequence = dialog->sequenceValue();
    delete dialog;

    // cancelled or nothing to do?
    if ( result==QDialog::Rejected || (newType==oldType && newSequence==oldSequence) ) {
        return;
    }

    if ( newSequence.isEmpty() ) {
        m_kileInfo->scriptManager()->removeEditorKeySequence(script);
    }
    else {
        if ( newType == KileScript::Script::KEY_SEQUENCE ) {
            QPair<int, QString> pair = m_kileInfo->editorKeySequenceManager()->checkSequence(newSequence, oldSequence);
            if(pair.first == 0) {
                m_kileInfo->scriptManager()->setEditorKeySequence(script, newType, newSequence);
            }
            KileEditorKeySequence::Action *action = m_kileInfo->editorKeySequenceManager()->getAction(pair.second);
            QString description = (!action) ? QString() : action->getDescription();
            switch(pair.first) {
            case 1:
                KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The sequence \"%1\" is already assigned to the action \"%2\"", newSequence, description),
                                                             i18n("Sequence Already Assigned"));
                return;
            case 2:
                KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The sequence \"%1\" is a subsequence of \"%2\", which is already assigned to the action \"%3\"", newSequence, pair.second, description),
                                                             i18n("Sequence Already Assigned"));
                return;
            case 3:
                KMessageBox::sorry(m_kileInfo->mainWindow(), i18n("The shorter sequence \"%1\" is already assigned to the action \"%2\"", pair.second, description),
                                                             i18n("Sequence Already Assigned"));
                return;
            }
        }
        m_kileInfo->scriptManager()->setEditorKeySequence(script, newType, newSequence);
    }
    QTimer::singleShot(0, this, SLOT(update()));
}

void ScriptsManagement::removeSelectedKeySequence()  // <--------------------- seq oder shortcut
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

