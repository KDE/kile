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

#ifndef PARSERMANAGER_H
#define PARSERMANAGER_H

#include <QList>
#include <QMultiHash>
#include <QObject>
#include <QQueue>

#include <KUrl>

class KileInfo;

namespace KileDocument {
	class Info;
	class TextInfo;
}

namespace KileTool {
	class Base;
}

namespace KileParser {

class ParserOutput;
class DocumentParserThread;
class OutputParserThread;

class Manager : public QObject
{
	Q_OBJECT

public:
	explicit Manager(KileInfo *ki, QObject *parent = 0);
	~Manager();

	void parseDocument(KileDocument::TextInfo* textInfo);

	void parseOutput(KileTool::Base *tool, const QString& fileName, const QString& sourceFile,
                                               // for QuickPreview
	                                       const QString& texFileName = "", int selrow = -1, int docrow = -1);

	bool isDocumentParsingComplete();

	void stopDocumentParsing(const KUrl& url);

Q_SIGNALS:
	void documentParsingComplete();
	void documentParsingStarted();

protected Q_SLOTS:
	void handleOutputParsingComplete(const KUrl& url, KileParser::ParserOutput *output);

	void removeToolFromUrlHash(KileTool::Base *tool);

private:
	KileInfo *m_ki;
	DocumentParserThread *m_documentParserThread;
	OutputParserThread *m_outputParserThread;
	QMultiHash<QString, KileTool::Base*> m_urlToToolHash;
};

}

#endif
