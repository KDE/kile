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

#include "latexoutputfilter.h"

using namespace std;

LatexOutputFilter::LatexOutputFilter(   LatexOutputInfoArray* LatexOutputInfoArray ,
MessageWidget* LogWidget,
MessageWidget* OutputWidget ):
OutputFilter( LogWidget, OutputWidget ),
m_nErrors(0),
m_nWarnings(0),
m_nBadBoxes(0),
m_nOutputPages(0),
m_bFileNameOverLines(false),
m_InfoList(LatexOutputInfoArray)
{
}

LatexOutputFilter::~ LatexOutputFilter()
{
}

/**
Returns the number of output pages, that have been created by LaTeX.
*/
int LatexOutputFilter::GetNumberOfOutputPages() const
{
    return m_nOutputPages;
}


QString LatexOutputFilter::GetResultString()
{
    QString  strResult;
    //strResult.Format(STE_LATEX_RESULT, m_nErrors, m_nWarnings, m_nBadBoxes, m_nOutputPages);
    return strResult;
}


bool LatexOutputFilter::OnPreCreate()
{
    m_nErrors = 0;
    m_nWarnings = 0;
    m_nBadBoxes = 0;
    m_nOutputPages = 0;

    return true;
}


void LatexOutputFilter::UpdateFileStack(const QString &strLine)
{
    /*
    With the code in this function we catch non-file-related stuff as well.
    Consider the following output from TeX:

        Underfull \hbox (badness 3323) in paragraph at lines 87--88

    This pushes the wrong filename "badness 3323" on our filestack.
    But that does not matter. We will pop it before detecting the bad box
    in ParseLine - so we have the correct filename again (reported by TeX
    somewhat earlier).
    Lets call all braces "(...)", which are not related to files, "non-file-braces".

    For this function, we need the assumption, that TeX closes all non-file-braces
    before reporting an error; at least before we detect it in ParseLine.
    I did not see any other behaviour of TeX and I can not imagine it.
    So I think, this assumption is correct. The assertion	"ASSERT(!m_stackFile.empty());"
    below will help us to find out, whether I am right or not.
    */

    int i=0, pos=0;
    int j;
    int nStrMax = strLine.length();

    //If the last run of this func discovered a filename, that
    // was printed out over more than one line, we will directly
    // continue to read it in.
    if (m_bFileNameOverLines)
        goto ScanNextLine;

    for (; i < nStrMax; i++)
    {
        switch (strLine[i])
        {
            case '(':
            {
                ScanNextLine:
                for ( j = i+1;
                    ( j < nStrMax )
                    && ( strLine[j] != '[' )
                    && ( strLine[j] != ')' )
                    && ( strLine[j] != '(' )
                    && ( strLine[j] != '{' );
                    j++);

                /*
                We need to push even empty strings on the stack, because only this
                assures, that we get every starting and closing brace.
                Consider the following output from TeX (it is ONE! line of output):

                    \OT1/cmr/m/n/8 Punk-ten. $\OML/cmm/m/it/8 K[] \OT1/cmr/m/n/8 = ([]\OML/cmm/m/it
                    /8 ; []; []\OT1/cmr/m/n/8 )$. $\OML/cmm/m/it/8 K[] \OT1/cmr/m/n/8 =

                The first line has a starting brace '(' directly followed by a '['.
                This will produce an empty string (i.e. i == j-1) due to the code
                in the for-next-loop above. But we need the code above - no chance to change it.
                Certainly, adding empty strings, will somehow corrupt our file stack, but as mentioned
                above, we catch other non-file-related stuff as well.
                And the hope/knowledge is, that TeX closes the brace before reporting an error.
                */

                QString  strFile(strLine.mid(i+1, (j-1) - (i+1) + 1));

                if (m_bFileNameOverLines)
                {
                    strFile = m_strPartialFileName + strLine[0] + strFile;
                    m_bFileNameOverLines = false;
                }

                //Has the filename been broken up over two or even more lines?
                if ( (j == nStrMax) && (nStrMax >= 79)
                    && ((pos=strFile.findRev('.'))> 0 && strFile.length() - pos < 3 ) || pos < 0)
                {
                    //Yes - save it and wait for the next line to read
                    m_strPartialFileName = strFile;
                    m_bFileNameOverLines = true;
                    i = j-1;
                }
                else
                {
                    //No - push it
                    m_stackFile.push(strFile.stripWhiteSpace());
                    kdDebug() << "Push Filestack (" << m_stackFile.size() << ") " << strFile.stripWhiteSpace() << endl;
                    i = j-1;
                }
            }
            break;

            case ')':
                //If we are good in parsing, this ASSERT should not happen.
                //Comment this out, if it always asserts or try to find the error,
                //where we pop something from the stackFile, what should not be poped.
                //
                // Yeah, well, I found a case:
                // Mostly TeX prints out some parts of the processed tex-file after reporting a bad box.
                // And sometimes not! But if this part contains a closing brace ')', we will get
                // an assertion here somewhat after a wrong poping caused by this ')'. Any Idea?
                assert(!m_stackFile.empty());
                if (!m_stackFile.empty())
                {
                    m_stackFile.pop();
                    kdDebug() << "Pop Filestack (" << m_stackFile.size() << ") " << m_stackFile.top()  << endl;
                }
                break;
        }
    }
}


