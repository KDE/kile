/***************************************************************************
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnhout
                           (C) 2005-2007  by Holger Danielsson
    email                : Jeroen.Wijnhout@kdemail.net
                           holger.danielsson@versanet.de
 ***************************************************************************/

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

#include "kileconfigdialog.h"

#include <q3vbox.h>
#include <qlayout.h>
#include <qtextcodec.h>
//Added by qt3to4:
#include <Q3Frame>

#include <kdeversion.h>
#include <klocale.h>
// #include <ksconfig.h>
#include <kiconloader.h>

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
#ifdef __GNUC__
#warning Skipped Qt::WStyle_DialogBorder
#endif
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

#ifdef __GNUC__
#warning Things left to be ported at line 83!
#endif
// 		setShowIconsInTreeList(true);
		addConfigFolder(i18n("Kile"),"kile");
		addConfigFolder(i18n("LaTeX"),"tex");
		addConfigFolder(i18n("Tools"),"gear");
		addConfigFolder(i18n("Editor"),"edit");

		// setup all configuration pages
		setupGeneralOptions();
		setupCodeCompletion();   // complete configuration (dani)
		setupHelp();
		setupScripting();

		setupLatex();
		setupEnvironment();
		setupGraphics();
		setupStructure();
		setupSymbolView();

		setupTools();
		setupQuickPreview();     // QuickPreview (dani)

		setupEditor();
		showButtonSeparator(true);

