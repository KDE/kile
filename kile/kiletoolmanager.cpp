/***************************************************************************
                          kiletoolmanager.cpp  -  description
                             -------------------
    begin                : Tue Nov 25 2003
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

#include <qstring.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qregexp.h>

#include <kaction.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/partmanager.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>

#include "kileinfo.h"
#include "kileproject.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kilestdtools.h"
#include "kilelogwidget.h"
#include "kileoutputwidget.h"
 
namespace KileTool
{
	QueueItem::QueueItem(Base *tool, const QString & cfg) : m_tool(tool), m_cfg(cfg)
	{
	}

	QueueItem::~QueueItem()
	{
		delete m_tool;
	}

	Base* Queue::tool() const
	{
		if ( head() )
			return head()->tool();
		else
			return 0;
	}

	const QString Queue::cfg() const
	{
		if ( head() )
			return head()->cfg();
		else
			return QString::null;
	}

	Manager::Manager(KileInfo *ki, KConfig *config, KileWidget::LogMsg *log, KileWidget::Output *output, KParts::PartManager *manager, QWidgetStack *stack, KAction *stop, uint to) :
		m_ki(ki),
		m_config(config),
		m_log(log),
		m_output(output),
		m_pm(manager),
		m_stack(stack),
		m_stop(stop),
		m_bClear(true),
		m_nTimeout(to)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(enableClear()));
		connect(stop, SIGNAL(activated()), this, SLOT(stop()));
	}
	
	Manager::~Manager()
	{
	}

	void Manager::enableClear()
	{
		m_bClear = true;
	}

	bool Manager::queryContinue(const QString & question, const QString & caption /*= QString::null*/)
	{
		return (KMessageBox::warningContinueCancel(m_stack, question, caption) == KMessageBox::Continue);
	}

	int Manager::run(const QString &tool, const QString & cfg)
	{
		if (!m_factory)
		{
			m_log->printMsg(Error, i18n("No factory installed, contact the author of Kile."));
			return ConfigureFailed;
		}
	
		Base* pTool = m_factory->create(tool);
		if (!pTool)
		{
			m_log->printMsg(Error, i18n("Unknown tool %1.").arg(tool));
			return ConfigureFailed;
		}
		
		return run(pTool, cfg);
	}

	int Manager::run(Base *tool, const QString & cfg)
	{
		kdDebug() << "==KileTool::Manager::run(Base*)============" << endl;
		if (m_bClear)
		{
			m_log->clear();
			m_output->clear();
		}

		//FIXME: shouldn't restart timer if a Sequence command takes longer than the 10 secs
		//restart timer, so we only clear the logs if a tool is started after 10 sec.
		m_bClear=false;
		m_timer->start(m_nTimeout);

		m_queue.enqueue(new QueueItem(tool, cfg));
		kdDebug() << "\tin queue: " << m_queue.count() << endl;
		if ( m_queue.count() == 1 )
			return runNextInQueue();
		else
			return ConfigureFailed;
	}

	int Manager::runNextInQueue()
	{
		Base *head = m_queue.tool();
		if (head)
		{
			if (m_log->lines() > 1) 
				m_log->append("\n");

			int status;
			if ( (status=head->run()) != Running ) //tool did not even start, clear queue
			{
				stop();
				m_queue.setAutoDelete(true); m_queue.clear(); m_queue.setAutoDelete(false);
				return status;
			}
			return Running;
		}

		return ConfigureFailed;
	}

	void Manager::initTool(Base *tool)
	{
		tool->setInfo(m_ki);
		tool->setConfig(m_config);

		connect(tool, SIGNAL(message(int, const QString &, const QString &)), m_log, SLOT(printMsg(int, const QString &, const QString &)));
		connect(tool, SIGNAL(output(const QString &)), m_output, SLOT(receive(const QString &)));
		connect(tool, SIGNAL(done(Base*,int)), this, SLOT(done(Base*, int)));
		connect(tool, SIGNAL(start(Base* )), this, SLOT(started(Base*)));
		connect(tool, SIGNAL(requestSaveAll()), this, SIGNAL(requestSaveAll()));
	}

	void Manager::started(Base *tool)
	{
		kdDebug() << "STARTING tool: " << tool->name() << endl;
		m_stop->setEnabled(true);

		if (tool->isViewer()) 
		{
			if ( tool == m_queue.tool() ) m_queue.dequeue();
			m_stop->setEnabled(false);
			runNextInQueue();
		}
	}

	void Manager::stop()
	{
		m_stop->setEnabled(false);
		if ( m_queue.tool() )
			m_queue.tool()->stop();
	}

	void Manager::done(Base *tool, int result)
	{
		m_stop->setEnabled(false);

		if ( tool != m_queue.tool() ) //oops, tool finished async, could happen with view tools
		{
			delete tool;
			return;
		}

		delete m_queue.dequeue();

		if ( result == Aborted)
			tool->sendMessage(Error, i18n("Aborted"));

		if ( result != Success && result != Silent ) //abort execution, delete all remaing tools
		{
			m_queue.setAutoDelete(true); m_queue.clear(); m_queue.setAutoDelete(false);
		}
		else //continue
			runNextInQueue();
	}

	QString Manager::currentGroup(const QString &name, bool usequeue)
	{
		KileProject *project = m_ki->activeProject();
		if (project)
		{
			QString cfg = configName(name, dynamic_cast<KConfig*>(project->config()));
			if ( cfg.length() > 0 ) return groupFor(name, cfg);
		}

		if (usequeue && m_queue.tool() && (m_queue.tool()->name() == name) && (m_queue.cfg() != QString::null) )
			return  groupFor(name, m_queue.cfg());
		else
			return groupFor(name, m_config);
	}

	bool Manager::retrieveEntryMap(const QString & name, Config & map, bool usequeue)
	{
		QString group = currentGroup(name, usequeue);

		kdDebug() << "==KileTool::Manager::retrieveEntryMap=============" << endl;
		kdDebug() << "\t" << name << " => " << group << endl;
		if ( m_config->hasGroup(group) )
		{
			map = m_config->entryMap(group);
		}
		else
			return false;

		return true;
	}

	void Manager::saveEntryMap(const QString & name, Config & map, bool usequeue)
	{
		kdDebug() << "==KileTool::Manager::saveEntryMap=============" << endl;
		QString group = currentGroup(name, usequeue);
		kdDebug() << "\t" << name << " => " << group << endl;
		m_config->setGroup(group);

		Config::Iterator it;
		for ( it = map.begin() ; it != map.end(); it ++)
		{
			m_config->writeEntry(it.key(), it.data());
		}
	}

	bool Manager::configure(Base *tool)
	{
		kdDebug() << "==KileTool::Manager::configure()===============" << endl;
		//configure the tool

		Config map;

		if ( ! retrieveEntryMap(tool->name(), map) )
		{
			m_log->printMsg(Error, i18n("Can't find the tool %1 in the configuration database.").arg(tool->name()));
			return false;
		}

		tool->setEntryMap(map);

		return true;
	}

	void Manager::wantGUIState(const QString & state)
	{
		kdDebug() << "REQUESTED state: " << state << endl;
		emit(requestGUIState(state));
	}

	QStringList toolList(KConfig *config, bool menuOnly)
	{
		kdDebug() << "==KileTool::toolList()==================" << endl;

		int index;
		bool place, separator;
		QStringList groups = config->groupList(), tools;
		QRegExp re = QRegExp("Tool/(.+)/.+");

		for ( uint i = 0; i < groups.count(); i++ )
		{
			if ( re.exactMatch(groups[i]) )
			{
				if ( ! groups[i].endsWith(configName(re.cap(1), config)) ) continue;

				config->setGroup(groups[i]);
				toolbarInfoFor(re.cap(1), index, place, separator, config);
				if (!place) index=1000; //some large number, assumption: #tools < 1000
				index += 1000;
				//append number to tool name, little trick to make sorting easy

				if ( (! menuOnly) || ( menuFor(re.cap(1),config) != "none" ) )
				{
					tools.append(QString::number(index)+"/"+ re.cap(1));
				}
			}
		}

		tools.sort();

		for ( uint i = 0; i < tools.count(); i++ )
		{
			tools[i] = tools[i].section('/',1,1);
			kdDebug() << i << " : " << tools[i] << endl;
		}

		return tools;
	}

	
	QString configName(const QString & tool, KConfig *config)
	{
		config->setGroup("Tools");
		return config->readEntry(tool, "");
	}

	void setConfigName(const QString & tool, const QString & name, KConfig *config)
	{
		kdDebug() << "==KileTool::Manager::setConfigName(" << tool << "," << name << ")===============" << endl;
		config->setGroup("Tools");
		config->writeEntry(tool, name);
	}

	QString groupFor(const QString &tool, KConfig *config)
	{
		return groupFor(tool, configName(tool, config));
	}

	QString groupFor(const QString & tool, const QString & cfg /* = Default */ )
	{
		return "Tool/" + tool + "/" + cfg;
	}

	void extract(const QString &str, QString &tool, QString &cfg)
	{
		static QRegExp re("(.*)\\((.*)\\)");
		QString lcl = str;
		lcl.stripWhiteSpace();
		cfg = QString::null;
		if ( re.exactMatch(lcl) )
		{
			tool = re.cap(1).stripWhiteSpace();
			cfg = re.cap(2).stripWhiteSpace();
		}
		else
			tool = lcl;
	}

	QString format(const QString & tool, const QString &cfg)
	{
		if (cfg != QString::null)
			return tool+"("+cfg+")";
		else
			return tool;
	}

	QStringList configNames(const QString &tool, KConfig *config)
	{
		QStringList groups = config->groupList(), configs;
		QRegExp re = QRegExp("Tool/"+ tool +"/(.+)");

		for ( uint i = 0; i < groups.count(); i++ )
		{
			if ( re.exactMatch(groups[i]) )
			{
				configs.append(re.cap(1));
			}
		}

		return configs;
	}

	QString menuFor(const QString &tool, KConfig *config)
	{
		config->setGroup("ToolsGUI");
		return config->readEntry(tool, "Other,none,false").section(',',0,0);
	}

	void toolbarInfoFor(const QString &tool, int &pos, bool &place, bool &separator, KConfig *config)
	{
		config->setGroup("ToolsGUI");
		QString entry = config->readEntry(tool, "Other,none,false,latex");

		bool ok;
		QString strpos = entry.section(',',1,1);
		place = (strpos != "none");
		pos = strpos.toInt(&ok);
		if (!ok) pos = -1;
		separator = entry.section(',',2,2) == "true";
	}

	QString iconFor(const QString &tool, KConfig *config)
	{
		config->setGroup("ToolsGUI");
		return config->readEntry(tool, "Other,none,false,latex").section(',',3,3);
	}

	void setGUIOptions(const QString &tool, const QString &menu, int pos, bool place, bool separator, const QString &icon, KConfig *config)
	{
		QString entry = menu+",";
		if (place) entry += QString::number(pos)+",";
		else entry += "none,";

		entry += separator ? "true" : "false";
		entry += ","+icon;

		config->setGroup("ToolsGUI");
		config->writeEntry(tool, entry);
	}
}

#include "kiletoolmanager.moc"
