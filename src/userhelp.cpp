/**********************************************************************************************
  Copyright (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
                2008-2017 by Michel Ludwig (michel.ludwig@kdemail.net)
 **********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "userhelp.h"

#include <QFileInfo>
#include <QMenu>

#include <KMessageBox>
#include <QUrl>
#include <KRun>

#include "kileactions.h"
#include "kileconfig.h"
#include "kiledebug.h"
#include "kilestdtools.h"
#include "dialogs/userhelpdialog.h"

namespace KileHelp
{

UserHelp::UserHelp(KileTool::Manager *manager, KActionMenu *userHelpActionMenu, QWidget* mainWindow)
    : m_manager(manager), m_userHelpActionMenu(userHelpActionMenu), m_mainWindow(mainWindow)
{
    setupUserHelpMenu();
}

UserHelp::~UserHelp()
{
    clearActionList();
}

void UserHelp::clearActionList()
{
    for(QList<QAction*>::iterator i = m_actionList.begin(); i != m_actionList.end(); ++i) {
        delete *i;
    }
    m_actionList.clear();
}

void UserHelp::readConfig(QStringList& menuList, QList<QUrl>& fileList)
{
    menuList.clear();
    fileList.clear();

    // first read all entries
    KConfig *config = m_manager->config();
    KConfigGroup configGroup = config->group("UserHelp");
    int entries = configGroup.readEntry("entries", int(0));
    for(int i = 0; i < entries; ++i) {
        QString menu = configGroup.readEntry(QString("menu%1").arg(i));
        menuList << menu;
        if(!menu.isEmpty() && menu != "-") {
            fileList << configGroup.readEntry(QString("file%1").arg(i), QUrl());
        }
        else {
            fileList << QUrl();
        }
    }
}

void UserHelp::writeConfig(const QStringList& menuList, const QList<QUrl>& fileList)
{
    //KILE_DEBUG_MAIN << "\tuserhelp: write config";
    int entries = menuList.count();

    // first delete old entries
    KConfig *config = m_manager->config();
    config->deleteGroup("UserHelp");

    // then write new entries
    KConfigGroup configGroup = config->group("UserHelp");
    configGroup.writeEntry("entries", entries);
    for(int i = 0; i < entries; ++i) {
        QString menu = menuList[i];
        configGroup.writeEntry(QString("menu%1").arg(i), menu);
        if(menu != "-") {
            configGroup.writeEntry(QString("file%1").arg(i), fileList[i]);
        }
    }
}

void UserHelp::setupUserHelpMenu()
{
    QStringList menuList;
    QList<QUrl> urlList;
    readConfig(menuList, urlList);

    clearActionList();

    m_userHelpActionMenu->setEnabled(menuList.count() > 0);
    QList<QUrl>::iterator j = urlList.begin();

    for(QStringList::iterator i = menuList.begin(); i != menuList.end(); ++i) {
        QString menu = *i;
        // first look, if this entry is a separator
        if(menu == "-" ) {
            QAction *action = m_userHelpActionMenu->addSeparator();
            m_actionList.append(action);
        }
        else {
            QUrl url = *j;

            // some file types have an icon
            QFileInfo fi(url.fileName());
            QString ext = fi.suffix();
            if(ext == "htm") {
                ext = "html";
            }
            KileAction::VariantSelection *action = new KileAction::VariantSelection(menu, QVariant::fromValue(url), this);
            if(!url.isLocalFile() ||  ext == "html" || ext == "dvi" || ext == "ps" || ext == "pdf") {
                QString icon = (!url.isLocalFile()) ? "viewhtml" : "view" + ext;
                action->setIcon(QIcon::fromTheme(icon));
            }
            connect(action, SIGNAL(triggered(QUrl)), this, SLOT(slotUserHelpActivated(QUrl)));
            m_userHelpActionMenu->addAction(action);
            m_actionList.append(action);
        }
        ++j;
    }
}

void UserHelp::enableUserHelpEntries(bool state)
{
    QStringList menuList;
    QList<QUrl> urlList;
    readConfig(menuList, urlList);
    m_userHelpActionMenu->setEnabled(state && (menuList.size() > 0));
}

void UserHelp::slotUserHelpActivated(const QUrl &url)
{
    KILE_DEBUG_MAIN << "==slotUserHelpActivated(" << url << ")============";

    // does the files exist?
    QFileInfo fi(url.toLocalFile());
    bool local = url.isLocalFile();
    if(local && !fi.exists()) {
        KMessageBox::error(m_mainWindow, i18n("The file '%1' does not exist.", url.toDisplayString()));
        return;
    }

    // show help file
    KILE_DEBUG_MAIN << "\tshow userhelpfile (" << url << ")";

    // determine, how to show the file
    QString type;
    if(local) {
        QString ext = fi.suffix();
        if(ext == "dvi") {
            type = "ViewDVI";
        }
        else if(ext == "ps") {
            type = "ViewPS";
        }
        else if(ext == "pdf") {
            type = "ViewPDF";
        }
        else if(ext == "html" || ext == "htm") {
            type = "ViewHTML";
        }
    }

    KileTool::Base *tool = Q_NULLPTR;
    if(!type.isEmpty() && type != "ViewHTML") {
        tool = m_manager->createTool(type, "Okular", false);
    }
    if(tool) {
        tool->setFlags(0);
        tool->setSource(url.toLocalFile());
        m_manager->run(tool);
    }
    else {
        new KRun(url,m_mainWindow);
    }
}

void UserHelp::userHelpDialog()
{
    QStringList menuList;
    QList<QUrl> fileList;
    readConfig(menuList, fileList);

    KileDialog::UserHelpDialog *dialog = new KileDialog::UserHelpDialog();
    dialog->setParameter(menuList, fileList);
    if(dialog->exec()) {
        //KILE_DEBUG_MAIN << "\t new userhelp entries accepted";
        dialog->getParameter(menuList, fileList);
        writeConfig(menuList, fileList);
        setupUserHelpMenu();
    }

    delete dialog;
}

}

