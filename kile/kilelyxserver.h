/***************************************************************************
                          kilelyxserver.h  -  description
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

#ifndef _LYXSERVER_H_
#define _LYXSERVER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include <qptrlist.h>
#include <qintdict.h>
#include <qstringlist.h>
#include <qthread.h>

/**
 * @short Simple server that impersonates as LyX to work with gBib, pyBibliographer etc.
 * @author Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
 * @version 0.1
 */

class QFile;
class QSocketNotifier;
namespace KileAction { class TagData; }

class KileLyxServer : public QObject
{
	Q_OBJECT

public:
	KileLyxServer(bool start = true);
	~KileLyxServer();

	bool isRunning() { return m_running; }

public slots:
	bool start();
	void stop();
	void processLine(const QString &);

private:
	void openPipes();
	void createPipes();
	void removePipes();

signals:
	void insert(const KileAction::TagData &);

private:
	QValueList<int>					m_fds;
	bool							m_running;
	QStringList						m_pipes;
	QPtrList<QFile>					m_pipeFile;
	QThread							*m_thread;
};

class LyxPipeThread : public QThread
{
public:
	LyxPipeThread(KileLyxServer *receiver, QValueList<int> fds);
	
protected:
	virtual void run();

private:
	KileLyxServer 	*m_receiver;
	QValueList<int>	m_fds;
};

#endif // _LYXSERVER_H_
