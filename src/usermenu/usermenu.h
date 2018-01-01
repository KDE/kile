/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2017 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERMENU_H
#define USERMENU_H

#include <QList>
#include <QDomDocument>

#include <KProcess>
#include <KActionCollection>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "kileinfo.h"

#include "usermenu/usermenudata.h"


namespace KileMenu {

class UserMenu : public QObject
{
    Q_OBJECT

public:
    enum MenuLocation { StandAloneLocation = 0x00, LaTeXMenuLocation = 0x01 };

    UserMenu(KileInfo *ki, QObject *receiver);
    ~UserMenu();

    void installXmlMenufile();
    void removeXmlMenufile();
    QString xmlFile() const {
        return m_currentXmlFile;
    }
    bool isEmpty();

    bool installXml(const QString &filename);
    void refreshActionProperties();
    void removeShortcuts();

    void updateGUI();
    void updateKeyBindings();

    QMenu* getMenuItem();

    inline MenuLocation getUserMenuLocation() const
    {
        switch(KileConfig::userMenuLocation()) {
        case 0:
            return StandAloneLocation;
        default:
            return LaTeXMenuLocation;
        }
    }

    QList<QAction *> contextMenuActions() const {
        return m_actionlistContextMenu;
    }
    QList<QAction *> menuActions() const {
        return m_actionlist;
    }

    static QString selectUserMenuDir();

public Q_SLOTS:
    void installXmlFile(const QString &filename);
    void removeXmlFile();

private Q_SLOTS:
    void slotUserMenuAction();
    void slotProcessOutput();
    void slotProcessExited(int /* exitCode */, QProcess::ExitStatus exitStatus);

Q_SIGNALS:
    void sendText(const QString&);
    void triggered();
    void updateStatus();

private:
    KileInfo *m_ki;
    QObject * m_receiver;
    QString m_currentXmlFile;

    QAction *m_wizardAction1, *m_latexAction1;
    QAction *m_wizardAction2, *m_latexAction2;
    QMenu *m_latexMenuEntry;

    int m_actionsContextMenu;
    QList<QAction *> m_actionlistContextMenu;
    QList<QAction *> m_actionlist;
    QList<UserMenuData> m_menudata;

    KActionCollection *m_actioncollection;

    KProcess *m_proc;
    QString m_procOutput;
    KTextEditor::View *m_procView;
    const UserMenuData *m_procMenudata;

    void addSpecialActionsToMenus();
    void updateUsermenuPosition();
    void setStandAloneMenuVisible(bool state, bool show);
    QAction *createAction(const QString &name);
    void clear();

    void installXmlSubmenu(const QDomElement &element, QMenu *parentmenu, int &actionnumber);
    void installXmlMenuentry(const QDomElement &element, QMenu *parentmenu, int &actionnumber);

    void removeActionProperties();
    void updateXmlFile(const QString &filename);
    bool updateXmlSubmenu(QDomDocument &doc, QDomElement &element, int &actionnumber);
    bool updateXmlMenuentry(QDomDocument &doc, QDomElement &element, int &actionnumber);

    void execActionText(KTextEditor::View *view, const UserMenuData &menudata);
    void execActionFileContent(KTextEditor::View *view, const UserMenuData &menudata);
    void execActionProgramOutput(KTextEditor::View *view, const UserMenuData &menudata);

    void insertText(KTextEditor::View *view, const QString &text, bool replaceSelection, bool selectInsertion);
    bool str2bool(const QString &value);

};

}

#endif
