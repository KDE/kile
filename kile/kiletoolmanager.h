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
 ****************************************************************************/

#ifndef KILETOOLMANAGER_H
#define KILETOOLMANAGER_H

#include <qobject.h>
#include <qstringlist.h>
#include <q3ptrqueue.h>

#include "kiletool.h"

class QTimer;
class Q3WidgetStack;

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
		QueueItem(Base *tool, const QString & cfg = QString::null, bool block = false);
		~QueueItem();

		Base* tool() const { return m_tool; }
		const QString cfg() const { return m_cfg; }
		bool shouldBlock() { return m_bBlock; }

	private:
		Base	*m_tool;
		QString	m_cfg;
		bool		m_bBlock;
	};

	class Queue : public Q3PtrQueue<QueueItem>
	{
	public:
		Base* tool() const;
		const QString cfg() const;
		bool shouldBlock() const;

		void enqueueNext(QueueItem *);
	};
	
	class Manager : public QObject
	{
		Q_OBJECT
		
	public:
		Manager(KileInfo *ki, KConfig *config, KileWidget::LogMsg *log, KileWidget::Output *output, KParts::PartManager *, Q3WidgetStack *, KAction *, uint to);
		~Manager();

	public:
		void initTool(Base*);
		bool configure(Base*, const QString & cfg = QString::null);
		bool retrieveEntryMap(const QString & name, Config & map, bool usequeue = true, bool useproject = true, const QString & cfg = QString::null);
		void saveEntryMap(const QString & name, Config & map, bool usequeue = true, bool useproject = true);
		QString currentGroup(const QString &name, bool usequeue = true, bool useproject = true);

		void wantGUIState(const QString &);
		
		KParts::PartManager * partManager() { return m_pm; }
		Q3WidgetStack * widgetStack() { return m_stack; }
		
		KileInfo * info() { return m_ki; }
		KConfig * config() { return m_config; }
		
		void setFactory(Factory* fac) { m_factory = fac; }
		Factory* factory() { return m_factory; }

		bool queryContinue(const QString & question, const QString & caption = QString::null);

		bool shouldBlock();
		int lastResult() { return m_nLastResult; }

	public slots:
		void started(Base*);
		void done(Base *, int);

		int run(const QString &, const QString & = QString::null, bool insertAtTop = false, bool block = false);
		int run(Base *, const QString & = QString::null, bool insertAtTop = false, bool block = false);

		int runNext(const QString &, const QString & = QString::null, bool block = false);
		int runNext(Base *, const QString & = QString::null, bool block = false);

		int runBlocking(const QString &, const QString & = QString::null, bool = false);
		int runNextBlocking(const QString &, const QString & = QString::null);

		void stop(); //should be a slot that stops the active tool and clears the queue

	private slots:
		int runNextInQueue();
		void enableClear();

	signals:
		void requestGUIState(const QString &);
		void requestSaveAll(bool, bool);
		void jumpToFirstError();
		void toolStarted();
		void previewDone();

	private:
		KileInfo		*m_ki;
		KConfig		*m_config;
		KileWidget::LogMsg		*m_log;
		KileWidget::Output		*m_output;
		KParts::PartManager	*m_pm;
		Q3WidgetStack 			*m_stack;
		KAction				*m_stop;
		Factory				*m_factory;
		Queue				m_queue;
		QTimer				*m_timer;
		bool					m_bClear;
		int					m_nLastResult;
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
