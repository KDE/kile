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

#ifndef BIBTEXPARSER_H
#define BIBTEXPARSER_H

#include <QLinkedList>

#include "documentinfo.h"
#include "kileconstants.h"
#include "kileextensions.h"
#include "parser.h"

namespace KileParser {

class BibTeXParserOutput : public ParserOutput {
public:
	BibTeXParserOutput();
	virtual ~BibTeXParserOutput();

	QStringList bibItems;
};


class BibTeXParser : public Parser
{
	Q_OBJECT

public:
	BibTeXParser(ParserThread *parserThread, QObject *parent = NULL);
	virtual ~BibTeXParser();

	ParserOutput* parse(const QStringList& textLines);
};

}

#endif