/***************************************************************************
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

#include <qstringlist.h>

#include <keditlistbox.h>

#include "kiletool.h"

class QGridLayout;

namespace KileTool { class Manager; }
class ToolConfigWidget;
class ProcessToolConfigWidget;
class LibraryToolConfigWidget;
class QuickToolConfigWidget;
class LaTeXToolConfigWidget;
class ViewBibToolConfigWidget;

namespace KileWidget
{
	class ToolConfig : public QWidget
	{
		Q_OBJECT

		enum GeneralBasicStack { GBS_None = 1, GBS_Process, GBS_Library, GBS_DocPart, GBS_Sequence, GBS_Error };
		enum GeneralExtraStack { GES_None = 1, GES_LaTeX/*, GES_ViewBib*/ };

	public:
		ToolConfig(KileTool::Manager *mngr, QWidget *parent, const char * name = 0);

	public slots:
		void writeConfig();

	private:
		void setupAdvanced();
		void setupGeneral();
		int indexQuickBuild();
		
	private slots:
		void updateGeneral();
		void updateAdvanced();
		void switchTo(const QString & tool, bool save = true);
		void updateToollist();
		void updateConfiglist();
		void selectIcon();
		void setMenu(const QString &);
		void switchConfig(int index = -1);
		void switchConfig(const QString &);

		void newTool();
		void newConfig();
		void removeTool();
		void removeConfig();
		void writeStdConfig(const QString &, const QString &);
		void writeDefaults();

		void setCommand(const QString &);
		void setOptions(const QString &);
		void setLibrary(const QString &);
		void setLibOptions(const QString &);
		void setClassName(const QString &);
		void setState(const QString &);
		void setSequence(const QString &);
		void setClose(bool);
		void setTarget(const QString &);
		void setRelDir(const QString &);
		void setLaTeXCheckRoot(bool);
		void setLaTeXJump(bool);
		void setLaTeXAuto(bool);
		void setRunLyxServer(bool);
		void setFrom(const QString &);
		void setTo(const QString &);
		void setClass(const QString &);
		void switchClass(const QString &);
		void switchType(int);

	signals:
		void changed();

	private:
		ToolConfigWidget	*m_configWidget;
		KileTool::Manager	*m_manager;
		KConfig			*m_config;
		KileTool::Config	m_map;
		QGridLayout		*m_layout;
		QString			m_current, m_icon;
		QStringList		m_classes;
		QWidget			*m_tabGeneral, *m_tabAdvanced, *m_tabMenu;
		ProcessToolConfigWidget	*m_ptcw;
		LibraryToolConfigWidget	*m_ltcw;
		QuickToolConfigWidget	*m_qtcw;
		LaTeXToolConfigWidget	*m_LaTeXtcw;
	};
}

#endif
