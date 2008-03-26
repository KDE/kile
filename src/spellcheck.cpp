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

#include "spellcheck.h"

#include <QHash>
#include <QTimer>

#include <KTextEditor/SmartInterface>
#include <KTextEditor/SmartRange>
#include <KTextEditor/View>
#include <sonnet/speller.h>

#include "documentinfo.h"
#include "kiledebug.h"
#include "kileviewmanager.h"

namespace KileSpellCheck {

OnTheFlyChecker::OnTheFlyChecker(QObject *parent)
: QObject(parent), m_currentlyCheckedLine(invalidSpellCheckQueueItem)
{
	KILE_DEBUG() << "created";
	m_stop = false;

	QTimer::singleShot(0, this, SLOT(spellCheckLine()));

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

void OnTheFlyChecker::stop()
{
	m_stop = true;
}

void OnTheFlyChecker::textInserted(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(m_stop) {
		KILE_DEBUG() << "exited as we have to stop";
		return;
	}
	int rangeBegin = range.start().line();
	int rangeEnd = range.end().line();
	for(int i = rangeEnd; i >= rangeBegin; --i) {
		m_spellCheckQueue.push_front(SpellCheckQueueItem(document, SpellCheckLine(i, document->line(i))));
		KILE_DEBUG() << "added "
		             << SpellCheckQueueItem(document, SpellCheckLine(i, document->line(i)))
		             << " to the queue, which has a length of " << m_spellCheckQueue.size();
	}
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(m_stop) {
		KILE_DEBUG() << "exited as we have to stop";
		return;
	}
	if(m_currentlyCheckedLine != invalidSpellCheckQueueItem) { // check in progress
		KILE_DEBUG() << "sleeping";
		KILE_DEBUG() << "continuing";
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
			m_currentlyCheckedLine =
			   SpellCheckQueueItem(document, SpellCheckLine(line, document->line(line)));
			m_spellCheckQueue.push_front(m_currentlyCheckedLine);
			KILE_DEBUG() << "added the line " << line;
		}
		m_currentlyCheckedLine = invalidSpellCheckQueueItem;
	}
	for(int i = rangeEnd; i >= rangeBegin; --i) {
		if(i < document->lines()) {
			m_spellCheckQueue.push_front(SpellCheckQueueItem(document,
			                             SpellCheckLine(i, document->line(i))));
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
	if(m_stop) {
		KILE_DEBUG() << "exited as we have to stop";
		return;
	}
	SpellCheckQueueItem queueItem;
	if(m_currentlyCheckedLine != invalidSpellCheckQueueItem) {
		KILE_DEBUG() << "exited as a check is currenly in progress";
		return;
	}
	if(m_spellCheckQueue.isEmpty()) {
		KILE_DEBUG() << "exited as there is nothing to do";
		return;
	}
	queueItem = m_spellCheckQueue.takeFirst();
	m_currentlyCheckedLine = queueItem;

	KTextEditor::Document *document = queueItem.first;
	int line = queueItem.second.first;
	KILE_DEBUG() << "for the line " << line;
	// clear all the highlights that are currently present on the line that
	// is supposed to be checked
	KTextEditor::SmartInterface *smartInterface =
	                             qobject_cast<KTextEditor::SmartInterface*>(document);
	if(smartInterface) {
		smartInterface->smartMutex()->lock();
		const QList<KTextEditor::SmartRange*> highlightsList =
		                                      smartInterface->documentHighlights();
		for(QList<KTextEditor::SmartRange*>::const_iterator i = highlightsList.begin();
		    i != highlightsList.end(); ++i) {
			if((*i)->start().line() <= line && line <= (*i)->end().line()) {
				delete (*i);
			}
		}
		smartInterface->smartMutex()->unlock();
	}

	QString text = queueItem.second.second;
	KILE_DEBUG() << "next spell checking line " << text;
	m_backgroundChecker->setText(text); // don't call 'start()' after this!
	KILE_DEBUG() << "exited";
}

void OnTheFlyChecker::misspelling(const QString &word, int start)
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	if(m_stop) {
		KILE_DEBUG() << "exited as we have to stop";
		return;
	}
	if(m_currentlyCheckedLine == invalidSpellCheckQueueItem) {
		KILE_DEBUG() << "exited as no spell check is taking place";
		return;
	}
	SpellCheckQueueItem queueItem;
	queueItem = m_currentlyCheckedLine;
	KILE_DEBUG() << "misspelled " << word
	                              << " at line "
	                              << queueItem.second.first << " column " << start;

	KTextEditor::Document *document = queueItem.first;
	int line = queueItem.second.first;

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
		attribute->setFontBold(true);
// 		attribute->setOutline(QBrush(QColor(Qt::red)));
		smartRange->setAttribute(KTextEditor::Attribute::Ptr(attribute));
		smartInterface->addHighlightToDocument(smartRange);
		smartInterface->smartMutex()->unlock();
	}

	if(m_stop) {
		return;
	}
	m_backgroundChecker->continueChecking();
	KILE_DEBUG() << "exited";
}

void OnTheFlyChecker::spellCheckDone()
{
	KILE_DEBUG() << "entered in thread " << QThread::currentThreadId();
	KILE_DEBUG() << "on-the-fly spell check done, queue length " << m_spellCheckQueue.size();
	if(m_stop) {
		KILE_DEBUG() << "exited as we have to stop";
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
		const QList<KTextEditor::SmartRange*> highlightsList =
		                                      smartInterface->documentHighlights();
		KILE_DEBUG() << "number of smart ranges " << highlightsList.size();
	}
	m_currentlyCheckedLine = invalidSpellCheckQueueItem;
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
	KILE_DEBUG() << "exited";
}

//////////////////////////////////// Manager //////////////////////////////////////////////

Manager::Manager(QObject *parent, KileView::Manager *viewManager)
: QThread(parent), m_viewManager(viewManager)
{
	// necessary for connecting signals acress Threads
	qRegisterMetaType<KTextEditor::Document*>("KTextEditor::Document*");
	qRegisterMetaType<KTextEditor::Range>("KTextEditor::Range");

	connect(this, SIGNAL(onTheFlyCheckerSetup()),
	        this, SLOT(onTheFlyCheckOpenDocuments()), Qt::BlockingQueuedConnection);
}

Manager::~Manager()
{
	stopOnTheFlySpellCheckThread();
}

void Manager::onTheFlyCheckDocument(KTextEditor::Document *document)
{
	emit(textInserted(document, document->documentRange()));
}

void Manager::addOnTheFlySpellChecking(KileDocument::TextInfo *info)
{
	connect(info->getDoc(), SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                this, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)));
	connect(info->getDoc(), SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)),
                this, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)));
}

