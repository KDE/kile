/***************************************************************************
                          kiletoolcapability.h  -  description
                             -------------------
    begin                : Sat Apr 3 2004
    copyright            : (C) 2004 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#ifndef KILETOOLCAPABILITY_H
#define KILETOOLCAPABILITY_H

#include <qobject.h>
#include <qstring.h>

class KProcess;
class KShellProcess;

namespace KileTool
{
	class Capability : public QObject
	{
		Q_OBJECT

	public:
		Capability(const QString &name, const QString & command, const QString & testfile = QString::null);
		const QString & command() const { return m_command; }
		const QString & name() const { return m_name; }

	public slots:
		virtual void setResult(bool);

	private:
		QString	m_name;
		QString	m_command;
	};

	class SrcSpecialCapability : public Capability
	{
		Q_OBJECT

	public:
		SrcSpecialCapability();

	public slots:
		void setResult(bool);
	};

	class CapabilityTester : public QObject
	{
		Q_OBJECT

	public:
		CapabilityTester(Capability *capa);
		~CapabilityTester();

		void startTest();

	signals:
		void finished(bool);

	private slots:
		void done(KProcess *);

	private:
		KShellProcess		*m_process;
		Capability			*m_capability;
	};
}

#endif
