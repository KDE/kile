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

#ifndef USERMENUDATA_H
#define USERMENUDATA_H

#include <QStringList>

namespace KileMenu {

class UserMenuData {

	public:

		enum MenuType { Text=0x00, FileContent=0x01, Program=0x02, Separator=0x03, Submenu=0x04 };

		enum XmlMenuTag {  XML_PLAINTEXT        = 0x00,
		                   XML_FILENAME         = 0x01,
		                   XML_PARAMETER        = 0x02,
		                   XML_ICON             = 0x03,
		                   XML_SHORTCUT         = 0x04,
		                   XML_NEEDSSELECTION   = 0x05,
		                   XML_USECONTEXTMENU   = 0x06,
		                   XML_REPLACESELECTION = 0x07,
		                   XML_SELECTINSERTION  = 0x08,
		                   XML_INSERTOUTPUT     = 0x09,
		                   XML_TITLE            = 0x0a
		                };

		UserMenuData();
		virtual ~UserMenuData() {}

		void clear();

		MenuType    menutype;
		QString     menutitle;
		QString     filename;
		QString     parameter;
		QString     text;
		QString     icon;
		QString     shortcut;

		bool        needsSelection;
		bool        useContextMenu;
		bool        replaceSelection;
		bool        selectInsertion;
		bool        insertOutput;

		// static lists for xml menu attributes/tags
		static QStringList xmlMenuAttrList;
		static QStringList xmlMenuTagList;

		static UserMenuData::MenuType xmlMenuType(const QString &name);
		static QString xmlMenuTypeName(int index);

		static int xmlMenuTag(const QString &tag);
		static QString xmlMenuTagName(int index);

};


}

#endif