void LatexOutputFilter::FlushCurrentItem()
{
    int nItemType = m_currentItem.type();
    found=true;

    switch (nItemType)
    {
        case itmError:
            m_nErrors++;
            m_InfoList->push_back(m_currentItem);
            kdDebug() << "Flushing Error in" << m_currentItem.source()
                << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() <<  endl;
            break;
        case itmWarning:
            m_nWarnings++;
            m_InfoList->push_back(m_currentItem);
            kdDebug() << "Flushing Warning in " << m_currentItem.source()
                << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() << endl;
            break;
        case itmBadBox:
            m_nBadBoxes++;
            m_InfoList->push_back(m_currentItem);
            kdDebug() << "Flushing BadBox in " << m_currentItem.source()
                << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() << endl;
            break;

        default:                                                                //no important item -> do nothing
            ;
    }

    m_currentItem.Clear();
}


short LatexOutputFilter::ParseLine(QString strLine, short dwCookie)
{
    UpdateFileStack(strLine);
    found=false;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // parse for new error, warning or bad box

    //Catching Errors
    static QRegExp::QRegExp  error1("^! LaTeX Error: (.*)$", true);
    static QRegExp::QRegExp  error2("^! (.*)$", true);                          // This could catch warnings, so run it last

    //Catching Warnings
    static QRegExp::QRegExp  warning1("^(! )?(La|pdf)TeX .*(W|w)arning.*: (.*)", true);
    static QRegExp::QRegExp  warning2("^LaTeX .*warning: (.*) on input line ([0-9]+)", true);
    static QRegExp::QRegExp  warning3(".*(W|w)arning.*: (.*)", true);           //Catches package warnings

    //Catching Bad Boxes
    static QRegExp::QRegExp  badBox1("^(Over|Under)full \\\\[hv]box .*at lines ([0-9]+)--([0-9]+)", true);
    //Use the following only, if you know how to get the source line for it.
    // This is not simple, as TeX is not reporting it.
    //static QRegExp::QRegExp	badBox2("^(Over|Under)full \\\\[hv]box .* has occurred while \\output is active", true);

    //Catching the source line numbers of error/warning types
    // which are reported over more than one line in the output
    static QRegExp::QRegExp  line1("l\\.([0-9]+)", true);
    static QRegExp::QRegExp  line2("line ([0-9]+)", true);

    //Catching the number of output pages
    static QRegExp::QRegExp  output1("Output written on .* \\((\\d*) page.*\\)", true);
    static QRegExp::QRegExp  output2("No pages of output", true);

    /* ABOUT THE ORDER OF SEARCHING THE OUTPUT

            Every TeX-User needs to correct an error, if he wants to have a valid document.
            Therefore, errors are quite rare.
            For some warnings, but not all, there is also a need to correct them - to get a better document.
            Therefore, warnings may occure more often than errors.
            Most users are not going to correct a bad box. Sometimes it is not possible at all (at least
            for the average TeX-user).
            Therefore, bad boxes are the most common type of catched entities.

            To speed up parsing the output, it is preferable to look for the common stuff first.
    ==> 1. bad boxes; 2. warnings; 3. errors; 4. srcline-numbers; 5. other stuff

    */
    if (badBox1.search(strLine) > -1)
    {
        FlushCurrentItem();

        //Get the srcline for it.
        // - Mostly TeX reports something like "100--103" ==> so it is the first one.
        // - Sometimes TeX reports "167--47". This comes up, if an input
        // - file was just closed. The first number (n1) means the last line of the input file,
        // - whereas the second number (n2) means the line after the \input-command in the
        // - "master"-file. Certainly there is no need, that we always have (n1 > n2),
        // - but we will not catch the case (n1 < n2) correctly anyway, because we do not
        // - have enough information about "closing files by TeX" here.
        // -
        // - So I make the assumption, that the last line of the input file is very likely
        // - to be greater than the line of the \input-command.
        // - ==> Take min(n1,n2) as the srcline. This will do a good job for more cases than
        // - just taking the first number. But the problem still remains for some (rare) cases.
        // -
        int n1 = badBox1.cap(2).toInt();
        int n2 = badBox1.cap(3).toInt();

        m_currentItem = LatexOutputInfo(
            m_stackFile.empty()? "" : m_stackFile.top(),
            (n1 < n2) ? n1 : n2,
            GetCurrentOutputLine(),
            badBox1.cap(1),
            itmBadBox);
    }
    else if (warning2.search(strLine) > -1)
    {
        FlushCurrentItem();
        m_currentItem = LatexOutputInfo(
            m_stackFile.empty()? "" : m_stackFile.top(),
            warning2.cap(2).toInt(),
            GetCurrentOutputLine(),
            warning2.cap(1),
            itmWarning);
    }
    else if (warning1.search(strLine) > -1)
    {
        FlushCurrentItem();
        m_currentItem = LatexOutputInfo(
            m_stackFile.empty()? "" : m_stackFile.top(),
            0,
            GetCurrentOutputLine(),
            warning3.cap(1),
            itmWarning);
    }
    else if (warning3.search(strLine) > -1)
    {
        FlushCurrentItem();
        m_currentItem = LatexOutputInfo(
            m_stackFile.empty()? "" : m_stackFile.top(),
            0,
            GetCurrentOutputLine(),
            warning3.cap(1),
            itmWarning);
    }
    else if (error1.search(strLine) > -1)
    {
        FlushCurrentItem();
        m_currentItem = LatexOutputInfo(
            m_stackFile.empty()? "" : m_stackFile.top(),
            0,
            GetCurrentOutputLine(),
            error1.cap(1),
            itmError);
    }
    else if (error2.search(strLine) > -1)
    {
        FlushCurrentItem();
        m_currentItem = LatexOutputInfo(
            m_stackFile.empty()? "" : m_stackFile.top(),
            0,
            GetCurrentOutputLine(),
            error2.cap(1),
            itmError);
    }
    else if (line1.search(strLine) > -1)
    {
        //Catching the line number of some errors/warnings types, which do not
        // report their place of occurence in their first line. Consider the
        // following example (catched by line1):
        //
        //! Undefined control sequence.
        //<argument> \pa
        //               rskip
        //l.2 \setlength{\pa rskip}{0.5ex}
        if (m_currentItem.sourceLine() == 0)
        {
            m_currentItem.setSourceLine(line1.cap(1).toInt());
        }
    }
    else if (line2.search(strLine) > -1)
    {
        //Only change the SrcLine, if it could not been assigned yet (== 0), !!!
        // because TeX or a TeX-Package is reporting the errorneus input line
        // one line after reporting the error itself. Consider the following
        // example (catched by line2):
        //
        //Package amsmath Warning: Cannot use `split' here;
        //(amsmath)                trying to recover with `aligned' on input line 65.
        //
        //But: Do not change an Item, if the SrcLine could be retrieved from the output!!!
        if (m_currentItem.sourceLine() == 0)
        {
            m_currentItem.setSourceLine(line2.cap(1).toInt());
        }
    }
    else if (output1.search(strLine) > -1)
    {
        //LaTeX said 'Output written on _file_ (%m_nOutputPages% pages, _number_ bytes)'
        m_nOutputPages = output1.cap(1).toInt();
    }
    else if (output2.search(strLine) > -1)
    {
        //LaTeX said 'No pages of output'
        m_nOutputPages = 0;
    }

    return dwCookie;
}


