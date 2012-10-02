/**************************************************************************
*   Copyright (C) 2011-2012 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "parsermanager.h"

#include "documentinfo.h"
#include "errorhandler.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kiletool_enums.h"
#include "latexoutputparser.h"
#include "parserthread.h"
#include "widgets/logwidget.h"

namespace KileParser {

Manager::Manager(KileInfo *info, QObject *parent) :
	QObject(parent),
	m_ki(info)
{
	KILE_DEBUG();
	m_documentParserThread = new DocumentParserThread(m_ki, this);
	// we have to make this connection 'blocking' to ensure that when 'ParserThread::isDocumentParsingComplete()'
	// returns true, all document info objects have been passed the information obtained from parsing already
	connect(m_documentParserThread, SIGNAL(parsingComplete(const KUrl&, KileParser::ParserOutput*)),
	        m_ki->docManager(), SLOT(handleParsingComplete(const KUrl&, KileParser::ParserOutput*)), Qt::BlockingQueuedConnection);
	// the next two can't be made 'blocking' as they are emitted when a mutex is held
	connect(m_documentParserThread, SIGNAL(parsingQueueEmpty()),
	        this, SIGNAL(documentParsingComplete()), Qt::QueuedConnection);
	connect(m_documentParserThread, SIGNAL(parsingStarted()),
	        this, SIGNAL(documentParsingStarted()), Qt::QueuedConnection);
	m_documentParserThread->start();

	m_outputParserThread = new OutputParserThread(m_ki, this);
	connect(m_outputParserThread, SIGNAL(parsingComplete(const KUrl&, KileParser::ParserOutput*)),
	        this, SLOT(handleOutputParsingComplete(const KUrl&, KileParser::ParserOutput*)), Qt::QueuedConnection);
	m_outputParserThread->start();
}


Manager::~Manager()
{
	KILE_DEBUG() << "destroying...";
	m_documentParserThread->stopParsing();
	m_outputParserThread->stopParsing();
}

void Manager::parseDocument(KileDocument::TextInfo* textInfo)
{
	KILE_DEBUG() << textInfo;
	m_documentParserThread->addDocument(textInfo);
}

void Manager::parseOutput(KileTool::Base *tool, const QString& fileName, const QString& sourceFile,
                                                const QString& texFileName, int selrow, int docrow)
{
	KILE_DEBUG() << fileName << sourceFile;
	m_outputParserThread->addLaTeXLogFile(fileName, sourceFile, texFileName, selrow, docrow);
	connect(tool, SIGNAL(aboutToBeDestroyed(KileTool::Base*)),
	        this, SLOT(removeToolFromUrlHash(KileTool::Base*)), Qt::UniqueConnection);
	if(!m_urlToToolHash.contains(fileName, tool)) {
		m_urlToToolHash.insert(fileName, tool);
	}
}

bool Manager::isDocumentParsingComplete()
{
	return m_documentParserThread->isParsingComplete();
}

void Manager::stopDocumentParsing(const KUrl& url)
{
	m_documentParserThread->removeDocument(url);
}

void Manager::handleOutputParsingComplete(const KUrl& url, KileParser::ParserOutput *output)
{
	KILE_DEBUG();
	const QString fileName = url.toLocalFile();

	QList<KileTool::Base*> toolList = m_urlToToolHash.values(fileName);
	m_urlToToolHash.remove(fileName);

	LaTeXOutputParserOutput *latexOutput = dynamic_cast<LaTeXOutputParserOutput*>(output);
	if(!latexOutput) {
		KILE_DEBUG() << "NULL output given";
		return;
	}
	if(toolList.isEmpty()) { // no tool was found, which means that all the tools for 'url'
		return;          // have been killed and we do nothing
	}
	if(!latexOutput->problem.isEmpty()) {
		m_ki->errorHandler()->printProblem(KileTool::Warning, latexOutput->problem);
		return;
	}
	// use the returned list as the new global error information list
	m_ki->errorHandler()->setMostRecentLogInformation(latexOutput->logFile, latexOutput->infoList);
	// finally, inform the tools waiting for the error information
	Q_FOREACH(KileTool::Base *tool, toolList) {
		tool->installLaTeXOutputParserResult(latexOutput->nErrors, latexOutput->nWarnings,
		                                                           latexOutput->nBadBoxes,
		                                                           latexOutput->infoList,
		                                                           latexOutput->logFile);
	}
}

void Manager::removeToolFromUrlHash(KileTool::Base *tool)
{
	KILE_DEBUG();
	QMultiHash<QString, KileTool::Base*>::iterator i = m_urlToToolHash.begin();
	while(i != m_urlToToolHash.end()) {
		const QString fileName = i.key();
		if(i.value() == tool) {
			i = m_urlToToolHash.erase(i);
			if(!m_urlToToolHash.contains(fileName)) {
				m_outputParserThread->removeFile(fileName);
			}
		}
		else {
			++i;
		}
	}
}

}

#include "parsermanager.moc"
