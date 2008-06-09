/**************************************************************************
*   Copyright (C) 2008 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/* If ever threads should be used again, thread communication and
 * synchronization ought to be done with blocking queued signal connections.
 */

#include "spellcheck.h"

#include <QMutex>
#include <QHash>
#include <QTimer>
#include <QThread>

#include <KTextEditor/SmartInterface>
#include <KTextEditor/SmartRange>
#include <KTextEditor/View>
#include <sonnet/speller.h>

#include "documentinfo.h"
#include "kiledebug.h"
#include "kileviewmanager.h"

namespace KileSpellCheck {

OnTheFlyChecker::OnTheFlyChecker(QObject *parent)
: QObject(parent), m_currentlyCheckedLine(invalidSpellCheckQueueItem), m_enabled(false)
{
	KILE_DEBUG() << "created";

	m_backgroundChecker = new Sonnet::BackgroundChecker(Sonnet::Speller(), this);
	connect(m_backgroundChecker,
	        SIGNAL(misspelling(const QString&,int)),
	        this,
	        SLOT(misspelling(const QString&,int)));
	connect(m_backgroundChecker, SIGNAL(done()), this, SLOT(spellCheckDone()));
}

OnTheFlyChecker::~OnTheFlyChecker()
{
	KILE_DEBUG() << "destroyed";
}

const OnTheFlyChecker::SpellCheckQueueItem OnTheFlyChecker::invalidSpellCheckQueueItem =
                       SpellCheckQueueItem(NULL, SpellCheckLine(-1, QString()));

void OnTheFlyChecker::setEnabled(bool b)
{
	m_enabled = b;
	if(b) {
		QTimer::singleShot(0, this, SLOT(spellCheckLine()));
	}
	else {
		m_currentlyCheckedLine = invalidSpellCheckQueueItem;
		m_spellCheckQueue.clear();
	}
}

void OnTheFlyChecker::textInserted(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(!m_enabled) {
		KILE_DEBUG() << "leaving as on-the-fly checking is disabled "
		             << QThread::currentThreadId();
		return;
	}
	int rangeBegin = range.start().line();
	int rangeEnd = range.end().line();
	for(int i = rangeEnd; i >= rangeBegin; --i) {
		QString text = document->line(i);
		if(!text.isEmpty()) {
			m_spellCheckQueue.push_front(SpellCheckQueueItem(document, SpellCheckLine(i, text)));
		}
		KILE_DEBUG() << "added "
		             << SpellCheckQueueItem(document, SpellCheckLine(i, document->line(i)))
		             << " to the queue, which has a length of " << m_spellCheckQueue.size();
	}
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(!m_enabled) {
		KILE_DEBUG() << "leaving as on-the-fly checking is disabled "
		             << QThread::currentThreadId();
		return;
	}
	int rangeBegin = range.start().line();
	int rangeEnd = range.end().line();
	for(QList<SpellCheckQueueItem>::iterator i = m_spellCheckQueue.begin();
	    i != m_spellCheckQueue.end();) {
		int line = (*i).second.first;
		KTextEditor::Document *lineDocument = (*i).first;
		if(document == lineDocument && rangeBegin <= line && line <= rangeEnd) {
			KILE_DEBUG() << "erasing range " << *i;
			i = m_spellCheckQueue.erase(i);
		}
		else {
			++i;
		}
	}
	if(m_currentlyCheckedLine != invalidSpellCheckQueueItem) {
		int line = m_currentlyCheckedLine.second.first;
		KTextEditor::Document *lineDocument = m_currentlyCheckedLine.first;
		if(document != lineDocument) {
			m_spellCheckQueue.push_front(m_currentlyCheckedLine);
		}
		if(line < document->lines() && !(rangeBegin <= line && line <= rangeEnd)) {
			QString text = document->line(line);
			if(!text.isEmpty()) {
				m_spellCheckQueue.push_front(SpellCheckQueueItem(document,
				                             SpellCheckLine(line, text)));
			}
			KILE_DEBUG() << "added the line " << line;
		}
		m_currentlyCheckedLine = invalidSpellCheckQueueItem;
	}
	for(int i = rangeEnd; i >= rangeBegin; --i) {
		if(i < document->lines()) {
			QString text = document->line(i);
			if(!text.isEmpty()) {
				m_spellCheckQueue.push_front(SpellCheckQueueItem(document,
				                             SpellCheckLine(i, text)));
			}
		}
		KILE_DEBUG() << "added "
		             << SpellCheckQueueItem(document, SpellCheckLine(i, document->line(i)))
		             << " to the queue, which has a length of " << m_spellCheckQueue.size();
	}
	KILE_DEBUG() << "exited";
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::freeDocument(KTextEditor::Document *document)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();

	for(QList<SpellCheckQueueItem>::iterator i = m_spellCheckQueue.begin();
	    i != m_spellCheckQueue.end();) {
		KTextEditor::Document *lineDocument = (*i).first;
		if(document == lineDocument) {
			KILE_DEBUG() << "erasing range " << *i;
			i = m_spellCheckQueue.erase(i);
		}
		else {
			++i;
		}
	}
	if(m_currentlyCheckedLine != invalidSpellCheckQueueItem) {
		KTextEditor::Document *lineDocument = m_currentlyCheckedLine.first;
		if(document != lineDocument) {
			m_spellCheckQueue.push_front(m_currentlyCheckedLine);
		}
	}
	m_currentlyCheckedLine = invalidSpellCheckQueueItem;
	KILE_DEBUG() << "exited";
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::spellCheckLine()
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(!m_enabled) {
		KILE_DEBUG() << "leaving as on-the-fly checking is disabled "
		             << QThread::currentThreadId();
		return;
	}
	if(m_currentlyCheckedLine != invalidSpellCheckQueueItem) {
		KILE_DEBUG() << "exited as a check is currenly in progress";
		return;
	}
	if(m_spellCheckQueue.isEmpty()) {
		KILE_DEBUG() << "exited as there is nothing to do";
		return;
	}
	m_currentlyCheckedLine = m_spellCheckQueue.takeFirst();

	KTextEditor::Document *document = m_currentlyCheckedLine.first;
	int line = m_currentlyCheckedLine.second.first;
	KILE_DEBUG() << "for the line " << line;
	// clear all the highlights that are currently present on the line that
	// is supposed to be checked
	KTextEditor::SmartInterface *smartInterface =
	                             qobject_cast<KTextEditor::SmartInterface*>(document);
	if(smartInterface) {
		smartInterface->smartMutex()->lock();
		const QList<KTextEditor::SmartRange*> highlightsList =
		                                      smartInterface->documentHighlights();
		QList<KTextEditor::SmartRange*> topRanges;
		for(QList<KTextEditor::SmartRange*>::const_iterator i = highlightsList.begin();
		    i != highlightsList.end(); ++i) {
			if((*i)->depth() == 0) {
				topRanges.push_back(*i);
			}
		}
		for(QList<KTextEditor::SmartRange*>::iterator i = topRanges.begin();
		    i != topRanges.end(); ++i) {
			if((*i)->start().line() <= line && line <= (*i)->end().line()) {
// 				smartInterface->removeHighlightFromDocument(*i);
				delete(*i);
			}
		}
		smartInterface->smartMutex()->unlock();
	}

	QString text = m_currentlyCheckedLine.second.second;
	KILE_DEBUG() << "next spell checking line " << text;
	m_backgroundChecker->setText(text); // don't call 'start()' after this!
	KILE_DEBUG() << "exited";
}

void OnTheFlyChecker::misspelling(const QString &word, int start)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(!m_enabled) {
		KILE_DEBUG() << "leaving as on-the-fly checking is disabled "
		             << QThread::currentThreadId();
		return;
	}
	if(m_currentlyCheckedLine == invalidSpellCheckQueueItem) {
		KILE_DEBUG() << "exited as no spell check is taking place";
		return;
	}
	KILE_DEBUG() << "misspelled " << word
	                              << " at line "
	                              << m_currentlyCheckedLine.second.first
	                              << " column " << start;

