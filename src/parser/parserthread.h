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

#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include <QMutex>
#include <QPair>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>

#include <KUrl>

#include "documentinfo.h"

class KileInfo;

namespace KileDocument {
	class Info;
	class TextInfo;
}

namespace KileParser {

class Parser;
class ParserOutput;

enum ParserType { LaTeX = 0, BibTeX };

// NOTE: we cannot store pointer to TextInfo objects in the queue
//       as this would cause too many problems when they are deleted
//       and their content is still being parsed
class ParserQueueItem
{
public:
	ParserQueueItem();
	ParserQueueItem(const KUrl& url, QStringList lines,
	                                 ParserType parserType,
	                                 const QMap<QString, KileStructData>* dictStructLevel,
	                                 bool showSectioningLabels,
	                                 bool showStructureTodo);

	KUrl url;
	QStringList lines;
	ParserType parserType;
	const QMap<QString, KileStructData>* dictStructLevel;
	bool showSectioningLabels;
	bool showStructureTodo;
};

class ParserThread : public QThread
{
	Q_OBJECT

public:
	explicit ParserThread(KileInfo *info, QObject *parent = 0);
	~ParserThread();

	void stopParsing();

	bool shouldContinueDocumentParsing();

	bool isParsingComplete();

Q_SIGNALS:
	/**
	 * The ownership of the 'output' object is tranferred to the slot(s)
	 * connected to this signal.
	 **/
	void parsingComplete(const KUrl& url, KileParser::ParserOutput* output);

	void parsingQueueEmpty();
	void parsingStarted();

public Q_SLOTS:
	void addDocument(KileDocument::TextInfo *textInfo);
	void removeDocument(KileDocument::TextInfo *textInfo);

protected:
	void run();

private Q_SLOTS:
	void handleDocumentClosed(KTextEditor::Document *doc);

private:
	KileInfo *m_ki;
	bool m_keepParserThreadAlive;
	bool m_keepParsingDocument;
	QQueue<ParserQueueItem> m_parserQueue;
	KUrl m_currentlyParsedUrl;
	QMutex m_parserMutex;
	QWaitCondition m_queueEmptyWaitCondition;

	void removeParsingForURL(const KUrl& url);
};

}

#endif
