/***************************************************************************
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnout
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

#ifndef TOOLSOPTIONSDIALOG_H
#define TOOLSOPTIONSDIALOG_H

#include <kconfigdialog.h>

#include <kdeversion.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/configinterfaceextension.h>

#include "configcodecompletion.h"     // code completion (dani)



class QFrame;
class KSpellConfig;
class KConfig;

namespace KileWidget { class ToolConfig; }
class KileWidgetHelpConfig;
class KileWidgetLatexConfig;
class KileWidgetGeneralConfig;
class KileWidgetEncodingConfig;
namespace KileTool { class Manager; }

namespace KileDialog
{
	class Config : public KConfigDialog
	{
		Q_OBJECT

	public:
		Config( KConfig *config, KileTool::Manager *, QWidget* parent = 0);
		~Config();

	private slots:
		void slotOk();
		void slotCancel();

	private:
		QFrame* toolsPage;
		QFrame* quickPage;
		QFrame* spellingPage;

		KConfig *m_config;
		KSpellConfig *ksc;
		KileWidget::ToolConfig	*m_toolConfig;
		KileTool::Manager		*m_toolMngr;

		// CodeCompletion (dani)
		ConfigCodeCompletion *completePage;

		KileWidgetHelpConfig *helpPage;
		KileWidgetLatexConfig *latexPage;
		KileWidgetGeneralConfig *generalPage;
		KileWidgetEncodingConfig *encodingPage;

		// setup configuration
		void setupGeneralOptions();
		void setupEncodingOptions();
		void setupTools();
		void setupSpelling();
		void setupLatex();
		void setupCodeCompletion();
		void setupHelp();

		// write configuration
		void writeGeneralOptionsConfig();
		void writeToolsConfig();
		void writeSpellingConfig();
	};
}
#endif
