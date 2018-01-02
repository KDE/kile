/**********************************************************************************
*   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)           *
*                 2011 by Michel Ludwig (michel.ludwig@kdemail.net)               *
***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LATEXOUTPUTPARSER_H
#define LATEXOUTPUTPARSER_H

#include <QLinkedList>
#include <QStack>

#include "kileconstants.h"
#include "kileextensions.h"
#include "outputinfo.h"
#include "parser.h"

namespace KileParser {

class LaTeXOutputParserInput : public ParserInput
{
public:
    LaTeXOutputParserInput(const QUrl &url, KileDocument::Extensions *extensions,
                           const QString& sourceFile,
                           // for QuickPreview
                           const QString &texfilename = "", int selrow = -1, int docrow = -1);

    KileDocument::Extensions *extensions;
    QString sourceFile;
    QString texfilename;
    int selrow;
    int docrow;
};

class LaTeXOutputParserOutput : public ParserOutput {
public:
    LaTeXOutputParserOutput();
    virtual ~LaTeXOutputParserOutput();

    QString problem;
    QString logFile;
    LatexOutputInfoArray infoList;
    int nWarnings;
    int nErrors;
    int nBadBoxes;
};

class LOFStackItem
{
public:
    explicit LOFStackItem(const QString& file = QString(), bool sure = false) : m_file(file), m_reliable(sure) {}

    const QString& file() const {
        return m_file;
    }
    void setFile(const QString & file) {
        m_file = file;
    }

    bool reliable() const {
        return m_reliable;
    }
    void setReliable(bool sure) {
        m_reliable = sure;
    }

private:
    QString m_file;
    bool m_reliable;
};

class LaTeXOutputParser : public Parser
{
    Q_OBJECT

public:
    LaTeXOutputParser(ParserThread *parserThread, LaTeXOutputParserInput *input, QObject *parent = Q_NULLPTR);
    virtual ~LaTeXOutputParser();

    ParserOutput* parse();

    void updateInfoLists(const QString &texfilename, int selrow, int docrow);

    enum {Start = 0, FileName, FileNameHeuristic, Error, Warning, BadBox, LineNumber};

    const QString& log() const {
        return m_log;
    }

    const QString& source() const  {
        return m_source;
    }
    const QString& path() const {
        return m_srcPath;
    }

protected:
    /**
    Parses the given line for the start of new files or the end of
    old files.
    */
    void updateFileStack(const QString &strLine, short & dwCookie);
    void updateFileStackHeuristic(const QString &strLine, short & dwCookie);

    /**
    Forwards the currently parsed item to the item list.
    */
    void flushCurrentItem();

public:
    /** Return number of errors etc. found in log-file. */
    void getErrorCount(int *errors, int *warnings, int *badboxes);
    void clearErrorCount() {
        m_nErrors=m_nWarnings=m_nBadBoxes=0 ;
    }

protected:
    virtual short parseLine(const QString & strLine, short dwCookie);

    bool detectError(const QString & strLine, short &dwCookie);
    bool detectWarning(const QString & strLine, short &dwCookie);
    bool detectBadBox(const QString & strLine, short &dwCookie);
    bool detectLaTeXLineNumber(QString & warning, short & dwCookie, int len);
    bool detectBadBoxLineNumber(QString & strLine, short & dwCookie, int len);

    bool fileExists(const QString & name);

protected:
    /**
    These constants are describing, which item types is currently
    parsed.
    */
    enum tagCookies
    {
        itmNone = 0,
        itmError,
        itmWarning,
        itmBadBox
    };


private:
    KileDocument::Extensions *m_extensions;
    /** number or errors detected */
    int m_nErrors;

    /** number of warning detected */
    int m_nWarnings;

    /** number of bad boxes detected */
    int m_nBadBoxes;

    int m_nParens;

    int m_nOutputLines;
    QString m_log, m_source, m_srcPath;

    /** Pointer to list of Latex output information */
    LatexOutputInfoArray *m_infoList;

    QString m_logFile;

    // for QuickPreview
    QString texfilename;
    int selrow;
    int docrow;

    /**
    Stack containing the files parsed by the compiler. The top-most
    element is the currently parsed file.
    */
    QStack<LOFStackItem> m_stackFile;

    /** The item currently parsed. */
    LatexOutputInfo m_currentItem;

    /**
    Returns the zero based index of the currently parsed line in the
    output file.
    */
    int GetCurrentOutputLine() const;

    void setSource(const QString &src);

};

}

#endif
