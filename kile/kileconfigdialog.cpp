/***************************************************************************
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnhout
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

// 2005-12-02 dani
//  - put configuration of Kile and Kate together in one dialog
//  - items are shown as a tree list
//  - encoding config page and spelling page are removed, 
//    because settings are also avaiblable with Kate
//  - geometry of the dialog are saved and restored, because
//    the initial values may be bad in some languages

#include <qvbox.h>
#include <qlayout.h>
#include <qtextcodec.h>

#include <kdeversion.h>
#include <klocale.h>
#include <ksconfig.h>
#include <kiconloader.h>

#include "kiletoolmanager.h"
#include "kiletoolconfigwidget.h"
#include "kileconfigdialog.h"
#include "kileviewmanager.h"
#include "helpconfigwidget.h"
#include "latexconfigwidget.h"
#include "generalconfigwidget.h"
#include "previewconfigwidget.h"
#include "scriptingconfigwidget.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "kileedit.h"

#include <kdebug.h>

namespace KileDialog
{
	Config::Config(KConfig *config, KileInfo *ki, QWidget* parent)
		: KDialogBase( KDialogBase::TreeList, Qt::WStyle_DialogBorder,
		               parent, "kileconfiguration", true, i18n("Configure"), Ok|Cancel, Ok ),
		  m_config(config),
		  m_ki(ki)
	{
		m_config->sync();

		// we need a dialog manager
		m_manager = new KConfigDialogManager(this,KileConfig::self());

		setShowIconsInTreeList(true);
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
		setupStructure();

		setupTools();
		setupQuickPreview();     // QuickPreview (dani)

		setupEditor();
		enableButtonSeparator(true);

		// calculate size for opening
		if ( ! m_config->hasGroup("KileConfigDialog") )
			incInitialSize(QSize(50,0));
		else
			setInitialSize( configDialogSize("KileConfigDialog") );

		// setup connections
		//connect(m_manager, SIGNAL(widgetModified()), this, SLOT(slotWidgetModified()));
		connect(this, SIGNAL(okClicked()), m_manager, SLOT(updateSettings()));
	}

	Config::~Config()
	{
		saveDialogSize("KileConfigDialog");
		delete m_manager;
	}

	void Config::show()
	{
		if ( KileConfig::unfoldConfigTree() )
			unfoldTreeList();
		m_manager->updateWidgets();
		KDialogBase::show();
	}

	//////////////////// add a new folder ////////////////////

	void Config::addConfigFolder(const QString &section,const QString &icon)
	{
		QStringList path;
		path << section;

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
		setFolderIcon(path, SmallIcon(icon, KIcon::SizeSmallMedium));
#else
                setFolderIcon(path, SmallIcon(icon));
#endif
	}

	//////////////////// add a new page ////////////////////

	void Config::addConfigPage(QWidget *page,
	                           const QString &sectionName,const QString &itemName,
	                           const QString &pixmapName, const QString &header,
	                           bool addSpacer)
	{
		kdDebug() << "slot: add config page item=" << itemName << endl;

		// add page
		QStringList path;
		path << sectionName << itemName;
	
		QVBox *vbox = addVBoxPage(path, header,
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
                SmallIcon(pixmapName,KIcon::SizeSmallMedium)
#else
                SmallIcon(pixmapName)
#endif
                );
		vbox->setSpacing(0); 
		vbox->setMargin(0);
		page->reparent(((QWidget*)vbox),0,QPoint());
		if ( addSpacer )
		{
			QFrame *spacer = new QFrame(vbox);
			vbox->setStretchFactor(spacer,1);
		}

		// add to the dialog manager
		m_manager->addWidget(page);
	}

	//////////////////// General Options ////////////////////

	void Config::setupGeneralOptions()
	{
		generalPage = new KileWidgetGeneralConfig(0, "LaTeX");
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
		scriptingPage = new KileWidgetScriptingConfig(this, "Scripting");
		addConfigPage(scriptingPage,i18n("Kile"),i18n("Scripting"),"exec",i18n("Scripting Support"));
	}

	//////////////////// LaTeX specific editing options ////////////////////

	//////////////////// Complete configuration (dani) ////////////////////

	void Config::setupCodeCompletion()
	{
		completePage = new ConfigCodeCompletion(m_config);
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
		helpPage = new KileWidgetHelpConfig(0); 
		helpPage->setHelp(m_ki->help());

		addConfigPage(helpPage,i18n("Kile"),i18n("Help"),"help");
	}

	//////////////////// LaTeX environments ////////////////////

	void Config::setupLatex()
	{
		latexPage = new KileWidgetLatexConfig(0, "LaTeX"); 
		latexPage->kcfg_DoubleQuotes->insertStringList( m_ki->editorExtension()->doubleQuotesList() ); 
		latexPage->setLatexCommands(m_config,m_ki->latexCommands());

		latexPage->m_lbImagemagick->setText( ( KileConfig::imagemagick() ) ? i18n("installed") : i18n("not installed") ); 

		addConfigPage(latexPage,i18n("LaTeX"),i18n("General"),"configure");
	}

	void Config::setupEnvironment()
	{
		envPage = new KileWidgetEnvironmentConfig(0, "LaTeX");
      addConfigPage(envPage,i18n("LaTeX"),i18n("Environments"),"environment");
	}

	void Config::setupStructure()
	{
		structurePage = new KileWidgetStructureViewConfig(0, "StructureView");
      addConfigPage(structurePage,i18n("LaTeX"),i18n("Structure View"),"view_tree");
	}

	//////////////////// Editor ////////////////////

	void Config::setupEditor()
	{
		Kate::View *view = m_ki->viewManager()->currentTextView();
		m_editorOpened = ( view != 0L );
		m_editorSettingsChanged = false;

		if ( ! m_editorOpened )
			return;

		editorPages.setAutoDelete(false);
		editorPages.clear();

		KTextEditor::ConfigInterfaceExtension *iface;
		iface = dynamic_cast<KTextEditor::ConfigInterfaceExtension *>( view->getDoc() );

		QStringList path;
		for (uint i=0; i<iface->configPages(); i++)
		{
			path.clear();
			path << i18n("Editor") << iface->configPageName(i);

			// create a new vbox page and add the config page
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
			QVBox *page = addVBoxPage(path,iface->configPageFullName(i), iface->configPagePixmap(i,KIcon::SizeSmallMedium) );
#else
			QVBox *page = addVBoxPage(path,iface->configPageFullName(i), iface->configPagePixmap(i) );
#endif
			KTextEditor::ConfigPage *configPage = iface->configPage(i,page);
			connect( configPage, SIGNAL(changed()), this, SLOT(slotChanged()) );
			editorPages.append(configPage);
		}
	}

	//////////////////// encoding  ////////////////////

	QString Config::readKateEncoding()
	{
		m_config->setGroup("Kate Document Defaults");
		return m_config->readEntry("Encoding",QString::null);
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
		kdDebug() << "   slot ok (" << m_manager->hasChanged() << ","  << m_editorSettingsChanged << ")" << endl;

		// editor settings are only available, when at least one document is opened
		if ( m_editorOpened && m_editorSettingsChanged )
		{
			for (uint i=0; i<editorPages.count(); i++)
				editorPages.at(i)->apply();
 			m_ki->viewManager()->currentTextView()->getDoc()->writeConfig();
			
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
		kdDebug() << "   slot cancel" << endl;
		m_config->rollback();
		accept();
	}

	void Config::slotChanged()
	{
		kdDebug() << "   slot changed" << endl;
		m_editorSettingsChanged = true;
	}

/*
void Config::slotWidgetModified()
{
	kdDebug() << "slot: widget modified --> " << m_manager->hasChanged()  << endl;
  //emit widgetModified();
}
*/
}

#include "kileconfigdialog.moc"
