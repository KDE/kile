/**********************************************************************************
*   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)           *
*                 2005-2007 by Holger Danielsson (holger.danielsson@versanet.de)  *
*                 2006-2022 by Michel Ludwig (michel.ludwig@kdemail.net)          *
***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "parser.h"

#include <QStringList>

#include "parserthread.h"

namespace KileParser {

StructureViewItem::StructureViewItem(const QString &title, uint line, uint column, int type, int level, uint startline, uint startcol,
                                     const QString &pix, const QString &folder)
    :  title(title),
       line(line),
       column(column),
       type(type),
       level(level),
       startline(startline),
       startcol(startcol),
       pix(pix),
       folder(folder)
{
}

StructureViewItem::~StructureViewItem()
{
}

ParserInput::ParserInput(const QUrl &url)
    : url(url)
{
}

ParserInput::~ParserInput()
{
}

ParserOutput::~ParserOutput()
{
    for(StructureViewItem *item : structureViewItems) {
        delete(item);
    }
}

Parser::Parser(ParserThread *parserThread, QObject *parent) :
    QObject(parent),
    m_parserThread(parserThread)
{
}

Parser::~Parser()
{
}

QString Parser::processTextline(const QString &line, TodoResult &todo)
{
    static QRegularExpression reComments(QStringLiteral("[^\\\\](%.*$)"));
    QString s = line;
    todo.type = -1;
    if(!s.isEmpty()) {
        // remove comment lines
        if(s[0] == QLatin1Char('%')) {
            searchTodoComment(s,0,todo);
            s.clear();
        }
        else {
            //remove escaped \ characters
            s.replace(QStringLiteral("\\\\"), QStringLiteral("  "));

            //remove comments
            const auto match = reComments.match(s);
            if(match.hasMatch()) {
                searchTodoComment(s, match.capturedStart(1), todo);
                s = s.left(match.capturedStart(1));
            }
        }
    }
    return s;
}

void Parser::searchTodoComment(const QString &s, uint startpos, TodoResult &todo)
{
    static QRegularExpression reTodoComment(QStringLiteral("\\b(TODO|FIXME)\\b(:|\\s)?\\s*(.*)"));

    auto match = reTodoComment.match(s, startpos);
    if (match.hasMatch()) {
        todo.type = (match.capturedView(1) == QLatin1String("TODO")) ? KileStruct::ToDo : KileStruct::FixMe;
        todo.colTag = match.capturedStart(1);
        todo.colComment = match.capturedStart(3);
        todo.comment = match.captured(3).trimmed();
    }
}

// match a { with the corresponding }
// pos is the position of the {
QString Parser::matchBracket(const QStringList& textLines, QChar obracket, int &l, int &pos)
{
    QChar cbracket;
    if (obracket == QLatin1Char('{')) {
        cbracket = QLatin1Char('}');
    }
    else if (obracket == QLatin1Char('[')) {
        cbracket = QLatin1Char(']');
    }
    else if (obracket == QLatin1Char('(')) {
        cbracket = QLatin1Char(')');
    }

    QString line, grab = QStringLiteral("");
    int count=0;
    ++pos;

    TodoResult todo;
    while(l < textLines.size()) {
        line = processTextline(textLines[l], todo);
        int len = line.length();
        for (int i = pos; i < len; ++i) {
            if (line[i] == QLatin1Char('\\') && (line[i+1] == obracket || line[i+1] == cbracket)) {
                ++i;
            }
            else if(line[i] == obracket) {
                ++count;
            }
            else if(line[i] == cbracket) {
                --count;
                if (count < 0) {
                    pos = i;
                    return grab;
                }
            }

            grab += line[i];
        }
        ++l;
        pos = 0;
    }

    return QString();
}

QString Parser::getTextLine(const QStringList& textLines, int line)
{
    if(line < 0 || line >= textLines.size()) {
        return QString();
    }
    return textLines[line];
}

}

