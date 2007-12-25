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
// #include <ksconfig.h>
#include <KIconLoader>

#include <KTextEditor/ConfigPage>
#include <KTextEditor/EditorChooser>

#include "kiletoolmanager.h"
#include "kiletoolconfigwidget.h"
#include "kileviewmanager.h"
#include "helpconfigwidget.h"
#include "latexconfigwidget.h"
#include "generalconfigwidget.h"
#include "previewconfigwidget.h"
#include "scriptingconfigwidget.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "kileedit.h"

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
		KPageWidgetItem* latexPageWidgetItem = addConfigFolder(i18n("LaTeX"), "tex");
		KPageWidgetItem* toolsPageWidgetItem = addConfigFolder(i18n("Tools"), "system-run");
		KPageWidgetItem* editorPageWidgetItem = addConfigFolder(i18n("Editor"), "accessories-text-editor");

		// setup all configuration pages
		setupGeneralOptions(kilePageWidgetItem);
		setupCodeCompletion(kilePageWidgetItem);   // complete configuration (dani)
		setupHelp(kilePageWidgetItem);
		setupScripting(kilePageWidgetItem);

		setupLatex(latexPageWidgetItem);
		setupEnvironment(latexPageWidgetItem);
		setupGraphics(latexPageWidgetItem);
		setupStructure(latexPageWidgetItem);
		setupSymbolView(latexPageWidgetItem);

		setupTools(toolsPageWidgetItem);
		setupQuickPreview(toolsPageWidgetItem);     // QuickPreview (dani)

		setupEditor(editorPageWidgetItem);
		showButtonSeparator(true);

