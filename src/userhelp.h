/***********************************************************************************************
                           userhelp.h
----------------------------------------------------------------------------
    date                 : Aug 17 2006
    version              : 0.15
    copyright            : (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
                               2008-2014 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <KConfig>
#include <KXmlGuiWindow>
#include <KMenuBar>
#include <KLocale>
#include <KUrl>

#include "kiletoolmanager.h"

namespace KileHelp
{

class UserHelp: public QObject
{
	Q_OBJECT

public:
	UserHelp(KileTool::Manager *manager, KXmlGuiWindow *mainWindow);
	~UserHelp();
	void userHelpDialog();
	void enableUserHelpEntries(bool state);
	void rebuildMenu();

private Q_SLOTS:
	void slotUserHelpActivated(const KUrl& url);
	//void slotUserHelpDialog();

private:
	void clearActionList();
	void readConfig(QStringList& menuList, QList<KUrl>& fileList);
	void writeConfig(const QStringList& menuList, const QList<KUrl>& fileList);

	void setupUserHelpMenu();

	KileTool::Manager *m_manager;
	KXmlGuiWindow *m_mainWindow;

	QList<QAction*> m_actionList;
};

}

#endif
