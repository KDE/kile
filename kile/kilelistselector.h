/***************************************************************************
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KILELISTSELECTOR_H
#define KILELISTSELECTOR_H

#include <kdialogbase.h>

class KListBox;
class QStringList;

class KileListSelector : public KDialogBase
{
public:
	KileListSelector(const QStringList & list, const QString &caption, const QString &select, QWidget *parent = 0, const char * name = 0);
	~KileListSelector() {}

	int currentItem() { return m_listbox->currentItem();}

private:
	KListBox	*m_listbox;
};

#endif
