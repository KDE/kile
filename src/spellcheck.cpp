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

#include "kiledebug.h"
#include "kileviewmanager.h"

namespace KileSpellCheck {

OnTheFlyChecker::OnTheFlyChecker(QObject *parent)
: QThread(parent), m_currentlyCheckedLine(invalidSpellCheckQueueItem)
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

void OnTheFlyChecker::run()
{
	KILE_DEBUG() << "spell checking thread started";
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
	exec();
}

void OnTheFlyChecker::textAdded(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "textAdded called";
	m_spellCheckQueueMutex.lock();
	int rangeBegin = range.start().line();
	int rangeEnd = range.end().line();
	for(int i = rangeEnd; i >= rangeBegin; --i) {
		m_spellCheckQueue.push_front(SpellCheckQueueItem(document, SpellCheckLine(i, document->line(i))));
		KILE_DEBUG() << "added "
		             << SpellCheckQueueItem(document, SpellCheckLine(i, document->line(i)))
		             << " to the queue, which has a length of " << m_spellCheckQueue.size();
	}
	m_spellCheckQueueMutex.unlock();
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "textRemoved called";
	m_backgroundCheckerMutex.lock();
	m_backgroundChecker->stop();
	m_backgroundCheckerMutex.unlock();

	m_spellCheckQueueMutex.lock();
	int rangeBegin = range.start().line();
	int rangeEnd = range.end().line();
	for(QList<SpellCheckQueueItem>::iterator i = m_spellCheckQueue.begin();
	    i != m_spellCheckQueue.end(); ++i) {
		int line = (*i).second.first;
		KTextEditor::Document *lineDocument = (*i).first;
		if(document == lineDocument && rangeBegin <= line && line <= rangeEnd) {
			KILE_DEBUG() << "erasing range " << *i;
			m_spellCheckQueue.erase(i);
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
	m_spellCheckQueueMutex.unlock();
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::documentDestroyed(QObject *object)
{
	KILE_DEBUG() << "documentDestroyed called";
	m_backgroundCheckerMutex.lock();
	m_backgroundChecker->stop();
	m_backgroundCheckerMutex.unlock();

	m_spellCheckQueueMutex.lock();
	for(QList<SpellCheckQueueItem>::iterator i = m_spellCheckQueue.begin();
	    i != m_spellCheckQueue.end(); ++i) {
		KTextEditor::Document *lineDocument = (*i).first;
		if(object == lineDocument) {
			KILE_DEBUG() << "erasing range " << *i;
			m_spellCheckQueue.erase(i);
		}
	}
	if(m_currentlyCheckedLine != invalidSpellCheckQueueItem) {
		KTextEditor::Document *lineDocument = m_currentlyCheckedLine.first;
		if(object != lineDocument) {
			m_spellCheckQueue.push_front(m_currentlyCheckedLine);
		}
		else {
			m_currentlyCheckedLine = invalidSpellCheckQueueItem;
		}
	}
	m_spellCheckQueueMutex.unlock();
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

void OnTheFlyChecker::spellCheckLine()
{
	KILE_DEBUG() << "spellCheckLine called";
	SpellCheckQueueItem queueItem;
	m_spellCheckQueueMutex.lock();
	if(m_spellCheckQueue.isEmpty() || m_currentlyCheckedLine != invalidSpellCheckQueueItem) {
		m_spellCheckQueueMutex.unlock();
		return;
	}
	queueItem = m_spellCheckQueue.takeFirst();
	m_currentlyCheckedLine = queueItem;
	m_spellCheckQueueMutex.unlock();

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

	m_backgroundCheckerMutex.lock();
	QString text = queueItem.second.second;
	KILE_DEBUG() << "spell checking checking line " << text;
	m_backgroundChecker->setText(text); // don't call 'start()' after this!
	m_backgroundCheckerMutex.unlock();
}

void OnTheFlyChecker::misspelling(const QString &word, int start)
{
	SpellCheckQueueItem queueItem;
	m_spellCheckQueueMutex.lock();
	queueItem = m_currentlyCheckedLine;
	m_spellCheckQueueMutex.unlock();

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

	m_backgroundCheckerMutex.lock();
	m_backgroundChecker->continueChecking();
	m_backgroundCheckerMutex.unlock();

}

void OnTheFlyChecker::spellCheckDone()
{
	KILE_DEBUG() << "on-the-fly spell check done, queue length " << m_spellCheckQueue.size();
	m_spellCheckQueueMutex.lock();
	KTextEditor::Document *document = m_currentlyCheckedLine.first;
	KTextEditor::SmartInterface *smartInterface =
	                             qobject_cast<KTextEditor::SmartInterface*>(document);
	if(smartInterface) {
		const QList<KTextEditor::SmartRange*> highlightsList =
		                                      smartInterface->documentHighlights();
		KILE_DEBUG() << "number of smart ranges " << highlightsList.size();
	}
	m_currentlyCheckedLine = invalidSpellCheckQueueItem;
	m_spellCheckQueueMutex.unlock();
	QTimer::singleShot(0, this, SLOT(spellCheckLine()));
}

//////////////////////////////////// Manager //////////////////////////////////////////////

Manager::Manager(QObject *parent, KileView::Manager *viewManager)
: QObject(parent), m_viewManager(viewManager), m_onTheFlyChecker(NULL)
{
}

Manager::~Manager()
{
}

void Manager::onTheFlyCheckDocument(KTextEditor::Document *document)
{
	textInserted(document, document->documentRange());
}

void Manager::textChanged(KTextEditor::Document *document,
                                 const KTextEditor::Range & /* oldRange */,
                                 const KTextEditor::Range &newRange)
{
	KILE_DEBUG() << "text changed: " << document->text(newRange);

}

void Manager::textInserted(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "text inserted: " << document->text(range);
	if(m_onTheFlyChecker) {
		m_onTheFlyChecker->textAdded(document, range);
	}
}

void Manager::textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range)
{
	KILE_DEBUG() << "text removed: " << range;
	if(m_onTheFlyChecker) {
		m_onTheFlyChecker->textRemoved(document, range);
	}
}

void Manager::documentDestroyed(QObject *object)
{
	if(m_onTheFlyChecker) {
		m_onTheFlyChecker->documentDestroyed(object);
	}
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
	if(m_onTheFlyChecker) {
		return;
	}

	m_onTheFlyChecker = new OnTheFlyChecker(this);
	m_onTheFlyChecker->start();
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
	if(!m_onTheFlyChecker) {
		return;
	}
	m_onTheFlyChecker->exit();
	removeOnTheFlyHighlighting();
	delete m_onTheFlyChecker;
	m_onTheFlyChecker = NULL;
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

}

#include "spellcheck.moc"
