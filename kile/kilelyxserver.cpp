/***************************************************************************
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
#include <unistd.h> //read

#include "kilelyxserver.h"
#include "kileactions.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qsocketnotifier.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>

KileLyxServer::KileLyxServer(bool st) :
	m_running(false)
{
	m_pipeIn.setAutoDelete(true);
	m_notifier.setAutoDelete(true);
	m_file.setAutoDelete(false);

	QString home(getenv("HOME"));
	m_pipes << home+"/.lyxpipe.in" << home+"/.lyx/lyxpipe.in";
	m_pipes << home+"/.lyxpipe.out" << home+"/.lyx/lyxpipe.out";

	if (st) start();
}

KileLyxServer::~KileLyxServer()
{
	stop();
	removePipes();
}

bool KileLyxServer::start()
{
	if (m_running) stop();

	kdDebug() << "starting the LyX server..." << endl;

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
				kdDebug() << "Created notifier for " << (*it)->name() << endl;
			}
			else
				kdDebug() << "No notifier created fro " << (*it)->name() << endl;
			++it;
		}
		m_running=true;
	}

	return m_running;
}

bool KileLyxServer::openPipes()
{
	bool opened = false;
	QFileInfo info;
	QFile *file;

	for (uint i=0; i < m_pipes.count(); ++i)
	{
		info.setFile(m_pipes[i]);
		if ( ! info.exists() )
		{
			mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH;
			//create the dir first
			if (mkdir(info.dirPath().ascii(), perms | S_IXUSR) == -1)
				perror( "Could not create directory for pipe ");
			else
				kdDebug() << "Created directory " << info.dirPath() << endl;

			if (mkfifo(m_pipes[i].ascii(), perms) == -1)
   				perror( "Could not create pipe ");
			else
				kdDebug() << "Created pipe " << m_pipes[i] << endl;
		}

		file  = new QFile(info.absFilePath());
		if (!file->open(IO_ReadWrite))
		{
			kdError() << "Could not open " << info.absFilePath() << endl;
		}
		else
		{
			kdDebug() << "Opened " << info.absFilePath() << endl;
			m_pipeIn.append(file);
			m_file.insert(file->handle(),file);
			opened=true;
		}
	}

	return opened;
}

void KileLyxServer::stop()
{
	kdDebug() << "stopping the LyX server..." << endl;

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

void KileLyxServer::removePipes()
{
 	for ( uint i = 0; i < m_pipes.count(); ++i) 
		QFile::remove(m_pipes[i]);
}

void KileLyxServer::processLine(const QString &line)
{
	QRegExp cite(":citation-insert:(.*)$");
	QRegExp bibtexdbadd(":bibtex-database-add:(.*)$");

	if (cite.search(line) > -1)
		emit(insert(KileAction::TagData("Cite", "\\cite{"+cite.cap(1)+"}", QString::null, 7+cite.cap(1).length())));
	else if ( bibtexdbadd.search(line) > -1 )
		emit(insert(KileAction::TagData("BibTeX db add", "\\bibliography{"+ bibtexdbadd.cap(1) + "}", QString::null, 15+bibtexdbadd.cap(1).length())));
}

void KileLyxServer::receive(int fd)
{
 	if (m_file[fd])
 	{
 		int bytesRead;
 		int const size = 256;
        char buffer[size];
 		if ((bytesRead = read(fd, buffer, size - 1)) > 0 ) 
 		{
  			buffer[bytesRead] = '\0'; // turn it into a c string
            QStringList cmds = QStringList::split('\n', QString(buffer).stripWhiteSpace());
			for ( uint i = 0; i < cmds.count(); ++i )
				processLine(cmds[i]);
		}
 	}
}

#include "kilelyxserver.moc"
