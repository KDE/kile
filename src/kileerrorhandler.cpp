/*************************************************************************
    begin                : Tue May 25 2004
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2008-2011 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kileerrorhandler.h"

#include <QFileInfo>
#include <QHash>
#include <QRegExp>

#include <KLocale>
#include <KUrl>
#include <KTabWidget>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "kiletool_enums.h"
#include "widgets/logwidget.h"
#include "widgets/outputview.h"
#include "kileinfo.h"
#include "outputinfo.h"
#include "kiledocmanager.h"
#include "widgets/sidebar.h"

KileErrorHandler::KileErrorHandler(QObject *parent, KileInfo *info, const char *name)
 : QObject(parent), m_ki(info), m_nCurrentError(-1)
{
	setObjectName(name);
}


KileErrorHandler::~KileErrorHandler()
{
}


void KileErrorHandler::reset()
{
	m_nCurrentError = -1;
	m_mostRecentLogFile.clear();
	m_mostRecentLaTeXOutputInfoList.clear();
}

void KileErrorHandler::setMostRecentLogInformation(const QString& logFile, const LatexOutputInfoArray& outputInfoList)
{
	m_mostRecentLogFile = logFile;
	m_mostRecentLaTeXOutputInfoList = outputInfoList;
	// and add them to the log widget
	displayProblemsInLogWidget(outputInfoList);
}

void KileErrorHandler::displayProblemsInLogWidget(const LatexOutputInfoArray& infoList)
{
	QString message;
	int type;

	//print detailed error info
	m_ki->logWidget()->setUpdatesEnabled(false);

	for(QList<LatexOutputInfo>::const_iterator i = infoList.begin();
	                                           i != infoList.end(); ++i) {

		const LatexOutputInfo& info = *i;
		message = info.source() + ':' + QString::number(info.sourceLine()) + ':' + info.message();
		switch(info.type()) {
			case LatexOutputInfo::itmBadBox:
				type = KileTool::ProblemBadBox;
				break;
			case LatexOutputInfo::itmError:
				type = KileTool::ProblemError;
				break;
			case LatexOutputInfo::itmWarning:
				type = KileTool::ProblemWarning;
				break;
			default:
				type = KileTool::Info;
				break;
		}
		KileWidget::LogWidget::ProblemInformation problem;
		problem.type = type;
		problem.message = message;
		problem.outputInfo = info;
		m_ki->logWidget()->printMessage(type, message, QString(), info, false, false);
	}

		m_ki->logWidget()->setUpdatesEnabled(true);
		m_ki->logWidget()->scrollToBottom();

}

void KileErrorHandler::ViewLog()
{
	KileWidget::LogWidget *logWidget = m_ki->logWidget();
	m_ki->focusLog();

	QFile logFile(m_mostRecentLogFile);
	if(!m_mostRecentLogFile.isEmpty() && logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QHash<int, OutputInfo> hash;

		for(QList<LatexOutputInfo>::iterator i = m_mostRecentLaTeXOutputInfoList.begin();
		                                     i != m_mostRecentLaTeXOutputInfoList.end(); ++i) {
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
	int sz = m_mostRecentLaTeXOutputInfoList.size();
	for(int i = 0; i < sz; ++i) {
		if(m_mostRecentLaTeXOutputInfoList[i].type() == LatexOutputInfo::itmError) {
			jumpToProblem(m_mostRecentLaTeXOutputInfoList[i]);
			m_nCurrentError = i;
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
	// 'm_mostRecentLogFile' is only nonempty when output information has been
	// provided
	const bool outputInformationPresent = !m_mostRecentLogFile.isEmpty();

	if (!m_mostRecentLaTeXOutputInfoList.isEmpty()) {
		int sz = m_mostRecentLaTeXOutputInfoList.size();
		int pl = forward ? 1 : -1;
		bool found = false;

		//look for next problem of requested type
		for(int i = 0; i < sz; ++i) {
			//always look at the whole outputInfo array, but start
			//at the problem adjacent to the current error
			//if we go beyond the bounds of the array we use
			//a simple "modulo" calculation to get within bounds again
			int index = (m_nCurrentError + (i + 1) *pl) % sz;
			while(index < 0) {
				index += sz;
			}

			if(m_mostRecentLaTeXOutputInfoList[index].type() == type) {
				m_nCurrentError = index;
				found = true;
				break;
			}
		}

		if(!found) {
			return;
		}

		//If the log file is being viewed, use this to jump to the errors,
		//otherwise, use the error summary display
		m_ki->logWidget()->highlight(m_mostRecentLaTeXOutputInfoList[m_nCurrentError]);

		jumpToProblem(m_mostRecentLaTeXOutputInfoList[m_nCurrentError]);
	}

	if(m_mostRecentLaTeXOutputInfoList.isEmpty() && outputInformationPresent) {
		m_ki->logWidget()->printMessage(i18n("No LaTeX warnings/errors detected."));
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

#include "kileerrorhandler.moc"
