/***************************************************************************
                           kilehelp.h
----------------------------------------------------------------------------
    date                 : Feb 09 2004
    version              : 0.10.0
    copyright            : (C) 2004 by Holger Danielsson
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

#ifndef KILEHELP_H
#define KILEHELP_H

#include <kconfig.h>
#include <kate/view.h>

#include <qmap.h>
#include <qstring.h>

#include <kileedit.h>
#include "usermenudialog.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "helpconfig.h"

/**
  *@author Holger Danielsson
  */

namespace KileHelp
{

	enum Type 
	{
		HelpTetex,
		HelpTetexGuide,
		HelpTetexDoc,
		HelpLatex,
		HelpLatexIndex,
		HelpLatexCommand,
		HelpLatexSubject,
		HelpLatexEnvironment
	};

	class Help
	{
	public:
		Help(KileEdit *edit);
		~Help() {}
		
		void setManager(KileTool::Manager *manager) { m_manager = manager; }

		// calls for help
		void helpLatex(KileHelp::Type type);
		void helpTetex(KileHelp::Type type);
		void helpKeyword(Kate::View *view);

		void readConfig() { m_hconfig->readConfig(); }

	private:
		KileTool::Manager *m_manager;
		KileEdit *m_edit;
		bool m_use;
		HelpConfig *m_hconfig;

		QMap<QString,QString> m_dictHelpKile;
		QMap<QString,QString> m_dictHelpTetex;

		void readHelpList(const QString &filename,QMap<QString,QString> &map);
		void showHelpFile(const QString &parameter);

		void helpTetexKeyword(Kate::View *view);
		void helpLatexKeyword(Kate::View *view);
		QString getKeyword(Kate::View *view);
	};
}

#endif
