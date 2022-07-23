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
    static QRegExp reComments("[^\\\\](%.*$)");
    QString s = line;
    todo.type = -1;
    if(!s.isEmpty()) {
        // remove comment lines
        if(s[0] == '%') {
            searchTodoComment(s,0,todo);
            s.clear();
        }
        else {
            //remove escaped \ characters
            s.replace("\\\\", "  ");

            //remove comments
            int pos = s.indexOf(reComments);
            if(pos != -1) {
                searchTodoComment(s, pos,todo);
                s = s.left(reComments.pos(1));
            }
        }
    }
    return s;
}

void Parser::searchTodoComment(const QString &s, uint startpos, TodoResult &todo)
{
    static QRegExp reTodoComment("\\b(TODO|FIXME)\\b(:|\\s)?\\s*(.*)");

    if(s.indexOf(reTodoComment, startpos) != -1) {
        todo.type = (reTodoComment.cap(1) == "TODO") ? KileStruct::ToDo : KileStruct::FixMe;
        todo.colTag = reTodoComment.pos(1);
        todo.colComment = reTodoComment.pos(3);
        todo.comment = reTodoComment.cap(3).trimmed();
    }
}

// match a { with the corresponding }
// pos is the position of the {
QString Parser::matchBracket(const QStringList& textLines, QChar obracket, int &l, int &pos)
{
    QChar cbracket;
    if(obracket == '{') {
        cbracket = '}';
    }
    else if(obracket == '[') {
        cbracket = ']';
    }
    else if(obracket == '(') {
        cbracket = ')';
    }

    QString line, grab = "";
    int count=0, len;
    ++pos;

    TodoResult todo;
    while(l < textLines.size()) {
        line = processTextline(textLines[l], todo);
        len = line.length();
        for (int i = pos; i < len; ++i) {
            if(line[i] == '\\' && (line[i+1] == obracket || line[i+1] == cbracket)) {
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

