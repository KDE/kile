/**************************************************************************
*   Copyright (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)       *
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
#include "kileinfo.h"
#include "bibtexparser.h"
#include "latexparser.h"

namespace KileParser {

ParserQueueItem::ParserQueueItem()
: dictStructLevel(NULL)
{
}

ParserQueueItem::ParserQueueItem(const KUrl& url, QStringList lines,
                                                  ParserType parserType,
                                                  const QMap<QString, KileStructData>* dictStructLevel,
                                                  bool showSectioningLabels,
                                                  bool showStructureTodo)
: url(url),
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
	KILE_DEBUG() << "destroying parser thread" << this;
	stopParsing();
	// wait for the thread to finish before it is deleted at
	// the end of this destructor
	KILE_DEBUG() << "waiting for parser thread to finish...";
	wait();
}

void ParserThread::addDocument(KileDocument::TextInfo *textInfo)
{
	KILE_DEBUG() << textInfo;
	KTextEditor::Document *document = textInfo->getDoc();
	const KUrl& url = document->url();
	if(!document) {
		KILE_DEBUG() << "KileDocument::TextInfo without document given!";
		return;
	}
	KILE_DEBUG() << "trying to obtain m_parserMutex";

	m_parserMutex.lock();
	// first, check whether the document is queued already
	QQueue<ParserQueueItem>::iterator it = m_parserQueue.begin();
	for(; it != m_parserQueue.end(); ++it) {
		if(it->url == url) {
			break;
		}
	}
	ParserType parserType = (dynamic_cast<KileDocument::BibInfo*>(textInfo) ? BibTeX : LaTeX);
	ParserQueueItem newItem(url, document->textLines(document->documentRange()),
	                             parserType,
	                             &(textInfo->dictStructLevel()),
	                             KileConfig::svShowSectioningLabels(),
	                             KileConfig::svShowTodo());

	if(it != m_parserQueue.end()) {
		KILE_DEBUG() << "document in queue already";
		*it = newItem;
	}
	else {
		if(m_currentlyParsedUrl == url) {
			KILE_DEBUG() << "re-parsing document";
			// stop the parsing of the document
			m_keepParsingDocument = false;
			// and add it as first element to the queue
			m_parserQueue.push_front(newItem);
		}
		else {
			KILE_DEBUG() << "adding to the end";
			m_parserQueue.push_back(newItem);
		}
		connect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), this, SLOT(handleDocumentClosed(KTextEditor::Document*)));
	}
	m_parserMutex.unlock();

	// finally, wake up threads waiting for the queue to be filled
	m_queueEmptyWaitCondition.wakeAll();
}

void ParserThread::removeDocument(KileDocument::TextInfo *textInfo)
{
	KILE_DEBUG();
	KTextEditor::Document *document = textInfo->getDoc();
	if(!document) {
		KILE_DEBUG() << "KileDocument::TextInfo without document given!";
		return;
	}
	removeParsingForURL(document->url());
}

void ParserThread::removeParsingForURL(const KUrl& url)
{
	KILE_DEBUG() << url;
	m_parserMutex.lock();
	// first, if the document is currently parsed, we stop the parsing
	if(m_currentlyParsedUrl == url) {
		KILE_DEBUG() << "document currently being parsed";
		m_keepParsingDocument = false;
	}
	// nevertheless, we remove all traces of the document from the queue
	for(QQueue<ParserQueueItem>::iterator it = m_parserQueue.begin(); it != m_parserQueue.end();) {
		if(it->url == url) {
			KILE_DEBUG() << "found it";
			it = m_parserQueue.erase(it);
		}
		else {
			++it;
		}
	}
	m_parserMutex.unlock();
}

void ParserThread::handleDocumentClosed(KTextEditor::Document *document)
{
	KILE_DEBUG();
	disconnect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), this, SLOT(handleDocumentClosed(KTextEditor::Document*)));
	removeParsingForURL(document->url());
}

void ParserThread::stopParsing()
{
	m_parserMutex.lock();
KILE_DEBUG();
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
	return m_parserQueue.isEmpty();
}

// the document that is currently parsed is always the head of the queue
void ParserThread::run()
{
	ParserQueueItem currentParsedItem;
	KILE_DEBUG() << "starting up...";
	while(true) {
		// first, try to extract the head of the queue
		m_parserMutex.lock();
		// clear the variable currently parsed url; might be necessary from the previous iteration
		m_currentlyParsedUrl = KUrl();
		// check if we should still be running before going to sleep
		if(!m_keepParserThreadAlive) {
			m_parserMutex.unlock();
			return;
		}
		// but if there are no items to be parsed, we go to sleep
		if(m_parserQueue.size() == 0) {
			KILE_DEBUG() << "going to sleep...";
			emit(parsingQueueEmpty());
			m_queueEmptyWaitCondition.wait(&m_parserMutex);
			KILE_DEBUG() << "woken up...";
		}
		// threads are woken up when an object of this class is destroyed; in
		// that case the queue might still be empty
		if(!m_keepParserThreadAlive) {
			m_parserMutex.unlock();
			return;
		}
		Q_ASSERT(m_parserQueue.size() > 0);
		KILE_DEBUG() << "queue length" << m_parserQueue.length();
		// now, extract the head
		currentParsedItem = m_parserQueue.dequeue();

		m_keepParsingDocument = true;
		m_currentlyParsedUrl = currentParsedItem.url;
		emit(parsingStarted());
		m_parserMutex.unlock();

		Parser *parser = NULL;
		if(currentParsedItem.parserType == LaTeX) {
			parser = new LaTeXParser(this, m_ki->extensions(), *(currentParsedItem.dictStructLevel),
			                                                   currentParsedItem.showSectioningLabels,
			                                                   currentParsedItem.showStructureTodo);
		}
		else if(currentParsedItem.parserType == BibTeX) {
			parser = new BibTeXParser(this);
		}

		if(parser) {
			ParserOutput *parserOutput = parser->parse(currentParsedItem.lines);
			delete parser;

			if(parserOutput) {
				emit(parsingComplete(currentParsedItem.url, parserOutput));
			}
		}
	}
}

}

#include "parserthread.moc"
