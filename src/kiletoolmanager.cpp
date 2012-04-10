/**************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                2011-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kiletoolmanager.h"

#include <QFileInfo>
#include <QRegExp>
#include <QTimer>

#include <KAction>
#include <KConfig>
#include <KLocale>
#include <KMessageBox>
#include <KParts/PartManager>

#include "kileconfig.h"
#include "kiledebug.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kileproject.h"
#include "kilestdtools.h"
#include "kiletool_enums.h"
#include "parser/parsermanager.h"
#include "widgets/logwidget.h"
#include "widgets/outputview.h"
#include "widgets/sidebar.h"

namespace KileTool
{
	QueueItem::QueueItem(Base *tool, bool block) : m_tool(tool), m_bBlock(block)
	{
	}

	QueueItem::~QueueItem()
	{
	}

	Base* Queue::tool() const
	{
		if(count() > 0 && head()) {
			return head()->tool();
		}
		else {
			return 0;
		}
	}

	bool Queue::shouldBlock() const
	{
		if(count() > 0 && head()) {
			return head()->shouldBlock();
		}
		else {
			return false;
		}
	}

	void Queue::enqueueNext(QueueItem *item)
	{
		if(count() < 2) {
			enqueue(item);
		}
		else {
			QueueItem *headitem = dequeue();
			Queue *oldqueue = new Queue(*this);

			clear();
			KILE_DEBUG() << "\tenqueueing: " << headitem->tool()->name() << endl;
			enqueue(headitem);
			KILE_DEBUG() << "\tenqueueing: " << item->tool()->name() << endl;
			enqueue(item);
			while(!oldqueue->isEmpty()) {
				KILE_DEBUG() << "\tenqueueing: " << oldqueue->head()->tool()->name() << endl;
				enqueue(oldqueue->dequeue());
			}
		}
	}

	Manager::Manager(KileInfo *ki, KConfig *config, KileWidget::LogWidget *log, KileWidget::OutputView *output, KParts::PartManager *manager, QStackedWidget *stack, KAction *stop, uint to) :
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
		connect(m_ki->parserManager(), SIGNAL(parsingComplete()), this, SLOT(handleParsingComplete()));

		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(enableClear()));
		connect(stop, SIGNAL(triggered()), this, SLOT(stop()));
		connect(stop, SIGNAL(destroyed(QObject*)), this, SLOT(stopActionDestroyed()));
	}

	Manager::~Manager()
	{
		KILE_DEBUG();

		for(QQueue<QueueItem*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
			// this will also stop any running processes
			delete (*i)->tool();
			delete (*i);
		}
		// tools have the tool manager as parent; so, all remaining tools will be deleted
		// after this, i.e. those that were scheduled for deletion via 'deleteLater' but
		// are no longer member of the queue
	}

	bool Manager::shouldBlock()
	{
		return m_queue.shouldBlock();
	}

	// in some cases the pointer m_stop might not be valid, therefore this helper function comes in handy
	void Manager::setEnabledStopButton(bool state){

		if(m_stop){
			m_stop->setEnabled(state);
		}
	}

	void Manager::enableClear()
	{
		m_bClear = true;
	}

	bool Manager::queryContinue(const QString & question, const QString & caption /*= QString()*/)
	{
		return (KMessageBox::warningContinueCancel(m_stack, question, caption, KStandardGuiItem::cont(), KStandardGuiItem::no(), "showNotALaTeXRootDocumentWarning") == KMessageBox::Continue);
	}

	void Manager::run(Base *tool)
	{
		// if the tool requests a save-all operation, we wait for the parsing to
		// be finished before launching it
		if(!tool->requestSaveAll() || m_ki->parserManager()->isDocumentParsingComplete()) {
			// parsing done, we can start the tool immediately
			runImmediately(tool);
			return;
		}
		connect(tool, SIGNAL(aboutToBeDestroyed(KileTool::Base*)),
		        this, SLOT(toolScheduledAfterParsingDestroyed(KileTool::Base*)), Qt::UniqueConnection);
		if(!m_toolsScheduledAfterParsingList.contains(tool)) {
			m_toolsScheduledAfterParsingList.push_back(tool);
		}
	}

	void Manager::toolScheduledAfterParsingDestroyed(KileTool::Base *tool)
	{
		m_toolsScheduledAfterParsingList.removeAll(tool);
	}

	void Manager::handleParsingComplete()
	{
		Q_FOREACH(Base *tool, m_toolsScheduledAfterParsingList) {
			disconnect(tool, SIGNAL(aboutToBeDestroyed(KileTool::Base*)),
		                   this, SLOT(toolScheduledAfterParsingDestroyed(KileTool::Base*)));
			runImmediately(tool);
		}
		m_toolsScheduledAfterParsingList.clear();
	}

	int Manager::runImmediately(Base *tool, bool insertNext /*= false*/, bool block /*= false*/, Base *parent /*= NULL*/)
	{
		KILE_DEBUG() << "==KileTool::Manager::runImmediately(Base *)============" << endl;
		if(m_bClear && (m_queue.count() == 0)) {
			m_log->clear();
			m_output->clear();
		}

		if(tool->needsToBePrepared()) {
			tool->prepareToRun();
		}

		//FIXME: shouldn't restart timer if a Sequence command takes longer than the 10 secs
		//restart timer, so we only clear the logs if a tool is started after 10 sec.
		m_bClear = false;
		m_timer->start(m_nTimeout);

		if(insertNext) {
			m_queue.enqueueNext(new QueueItem(tool, block));
		}
		else {
			m_queue.enqueue(new QueueItem(tool, block));
		}

		if(parent) {
			emit(childToolSpawned(parent,tool));
		}

		KILE_DEBUG() << "\tin queue: " << m_queue.count() << endl;
		if(m_queue.count() == 1) {
			return runNextInQueue();
		}
		else if(m_queue.count() > 1) {
			return Running;
		}
		else {
			return ConfigureFailed;
		}
	}

	int Manager::runChildNext(Base *parent, Base *tool, bool block /*= false*/)
	{
		return runImmediately(tool, true, block, parent);
	}

	int Manager::runNextInQueue()
	{
		Base *head = m_queue.tool();
		if(head) {
			if (m_log->isShowingOutput()) {
				m_log->addEmptyLine();
			}

			if(!head->isPrepared()) {
				head->prepareToRun();
			}

			int status;
			if((status=head->run()) != Running) { //tool did not even start, clear queue
				stop();
				for(QQueue<QueueItem*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
					(*i)->tool()->deleteLater();
					delete (*i);
				}
				m_queue.clear();
				return status;
			}

			m_log->startToolLogOutput();
			emit(toolStarted());

			return Running;
		}

		return ConfigureFailed;
	}

	Base* Manager::createTool(const QString& name, const QString &cfg, bool prepare)
	{
		if (!m_factory) {
			m_log->printMessage(Error, i18n("No factory installed, contact the author of Kile."));
			return NULL;
		}

		Base* pTool = m_factory->create(name, cfg, prepare);
		if (!pTool) {
			m_log->printMessage(Error, i18n("Unknown tool %1.", name));
			return NULL;
		}
		initTool(pTool);
		return pTool;
	}

	void Manager::initTool(Base *tool)
	{
		tool->setInfo(m_ki);
		tool->setConfig(m_config);

		connect(tool, SIGNAL(message(int, const QString &, const QString &)), m_log, SLOT(printMessage(int, const QString &, const QString &)));
		connect(tool, SIGNAL(output(const QString &)), m_output, SLOT(receive(const QString &)));
		connect(tool, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(done(KileTool::Base*, int)));
		connect(tool, SIGNAL(start(KileTool::Base*)), this, SLOT(started(KileTool::Base*)));
	}

	void Manager::started(Base *tool)
	{
		KILE_DEBUG() << "STARTING tool: " << tool->name() << endl;
		setEnabledStopButton(true);

		if (tool->isViewer()) {
			if(tool == m_queue.tool()) {
				m_queue.dequeue();
			}
			setEnabledStopButton(false);
			QTimer::singleShot(100, this, SLOT(runNextInQueue()));
		}
	}

	void Manager::stop()
	{
		setEnabledStopButton(false);
		if(m_queue.tool()) {
			m_queue.tool()->stop();
		}
	}

	void Manager::stopLivePreview()
	{
		KILE_DEBUG();

		Base *tool = m_queue.tool();

		if(tool && tool->isPartOfLivePreview()) {
			setEnabledStopButton(false);
			tool->stop();
		}

		deleteLivePreviewToolsFromQueue();
		deleteLivePreviewToolsFromRunningAfterParsingQueue();
	}

	void Manager::stopActionDestroyed()
	{
		m_stop = NULL;
	}

	void Manager::done(KileTool::Base *tool, int result)
	{
		setEnabledStopButton(false);
		m_nLastResult = result;

		m_log->endToolLogOutput();

		if(tool != m_queue.tool()) { //oops, tool finished async, could happen with view tools
			tool->deleteLater();
			return;
		}

		QueueItem *item = m_queue.dequeue();
		item->tool()->deleteLater();
		delete item;

		if(result == Aborted) {
			tool->sendMessage(Error, i18n("Aborted"));
		}

		if(result != Success && result != Silent) { //abort execution, delete all remaining tools
			if(tool->isPartOfLivePreview()) { // live preview was stopped / aborted
				deleteLivePreviewToolsFromQueue();
				// don't forget to run non-live preview tools that are pending
				runNextInQueue();
			}
			else {
				for(QQueue<QueueItem*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
					(*i)->tool()->deleteLater();
					delete (*i);
				}
				m_queue.clear();
				m_ki->focusLog();
			}
		}
		else { //continue
			runNextInQueue();
		}
	}

	void Manager::deleteLivePreviewToolsFromQueue()
	{
		for(QQueue<QueueItem*>::iterator i = m_queue.begin(); i != m_queue.end();) {
			QueueItem *item = *i;
			if(item->tool()->isPartOfLivePreview()) {
				i = m_queue.erase(i);
				item->tool()->deleteLater();
				delete item;
			}
			else {
				++i;
			}
		}
	}

	void Manager::deleteLivePreviewToolsFromRunningAfterParsingQueue()
	{
		for(QQueue<Base*>::iterator i = m_toolsScheduledAfterParsingList.begin(); i != m_toolsScheduledAfterParsingList.end();) {
			Base *tool = *i;
			if(tool->isPartOfLivePreview()) {
				i = m_toolsScheduledAfterParsingList.erase(i);
				delete tool;
			}
			else {
				++i;
			}
		}
	}

	QString Manager::currentGroup(const QString &name, bool usequeue, bool useproject)
	{
		if (useproject) {
			KileProject *project = m_ki->docManager()->activeProject();
			if(project) {
				QString cfg = configName(name, dynamic_cast<KConfig*>(project->config()));
				if(cfg.length() > 0) {
					return groupFor(name, cfg);
				}
			}
		}
		if(usequeue && !m_queue.isEmpty() && m_queue.tool() && (m_queue.tool()->name() == name) && (!m_queue.tool()->toolConfig().isEmpty())) {
			return groupFor(name, m_queue.tool()->toolConfig());
		}
		else {
			return groupFor(name, m_config);
		}
	}

	bool Manager::retrieveEntryMap(const QString & name, Config & map, bool usequeue, bool useproject, const QString & cfg /*= QString()*/)
	{
		QString group = (cfg.isEmpty()) ? currentGroup(name, usequeue, useproject) : groupFor(name, cfg);

		KILE_DEBUG() << "==KileTool::Manager::retrieveEntryMap=============" << endl;
		KILE_DEBUG() << "\t" << name << " => " << group << endl;
		if(m_config->hasGroup(group)) {
			map = m_config->entryMap(group);

			//use project overrides
			KileProject *project = m_ki->docManager()->activeProject();
			if(useproject && project) {
				KConfig *prjcfg = dynamic_cast<KConfig*>(project->config());
				if(prjcfg) {
					QString grp = groupFor(name, prjcfg);
					Config prjmap = prjcfg->entryMap(grp);
					for (Config::Iterator it  = prjmap.begin(); it != prjmap.end(); ++it) {
						map[it.key()] = it.value();
					}
				}
			}
		}
		else {
			return false;
		}

		return true;
	}

	void Manager::saveEntryMap(const QString & name, Config & map, bool usequeue, bool useproject)
	{
		KILE_DEBUG() << "==KileTool::Manager::saveEntryMap=============" << endl;
		QString group = currentGroup(name, usequeue, useproject);
		KILE_DEBUG() << "\t" << name << " => " << group << endl;
		KConfigGroup configGroup = m_config->group(group);

		Config::Iterator it;
		for(it = map.begin() ; it != map.end(); ++it) {
			if(!it.value().isEmpty()) {
				configGroup.writeEntry(it.key(), it.value());
			}
		}
	}

	bool Manager::configure(Base *tool, const QString& cfg /* = QString() */)
	{
		KILE_DEBUG() << "==KileTool::Manager::configure()===============" << endl;
		//configure the tool

		Config map;

		if(!retrieveEntryMap(tool->name(), map, true, true, cfg)) {
		QString group = (cfg.isEmpty()) ? currentGroup(tool->name(), true, true) : groupFor(tool->name(), cfg);
			m_log->printMessage(Error, i18n("Cannot find the tool \"%1\" in the configuration database.", group));
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

	KileView::Manager* Manager::viewManager()
	{
		return m_ki->viewManager();
	}

	KileTool::LivePreviewManager* Manager::livePreviewManager()
	{
		return m_ki->livePreviewManager();
	}

	KileParser::Manager* Manager::parserManager()
	{
		return m_ki->parserManager();
	}

	QStringList toolList(KConfig *config, bool menuOnly)
	{
		KILE_DEBUG() << "==KileTool::toolList()==================" << endl;

		QStringList groups = config->groupList(), tools;
		QRegExp re = QRegExp("Tool/(.+)/.+");
		QString name;

		for(int i = 0; i < groups.count(); ++i) {
			if(re.exactMatch(groups[i])) {
				name = configName(re.cap(1), config);

				if(name.isEmpty() || !groups[i].endsWith(name)) {
					continue;
				}

				if((!menuOnly) || (menuFor(re.cap(1), config) != "none")) {
					tools.append(re.cap(1));
				}
			}
		}

		tools.sort();
// 		KILE_DEBUG() << "tools " << tools.join(", ");

		return tools;
	}

	QList<ToolConfigPair> toolsWithConfigurationsBasedOnClass(KConfig *config, const QString& className)
	{
		QStringList groups = config->groupList(), tools;
		QRegExp re = QRegExp("Tool/(.+)/(.+)");
		QList<ToolConfigPair> toReturn;

		for(int i = 0; i < groups.count(); ++i) {
			if(re.exactMatch(groups[i])) {
				const QString toolName = re.cap(1);
				const QString configName = re.cap(2);

				if(toolName.isEmpty()) {
					continue;
				}

				if(config->group(groups[i]).readEntry("class", "") == className) {
					toReturn.push_back(ToolConfigPair(toolName, configName));
				}
			}
		}

		qSort(toReturn);

		return toReturn;
	}

	QString configName(const QString & tool, KConfig *config)
	{
		return config->group("Tools").readEntry(tool, QString());
	}

	void Manager::setConfigName(const QString &tool, const QString &name)
	{
		KileTool::setConfigName(tool, name, m_config);
	}

	void setConfigName(const QString &tool, const QString &name, KConfig *config)
	{
		KILE_DEBUG() << "==KileTool::Manager::setConfigName(" << tool << "," << name << ")===============" << endl;
		config->group("Tools").writeEntry(tool, name);
	}

	QString groupFor(const QString &tool, KConfig *config)
	{
		return groupFor(tool, configName(tool, config));
	}

	QString groupFor(const QString& tool, const QString& cfg /* = Default */ )
	{
		QString group = "Tool/" + tool + '/' + cfg;
		KILE_DEBUG() << "groupFor(const QString &" << tool << ", const QString & " << cfg << " ) = " << group;
		return group;
	}

	void extract(const QString &str, QString &tool, QString &cfg)
	{
		static QRegExp re("([^\\(]*)\\((.*)\\)");
		QString lcl = str;
		lcl.trimmed();
		cfg.clear();
		if(re.exactMatch(lcl)) {
			tool = re.cap(1).trimmed();
			cfg = re.cap(2).trimmed();
		}
		else
			tool = lcl;
		KILE_DEBUG() << "===void extract(const QString &str = " << str << " , QString &tool = " << tool << ", QString &cfg = " << cfg << " )===" << endl;
	}

	QString format(const QString & tool, const QString &cfg)
	{
		if (!cfg.isEmpty()) {
			return tool + '(' + cfg + ')';
		}
		else {
			return tool;
		}
	}

	QStringList configNames(const QString &tool, KConfig *config)
	{
		QStringList groups = config->groupList(), configs;
		QRegExp re = QRegExp("Tool/"+ tool +"/(.+)");

		for(int i = 0; i < groups.count(); ++i) {
			if(re.exactMatch(groups[i])) {
				configs.append(re.cap(1));
			}
		}

		return configs;
	}

	QString menuFor(const QString &tool, KConfig *config)
	{
		return config->group("ToolsGUI").readEntry(tool, "Other,application-x-executable").section(',', 0, 0);
	}

	QString iconFor(const QString &tool, KConfig *config)
	{
		return config->group("ToolsGUI").readEntry(tool, "Other,application-x-executable").section(',', 1, 1);
	}

	void setGUIOptions(const QString &tool, const QString &menu, const QString &icon, KConfig *config)
	{
		QString entry = menu + ',' + icon;

		config->group("ToolsGUI").writeEntry(tool, entry);
	}

	QString categoryFor(const QString &clss)
	{
		if(clss == "Compile" || clss == "LaTeX") {
			return "Compile";
		}
		if(clss == "Convert") {
			return "Convert";
		}
		if(clss == "View" || clss == "ViewBib" || clss == "ViewHTML" || clss == "ForwardDVI") {
			return "View";
		}
		if(clss == "Sequence") {
			return "Sequence";
		}
		if(clss == "Archive") {
			return "Archive";
		}

		return "Base";
	}
}

#include "kiletoolmanager.moc"
