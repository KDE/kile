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

#include "kilelyxserver.h"

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
	m_running(false),
	m_count(0)
{
	m_pipeIn.setAutoDelete(true);
	m_notifier.setAutoDelete(true);

	if (st)
		start();
}

KileLyxServer::~KileLyxServer()
{
	stop();
}

bool KileLyxServer::start()
{
	if (m_running)
		stop();

	//kdDebug() << "starting the LyX server..." << endl;

	if (openPipes())
	{
		QSocketNotifier *notifier;
		QPtrListIterator<QFile> it(m_pipeIn);
		while (it.current())
		{
			if ((*it)->name().right(3) == ".in" )
			{
				notifier = new QSocketNotifier((*it)->handle(), QSocketNotifier::Read, this);
				connect(notifier, SIGNAL(activated(int)), this, SLOT(receive(int)));
				m_notifier.append(notifier);
			}
			++it;
		}
		m_running=true;
	}

	return m_running;
}

bool KileLyxServer::openPipes()
{
	QString home(getenv("HOME"));

	QStringList pipes;
	pipes << home+"/.lyxpipe.in" << home+"/.lyx/lyxpipe.in";
	pipes << home+"/.lyxpipe.out" << home+"/.lyx/lyxpipe.out";

	bool opened = false;
	QFileInfo info;
	QFile *file;

	for (uint i=0; i < pipes.count(); i++)
	{
		info.setFile(pipes[i]);
		if ( ! info.exists() )
		{
			mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH;
			//create the dir first
			if (mkdir(info.dirPath().ascii(), perms | S_IXUSR) == -1)
				perror( "Could not create directory for pipe ");

			if (mkfifo(pipes[i].ascii(), perms) == -1)
				perror( "Could not create pipe ");
		}

		file  = new QFile(info.absFilePath());
		if (!file->open(IO_ReadWrite))
		{
			kdError() << "Could not open " << info.absFilePath() << endl;
		}
		else
		{
			//kdDebug() << "Opened " << info.absFilePath() << endl;
			m_pipeIn.append(file);
			m_file.insert(file->handle(),file);
			opened=true;

			//read all data on the pipe (this is to ensure that old unprocessed data is not going to bother us)
			//file->readAll(); //find a non-blocking solution
		}
	}

	return opened;
}

void KileLyxServer::stop()
{
	//kdDebug() << "stopping the LyX server after " << m_count << " requests..." << endl;

	QPtrListIterator<QFile> it(m_pipeIn);
	while (it.current())
	{
		(*it)->close();
		++it;
	}

	m_pipeIn.clear();
	m_notifier.clear();

	m_running=false;
}

void KileLyxServer::receive(int fd)
{
	m_count++;
	if (m_file[fd])
	{
		QString line;
		m_file[fd]->readLine(line, 80);
		line=line.stripWhiteSpace();
		m_count++;

		QRegExp cite(":citation-insert:(.*)$");
		QRegExp bibtexins(":bibtex-insert:(.*)$");
		QRegExp bibtexdbadd(":bibtex-database-add:(.*)$");
		
		if (cite.search(line) > -1)
			emit(insertCite(cite.cap(1)));
		else if ( bibtexins.search(line) > -1)
			emit(insertBibTeX(bibtexins.cap(1)));
		else if ( bibtexdbadd.search(line) )
			emit(insertBibTeXDatabaseAdd(bibtexdbadd.cap(1)));
	}
}

#include "kilelyxserver.moc"