	KTextEditor::Document *document = m_currentlyCheckedLine.first;
	int line = m_currentlyCheckedLine.second.first;

	KTextEditor::SmartInterface *smartInterface =
	                            qobject_cast<KTextEditor::SmartInterface*>(document);
	if(smartInterface) {
		smartInterface->smartMutex()->lock();
		KTextEditor::SmartRange *smartRange =
		                         smartInterface->newSmartRange(KTextEditor::Range(line,
		                                                                          start,
		                                                                          line,
		                                                                          start + word.length()));
		KTextEditor::Attribute *attribute = new KTextEditor::Attribute();
		attribute->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
		attribute->setUnderlineColor(QColor(Qt::red));
		smartRange->setAttribute(KTextEditor::Attribute::Ptr(attribute));
		smartInterface->addHighlightToDocument(smartRange);
		smartInterface->smartMutex()->unlock();
	}

	m_backgroundChecker->continueChecking();
	KILE_DEBUG() << "exited";
}

void OnTheFlyChecker::spellCheckDone()
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	KILE_DEBUG() << "on-the-fly spell check done, queue length " << m_spellCheckQueue.size();
	if(!m_enabled) {
		KILE_DEBUG() << "leaving as on-the-fly checking is disabled "
		             << QThread::currentThreadId();
		return;
	}
	if(m_currentlyCheckedLine == invalidSpellCheckQueueItem) {
		KILE_DEBUG() << "exited as no spell check is taking place";
		return;
	}
	KTextEditor::Document *document = m_currentlyCheckedLine.first;
	KTextEditor::SmartInterface *smartInterface =
	                             qobject_cast<KTextEditor::SmartInterface*>(document);
	if(smartInterface) {
		smartInterface->smartMutex()->lock();
		const QList<KTextEditor::SmartRange*> highlightsList =
		                                      smartInterface->documentHighlights();
		KILE_DEBUG() << "number of smart ranges " << highlightsList.size();
		smartInterface->smartMutex()->unlock();
	}
	m_currentlyCheckedLine = invalidSpellCheckQueueItem;
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
	KILE_DEBUG() << "exited";
}

