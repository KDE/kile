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
#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegateFactory>

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
    KConfigGroup configGroup = config->group(QStringLiteral("UserHelp"));
    int entries = configGroup.readEntry(QStringLiteral("entries"), int(0));
    for(int i = 0; i < entries; ++i) {
        QString menu = configGroup.readEntry(QStringLiteral("menu%1").arg(i));
        menuList << menu;
        if (!menu.isEmpty() && menu != QStringLiteral("-")) {
            fileList << configGroup.readEntry(QStringLiteral("file%1").arg(i), QUrl());
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
    config->deleteGroup(QStringLiteral("UserHelp"));

    // then write new entries
    KConfigGroup configGroup = config->group(QStringLiteral("UserHelp"));
    configGroup.writeEntry(QStringLiteral("entries"), entries);
    for(int i = 0; i < entries; ++i) {
        QString menu = menuList[i];
        configGroup.writeEntry(QStringLiteral("menu%1").arg(i), menu);
        if (menu != QStringLiteral("-")) {
            configGroup.writeEntry(QStringLiteral("file%1").arg(i), fileList[i]);
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
        if (menu == QStringLiteral("-")) {
            QAction *action = m_userHelpActionMenu->addSeparator();
            m_actionList.append(action);
        }
        else {
            QUrl url = *j;

            // some file types have an icon
            QFileInfo fi(url.fileName());
            QString ext = fi.suffix();
            if (ext == QStringLiteral("htm")) {
                ext = QStringLiteral("html");
            }
            KileAction::VariantSelection *action = new KileAction::VariantSelection(menu, QVariant::fromValue(url), this);
            if (!url.isLocalFile() ||  ext == QStringLiteral("html") || ext == QStringLiteral("dvi") || ext == QStringLiteral("ps") || ext == QStringLiteral("pdf")) {
                QString icon = (!url.isLocalFile()) ? QStringLiteral("viewhtml") : QStringLiteral("view") + ext;
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
        if (ext == QStringLiteral("dvi")) {
            type = QStringLiteral("ViewDVI");
        }
        else if (ext == QStringLiteral("ps")) {
            type = QStringLiteral("ViewPS");
        }
        else if (ext == QStringLiteral("pdf")) {
            type = QStringLiteral("ViewPDF");
        }
        else if (ext == QStringLiteral("html") || ext == QStringLiteral("htm")) {
            type = QStringLiteral("ViewHTML");
        }
    }

    KileTool::Base *tool = nullptr;
    if (!type.isEmpty() && type != QStringLiteral("ViewHTML")) {
        tool = m_manager->createTool(type, QStringLiteral("Okular"), false);
    }
    if(tool) {
        tool->setFlags(0);
        tool->setSource(url.toLocalFile());
        m_manager->run(tool);
    }
    else {
        auto job = new KIO::OpenUrlJob(url, m_mainWindow);
        job->setUiDelegate(KIO::createDefaultJobUiDelegate());
        job->start();
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

