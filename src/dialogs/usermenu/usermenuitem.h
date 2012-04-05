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

#ifndef LATEXMENUITEM_H
#define LATEXMENUITEM_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QIcon>

#include <KLocale>
#include <KShortcut>

#include "usermenu/usermenudata.h"

#define EMPTY_MENUENTRY    i18n("???")
#define EMPTY_SUBMENU      i18n("  >")
#define LENGTH_SUBSTITUTE  3

namespace KileMenu {

////////////////////////////// UserMenuItem //////////////////////////////

class UserMenuItem : public QTreeWidgetItem {

	public:

		UserMenuItem(UserMenuData::MenuType type, const QString &menutitle = QString());
		UserMenuItem(QTreeWidget *parent, QTreeWidgetItem *preceding,
		              UserMenuData::MenuType type, const QString &menutitle = QString());
		UserMenuItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceding,
		             UserMenuData::MenuType type, const QString &menutitle = QString());

		virtual ~UserMenuItem() {}

		enum ModelUserError { MODEL_ERROR_NONE=0x00,
		                      MODEL_ERROR_EMPTY=0x01,
		                      MODEL_ERROR_SUBMENU=0x02,
		                      MODEL_ERROR_TEXT=0x04,
		                      MODEL_ERROR_FILE_EMPTY=0x08,
		                      MODEL_ERROR_FILE_EXIST=0x10,
		                      MODEL_ERROR_FILE_EXECUTABLE=0x20,
		                    };

		void setModelData(bool executable=false);
		QString updateMenutitle();

		void setMenutype(UserMenuData::MenuType type) { m_data.menutype = type; }
		UserMenuData::MenuType menutype() { return m_data.menutype; }

		void setMenutitle(const QString &s) { m_data.menutitle = s; }
		const QString &menutitle() { return m_data.menutitle; }

		void setMenuicon(const QString &icon) { m_data.icon = icon; }
		QString menuicon() { return m_data.icon; }

		void setFilename(const QString &filename) { m_data.filename = filename; }
		const QString &filename() { return m_data.filename; }

		void setParameter(const QString &parameter) { m_data.parameter = parameter; }
		const QString &parameter() { return m_data.parameter; }

		void setPlaintext(const QString &text) { m_data.text = text; }
		const QString &plaintext() { return m_data.text; }

		void setShortcut(const QString &shortcut) { m_data.shortcut = shortcut; }
		QString shortcut() { return m_data.shortcut; }

		// checkboxes
		void setNeedsSelection(bool state) { m_data.needsSelection = state; }
		bool needsSelection() { return m_data.needsSelection; }

		void setUseContextMenu(bool state) { m_data.useContextMenu = state; }
		bool useContextMenu() { return m_data.useContextMenu; }

		void setReplaceSelection(bool state) { m_data.replaceSelection = state; }
		bool replaceSelection() { return m_data.replaceSelection; }

		void setSelectInsertion(bool state) { m_data.selectInsertion = state; }
		bool selectInsertion() { return m_data.selectInsertion; }

		void setInsertOutput(bool state) { m_data.insertOutput = state; }
		bool insertOutput() { return m_data.insertOutput; }

	private:
		UserMenuData  m_data;

		void clear();
		void initItem(UserMenuData::MenuType type, const QString &menutitle);

};


}

#endif
