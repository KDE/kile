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


#include "latexmenu/latexmenudata.h"

#include "kiledebug.h"


namespace KileMenu {

LatexmenuData::LatexmenuData()
{
	clear();
}

void LatexmenuData::clear()
{
	menutype  = Text;
	menutitle = QString::null;
	filename  = QString::null;
	parameter = QString::null;
	text      = QString::null;
	icon      = QString::null;
	shortcut  = QString::null;

	needsSelection   = false;
	useContextMenu   = false;
	replaceSelection = false;
	insertOutput     = false;
	selectInsertion  = false;
}

// static list for xml menu attributes
QStringList LatexmenuData::xmlMenuAttrList = QStringList() << "text" << "file" << "program" << "separator" << "submenu";

// static list for xml menu tags
QStringList LatexmenuData::xmlMenuTagList = QStringList() << "text" << "filename" << "parameter"
	                                                       << "icon" << "shortcut"
	                                                       << "needsSelection"     << "useContextMenu" << "replaceSelection"
	                                                       << "selectInsertion"    << "insertOutput"   << "title";

// static methods  for xml menu attributes
LatexmenuData::MenuType LatexmenuData::xmlMenuType(const QString &name)
{
	int index = xmlMenuAttrList.indexOf(name);
	return ( index >= 0 ) ? (LatexmenuData::MenuType)index : LatexmenuData::Text;
}

QString LatexmenuData::xmlMenuTypeName(int index)
{
	return xmlMenuAttrList[index];
}

// static methods  for xml menu tags
int LatexmenuData::xmlMenuTag(const QString &tag)
{
	return xmlMenuTagList.indexOf(tag);
}

QString LatexmenuData::xmlMenuTagName(int index)
{
	return xmlMenuTagList[index];
}


}