#ifdef __GNUC__
#warning Things left to be ported at line 111!
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
#warning Things left to be ported at line 128!
#endif
// 		saveDialogSize("KileConfigDialog");
		delete m_manager;
	}

	void Config::show()
	{
#ifdef __GNUC__
#warning Things left to be ported at line 126!
#endif
/*		if ( KileConfig::unfoldConfigTree() )
			unfoldTreeList();*/
		m_manager->updateWidgets();
		KDialog::show();
	}

	//////////////////// add a new folder ////////////////////

	void Config::addConfigFolder(const QString &section,const QString &icon)
	{
		QStringList path;
		path << section;

#ifdef __GNUC__
#warning Things left to be ported at line 142!
#endif
// 		setFolderIcon(path, SmallIcon(icon, KIconLoader::SizeSmallMedium));
	}

	//////////////////// add a new page ////////////////////

	void Config::addConfigPage(QWidget *page,
	                           const QString &sectionName,const QString &itemName,
	                           const QString &pixmapName, const QString &header,
	                           bool addSpacer)
	{
#ifdef __GNUC__
#warning Fix the configuration page creation!
#endif
//FIXME: port for KDE4
/*
		KILE_DEBUG() << "slot: add config page item=" << itemName << endl;

		// add page
		QStringList path;
		path << sectionName << itemName;
	
		KVBox *vbox = addPage(path, header, SmallIcon(pixmapName,KIconLoader::SizeSmallMedium));
		vbox->setSpacing(0); 
		vbox->setMargin(0);
		page->reparent(((QWidget*)vbox),0,QPoint());
		if ( addSpacer )
		{
			Q3Frame *spacer = new Q3Frame(vbox);
			vbox->setStretchFactor(spacer,1);
		}

		// add to the dialog manager
		m_manager->addWidget(page);
*/
	}

	//////////////////// General Options ////////////////////

	void Config::setupGeneralOptions()
	{
		generalPage = new KileWidgetGeneralConfig(this);
		generalPage->setObjectName("LaTeX");
		addConfigPage(generalPage,i18n("Kile"),i18n("General"),"configure",i18n("General Settings"));
	}
	
	//////////////////// Tools Configuration ////////////////////

	void Config::setupTools()
	{
		toolPage = new KileWidget::ToolConfig(m_ki->toolManager(), 0);
		addConfigPage(toolPage,i18n("Tools"),i18n("Build"),"launch",i18n("Build"),false);
	}

	//////////////////// Scripting  ////////////////////

	void Config::setupScripting()
	{
		scriptingPage = new KileWidgetScriptingConfig(this);
		scriptingPage->setObjectName("Scripting");
		addConfigPage(scriptingPage,i18n("Kile"),i18n("Scripting"),"exec",i18n("Scripting Support"));
	}

	//////////////////// LaTeX specific editing options ////////////////////

	//////////////////// Complete configuration (dani) ////////////////////

	void Config::setupCodeCompletion()
	{
		completePage = new ConfigCodeCompletion(m_config,m_ki->logWidget());
		completePage->readConfig();

		addConfigPage(completePage,i18n("Kile"),i18n("Complete"),"source",i18n("Code Completion"));
	}

	//////////////////// QuickPreview (dani) ////////////////////

	void Config::setupQuickPreview()
	{
		previewPage = new KileWidgetPreviewConfig(m_config,m_ki->quickPreview(),0);
		previewPage->readConfig();

		addConfigPage(previewPage,i18n("Tools"),i18n("Preview"),"preview",i18n("Quick Preview"));
	}

	void Config::setupHelp()
	{
		helpPage = new KileWidgetHelpConfig(this);
		helpPage->setHelp(m_ki->help());

		addConfigPage(helpPage,i18n("Kile"),i18n("Help"),"help");
	}

	//////////////////// LaTeX environments ////////////////////

	void Config::setupLatex()
	{
		latexPage = new KileWidgetLatexConfig(this);
		latexPage->setObjectName("LaTeX");
		latexPage->kcfg_DoubleQuotes->insertStringList( m_ki->editorExtension()->doubleQuotesList() ); 
		latexPage->setLatexCommands(m_config,m_ki->latexCommands());

		addConfigPage(latexPage,i18n("LaTeX"),i18n("General"),"configure");
	}

	void Config::setupEnvironment()
	{
		envPage = new KileWidgetEnvironmentConfig(this);
		envPage->setObjectName("LaTeX");
		addConfigPage(envPage,i18n("LaTeX"),i18n("Environments"),"environment");
	}

	void Config::setupGraphics()
	{
		graphicsPage = new KileWidgetGraphicsConfig(this);
		graphicsPage->setObjectName("Graphics");
		graphicsPage->m_lbImagemagick->setText( ( KileConfig::imagemagick() ) ? i18n("installed") : i18n("not installed") ); 
		addConfigPage(graphicsPage,i18n("LaTeX"),i18n("Graphics"),"graphicspage");
	}

	void Config::setupStructure()
	{
		structurePage = new KileWidgetStructureViewConfig(this);
		structurePage->setObjectName("StructureView");
		addConfigPage(structurePage,i18n("LaTeX"),i18n("Structure View"),"view_tree");
	}

	void Config::setupSymbolView()
	{
		symbolViewPage = new KileWidgetSymbolViewConfig(this);
		symbolViewPage->setObjectName("SymbolView");
		addConfigPage(symbolViewPage,i18n("LaTeX"),i18n("Symbol View"),"math0");
	}

	//////////////////// Editor ////////////////////

	void Config::setupEditor()
	{
		KTextEditor::View *view = m_ki->viewManager()->currentTextView();
		m_editorOpened = ( view != 0L );
		m_editorSettingsChanged = false;

		if ( ! m_editorOpened )
			return;

		editorPages.setAutoDelete(false);
		editorPages.clear();
#ifdef __GNUC__
#warning The editor configuration stuff still needs to be ported!
#endif
//FIXME: port for KDE4
/*
		KTextEditor::ConfigInterface *iface = dynamic_cast<KTextEditor::ConfigInterface*>(view->document());
		if(!iface) {
			return;
		}

		QStringList path;
		for (uint i=0; i<iface->configPages(); i++)
		{
			path.clear();
			path << i18n("Editor") << iface->configPageName(i);

			// create a new vbox page and add the config page
			KVBox *page = addVBoxPage(path,iface->configPageFullName(i), iface->configPagePixmap(i,KIconLoader::SizeSmallMedium) );
			KTextEditor::ConfigPage *configPage = iface->configPage(i,page);
			connect( configPage, SIGNAL(changed()), this, SLOT(slotChanged()) );
			editorPages.append(configPage);
		}
*/
	}

	//////////////////// encoding  ////////////////////

	QString Config::readKateEncoding()
	{
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
		KILE_DEBUG() << "   slot ok (" << m_manager->hasChanged() << ","  << m_editorSettingsChanged << ")" << endl;

		// editor settings are only available, when at least one document is opened
		if ( m_editorOpened && m_editorSettingsChanged )
		{
			for (uint i=0; i<editorPages.count(); i++)
				editorPages.at(i)->apply();
#ifdef __GNUC__
#warning Editor config saving stuff left to be ported!
#endif
//FIXME: port for KDE4
//			m_ki->viewManager()->currentTextView()->document()->writeConfig();
			
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
		KILE_DEBUG() << "   slot cancel" << endl;
#ifdef __GNUC__
#warning Check for KConfig.rollback() in KDE3!
#endif
// 		m_config->rollback();
		accept();
	}

	void Config::slotChanged()
	{
		KILE_DEBUG() << "   slot changed" << endl;
		m_editorSettingsChanged = true;
	}

/*
void Config::slotWidgetModified()
{
	KILE_DEBUG() << "slot: widget modified --> " << m_manager->hasChanged()  << endl;
  //emit widgetModified();
}
*/
}

#include "kileconfigdialog.moc"
