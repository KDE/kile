/***************************************************************************
                           cleandialog.h
----------------------------------------------------------------------------
    date                 : Jan 22 2004
    version              : 0.10
    copyright            : (C) 2004 by Holger Danielsson
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

#ifndef CLEANDIALOG_H
#define CLEANDIALOG_H

#include <kdialogbase.h>
#include <qstringlist.h>

class KListView;

/**
  *@author Holger Danielsson
  */

namespace KileDialog 
{
	class Clean : public KDialogBase
	{
	Q_OBJECT
	
	public: 
		Clean(QWidget *parent, const QString & filename, const QStringList &extlist);
		~Clean();
		const QStringList & getCleanlist();
	
	private:
		KListView		*listview;   
		QStringList	m_extlist;
	};
}

#endif