//////////////////////////////////// Manager //////////////////////////////////////////////

Manager::Manager(QObject *parent, KileView::Manager *viewManager)
: QObject(parent), m_viewManager(viewManager)
{
	m_onTheFlyChecker = new OnTheFlyChecker();
}

Manager::~Manager()
{
	stopOnTheFlySpellCheckThread();
	delete m_onTheFlyChecker;
}

void Manager::onTheFlyCheckDocument(KTextEditor::Document *document)
{
	m_onTheFlyChecker->textInserted(document, document->documentRange());
}

void Manager::addOnTheFlySpellChecking(KTextEditor::Document *doc)
{
	connect(doc, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                m_onTheFlyChecker, SLOT(textInserted(KTextEditor::Document*, const KTextEditor::Range&)));
	connect(doc, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)),
                m_onTheFlyChecker, SLOT(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)));
}

void Manager::removeOnTheFlySpellChecking(KTextEditor::Document *doc)
{
	disconnect(doc, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                m_onTheFlyChecker, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)));
	disconnect(doc, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)),
                m_onTheFlyChecker, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)));
	m_onTheFlyChecker->freeDocument(doc);
}

void Manager::setOnTheFlySpellCheckEnabled(bool b)
{
	if(b) {
		startOnTheFlySpellCheckThread();
	}
	else {
		stopOnTheFlySpellCheckThread();
	}
}

void Manager::startOnTheFlySpellCheckThread()
{
	KILE_DEBUG() << "starting spell check thread from thread " << QThread::currentThreadId();
	m_onTheFlyChecker->setEnabled(true);
	onTheFlyCheckOpenDocuments();
}

void Manager::onTheFlyCheckOpenDocuments()
{
	QHash<KTextEditor::Document*, bool> documentHash;
	const QList<KTextEditor::View*> textViews = m_viewManager->textViews();
	for(QList<KTextEditor::View*>::const_iterator i = textViews.begin();
	    i != textViews.end(); ++i) {
		KTextEditor::Document *document = (*i)->document();
		if(!documentHash.contains(document)) {
			onTheFlyCheckDocument(document);
			documentHash.insert(document, true);
		}
	}
}

void Manager::stopOnTheFlySpellCheckThread()
{
	m_onTheFlyChecker->setEnabled(false);
	removeOnTheFlyHighlighting();
}

void Manager::removeOnTheFlyHighlighting()
{
	const QList<KTextEditor::View*> textViews = m_viewManager->textViews();
	foreach ( const KTextEditor::View *view, textViews ) {
		if (!view) {
			continue;
		}
		KTextEditor::Document *document = view->document();
		KTextEditor::SmartInterface *smartInterface =
		             qobject_cast<KTextEditor::SmartInterface*>(document);
		if(smartInterface) {
			smartInterface->smartMutex()->lock();
// 			smartInterface->clearDocumentHighlights();
			const QList<KTextEditor::SmartRange*> highlightsList =
			                                      smartInterface->documentHighlights();
			for(QList<KTextEditor::SmartRange*>::const_iterator j = highlightsList.begin();
			j != highlightsList.end(); ++j) {
				delete(*j);
			}
			smartInterface->smartMutex()->unlock();
		}
	}
}

}

#include "spellcheck.moc"
