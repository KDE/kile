/**************************************************************************
*   Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net   *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SCRIPTSMANAGEMENTWIDGET_H
#define SCRIPTSMANAGEMENTWIDGET_H

#include <qtoolbutton.h>
#include <QTableWidget>
#include <QWidget>

#include <QTreeWidget>
#include <KToolBar>

class KileInfo;

namespace KileScript {
	class Manager;
	class Script;
};

namespace KileWidget {

	/**
	 * This widget is used to handle Kile's scripting functionality.
	 **/
	class ScriptsManagement : public QWidget {
		Q_OBJECT
		public:
			ScriptsManagement(KileInfo *kileInfo, QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
			~ScriptsManagement();

		public Q_SLOTS:
			/**
			 * Rebuilds the view.
			 **/
			void update();
			

		protected Q_SLOTS:
			/**
			 * Opens the currently selected script in Kile's editing area.
			 **/
			void openSelectedScript();

			/**
			 * Executes the currently selected script.
			 **/
			void executeSelectedScript();

			void configureSelectedShortcut();

			void removeSelectedShortcut();

			void updateButtonPanel();

		protected:
			KileInfo *m_kileInfo;
			QTreeWidget *m_treeWidget;

		private:
			int m_newButton;
			int m_executeButton;
			int m_openButton;
			int m_refreshButton;
			KToolBar *m_toolBar;

			QString determineKeySequence(KileScript::Script* script);
	};

}
#endif

