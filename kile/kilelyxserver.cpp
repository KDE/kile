/***************************************************************************
                          kilelyxserver.cpp  -  description
                             -------------------
    begin                : Sat Sept 9 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h> //getenv
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "kilelyxserver.h"
#include "kileactions.h"

#include <qlayout.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qsocketnotifier.h>
#include <qregexp.h>

#include <kdebug.h>
#include <ktextedit.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kurl.h>

KileLyxServer::KileLyxServer(bool st) :
	m_running(false)
{
	m_pipeFile.setAutoDelete(true);

	QString home(getenv("HOME"));
	//in pipes should go first
	m_pipes << home+"/.lyxpipe.in" << home+"/.lyx/lyxpipe.in";
	m_pipes << home+"/.lyxpipe.out" << home+"/.lyx/lyxpipe.out";
	
	if (st)
		start();
}

KileLyxServer::~KileLyxServer()
{
	stop();
	//TODO remove pipes here, otherwise LyX is confused, and we don't want that
}

bool KileLyxServer::start()
{
	if (m_running)
		stop();

	kdDebug() << "starting the LyX server..." << endl;

	createPipes();
	openPipes();
	
	m_running=true;
	m_thread = new LyxPipeThread(this, m_fds);
	m_thread->start();

	return m_running;
}

void KileLyxServer::stop()
{
	QPtrListIterator<QFile> it(m_pipeFile);
	while (it.current())
	{
		(*it)->close();
		++it;
	}
	m_pipeFile.clear();
	m_running=false;
}

void KileLyxServer::createPipes()
{
	for (uint i = 0; i < m_pipes.count(); i++)
	{
		QFileInfo info(m_pipes[i]);
		if ( ! info.exists() )
		{
			mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH;
			//create the dir first
			if (mkdir(info.dirPath().local8Bit(), perms | S_IXUSR) == -1)
				perror( "Could not create directory for pipe ");
	
			if (mkfifo(m_pipes[i].local8Bit(), perms) == -1)
				perror( "Could not create pipe ");
		}
	}
}

void KileLyxServer::removePipes()
{
	for (uint i = 0; i < m_pipes.count(); i++)
	{
		QFile file(m_pipes[i]);
		file.remove();
	}
}

void KileLyxServer::openPipes()
{
	for (uint i = 0; i < m_pipes.count(); i++)
	{
		QFile *file = new QFile(m_pipes[i]);
		if (!file->open(IO_ReadWrite))
		{
			kdError() << "Could not open " << m_pipes[i] << endl;
		}
		else
		{
			kdDebug() << "Opened " << m_pipes[i] << endl;
			m_pipeFile.append(file);
			if (m_pipes[i].endsWith(".in")) m_fds.append(file->handle());
		}
	}
}

void KileLyxServer::processLine(const QString &line)
{
	static QRegExp::QRegExp reCite(":citation-insert:(.*)$");
	static QRegExp::QRegExp reBibtexdbadd(":bibtex-database-add:(.*)$");

	if (reCite.search(line) > -1)
		emit insert(KileAction::TagData("Cite", "\\cite{"+reCite.cap(1)+"}", QString::null, 7+ reCite.cap(1).length()));
	else if ( reBibtexdbadd.search(line) > -1 )
		emit insert(KileAction::TagData("BibTeX db add", "\\bibliography{"+ reBibtexdbadd.cap(1) + "}", QString::null, 15+reBibtexdbadd.cap(1).length()));
}

LyxPipeThread::LyxPipeThread(KileLyxServer *receiver, QValueList<int> fds) : 
	m_receiver(receiver),
	m_fds(fds)
{}

void LyxPipeThread::run()
{
	fd_set set;
	struct timeval timeout;

	/* Initialize the file descriptor set. */
	FD_ZERO (&set);
	for (uint i = 0; i < m_fds.count(); i++)
		FD_SET (m_fds[i], &set);

	/* Initialize the timeout data structure. */
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;

	char *result = new char[2];
	size_t bytesRead = 1;
	QString line = QString::null;

	while (true)
	{
		select (FD_SETSIZE, &set, NULL, NULL, &timeout);
		for ( uint i = 0; i < m_fds.count(); i++ )
		{
			if (FD_ISSET(m_fds[i], &set)) 
			{
				bytesRead = read(m_fds[i], result, bytesRead);
				result[1] = '\0';
				if (bytesRead > 0) 
				{
					if ( result[0] != '\n' )
						line += QString::fromLocal8Bit(result, (int)bytesRead);
					else
					{
						kdDebug() << "read line : " << line.stripWhiteSpace() << endl;
						m_receiver->processLine(line); 
						line = QString::null;
					}
				}
			}
		}
	}
}

#include "kilelyxserver.moc"
