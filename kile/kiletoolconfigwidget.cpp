/***************************************************************************
                          kiletoolconfigwidget.cpp  -  description
                             -------------------
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

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klistbox.h>
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

#include "kiletoolconfigwidget.h"
#include "kiletoolmanager.h"
#include "kilestdtools.h"

namespace KileWidget
{
	ToolConfig::ToolConfig(KileTool::Manager *mngr, QWidget *parent, const char *name) :
		QWidget(parent, name),
		m_manager(mngr),
		m_basic(0L),
		m_advanced(0L),
		m_bAdvanced(false)
	{
		m_layout = new QGridLayout(this, 6, 6, 0, 10); m_layout->setColStretch(0, 0);
		m_layout->setRowStretch(0, 1); m_layout->setRowStretch(1, 1); m_layout->setRowStretch(2, 0); m_layout->setRowStretch(3, 2);
		m_layout->setRowStretch(4, 1); m_layout->setRowStretch(5, 1);

		m_lstbTools = new KListBox(this, "listbox"); m_layout->addMultiCellWidget(m_lstbTools, 0, 5, 0, 0, Qt::AlignLeft);
		m_lstbTools->setMaximumWidth(120);
		m_lstbTools->setMinimumWidth(120);
		updateToollist();

		m_lbName = new QLabel(this); m_layout->addMultiCellWidget(m_lbName, 0, 0, 1, 4);

		QLabel *lb = new QLabel(i18n("C&hoose a predefined configuration: "), this); m_layout->addWidget(lb, 1, 1, Qt::AlignLeft);
		m_cbPredef = new KComboBox(this); m_layout->addMultiCellWidget(m_cbPredef, 1, 1, 2, 4/*, Qt::AlignLeft*/);
		lb->setBuddy(m_cbPredef);
		connect(m_cbPredef, SIGNAL(activated(int)), this, SLOT(switchConfig(int)));

		m_pshbAdvanced = new KPushButton(i18n("&Advanced") +" >>", this); m_layout->addWidget(m_pshbAdvanced, 3, 1, Qt::AlignLeft);
		m_pshbAdvanced->setMaximumHeight(m_pshbAdvanced->sizeHint().height());
		connect(m_pshbAdvanced, SIGNAL(clicked()), this, SLOT(toggleAdvanced()));

		QGroupBox *grp = new QGroupBox(1, Qt::Vertical, this); m_layout->addMultiCellWidget(grp, 5, 5, 1, 2, Qt::AlignLeft);
		grp->setFrameStyle(QFrame::NoFrame);
		lb = new QLabel(i18n("&Menu:"), grp);
		m_cbMenu = new KComboBox(grp);
		QStringList lst; lst << "Quick" << "Compile" << "Convert" << "View" << "Other";
		m_cbMenu->insertStringList(lst);
		lb->setBuddy(m_cbMenu);
		m_pshbIcon = new KPushButton(grp);
		connect(m_cbMenu, SIGNAL(activated(const QString &)), this, SLOT(setMenu(const QString &)));
		connect(m_pshbIcon, SIGNAL(clicked()), this, SLOT(selectIcon()));

		QVBox *box = new QVBox(this); box->setSpacing(10);
		m_layout->addMultiCellWidget(box, 0, m_layout->numRows() - 1, m_layout->numCols()-1, m_layout->numCols()-1);
		KPushButton *pb = new KPushButton(i18n("Remove Tool"), box); pb->setMaximumHeight(pb->sizeHint().height());
		connect(pb, SIGNAL(clicked()), this, SLOT(removeTool()));

		pb = new KPushButton(i18n("New Tool..."), box); pb->setMaximumHeight(pb->sizeHint().height());
		connect(pb, SIGNAL(clicked()), this, SLOT(newTool()));

		pb = new KPushButton(i18n("Remove Config"), box); pb->setMaximumHeight(pb->sizeHint().height());
		connect(pb, SIGNAL(clicked()), this, SLOT(removeConfig()));

		pb = new KPushButton(i18n("New Config..."), box); pb->setMaximumHeight(pb->sizeHint().height());
		connect(pb, SIGNAL(clicked()), this, SLOT(newConfig()));

		pb = new KPushButton(i18n("&Default Settings"), box); pb->setMaximumHeight(pb->sizeHint().height());
		connect(pb, SIGNAL(clicked()), this, SLOT(writeDefaults()));

		m_current = m_lstbTools->text(0); m_manager->retrieveEntryMap(m_current, m_map, false, false);
		QString cfg = KileTool::configName(m_current, m_manager->config());
		m_cbPredef->insertItem(cfg);
		switchConfig(cfg);
		switchTo(m_current, false);
		connect(m_lstbTools, SIGNAL(highlighted(const QString &)), this, SLOT(switchTo(const QString &)));
	}

	void ToolConfig::writeDefaults()
	{
		if ( KMessageBox::warningContinueCancel(this, i18n("All your tool settings will be overwritten with the default settings, are you sure you want to continue?")) == KMessageBox::Continue )
		{
			m_manager->factory()->writeStdConfig();
			QStringList tools = KileTool::toolList(m_manager->config(), true);
			for ( uint i = 0; i < tools.count(); i++)
				switchTo(tools[i], false);
		}
	}

	void ToolConfig::updateToollist()
	{
		//kdDebug() << "==ToolConfig::updateToollist()====================" << endl;
		m_lstbTools->clear();
		m_lstbTools->insertStringList(KileTool::toolList(m_manager->config(), true)); m_lstbTools->sort();
	}

	void ToolConfig::setMenu(const QString & menu)
	{
		//kdDebug() << "==ToolConfig::setMenu(const QString & menu)====================" << endl;
		m_map["menu"] = menu;
	}

	void ToolConfig::writeConfig()
	{
		//kdDebug() << "==ToolConfig::writeConfig()====================" << endl;
		//save config
		m_manager->saveEntryMap(m_current, m_map, false);
		KileTool::setGUIOptions(m_current, m_cbMenu->currentText(), m_icon, m_manager->config());
	}

	void ToolConfig::switchConfig(int /*index*/)
	{
		//kdDebug() << "==ToolConfig::switchConfig(int /*index*/)====================" << endl;
		switchTo(m_current);
	}

	void ToolConfig::switchConfig(const QString & cfg)
	{
		//kdDebug() << "==ToolConfig::switchConfig(const QString & cfg)==========" << endl;
		for ( int i = 0; i < m_cbPredef->count(); i++)
		{
			if ( m_cbPredef->text(i) == cfg )
				m_cbPredef->setCurrentItem(i);
		}
	}

	void ToolConfig::switchTo(const QString & tool, bool save /* = true */)
	{
		//kdDebug() << "==ToolConfig::switchTo(const QString & tool, bool save /* = true */)====================" << endl;
		//save config
		if (save)
		{
			writeConfig();

			//update the config number
			QString cf = m_cbPredef->currentText();
			KileTool::setConfigName(m_current, cf, m_manager->config());
		}

		m_current = tool;

		m_lbName->setText("<center><h2>"+m_current+"</h2></center>");
		if (m_basic)
		{
			m_layout->remove(m_basic);
			m_basic->deleteLater();
		}
		if (m_advanced)
		{
			m_layout->remove(m_advanced);
			m_advanced->deleteLater();
		}
		m_map.clear();
		if (!m_manager->retrieveEntryMap(m_current, m_map, false, false))
			kdWarning() << "no entrymap" << endl;

		updateConfiglist();

		m_basic = new BasicTool(m_current, m_manager->config(), &m_map, this);
		m_layout->addMultiCellWidget(m_basic, 2, 2, 1, m_layout->numCols()-2, Qt::AlignLeft);
		m_basic->show();
		//kdDebug() << "after new BasicTool()" << endl;

		m_advanced = new AdvancedTool(m_current, &m_map, this);
		connect(m_advanced, SIGNAL(changed()), this, SLOT(switchConfig()));
		m_layout->addMultiCellWidget(m_advanced, 4, 4, 1, m_layout->numCols()-2, Qt::AlignLeft);
		if (m_bAdvanced) m_advanced->show();
		else m_advanced->hide();
		m_layout->invalidate();
		//kdDebug() << "after new AdvancedTool()" << endl;

		//show GUI info
		m_cbMenu->setCurrentText(KileTool::menuFor(m_current, m_manager->config()));
		m_icon=KileTool::iconFor(m_current, m_manager->config());
		m_pshbIcon->setPixmap(SmallIcon(m_icon));
	}

	void ToolConfig::updateConfiglist()
	{
		//kdDebug() << "==ToolConfig::updateConfiglist()=====================" << endl;
		m_cbPredef->clear();
		m_cbPredef->insertStringList(KileTool::configNames(m_current, m_manager->config()));
		QString cfg = KileTool::configName(m_current, m_manager->config());
		switchConfig(cfg);
		m_cbPredef->setEnabled(m_cbPredef->count() > 1);
	}

	void ToolConfig::toggleAdvanced()
	{
		//kdDebug() << "==ToolConfig::toggleAdvanced()=====================" << endl;
		m_bAdvanced = !m_bAdvanced;
		if (m_advanced)
		{
			if (m_bAdvanced) m_advanced->show();
			else m_advanced->hide();
		}
		m_pshbAdvanced->setText( m_bAdvanced ? i18n("Advanced") + " <<" : i18n("Advanced") + " >>" );
	}

	void ToolConfig::selectIcon()
	{
		//kdDebug() << "==ToolConfig::selectIcon()=====================" << endl;
		KIconDialog *dlg = new KIconDialog(this);
		QString res = dlg->openDialog();
		m_icon=res;

		writeConfig();
		m_pshbIcon->setPixmap(SmallIcon(m_icon));
	}

	void ToolConfig::newTool()
	{
		//kdDebug() << "==ToolConfig::newTool()=====================" << endl;
		bool ok;
		KConfig *config = m_manager->config();
		QString tool = KInputDialog::getText(i18n("New Tool"), i18n("Enter new tool name:"),"", &ok, this);
		if ( ok && tool != "")
		{
			if ( config->hasGroup(KileTool::groupFor(tool, config)) )
			{
				KMessageBox::error(this, i18n("A tool by the name %1 already exists.").arg(tool));
				return;
			}

			writeStdConfig(tool, "Default");

			m_lstbTools->blockSignals(true);
			updateToollist();
			switchTo(tool);
			for ( uint i = 0; i < m_lstbTools->count(); i++)
				if ( m_lstbTools->text(i) == tool )
				{
					m_lstbTools->setCurrentItem(i);
					break;
				}
			m_lstbTools->blockSignals(false);
		}
	}

	void ToolConfig::newConfig()
	{
		//kdDebug() << "==ToolConfig::newConfig()=====================" << endl;
		writeConfig();
		bool ok;
		QString cfg = KInputDialog::getText(i18n("New Configuration"), i18n("Enter new configuration name:"), "", &ok, this);
		if (ok && cfg != "")
		{
			//copy config
			m_manager->config()->setGroup(KileTool::groupFor(m_current, cfg));
			for (QMap<QString,QString>::Iterator it  = m_map.begin(); it != m_map.end(); ++it)
			{
				m_manager->config()->writeEntry(it.key(), it.data());
			}
			KileTool::setConfigName(m_current, cfg, m_manager->config());
			switchTo(m_current, false);
			switchConfig(cfg);
		}
	}

	void ToolConfig::writeStdConfig(const QString & tool, const QString & cfg)
	{
		//kdDebug() << "==ToolConfig::writeStdConfig(const QString & tool, const QString & cfg)=====================" << endl;
		KConfig *config = m_manager->config();
		config->setGroup(KileTool::groupFor(tool, cfg));
		config->writeEntry("class", "Compile");
		config->writeEntry("type", "Process");
		config->writeEntry("menu", "Compile");
		config->writeEntry("state", "Editor");

		config->setGroup("Tools");
		config->writeEntry(tool, cfg);
	}

	void ToolConfig::removeTool()
	{
		//kdDebug() << "==ToolConfig::removeTool()=====================" << endl;
		if ( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to remove the tool %1?").arg(m_current)) == KMessageBox::Continue )
		{
			KConfig *config = m_manager->config();
			QStringList cfgs = KileTool::configNames(m_current, config);
			for ( uint i = 0; i < cfgs.count(); i++)
			{
				config->deleteGroup(KileTool::groupFor(m_current, cfgs[i]));
			}
			config->setGroup("Tools");
			config->deleteEntry(m_current);
			int index = m_lstbTools->currentItem()-1;
			if ( index < 0 ) index=0;
			QString tool = m_lstbTools->text(index);
			m_lstbTools->blockSignals(true);
			updateToollist();
			m_lstbTools->setCurrentItem(index);
			switchTo(tool, false);
			m_lstbTools->blockSignals(false);
		}
	}

	void ToolConfig::removeConfig()
	{
		//kdDebug() << "==ToolConfig::removeConfig()=====================" << endl;
		writeConfig();
		if ( m_cbPredef->count() > 1)
		{
			if ( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to remove this configuration?") ) == KMessageBox::Continue )
			{
				m_manager->config()->deleteGroup(KileTool::groupFor(m_current, m_cbPredef->currentText()));
				updateConfiglist();
				KileTool::setConfigName(m_current, m_cbPredef->text(0), m_manager->config());
				switchTo(m_current, false);
			}
		}
		else
			KMessageBox::error(this, i18n("You need at least one configuration for each tool."), i18n("Cannot Remove Configuration"));
	}

	BasicTool::BasicTool(const QString & tool, KConfig *config, KileTool::Config *map, QWidget *parent) :
		QWidget(parent), m_tool(tool), m_map(map),  m_config(config),m_elbSequence(0L)
	{
		//kdDebug() << "==BasicTool::BasicTool()=====================" << endl;
		m_layout = new QGridLayout(this, 3, 2, 0, 10);
		m_layout->setColStretch(0, 0); m_layout->setColStretch(1, 1);
		QString type = (*m_map)["type"];
		if ( type == "Process" ) createProcess("");
		else if ( type == "Konsole" ) createKonsole();
		else if ( type == "Part" ) createPart();
		else if ( type == "DocPart" ) createDocPart();
		else if ( type == "Sequence" ) createSequence();
		else
		{
			m_layout->addWidget(new QLabel(i18n("Unknown tool type; your configuration data is malformed."), this), 0, 0, Qt::AlignLeft);
			m_layout->addWidget(new QLabel(i18n("Perhaps it is a good idea to restore the default settings."), this), 1, 0, Qt::AlignLeft);
		}

		QString cls = (*m_map)["class"];
		if ( cls == "LaTeX" )
			createLaTeX();
		else if ( cls == "ViewBib" )
			createViewBib();
	}

	void BasicTool::createProcess(const QString & str)
	{
		//kdDebug() << "==BasicTool::createProcess(const QString & str)=====================" << endl;
		int row = 0;
		QLabel *lb = new QLabel(str, this); m_layout->addMultiCellWidget(lb, row, row, 0, 1, Qt::AlignLeft);

		lb = new QLabel(i18n("Co&mmand:"), this); m_layout->addWidget(lb, row+1, 0, Qt::AlignLeft);
		KLineEdit *le = new KLineEdit(this); m_layout->addMultiCellWidget(le, row+1, row+1, 1, 1, Qt::AlignLeft);
		lb->setBuddy(le); le->setMinimumWidth(250);
		le->setText((*m_map)["command"]);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setCommand(const QString &)));

		lb = new QLabel(i18n("&Options:"), this); m_layout->addWidget(lb, row+2, 0, Qt::AlignLeft);
		le = new KLineEdit(this); m_layout->addMultiCellWidget(le, row+2, row+2, 1, 1, Qt::AlignLeft);
		lb->setBuddy(le); le->setMinimumWidth(250);
		le->setText((*m_map)["options"]);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setOptions(const QString &)));
	}

	void BasicTool::createKonsole()
	{
		//kdDebug() << "==BasicTool::createKonsole()=====================" << endl;
		createProcess(i18n("A Konsole window will be opened to start this tool in."));

		int row = m_layout->numRows();

		QCheckBox *ckClose = new QCheckBox(i18n("Close Konsole when tool is finished"), this);
		m_layout->addMultiCellWidget(ckClose, row, row, 0, 1, Qt::AlignLeft);
		connect(ckClose, SIGNAL(toggled(bool)), this, SLOT(setClose(bool)));
		ckClose->setChecked((*m_map)["close"] == "yes");
		setClose(ckClose->isChecked());
	}

	void BasicTool::createPart()
	{
		//kdDebug() << "==BasicTool::createPart()=====================" << endl;
		QLabel *lb = new QLabel(i18n("This tool will be started as an embedded component in Kile."), this); m_layout->addMultiCellWidget(lb, 0, 0, 0, 6, Qt::AlignLeft);

		int row = m_layout->numRows();

		lb = new QLabel(i18n("&Library:"), this); m_layout->addWidget(lb, row, 0, Qt::AlignLeft);
		KLineEdit *le = new KLineEdit(this); m_layout->addMultiCellWidget(le, row, row, 1, 1, Qt::AlignLeft);
		lb->setBuddy(le); le->setMinimumWidth(250);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setLibrary(const QString &)));
		le->setText((*m_map)["libName"]);

		lb = new QLabel(i18n("C&lass:"), this); m_layout->addWidget(lb, row + 1, 0, Qt::AlignLeft);
		le = new KLineEdit(this); m_layout->addMultiCellWidget(le, row+1, row+1, 1, 1,Qt::AlignLeft);
		lb->setBuddy(le); le->setMinimumWidth(250);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setClassName(const QString &)));
		le->setText((*m_map)["className"]);

		lb = new QLabel(i18n("&Options:"), this); m_layout->addWidget(lb, row, 0, Qt::AlignLeft);
		le = new KLineEdit(this); m_layout->addWidget(le, row, 1, Qt::AlignLeft);
		lb->setBuddy(le); le->setMinimumWidth(250);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setLibOptions(const QString &)));
		le->setText((*m_map)["libOptions"]);
	}

	void BasicTool::createDocPart()
	{
		//kdDebug() << "==BasicTool::createDocPart()=====================" << endl;
	}

	void BasicTool::createSequence()
	{
		//kdDebug() << "==BasicTool::createSequence()=====================" << endl;
		int row = m_layout->numRows();

		m_layout->addMultiCellWidget(new QuickTool(m_map, m_config, this), row, row, 0, m_layout->numCols()-1);
	}

	void BasicTool::createLaTeX()
	{
		//kdDebug() << "==BasicTool::createLaTeX()=====================" << endl;
		int row = m_layout->numRows();

		QCheckBox *cbRoot = new QCheckBox(i18n("Check if root document is a LaTeX root before running LaTeX on it."), this);
		m_layout->addMultiCellWidget(cbRoot, row, row, 0, m_layout->numCols()-1, Qt::AlignLeft);
		cbRoot->setChecked((*m_map)["checkForRoot"] == "yes");
		connect(cbRoot, SIGNAL(toggled(bool)), this, SLOT(setLaTeXCheckRoot(bool)));

		QCheckBox *cbJump = new QCheckBox(i18n("Jump to first error in case running LaTeX failed."), this);
		m_layout->addMultiCellWidget(cbJump, row + 1, row + 1, 0, m_layout->numCols()-1, Qt::AlignLeft);
		cbJump->setChecked((*m_map)["jumpToFirstError"] == "yes");
		connect(cbJump, SIGNAL(toggled(bool)), this, SLOT(setLaTeXJump(bool)));

		QCheckBox *cbAuto = new QCheckBox(i18n("Automatically run BibTeX, MakeIndex, rerun LaTeX when necessary."), this);
		m_layout->addMultiCellWidget(cbAuto, row + 2, row + 2, 0, m_layout->numCols()-1, Qt::AlignLeft);
		cbAuto->setChecked((*m_map)["autoRun"] == "yes");
		connect(cbAuto, SIGNAL(toggled(bool)), this, SLOT(setLaTeXAuto(bool)));
	}

	void BasicTool::createViewBib()
	{
		//kdDebug() << "==BasicTool::createViewBib()=====================" << endl;
		int row = m_layout->numRows();

		QCheckBox *cbLyxServer = new QCheckBox(i18n("Let Kile process LyX commands sent by bibliography editors/viewers."), this);
		m_layout->addMultiCellWidget(cbLyxServer, row, row, 0, m_layout->numCols()-1, Qt::AlignLeft);
		m_config->setGroup("Tools");
		cbLyxServer->setChecked(m_config->readBoolEntry("RunLyxServer", true));
		connect(cbLyxServer, SIGNAL(toggled(bool)), this, SLOT(setRunLyxServer(bool)));
	}

	void BasicTool::setCommand(const QString & command) { (*m_map)["command"] = command; }
	void BasicTool::setOptions(const QString & options) { (*m_map)["options"] = options; }
	void BasicTool::setLibrary(const QString & lib) { (*m_map)["libName"] = lib; }
	void BasicTool::setLibOptions(const QString & options) { (*m_map)["libOptions"] = options; }
	void BasicTool::setClassName(const QString & name) { (*m_map)["className"] = name; }
	void BasicTool::setSequence(const QString & /*sequence*/) { if (m_elbSequence) (*m_map)["sequence"] = m_elbSequence->items().join(","); }
	void BasicTool::setClose(bool on) { (*m_map)["close"] = on ? "yes" : "no"; }
	void BasicTool::setTarget(const QString & trg) { (*m_map)["target"] = trg; }
	void BasicTool::setRelDir(const QString & rd) { (*m_map)["relDir"] = rd; }
	void BasicTool::setLaTeXCheckRoot(bool ck) { (*m_map)["checkForRoot"] = ck ? "yes" : "no"; }
	void BasicTool::setLaTeXJump(bool ck) { (*m_map)["jumpToFirstError"] = ck ? "yes" : "no"; }
	void BasicTool::setLaTeXAuto(bool ck) { (*m_map)["autoRun"] = ck ? "yes" : "no"; }
	void BasicTool::setRunLyxServer(bool ck)
	{
		//kdDebug() << "setRunLyxServer" << endl;
		m_config->setGroup("Tools");
		m_config->writeEntry("RunLyxServer", ck);
	}

	AdvancedTool::AdvancedTool(const QString & /*tool*/, KileTool::Config  *map, QWidget *parent) : QWidget(parent), m_map(map)
	{
		//kdDebug() << "==AdvancedTool::AdvancedTool()====================" << endl;
		m_layout = new QGridLayout(this, 2, 4, 0, 10);
		QLabel *lb = new QLabel(i18n("&Type:"), this); m_layout->addWidget(lb, 0, 0, Qt::AlignLeft);
		m_cbType = new KComboBox(this); m_layout->addWidget(m_cbType, 0, 1, Qt::AlignLeft);
		lb->setBuddy(m_cbType);
		m_cbType->insertItem(i18n("Run Outside of Kile")); m_cbType->insertItem("Run in Konsole"); m_cbType->insertItem("Run Embedded in Kile"); m_cbType->insertItem("Use HTML Viewer"); m_cbType->insertItem("Run Sequence of Tools");
		connect(m_cbType, SIGNAL(activated(int)), this, SLOT(switchType(int)));

		QString type = (*m_map)["type"];
		if ( type == "Process" ) m_cbType->setCurrentItem(0);
		else if ( type == "Konsole" ) m_cbType->setCurrentItem(1);
		else if ( type == "Part" ) m_cbType->setCurrentItem(2);
		else if ( type == "DocPart" ) m_cbType->setCurrentItem(3);
		else if ( type == "Sequence" ) m_cbType->setCurrentItem(4);

		QStringList classes;
		classes << "Compile" << "Convert" << "View" <<  "Sequence" << "LaTeX" << "ViewHTML" << "ViewBib" << "ForwardDVI" << "Base";
		lb = new QLabel(i18n("C&lass:"), this); m_layout->addWidget(lb, 0, 2, Qt::AlignLeft);
		m_cbClasses = new KComboBox(this); m_layout->addWidget(m_cbClasses, 0, 3, Qt::AlignLeft);
		m_cbClasses->insertStringList(classes);
		connect(m_cbClasses, SIGNAL(activated(const QString &)), this, SLOT(switchClass(const QString& )));
		int index = classes.findIndex((*m_map)["class"]);
		if ( index == -1 ) index = classes.count()-1;
		m_cbClasses->setCurrentItem(index);
		void setFrom(const QString &);
		void setTo(const QString &);
		lb->setBuddy(m_cbClasses);

		createFromTo();

		QString cls = (*m_map)["class"];
		if ( cls == "ViewHTML" )
			createViewHTML();
	}

	void AdvancedTool::switchType(int index)
	{
		//kdDebug() << "==AdvancedTool::switchType()===========" << endl;
		switch (index)
		{
		case 0 : (*m_map)["type"] = "Process"; break;
		case 1 : (*m_map)["type"] = "Konsole"; break;
		case 2 : (*m_map)["type"] = "Part"; break;
		case 3 : (*m_map)["type"] = "DocPart"; break;
		case 4 : (*m_map)["type"] = "Sequence"; break;
		default : (*m_map)["type"] = "Process"; break;
		}
		//kdDebug() << "\temitting changed()" << endl;
		emit(changed());
	}

	void AdvancedTool::createViewHTML()
	{
		//kdDebug() << "==AdvancedTool::createViewHTML()====================" << endl;
		int row = m_layout->numRows();

		QLabel *lb = new QLabel(i18n("&File to view: "), this); m_layout->addWidget(lb, row, 0, Qt::AlignLeft);
		KLineEdit *le = new KLineEdit(this); m_layout->addWidget(le, row, 1, Qt::AlignLeft);
		le->setText((*m_map)["target"]); lb->setBuddy(le);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setTarget(const QString &)));

		lb = new QLabel(i18n("Relative &directory:"), this); m_layout->addWidget(lb, row, 2, Qt::AlignLeft);
		le = new KLineEdit(this); m_layout->addWidget(le, row, 3, Qt::AlignLeft);
		le->setText((*m_map)["relDir"]); lb->setBuddy(le);
		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setRelDir(const QString &)));
	}

	void AdvancedTool::createFromTo()
	{
		//kdDebug() << "==AdvancedTool::createFromTo()====================" << endl;
		int row = m_layout->numRows();

		QHBox *box = new QHBox(this); m_layout->addMultiCellWidget(box, row, row, 0, m_layout->numCols()-1, Qt::AlignLeft);
		box->setSpacing(5);
		QString cat = KileTool::categoryFor((*m_map)["class"]);
		QString extFrom = (*m_map)["from"], extTo = (*m_map)["to"];
		bool src = true, trg = true;
		QString from = i18n("&Source file extension:"), to = i18n("&Target file extension:");

		if ( (cat == "Compile" )  && (extFrom == "") ) { src = false; }
		else if ( cat == "View" ) { src = false; to = i18n("&Extension:"); }
		else if ( cat == "Sequence" ) { src = trg = false; }

		QLabel *lb;
		KLineEdit *le;
		if ( src )
		{
			lb = new QLabel(from, box);
			le = new KLineEdit(box);
			lb->setBuddy(le);
			le->setText(extFrom);
			le->setMaximumWidth(50);
			connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setFrom(const QString &)));
		}
		if ( trg )
		{
			lb = new QLabel(to, box);
			le = new KLineEdit(box);
			lb->setBuddy(le);
			le->setText(extTo);
			le->setMaximumWidth(50);
			connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(setTo(const QString &)));
		}
	}

	void AdvancedTool::switchClass(const QString & cls)
	{
		//kdDebug() << "==AdvancedTool::switchClass(" << cls << ")=====" << endl;
		if ( (*m_map)["class"] != cls )
		{
			setClass(cls);
			//kdDebug() << "\temitting changed()" << endl;
			emit(changed());
		}
	}

	void AdvancedTool::setFrom(const QString & from) { (*m_map)["from"] = from; }
	void AdvancedTool::setTo(const QString & to) { (*m_map)["to"] = to; }
	void AdvancedTool::setClass(const QString & cls) { (*m_map)["class"] = cls; }
	void AdvancedTool::setTarget(const QString & trg) { (*m_map)["target"] = trg; }
	void AdvancedTool::setRelDir(const QString & rd) { (*m_map)["relDir"] = rd; }

	QuickTool::QuickTool(KileTool::Config *map, KConfig *config, QWidget *parent, const char *name) :
		QWidget(parent,name),
		m_config(config),
		m_map(map)
	{
		//kdDebug() << "==QuickTool::QuickTool()====================" << endl;
		QGridLayout *layout = new QGridLayout(this, 7, 5, 5, 5);
		layout->setRowStretch(1, 2);

		QLabel *lb = new QLabel(i18n("&Tool"), this); layout->addMultiCellWidget(lb, 0, 0, 0, 1);
		m_cbTools = new KComboBox(this); layout->addMultiCellWidget(m_cbTools, 1, 1, 0, 1);
		lb->setBuddy(m_cbTools);
		QStringList list = KileTool::toolList(m_config, false);
		list.sort();
		m_cbTools->insertStringList(list);

		lb = new QLabel(i18n("&Configuration"), this); layout->addMultiCellWidget(lb, 0, 0, 2, 3);
		m_cbConfigs = new KComboBox(this); layout->addMultiCellWidget(m_cbConfigs, 1, 1, 2, 3);
		lb->setBuddy(m_cbConfigs);
		updateConfigs(m_cbTools->currentText());
		connect(m_cbTools, SIGNAL(activated(const QString &)), this, SLOT(updateConfigs(const QString& )));

		m_pbAdd = new KPushButton(i18n("&Add"), this); layout->addWidget(m_pbAdd, 1, 4);

		m_lstbSeq = new KListBox(this); layout->addMultiCellWidget(m_lstbSeq, 2, 6, 0, 3);
		QString tl, cfg;
		list = QStringList::split(",",(*m_map)["sequence"]);
		for ( uint i=0; i < list.count(); i++)
		{
			KileTool::extract(list[i], tl, cfg);
			if ( cfg != QString::null )
				m_lstbSeq->insertItem(tl+" ("+cfg+")");
			else
				m_lstbSeq->insertItem(tl);
		}

		m_pbUp = new KPushButton(i18n("Move &Up"), this); layout->addWidget(m_pbUp, 3, 4);
		m_pbDown = new KPushButton(i18n("Move &Down"), this); layout->addWidget(m_pbDown, 4, 4);
		m_pbRemove = new KPushButton(i18n("&Remove"), this); layout->addWidget(m_pbRemove, 5, 4);

		connect(m_pbAdd, SIGNAL(clicked()), this, SLOT(add()));
		connect(m_pbRemove, SIGNAL(clicked()), this, SLOT(remove()));
		connect(m_pbUp, SIGNAL(clicked()), this, SLOT(up()));
		connect(m_pbDown, SIGNAL(clicked()), this, SLOT(down()));
	}

	void QuickTool::updateConfigs(const QString &tool)
	{
		//kdDebug() << "==QuickTool::updateConfigs(const QString &tool)====================" << endl;
		m_cbConfigs->clear();
		m_cbConfigs->insertItem(i18n("not specified"));
		m_cbConfigs->insertStringList(KileTool::configNames(tool, m_config));
	}

	void QuickTool::up()
	{
		int current = m_lstbSeq->currentItem();
		if ( (current != -1) && (current > 0) )
		{
			QString text = m_lstbSeq->text(current-1);
			m_lstbSeq->changeItem(m_lstbSeq->text(current), current-1);
			m_lstbSeq->changeItem(text, current);
			m_lstbSeq->setCurrentItem(current-1);
			changed();
		}
	}

	void QuickTool::down()
	{
		int current = m_lstbSeq->currentItem();
		if ( (current != -1) && (current < ( ((int)m_lstbSeq->count())-1) ))
		{
			QString text = m_lstbSeq->text(current+1);
			m_lstbSeq->changeItem(m_lstbSeq->text(current), current+1);
			m_lstbSeq->changeItem(text, current);
			m_lstbSeq->setCurrentItem(current+1);
			changed();
		}
	}

	void QuickTool::add()
	{
		QString entry = m_cbTools->currentText();
		if ( m_cbConfigs->currentText() != i18n("not specified") )
			entry += " (" + m_cbConfigs->currentText() + ")";
		m_lstbSeq->insertItem(entry);
		changed();
	}

	void QuickTool::remove()
	{
		int current = m_lstbSeq->currentItem();
		if ( current != -1)
		{
			m_lstbSeq->removeItem(current);
			changed();
		}
	}

	void QuickTool::changed()
	{
		//kdDebug() << "==QuickTool::changed()====================" << endl;
		QString sequence, tool, cfg;

		for (uint i = 0; i < m_lstbSeq->count(); i++)
		{
			KileTool::extract(m_lstbSeq->text(i), tool, cfg);
			sequence += KileTool::format(tool,cfg)+",";
			//kdDebug() << "SEQUENCE: " << sequence << endl;
		}
		if (sequence.endsWith(",") ) sequence = sequence.left(sequence.length()-1);
		(*m_map)["sequence"] = sequence;
	}
}

#include "kiletoolconfigwidget.moc"
