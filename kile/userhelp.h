/***************************************************************************
                           userhelp.h
----------------------------------------------------------------------------
    date                 : Mar 10 2005
    version              : 0.12
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

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

#include <qwidget.h>   
#include <qpopupmenu.h>   
#include <qstringlist.h>

#include <kmenubar.h>
#include <kconfig.h>
#include <klocale.h>

#include "kiletoolmanager.h"

namespace KileHelp
{

class UserHelp: public QObject 
{
	Q_OBJECT
   
public: 
	UserHelp(KileTool::Manager *manager, KMenuBar *menubar);
	~UserHelp();
	void updateEntries(const QStringList &entries, const QStringList &files, bool save = true);
	void userHelpDialog();   

private slots:
	void slotUserHelpActivated(int index);   
	//void slotUserHelpDialog();   
	
private:
	void readConfig();
	void writeConfig();

	void setupUserHelpMenu();
	QPopupMenu *getHelpPopup();
	int getHelpIndex(QPopupMenu *popup);

	KileTool::Manager *m_manager;
	KMenuBar *m_menubar;
	
	QPopupMenu *m_helpmenu, *m_helppopup;
	int m_helpid, m_sepid;
	QStringList m_menuentries, m_helpfiles;
};   

}

#endif
