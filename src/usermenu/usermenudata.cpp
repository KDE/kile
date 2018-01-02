/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2018 by Michel Ludwig (michel.ludwig@gmail.com)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "usermenu/usermenudata.h"

#include "kiledebug.h"


namespace KileMenu {

UserMenuData::UserMenuData()
{
    clear();
}

void UserMenuData::clear()
{
    menutype  = Text;
    menutitle.clear();
    filename.clear();
    parameter.clear();
    text.clear();
    icon.clear();
    shortcut.clear();

    needsSelection   = false;
    useContextMenu   = false;
    replaceSelection = false;
    insertOutput     = false;
    selectInsertion  = false;
}

// static list for xml menu attributes
QStringList UserMenuData::xmlMenuAttrList = QStringList() << "text" << "file" << "program" << "separator" << "submenu";

// static list for xml menu tags
QStringList UserMenuData::xmlMenuTagList = QStringList() << "text" << "filename" << "parameter"
        << "icon" << "shortcut"
        << "needsSelection"     << "useContextMenu" << "replaceSelection"
        << "selectInsertion"    << "insertOutput"   << "title";

// static methods  for xml menu attributes
UserMenuData::MenuType UserMenuData::xmlMenuType(const QString &name)
{
    int index = xmlMenuAttrList.indexOf(name);
    return ( index >= 0 ) ? (UserMenuData::MenuType)index : UserMenuData::Text;
}

QString UserMenuData::xmlMenuTypeName(int index)
{
    return xmlMenuAttrList[index];
}

// static methods  for xml menu tags
int UserMenuData::xmlMenuTag(const QString &tag)
{
    return xmlMenuTagList.indexOf(tag);
}

QString UserMenuData::xmlMenuTagName(int index)
{
    return xmlMenuTagList[index];
}


// first, every backslash is replaced with the string "\\;" (backslash, followed by ':')
// then, every line feed character '\n' is replaced with the string "\\n" (backslash, followed by 'n')
QString UserMenuData::encodeLineFeed(const QString& string)
{
    QString toReturn = string;
    toReturn = toReturn.replace(QLatin1Char('\\'), QLatin1String("\\;"));
    toReturn = toReturn.replace(QLatin1Char('\n'), QLatin1String("\\n"));

    return toReturn;
}

QString UserMenuData::decodeLineFeed(const QString& string)
{
    QString toReturn = string;
    toReturn = toReturn.replace(QLatin1String("\\n"), QLatin1String("\n"));
    toReturn = toReturn.replace(QLatin1String("\\;"), QLatin1String("\\"));

    return toReturn;
}

}

