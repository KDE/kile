/*************************************************************************
   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                 2008-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "errorhandler.h"

#include <QFileInfo>
#include <QHash>
#include <QLabel>
#include <QMenu>
#include <QRegExp>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>

#include <KActionCollection>
#include <KLocale>
#include <KUrl>
#include <KTabWidget>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KSelectAction>
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kileproject.h"
#include "kiletool_enums.h"
#include "kileviewmanager.h"
#include "outputinfo.h"
#include "utilities.h"
#include "widgets/logwidget.h"
#include "widgets/outputview.h"
#include "widgets/sidebar.h"

KileErrorHandler::KileErrorHandler(QObject *parent, KileInfo *info, KActionCollection *ac)
 : QObject(parent), m_ki(info), m_errorHanderToolBar(NULL), m_currentLaTeXOutputHandler(NULL)
{
	setObjectName("ErrorHandler");

	createActions(ac);
	setOutputActionsEnabled(false);

	m_compilationResultLabel = new QLabel();

	m_mainLogWidget = new KileWidget::LogWidget();
	m_mainLogWidget->setFocusPolicy(Qt::ClickFocus);
	m_mainLogWidget->setMinimumHeight(40);

	connect(m_mainLogWidget, SIGNAL(outputInfoSelected(const OutputInfo&)),
	        this, SLOT(jumpToProblem(const OutputInfo&)));
	connect(m_mainLogWidget, SIGNAL(showingErrorMessage(QWidget*)),
	        this, SIGNAL(showingErrorMessage(QWidget*)));
	connect(m_mainLogWidget, SIGNAL(showingErrorMessage(QWidget*)),
	        this, SLOT(showMessagesOutput()));
	m_errorLogWidget = new KileWidget::LogWidget(KileWidget::LogWidget::NoHideActions);
	connect(m_errorLogWidget, SIGNAL(outputInfoSelected(const OutputInfo&)),
	        this, SLOT(jumpToProblem(const OutputInfo&)));
	m_warningLogWidget = new KileWidget::LogWidget(KileWidget::LogWidget::NoHideActions);
	connect(m_warningLogWidget, SIGNAL(outputInfoSelected(const OutputInfo&)),
	        this, SLOT(jumpToProblem(const OutputInfo&)));
	m_badBoxLogWidget = new KileWidget::LogWidget(KileWidget::LogWidget::NoHideActions);
	connect(m_badBoxLogWidget, SIGNAL(outputInfoSelected(const OutputInfo&)),
	        this, SLOT(jumpToProblem(const OutputInfo&)));

	// FIXME: suggestions for icons: utilities-log-viewer, script-error, dialog-warning
	m_outputTabWidget = new QTabWidget();
	m_outputTabWidget->setTabPosition(QTabWidget::South);
	m_outputTabWidget->setTabsClosable(false);
	m_outputTabWidget->addTab(m_mainLogWidget, i18n("Messages"));
	m_outputTabWidget->addTab(m_errorLogWidget, i18n("Errors"));
	m_outputTabWidget->addTab(m_warningLogWidget, i18n("Warnings"));
	m_outputTabWidget->addTab(m_badBoxLogWidget, i18n("BadBoxes"));

	connect(m_ki->viewManager(), SIGNAL(textViewActivated(KTextEditor::View*)),
	        this, SLOT(updateCurrentLaTeXOutputHandler()));
	connect(m_ki->viewManager(), SIGNAL(textViewClosed(KTextEditor::View*,bool)),
	        this, SLOT(updateCurrentLaTeXOutputHandler()));
	connect(m_ki->docManager(), SIGNAL(documentOpened(KileDocument::TextInfo*)),
	        this, SLOT(updateCurrentLaTeXOutputHandler()));
	connect(m_ki->docManager(), SIGNAL(projectOpened(KileProject*)),
	        this, SLOT(handleProjectOpened(KileProject*)));

	showMessagesOutput();
}

KileErrorHandler::~KileErrorHandler()
{
}

void KileErrorHandler::createActions(KActionCollection *ac)
{
	m_viewLogAction = ac->addAction("ViewLog", this, SLOT(ViewLog()));
	m_viewLogAction->setText(i18n("View Log File"));
	m_viewLogAction->setShortcut(KShortcut(Qt::ALT + Qt::Key_0));
	m_viewLogAction->setIcon(KIcon("viewlog"));

	m_previousErrorAction = ac->addAction("PreviousError", this, SLOT(PreviousError()));
	m_previousErrorAction->setText(i18n("Previous LaTeX Error"));
	m_previousErrorAction->setIcon(KIcon("errorprev"));

	m_nextErrorAction = ac->addAction("NextError", this, SLOT(NextError()));
	m_nextErrorAction->setText(i18n("Next LaTeX Error"));
	m_nextErrorAction->setIcon(KIcon("errornext"));

	m_previousWarningAction = ac->addAction("PreviousWarning", this, SLOT(PreviousWarning()));
	m_previousWarningAction->setText(i18n("Previous LaTeX Warning"));
	m_previousWarningAction->setIcon(KIcon("warnprev"));

	m_nextWarningAction = ac->addAction("NextWarning", this, SLOT(NextWarning()));
	m_nextWarningAction->setText(i18n("Next LaTeX Warnings"));
	m_nextWarningAction->setIcon(KIcon("warnnext"));

	m_previousBadBoxAction = ac->addAction("PreviousBadBox", this, SLOT(PreviousBadBox()));
	m_previousBadBoxAction->setText(i18n("Previous LaTeX BadBox"));
	m_previousBadBoxAction->setIcon(KIcon("bboxprev"));

	m_nextBadBoxAction = ac->addAction("NextBadBox", this, SLOT(NextBadBox()));
	m_nextBadBoxAction->setText(i18n("Next LaTeX BadBox"));
	m_nextBadBoxAction->setIcon(KIcon("bboxnext"));
}

void KileErrorHandler::setErrorHandlerToolBar(QToolBar *toolBar)
{
	m_errorHanderToolBar = toolBar;
	toolBar->addAction(m_viewLogAction);
	toolBar->addAction(m_previousErrorAction);
	toolBar->addAction(m_nextErrorAction);
	toolBar->addAction(m_previousWarningAction);
	toolBar->addAction(m_nextWarningAction);
	toolBar->addAction(m_previousBadBoxAction);
	toolBar->addAction(m_nextBadBoxAction);

}

void KileErrorHandler::setOutputActionsEnabled(bool b)
{
	m_viewLogAction->setEnabled(b);
	m_previousErrorAction->setEnabled(b);
	m_nextErrorAction->setEnabled(b);
	m_previousWarningAction->setEnabled(b);
	m_nextWarningAction->setEnabled(b);
	m_previousBadBoxAction->setEnabled(b);
	m_nextBadBoxAction->setEnabled(b);
}

QLabel* KileErrorHandler::compilationResultLabel()
{
	return m_compilationResultLabel;
}

QWidget* KileErrorHandler::outputWidget()
{
	return m_outputTabWidget;
}

bool KileErrorHandler::areMessagesShown() const
{
	return m_mainLogWidget->isShowingOutput();
}

void KileErrorHandler::addEmptyLineToMessages()
{
	m_mainLogWidget->addEmptyLine();
}

void KileErrorHandler::startToolLogOutput()
{
	m_mainLogWidget->startToolLogOutput();
}

void KileErrorHandler::endToolLogOutput()
{
	m_mainLogWidget->endToolLogOutput();
}

void KileErrorHandler::printMessage(const QString& message)
{
	m_mainLogWidget->printMessage(message);
}

void KileErrorHandler::printMessage(int type, const QString& message, const QString &tool,
                                    const OutputInfo& outputInfo, bool allowSelection,
                                    bool scroll)
{
	m_mainLogWidget->printMessage(type, message, tool, outputInfo, allowSelection, scroll);
}

void KileErrorHandler::printProblem(int type, const QString& problem, const OutputInfo& outputInfo)
{
	m_mainLogWidget->printProblem(type, problem, outputInfo);
}

void KileErrorHandler::clearMessages()
{
	m_mainLogWidget->clear();
}

void KileErrorHandler::showMessagesOutput()
{
	m_outputTabWidget->setCurrentWidget(m_mainLogWidget);
}

void KileErrorHandler::showErrorsOutput()
{
	m_outputTabWidget->setCurrentWidget(m_errorLogWidget);
}

void KileErrorHandler::showWarningsOutput()
{
	m_outputTabWidget->setCurrentWidget(m_warningLogWidget);
}

void KileErrorHandler::showBadBoxesOutput()
{
	m_outputTabWidget->setCurrentWidget(m_badBoxLogWidget);
}

void KileErrorHandler::handleProjectOpened(KileProject *project)
{
	connect(project, SIGNAL(aboutToBeDestroyed(KileProject*)),
	        this, SLOT(updateCurrentLaTeXOutputHandler()),
	        Qt::UniqueConnection);
	connect(project, SIGNAL(projectItemAdded(KileProject*,KileProjectItem*)),
	        this, SLOT(updateCurrentLaTeXOutputHandler()),
	        Qt::UniqueConnection);
	connect(project, SIGNAL(projectItemRemoved(KileProject*,KileProjectItem*)),
	        this, SLOT(updateCurrentLaTeXOutputHandler()),
	        Qt::UniqueConnection);
}

void KileErrorHandler::handleLaTeXToolDone(KileTool::Base *tool, int i, bool childToolSpawned)
{
	Q_UNUSED(i);

	KileTool::LaTeX *latex = dynamic_cast<KileTool::LaTeX*>(tool);
	if(!latex) {
		return;
	}
	if(childToolSpawned) {
		return;
	}
	if(latex->latexOutputHandler() == m_currentLaTeXOutputHandler) {
		updateForCompilationResult();
	}
}

void KileErrorHandler::handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child)
{
	if(!dynamic_cast<KileTool::LaTeX*>(parent) || !dynamic_cast<KileTool::LaTeX*>(child)) {
		return;
	}

	connect(child, SIGNAL(done(KileTool::Base*, int, bool)),
		this, SLOT(handleLaTeXToolDone(KileTool::Base*, int, bool)));
}

void KileErrorHandler::updateCurrentLaTeXOutputHandler()
{
	LaTeXOutputHandler *h = NULL;
	m_ki->getCompileName(false, &h);
	if(h == m_currentLaTeXOutputHandler) {
		return;
	}
	m_currentLaTeXOutputHandler = h;

	if(!m_currentLaTeXOutputHandler) {
		setOutputActionsEnabled(false);
		clearErrorOutput();
	}
	else {
		setOutputActionsEnabled(true);
		updateForCompilationResult();
	}
	emit(currentLaTeXOutputHandlerChanged(m_currentLaTeXOutputHandler));
}

void KileErrorHandler::updateForCompilationResult()
{
	if(!m_currentLaTeXOutputHandler) {
		return;
	}
	m_errorLogWidget->clear();
	displayProblemsInLogWidget(m_errorLogWidget, m_currentLaTeXOutputHandler->outputList(), KileErrorHandler::OnlyErrors);
	m_warningLogWidget->clear();
	displayProblemsInLogWidget(m_warningLogWidget, m_currentLaTeXOutputHandler->outputList(), KileErrorHandler::OnlyWarnings);
	m_badBoxLogWidget->clear();
	displayProblemsInLogWidget(m_badBoxLogWidget, m_currentLaTeXOutputHandler->outputList(), KileErrorHandler::OnlyBadBoxes);

	const int nErrors = m_currentLaTeXOutputHandler->numberOfErrors();
	const int nWarnings = m_currentLaTeXOutputHandler->numberOfWarnings();
	const int nBadBoxes = m_currentLaTeXOutputHandler->numberOfBadBoxes();
	QString errorString, warningString, badBoxString;

	if(nErrors >= 0) {
		errorString = i18n("Errors: %1", nErrors);
	}
	if(nWarnings >= 0) {
		warningString = i18n("Warnings: %1", nWarnings);
	}
	if(nBadBoxes >= 0) {
		badBoxString = i18n("BadBoxes: %1", nBadBoxes);
	}

	m_compilationResultLabel->setText(i18nc("Result of the compilation w.r.t. number of errors/warnings/badboxes",
	                                        "%1  %2  %3", errorString, warningString, badBoxString));
}

void KileErrorHandler::clearErrorOutput()
{
	m_compilationResultLabel->clear();
	m_errorLogWidget->clear();
	m_warningLogWidget->clear();
	m_badBoxLogWidget->clear();
}

void KileErrorHandler::setMostRecentLogInformation(const QString& logFile, const LatexOutputInfoArray& outputInfoList)
{
	Q_UNUSED(logFile);

	// add them to the log widget
	displayProblemsInMainLogWidget(outputInfoList);
}

void KileErrorHandler::displayProblemsInMainLogWidget(const LatexOutputInfoArray& infoList)
{
	displayProblemsInLogWidget(m_mainLogWidget, infoList);
}

void KileErrorHandler::displayProblemsInLogWidget(KileWidget::LogWidget *logWidget, const LatexOutputInfoArray& infoList, ProblemType problemType)
{
	QString message;
	int type = KileTool::Info;

	//print detailed error info
	logWidget->setUpdatesEnabled(false);

	for(QList<LatexOutputInfo>::const_iterator i = infoList.begin();
	                                           i != infoList.end(); ++i) {

		const LatexOutputInfo& info = *i;
		message = info.source() + ':' + QString::number(info.sourceLine()) + ':' + info.message();
		switch(info.type()) {
			case LatexOutputInfo::itmBadBox:
				if(problemType == AllProblems || problemType == OnlyBadBoxes) {
					type = KileTool::ProblemBadBox;
				}
				else {
					continue;
				}
				break;
			case LatexOutputInfo::itmError:
				if(problemType == AllProblems || problemType == OnlyErrors) {
					type = KileTool::ProblemError;
				}
				else {
					continue;
				}
				break;
			case LatexOutputInfo::itmWarning:
				if(problemType == AllProblems || problemType == OnlyWarnings) {
					type = KileTool::ProblemWarning;
				}
				else {
					continue;
				}
				break;
			default:
				type = KileTool::Info;
				break;
		}
		KileWidget::LogWidget::ProblemInformation problem;
		problem.type = type;
		problem.message = message;
		problem.outputInfo = info;
		logWidget->printMessage(type, message, QString(), info, false, false);
	}

		logWidget->setUpdatesEnabled(true);
		logWidget->scrollToBottom();
}

void KileErrorHandler::printNoInformationAvailable()
{
	m_mainLogWidget->printMessage(i18n("No information about warnings or errors is available."));
}

void KileErrorHandler::ViewLog()
{
	if(!m_currentLaTeXOutputHandler) {
		printNoInformationAvailable();
		return;
	}

	const LatexOutputInfoArray& outputInfoList = m_currentLaTeXOutputHandler->outputList();

	KileWidget::LogWidget *logWidget = m_mainLogWidget;
	m_ki->focusLog();

	QFile logFile(m_currentLaTeXOutputHandler->logFile());
	if(!m_currentLaTeXOutputHandler->logFile().isEmpty() && logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QHash<int, OutputInfo> hash;

		for(QList<LatexOutputInfo>::const_iterator i = outputInfoList.begin();
		                                           i != outputInfoList.end(); ++i) {
			LatexOutputInfo info = *i;
			hash[info.outputLine()] = info;
		}

		QTextStream textStream(&logFile);

		for(int lineNumber = 0; !textStream.atEnd(); ++lineNumber) {
			int type = -1;
			const QString line = textStream.readLine();
			if(hash.find(lineNumber) != hash.end()) {
				switch(hash[lineNumber].type()) {
					case LatexOutputInfo::itmError:
						type = KileTool::Error;
						break;
					case LatexOutputInfo::itmWarning:
						type = KileTool::Warning;
						break;
					case LatexOutputInfo::itmBadBox:
						type = KileTool::ProblemBadBox;
						break;
				}
			}
			// don't scroll to the item as this will lead to severely degraded performance
			logWidget->printMessage(type, line, QString(), hash[lineNumber], true, false);
		}

		logWidget->scrollToBottom();
	}
	else {
		logWidget->printProblem(KileTool::Error, i18n("Cannot open log file; did you run LaTeX?"));
	}
}

void KileErrorHandler::jumpToFirstError()
{
	if(!m_currentLaTeXOutputHandler) {
		printNoInformationAvailable();
		return;
	}

	const LatexOutputInfoArray& outputInfoList = m_currentLaTeXOutputHandler->outputList();

	int sz = outputInfoList.size();
	for(int i = 0; i < sz; ++i) {
		if(outputInfoList[i].type() == LatexOutputInfo::itmError) {
			// this has to be before calling 'jumpToProblem' as this might change 'm_currentLaTeXOutputHandler'
			m_currentLaTeXOutputHandler->setCurrentError(i);
			jumpToProblem(outputInfoList[i]);
			break;
		}
	}
}

void KileErrorHandler::jumpToProblem(const OutputInfo& info)
{
	QString file = m_ki->getFullFromPrettyName(info, info.source());

	if(!file.isEmpty()) {
		m_ki->docManager()->fileOpen(KUrl(file));
		int line = (info.sourceLine() > 0) ? (info.sourceLine() - 1) : 0;

		KTextEditor::Document *doc = m_ki->docManager()->docFor(KUrl(file));
		if(doc) {
			KTextEditor::View* view = doc->views().first();
			if(view) {
				view->setCursorPosition(KTextEditor::Cursor(line, 0));
			}
		}
	}
}

void KileErrorHandler::jumpToProblem(int type, bool forward)
{
	if(!m_currentLaTeXOutputHandler) {
		printNoInformationAvailable();
		return;
	}

	const LatexOutputInfoArray& outputInfoList = m_currentLaTeXOutputHandler->outputList();

	if (!outputInfoList.isEmpty()) {
		int sz = outputInfoList.size();
		int pl = forward ? 1 : -1;
		bool found = false;

		//look for next problem of requested type
		for(int i = 0; i < sz; ++i) {
			//always look at the whole outputInfo array, but start
			//at the problem adjacent to the current error
			//if we go beyond the bounds of the array we use
			//a simple "modulo" calculation to get within bounds again
			int index = (m_currentLaTeXOutputHandler->currentError() + (i + 1) *pl) % sz;
			while(index < 0) {
				index += sz;
			}

			if(outputInfoList[index].type() == type) {
				m_currentLaTeXOutputHandler->setCurrentError(index);
				found = true;
				break;
			}
		}

		if(!found) {
			return;
		}

		//If the log file is being viewed, use this to jump to the errors,
		//otherwise, use the error summary display
		m_mainLogWidget->highlight(outputInfoList[m_currentLaTeXOutputHandler->currentError()]);

		jumpToProblem(outputInfoList[m_currentLaTeXOutputHandler->currentError()]);
	}

	if(outputInfoList.isEmpty()) {
		m_mainLogWidget->printMessage(i18n("No LaTeX warnings/errors detected."));
	}
}

void KileErrorHandler::NextError()
{
	jumpToProblem(LatexOutputInfo::itmError, true);
}

void KileErrorHandler::PreviousError()
{
	jumpToProblem(LatexOutputInfo::itmError, false);
}

void KileErrorHandler::NextWarning()
{
	jumpToProblem(LatexOutputInfo::itmWarning, true);
}

void KileErrorHandler::PreviousWarning()
{
	jumpToProblem(LatexOutputInfo::itmWarning, false);
}

void KileErrorHandler::NextBadBox()
{
	jumpToProblem(LatexOutputInfo::itmBadBox, true);
}

void KileErrorHandler::PreviousBadBox()
{
	jumpToProblem(LatexOutputInfo::itmBadBox, false);
}

#include "errorhandler.moc"
