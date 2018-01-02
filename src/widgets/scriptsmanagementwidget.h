/**************************************************************************
*   Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net   *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SCRIPTSMANAGEMENTWIDGET_H
#define SCRIPTSMANAGEMENTWIDGET_H

#include <QTreeWidget>
#include <QWidget>

#include <QAction>
#include <KToolBar>

class KileInfo;

namespace KileScript {
class Manager;
class Script;
}

namespace KileWidget {

/**
 * This widget is used to control Kile's scripting features.
 **/
class ScriptsManagement : public QWidget {
    Q_OBJECT
public:
    ScriptsManagement(KileInfo *kileInfo, QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = 0);
    ~ScriptsManagement();

public:
    int scriptNameColumnWidth();
    void setScriptNameColumnWidth(int width);

public Q_SLOTS:
    /**
     * Rebuilds the view.
     **/
    void update();

protected Q_SLOTS:
    /**
     * Opens the currently selected script in Kile's editing area.
     **/
    void openSelectedScript();

    /**
     * Executes the currently selected script.
     **/
    void executeSelectedScript();

    void configureSelectedKeySequence();

    void removeSelectedKeySequence();

    void updateButtonPanel();

protected:
    KileInfo *m_kileInfo;
    QTreeWidget *m_treeWidget;

private:
    QAction *m_runAction, *m_scriptOpenAction, *m_configureKeySequenceAction, *m_removeKeySequenceAction;
    KToolBar *m_toolBar;
};

}
#endif

