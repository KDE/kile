/**************************************************************************
*   Copyright (C) 2011-2022 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BIBTEXPARSER_H
#define BIBTEXPARSER_H

#include "documentinfo.h"
#include "kileconstants.h"
#include "kileextensions.h"
#include "parser.h"

namespace KileParser {

class BibTeXParserInput : public ParserInput
{
public:
    BibTeXParserInput(const QUrl &url, const QStringList &textLines);

    QStringList textLines;
};

class BibTeXParserOutput : public ParserOutput {
public:
    BibTeXParserOutput();
    virtual ~BibTeXParserOutput() override;

    QStringList bibItems;
};


class BibTeXParser : public Parser
{
    Q_OBJECT

public:
    BibTeXParser(ParserThread *parserThread, BibTeXParserInput *input, QObject *parent = nullptr);
    virtual ~BibTeXParser() override;

    ParserOutput* parse() override;

protected:
    QStringList m_textLines;
};

}

#endif
