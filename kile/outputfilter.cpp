/***************************************************************************
                          latexoutputfilter.cpp  -  description
                             -------------------
    begin                : Die Sep 16 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfile.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <ktextedit.h>
#include <klocale.h>

#include "kiletool_enums.h"
#include "outputfilter.h"

using namespace std;

OutputFilter::OutputFilter() :
	m_log(QString::null)
{
}

OutputFilter::~ OutputFilter()
{
}

short OutputFilter::parseLine(const QString & /*strLine*/, short /*dwCookie*/)
{
    return 0;
}


bool OutputFilter::OnTerminate()
{
    return true;
}

void OutputFilter::setSource(const QString &src)
{
	m_source = src;
	m_srcPath = QFileInfo(src).dirPath();
}

bool OutputFilter::Run(QString logfile)
{
	short sCookie = 0;
	QString s;
	QFile f(logfile);

	m_log = QString::null;
	m_nOutputLines = 0;

	if ( f.open(IO_ReadOnly) )
	{
		QTextStream t( &f );
		while ( !t.eof() )
		{
			s=t.readLine()+"\n";
			sCookie = parseLine(s.stripWhiteSpace(), sCookie);
			m_nOutputLines++;

			m_log += s;
		}
		f.close();
	}
	else
	{
		emit(problem(KileTool::Warning, i18n("Cannot open log file! Did you run LaTeX?")));
		return false;
	}

	return OnTerminate();
}


int OutputFilter::GetCurrentOutputLine() const
{
    return m_nOutputLines;
}

#include "outputfilter.moc"
