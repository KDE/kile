/***************************************************************************
                          kiletoolmanager.h  -  description
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
 ****************************************************************************/

#ifndef KILETOOLMANAGER_H
#define KILETOOLMANAGER_H

#include <qobject.h>
#include <qstringlist.h>
#include <qptrqueue.h>

#include "kiletool.h"

class QTimer;
class QWidgetStack;

class KConfig;
class KTextEdit;
class KAction;
namespace KParts { class PartManager; }

class KileInfo;
namespace KileWidget { class LogMsg; class Output; }

namespace KileTool
{
	class Factory;

	class QueueItem
	{
	public:
		QueueItem(Base *tool, const QString & cfg = QString::null);
		~QueueItem();

		Base* tool() const { return m_tool; }
		const QString cfg() const { return m_cfg; }

	private:
		Base	*m_tool;
		QString	m_cfg;
	};

	class Queue : public QPtrQueue<QueueItem>
	{
	public:
		Base* tool() const;
		const QString cfg() const;
	};
	
	class Manager : public QObject
	{
		Q_OBJECT
		
	public:
		Manager(KileInfo *ki, KConfig *config, KileWidget::LogMsg *log, KileWidget::Output *output, KParts::PartManager *, QWidgetStack *, KAction *, uint to);
		~Manager();

	public:
		void initTool(Base*);
		bool configure(Base*);
		bool retrieveEntryMap(const QString & name, Config & map, bool usequeue = true, bool useproject = true);
		void saveEntryMap(const QString & name, Config & map, bool usequeue = true);
		QString currentGroup(const QString &name, bool usequeue = true);
		//QString configName(const QString & tool);
		//void setConfigName(const QString & tool, const QString & name);

		void wantGUIState(const QString &);
		
		KParts::PartManager * partManager() { return m_pm; }
		QWidgetStack * widgetStack() { return m_stack; }
		
		KileInfo * info() { return m_ki; }
		KConfig * config() { return m_config; }
		
		void setFactory(Factory* fac) { m_factory = fac; }
		Factory* factory() { return m_factory; }

		bool queryContinue(const QString & question, const QString & caption = QString::null);

	public slots:
		void started(Base*);
		void done(Base *, int);
		
		int run(const QString &, const QString & = QString::null);
		int run(Base *, const QString & = QString::null);

		void stop(); //should be a slot that stops the active tool and clears the queue

	private slots:
		int runNextInQueue();
		void enableClear();

	signals:
		void requestGUIState(const QString &);
		void requestSaveAll();

	private:
		KileInfo		*m_ki;
		KConfig		*m_config;
		KileWidget::LogMsg		*m_log;
		KileWidget::Output		*m_output;
		KParts::PartManager	*m_pm;
		QWidgetStack 			*m_stack;
		KAction				*m_stop;
		Factory				*m_factory;
		Queue				m_queue;
		QTimer				*m_timer;
		bool					m_bClear;
		uint					m_nTimeout;
	};

	QStringList toolList(KConfig *config, bool menuOnly = false);
	QStringList configNames(const QString &tool, KConfig *config);

	QString configName(const QString & tool, KConfig *);
	void setConfigName(const QString & tool, const QString & name, KConfig *);

	QString groupFor(const QString & tool, KConfig *);
	QString groupFor(const QString & tool, const QString & cfg = "Default" );

	void extract(const QString &str, QString &tool, QString &cfg);
	QString format(const QString & tool, const QString &cfg);

	QString menuFor(const QString &tool, KConfig *config);
	QString iconFor(const QString &tool, KConfig *config);

	QString categoryFor(const QString &clss);

	void setGUIOptions(const QString &tool, const QString &menu, const QString &icon, KConfig *config);
}

#endif
