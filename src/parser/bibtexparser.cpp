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
#include <QDebug>
#include <QRegularExpression>

#include <KLocalizedString>

#include "kiledebug.h"
#include "codecompletion.h"
#include "parserthread.h"

namespace KileParser {

BibTeXParserInput::BibTeXParserInput(const QUrl &url, const QStringList &textLines)
    : ParserInput(url),
      textLines(textLines)
{
}

BibTeXParserOutput::BibTeXParserOutput()
{
}

BibTeXParserOutput::~BibTeXParserOutput()
{
    qCDebug(LOG_KILE_PARSER);
}

BibTeXParser::BibTeXParser(ParserThread *parserThread, BibTeXParserInput *input, QObject *parent)
    : Parser(parserThread, parent),
      m_textLines(input->textLines)
{
}

BibTeXParser::~BibTeXParser()
{
    qCDebug(LOG_KILE_PARSER);
}

ParserOutput* BibTeXParser::parse()
{
    BibTeXParserOutput *parserOutput = new BibTeXParserOutput();

    qCDebug(LOG_KILE_PARSER);

    static QRegularExpression reItem("^(\\s*)@([a-zA-Z]+)");
    static QRegularExpression reSpecial("^string|preamble|comment$");

    QString key;
    int col = 0, startcol, startline = 0;

// 	Q_EMIT(parsingStarted(m_doc->lines()));
    for(int i = 0; i < m_textLines.size(); ++i) {
        if(!m_parserThread->shouldContinueDocumentParsing()) {
            qCDebug(LOG_KILE_PARSER) << "stopping...";
            delete(parserOutput);
            return nullptr;
        }
// 		Q_EMIT(parsingUpdate(i));
        QString s = getTextLine(m_textLines, i);
        auto match = reItem.match(s);
        if (!match.hasMatch()) {
            continue;
        }

        auto specialMatch = reSpecial.match(match.captured(2).toLower());

        if (specialMatch.hasMatch()) {
            continue;
        }

        qCDebug(LOG_KILE_PARSER) << "found: " << match.captured(2);
        //start looking for key
        key = "";
        bool keystarted = false;
        int state = 0;
        startcol = match.captured(1).length();
        col  = startcol + match.captured(2).length();

        while(col < static_cast<int>(s.length())) {
            ++col;
            if(col == static_cast<int>(s.length())) {
                do {
                    ++i;
                    s = getTextLine(m_textLines, i);
                }
                while((s.length() == 0) && (i < m_textLines.size()));

                if(i == m_textLines.size()) {
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
                    qCDebug(LOG_KILE_PARSER) << "found: " << key;
                    parserOutput->bibItems.append(key);
                    parserOutput->structureViewItems.push_back(new StructureViewItem(key, startline+1, startcol, KileStruct::BibItem, 0, startline+1, startcol, "viewbib", match.captured(2).toLower()));
                    break;
                }
                else {
                    key += s[col];
                    if(!keystarted) {
                        startcol = col;
                        startline = i;
                    }
                    keystarted = true;
                }
            }
        }
    }

    return parserOutput;
}


}

