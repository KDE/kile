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
    char c;
    short BytesRead;
    QString strLine;
    short sCookie = 0;
    bool bLastWasNewLine = false;
    QFile m_if;

    m_nOutputLines = 0;
    m_if.setName( logfile );
    m_if.open( IO_ReadOnly  );

    while(!m_if.atEnd() )
    {
        switch( c = m_if.getch() )
        {
            case '\n':
            case '\r':
                if (!bLastWasNewLine)
                {
                    bLastWasNewLine = true;
                    AddLine( strLine );
                    sCookie = ParseLine(strLine,sCookie);
                    strLine = "";
                }
                m_nOutputLines++;
                break;

            default:
                bLastWasNewLine = false;
                strLine += c;
                break;
        }
    }

    if (strLine.length() > 0)
    {
        //AddLine( strLine );
        ParseLine(strLine, sCookie);
    }

    m_if.close();

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
