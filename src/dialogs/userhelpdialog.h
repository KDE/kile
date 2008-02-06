/***************************************************************************
                           userhelpdialog.h
----------------------------------------------------------------------------
    date                 : Jul 22 2005
    version              : 0.20
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERHELPDIALOG_H
#define USERHELPDIALOG_H

#include <KDialog>

#include <QStringList>

#include <KLineEdit>

class QListWidget;

class KLineEdit;
class KPushButton;

namespace KileDialog
{

class UserHelpDialog : public KDialog
{
		Q_OBJECT

	public:
		UserHelpDialog(QWidget *parent = 0, const char *name = 0);
		~UserHelpDialog() {}

		void setParameter(const QStringList &menuentries, const QStringList &helpfiles);
		void getParameter(QStringList &userhelpmenulist, QStringList &userhelpfilelist);
	private:
		QListWidget *m_menulistbox;
		KLineEdit *m_fileedit;
		KPushButton *m_add, *m_remove, *m_addsep, *m_up, *m_down;

		QStringList m_filelist;

		void updateButton();

	private Q_SLOTS:
		void slotChange();
		void slotAdd();
		void slotRemove();
		void slotAddSep();
		void slotUp();
		void slotDown();
};

class UserHelpAddDialog : public KDialog
{
		Q_OBJECT

	public:
		UserHelpAddDialog(QListWidget *menulistbox, QWidget *parent = 0);
		~UserHelpAddDialog() {}

	private:
		KLineEdit *m_leMenuEntry, *m_leHelpFile;
		KPushButton *m_pbChooseFile, *m_pbChooseHtml;
		QListWidget *m_menulistbox;

	public:
		QString getMenuitem() {
			return m_leMenuEntry->text();
		}
		QString getHelpfile() {
			return m_leHelpFile->text();
		}

	private Q_SLOTS:
		void slotChooseFile();
		void slotChooseHtml();
		void slotOk();
};

}

#endif
