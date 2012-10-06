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

DocumentParserInput::DocumentParserInput(const KUrl& url, QStringList lines,
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
	KILE_DEBUG() << "destroying parser thread" << this;
	stopParsing();
	// wait for the thread to finish before it is deleted at
	// the end of this destructor
	KILE_DEBUG() << "waiting for parser thread to finish...";
	wait();
	// and delete remaining queue items (no mutex is required
	// as the thread's execution has stopped)
	qDeleteAll(m_parserQueue);
}

void ParserThread::addParserInput(ParserInput *input)
{
	KILE_DEBUG() << input;
	KILE_DEBUG() << "trying to obtain m_parserMutex";

	m_parserMutex.lock();
	// first, check whether the document is queued already
	QQueue<ParserInput*>::iterator it = m_parserQueue.begin();
	for(; it != m_parserQueue.end(); ++it) {
		if((*it)->url == input->url) {
			break;
		}
	}

	if(it != m_parserQueue.end()) {
		KILE_DEBUG() << "document in queue already";
		*it = input;
	}
	else {
		if(m_currentlyParsedUrl == input->url) {
			KILE_DEBUG() << "re-parsing document";
			// stop the parsing of the document
			m_keepParsingDocument = false;
			// and add it as first element to the queue
			m_parserQueue.push_front(input);
		}
		else {
			KILE_DEBUG() << "adding to the end";
			m_parserQueue.push_back(input);
		}
	}
	m_parserMutex.unlock();

	// finally, wake up threads waiting for the queue to be filled
	m_queueEmptyWaitCondition.wakeAll();
}

void ParserThread::removeParserInput(const KUrl& url)
{
	KILE_DEBUG() << url;
	m_parserMutex.lock();
	// first, if the document is currently parsed, we stop the parsing
	if(m_currentlyParsedUrl == url) {
		KILE_DEBUG() << "document currently being parsed";
		m_keepParsingDocument = false;
	}
	// nevertheless, we remove all traces of the document from the queue
	for(QQueue<ParserInput*>::iterator it = m_parserQueue.begin(); it != m_parserQueue.end();) {
		ParserInput *input = *it;
		if(input->url == url) {
			KILE_DEBUG() << "found it";
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
	KILE_DEBUG();
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
	KILE_DEBUG() << "starting up...";
	while(true) {
		// first, try to extract the head of the queue
		m_parserMutex.lock();
		// clear the variable currently parsed url; might be necessary from the previous iteration
		m_currentlyParsedUrl = KUrl();
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
			KILE_DEBUG() << "going to sleep...";
			emit(parsingQueueEmpty());
			m_queueEmptyWaitCondition.wait(&m_parserMutex);
			KILE_DEBUG() << "woken up...";
		}
		// threads are woken up when an object of this class is destroyed; in
		// that case the queue might still be empty
		if(!m_keepParserThreadAlive) {
			m_parserMutex.unlock();
			// remaining queue elements are deleted in the destructor
			return;
		}
		Q_ASSERT(m_parserQueue.size() > 0);
		KILE_DEBUG() << "queue length" << m_parserQueue.length();
		// now, extract the head
		currentParsedItem = m_parserQueue.dequeue();

		m_keepParsingDocument = true;
		m_currentlyParsedUrl = currentParsedItem->url;
		emit(parsingStarted());
		m_parserMutex.unlock();

		Parser *parser = createParser(currentParsedItem);

		ParserOutput *parserOutput = NULL;
		if(parser) {
			parserOutput = parser->parse();
		}

		delete currentParsedItem;
		delete parser;

		// we also emit when 'parserOutput == NULL' as this will be used to indicate
		// that some error has occurred;
		// as this call will be blocking, one has to make sure that no mutex is held
		emit(parsingComplete(m_currentlyParsedUrl, parserOutput));
	}
	KILE_DEBUG() << "leaving...";
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

	return NULL;
}

void DocumentParserThread::addDocument(KileDocument::TextInfo *textInfo)
{
	KILE_DEBUG() << textInfo;
	const KUrl url = m_ki->docManager()->urlFor(textInfo);
	Q_ASSERT(!url.isEmpty());
	ParserInput* newItem = NULL;
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
	KILE_DEBUG();
	KTextEditor::Document *document = textInfo->getDoc();
	if(!document) {
		return;
	}
	removeParserInput(document->url());
}

void DocumentParserThread::removeDocument(const KUrl& url)
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
    return NULL;
}

void OutputParserThread::addLaTeXLogFile(const QString& logFile, const QString& sourceFile,
                                         const QString& texFileName, int selrow, int docrow)
{
	KILE_DEBUG() << logFile << sourceFile;

	ParserInput* newItem = new LaTeXOutputParserInput(KUrl::fromPath(logFile), m_ki->extensions(),
	                                                                           sourceFile,
	                                                                           texFileName, selrow, docrow);
	addParserInput(newItem);
}

void OutputParserThread::removeFile(const QString& fileName)
{
	removeParserInput(KUrl::fromPath(fileName));
}

}

#include "parserthread.moc"