#ifdef __GNUC__
#warning Things left to be ported!
#endif
/*
		// calculate size for opening
		if ( ! m_config->hasGroup("KileConfigDialog") )
			incInitialSize(QSize(50,0));
		else
			setInitialSize( configDialogSize("KileConfigDialog") );
*/

		// setup connections
		//connect(m_manager, SIGNAL(widgetModified()), this, SLOT(slotWidgetModified()));
		connect(this, SIGNAL(okClicked()), m_manager, SLOT(updateSettings()));
	}

	Config::~Config()
	{
#ifdef __GNUC__
#warning Things left to be ported!
#endif
// 		saveDialogSize("KileConfigDialog");
		delete m_manager;
	}

	void Config::show()
	{
#ifdef __GNUC__
#warning Things left to be ported!
#endif
/*		if ( KileConfig::unfoldConfigTree() )
			unfoldTreeList();*/
		m_manager->updateWidgets();
		KDialog::show();
	}

	//////////////////// add a new folder ////////////////////

	KPageWidgetItem* Config::addConfigFolder(const QString &section, const QString &icon)
	{
		KPageWidgetItem *toReturn = addPage(NULL, section);
		toReturn->setIcon(KIcon(SmallIcon(icon, KIconLoader::SizeSmallMedium)));

		return toReturn;
	}

	//////////////////// add a new page ////////////////////

	KPageWidgetItem* Config::addConfigPage(KPageWidgetItem* parent, QWidget *page, const QString &itemName,
                                   const QString &pixmapName, const QString &header,
	                           bool addSpacer)
	{
		return addConfigPage(parent, page, itemName, KIcon(SmallIcon(pixmapName, KIconLoader::SizeSmallMedium)), header, addSpacer);
	}

	KPageWidgetItem* Config::addConfigPage(KPageWidgetItem* parent, QWidget *page,
                                               const QString &itemName, const KIcon& icon,
                                               const QString &header, bool addSpacer)
	{
		KILE_DEBUG() << "slot: add config page item=" << itemName;

		// add page
		KPageWidgetItem *pageWidgetItem = addSubPage(parent, page, itemName);
		pageWidgetItem->setIcon(icon);
		pageWidgetItem->setHeader(header);
#ifdef __GNUC__
#warning Still some things left here!
#endif
// 		if ( addSpacer )
// 		{
// 			Q3Frame *spacer = new Q3Frame(vbox);
// 			vbox->setStretchFactor(spacer,1);
// 		}

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
		toolPage = new KileWidget::ToolConfig(m_ki->toolManager(), 0);
		addConfigPage(parent, toolPage, i18n("Build"), "launch", i18n("Build"), false);
	}

	//////////////////// Scripting  ////////////////////

	void Config::setupScripting(KPageWidgetItem* parent)
	{
		scriptingPage = new KileWidgetScriptingConfig(this);
		scriptingPage->setObjectName("Scripting");
		addConfigPage(parent, scriptingPage, i18n("Scripting"), "exec", i18n("Scripting Support"));
	}

	//////////////////// LaTeX specific editing options ////////////////////

	//////////////////// Complete configuration (dani) ////////////////////

	void Config::setupCodeCompletion(KPageWidgetItem* parent)
	{
		completePage = new ConfigCodeCompletion(m_config,m_ki->logWidget());
		completePage->readConfig();

		addConfigPage(parent, completePage, i18n("Complete"), "source", i18n("Code Completion"));
	}

	//////////////////// QuickPreview (dani) ////////////////////

	void Config::setupQuickPreview(KPageWidgetItem* parent)
	{
		previewPage = new KileWidgetPreviewConfig(m_config,m_ki->quickPreview(),0);
		previewPage->readConfig();

		addConfigPage(parent, previewPage, i18n("Preview"), "preview", i18n("Quick Preview"));
	}

	void Config::setupHelp(KPageWidgetItem* parent)
	{
		helpPage = new KileWidgetHelpConfig(this);
		helpPage->setHelp(m_ki->help());

		addConfigPage(parent, helpPage, i18n("Help"),"help");
	}

	//////////////////// LaTeX environments ////////////////////

	void Config::setupLatex(KPageWidgetItem* parent)
	{
		latexPage = new KileWidgetLatexConfig(this);
		latexPage->setObjectName("LaTeX");
		latexPage->kcfg_DoubleQuotes->insertStringList( m_ki->editorExtension()->doubleQuotesList() ); 
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
		addConfigPage(parent, structurePage, i18n("Structure View"), "view_tree");
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

		KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
		for(int i = 0; i < editor->configPages(); ++i) {
			KTextEditor::ConfigPage *configPage = editor->configPage(i, parent->widget());

			KPageWidgetItem *pageWidgetItem = addConfigPage(parent, configPage, editor->configPageName(i), editor->configPageIcon(i), editor->configPageFullName(i));
			connect(configPage, SIGNAL(changed()), this, SLOT(slotChanged()));
			m_editorPages.append(pageWidgetItem);
		}
	}

	//////////////////// encoding  ////////////////////

	QString Config::readKateEncoding()
	{
#ifdef __GNUC__
#warning The editor's encoding cannot be read like this!
#endif
		KConfigGroup group = m_config->group("Kate Document Defaults");
		return group.readEntry("Encoding", QString());
	}
	
	void Config::syncKileEncoding()
	{
		QString enc = readKateEncoding();
		if ( enc.isEmpty() )
				enc = QString::fromLatin1(QTextCodec::codecForLocale()->name());
		KileConfig::setDefaultEncoding( enc );
	}
	//////////////////// slots ////////////////////

	void Config::slotOk()
	{
		KILE_DEBUG() << "   slot ok (" << m_manager->hasChanged() << ","  << m_editorSettingsChanged << ")";

		// editor settings are only available, when at least one document is opened
		if(m_editorSettingsChanged) {
			for(QList<KPageWidgetItem*>::iterator i = m_editorPages.begin(); i != m_editorPages.end(); ++i) {
				KTextEditor::ConfigPage *configPage = static_cast<KTextEditor::ConfigPage*>((*i)->widget());
				configPage->apply();
			}

			KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
			if(editor) {
				editor->writeConfig(m_config);
			}

			// take Kate's encoding for Kile
			syncKileEncoding();
		}

		// Kile settings
		toolPage->writeConfig();      // config all tools
		completePage->writeConfig();  // Complete configuration (dani)
		previewPage->writeConfig();   // Quick Preview (dani)

		m_config->sync();
		emit okClicked(); // Otherwise, the KConfigXT machine doesn't start...

		// oder m_manager->updateSettings();
		accept();
	}

	void Config::slotCancel()
	{
		KILE_DEBUG() << "   slot cancel";
#ifdef __GNUC__
#warning Check for KConfig.rollback() in KDE3!
#endif
// 		m_config->rollback();
		accept();
	}

	void Config::slotChanged()
	{
		KILE_DEBUG() << "   slot changed";
		m_editorSettingsChanged = true;
	}

/*
void Config::slotWidgetModified()
{
	KILE_DEBUG() << "slot: widget modified --> " << m_manager->hasChanged();
  //emit widgetModified();
}
*/
}

#include "configurationdialog.moc"
