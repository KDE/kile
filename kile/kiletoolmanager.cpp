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
 
namespace KileTool
{
	Manager::Manager(KileInfo *ki, KConfig *config, KileWidget::LogMsg *log, KTextEdit *output, KParts::PartManager *manager, QWidgetStack *stack, KAction *stop) :
		m_ki(ki),
		m_config(config),
		m_log(log),
		m_output(output),
		m_pm(manager),
		m_stack(stack),
		m_stop(stop)
	{
		connect(stop, SIGNAL(activated()), this, SIGNAL(stop()));
	}
	
	Manager::~Manager()
	{
	}
	
	void Manager::run(const QString &tool)
	{
		if (!m_factory)
		{
			recvMessage(Error, i18n("No factory installed, contact the author of Kile."));
			return;
		}
	
		Base* pTool = m_factory->create(tool);
		if (!pTool)
		{
			recvMessage(Error, i18n("Unknown tool %1.").arg(tool));
			return;
		}
		
		run(pTool);
	}

	void Manager::run(Base *tool)
	{
		m_log->clear();
		m_output->clear();
		tool->run();
	}

	void Manager::initTool(Base *tool)
	{
		tool->setInfo(m_ki);
		tool->setConfig(m_config);

		connect(tool, SIGNAL(message(int, const QString &, const QString &)), this, SLOT(recvMessage(int, const QString &, const QString &)));
		connect(tool, SIGNAL(output(char *,int)), this, SLOT(recvOutput(char *, int)));
		connect(tool, SIGNAL(done(Base*,int)), this, SLOT(done(Base*, int)));
		connect(tool, SIGNAL(start(Base* )), this, SLOT(started(Base*)));
		connect(tool, SIGNAL(requestSaveAll()), this, SIGNAL(requestSaveAll()));
	}

	void Manager::started(Base *tool)
	{
		kdDebug() << "STARTING tool: " << tool->name() << endl;
		m_stop->setEnabled(true);
		connect(this, SIGNAL(stop()), tool, SLOT(stop()));
	}

	void Manager::done(Base *tool, int result)
	{
		kdDebug() << "DELETING tool : " << tool->name() << endl;
		m_stop->setEnabled(false);
		if ( result == Aborted)
			tool->sendMessage(Error, i18n("Aborted"));

		delete tool;
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
			recvMessage(Error, i18n("Can't find the tool %1 in the configuration database.").arg(tool->name()));
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
	
	void Manager::recvMessage(int type, const QString & msg, const QString & tool)
	{
		m_log->printMsg(type, msg, tool);
	}
	
	void Manager::recvOutput(char *buffer, int buflen)
	{
		//kdDebug() << "received output: " << buffer << endl;
	
		int row = (m_output->paragraphs() == 0)? 0 : m_output->paragraphs()-1;
		int col = m_output->paragraphLength(row);
		QString s=QCString(buffer,buflen+1);
		m_output->setCursorPosition(row,col);
		m_output->insertAt(s, row, col);
	}
}

#include "kiletoolmanager.moc"
