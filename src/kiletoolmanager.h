/**************************************************************************************
    begin                : Tue Nov 25 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 **************************************************************************************/

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

#include <QStackedWidget>
#include <QStringList>
#include <QObject>
#include <QQueue>

#include "kiletool.h"

class QTimer;

class KConfig;
class KTextEdit;
class KAction;
namespace KParts { class PartManager; }

class KileInfo;
namespace KileWidget { class LogWidget; class OutputView; }

namespace KileTool
{
	class Factory;

	class QueueItem
	{
	public:
		explicit QueueItem(Base *tool, const QString & cfg = QString(), bool block = false);
		~QueueItem();

		Base* tool() const { return m_tool; }
		const QString cfg() const { return m_cfg; }
		bool shouldBlock() { return m_bBlock; }

	private:
		Base	*m_tool;
		QString	m_cfg;
		bool		m_bBlock;
	};

	class Queue : public QQueue<QueueItem*>
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
		Manager(KileInfo *ki, KConfig *config, KileWidget::LogWidget *log, KileWidget::OutputView *output, KParts::PartManager *, QStackedWidget* stack, KAction *, uint to);
		~Manager();

	public:
		void initTool(Base*);
		bool configure(Base*, const QString & cfg = QString());
		bool retrieveEntryMap(const QString & name, Config & map, bool usequeue = true, bool useproject = true, const QString & cfg = QString());
		void saveEntryMap(const QString & name, Config & map, bool usequeue = true, bool useproject = true);
		QString currentGroup(const QString &name, bool usequeue = true, bool useproject = true);

		void wantGUIState(const QString &);
		
		KParts::PartManager * partManager() { return m_pm; }
		QStackedWidget* widgetStack() { return m_stack; }
		
		KileInfo * info() { return m_ki; }
		KConfig * config() { return m_config; }
		
		void setFactory(Factory* fac) { m_factory = fac; }
		Factory* factory() { return m_factory; }

		bool queryContinue(const QString & question, const QString & caption = QString());

		bool shouldBlock();
		int lastResult() { return m_nLastResult; }

	public Q_SLOTS:
		int run(const QString&, const QString& = QString(), bool insertAtTop = false, bool block = false);
		int run(Base *, const QString& = QString(), bool insertAtTop = false, bool block = false);

		int runNext(const QString&, const QString& = QString(), bool block = false);
		int runNext(Base *, const QString& = QString(), bool block = false);

		int runBlocking(const QString&, const QString& = QString(), bool = false);
		int runNextBlocking(const QString&, const QString& = QString());

	private:
		void setEnabledStopButton(bool state);

	private Q_SLOTS:
		int runNextInQueue();
		void enableClear();

		void started(Base*);
		void done(Base *, int);

		void stop(); //should be a slot that stops the active tool and clears the queue
		void stopActionDestroyed();

	Q_SIGNALS:
		void requestGUIState(const QString &);
		void requestSaveAll(bool, bool);
		void jumpToFirstError();
		void toolStarted();
		void previewDone();

	private:
		KileInfo		*m_ki;
		KConfig		*m_config;
		KileWidget::LogWidget		*m_log;
		KileWidget::OutputView		*m_output;
		KParts::PartManager	*m_pm;
		QStackedWidget				*m_stack;
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
