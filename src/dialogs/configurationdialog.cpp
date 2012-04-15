/***************************************************************************************************
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2005-2007  by Holger Danielsson (holger.danielsson@versanet.de)
                           (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2005-12-02 dani
//  - put configuration of Kile and Kate together in one dialog
//  - items are shown as a tree list
//  - encoding config page and spelling page are removed,
//    because settings are also avaiblable with Kate
//  - geometry of the dialog are saved and restored, because
//    the initial values may be bad in some languages

// 2007-03-17 dani
//  - add support for auto insert $
//  - move graphics config to a separate page

#include "dialogs/configurationdialog.h"

#include <QLayout>
#include <QTextCodec>

#include <kdeversion.h>
#include <KLocale>
#include <KIconLoader>
#include <KVBox>

#include <KTextEditor/ConfigPage>
#include <KTextEditor/EditorChooser>

#include "kiledocmanager.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"

#include "widgets/appearanceconfigwidget.h"
#include "widgets/generalconfigwidget.h"
#include "widgets/helpconfigwidget.h"
#include "widgets/latexconfigwidget.h"
#include "widgets/livepreviewconfigwidget.h"
#include "widgets/previewconfigwidget.h"
#include "widgets/scriptingconfigwidget.h"
#include "widgets/toolconfigwidget.h"
#include "widgets/usermenuconfigwidget.h"

#include "kileconfig.h"
#include "kileinfo.h"
#include "editorextension.h"

#include "kiledebug.h"

namespace KileDialog
{
	Config::Config(KConfig *config, KileInfo *ki, QWidget* parent)
		: KPageDialog(parent),
		  m_config(config),
		  m_ki(ki)
	{
		setCaption(i18n("Configure"));
		setModal(true);
		setButtons(Ok | Cancel);
		setDefaultButton(Ok);
		showButtonSeparator(true);
		setObjectName("kileconfiguration");
		setFaceType(Tree);

		m_config->sync();

		// we need a dialog manager
		m_manager = new KConfigDialogManager(this,KileConfig::self());

		KPageWidgetItem* kilePageWidgetItem = addConfigFolder(i18n("Kile"), "kile");
		KPageWidgetItem* latexPageWidgetItem = addConfigFolder(i18n("LaTeX"), "latex-config");
		KPageWidgetItem* toolsPageWidgetItem = addConfigFolder(i18n("Tools"), "system-run");
		KPageWidgetItem* editorPageWidgetItem = addConfigFolder(i18n("Editor"), "accessories-text-editor");

		// setup all configuration pages
		setupGeneralOptions(kilePageWidgetItem);
		setupAppearance(kilePageWidgetItem);
		setupCodeCompletion(kilePageWidgetItem);   // complete configuration (dani)
		setupHelp(kilePageWidgetItem);
		setupScripting(kilePageWidgetItem);
		setupUsermenu(kilePageWidgetItem);
		setupLivePreview(kilePageWidgetItem);

		setupLatex(latexPageWidgetItem);
		setupEnvironment(latexPageWidgetItem);
		setupGraphics(latexPageWidgetItem);
		setupStructure(latexPageWidgetItem);
		setupSymbolView(latexPageWidgetItem);

		setupTools(toolsPageWidgetItem);
		setupQuickPreview(toolsPageWidgetItem);     // QuickPreview (dani)

		setupEditor(editorPageWidgetItem);
		showButtonSeparator(true);

		m_configDialogSize = m_config->group("KileConfigDialog");
		restoreDialogSize(m_configDialogSize);

		// setup connections
		//connect(m_manager, SIGNAL(widgetModified()), this, SLOT(slotWidgetModified()));
		connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
		connect(this, SIGNAL(cancelClicked()), this, SLOT(slotCancel()));
		connect(this, SIGNAL(okClicked()), m_manager, SLOT(updateSettings()));
	}

	Config::~Config()
	{
		saveDialogSize(m_configDialogSize);
		delete m_manager;
	}

	void Config::show()
	{
		m_manager->updateWidgets();
		KDialog::show();
	}

	//////////////////// add a new folder ////////////////////

	KPageWidgetItem* Config::addConfigFolder(const QString &section, const QString &icon)
	{
		KPageWidgetItem *toReturn = addPage(0, section);
		toReturn->setIcon(KIcon(icon));

		return toReturn;
	}

	//////////////////// add a new page ////////////////////

	KPageWidgetItem* Config::addConfigPage(KPageWidgetItem* parent, QWidget *page, const QString &itemName,
                                   const QString &pixmapName, const QString &header)
	{
		return addConfigPage(parent, page, itemName, KIcon(pixmapName), header);
	}

	KPageWidgetItem* Config::addConfigPage(KPageWidgetItem* parent, QWidget *page,
                                               const QString &itemName, const KIcon& icon,
                                               const QString &header)
	{
		KILE_DEBUG() << "slot: add config page item=" << itemName;

		// add page
		KPageWidgetItem *pageWidgetItem = addSubPage(parent, page, itemName);
		pageWidgetItem->setIcon(icon);
		pageWidgetItem->setHeader(header);

		// add to the dialog manager
		m_manager->addWidget(page);

		return pageWidgetItem;
	}

	//////////////////// General Options ////////////////////

	void Config::setupGeneralOptions(KPageWidgetItem* parent)
	{
		generalPage = new KileWidgetGeneralConfig(this);
		generalPage->setObjectName("LaTeX");
		addConfigPage(parent, generalPage, i18n("General"), "configure", i18n("General Settings"));
	}

	//////////////////// Tools Configuration ////////////////////

	void Config::setupTools(KPageWidgetItem* parent)
	{
		toolPage = new KileWidget::ToolConfig(m_ki->toolManager(), this);
		addConfigPage(parent, toolPage, i18n("Build"), "application-x-executable", i18n("Build"));
	}

	//////////////////// Scripting  ////////////////////

	void Config::setupScripting(KPageWidgetItem* parent)
	{
		scriptingPage = new KileWidgetScriptingConfig(this);
		scriptingPage->setObjectName("Scripting");
		// we hide this now as the feature is not implemented currently
		scriptingPage->executionTimeLimitGroupBox->setHidden(true);
		addConfigPage(parent, scriptingPage, i18n("Scripting"), "application-x-executable-script", i18n("Scripting Support"));
	}

	//////////////////// Usermenu  ////////////////////

	void Config::setupUsermenu(KPageWidgetItem *parent)
	{
		usermenuPage = new KileWidgetUsermenuConfig(m_ki->userMenu(),this);
		usermenuPage->setObjectName("Usermenu");
		addConfigPage(parent, usermenuPage, i18n("User Menu"), "usermenu-install", i18n("User Menu"));

	}

	//////////////////// LaTeX specific editing options ////////////////////

	//////////////////// Complete configuration (dani) ////////////////////

	void Config::setupCodeCompletion(KPageWidgetItem* parent)
	{
		completePage = new CodeCompletionConfigWidget(m_config, m_ki->logWidget());
		completePage->readConfig();

		addConfigPage(parent, completePage, i18n("Complete"), "text-x-tex", i18n("Code Completion"));
	}

	//////////////////// QuickPreview (dani) ////////////////////

	void Config::setupQuickPreview(KPageWidgetItem* parent)
	{
		previewPage = new KileWidgetPreviewConfig(m_config,m_ki->quickPreview(),this);
		previewPage->readConfig();

		addConfigPage(parent, previewPage, i18n("Preview"), "preview", i18n("Quick Preview"));
	}

	void Config::setupHelp(KPageWidgetItem* parent)
	{
		helpPage = new KileWidgetHelpConfig(this);
		helpPage->setHelp(m_ki->help());

		addConfigPage(parent, helpPage, i18n("Help"),"help-browser");
	}

	void Config::setupLivePreview(KPageWidgetItem* parent)
	{
		livePreviewPage = new KileWidgetLivePreviewConfig(m_config, this);
		livePreviewPage->readConfig();

		addConfigPage(parent, livePreviewPage, i18n("Live Preview"), "preview", i18n("Live Preview"));
	}

	void Config::setupAppearance(KPageWidgetItem* parent)
	{
		appearancePage = new KileWidgetAppearanceConfig(m_config, this);
		appearancePage->readConfig();

		addConfigPage(parent, appearancePage, i18n("Appearance"), "preview", i18n("Appearance"));
	}

	//////////////////// LaTeX environments ////////////////////

	void Config::setupLatex(KPageWidgetItem* parent)
	{
		latexPage = new KileWidgetLatexConfig(this);
		latexPage->setObjectName("LaTeX");
		latexPage->kcfg_DoubleQuotes->addItems(m_ki->editorExtension()->doubleQuotesListI18N());
		latexPage->setLatexCommands(m_config,m_ki->latexCommands());

		addConfigPage(parent, latexPage, i18n("General"), "configure");
	}

	void Config::setupEnvironment(KPageWidgetItem* parent)
	{
		envPage = new KileWidgetEnvironmentConfig(this);
		envPage->setObjectName("LaTeX");
		addConfigPage(parent, envPage, i18n("Environments"), "environment");
	}

	void Config::setupGraphics(KPageWidgetItem* parent)
	{
		graphicsPage = new KileWidgetGraphicsConfig(this);
		graphicsPage->setObjectName("Graphics");
		graphicsPage->m_lbImagemagick->setText( ( KileConfig::imagemagick() ) ? i18n("installed") : i18n("not installed") );
		addConfigPage(parent, graphicsPage, i18n("Graphics"), "graphicspage");
	}

	void Config::setupStructure(KPageWidgetItem* parent)
	{
		structurePage = new KileWidgetStructureViewConfig(this);
		structurePage->setObjectName("StructureView");
		addConfigPage(parent, structurePage, i18n("Structure View"), "view-list-tree");
	}

	void Config::setupSymbolView(KPageWidgetItem* parent)
	{
		symbolViewPage = new KileWidgetSymbolViewConfig(this);
		symbolViewPage->setObjectName("SymbolView");
		addConfigPage(parent, symbolViewPage, i18n("Symbol View"), "math0");
	}

	//////////////////// Editor ////////////////////

	void Config::setupEditor(KPageWidgetItem* parent)
	{
		m_editorSettingsChanged = false;

		m_editorPages.clear();

		KTextEditor::Editor* editor = m_ki->docManager()->getEditor();
		if(!editor) {
			return;
		}
		for(int i = 0; i < editor->configPages(); ++i) {
			KVBox *configPageParent = new KVBox(this);
			KTextEditor::ConfigPage *configPage = editor->configPage(i, configPageParent);

			KPageWidgetItem *pageWidgetItem = addConfigPage(parent, configPageParent, editor->configPageName(i), editor->configPageIcon(i), editor->configPageFullName(i));
			connect(configPage, SIGNAL(changed()), this, SLOT(slotChanged()));
			m_editorPages.insert(pageWidgetItem, configPage);
		}
	}

	//////////////////// slots ////////////////////

	void Config::slotOk()
	{
		KILE_DEBUG() << "   slot ok (" << m_manager->hasChanged() << ","  << m_editorSettingsChanged << ")";

		// editor settings are only available, when at least one document is opened
		if(m_editorSettingsChanged) {
			QMapIterator<KPageWidgetItem*, KTextEditor::ConfigPage*> i(m_editorPages);
			while (i.hasNext()) {
				i.next();
				i.value()->apply();
			}

			KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
			if(editor) {
				editor->writeConfig(m_config);
			}
		}

		// Kile settings
		toolPage->writeConfig();      // config all tools
		completePage->writeConfig();  // Complete configuration (dani)
		previewPage->writeConfig();   // Quick Preview (dani)
		usermenuPage->writeConfig();
		livePreviewPage->writeConfig();

		m_config->sync();

		// oder m_manager->updateSettings();
		accept();
	}

	void Config::slotCancel()
	{
		KILE_DEBUG() << "   slot cancel";
		m_config->markAsClean();
		accept();
	}

	void Config::slotChanged()
	{
		KILE_DEBUG() << "   slot changed";
		m_editorSettingsChanged = true;
	}

}

#include "configurationdialog.moc"