bool LatexOutputFilter::OnTerminate()
{
    FlushCurrentItem();

    return m_nErrors == 0;
}


unsigned int LatexOutputFilter::Run(QString logfile)
{
    m_InfoList->clear();
    m_nErrors = m_nWarnings = m_nBadBoxes = 0;
    unsigned int result = OutputFilter::Run(logfile);

    if (m_LogWidget)
    {
		QString Message;
		QString color;

		//print detailed error info
		for (uint i=0; i < m_InfoList->count() ; i++)
		{
			Message = QString("%1:%2:%3").arg((*m_InfoList)[i].source()).arg((*m_InfoList)[i].sourceLine()).arg((*m_InfoList)[i].message());
			switch ( (*m_InfoList)[i].type()  )
			{
				case LatexOutputInfo::itmBadBox	: color = "black"; break;
				case LatexOutputInfo::itmError		: color = "red"; break;
				case LatexOutputInfo::itmWarning	: color = "blue"; break;
				default : color="black";break;
			}
			Message = "<font color="+color+">"+Message+"</font>";
			m_LogWidget->append(Message);
		}
    }
    return result;
}


/** Return number of errors etc. found in log-file. */
void LatexOutputFilter::GetErrorCount(int *errors, int *warnings, int *badboxes)
{
    *errors = m_nErrors;
    *warnings = m_nWarnings;
    *badboxes = m_nBadBoxes;
}
