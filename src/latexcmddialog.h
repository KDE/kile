/***************************************************************************
                         latexcmddialog.h
                         --------------
    date                 : Jul 25 2005
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


#ifndef LATEXCMDDIALOG_H
#define LATEXCMDDIALOG_H

#include <KDialog>

#include <QMap>

#include "latexcmd.h"

class Q3ListView;
class QCheckBox;
class QComboBox;
class QLabel;
class QTabWidget;

class K3ListView;
class K3ListViewItem;
class KConfig;
class KLineEdit;
class KPushButton;
class KTabWidget;

namespace KileDialog
{

class NewLatexCommand : public KDialog
{
		Q_OBJECT

	public:
		NewLatexCommand(QWidget *parent, const QString &caption,
										const QString &groupname, K3ListViewItem *lvitem,
										KileDocument::CmdAttribute cmdtype, QMap<QString, bool> *dict);
		~NewLatexCommand() {}
		void getParameter(QString &name, KileDocument::LatexCmdAttributes &attr);

	private:
		KLineEdit *m_edName;
		QCheckBox *m_chStarred, *m_chEndofline, *m_chMath;
		QComboBox *m_coTab, *m_coOption, *m_coParameter;

		bool m_addmode, m_envmode;
		bool m_useMathOrTab, m_useOption, m_useParameter;
		KileDocument::CmdAttribute m_cmdType;
		QMap<QString, bool> *m_dict;

	private Q_SLOTS:
		void slotOk();
};


class LatexCommandsDialog : public KDialog
{
		Q_OBJECT

	public:
		LatexCommandsDialog(KConfig *config, KileDocument::LatexCommands *commands, QWidget *parent = 0, const char *name = 0);
		~LatexCommandsDialog() {}

		//enum EnvParameter { envName,envStarred,envEOL,envMath,envTab,envOption };

	private:
		enum LVmode { lvEnvMode = 1, lvCmdMode = 2 };

		KConfig *m_config;
		KileDocument::LatexCommands *m_commands;
		QMap<QString, bool> m_dictCommands;
		bool m_commandChanged;

		K3ListView *m_lvEnvironments, *m_lvCommands;
		K3ListViewItem *m_lviList, *m_lviTabular, *m_lviMath, *m_lviAmsmath, *m_lviVerbatim;
		K3ListViewItem *m_lviLabels, *m_lviReferences, *m_lviCitations;
		K3ListViewItem *m_lviInputs;
		QTabWidget *m_tab;
		KPushButton *m_btnAdd, *m_btnDelete, *m_btnEdit;
		QCheckBox *m_cbUserDefined;

		void resetListviews();
		LVmode getListviewMode();
		KileDocument::CmdAttribute getCommandMode(K3ListViewItem *item);
		bool isParentItem(K3ListViewItem *item);

		void setEntry(K3ListViewItem *parent, const QString &name, KileDocument::LatexCmdAttributes &attr);
		void getEntry(K3ListViewItem *item, KileDocument::LatexCmdAttributes &attr);

		bool isUserDefined(const QString &name);
		bool hasUserDefined(K3ListView *listview);

		void resetEnvironments();
		void resetCommands();
		void getListviewStates(bool states[]);
		void setListviewStates(bool states[]);

		void readConfig();
		void writeConfig(K3ListView *listview, const QString &groupname, bool env);

	private Q_SLOTS:
		void slotPageChanged(QWidget *);
		void slotEnableButtons();
		void slotAddClicked();
		void slotDeleteClicked();
		void slotEditClicked();
		void slotUserDefinedClicked();
		void slotButtonClicked(int button);
};

}

#endif
