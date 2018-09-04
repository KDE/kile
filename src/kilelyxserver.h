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

#ifndef _LYXSERVER_H_
#define _LYXSERVER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <QHash>
#include <QList>
#include <QObject>
#include <QStringList>

#include <QTemporaryDir>

#include <sys/types.h>

#ifdef _MSC_VER
typedef int mode_t;
#endif

/**
 * @short Simple server that impersonates as LyX to work with gBib, pyBibliographer etc.
 * @author Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
 * @author Thomas Braun
 * @version 0.2
 */

class QFile;
class QSocketNotifier;
namespace KileAction {
class TagData;
}

class KileLyxServer : public QObject
{
    Q_OBJECT

public:
    explicit KileLyxServer(bool start = true);
    ~KileLyxServer();

    bool isRunning() {
        return m_running;
    }

public Q_SLOTS:
    bool start();
    void stop();

private Q_SLOTS:
    void receive(int);

private:
    bool openPipes();
    void removePipes();
    void processLine(const QString &);

Q_SIGNALS:
    void insert(const KileAction::TagData &);

private:
    mode_t 				m_perms;
    QTemporaryDir			*m_tempDir;
    QList<QFile*>			m_pipeIn;
    QList<QSocketNotifier*>		m_notifier;
    QHash<int, QFile*>		m_file;
    bool				m_running;
    QStringList 			m_links,m_pipes;
};

#endif // _LYXSERVER_H_
