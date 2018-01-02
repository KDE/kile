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

#include "parserthread.h"

#include "documentinfo.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "bibtexparser.h"
#include "latexparser.h"
#include "latexoutputparser.h"

namespace KileParser {

DocumentParserInput::DocumentParserInput(const QUrl &url, QStringList lines,
        ParserType parserType,
        const QMap<QString, KileStructData>* dictStructLevel,
        bool showSectioningLabels,
        bool showStructureTodo)
    : ParserInput(url),
      lines(lines),
      parserType(parserType),
      dictStructLevel(dictStructLevel),
      showSectioningLabels(showSectioningLabels),
      showStructureTodo(showStructureTodo)
{
}

ParserThread::ParserThread(KileInfo *info, QObject *parent) :
    QThread(parent),
    m_ki(info),
    m_keepParserThreadAlive(true)
{
}

ParserThread::~ParserThread()
{
    qCDebug(LOG_KILE_PARSER) << "destroying parser thread" << this;
    stopParsing();
    // wait for the thread to finish before it is deleted at
    // the end of this destructor
    qCDebug(LOG_KILE_PARSER) << "waiting for parser thread to finish...";
    wait();
    // and delete remaining queue items (no mutex is required
    // as the thread's execution has stopped)
    qDeleteAll(m_parserQueue);
}

void ParserThread::addParserInput(ParserInput *input)
{
    qCDebug(LOG_KILE_PARSER) << input;
    qCDebug(LOG_KILE_PARSER) << "trying to obtain m_parserMutex";

    m_parserMutex.lock();
    // first, check whether the document is queued already
    QQueue<ParserInput*>::iterator it = m_parserQueue.begin();
    for(; it != m_parserQueue.end(); ++it) {
        if((*it)->url == input->url) {
            break;
        }
    }

    if(it != m_parserQueue.end()) {
        qCDebug(LOG_KILE_PARSER) << "document in queue already";
        *it = input;
    }
    else {
        if(m_currentlyParsedUrl == input->url) {
            qCDebug(LOG_KILE_PARSER) << "re-parsing document";
            // stop the parsing of the document
            m_keepParsingDocument = false;
            // and add it as first element to the queue
            m_parserQueue.push_front(input);
        }
        else {
            qCDebug(LOG_KILE_PARSER) << "adding to the end";
            m_parserQueue.push_back(input);
        }
    }
    m_parserMutex.unlock();

    // finally, wake up threads waiting for the queue to be filled
    m_queueEmptyWaitCondition.wakeAll();
}

void ParserThread::removeParserInput(const QUrl &url)
{
    qCDebug(LOG_KILE_PARSER) << url;
    m_parserMutex.lock();
    // first, if the document is currently parsed, we stop the parsing
    if(m_currentlyParsedUrl == url) {
        qCDebug(LOG_KILE_PARSER) << "document currently being parsed";
        m_keepParsingDocument = false;
    }
    // nevertheless, we remove all traces of the document from the queue
    for(QQueue<ParserInput*>::iterator it = m_parserQueue.begin(); it != m_parserQueue.end();) {
        ParserInput *input = *it;
        if(input->url == url) {
            qCDebug(LOG_KILE_PARSER) << "found it";
            it = m_parserQueue.erase(it);
            delete input;
        }
        else {
            ++it;
        }
    }
    m_parserMutex.unlock();
}

void ParserThread::stopParsing()
{
    qCDebug(LOG_KILE_PARSER);
    m_parserMutex.lock();

    m_keepParserThreadAlive = false;
    m_keepParsingDocument = false;
    m_parserMutex.unlock();
    // wake all the threads that are still waiting for the queue to fill up
    m_queueEmptyWaitCondition.wakeAll();
}

bool ParserThread::shouldContinueDocumentParsing()
{
    QMutexLocker locker(&m_parserMutex);
    return m_keepParsingDocument;
}

bool ParserThread::isParsingComplete()
{
    QMutexLocker locker(&m_parserMutex);
    // as the parser queue might be empty but a document is still being parsed,
    // we additionally have to check whether 'm_currentlyParsedUrl' is empty or not
    return m_parserQueue.isEmpty() && m_currentlyParsedUrl.isEmpty();
}

// the document that is currently parsed is always the head of the queue
void ParserThread::run()
{
    ParserInput* currentParsedItem;
    qCDebug(LOG_KILE_PARSER) << "starting up...";
    while(true) {
        // first, try to extract the head of the queue
        m_parserMutex.lock();
        // clear the variable currently parsed url; might be necessary from the previous iteration
        m_currentlyParsedUrl = QUrl();
        // check if we should still be running before going to sleep
        if(!m_keepParserThreadAlive) {
            m_parserMutex.unlock();
            // remaining queue elements are deleted in the destructor
            return;
        }
        // but if there are no items to be parsed, we go to sleep.
        // However, we have to be careful and use a 'while' loop here
        // as it can happen that an item is added to the queue but this
        // thread is woken up only after it has been removed again.
        while(m_parserQueue.size() == 0 && m_keepParserThreadAlive) {
            qCDebug(LOG_KILE_PARSER) << "going to sleep...";
            emit(parsingQueueEmpty());
            m_queueEmptyWaitCondition.wait(&m_parserMutex);
            qCDebug(LOG_KILE_PARSER) << "woken up...";
        }
        // threads are woken up when an object of this class is destroyed; in
        // that case the queue might still be empty
        if(!m_keepParserThreadAlive) {
            m_parserMutex.unlock();
            // remaining queue elements are deleted in the destructor
            return;
        }
        Q_ASSERT(m_parserQueue.size() > 0);
        qCDebug(LOG_KILE_PARSER) << "queue length" << m_parserQueue.length();
        // now, extract the head
        currentParsedItem = m_parserQueue.dequeue();

        m_keepParsingDocument = true;
        m_currentlyParsedUrl = currentParsedItem->url;
        emit(parsingStarted());
        m_parserMutex.unlock();

        Parser *parser = createParser(currentParsedItem);

        ParserOutput *parserOutput = Q_NULLPTR;
        if(parser) {
            parserOutput = parser->parse();
        }

        delete currentParsedItem;
        delete parser;

        // we also emit when 'parserOutput == Q_NULLPTR' as this will be used to indicate
        // that some error has occurred;
        // as this call will be blocking, one has to make sure that no mutex is held
        emit(parsingComplete(m_currentlyParsedUrl, parserOutput));
    }
    qCDebug(LOG_KILE_PARSER) << "leaving...";
    // remaining queue elements are deleted in the destructor
}

DocumentParserThread::DocumentParserThread(KileInfo *info, QObject *parent)
    : ParserThread(info, parent)
{
}

DocumentParserThread::~DocumentParserThread()
{
}

Parser* DocumentParserThread::createParser(ParserInput *input)
{
    if(dynamic_cast<LaTeXParserInput*>(input)) {
        return new LaTeXParser(this, dynamic_cast<LaTeXParserInput*>(input));
    }
    else if(dynamic_cast<BibTeXParserInput*>(input)) {
        return new BibTeXParser(this, dynamic_cast<BibTeXParserInput*>(input));
    }

    return Q_NULLPTR;
}

void DocumentParserThread::addDocument(KileDocument::TextInfo *textInfo)
{
    qCDebug(LOG_KILE_PARSER) << textInfo;
    const QUrl url = m_ki->docManager()->urlFor(textInfo);
    if(url.isEmpty()) { // if the url is empty (which can happen with new documents),
        return;     // we can't do anything as not even the results of the parsing can be displayed
    }

    ParserInput* newItem = Q_NULLPTR;
    if(dynamic_cast<KileDocument::BibInfo*>(textInfo)) {
        newItem = new BibTeXParserInput(url, textInfo->documentContents());
    }
    else {
        newItem = new LaTeXParserInput(url, textInfo->documentContents(),
                                       m_ki->extensions(),
                                       textInfo->dictStructLevel(),
                                       KileConfig::svShowSectioningLabels(),
                                       KileConfig::svShowTodo());
    }
    addParserInput(newItem);

    // It is not very useful to watch for the destruction of 'textInfo' here and stop the parsing
    // for 'textInfo' whenever that happens as at that moment it probably won't have a document
    // anymore nor would it still be associated with a project item.
    // It is better to call 'removeDocument' from the point when 'textInfo' is going to be deleted!
}

void DocumentParserThread::removeDocument(KileDocument::TextInfo *textInfo)
{
    qCDebug(LOG_KILE_PARSER);
    KTextEditor::Document *document = textInfo->getDoc();
    if(!document) {
        return;
    }
    removeParserInput(document->url());
}

void DocumentParserThread::removeDocument(const QUrl &url)
{
    removeParserInput(url);
}

OutputParserThread::OutputParserThread(KileInfo *info, QObject *parent)
    : ParserThread(info, parent)
{
}

OutputParserThread::~OutputParserThread()
{
}

Parser* OutputParserThread::createParser(ParserInput *input)
{
    if(dynamic_cast<LaTeXOutputParserInput*>(input)) {
        return new LaTeXOutputParser(this, dynamic_cast<LaTeXOutputParserInput*>(input));
    }
    return Q_NULLPTR;
}

void OutputParserThread::addLaTeXLogFile(const QString& logFile, const QString& sourceFile,
        const QString& texFileName, int selrow, int docrow)
{
    qCDebug(LOG_KILE_PARSER) << logFile << sourceFile;

    ParserInput* newItem = new LaTeXOutputParserInput(QUrl::fromLocalFile(logFile), m_ki->extensions(),
            sourceFile,
            texFileName, selrow, docrow);
    addParserInput(newItem);
}

void OutputParserThread::removeFile(const QString& fileName)
{
    removeParserInput(QUrl::fromLocalFile(fileName));
}

}

