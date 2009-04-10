/***************************************************************************
    begin                : Sat Sept 9 2003
    edit		 : Wed Jan 14 2009
    copyright            : (C) 2003 by Jeroen Wijnhout, 2007-2009 by Thomas Braun
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

#include "kilelyxserver.h"

#include <sys/stat.h>
#include <stdlib.h> //getenv
#include <unistd.h> //read
#include <fcntl.h>

#include "kileactions.h"
#include "kiledebug.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSocketNotifier>
#include <QRegExp>

#include <KLocale>

KileLyxServer::KileLyxServer(bool startMe) :
	m_perms(S_IRUSR | S_IWUSR), m_running(false)
{	
	KILE_DEBUG() << "===KileLyxServer::KileLyxServer(bool" << startMe << ")=== ";

	m_tempDir = new KTempDir();
	if(m_tempDir->status() != 0) {
		KILE_DEBUG() << "an error ocurred while creating a tempfile" ;
		return;
	}

	m_links << ".lyxpipe.in" << ".lyx/lyxpipe.in";
	m_links << ".lyxpipe.out" << ".lyx/lyxpipe.out";

	for(int i = 0; i < m_links.count() ; ++i) {
		m_pipes.append( m_tempDir->name() + m_links[i] );
		m_links[i].prepend(QDir::homePath() + QDir::separator() );
		KILE_DEBUG() << "m_pipes[" << i << "]=" << m_pipes[i];
		KILE_DEBUG() << "m_links[" << i << "]=" << m_links[i];
	}

	if(startMe) {
		start();
	}
}

KileLyxServer::~KileLyxServer()
{
	stop();
	removePipes();

	delete m_tempDir;

	for(QList<QFile*>::iterator i = m_pipeIn.begin(); i != m_pipeIn.end(); ++i) {
		delete *i;
	}

	for(QList<QSocketNotifier*>::iterator i = m_notifier.begin(); i != m_notifier.end(); ++i) {
		delete *i;
	}
}

bool KileLyxServer::start()
{
	if (m_running) {
		stop();
	}

	KILE_DEBUG() << "Starting the LyX server...";

	if (openPipes()) {
		QSocketNotifier *notifier;
		for(QList<QFile*>::iterator it = m_pipeIn.begin(); it != m_pipeIn.end(); ++it) {
			if ((*it)->fileName().right(3) == ".in" ) {
				notifier = new QSocketNotifier((*it)->handle(), QSocketNotifier::Read, this);
				connect(notifier, SIGNAL(activated(int)), this, SLOT(receive(int)));
				m_notifier.append(notifier);
				KILE_DEBUG() << "Created notifier for " << (*it)->fileName();
			}
			else {
				KILE_DEBUG() << "No notifier created for " << (*it)->fileName();
			}
		}
		m_running=true;
	}

	return m_running;
}

bool KileLyxServer::openPipes()
{
	KILE_DEBUG() << "===bool KileLyxServer::openPipes()===";
	
	#ifdef Q_WS_WIN
		kError() << "kile's lyx server can not work on windows since we don't have pipes";
		kError() << "And also lyx itself does not support it, see  http://wiki.lyx.org/LyX/LyXServer";
		return false;
	#else
		bool opened = false;
		QFileInfo pipeInfo,linkInfo;
		QFile *file;
		struct stat buf;
		struct stat *stats = &buf;
		
		QDir lyxDir(QDir::homePath() + QDir::separator() + ".lyx");
		if(!lyxDir.exists()){
			KILE_DEBUG() << "Directory " << lyxDir.absolutePath() << " does not exist";
			if(mkdir(QFile::encodeName( lyxDir.path() ), m_perms | S_IXUSR) == -1){
				kError() << "Could not create directory";
			}
			else{
				KILE_DEBUG() << "Directory created successfully";	
			}
		}
		
		for(int i = 0; i < m_pipes.count(); ++i) {
			pipeInfo.setFile(m_pipes[i]);
			linkInfo.setFile(m_links[i]);
			
			QFile::remove(linkInfo.absoluteFilePath());
			linkInfo.refresh();

			KILE_DEBUG() << "pipe=" << m_pipes[i] << endl;
			KILE_DEBUG() << "link=" << m_links[i] << endl;
			
			if(!pipeInfo.exists()) {
				//create the dir first
				if(!QFileInfo(pipeInfo.absolutePath()).exists()) {
					if(mkdir(QFile::encodeName( pipeInfo.path() ), m_perms | S_IXUSR) == -1) {
						kError() << "Could not create directory for pipe";
						continue;
					}
					else {
						KILE_DEBUG() << "Created directory " << pipeInfo.path();
					}
				}
					if (mkfifo(QFile::encodeName( pipeInfo.absoluteFilePath() ), m_perms) != 0) {
						kError() << "Could not create pipe: " << pipeInfo.absoluteFilePath();
						continue;
					}
					else {
						KILE_DEBUG() << "Created pipe: " << pipeInfo.absoluteFilePath();
					}
			}
			
			if(symlink(QFile::encodeName(pipeInfo.absoluteFilePath()),QFile::encodeName(linkInfo.absoluteFilePath())) != 0) {
				kError() << "Could not create symlink: " << linkInfo.absoluteFilePath() << " --> " << pipeInfo.absoluteFilePath();
				continue;
			}
			
			// the file object will be deleted at the shutdown of the server in KileLyxServer::stop()
			file  = new QFile(pipeInfo.absoluteFilePath());
			pipeInfo.refresh();

			if(pipeInfo.exists() && file->open(QIODevice::ReadWrite)) { // in that order we don't create the file if it does not exist
				KILE_DEBUG() << "Opened file: " << pipeInfo.absoluteFilePath();
				fstat(file->handle(),stats);
				if(!S_ISFIFO(stats->st_mode)) {
					kError() << "The file " << pipeInfo.absoluteFilePath() <<  "we just created is not a pipe!";
					file->close();
					delete file;
					continue;
				}
				else { 
					m_pipeIn.append(file);
					m_file.insert(file->handle(), file);
					opened = true;
					KILE_DEBUG() << "everything is correct :)" << endl;
				}
			}
			else {
				kError() << "Could not open " << pipeInfo.absoluteFilePath();
			}
		}
		return opened;
	#endif
}

void KileLyxServer::stop()
{
	KILE_DEBUG() << "Stopping the LyX server...";

	for(QList<QFile*>::iterator it = m_pipeIn.begin(); it != m_pipeIn.end(); ++it) {
		(*it)->close();
		delete *it;
	}

	for(QList<QSocketNotifier*>::iterator i = m_notifier.begin(); i != m_notifier.end(); ++i) {
		delete *i;
	}

	m_pipeIn.clear();
	m_notifier.clear();

	m_running=false;
}

void KileLyxServer::removePipes()
{
	for (int i = 0; i < m_links.count(); ++i) {
		QFile::remove(m_links[i]);
	}
	for (int i = 0; i < m_pipes.count(); ++i) {
		QFile::remove(m_pipes[i]);
	}
}

void KileLyxServer::processLine(const QString &line)
{
	KILE_DEBUG() << "===void KileLyxServer::processLine(const QString " << line << ")===";
	
	QRegExp reCite(":citation-insert:(.*)$");
	QRegExp reBibtexdbadd(":bibtex-database-add:(.*)$");
	QRegExp rePaste(":paste:(.*)$");
	
	if(line.indexOf(reCite) != -1) {
		emit(insert(KileAction::TagData(i18n("Cite"), "\\cite{"+reCite.cap(1)+'}')));
	}
	else if(line.indexOf(reBibtexdbadd) != -1) {
		emit(insert(KileAction::TagData(i18n("Add BibTeX database"), "\\bibliography{"+ reBibtexdbadd.cap(1) + '}')));
	}
	else if(line.indexOf(rePaste) != -1) {
		emit(insert(KileAction::TagData(i18n("Paste"), rePaste.cap(1))));
	}
}

void KileLyxServer::receive(int fd)
{
	if(m_file[fd]) {
		int bytesRead;
		int const size = 256;
		char buffer[size];
		if((bytesRead = read(fd, buffer, size - 1)) > 0) {
			buffer[bytesRead] = '\0'; // turn it into a c string
			QStringList cmds = QString(buffer).trimmed().split('\n');
			for(int i = 0; i < cmds.count(); ++i) {
				processLine(cmds[i]);
			}
		}
	}
}

#include "kilelyxserver.moc"
