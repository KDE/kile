/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QFile>
#include <QFileInfo>

#include "dialogs/usermenu/usermenuitem.h"


namespace KileMenu {

UserMenuItem::UserMenuItem(UserMenuData::MenuType type, const QString &menutitle)
	: QTreeWidgetItem()
{
	initItem(type,menutitle);
}

UserMenuItem::UserMenuItem( QTreeWidget *parent, QTreeWidgetItem *preceding, UserMenuData::MenuType type, const QString &menutitle)
	: QTreeWidgetItem(parent,preceding)
{
	initItem(type,menutitle);
}

UserMenuItem::UserMenuItem( QTreeWidgetItem *parent, QTreeWidgetItem *preceding, UserMenuData::MenuType type, const QString &menutitle)
	: QTreeWidgetItem(parent,preceding)
{
	initItem(type,menutitle);
}

void UserMenuItem::initItem(UserMenuData::MenuType type, const QString &menutitle)
{
	clear();
	setText(0,menutitle);

	m_data.menutitle = menutitle;
	m_data.menutype = type;

	setData(0, Qt::UserRole+1, UserMenuData::xmlMenuTypeName(type));
	setData(0, Qt::UserRole+2, MODEL_ERROR_NONE);
}

// check for possible errors and save for use with model data
void UserMenuItem::setModelData(bool executable)
{
	int modelerror = MODEL_ERROR_NONE;

	if ( m_data.menutitle.isEmpty() && m_data.menutype!=UserMenuData::Separator ) {
		modelerror |= UserMenuItem::MODEL_ERROR_EMPTY;
	}

	if ( m_data.menutype==UserMenuData::Submenu && childCount()==0 ) {
		modelerror |= UserMenuItem::MODEL_ERROR_SUBMENU;
	}
	else if ( m_data.menutype==UserMenuData::Text && m_data.text.isEmpty() ) {
		modelerror |= UserMenuItem::MODEL_ERROR_TEXT;
	}
	else if ( m_data.menutype == UserMenuData::FileContent ) {
		if ( m_data.filename.isEmpty() ) {
			modelerror |= UserMenuItem::MODEL_ERROR_FILE_EMPTY;
		}
		else if ( !QFile::exists(m_data.filename) ) {
			modelerror |= UserMenuItem::MODEL_ERROR_FILE_EXIST;
		}
	}
	else if ( m_data.menutype == UserMenuData::Program && !executable ) {
		modelerror |= UserMenuItem::MODEL_ERROR_FILE_EXECUTABLE;
	}

	setData(0,Qt::UserRole+2,modelerror);
}

// two possible errors in the menutree are made visible for the user
//  - if no menutitle is given, it is changed to '???'
//  - if a (useless) submenu with no children is given, the menutitle 'title' is changed to 'title >'
QString UserMenuItem::updateMenutitle()
{
	QString menutitle = m_data.menutitle;
	if ( menutitle.isEmpty() ) {
		menutitle = EMPTY_MENUENTRY;
	}
	else if ( m_data.menutype==UserMenuData::Submenu && childCount()==0 ) {
		menutitle += EMPTY_SUBMENU;
	}
	return menutitle;
}


void UserMenuItem::clear()
{
	m_data.clear();
}


}
