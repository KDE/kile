/***************************************************************************
                          kiletoolconfigwidget.h  -  description
                             -------------------
    begin                : Sat 3-1 20:40:00 CEST 2004
    copyright            : (C) 2004 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILETOOLCONFIGWIDGET_H
#define KILETOOLCONFIGWIDGET_H

#include <qhbox.h>

#include <keditlistbox.h>

#include "kiletool.h"

class QGridLayout;
class QCheckBox;
class QSpinBox;
class QLabel;

class KListBox;
class KComboBox;
class KPushButton;
namespace KileTool { class Manager; }

namespace KileWidget
{
	class BasicTool : public QWidget
	{
		Q_OBJECT

	public:
		BasicTool(const QString & tool, KConfig *, KileTool::Config *map, QWidget *parent);

		void createProcess(const QString &);
		void createKonsole();
		void createPart();
		void createDocPart();
		void createSequence();

		void createLaTeX();
		void createViewBib();
		void createViewHTML();

	private slots:
		void setCommand(const QString &);
		void setOptions(const QString &);
		void setLibrary(const QString &);
		void setLibOptions(const QString &);
		void setClassName(const QString &);
		void setSequence(const QString &);
		void setClose(bool);
		void setTarget(const QString &);
		void setRelDir(const QString &);
		void setLaTeXCheckRoot(bool);
		void setRunLyxServer(bool);

	private:
		QString			m_tool;
		KileTool::Config	*m_map;
		QGridLayout		*m_layout;
		KConfig			*m_config;
		KComboBox		*m_cbTools;
		KEditListBox		*m_elbSequence;
	};

	class AdvancedTool : public QWidget
	{
		Q_OBJECT

	public:
		AdvancedTool(const QString & tool, KileTool::Config *map, QWidget *parent);

		void createFromTo();

	signals:
		void changed();

	public slots:
		void switchType(int);

	private slots:
		void setFrom(const QString &);
		void setTo(const QString &);
		void setClass(const QString &);

	private:
		KileTool::Config	*m_map;
		KComboBox		*m_cbType, *m_cbClasses;
		QGridLayout		*m_layout;
	};

	class QuickTool : public QWidget
	{
		Q_OBJECT

	public:
		QuickTool(KileTool::Config *map, KConfig *, QWidget *parent, const char *name = 0);

	public slots:
		void updateConfigs(const QString &);
		void up();
		void down();
		void add();
		void remove();
		void changed();

	private:
		KConfig			*m_config;
		KileTool::Config	*m_map;
		KComboBox		*m_cbTools, *m_cbConfigs;
		KPushButton		*m_pbAdd, *m_pbRemove, *m_pbUp, *m_pbDown;
		KListBox			*m_lstbSeq;
	};

	class ToolConfig : public QWidget
	{
		Q_OBJECT

	public:
		ToolConfig(KileTool::Manager *mngr, QWidget *parent, const char * name = 0);

	public slots:
		void switchTo(const QString & tool, bool save = true);
		void toggleAdvanced();
		void toggleSeparator();
		void updateToolbar();
		void updateToollist();
		void updateConfiglist();
		void selectIcon();
		void setMenu(const QString &);
		void writeConfig();
		void switchConfig(int index = -1);
		void switchConfig(const QString &);

		void newTool();
		void newConfig();
		void removeTool();
		void removeConfig();
		void writeStdConfig(const QString &, const QString &);

	private:
		KileTool::Manager	*m_manager;
		KileTool::Config	m_map;
		BasicTool			*m_basic;
		AdvancedTool		*m_advanced;
		QGridLayout		*m_layout;
		KListBox			*m_lstbTools;
		QLabel			*m_lbName, *m_lbPosition;
		KComboBox		*m_cbPredef, *m_cbMenu;
		QCheckBox		*m_ckToolbar, *m_ckSeparator;
		QString			m_current, m_icon;
		KPushButton		*m_pshbAdvanced, *m_pshbIcon;
		QSpinBox			*m_spinPosition;
		bool				m_bAdvanced;
	};
}

#endif
