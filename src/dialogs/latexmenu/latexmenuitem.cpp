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

#include "dialogs/latexmenu/latexmenuitem.h"


namespace KileMenu {

LatexmenuItem::LatexmenuItem(LatexmenuData::MenuType type, const QString &menutitle)
	: QTreeWidgetItem()
{
	initItem(type,menutitle);
}

LatexmenuItem::LatexmenuItem( QTreeWidget *parent, QTreeWidgetItem *preceding, LatexmenuData::MenuType type, const QString &menutitle)
	: QTreeWidgetItem(parent,preceding)
{
	initItem(type,menutitle);
}

LatexmenuItem::LatexmenuItem( QTreeWidgetItem *parent, QTreeWidgetItem *preceding, LatexmenuData::MenuType type, const QString &menutitle)
	: QTreeWidgetItem(parent,preceding)
{
	initItem(type,menutitle);
}

void LatexmenuItem::initItem(LatexmenuData::MenuType type, const QString &menutitle)
{
	clear();
	setText(0,menutitle);

	m_data.menutitle = menutitle;
	m_data.menutype = type;

	setData(0, Qt::UserRole+1, LatexmenuData::xmlMenuTypeName(type));
	setData(0, Qt::UserRole+2, MODEL_ERROR_NONE);
}

// check for possible errors and save for use with model data
void LatexmenuItem::setModelData(bool executable)
{
	int modelerror = MODEL_ERROR_NONE;

	if ( m_data.menutitle.isEmpty() && m_data.menutype!=LatexmenuData::Separator ) {
		modelerror |= LatexmenuItem::MODEL_ERROR_EMPTY;
	}

	if ( m_data.menutype==LatexmenuData::Submenu && childCount()==0 ) {
		modelerror |= LatexmenuItem::MODEL_ERROR_SUBMENU;
	}
	else if ( m_data.menutype==LatexmenuData::Text && m_data.text.isEmpty() ) {
		modelerror |= LatexmenuItem::MODEL_ERROR_TEXT;
	}
	else if ( m_data.menutype == LatexmenuData::FileContent ) {
		if ( m_data.filename.isEmpty() ) {
			modelerror |= LatexmenuItem::MODEL_ERROR_FILE_EMPTY;
		}
		else if ( !QFile::exists(m_data.filename) ) {
			modelerror |= LatexmenuItem::MODEL_ERROR_FILE_EXIST;
		}
	}
	else if ( m_data.menutype == LatexmenuData::Program && !executable ) {
		modelerror |= LatexmenuItem::MODEL_ERROR_FILE_EXECUTABLE;
	}

	setData(0,Qt::UserRole+2,modelerror);
}

// two possible errors in the menutree are made visible for the user
//  - if no menutitle is given, it is changed to '???'
//  - if a (useless) submenu with no children is given, the menutitle 'title' is changed to 'title >'
QString LatexmenuItem::updateMenutitle()
{
	QString menutitle = m_data.menutitle;
	if ( menutitle.isEmpty() ) {
		menutitle = EMPTY_MENUENTRY;
	}
	else if ( m_data.menutype==LatexmenuData::Submenu && childCount()==0 ) {
		menutitle += EMPTY_SUBMENU;
	}
	return menutitle;
}


void LatexmenuItem::clear()
{
	m_data.clear();
}


}
