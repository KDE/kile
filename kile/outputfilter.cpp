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
#include <assert.h>

#include <kdebug.h>
#include "outputfilter.h"

using namespace std;

OutputFilter::OutputFilter( MessageWidget* LogWidget, MessageWidget* OutputWidget)
{
    m_log = 0L;
    m_LogWidget = LogWidget;
    m_OutputWidget = OutputWidget;
}

OutputFilter::~ OutputFilter()
{
}

short OutputFilter::ParseLine(QString strLine, short dwCookie)
{
    return 0;
}


bool OutputFilter::OnTerminate()
{
    return true;
}


unsigned int OutputFilter::Run(QString logfile)
{
	short sCookie = 0;
	QString s;
	QFile f(logfile);
	
	m_nOutputLines = 0;
	
	if ( f.open(IO_ReadOnly) )
	{
		QTextStream t( &f );
		while ( !t.eof() )
		{
			s=t.readLine()+"\n";
			sCookie = ParseLine(s,sCookie);
			m_nOutputLines++;
			if (m_log)  (*m_log) += s;
		}
		f.close();
	}
	
	return !OnTerminate();
}


int OutputFilter::GetCurrentOutputLine() const
{
    return m_nOutputLines;
}


/** Adds the specified line of text to the putput view.

Can be used in addition to add text to the output view (comments etc.) */
void OutputFilter::AddLine(QString line)
{
    if (m_OutputWidget)
        m_OutputWidget->append(line);
}
