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

#include <qlayout.h>

#include <kdeversion.h>
#include <klocale.h>
#include <ksconfig.h>

#include "kiletoolmanager.h"
#include "kiletoolconfigwidget.h"
#include "kileconfigdialog.h"
#include "helpconfigwidget.h"
#include "latexconfigwidget.h"
#include "generalconfigwidget.h"
#include "encodingconfigwidget.h"
#include "kileconfig.h"

namespace KileDialog
{
	Config::Config(KConfig *config, KileTool::Manager *mngr, QWidget* parent)
		:KConfigDialog(parent, "kileconfiguration", KileConfig::self(), IconList, Ok|Cancel, Ok, true),
		m_config(config),
		m_toolMngr(mngr)
	{
		m_config->sync();
		setShowIconsInTreeList(true);

		// setup all configuration pages
		setupGeneralOptions();
		setupEncodingOptions();
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
		QFrame *page = new QFrame(0, "General");

		generalPage = new KileWidgetGeneralConfig(page, "LaTeX");

		QVBoxLayout *vbox = new QVBoxLayout(page);
		vbox->addWidget(generalPage);
		vbox->addStretch();

		addPage(page, i18n("General"), "configure");
	}
	
	void Config::setupEncodingOptions()
	{
		QFrame *page = new QFrame(0, "Encoding");

		encodingPage = new KileWidgetEncodingConfig(page, "Encoding");

		encodingPage->setEncoding(KileConfig::defaultEncoding());
		
		QVBoxLayout *vbox = new QVBoxLayout(page);
		vbox->addWidget(encodingPage);
		vbox->addStretch();

		addPage(page, i18n("Encoding"), "gear");
	}

	//////////////////// Tools Configuration ////////////////////

	void Config::setupTools()
	{
		toolsPage = new QFrame(0, "build");

		QVBoxLayout *toolsLayout = new QVBoxLayout(toolsPage, 5);
		m_toolConfig = new KileWidget::ToolConfig(m_toolMngr, toolsPage);
		toolsLayout->addWidget(m_toolConfig);

		addPage(toolsPage, i18n("Build"), "gear");
	}

	//////////////////// Spelling Configuration ////////////////////

	void Config::setupSpelling()
	{
		spellingPage = new QFrame(0, "Spelling");

		QGridLayout *gbox2 = new QGridLayout( spellingPage,2,2,5,5,"" );
		gbox2->addRowSpacing( 0, fontMetrics().lineSpacing() );

		QGroupBox* GroupBox3= new QGroupBox(2,Qt::Horizontal,i18n("Spelling"),spellingPage, "ButtonGroup" );
		ksc = new KSpellConfig(GroupBox3, "spell",0, false );

		gbox2->addMultiCellWidget(GroupBox3,0,0,0,1,0);

		addPage(spellingPage, i18n("Spelling"), "spellcheck");
	}

	//////////////////// LaTeX specific editing options ////////////////////

	void Config::setupLatex()
	{
		QFrame *page = new QFrame(this, "codecompframe");
		latexPage = new KileWidgetLatexConfig(page, "LaTeX");

		QVBoxLayout *vbox = new QVBoxLayout(page);
		vbox->addWidget(latexPage);
		vbox->addStretch();

		addPage(page, i18n("LaTeX"), "tex");
	}


	//////////////////// Complete configuration (dani) ////////////////////

	void Config::setupCodeCompletion()
	{
		QFrame *page = new QFrame(this, "codecompframe");
		completePage = new ConfigCodeCompletion(m_config,page);
		completePage->readConfig();

		QVBoxLayout *vbox = new QVBoxLayout(page);
		vbox->addWidget(completePage);

		addPage(page, i18n("Complete"), "source");
	}

	void Config::setupHelp()
	{
		QFrame *page = new QFrame(this, "helpframe");
		helpPage = new KileWidgetHelpConfig(page);

		QVBoxLayout *vbox = new QVBoxLayout(page);
		vbox->addWidget(helpPage);
		vbox->addStretch();

		addPage(page, i18n("Help"), "help");
	}

	//////////////////// write new configuration ////////////////////

	void Config::slotOk()
	{
		writeToolsConfig();
		writeSpellingConfig();
		completePage->writeConfig();  // Complete configuration (dani)

		KileConfig::setDefaultEncoding(encodingPage->encoding());
		
		m_config->sync();

		emit okClicked(); // Otherwise, the KConfigXT machine doesn't start...
		accept();
	}

	void Config::slotCancel()
	{
		m_config->rollback();
		accept();
	}

	void Config::writeToolsConfig()
	{
		m_toolConfig->writeConfig();
	}

	void Config::writeSpellingConfig()
	{
		ksc->writeGlobalSettings();
	}
}

#include "kileconfigdialog.moc"
