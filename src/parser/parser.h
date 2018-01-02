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

#ifndef PARSER_H
#define PARSER_H

#include <QLinkedList>
#include <QObject>

#include <QUrl>

class KileInfo;

namespace KileDocument {
class Info;
class TextInfo;
}

namespace KileParser {

struct TodoResult
{
    int type;
    uint colTag;
    uint colComment;
    QString comment;
};

class ParserThread;

class StructureViewItem {
public:
    StructureViewItem(const QString &title, uint line, uint column, int type, int level, uint startline, uint startcol,
                      const QString &pix, const QString &folder);
    ~StructureViewItem();

    QString title;
    uint line;
    uint column;
    int type;
    int level;
    uint startline;
    uint startcol;
    QString pix;
    QString folder;
};

class ParserInput {
public:
    explicit ParserInput(const QUrl &url);
    virtual ~ParserInput();

    QUrl url;
};

class ParserOutput {
public:
    virtual ~ParserOutput();

    QLinkedList<StructureViewItem*> structureViewItems;
};

class Parser : public QObject
{
    Q_OBJECT

public:
    explicit Parser(ParserThread *parserThread, QObject *parent = Q_NULLPTR);
    virtual ~Parser();

    virtual ParserOutput* parse() = 0;

protected:
    ParserThread *m_parserThread;

    QString processTextline(const QString &line, TodoResult &todo);
    void searchTodoComment(const QString &s, uint startpos, TodoResult &todo);
    QString matchBracket(const QStringList& textLines, QChar obracket, int &l, int &pos);
    // for now, we have to emulate the behaviour of 'KTextEditor::Document::line':
    // we return an empty string if the given line number is invalid
    QString getTextLine(const QStringList& textLines, int line);
};

}

#endif
