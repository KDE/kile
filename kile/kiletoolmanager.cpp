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

#include <kaction.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/partmanager.h>

#include "kileinfo.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kiletool.h"
#include "kilestdtools.h"
#include "kilelogwidget.h"
#include "kileoutputwidget.h"
 
namespace KileTool
{
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

	void Manager::run(const QString &tool)
	{
		if (!m_factory)
		{
			m_log->printMsg(Error, i18n("No factory installed, contact the author of Kile."));
			return;
		}
	
		Base* pTool = m_factory->create(tool);
		if (!pTool)
		{
			m_log->printMsg(Error, i18n("Unknown tool %1.").arg(tool));
			return;
		}
		
		run(pTool);
	}

	void Manager::run(Base *tool)
	{
		kdDebug() << "==KileTool::Manager::run(Base*)============" << endl;
		if (m_bClear)
		{
			m_log->clear();
			m_output->clear();
		}

		//restart timer, so we only clear the logs if a tool is started after 10 sec.
		m_bClear=false;
		m_timer->start(m_nTimeout);

		m_queue.enqueue(tool);
		kdDebug() << "\tin queue: " << m_queue.count() << endl;
		if ( m_queue.count() == 1 )
			runNextInQueue();
	}

	void Manager::runNextInQueue()
	{
		Base *head = m_queue.head();
		if (head)
		{
			if ( head->run() != Running ) //tool did not even start, remove it from the queue
			{
				delete m_queue.dequeue();
				runNextInQueue();
			}
		}
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
			if ( tool == m_queue.head() ) m_queue.dequeue();
			m_stop->setEnabled(false);
			runNextInQueue();
		}
	}

	void Manager::stop()
	{
		m_stop->setEnabled(false);
		if ( m_queue.head() )
			m_queue.head()->stop();
	}

	void Manager::done(Base *tool, int result)
	{
		m_stop->setEnabled(false);

		if ( tool != m_queue.head() ) //oops, tool finished async, could happen with view tools
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
	
	bool Manager::configure(Base *tool)
	{
		kdDebug() << "==KileTool::Manager::configure()===============" << endl;
		//configure the tool

		typedef QMap<QString,QString> entryMap;
		entryMap map;

		QString group = "Tool/"+tool->name();
		if (m_config->hasGroup(group ) )
		{
			kdDebug() << "\tretrieving entry map " << group << endl;
			map = m_config->entryMap(group);
			kdDebug() << "\t\tretrieved " << map.size() << " items" << endl;
			tool->setEntryMap(map);
		}
		else
		{
			kdDebug() << "\tgroup " << group << " not found" << endl;
			m_log->printMsg(Error, i18n("Can't find the tool %1 in the configuration database.").arg(tool->name()));
			return false;
		}

		//print the entry map
		/*entryMap::ConstIterator it;
		for ( it = map.begin(); it != map.end(); ++it ) 
		{
			kdDebug() << "\tentry: " << it.key() << " = " << map[it.key()] << endl;
		}*/

		return true;
	}

	void Manager::wantGUIState(const QString & state)
	{
		kdDebug() << "REQUESTED state: " << state << endl;
		emit(requestGUIState(state));
	}

	QStringList toolList(KConfig *config)
	{
		kdDebug() << "==KileTool::toolList()==================" << endl;

		int index;
		bool ok;
		QStringList groups = config->groupList(), tools;

		for ( uint i = 0; i < groups.count(); i++ )
		{
			if ( groups[i].startsWith("Tool/") )
			{
				config->setGroup(groups[i]);
				index = config->readEntry("toolbarPos", "none").toInt(&ok);
				if (!ok) index=1000; //some large number, assumption: #tools < 1000
				index += 1000;
				//append number to tool name, little trick to make sorting easy
				tools.append(QString::number(index)+"/"+groups[i]);
			}
		}

		tools.sort();

		for ( uint i = 0; i < tools.count(); i++ )
		{
			tools[i] = tools[i].section('/',2,2);
			kdDebug() << i << " : " << tools[i] << endl;
		}

		return tools;
	}
}

#include "kiletoolmanager.moc"
