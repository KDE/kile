/***************************************************************************
                          kiletoolcapability.cpp  -  description
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

#include <kdebug.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kstddirs.h>

#include "kiletoolmanager.h"
#include "kiletoolcapability.h"

namespace KileTool
{
	Capability::Capability(const QString & name, const QString & command, const QString & testfile /*= QString::null*/) : 
		m_name(name),
		m_command(command)
	{
		if ( testfile != QString::null )
			m_command.replace("%file", KGlobal::dirs()->findResource("appdata","test/" + testfile));
	}

	void Capability::setResult(bool result)
	{
		setCapability(name(), result);
	}

	SrcSpecialCapability::SrcSpecialCapability() : Capability("LaTeXSrcSpecials", "latex -src %file", "test.tex")
	{}

	void SrcSpecialCapability::setResult(bool result)
	{
		QStringList tools; tools << "LaTeX" << "TeX" << "PDFLaTeX" << "PDFTeX";

		for ( uint i = 0; i < tools.count(); i ++)
		{
			if ( configName(tools[i], KGlobal::config()) == "Default" )
				setConfigName(tools[i], "Modern", KGlobal::config());
		}

		Capability::setResult(result);
	}

	CapabilityTester::CapabilityTester(Capability *capa) : m_capability(capa)
	{
		m_process = new KShellProcess();

		connect(m_process, SIGNAL(processExited(KProcess* )), this, SLOT(done(KProcess* )));
		connect(this, SIGNAL(finished(bool)), m_capability, SLOT(setResult(bool)));

		*m_process << " ( " +  m_capability->command()  + " ) " << " 2>/dev/null >/dev/null && echo ok || echo failed";
	}

	CapabilityTester::~CapabilityTester()
	{
		delete m_capability;
	}

	void CapabilityTester::done(KProcess *process)
	{
		bool result = false;
		if ( process->normalExit() )
			result = (process->exitStatus() == 0);

		emit finished(result);

		deleteLater();
	}

	void CapabilityTester::startTest()
	{
		if (!m_process->start(KProcess::NotifyOnExit))
			emit finished(false);
	}
}

#include "kiletoolcapability.moc"
