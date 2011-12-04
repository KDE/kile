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

#include "parsermanager.h"

#include "documentinfo.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "parserthread.h"

namespace KileParser {

Manager::Manager(KileInfo *info, QObject *parent) :
	QObject(parent),
	m_ki(info)
{
	KILE_DEBUG();
	m_parserThread = new ParserThread(m_ki, this);
	connect(m_parserThread, SIGNAL(parsingComplete(const KUrl&, KileParser::ParserOutput*)),
	        m_ki->docManager(), SLOT(handleParsingComplete(const KUrl&, KileParser::ParserOutput*)), Qt::QueuedConnection);
	connect(m_parserThread, SIGNAL(parsingQueueEmpty()),
	        this, SIGNAL(parsingComplete()), Qt::QueuedConnection);
	connect(m_parserThread, SIGNAL(parsingStarted()),
	        this, SIGNAL(parsingStarted()), Qt::QueuedConnection);
	m_parserThread->start();
}


Manager::~Manager()
{
	KILE_DEBUG() << "destroying...";
	m_parserThread->stopParsing();
}

void Manager::parseDocument(KileDocument::TextInfo* textInfo)
{
	KILE_DEBUG() << textInfo;
	m_parserThread->addDocument(textInfo);
}

bool Manager::isParsingComplete()
{
	return m_parserThread->isParsingComplete();
}

}

#include "parsermanager.moc"
