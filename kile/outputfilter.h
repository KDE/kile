/***************************************************************************
                          latexoutputfilter.h  -  description
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

#ifndef OUTPUTFILTER_H
#define OUTPUTFILTER_H

#include <qvaluestack.h>
#include <qstring.h>
#include <qwidget.h>

#include "outputinfo.h"
#include "messagewidget.h"

/**An object of this class is used to parse the output messages
of any third-party tool.

@author Thorsten Lück
  *@author Jeroen Wijnhout
  */

class OutputFilter
{
    public:
        OutputFilter(MessageWidget *LogWidget = NULL, MessageWidget* OutputWidget = NULL);
        virtual ~OutputFilter();

    protected:

    public:
        virtual unsigned int Run(QString logfile);
        /** Adds the specified line of text to the putput view.

        Can be used in addition to add text to the output view (comments etc.) */
        void AddLine(QString line);

    protected:
        virtual short ParseLine(QString strLine, short dwCookie);
        virtual bool OnTerminate();
        /**
        Returns the zero based index of the currently parsed line in the
        output file.
        */
        int GetCurrentOutputLine() const;

    private:

        // types
    protected:

        // attributes
    private:
        /** Number of current line in output file */
        unsigned int m_nOutputLines;

    protected:
        /** the view, the errors and warnings are reported to */
        MessageWidget *m_LogWidget;

        /** the view, where the logfile is printed to */
        MessageWidget *m_OutputWidget;
};
#endif
