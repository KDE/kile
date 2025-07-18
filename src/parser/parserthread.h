/**************************************************************************
*   Copyright (C) 2011-2019 by Michel Ludwig (michel.ludwig@kdemail.net)  *
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
#include <QThread>
#include <QQueue>
#include <QWaitCondition>

#include <QUrl>

#include "documentinfo.h"

#include "parser.h"

class KileInfo;

namespace KileDocument {
class Info;
class TextInfo;
}

namespace KileParser {

class Parser;
class ParserInput;
class ParserOutput;

enum ParserType { LaTeX = 0, BibTeX };

class ParserThread : public QThread
{
    Q_OBJECT

public:
    explicit ParserThread(KileInfo *info, QObject *parent = nullptr);
    virtual ~ParserThread();

    void stopParsing();

    bool shouldContinueDocumentParsing();

    bool isParsingComplete();

Q_SIGNALS:
    /**
     * The ownership of the 'output' object is transferred to the slot(s)
     * connected to this signal.
     **/
    void parsingComplete(const QUrl &url, KileParser::ParserOutput* output);

    void parsingQueueEmpty();
    void parsingStarted();

protected:
    KileInfo *m_ki;

    void addParserInput(ParserInput *input);
    void removeParserInput(const QUrl &url);

    void run() override;

    virtual Parser* createParser(ParserInput *input) = 0;

private:
    bool m_keepParserThreadAlive;
    bool m_keepParsingDocument;
    QQueue<ParserInput*> m_parserQueue;
    QUrl m_currentlyParsedUrl;
    QMutex m_parserMutex;
    QWaitCondition m_queueEmptyWaitCondition;
};


class DocumentParserThread : public ParserThread
{
    Q_OBJECT

public:
    explicit DocumentParserThread(KileInfo *info, QObject *parent = nullptr);
    virtual ~DocumentParserThread() override;

public Q_SLOTS:
    void addDocument(KileDocument::TextInfo *textInfo);
    void removeDocument(KileDocument::TextInfo *textInfo);
    void removeDocument(const QUrl &url);

protected:
    virtual Parser* createParser(ParserInput *input) override;

};


class OutputParserThread: public ParserThread
{
    Q_OBJECT

public:
    explicit OutputParserThread(KileInfo *info, QObject *parent = nullptr);
    virtual ~OutputParserThread() override;

public Q_SLOTS:
    void addLaTeXLogFile(const QString& logFile, const QString& sourceFile,
                         // for QuickPreview
                         const QString& texFileName = QString(), int selrow = -1, int docrow = -1);
    void removeFile(const QString& fileName);

protected:
    virtual Parser* createParser(ParserInput *input) override;
};

}

#endif
