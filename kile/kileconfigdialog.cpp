/***************************************************************************
                          toolsoptionsdialog.cpp  -  description
                             -------------------
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

#include <qlabel.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qvgroupbox.h>

#include <klineedit.h>
#include <knuminput.h>
#include <knumvalidator.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksconfig.h>
#include <kconfig.h>

#include "kiletoolmanager.h"
#include "kiletoolconfigwidget.h"
#include "kileconfigdialog.h"
#include "helpconfigwidget.h"

namespace KileDialog
{
	Config::Config(KConfig *config, KileTool::Manager *mngr, QWidget* parent,  const char* name)
		:KDialogBase( KDialogBase::IconList, i18n("Configure Kile"), Ok|Cancel,Ok, parent, name, true, true ),
		m_config(config),
		m_toolMngr(mngr)
	{
		m_config->sync();
		setShowIconsInTreeList(true);
		
		// setup all configuration pages
		setupGeneralOptions();
		setupTools();
		setupSpelling();
		setupLatex();
		setupCodeCompletion();   // complete configuration (dani)
		setupHelp();
	}
	
	
	Config::~Config()
	{}
	
	//////////////////// General Options ////////////////////
	
	void Config::setupGeneralOptions()
	{
		generalPage = addPage(i18n("General"),i18n("General options"),
					KGlobal::instance()->iconLoader()->loadIcon( "configure", KIcon::NoGroup, KIcon::SizeMedium ));
		
		QVBoxLayout *genLayout = new QVBoxLayout(generalPage,5);
		
		//autosave options
		QGroupBox *autosaveGroup = new QGroupBox(3,Qt::Horizontal,i18n("Autosave options"),generalPage);
		
		checkAutosave = new QCheckBox(autosaveGroup, "Autosave");
		checkAutosave->setText(i18n("Auto&save"));
		
		QLabel *lb = new QLabel(i18n("Interval &time in minutes (1 - 9999) : "),autosaveGroup);
		asIntervalInput=new KIntNumInput(autosaveGroup,"asIntervalInput");
		asIntervalInput->setRange(1, 9999, 1, false);
		lb->setBuddy(asIntervalInput);
		
		genLayout->addWidget(autosaveGroup);
		genLayout->setStretchFactor(autosaveGroup,0);
	
		//fill in autosave
		m_config->setGroup("Files");
		checkAutosave->setChecked(m_config->readBoolEntry("Autosave",true));
		asIntervalInput->setValue(m_config->readLongNumEntry("AutosaveInterval",600000)/60000);
	
		QGroupBox *structGroup = new QGroupBox(2, Qt::Horizontal, i18n("Structure view options"), generalPage);
		genLayout->addWidget(structGroup);
		genLayout->setStretchFactor(structGroup,0);

		//default structure level
		lb = new QLabel(i18n("Default expansion &level for the structure view (1 part - 5 subsubsection) : "),structGroup);
		spinLevel = new QSpinBox(1,5, 1, structGroup);
		m_config->setGroup("Structure");
		spinLevel->setValue(m_config->readNumEntry("DefaultLevel",1));
		lb->setBuddy(spinLevel);
	
		//reopen files and projects
		checkRestore = new QCheckBox(i18n("Reopen files and projects on startup."), generalPage );
		genLayout->addWidget(checkRestore);
		genLayout->setStretchFactor(checkRestore,5);
		m_config->setGroup("Files");
		checkRestore->setChecked(m_config->readBoolEntry("Restore",true));
		
			//template variables
		QGroupBox *templateGroup = new QGroupBox(2,Qt::Horizontal, i18n("Template variables"), generalPage);
		
		QLabel *lbAuthor = new QLabel(i18n("&Author"),templateGroup);
		templAuthor = new KLineEdit(templateGroup, "templAuthor");
		lbAuthor->setBuddy(templAuthor);
		QLabel *lbDocClassOpt = new QLabel(i18n("&Documentclass options"),templateGroup);
		templDocClassOpt = new KLineEdit(templateGroup, "templDocClassOpt");
		lbDocClassOpt->setBuddy(templDocClassOpt);
		QLabel *lbEnc = new QLabel(i18n("Input &encoding"), templateGroup);
		templEncoding = new KLineEdit(templateGroup, "templEncoding");
		lbEnc->setBuddy(templEncoding);
		
		genLayout->addWidget(templateGroup);
		//	genLayout->setStretchFactor(templateGroup,0);

		QGroupBox *cleanUpGroup = new QGroupBox(1, Qt::Horizontal, i18n("File clean-up details"), generalPage);
		genLayout->addWidget(cleanUpGroup);
		genLayout->setStretchFactor(cleanUpGroup,0);
		checkCleanUpAfterClose = new QCheckBox(i18n("Automatically clean-up files after close."), cleanUpGroup );
		fileExtensionList = new KLineEdit( cleanUpGroup, "fileExtensionList" );

		m_config->setGroup("Files");
		checkCleanUpAfterClose->setChecked(m_config->readBoolEntry("CleanUpAfterClose",false));
		fileExtensionList->setText(m_config->readListEntry("CleanUpFileExtensions").join(" "));

		genLayout->addStretch();                     // looks better (dani)

		//fill in template variables
		m_config->setGroup( "User" );
		templAuthor->setText(m_config->readEntry("Author",""));
		templDocClassOpt->setText(m_config->readEntry("DocumentClassOptions","a4paper,10pt"));
		templEncoding->setText(m_config->readEntry("Template Encoding",""));
	}
	
	//////////////////// Tools Configuration ////////////////////
	
	void Config::setupTools() 
	{
		QFrame *toolsPage = addPage(i18n("Tools"),i18n("Tools Configuration"),
					KGlobal::instance()->iconLoader()->loadIcon( "gear", KIcon::NoGroup, KIcon::SizeMedium ));
	
		QVBoxLayout *toolsLayout = new QVBoxLayout(toolsPage, 5);
		m_toolConfig = new KileWidget::ToolConfig(m_toolMngr, toolsPage);
		toolsLayout->addWidget(m_toolConfig);
	}
	
	//////////////////// Spelling Configuration ////////////////////
	
	void Config::setupSpelling() 
	{
		spellingPage = addPage(i18n("Spelling"),i18n("Spelling Configuration"),
					KGlobal::instance()->iconLoader()->loadIcon( "spellcheck", KIcon::NoGroup, KIcon::SizeMedium ));
		
		QGridLayout *gbox2 = new QGridLayout( spellingPage,2,2,5,5,"" );
		gbox2->addRowSpacing( 0, fontMetrics().lineSpacing() );
		
		QGroupBox* GroupBox3= new QGroupBox(2,Qt::Horizontal,i18n("Spelling"),spellingPage, "ButtonGroup" );
		
		ksc = new KSpellConfig(GroupBox3, "spell",0, false );
		
		gbox2->addMultiCellWidget(GroupBox3,0,0,0,1,0);
	
	}
	
	//////////////////// LaTeX specific editing options ////////////////////
	
	void Config::setupLatex()
	{
		editPage = addPage(i18n("LaTeX"),i18n("LaTeX specific editing options"),
			KGlobal::instance()->iconLoader()->loadIcon( "tex", KIcon::NoGroup, KIcon::SizeMedium ));
	
		// Layout
		QVBoxLayout *vbox = new QVBoxLayout(editPage, 5,KDialog::spacingHint() );
		
		// first groupbox: environments
		QGroupBox *group1 = new QGroupBox(1, Qt::Horizontal, i18n("Environments"), editPage);
		checkEnv = new QCheckBox(i18n("Automatically complete \\begin{env} with \\end{env}"),group1);
		
		// second groupbox: include graphics
		QVGroupBox* group2= new QVGroupBox(i18n("Include Graphics"),editPage );
		QWidget *widget = new QWidget(group2);
		QGridLayout *grid = new QGridLayout( widget, 5,2, 6,6, "");
		grid->addRowSpacing( 0, fontMetrics().lineSpacing() );
		grid->addColSpacing( 0, fontMetrics().lineSpacing() );
		// grid->setColStretch(1,1);
		
		QLabel *label1 = new QLabel(i18n("default resolution:"), widget);
		grid->addWidget( label1, 0,0 );
		edit_res= new KLineEdit("",widget);
		grid->addWidget( edit_res, 0,1 );
		
		QLabel *label2 = new QLabel(i18n("(used when the picture offers no resolution)"), widget);
		grid->addWidget( label2, 1,1 );
		
		QLabel *label3 = new QLabel(i18n("bounding box:"), widget);
		grid->addWidget( label3, 2,0 );
		cb_boundingbox = new QCheckBox(i18n("try to determine from the picture"),widget);
		grid->addWidget( cb_boundingbox, 2,1);
		
		QLabel *label4 = new QLabel(i18n("(you have to install the ImageMagick package to use this option)"), widget);
		grid->addWidget( label4, 3,1 );
		
		QLabel *label5 = new QLabel(i18n("ImageMagick:"), widget);
		grid->addWidget( label5, 4,0 );
		QLabel *lb_imagemagick = new QLabel("",widget);
		grid->addWidget( lb_imagemagick, 4,1);
		
		vbox->addWidget(group1);
		vbox->addWidget(group2);
		vbox->addStretch();
		
		//fill in
		m_config->setGroup( "Editor Ext" );
		checkEnv->setChecked(m_config->readBoolEntry( "Complete Environment", true));
		
		m_config->setGroup("IncludeGraphics");
		cb_boundingbox->setChecked( m_config->readBoolEntry("boundingbox", true) );
		edit_res->setText( m_config->readEntry("resolution","300") );
		if ( m_config->readBoolEntry("imagemagick", true) )
			lb_imagemagick->setText("installed");
		else
			lb_imagemagick->setText("not installed");
	}
	
	
	//////////////////// Complete configuration (dani) ////////////////////
	
	void Config::setupCodeCompletion()
	{
		QFrame *page =  addPage(i18n("Complete"),i18n("Complete Configuration"),
					KGlobal::instance()->iconLoader()->loadIcon("source",KIcon::NoGroup,KIcon::SizeMedium)
					);
		
		completePage = new ConfigCodeCompletion(page);
		completePage->readConfig(m_config);
		
		QVBoxLayout *vbox = new QVBoxLayout(page);
		vbox->addWidget(completePage);
	}

	void Config::setupHelp()
	{
		QFrame *page = addPage(i18n("Help"),i18n("Help Configuration"),
					KGlobal::instance()->iconLoader()->loadIcon("help",KIcon::NoGroup,KIcon::SizeMedium)
					);

		helpPage = new KileWidgetHelpConfig(page);
		helpPage->readConfig();
	}

	//////////////////// write new configuration ////////////////////
	
	void Config::slotOk()
	{
		writeGeneralOptionsConfig();
		writeToolsConfig();
		writeSpellingConfig();
		writeLatexConfig();
		completePage->writeConfig(m_config);  // Complete configuration (dani)
		helpPage->writeConfig();

		m_config->sync();

		accept();
	}
	
	void Config::slotCancel()
	{
		m_config->rollback();
		accept();
	}

	void Config::writeGeneralOptionsConfig()
	{
		m_config->setGroup( "Files" );
		m_config->writeEntry("Restore", checkRestore->isChecked());
		m_config->writeEntry("Autosave",checkAutosave->isChecked());
		m_config->writeEntry("AutosaveInterval",asIntervalInput->value()*60000);
		m_config->writeEntry("CleanUpAfterClose", checkCleanUpAfterClose->isChecked());
		m_config->writeEntry("CleanUpFileExtensions", QStringList::split(" ", fileExtensionList->text()));
	
		m_config->setGroup( "User" );
		m_config->writeEntry("Author",templAuthor->text());
		m_config->writeEntry("DocumentClassOptions",templDocClassOpt->text());
		m_config->writeEntry("Template Encoding",templEncoding->text());
	
		m_config->setGroup("Structure");
		m_config->writeEntry("DefaultLevel", spinLevel->value());
	}
	
	void Config::writeToolsConfig()
	{
		m_toolConfig->writeConfig();
	}
	
	void Config::writeSpellingConfig()
	{
		ksc->writeGlobalSettings();
	}
	
	void Config::writeLatexConfig()
	{
		m_config->setGroup( "Editor Ext" );
		m_config->writeEntry("Complete Environment", checkEnv->isChecked());
	
		m_config->setGroup("IncludeGraphics");
		m_config->writeEntry("boundingbox",cb_boundingbox->isChecked());
		m_config->writeEntry("resolution",edit_res->text());  
	}
}

#include "kileconfigdialog.moc"
