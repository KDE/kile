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
#include <ktexteditor/configpage.h>
#include <kconfigdialogmanager.h>

#include "configcodecompletion.h"     // code completion (dani)
#include "previewconfigwidget.h"      // QuickPreview (dani)
#include "envconfigwidget.h"          // environments (dani)
#include "graphicsconfigwidget.h"     // graphics (dani)
#include "structureconfigwidget.h"    // structure view (dani)
#include "symbolviewconfigwidget.h"
//Added by qt3to4:
#include <Q3Frame>
#include <Q3PtrList>

class Q3Frame;
class KConfig;

namespace KileWidget { class ToolConfig; }
class KileWidgetHelpConfig;
class KileWidgetLatexConfig;
class KileWidgetGeneralConfig;
class KileWidgetEnvironmentConfig;
class KileWidgetGraphicsConfig;
class KileWidgetStructureViewConfig;
class KileWidgetScriptingConfig;

namespace KileTool { class Manager; }

namespace KileDialog
{
	class Config : public KPageDialog
	{
		Q_OBJECT

	public:
		Config( KConfig *config, KileInfo *ki, QWidget* parent = 0);
		~Config();

		virtual void show();

	//signals:
	//	void widgetModified();

	private slots:
		void slotOk();
		void slotCancel();
		void slotChanged();

	private:
		// dialog manager
		KConfigDialogManager *m_manager;

		KConfig *m_config;
		KileInfo *m_ki;

		bool m_editorSettingsChanged;
		bool m_editorOpened;

		KileWidget::ToolConfig	*toolPage;

		// CodeCompletion (dani)
		ConfigCodeCompletion *completePage;
		KileWidgetPreviewConfig *previewPage;

		KileWidgetHelpConfig *helpPage;
		KileWidgetLatexConfig *latexPage;
		KileWidgetGeneralConfig *generalPage;
		KileWidgetEnvironmentConfig *envPage;
		KileWidgetGraphicsConfig *graphicsPage;
		KileWidgetStructureViewConfig *structurePage;
		KileWidgetSymbolViewConfig *symbolViewPage;
		KileWidgetScriptingConfig *scriptingPage;


		// setup configuration
		void addConfigFolder(const QString &section,const QString &icon);

		void addConfigPage( QWidget *page,
		                    const QString &sectionName,const QString &itemName,
		                    const QString &pixmapName, const QString &header=QString::null,
		                    bool addSpacer = true );

		void setupGeneralOptions();
		void setupTools();
		void setupLatex();
		void setupCodeCompletion();
		void setupQuickPreview();
		void setupHelp();
		void setupEditor();
		void setupEnvironment();
		void setupGraphics();
		void setupStructure();
		void setupSymbolView();
		void setupScripting();

		// write configuration
		void writeGeneralOptionsConfig();

		// synchronize encoding
		QString readKateEncoding();
		void syncKileEncoding();

		// editor pages
		Q3PtrList<KTextEditor::ConfigPage> editorPages;
	};
}
#endif
