/***************************************************************************
    begin                : Oct 03 2011
    author               : dani
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LATEXMENU_H
#define LATEXMENU_H

#include <QList>
#include <QDomDocument>

#include <KProcess>
#include <KActionCollection>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "kileinfo.h"

#include "latexmenu/latexmenudata.h"


namespace KileMenu {

class LatexUserMenu : public QObject
{
	Q_OBJECT

	public:
		enum MenuPosition { DaniMenuPosition=0x00, LatexMenuPosition=0x01 };
		
		LatexUserMenu(KileInfo *ki, QObject *receiver);
		~LatexUserMenu();

		void installXmlMenufile();
		void removeXmlMenufile();
		QString xmlFile() const { return m_currentXmlFile; }
		bool isEmpty();
		
		bool installXml(const QString &filename);
		void refreshActionProperties();
		void removeShortcuts();

		void updateGui();
		void updateKeyBindings();
		
		QMenu *getMenuItem() const { return m_latexmenu; }
		
		QList<KAction *> contextMenuActions() const { return m_actionlistContextMenu; }
		QList<KAction *> menuActions() const { return m_actionlist; }
		
		static QString selectLatexmenuDir();

	public Q_SLOTS:
		void slotInstallXmlFile(const QString &filename);
		void slotRemoveXmlFile();
		void slotChangeMenuPosition(int newPosition);
		
	private Q_SLOTS:
		void slotLatexmenuAction();
		void slotProcessOutput();
		void slotProcessExited(int /* exitCode */, QProcess::ExitStatus exitStatus);

	Q_SIGNALS:
		void sendText(const QString&);
		void triggered();
		void updateStatus();

	private:
		KileInfo *m_ki;
		QObject * m_receiver;
		QString m_currentXmlFile;
		
		int m_menuPosition;
		QAction *m_wizardAction1, *m_latexAction1;
		KAction *m_wizardAction2, *m_latexAction2;
		QMenu *m_latexMenuEntry;

		int m_actionsContextMenu;
		QList<KAction *> m_actionlistContextMenu;
		QList<KAction *> m_actionlist;
		QList<LatexmenuData> m_menudata;
		
		QMenu *m_latexmenu;
		KActionCollection *m_actioncollection;

		KProcess *m_proc;
		QString m_procOutput;
		KTextEditor::View *m_procView;
		const LatexmenuData *m_procMenudata;
		
		void updateUsermenuPosition();
		void setVisibleDaniMenu(bool state, bool show);
		KAction *createAction(const QString &name);
		void clear();

		void installXmlSubmenu(const QDomElement &element, QMenu *parentmenu, int &actionnumber);
		void installXmlMenuentry(const QDomElement &element, QMenu *parentmenu, int &actionnumber);

		void removeActionProperties();
		void updateXmlFile(const QString &filename);
		bool updateXmlSubmenu(QDomDocument &doc, QDomElement &element, int &actionnumber);
		bool updateXmlMenuentry(QDomDocument &doc, QDomElement &element, int &actionnumber); 

		void execActionText(KTextEditor::View *view, const LatexmenuData &menudata);
		void execActionFileContent(KTextEditor::View *view, const LatexmenuData &menudata);
		void execActionProgramOutput(KTextEditor::View *view, const LatexmenuData &menudata);

		void insertText(KTextEditor::View *view, const QString &text, bool replaceSelection, bool selectInsertion);
		bool str2bool(const QString &value);

};

}

#endif
