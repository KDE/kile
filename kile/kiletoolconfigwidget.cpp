/***************************************************************************
    begin                : Sat 3-1 20:40:00 CEST 2004
    copyright            : (C) 2004 by Jeroen Wijnhout
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

#include "kiletoolconfigwidget.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <q3groupbox.h>
#include <qspinbox.h>
#include <q3vbox.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "kiledebug.h"
#include <k3listbox.h>
#include <keditlistbox.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "toolconfigwidget.h"
#include "processtoolconfigwidget.h"
#include "librarytoolconfigwidget.h"
#include "quicktoolconfigwidget.h"
#include "latextoolconfigwidget.h"
#include "newtoolwizard.h"

namespace KileWidget
{
	ToolConfig::ToolConfig(KileTool::Manager *mngr, QWidget *parent, const char *name) :
		QWidget(parent, name),
		m_manager(mngr)
	{
		m_config = m_manager->config();
		m_layout = new Q3GridLayout(this, 1, 1, 0, 0);
		m_configWidget = new ToolConfigWidget(this);
		m_layout->addWidget(m_configWidget, 0, 0);

		m_tabGeneral = m_configWidget->m_tab->page(0);
		m_tabAdvanced = m_configWidget->m_tab->page(1);
		m_tabMenu = m_configWidget->m_tab->page(2);

		updateToollist();
		// --->m_configWidget->m_lstbTools->setSelected(0, true);
		m_configWidget->m_lstbTools->setSelected(indexQuickBuild(), true);
		connect(m_configWidget->m_cbConfig, SIGNAL(activated(int)), this, SLOT(switchConfig(int)));

		QStringList lst; lst << i18n( "Quick" ) << i18n( "Compile" ) << i18n( "Convert" ) << i18n( "View" ) << i18n( "Other" );
		m_configWidget->m_cbMenu->insertStringList(lst);
		connect(m_configWidget->m_cbMenu, SIGNAL(activated(const QString &)), this, SLOT(setMenu(const QString &)));
		connect(m_configWidget->m_pshbIcon, SIGNAL(clicked()), this, SLOT(selectIcon()));

		connect(m_configWidget->m_pshbRemoveTool, SIGNAL(clicked()), this, SLOT(removeTool()));
		connect(m_configWidget->m_pshbNewTool, SIGNAL(clicked()), this, SLOT(newTool()));
		connect(m_configWidget->m_pshbRemoveConfig, SIGNAL(clicked()), this, SLOT(removeConfig()));
		connect(m_configWidget->m_pshbNewConfig, SIGNAL(clicked()), this, SLOT(newConfig()));
		connect(m_configWidget->m_pshbDefault, SIGNAL(clicked()), this, SLOT(writeDefaults()));

		//--->m_current = m_configWidget->m_lstbTools->text(0);
		m_current = m_configWidget->m_lstbTools->currentText();
		m_manager->retrieveEntryMap(m_current, m_map, false, false);
		QString cfg = KileTool::configName(m_current, m_config);
		m_configWidget->m_cbConfig->insertItem(cfg);

		setupGeneral();
		setupAdvanced();

		switchConfig(cfg);
		switchTo(m_current, false);
		connect(m_configWidget->m_lstbTools, SIGNAL(highlighted(const QString &)), this, SLOT(switchTo(const QString &)));

		connect(this, SIGNAL(changed()), this, SLOT(updateAdvanced()));
		connect(this, SIGNAL(changed()), this, SLOT(updateGeneral()));
	}

	void ToolConfig::setupAdvanced()
	{
		m_configWidget->m_cbType->insertItem(i18n("Run Outside of Kile"));
		m_configWidget->m_cbType->insertItem(i18n("Run in Konsole"));
		m_configWidget->m_cbType->insertItem(i18n("Run Embedded in Kile"));
		m_configWidget->m_cbType->insertItem(i18n("Use HTML Viewer"));
		m_configWidget->m_cbType->insertItem(i18n("Run Sequence of Tools"));
		connect(m_configWidget->m_cbType, SIGNAL(activated(int)), this, SLOT(switchType(int)));
		connect(m_configWidget->m_ckClose, SIGNAL(toggled(bool)), this, SLOT(setClose(bool)));

		m_classes << "Compile" << "Convert" << "Archive" << "View" <<  "Sequence" << "LaTeX" << "ViewHTML" << "ViewBib" << "ForwardDVI" << "Base";
		m_configWidget->m_cbClass->insertStringList(m_classes);
		connect(m_configWidget->m_cbClass, SIGNAL(activated(const QString &)), this, SLOT(switchClass(const QString &)));

		connect(m_configWidget->m_leSource, SIGNAL(textChanged(const QString &)), this, SLOT(setFrom(const QString &)));
		connect(m_configWidget->m_leTarget, SIGNAL(textChanged(const QString &)), this, SLOT(setTo(const QString &)));
		connect(m_configWidget->m_leFile, SIGNAL(textChanged(const QString &)), this, SLOT(setTarget(const QString &)));
		connect(m_configWidget->m_leRelDir, SIGNAL(textChanged(const QString &)), this, SLOT(setRelDir(const QString &)));

		m_configWidget->m_cbState->insertItem("Editor");
		m_configWidget->m_cbState->insertItem("Viewer");
		m_configWidget->m_cbState->insertItem("HTMLpreview");
		connect(m_configWidget->m_cbState, SIGNAL(activated(const QString &)), this, SLOT(setState(const QString &)));
	}

	void ToolConfig::updateAdvanced()
	{
		bool enablekonsoleclose = false;
		QString type = m_map["type"];
		if ( type == "Process" ) m_configWidget->m_cbType->setCurrentItem(0);
		else if ( type == "Konsole" )
		{
			m_configWidget->m_cbType->setCurrentItem(1);
			enablekonsoleclose = true;
		}
		else if ( type == "Part" ) m_configWidget->m_cbType->setCurrentItem(2);
		else if ( type == "DocPart" ) m_configWidget->m_cbType->setCurrentItem(3);
		else if ( type == "Sequence" ) m_configWidget->m_cbType->setCurrentItem(4);
		m_configWidget->m_ckClose->setEnabled(enablekonsoleclose);

		QString state = m_map["state"];
		if ( state.isEmpty() ) state = "Editor";
		m_configWidget->m_cbState->setCurrentText(state);

		int index = m_classes.findIndex(m_map["class"]);
		if ( index == -1 ) index = m_classes.count()-1;
		m_configWidget->m_cbClass->setCurrentItem(index);
		m_configWidget->m_ckClose->setChecked(m_map["close"] == "yes");
		m_configWidget->m_leSource->setText(m_map["from"]);
		m_configWidget->m_leTarget->setText(m_map["to"]);
		m_configWidget->m_leFile->setText(m_map["target"]);
		m_configWidget->m_leRelDir->setText(m_map["relDir"]);
	}

	void ToolConfig::setupGeneral()
	{
		m_configWidget->m_stackBasic->addWidget(new QLabel(i18n("Use the \"Advanced\" tab to configure this tool."), this), GBS_None);

		m_ptcw = new ProcessToolConfigWidget(m_configWidget->m_stackBasic);
		m_configWidget->m_stackBasic->addWidget(m_ptcw, GBS_Process);
		connect(m_ptcw->m_leCommand, SIGNAL(textChanged(const QString &)), this, SLOT(setCommand(const QString &)));
		connect(m_ptcw->m_leOptions, SIGNAL(textChanged(const QString &)), this, SLOT(setOptions(const QString &)));

		m_ltcw = new LibraryToolConfigWidget(m_configWidget->m_stackBasic);
		m_configWidget->m_stackBasic->addWidget(m_ltcw, GBS_Library);
		connect(m_ltcw->m_leLibrary, SIGNAL(textChanged(const QString &)), this, SLOT(setLibrary(const QString &)));
		connect(m_ltcw->m_leLibClass, SIGNAL(textChanged(const QString &)), this, SLOT(setClassName(const QString &)));
		connect(m_ltcw->m_leOptions, SIGNAL(textChanged(const QString &)), this, SLOT(setLibOptions(const QString &)));

		m_qtcw = new QuickToolConfigWidget(m_configWidget->m_stackBasic);
		m_configWidget->m_stackBasic->addWidget(m_qtcw, GBS_Sequence);
		connect(m_qtcw, SIGNAL(sequenceChanged(const QString &)), this, SLOT(setSequence(const QString &)));

		m_configWidget->m_stackBasic->addWidget(new QLabel(i18n("Unknown tool type; your configuration data is malformed.\nPerhaps it is a good idea to restore the default settings."), this), GBS_Error);

		m_configWidget->m_stackExtra->addWidget(new QWidget(this), GES_None);

		m_LaTeXtcw = new LaTeXToolConfigWidget(m_configWidget->m_stackExtra);
		m_configWidget->m_stackExtra->addWidget(m_LaTeXtcw, GES_LaTeX);
		connect(m_LaTeXtcw->m_ckRootDoc, SIGNAL(toggled(bool)), this, SLOT(setLaTeXCheckRoot(bool)));
		connect(m_LaTeXtcw->m_ckJump, SIGNAL(toggled(bool)), this, SLOT(setLaTeXJump(bool)));
		connect(m_LaTeXtcw->m_ckAutoRun, SIGNAL(toggled(bool)), this, SLOT(setLaTeXAuto(bool)));

// 		m_ViewBibtcw = new ViewBibToolConfigWidget(m_configWidget->m_stackExtra);
// 		m_configWidget->m_stackExtra->addWidget(m_ViewBibtcw, GES_ViewBib);
// 		connect(m_ViewBibtcw->m_ckRunLyxServer, SIGNAL(toggled(bool)), this, SLOT(setRunLyxServer(bool)));
	}

	void ToolConfig::updateGeneral()
	{
		QString type = m_map["type"];

		int basicPage = GBS_None;
		int extraPage = GES_None;

		if ( type == "Process" || type == "Konsole" ) basicPage = GBS_Process;
		else if ( type == "Part" ) basicPage = GBS_Library;
		else if ( type == "DocPart" ) basicPage = GBS_None;
		else if ( type == "Sequence" )
		{
			basicPage = GBS_Sequence;
			m_qtcw->updateSequence(m_map["sequence"]);
		}
		else basicPage = GBS_Error;

		QString cls = m_map["class"];
		if ( cls == "LaTeX" )
			extraPage = GES_LaTeX;
// 		else if ( cls == "ViewBib" )
// 			extraPage = GES_ViewBib;

		m_ptcw->m_leCommand->setText(m_map["command"]);
		m_ptcw->m_leOptions->setText(m_map["options"]);

		m_ltcw->m_leLibrary->setText(m_map["libName"]);
		m_ltcw->m_leLibClass->setText(m_map["className"]);
		m_ltcw->m_leOptions->setText(m_map["libOptions"]);

		m_LaTeXtcw->m_ckRootDoc->setChecked(m_map["checkForRoot"] == "yes");
		m_LaTeXtcw->m_ckJump->setChecked(m_map["jumpToFirstError"] == "yes");
		m_LaTeXtcw->m_ckAutoRun->setChecked(m_map["autoRun"] == "yes");

// 		m_config->setGroup("Tools");
// 		m_ViewBibtcw->m_ckRunLyxServer->setChecked(m_config->readBoolEntry("RunLyxServer", true));

		KILE_DEBUG() << "showing pages " << basicPage << " " << extraPage << endl;
		m_configWidget->m_stackBasic->raiseWidget(basicPage);
		m_configWidget->m_stackExtra->raiseWidget(extraPage);

		if ( m_configWidget->m_stackBasic->widget(basicPage) )
		{
			QSize szHint = m_configWidget->m_stackBasic->widget(basicPage)->sizeHint();
            if (szHint.height() > 0)
			     m_configWidget->m_stackBasic->setMaximumHeight(szHint.height());
		}
		if ( m_configWidget->m_stackExtra->widget(extraPage) )
		{
			QSize szHint = m_configWidget->m_stackExtra->widget(extraPage)->sizeHint();
            if (szHint.height() > 0)
			     m_configWidget->m_stackExtra->setMaximumHeight(szHint.height());
		}
		m_configWidget->layout()->invalidate();
	}

	void ToolConfig::writeDefaults()
	{
		if ( KMessageBox::warningContinueCancel(this, i18n("All your tool settings will be overwritten with the default settings, are you sure you want to continue?")) == KMessageBox::Continue )
		{
			QStringList groups = m_config->groupList();
			QRegExp re = QRegExp("Tool/(.+)/.+");
			for ( uint i = 0; i < groups.count(); ++i )
				if ( re.exactMatch(groups[i]) )
					m_config->deleteGroup(groups[i],true);
			
			m_manager->factory()->writeStdConfig();
			m_config->sync();
			updateToollist();
  			QStringList tools = KileTool::toolList(m_config, true);
			for ( uint i = 0; i < tools.count(); ++i){
				switchTo(tools[i], false);// needed to retrieve the new map
 				switchTo(tools[i],true); // this writes the newly retrieved entry map (and not an perhaps changed old one)
			}
			int index = indexQuickBuild();
			switchTo(tools[index], false);
			m_configWidget->m_lstbTools->setSelected(index, true);
		}
	}

	void ToolConfig::updateToollist()
	{
		//KILE_DEBUG() << "==ToolConfig::updateToollist()====================" << endl;
		m_configWidget->m_lstbTools->clear();
		m_configWidget->m_lstbTools->insertStringList(KileTool::toolList(m_config, true));
		m_configWidget->m_lstbTools->sort();
	}

	void ToolConfig::setMenu(const QString & menu)
	{
		//KILE_DEBUG() << "==ToolConfig::setMenu(const QString & menu)====================" << endl;
		m_map["menu"] = menu;
	}

	void ToolConfig::writeConfig()
	{
		//KILE_DEBUG() << "==ToolConfig::writeConfig()====================" << endl;
		//save config
		m_manager->saveEntryMap(m_current, m_map, false, false);
		KileTool::setGUIOptions(m_current, m_configWidget->m_cbMenu->currentText(), m_icon, m_config);
	}

	int ToolConfig::indexQuickBuild()
	{
		int index = m_configWidget->m_lstbTools->index( m_configWidget->m_lstbTools->findItem("QuickBuild",Qt::ExactMatch) );
		
		return ( index >= 0 ) ? index : 0;
	}
	
	void ToolConfig::switchConfig(int /*index*/)
	{
		//KILE_DEBUG() << "==ToolConfig::switchConfig(int /*index*/)====================" << endl;
		switchTo(m_current);
	}

	void ToolConfig::switchConfig(const QString & cfg)
	{
		//KILE_DEBUG() << "==ToolConfig::switchConfig(const QString & cfg)==========" << endl;
		for ( int i = 0; i < m_configWidget->m_cbConfig->count(); ++i)
		{
			if ( m_configWidget->m_cbConfig->text(i) == cfg )
				m_configWidget->m_cbConfig->setCurrentItem(i);
		}
	}

	void ToolConfig::switchTo(const QString & tool, bool save /* = true */)
	{
		//KILE_DEBUG() << "==ToolConfig::switchTo(const QString & tool, bool save /* = true */)====================" << endl;
		//save config
		if (save)
		{
			writeConfig();

			//update the config number
			QString cf = m_configWidget->m_cbConfig->currentText();
			KileTool::setConfigName(m_current, cf, m_config);
		}

		m_current = tool;

		m_map.clear();
		if (!m_manager->retrieveEntryMap(m_current, m_map, false, false))
			kWarning() << "no entrymap" << endl;

		updateConfiglist();
		updateGeneral();
		updateAdvanced();

		//show GUI info
		m_configWidget->m_cbMenu->setCurrentText(KileTool::menuFor(m_current, m_config));
		m_icon=KileTool::iconFor(m_current, m_config);
		if ( m_icon.isEmpty() )
			m_configWidget->m_pshbIcon->setPixmap(QString::null);
		else
			m_configWidget->m_pshbIcon->setPixmap(SmallIcon(m_icon));
	}

	void ToolConfig::updateConfiglist()
	{
		//KILE_DEBUG() << "==ToolConfig::updateConfiglist()=====================" << endl;
		m_configWidget->m_cbConfig->clear();
		m_configWidget->m_cbConfig->insertStringList(KileTool::configNames(m_current, m_config));
		QString cfg = KileTool::configName(m_current, m_config);
		switchConfig(cfg);
		m_configWidget->m_cbConfig->setEnabled(m_configWidget->m_cbConfig->count() > 1);
	}

	void ToolConfig::selectIcon()
	{
		KILE_DEBUG() << "icon ---> " << m_icon << endl;
		//KILE_DEBUG() << "==ToolConfig::selectIcon()=====================" << endl;
		KIconDialog *dlg = new KIconDialog(this);
		QString res = dlg->openDialog();
		if ( m_icon != res ) {
			if ( res.isEmpty() ) return;
		
			m_icon = res;
			writeConfig();
			if ( m_icon.isEmpty() )
				m_configWidget->m_pshbIcon->setPixmap(QString::null);
			else
				m_configWidget->m_pshbIcon->setPixmap(SmallIcon(m_icon));
		}
	}

	void ToolConfig::newTool()
	{
		//KILE_DEBUG() << "==ToolConfig::newTool()=====================" << endl;
		NewToolWizard *ntw = new NewToolWizard(this);
		if (ntw->exec())
		{
			QString toolName = ntw->toolName();
			QString parentTool = ntw->parentTool();

			writeStdConfig(toolName, "Default");
			if ( parentTool != ntw->customTool() )
			{
				//copy tool info
				KileTool::Config tempMap;
				m_manager->retrieveEntryMap(parentTool, tempMap, false, false);
				m_config->setGroup(KileTool::groupFor(toolName, "Default"));
				m_config->writeEntry("class", tempMap["class"]);
				m_config->writeEntry("type", tempMap["type"]);
				m_config->writeEntry("state", tempMap["state"]);
				m_config->writeEntry("close", tempMap["close"]);
				m_config->writeEntry("checkForRoot", tempMap["checkForRoot"]);
				m_config->writeEntry("autoRun", tempMap["autoRun"]);
				m_config->writeEntry("jumpToFirstError", tempMap["jumpToFirstError"]);
			}

			m_configWidget->m_lstbTools->blockSignals(true);
			updateToollist();
			switchTo(toolName);
			for ( uint i = 0; i < m_configWidget->m_lstbTools->count(); ++i)
				if ( m_configWidget->m_lstbTools->text(i) == toolName )
				{
					m_configWidget->m_lstbTools->setCurrentItem(i);
					break;
				}
			m_configWidget->m_lstbTools->blockSignals(false);
		}
	}

	void ToolConfig::newConfig()
	{
		//KILE_DEBUG() << "==ToolConfig::newConfig()=====================" << endl;
		writeConfig();
		bool ok;
		QString cfg = KInputDialog::getText(i18n("New Configuration"), i18n("Enter new configuration name:"), "", &ok, this);
		if (ok && (!cfg.isEmpty()))
		{
			//copy config
			m_config->setGroup(KileTool::groupFor(m_current, cfg));
			for (QMap<QString,QString>::Iterator it  = m_map.begin(); it != m_map.end(); ++it)
			{
				m_config->writeEntry(it.key(), it.data());
			}
			KileTool::setConfigName(m_current, cfg, m_config);
			switchTo(m_current, false);
			switchConfig(cfg);
		}
	}

	void ToolConfig::writeStdConfig(const QString & tool, const QString & cfg)
	{
		m_config->setGroup(KileTool::groupFor(tool, cfg));
		m_config->writeEntry("class", "Compile");
		m_config->writeEntry("type", "Process");
		m_config->writeEntry("menu", "Compile");
		m_config->writeEntry("state", "Editor");
		m_config->writeEntry("close", "no");

		m_config->setGroup("Tools");
		m_config->writeEntry(tool, cfg);
	}

	void ToolConfig::removeTool()
	{
		//KILE_DEBUG() << "==ToolConfig::removeTool()=====================" << endl;
		if ( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to remove the tool %1?").arg(m_current)) == KMessageBox::Continue )
		{
			KConfig *config = m_config;
			QStringList cfgs = KileTool::configNames(m_current, config);
			for ( uint i = 0; i < cfgs.count(); ++i)
			{
				config->deleteGroup(KileTool::groupFor(m_current, cfgs[i]));
			}
			config->setGroup("Tools");
			config->deleteEntry(m_current);
			int index = m_configWidget->m_lstbTools->currentItem()-1;
			if ( index < 0 ) index=0;
			QString tool = m_configWidget->m_lstbTools->text(index);
			m_configWidget->m_lstbTools->blockSignals(true);
			updateToollist();
			m_configWidget->m_lstbTools->setCurrentItem(index);
			switchTo(tool, false);
			m_configWidget->m_lstbTools->blockSignals(false);
		}
	}

	void ToolConfig::removeConfig()
	{
		//KILE_DEBUG() << "==ToolConfig::removeConfig()=====================" << endl;
		writeConfig();
		if ( m_configWidget->m_cbConfig->count() > 1)
		{
			if ( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to remove this configuration?") ) == KMessageBox::Continue )
			{
				m_config->deleteGroup(KileTool::groupFor(m_current, m_configWidget->m_cbConfig->currentText()));
				updateConfiglist();
				KileTool::setConfigName(m_current, m_configWidget->m_cbConfig->text(0), m_config);
				switchTo(m_current, false);
			}
		}
		else
			KMessageBox::error(this, i18n("You need at least one configuration for each tool."), i18n("Cannot Remove Configuration"));
	}

	void ToolConfig::switchClass(const QString & cls)
	{
		if ( m_map["class"] != cls )
		{
			setClass(cls);
			emit(changed());
		}
	}

	void ToolConfig::switchType(int index)
	{
		switch (index)
		{
		case 0 : m_map["type"] = "Process"; break;
		case 1 : m_map["type"] = "Konsole"; break;
		case 2 : m_map["type"] = "Part"; break;
		case 3 : m_map["type"] = "DocPart"; break;
		case 4 : m_map["type"] = "Sequence"; break;
		default : m_map["type"] = "Process"; break;
		}
		emit(changed());
	}

	void ToolConfig::setCommand(const QString & command) { m_map["command"] = command.trimmed(); }
	void ToolConfig::setOptions(const QString & options) { m_map["options"] = options.trimmed(); }
	void ToolConfig::setLibrary(const QString & lib) { m_map["libName"] = lib.trimmed(); }
	void ToolConfig::setLibOptions(const QString & options) { m_map["libOptions"] = options.trimmed(); }
	void ToolConfig::setClassName(const QString & name) { m_map["className"] = name.trimmed(); }
	void ToolConfig::setState(const QString & state)
	{
		QString str = state.trimmed();
		if ( str .isEmpty() ) str = "Editor";
		m_map["state"] = str;
	}
	void ToolConfig::setSequence(const QString & sequence) { m_map["sequence"] = sequence.trimmed(); }
	void ToolConfig::setClose(bool on) { m_map["close"] = on ? "yes" : "no"; }
	void ToolConfig::setTarget(const QString & trg) { m_map["target"] = trg.trimmed(); }
	void ToolConfig::setRelDir(const QString & rd) { m_map["relDir"] = rd.trimmed(); }
	void ToolConfig::setLaTeXCheckRoot(bool ck) { m_map["checkForRoot"] = ck ? "yes" : "no"; }
	void ToolConfig::setLaTeXJump(bool ck) { m_map["jumpToFirstError"] = ck ? "yes" : "no"; }
	void ToolConfig::setLaTeXAuto(bool ck) { m_map["autoRun"] = ck ? "yes" : "no"; }
	void ToolConfig::setRunLyxServer(bool ck)
	{
		//KILE_DEBUG() << "setRunLyxServer" << endl;
		m_config->setGroup("Tools");
		m_config->writeEntry("RunLyxServer", ck);
	}
	void ToolConfig::setFrom(const QString & from) { m_map["from"] = from.trimmed(); }
	void ToolConfig::setTo(const QString & to) { m_map["to"] = to.trimmed(); }
	void ToolConfig::setClass(const QString & cls) { m_map["class"] = cls.trimmed(); }
}

#include "kiletoolconfigwidget.moc"
