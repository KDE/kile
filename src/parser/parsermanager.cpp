/**************************************************************************
*   Copyright (C) 2011-2014 by Michel Ludwig (michel.ludwig@kdemail.net)  *
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
    qCDebug(LOG_KILE_PARSER);
    m_documentParserThread = new DocumentParserThread(m_ki, this);
    // we have to make this connection 'blocking' to ensure that when 'ParserThread::isDocumentParsingComplete()'
    // returns true, all document info objects have been passed the information obtained from parsing already
    connect(m_documentParserThread, SIGNAL(parsingComplete(QUrl,KileParser::ParserOutput*)),
            m_ki->docManager(), SLOT(handleParsingComplete(QUrl,KileParser::ParserOutput*)), Qt::BlockingQueuedConnection);
    // the next two can't be made 'blocking' as they are emitted when a mutex is held
    connect(m_documentParserThread, SIGNAL(parsingQueueEmpty()),
            this, SIGNAL(documentParsingComplete()), Qt::QueuedConnection);
    connect(m_documentParserThread, SIGNAL(parsingStarted()),
            this, SIGNAL(documentParsingStarted()), Qt::QueuedConnection);
    m_documentParserThread->start();

    m_outputParserThread = new OutputParserThread(m_ki, this);
    connect(m_outputParserThread, SIGNAL(parsingComplete(QUrl,KileParser::ParserOutput*)),
            this, SLOT(handleOutputParsingComplete(QUrl,KileParser::ParserOutput*)), Qt::QueuedConnection);
    m_outputParserThread->start();
}


Manager::~Manager()
{
    qCDebug(LOG_KILE_PARSER) << "destroying...";
    m_documentParserThread->stopParsing();
    m_outputParserThread->stopParsing();
}

void Manager::parseDocument(KileDocument::TextInfo* textInfo)
{
    qCDebug(LOG_KILE_PARSER) << textInfo;
    m_documentParserThread->addDocument(textInfo);
}

void Manager::parseOutput(KileTool::Base *tool, const QString& fileName, const QString& sourceFile,
                          const QString& texFileName, int selrow, int docrow)
{
    qCDebug(LOG_KILE_PARSER) << fileName << sourceFile;
    m_outputParserThread->addLaTeXLogFile(fileName, sourceFile, texFileName, selrow, docrow);
    connect(tool, SIGNAL(aboutToBeDestroyed(KileTool::Base*)),
            this, SLOT(removeToolFromUrlHash(KileTool::Base*)), Qt::UniqueConnection);

    // using 'fileName' directly is tricky as it might contain occurrences of '//' which are filtered out
    // by QUrl (given as argument in 'handleOutputParsingComplete') and the matching won't work anymore;
    // so we use QUrl already here
    const QUrl url = QUrl::fromLocalFile(fileName);
    if(!m_urlToToolHash.contains(url, tool)) {
        m_urlToToolHash.insert(url, tool);
    }
}

bool Manager::isDocumentParsingComplete()
{
    return m_documentParserThread->isParsingComplete();
}

void Manager::stopDocumentParsing(const QUrl &url)
{
    m_documentParserThread->removeDocument(url);
}

void Manager::handleOutputParsingComplete(const QUrl &url, KileParser::ParserOutput *output)
{
    qCDebug(LOG_KILE_PARSER) << url;
    QList<KileTool::Base*> toolList = m_urlToToolHash.values(url);
    m_urlToToolHash.remove(url);

    LaTeXOutputParserOutput *latexOutput = dynamic_cast<LaTeXOutputParserOutput*>(output);
    if(!latexOutput) {
        qCDebug(LOG_KILE_PARSER) << "Q_NULLPTR output given";
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
    QMultiHash<QUrl, KileTool::Base*>::iterator i = m_urlToToolHash.begin();
    while(i != m_urlToToolHash.end()) {
        const QUrl url = i.key();
        if(i.value() == tool) {
            i = m_urlToToolHash.erase(i);
            // any more mappings for 'url' -> 'tool' left?
            if(!m_urlToToolHash.contains(url)) {
                m_outputParserThread->removeFile(url.toLocalFile());
            }
        }
        else {
            ++i;
        }
    }
}

}

