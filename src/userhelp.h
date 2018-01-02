/***********************************************************************************************
                           userhelp.h
----------------------------------------------------------------------------
    date                 : Aug 17 2006
    version              : 0.15
    copyright            : (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERHELP_H
#define USERHELP_H

#include <QList>
#include <QStringList>
#include <QWidget>

#include <KActionMenu>
#include <KConfig>
#include <QMenuBar>
#include <QUrl>

#include "kiletoolmanager.h"

namespace KileHelp
{

class UserHelp: public QObject
{
    Q_OBJECT

public:
    UserHelp(KileTool::Manager *manager, KActionMenu *userHelpActionMenu, QWidget *mainWindow);
    ~UserHelp();
    void userHelpDialog();
    void enableUserHelpEntries(bool state);

private Q_SLOTS:
    void slotUserHelpActivated(const QUrl &url);
    //void slotUserHelpDialog();

private:
    void clearActionList();
    void readConfig(QStringList& menuList, QList<QUrl>& fileList);
    void writeConfig(const QStringList& menuList, const QList<QUrl>& fileList);

    void setupUserHelpMenu();

    KileTool::Manager *m_manager;
    KActionMenu *m_userHelpActionMenu;
    QWidget *m_mainWindow;

    QList<QAction*> m_actionList;
};

}

#endif
