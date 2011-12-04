/**********************************************************************************
*   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)           *
*                 2005-2007 by Holger Danielsson (holger.danielsson@versanet.de)  *
*                 2006-2011 by Michel Ludwig (michel.ludwig@kdemail.net)          *
***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "bibtexparser.h"

#include <QFileInfo>
#include <QRegExp>

#include <KLocale>

#include "codecompletion.h"
#include "parserthread.h"

namespace KileParser {

BibTeXParserOutput::BibTeXParserOutput()
{
}

BibTeXParserOutput::~BibTeXParserOutput()
{
    KILE_DEBUG();
}

BibTeXParser::BibTeXParser(ParserThread *parserThread, QObject *parent) :
	Parser(parserThread, parent)
{
}

BibTeXParser::~BibTeXParser()
{
    KILE_DEBUG();
}

ParserOutput* BibTeXParser::parse(const QStringList& textLines)
{
	BibTeXParserOutput *parserOutput = new BibTeXParserOutput();

	KILE_DEBUG();

	static QRegExp reItem("^(\\s*)@([a-zA-Z]+)");
	static QRegExp reSpecial("string|preamble|comment");

	QString s, key;
	int col = 0, startcol, startline = 0;

// 	emit(parsingStarted(m_doc->lines()));
	for(int i = 0; i < textLines.size(); ++i) {
		if(!m_parserThread->shouldContinueDocumentParsing()) {
			KILE_DEBUG() << "stopping...";
			delete(parserOutput);
			return NULL;
		}
// 		emit(parsingUpdate(i));
		s = getTextLine(textLines, i);
		if((s.indexOf(reItem) != -1) && !reSpecial.exactMatch(reItem.cap(2).toLower())) {
			KILE_DEBUG() << "found: " << reItem.cap(2);
			//start looking for key
			key = "";
			bool keystarted = false;
			int state = 0;
			startcol = reItem.cap(1).length();
			col  = startcol + reItem.cap(2).length();

			while(col < static_cast<int>(s.length())) {
				++col;
				if(col == static_cast<int>(s.length())) {
					do {
						++i;
						s = getTextLine(textLines, i);
					}
					while((s.length() == 0) && (i < textLines.size()));

					if(i == textLines.size()) {
						break;
					}
					col = 0;
				}

				if(state == 0) {
					if(s[col] == '{') {
						state = 1;
					}
					else if(!s[col].isSpace()) {
						break;
					}
				}
				else if(state == 1) {
					if(s[col] == ',') {
						key = key.trimmed();
						KILE_DEBUG() << "found: " << key;
						parserOutput->bibItems.append(key);
						parserOutput->structureViewItems.push_back(new StructureViewItem(key, startline+1, startcol, KileStruct::BibItem, 0, startline+1, startcol, "viewbib", reItem.cap(2).toLower()));
						break;
					}
					else {
						key += s[col];
						if(!keystarted) {
							startcol = col; startline = i;
						}
						keystarted = true;
					}
				}
			}
		}
	}

	return parserOutput;;
}


}

#include "bibtexparser.moc"
