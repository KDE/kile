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

/**
 * @short Simple server that impersonates as LyX to work with gBib, pyBibliographer etc.
 * @author Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
 * @version 0.1
 */

class QFile;
class QSocketNotifier;

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

protected slots:
	void receive(int);
	bool openPipes();

signals:
	void insertCite(const QString&);
	void insertBibTeX(const QString&);
	void insertBibTeXDatabaseAdd(const QString&);

private:
	QPtrList<QFile>					m_pipeIn;
	QPtrList<QSocketNotifier>	m_notifier;
	QIntDict<QFile>					m_file;
	bool										m_running;
	int											m_count;
};

#endif // _LYXSERVER_H_
