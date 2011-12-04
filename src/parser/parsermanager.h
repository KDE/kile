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

#ifndef PARSERMANAGER_H
#define PARSERMANAGER_H

#include <QList>
#include <QObject>
#include <QQueue>

class KileInfo;

namespace KileDocument {
	class Info;
	class TextInfo;
}

namespace KileParser {

class ParserThread;

class Manager : public QObject
{
	Q_OBJECT

public:
	explicit Manager(KileInfo *ki, QObject *parent = 0);
	~Manager();

	void parseDocument(KileDocument::TextInfo* textInfo);

	bool isParsingComplete();

Q_SIGNALS:
	void parsingComplete();
	void parsingStarted();

private:
	KileInfo *m_ki;
	ParserThread *m_parserThread;
};

}

#endif