void Manager::removeOnTheFlySpellChecking(KTextEditor::Document *doc)
{
	disconnect(doc, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                this, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)));
	disconnect(doc, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)),
                this, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)));
	emit(freeDocument(doc));
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
	if(isRunning()) {
		return;
	}
	m_onTheFlyCheckerMutex.lock();
	m_onTheFlyCheckerSetup = false;
	m_onTheFlyCheckerMutex.unlock();
	KILE_DEBUG() << "starting spell check thread from thread " << QThread::currentThreadId();
	start();
	// wait for all the signal connections to be made before calling 'onTheFlyCheckOpenDocuments()'
	m_onTheFlyCheckerMutex.lock();
	while(!m_onTheFlyCheckerSetup) {
		m_onTheFlyCheckerSetupWaitCondition.wait(&m_onTheFlyCheckerMutex, 200);
	}
	m_onTheFlyCheckerMutex.unlock();
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
	if(!isRunning()) {
		return;
	}
	emit(stop());
	quit();
	wait();
	removeOnTheFlyHighlighting();
}

void Manager::removeOnTheFlyHighlighting()
{
	const QList<KTextEditor::View*> textViews = m_viewManager->textViews();
	for(QList<KTextEditor::View*>::const_iterator i = textViews.begin();
	    i != textViews.end(); ++i) {
		KTextEditor::Document *document = (*i)->document();
		KTextEditor::SmartInterface *smartInterface =
		             qobject_cast<KTextEditor::SmartInterface*>(document);
		if(smartInterface) {
			smartInterface->smartMutex()->lock();
			const QList<KTextEditor::SmartRange*> highlightsList =
			     smartInterface->documentHighlights();
			for(QList<KTextEditor::SmartRange*>::const_iterator i = highlightsList.begin();
			    i != highlightsList.end(); ++i) {
				delete (*i);
			}
			smartInterface->smartMutex()->unlock();
		}
	}
}

void Manager::run()
{
	KILE_DEBUG() << "spell checking thread started, id " << QThread::currentThreadId()
	                                                     << " " << QThread::currentThread();
	OnTheFlyChecker *onTheFlyChecker = new OnTheFlyChecker();

	// 'BlockingQueuedConnection' is important as we want the GUI thread has to wait until the document
	// has been removed from the spell check queue. Moreover, without blocking it might happen that
	// 'freeDocument' is called although there are still e.g. some 'textInsert' calls left
	connect(this, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                onTheFlyChecker, SLOT(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)),
                onTheFlyChecker, SLOT(textRemoved(KTextEditor::Document*, const KTextEditor::Range&)),
                Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(freeDocument(KTextEditor::Document*)),
                onTheFlyChecker, SLOT(freeDocument(KTextEditor::Document*)),
                Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(stop()), onTheFlyChecker, SLOT(stop()), Qt::BlockingQueuedConnection);
	m_onTheFlyCheckerMutex.lock();
	m_onTheFlyCheckerSetup = true;
	m_onTheFlyCheckerSetupWaitCondition.wakeAll();
	m_onTheFlyCheckerMutex.unlock();

	exec();
	KILE_DEBUG() << "spell check thread stopped";
	delete onTheFlyChecker;
}

}

#include "spellcheck.moc"
