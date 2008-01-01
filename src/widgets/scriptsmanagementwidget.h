/**************************************************************************
*   Copyright (C) 2006 by Michel Ludwig (michel.ludwig@kdemail.net)       *
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

#include <k3listview.h>
#include <KToolBar>

class KileInfo;

namespace KileScript {
	class Manager;
	class Script;
};

namespace KileWidget {

	/**
	 * This class represents an entry in the scripts list view.
	 **/
	class ScriptListViewItem : public K3ListViewItem {
		public:
			ScriptListViewItem(QWidget *managementWidget, K3ListView *parent, KileScript::Script *script, KileInfo *kileInfo);
			virtual ~ScriptListViewItem();

			KileScript::Script* getScript();

			virtual void setText(int column, const QString & text);
			virtual QString text(int column) const;

		protected:
			KileScript::Script *m_script;
			KileInfo *m_kileInfo;
			QWidget *m_managementWidget;
	};

	/**
	 * This widget is used to handle Kile's scripting functionality.
	 **/
	class ScriptsManagement : public QWidget {
		Q_OBJECT
		public:
			ScriptsManagement(KileInfo *kileInfo, QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
			~ScriptsManagement();
	
// 		Q_SIGNALS:

		public Q_SLOTS:
			/**
			 * Rebuilds the list view.
			 **/
			void updateListView();
			

		protected Q_SLOTS:
			/**
			 * Opens the currently selected script in Kile's editing area.
			 **/
			void openSelectedScript();

			/**
			 * Executes the currently selected script.
			 **/
			void executeSelectedScript();
// 			void configureSelectedShortcut();

			void updateButtonPanel();

		protected:
			KileInfo *m_kileInfo;
			K3ListView *m_scriptsListView;

		private:
			int m_newButton;
			int m_executeButton;
			int m_openButton;
			int m_refreshButton;
			KToolBar *m_toolbar;
	};

}
#endif

