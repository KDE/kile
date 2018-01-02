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

#ifndef LATEXPARSER_H
#define LATEXPARSER_H

#include <QLinkedList>

#include "documentinfo.h"
#include "kileconstants.h"
#include "kileextensions.h"
#include "parser.h"

namespace KileParser {

struct BracketResult
{
    BracketResult() : line(0), col(0) {}
    QString option, value;
    int line, col;
};

class LaTeXParserInput : public ParserInput
{
public:
    LaTeXParserInput(const QUrl &url, QStringList textLines,
                     KileDocument::Extensions *extensions,
                     const QMap<QString, KileStructData>& dictStructLevel,
                     bool showSectioningLabels,
                     bool showStructureTodo);

    QStringList textLines;
    KileDocument::Extensions *extensions;
    const QMap<QString, KileStructData>& dictStructLevel;
    bool showSectioningLabels;
    bool showStructureTodo;
};

class LaTeXParserOutput : public ParserOutput {
public:
    LaTeXParserOutput();
    virtual ~LaTeXParserOutput();

    QStringList labels;
    QStringList bibItems;
    QStringList deps;
    QStringList bibliography;
    QStringList packages;
    QStringList newCommands;
    QStringList asyFigures;
    QString preamble;
    bool bIsRoot;
};


class LaTeXParser : public Parser
{
    Q_OBJECT

public:
    LaTeXParser(ParserThread *parserThread, LaTeXParserInput *input,
                QObject *parent = Q_NULLPTR);
    virtual ~LaTeXParser();

    ParserOutput* parse();

protected:
    KileDocument::Extensions *m_extensions;
    QStringList m_textLines;
    const QMap<QString, KileStructData>& m_dictStructLevel;
    bool m_showSectioningLabels;
    bool m_showStructureTodo;

    BracketResult matchBracket(const QStringList& textLines, int &l, int &pos);
};

}

#endif
