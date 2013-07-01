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
#include <QMenu>
#include <QRegExp>
#include <QTimer>

#include <KActionCollection>
#include <KConfig>
#include <KLocale>
#include <KMessageBox>
#include <KParts/PartManager>
#include <KSelectAction>

#include "configurationmanager.h"
#include "errorhandler.h"
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

	Manager::Manager(KileInfo *ki, KConfig *config, KileWidget::OutputView *output, KParts::PartManager *manager, QStackedWidget *stack, KAction *stop, uint to, KActionCollection *ac) :
		m_ki(ki),
		m_config(config),
		m_output(output),
		m_pm(manager),
		m_stack(stack),
		m_stop(stop),
		m_bClear(true),
		m_nLastResult(Success),
		m_nTimeout(to),
		m_bibliographyBackendSelectAction(NULL)
	{
		connect(m_ki->parserManager(), SIGNAL(documentParsingComplete()), this, SLOT(handleDocumentParsingComplete()));

		connect(this, SIGNAL(childToolSpawned(KileTool::Base*,KileTool::Base*)),
		        m_ki->errorHandler(), SLOT(handleSpawnedChildTool(KileTool::Base*, KileTool::Base*)));

		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(enableClear()));
		connect(stop, SIGNAL(triggered()), this, SLOT(stop()));
		connect(stop, SIGNAL(destroyed(QObject*)), this, SLOT(stopActionDestroyed()));

		connect(m_ki->errorHandler(), SIGNAL(currentLaTeXOutputHandlerChanged(LaTeXOutputHandler*)), SLOT(currentLaTeXOutputHandlerChanged(LaTeXOutputHandler*)));

		//create actions must be invoked before buildBibliographyBackendSelection()!
		createActions(ac);
		buildBibliographyBackendSelection();

		connect(m_ki->configurationManager(), SIGNAL(configChanged()), SLOT(buildBibliographyBackendSelection()));
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

	void Manager::handleDocumentParsingComplete()
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
			m_ki->errorHandler()->clearMessages();
			m_output->clear();
		}

		if(dynamic_cast<KileTool::LaTeX*>(tool)) {
			connect(tool, SIGNAL(done(KileTool::Base*, int, bool)),
			        m_ki->errorHandler(), SLOT(handleLaTeXToolDone(KileTool::Base*, int, bool)));
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
		parent->setupAsChildTool(tool);

		return runImmediately(tool, true, block, parent);
	}

	int Manager::runNextInQueue()
	{
		Base *head = m_queue.tool();
		if(head) {
			if (m_ki->errorHandler()->areMessagesShown()) {
				m_ki->errorHandler()->addEmptyLineToMessages();
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

			m_ki->errorHandler()->startToolLogOutput();
			emit(toolStarted());

			return Running;
		}

		return ConfigureFailed;
	}

	Base* Manager::createTool(const QString& name, const QString &cfg, bool prepare)
	{
		if(!m_factory) {
			m_ki->errorHandler()->printMessage(Error, i18n("No factory installed, contact the author of Kile."));
			return NULL;
		}

		Base* pTool = m_factory->create(name, cfg, prepare);
		if(!pTool) {
			m_ki->errorHandler()->printMessage(Error, i18n("Unknown tool %1.", name));
			return NULL;
		}
		initTool(pTool);
		return pTool;
	}

	void Manager::initTool(Base *tool)
	{
		tool->setInfo(m_ki);
		tool->setConfig(m_config);

		connect(tool, SIGNAL(message(int, const QString &, const QString &)), m_ki->errorHandler(), SLOT(printMessage(int, const QString &, const QString &)));
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

		m_ki->errorHandler()->endToolLogOutput();

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
			m_ki->errorHandler()->printMessage(Error, i18n("Cannot find the tool \"%1\" in the configuration database.", group));
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

	QString commandFor(const QString& toolName, const QString& configName, KConfig *config)
	{
		return config->group(groupFor(toolName, configName)).readEntry("command", "");
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

bool KileTool::Manager::containsBibliographyTool(const ToolConfigPair& p) const
{
	return m_bibliographyToolsList.contains(p);
}

KileTool::ToolConfigPair KileTool::Manager::findFirstBibliographyToolForCommand(const QString& command) const
{
	// for now we will just select the first suitable tool
	Q_FOREACH(const KileTool::ToolConfigPair& tool, m_bibliographyToolsList) {
		const QString toolCommand = commandFor(tool, m_config);
		if (QString::compare(command, toolCommand, Qt::CaseInsensitive) == 0) {
			return tool;
		}
	}

	return KileTool::ToolConfigPair();
}

void KileTool::Manager::buildBibliographyBackendSelection()
{
	m_bibliographyBackendSelectAction->removeAllActions();
	m_bibliographyBackendSelectAction->menu()->clear();
	for(QMap<ToolConfigPair, KAction*>::iterator i = m_bibliographyBackendActionMap.begin(); i != m_bibliographyBackendActionMap.end(); ++i) {
		delete i.value();
	}
	m_bibliographyBackendActionMap.clear();
	m_bibliographyToolsList.clear();

	m_bibliographyBackendSelectAction->addAction(m_bibliographyBackendAutodetectAction);

	m_bibliographyToolsList = toolsWithConfigurationsBasedOnClass(m_config, BibliographyCompile::ToolClass);
	qSort(m_bibliographyToolsList); // necessary for the user-visible actions in the menu bar

	Q_FOREACH(const ToolConfigPair& tool, m_bibliographyToolsList) {
		// create an action for backend selection
		KAction* action = m_bibliographyBackendSelectAction->addAction(tool.userStringRepresentation());
		action->setData(qVariantFromValue(tool));
		m_bibliographyBackendActionMap[tool] = action;
	}

	m_bibliographyBackendSelectAction->menu()->addSeparator();
	m_bibliographyBackendSelectAction->menu()->addAction(m_bibliographyBackendResetAutodetectedAction);

	currentLaTeXOutputHandlerChanged(m_ki->findCurrentLaTeXOutputHandler());
}

void KileTool::Manager::createActions(KActionCollection *ac)
{
	delete m_bibliographyBackendSelectAction;

	m_bibliographyBackendSelectAction = new KSelectAction(i18n("Bibliography Back End"), this);
	m_bibliographyBackendAutodetectAction = m_bibliographyBackendSelectAction->addAction(i18n("Auto-Detect"));
	m_bibliographyBackendAutodetectAction->setStatusTip(i18n("Auto-detect the bibliography back end from LaTeX output"));
	m_bibliographyBackendSelectAction->setChecked(false);

	ac->addAction("bibbackend_select", m_bibliographyBackendSelectAction);

	m_bibliographyBackendResetAutodetectedAction = new KAction(i18n("Reset Auto-Detected Back End"), this);
	m_bibliographyBackendResetAutodetectedAction->setEnabled(false);

	connect(m_bibliographyBackendSelectAction, SIGNAL(triggered(QAction*)), SLOT(bibliographyBackendSelectedByUser()));
	connect(m_bibliographyBackendResetAutodetectedAction, SIGNAL(triggered(bool)), SLOT(resetAutodetectedBibliographyBackend()));
	connect(m_bibliographyBackendAutodetectAction, SIGNAL(toggled(bool)),
	        m_bibliographyBackendResetAutodetectedAction, SLOT(setEnabled(bool)));
}


void KileTool::Manager::bibliographyBackendSelectedByUser()
{
	LaTeXOutputHandler* h = m_ki->findCurrentLaTeXOutputHandler();
	QAction* currentBackendAction = m_bibliographyBackendSelectAction->currentAction();

	if (currentBackendAction == m_bibliographyBackendAutodetectAction) {
		h->setBibliographyBackendToolUserOverride(ToolConfigPair());
	}
	else {
		//here we do not need to check existence of tool
		h->setBibliographyBackendToolUserOverride(currentBackendAction->data().value<KileTool::ToolConfigPair>());
		h->setBibliographyBackendToolAutoDetected(ToolConfigPair());
	}
}

void KileTool::Manager::currentLaTeXOutputHandlerChanged(LaTeXOutputHandler* handler)
{
	if(!handler) {
		m_bibliographyBackendSelectAction->setEnabled(false);
		return;
	}

	m_bibliographyBackendSelectAction->setEnabled(true);

	if (!m_bibliographyBackendActionMap.empty()) {
		ToolConfigPair userOverrideBibBackend = handler->bibliographyBackendToolUserOverride();
		if(!userOverrideBibBackend.isValid()) {
			m_bibliographyBackendAutodetectAction->setChecked(true);
		}
		else {
			// here we have to check whether the action exists
			QMap<ToolConfigPair, KAction*>::const_iterator i = m_bibliographyBackendActionMap.constFind(userOverrideBibBackend);
			if (i != m_bibliographyBackendActionMap.constEnd()) {
				i.value()->setChecked(true);
			}
			else {
				// the user previously selected a bibtool backend which is (no longer) present - let's use autodetection;
				// this is done analogously in 'LaTeX::determineBibliographyBackend'
				m_bibliographyBackendAutodetectAction->setChecked(true);
			}
		}
	}
	else {
		m_bibliographyBackendAutodetectAction->setChecked(true);
	}
}

void KileTool::Manager::resetAutodetectedBibliographyBackend()
{
	LaTeXOutputHandler* h = m_ki->findCurrentLaTeXOutputHandler();
	if (h) {
		h->setBibliographyBackendToolAutoDetected(ToolConfigPair());
	}
}

#include "kiletoolmanager.moc"
