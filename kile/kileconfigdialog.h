/***************************************************************************
                          toolsoptionsdialog.h  -  description
                             -------------------
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

#include <kdialogbase.h>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/configinterfaceextension.h>

#include "configcodecompletion.h"     // code completion (dani)

class QLabel;

class QComboBox;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QButtonGroup;
class QFrame;

class KLineEdit;
class KSpellConfig;
class KColorButton;
class KIntNumInput;
class KConfig;

namespace KileWidget { class ToolConfig; }
class KileWidgetHelpConfig;
namespace KileTool { class Manager; }

namespace KileDialog 
{
	class Config : public KDialogBase
	{
		Q_OBJECT
		
	public:
		Config( KConfig *config, KileTool::Manager *, QWidget* parent = 0, const char* name = 0);
		~Config();
		
	private slots:
		void slotOk();
		void slotCancel();

	private:
		QLabel* TextLabel1, * TextLabel2,* TextLabel3;

		QLabel    *TextLabel6, *TextLabel7, *TextLabel8, *TextLabel9, *TextLabel10, *TextLabel11;
		QLabel    *TextLabel12, *TextLabel13, *TextLabel14;
		KLineEdit *LineEdit6,  *LineEdit7,  *LineEdit9,  *LineEdit10,  *LineEdit11, *LineEdit12, *LineEdit13, *LineEdit14;

		QComboBox *comboDvi, *comboPs, *comboPdf, *comboLatexHelp;
		QButtonGroup *ButtonGroup2;
		QSpinBox *spinLevel;
		QCheckBox *checkAutosave, *checkEnv, *checkRestore, *checkForRoot, *m_runlyxserver, *checkCleanUpAfterClose;
		QCheckBox *cb_boundingbox;

		KLineEdit* fileExtensionList;

		QRadioButton *checkLatex, *checkPdflatex,
		*checkDviSearch, *checkDvi, *checkDviPdf, *checkPsPdf;

		KIntNumInput *asIntervalInput;

		KLineEdit *templAuthor, *templDocClassOpt, *templEncoding, *edit_res;

		QFrame* generalPage;
		QFrame* toolsPage;
		QFrame* quickPage;
		QFrame* spellingPage;
		QFrame* editPage;

		KConfig *m_config;
		KSpellConfig *ksc;
		KileWidget::ToolConfig	*m_toolConfig;
		KileTool::Manager		*m_toolMngr;
	
		// CodeCompletion (dani)
		ConfigCodeCompletion *completePage;
		KileWidgetHelpConfig *helpPage;

		// setup configuration
		void setupGeneralOptions();
		void setupTools();
		void setupSpelling();
		void setupLatex();
		void setupCodeCompletion();
		void setupHelp();

		// write configuration
		void writeGeneralOptionsConfig();
		void writeToolsConfig();
		void writeSpellingConfig();
		void writeLatexConfig();
	};
}
#endif
