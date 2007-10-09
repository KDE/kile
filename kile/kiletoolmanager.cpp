/***************************************************************************
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

#include "kiletoolmanager.h"

#include <stdlib.h> //for getenv()
 
#include <qstring.h>
#include <qfileinfo.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qregexp.h>
#include <qthread.h>

#include <kaction.h>
#include <ktextedit.h>
#include <kconfig.h>
#include "kiledebug.h"
#include <klocale.h>
#include <kparts/partmanager.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <ksimpleconfig.h>

#include "kileinfo.h"
#include "kileconfig.h"
#include "kileproject.h"
#include "kiletool_enums.h"
#include "kilestdtools.h"
#include "kilelogwidget.h"
#include "kileoutputwidget.h"
#include "kiledocmanager.h"
#include "kilesidebar.h"

namespace KileTool
{
	QueueItem::QueueItem(Base *tool, const QString & cfg, bool block) : m_tool(tool), m_cfg(cfg), m_bBlock(block)
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

	bool Queue::shouldBlock() const
	{
		if ( head() )
			return head()->shouldBlock();
		else
			return false;
	}

	void Queue::enqueueNext(QueueItem *item)
	{
		if ( count() < 2 )
			enqueue(item);
		else
		{
			QueueItem *headitem = dequeue();
			Queue *oldqueue = new Queue(*this);

			setAutoDelete(false); clear();
			KILE_DEBUG() << "\tenqueueing: " << headitem->tool()->name() << endl;
			enqueue(headitem);
			KILE_DEBUG() << "\tenqueueing: " << item->tool()->name() << endl;
			enqueue(item);
			while ( oldqueue->head() )
			{
				KILE_DEBUG() << "\tenqueueing: " << oldqueue->head()->tool()->name() << endl;
				enqueue(oldqueue->dequeue());
			}
		}
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
		m_nLastResult(Success),
		m_nTimeout(to)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(enableClear()));
		connect(stop, SIGNAL(activated()), this, SLOT(stop()));
	}
	
	Manager::~Manager() {}

	bool Manager::shouldBlock()
	{
		return m_queue.shouldBlock();
	}

	void Manager::enableClear()
	{
		m_bClear = true;
	}

	bool Manager::queryContinue(const QString & question, const QString & caption /*= QString::null*/)
	{
		return (KMessageBox::warningContinueCancel(m_stack, question, caption, KStdGuiItem::cont(), "showNotALaTeXRootDocumentWarning") == KMessageBox::Continue);
	}

	int Manager::run(const QString &tool, const QString & cfg, bool insertNext /*= false*/, bool block /*= false*/)
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
		
		return run(pTool, cfg, insertNext, block);
	}

	int Manager::run(Base *tool, const QString & cfg, bool insertNext /*= false*/, bool block /*= false*/)
	{
		KILE_DEBUG() << "==KileTool::Manager::run(Base *)============" << endl;
		if (m_bClear && (m_queue.count() == 0) )
		{
			m_log->clear();
			m_ki->setLogPresent(false);
			m_output->clear();
		}

		if ( tool->needsToBePrepared() )
			tool->prepareToRun(cfg);
    else
      tool->configure(cfg);

		//FIXME: shouldn't restart timer if a Sequence command takes longer than the 10 secs
		//restart timer, so we only clear the logs if a tool is started after 10 sec.
		m_bClear=false;
		m_timer->start(m_nTimeout);

		if ( insertNext )
			m_queue.enqueueNext(new QueueItem(tool, cfg, block));
		else
			m_queue.enqueue(new QueueItem(tool, cfg, block));

		KILE_DEBUG() << "\tin queue: " << m_queue.count() << endl;
		if ( m_queue.count() == 1 )
			return runNextInQueue();
		else if ( m_queue.count() > 1 )
			return Running;
		else
			return ConfigureFailed;
	}

	int Manager::runNext(const QString &tool , const QString &config, bool block /*= false*/) 
	{
		return run(tool, config, true, block);
	}

	int Manager::runNext(Base *tool, const QString &config, bool block /*= false*/) 
	{
		return run(tool, config, true, block); 
	}

	int Manager::runBlocking(const QString &tool, const QString &config /*= QString::null*/, bool insertAtTop /*= false*/)
	{
		if ( run(tool, config, insertAtTop, true) == Running )
			return lastResult();
		else
			return Failed;
	}

	int Manager::runNextBlocking(const QString &tool, const QString &config)
	{
		return runBlocking(tool, config, true);
	}

	int Manager::runNextInQueue()
	{
		Base *head = m_queue.tool();
		if (head)
		{
			if (m_log->lines() > 1) 
				m_log->append("\n");

	        if ( ! head->isPrepared() )
    	        head->prepareToRun();

			int status;
			if ( (status=head->run()) != Running ) //tool did not even start, clear queue
			{
				stop();
				m_queue.setAutoDelete(true); m_queue.clear(); m_queue.setAutoDelete(false);
				return status;
			}

			emit(toolStarted());

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
		connect(tool, SIGNAL(requestSaveAll(bool, bool)), this, SIGNAL(requestSaveAll(bool, bool)));
	}

	void Manager::started(Base *tool)
	{
		KILE_DEBUG() << "STARTING tool: " << tool->name() << endl;
		m_stop->setEnabled(true);

		if (tool->isViewer()) 
		{
			if ( tool == m_queue.tool() ) m_queue.dequeue();
			m_stop->setEnabled(false);
			QTimer::singleShot(100, this, SLOT(runNextInQueue()));
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
		m_nLastResult = result;

		if ( tool != m_queue.tool() ) //oops, tool finished async, could happen with view tools
		{
			delete tool;
			return;
		}

		delete m_queue.dequeue();

		if ( result == Aborted)
			tool->sendMessage(Error, i18n("Aborted"));

		if ( result != Success && result != Silent ) //abort execution, delete all remaining tools
		{
			m_queue.setAutoDelete(true); m_queue.clear(); m_queue.setAutoDelete(false);
			m_ki->outputView()->showPage(m_log);
		}
		else //continue
			runNextInQueue();
	}

	QString Manager::currentGroup(const QString &name, bool usequeue, bool useproject)
	{
		if (useproject)
		{
			KileProject *project = m_ki->docManager()->activeProject();
			if (project)
			{
				QString cfg = configName(name, dynamic_cast<KConfig*>(project->config()));
				if ( cfg.length() > 0 ) return groupFor(name, cfg);
			}
		}

		if (usequeue && m_queue.tool() && (m_queue.tool()->name() == name) && (!m_queue.cfg().isNull()) )
			return groupFor(name, m_queue.cfg());
		else
			return groupFor(name, m_config);
	}

	bool Manager::retrieveEntryMap(const QString & name, Config & map, bool usequeue, bool useproject, const QString & cfg /*= QString::null*/)
	{
		QString group = (cfg == QString::null )
                    ? currentGroup(name, usequeue, useproject) : groupFor(name, cfg);

		KILE_DEBUG() << "==KileTool::Manager::retrieveEntryMap=============" << endl;
		KILE_DEBUG() << "\t" << name << " => " << group << endl;
		if ( m_config->hasGroup(group) )
		{
			map = m_config->entryMap(group);

			//use project overrides
			KileProject *project = m_ki->docManager()->activeProject();
			if ( useproject && project)
			{
				KConfig *prjcfg = dynamic_cast<KConfig*>(project->config());
				if ( prjcfg )
				{
					QString grp = groupFor(name, prjcfg);
					Config prjmap = prjcfg->entryMap(grp);
					for (Config::Iterator it  = prjmap.begin(); it != prjmap.end(); ++it)
					{
						map[it.key()] = it.data();
					}
				}
			}
		}
		else
			return false;

		return true;
	}

	void Manager::saveEntryMap(const QString & name, Config & map, bool usequeue, bool useproject)
	{
		KILE_DEBUG() << "==KileTool::Manager::saveEntryMap=============" << endl;
		QString group = currentGroup(name, usequeue, useproject);
		KILE_DEBUG() << "\t" << name << " => " << group << endl;
		m_config->setGroup(group);

		Config::Iterator it;
		for ( it = map.begin() ; it != map.end(); ++it)
		{
			if ( ! it.data().isEmpty() )
				m_config->writeEntry(it.key(), it.data());
		}
	}

	bool Manager::configure(Base *tool, const QString & cfg /*=QString::null*/)
	{
		KILE_DEBUG() << "==KileTool::Manager::configure()===============" << endl;
		//configure the tool

		Config map;

		if ( ! retrieveEntryMap(tool->name(), map, true, true, cfg) )
		{
			m_log->printMsg(Error, i18n("Cannot find the tool %1 in the configuration database.").arg(tool->name()));
			return false;
		}

		tool->setEntryMap(map);

		return true;
	}

	void Manager::wantGUIState(const QString & state)
	{
		KILE_DEBUG() << "REQUESTED state: " << state << endl;
		emit(requestGUIState(state));
	}

	QStringList toolList(KConfig *config, bool menuOnly)
	{
		KILE_DEBUG() << "==KileTool::toolList()==================" << endl;

		QStringList groups = config->groupList(), tools;
		QRegExp re = QRegExp("Tool/(.+)/.+");

		for ( uint i = 0; i < groups.count(); ++i )
		{
			if ( re.exactMatch(groups[i]) )
			{
				if ( ! groups[i].endsWith(configName(re.cap(1), config)) ) continue;

				if ( (! menuOnly) || ( menuFor(re.cap(1),config) != "none" ) )
				{
					tools.append(re.cap(1));
				}
			}
		}

		tools.sort();

		return tools;
	}

	QString configName(const QString & tool, KConfig *config)
	{
		config->setGroup("Tools");
		return config->readEntry(tool, "");
	}

	void setConfigName(const QString & tool, const QString & name, KConfig *config)
	{
		KILE_DEBUG() << "==KileTool::Manager::setConfigName(" << tool << "," << name << ")===============" << endl;
		config->setGroup("Tools");
		config->writeEntry(tool, name);
	}

	QString groupFor(const QString &tool, KConfig *config)
	{
		return groupFor(tool, configName(tool, config));
	}

	QString groupFor(const QString & tool, const QString & cfg /* = Default */ )
	{
		return "Tool/" + tool + '/' + cfg;
	}

	void extract(const QString &str, QString &tool, QString &cfg)
	{
		static QRegExp re("([^\\(]*)\\((.*)\\)");
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
		KILE_DEBUG() << "===void extract(const QString &str = " << str << " , QString &tool = " << tool << ", QString &cfg = " << cfg << " )===" << endl;
	}

	QString format(const QString & tool, const QString &cfg)
	{
		if (!cfg.isNull())
			return tool + '(' + cfg + ')';
		else
			return tool;
	}

	QStringList configNames(const QString &tool, KConfig *config)
	{
		QStringList groups = config->groupList(), configs;
		QRegExp re = QRegExp("Tool/"+ tool +"/(.+)");

		for ( uint i = 0; i < groups.count(); ++i )
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
		return config->readEntry(tool, "Other,gear").section(',',0,0);
	}

	QString iconFor(const QString &tool, KConfig *config)
	{
		config->setGroup("ToolsGUI");
		return config->readEntry(tool, "Other,gear").section(',',1,1);
	}

	void setGUIOptions(const QString &tool, const QString &menu, const QString &icon, KConfig *config)
	{
		QString entry = menu + ',' + icon;

		config->setGroup("ToolsGUI");
		config->writeEntry(tool, entry);
	}

	QString categoryFor(const QString &clss)
	{
		if ( clss == "Compile" || clss == "LaTeX" )
			return "Compile";
		if ( clss == "Convert" )
			return "Convert";
		if ( clss == "View" || clss == "ViewBib" || clss == "ViewHTML" || clss == "ForwardDVI" )
			return "View";
		if ( clss == "Sequence" )
			return "Sequence";
		if ( clss == "Archive")
			return "Archive";
		return "Base";
	}
}

#include "kiletoolmanager.moc"
