/***************************************************************************
                           userhelp.h
----------------------------------------------------------------------------
    date                 : Aug 17 2006
    version              : 0.15
    copyright            : (C) 2005-2006 by Holger Danielsson
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
#include <q3popupmenu.h>   
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
	void userHelpDialog();   
	void enableUserHelpEntries(bool state);

private slots:
	void slotUserHelpActivated(int index);   
	//void slotUserHelpDialog();   
	
private:
	void readConfig();
	void writeConfig();

	void setupUserHelpMenu();
	void expandHelpMenu();
	
	Q3PopupMenu *getHelpPopup();
	int getHelpIndex(Q3PopupMenu *popup);

	void updateEntries(const QStringList &entries, const QStringList &files, bool save = true);

	KileTool::Manager *m_manager;
	KMenuBar *m_menubar;
	
	Q3PopupMenu *m_helpmenu, *m_helppopup;
	int m_helpid, m_sepid;
	QStringList m_menuentries, m_helpfiles;
};   

}

#endif
