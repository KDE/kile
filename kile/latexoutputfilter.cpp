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

#include "latexoutputfilter.h"
#include "kiletool_enums.h"

using namespace std;

LatexOutputFilter::LatexOutputFilter(LatexOutputInfoArray* LatexOutputInfoArray) :
	m_nErrors(0),
	m_nWarnings(0),
	m_nBadBoxes(0),
	m_InfoList(LatexOutputInfoArray)
{
}

LatexOutputFilter::~ LatexOutputFilter()
{
}

bool LatexOutputFilter::OnPreCreate()
{
	m_nErrors = 0;
	m_nWarnings = 0;
	m_nBadBoxes = 0;

	return true;
}


bool LatexOutputFilter::fileExists(const QString & name)
{
	static QFileInfo::QFileInfo fi;

	if (name[0] == '/' )
	{
		fi.setFile(name);
		if ( fi.exists() && !fi.isDir() ) return true;
		else return false;
	}

	fi.setFile(path() + "/" + name);
	if ( fi.exists() && !fi.isDir() ) return true;

	fi.setFile(path() + "/" + name + ".tex");
	if ( fi.exists() && !fi.isDir() ) return true;

	return false;
}

// There are basically two ways to detect the current file TeX is processing:
//	1) Use \Input (i.c.w. srctex.sty or srcltx.sty) and \include exclusively. This will
//	cause (La)TeX to print the line ":<+ filename"  in the log file when opening a file, 
//	":<-" when closing a file. Filenames pushed on the stack in this mode are marked
//	as reliable.
//
//	2) Since people will probably also use the \input command, we also have to be
//	to detect the old-fashioned way. TeX prints '(filename' when opening a file and a ')'
//	when closing one. It is impossible to detect this with 100% certainty (TeX prints many messages
//	and even text (a context) from the TeX source file, there could be unbalanced parentheses), 
//	so we use a heuristic algorithm. In heuristic mode a ')' will only be considered as a signal that 
//	TeX is closing a file if the top of the stack is not marked as "reliable". 
//	Also, when scanning for a TeX error linenumber (which sometimes causes a context to be printed
//	to the log-file), updateFileStack is not called, helping not to pick up unbalanced parentheses
//	from the context.
void LatexOutputFilter::updateFileStack(const QString &strLine, short & dwCookie)
{
	kdDebug() << "==LatexOutputFilter::updateFileStack()================" << endl;

	static QString::QString strPartialFileName;

	switch (dwCookie)
	{
		//we're looking for a filename
		case Start : case FileNameHeuristic :
			//TeX is opening a file
			if ( strLine.startsWith(":<+ ") )
			{
				kdDebug() << "filename detected" << endl;
				//grab the filename, it might be a partial name (i.e. continued on the next line)
				strPartialFileName = strLine.mid(4).stripWhiteSpace();

				//change the cookie so we remember we aren't sure the filename is complete
				dwCookie = FileName;
			}
			//TeX closed a file
			else if ( strLine.startsWith(":<-") )
			{
				kdDebug() << "\tpopping : " << m_stackFile.top().file() << endl;
				m_stackFile.pop();
				dwCookie = Start;
			}
			else
			//fallback to the heuristic detection of filenames
				updateFileStackHeuristic(strLine, dwCookie);
		break;

		case FileName :
			//The partial filename was followed by '(', this means that TeX is signalling it is
			//opening the file. We are sure the filename is complete now. Don't call updateFileStackHeuristic
			//since we don't want the filename on the stack twice.
			if ( strLine.startsWith("(") || strLine.startsWith("\\openout") )
			{
				//push the filename on the stack and mark it as 'reliable'
				m_stackFile.push(LOFStackItem(strPartialFileName, true));
				kdDebug() << "\tpushed : " << strPartialFileName << endl;
				strPartialFileName = QString::null;
				dwCookie = Start;
			}
			//The partial filename was followed by an TeX error, meaning the file doesn't exist.
			//Don't push it on the stack, instead try to detect the error.
			else if ( strLine.startsWith("!") )
			{
				kdDebug() << "oops!" << endl;
				dwCookie = Start;
				strPartialFileName = QString::null;
				detectError(strLine, dwCookie);
			}
			//Partial filename still isn't complete.
			else
			{
				kdDebug() << "\tpartial file name, adding" << endl;
				strPartialFileName = strPartialFileName + strLine.stripWhiteSpace();
			}
		break;

		default : break;
	}
}

void LatexOutputFilter::updateFileStackHeuristic(const QString &strLine, short & dwCookie)
{
	kdDebug() << "==LatexOutputFilter::updateFileStackHeuristic()================" << endl;

	static QString::QString strPartialFileName;
	bool expectFileName = (dwCookie == FileNameHeuristic);
	int index = 0;

	//scan for parentheses and grab filenames
	for ( uint i = 0; i < strLine.length(); i ++)
	{
		//We're expecting a filename. If a filename really ends at this position one of the following must be true:
		//	1) Next character is a space (indicating the end of a filename (yes, there can't spaces in the
		//	path, this is a TeX limitation).
		//	2) We're at the end of the line, the filename is probably continued on the next line.
		//	3) The TeX was closed already, signalled by the ')'.
		if ( expectFileName && ( strLine[i+1].isSpace() || i +1  == strLine.length() || strLine[i+1] == ')' ))
		{
			//update the partial filename
			strPartialFileName =  strPartialFileName + strLine.mid(index, i-index + 1);

			//FIXME: improve these heuristics
			if ( 	strLine[i+1].isSpace() || ( (i < 78) && (i +1  == strLine.length()) ) ||
				strLine[i+1] == ')' ||
				fileExists(strPartialFileName) )
			{
				m_stackFile.push(LOFStackItem(strPartialFileName));
				kdDebug() << "\tpushed (i = " << i << " length = " << strLine.length() << "): " << strPartialFileName << endl;
				expectFileName = false;
				dwCookie = Start;
			}
			//Guess the filename is continued on the next line.
			else if ( i+1 == strLine.length() )
			{
				kdDebug() << "\tFilename spans more than one line." << endl;
				dwCookie = FileNameHeuristic;
			}
			//bail out
			else
			{
				dwCookie = Start;
				strPartialFileName = QString::null;
				expectFileName = false;
			}
		}
		//TeX is opening a file
		else if ( strLine[i] == '(' )
		{
			//we need to extract the filename
			expectFileName = true;
			strPartialFileName = QString::null;
			dwCookie = Start;

			//this is were the filename is supposed to start
			index = i + 1;
		}
		//TeX is closing a file
		else if ( strLine[i] == ')' )
		{
			kdDebug() << "\tpopping : " << m_stackFile.top().file() << endl;
			//If this filename was pushed on the stack by the reliable ":<+-" method, don't pop
			//a ":<-" will follow. This helps in preventing unbalanced ')' from popping filenames
			//from the stack too soon.
			if ( ! m_stackFile.top().reliable() )
				m_stackFile.pop();
			else
				kdDebug() << "\t\toh no, forget about it!" << endl;
		}
	}
}


void LatexOutputFilter::flushCurrentItem()
{
	kdDebug() << "==LatexOutputFilter::flushCurrentItem()================" << endl;
	int nItemType = m_currentItem.type();

	while (  (! fileExists(m_stackFile.top().file()) ) && (m_stackFile.count() > 1) )
		m_stackFile.pop();

	m_currentItem.setSource(m_stackFile.top().file());

	switch (nItemType)
	{
		case itmError:
			m_nErrors++;
			m_InfoList->push_back(m_currentItem);
			kdDebug() << "Flushing Error in" << m_currentItem.source() << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() <<  endl;
		break;

		case itmWarning:
			m_nWarnings++;
			m_InfoList->push_back(m_currentItem);
			kdDebug() << "Flushing Warning in " << m_currentItem.source() << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() << endl;
		break;

		case itmBadBox:
			m_nBadBoxes++;
			m_InfoList->push_back(m_currentItem);
			kdDebug() << "Flushing BadBox in " << m_currentItem.source() << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() << endl;
		break;

		default: break;
	}
	m_currentItem.Clear();
}

bool LatexOutputFilter::detectError(const QString & strLine, short &dwCookie)
{
	kdDebug() << "==LatexOutputFilter::detectError(" << strLine.length() << ")================" << endl;

	bool found = false, flush = false;

	static QRegExp::QRegExp reLaTeXError("^! LaTeX Error: (.*)$", false);
	static QRegExp::QRegExp rePDFLaTeXError("^Error: pdflatex (.*)$", false);
	static QRegExp::QRegExp reTeXError("^! (.*)");
	static QRegExp::QRegExp reLineNumber("^l\\.([0-9]+)(.*)");

	switch (dwCookie)
	{
		case Start :
			if ( (reLaTeXError.search(strLine) != -1) || (rePDFLaTeXError.search(strLine) != -1) )
			{
				kdDebug() << "\tError : " <<  reLaTeXError.cap(1) << endl;
				m_currentItem.setMessage(reLaTeXError.cap(1));
				found = true;
				dwCookie = strLine.endsWith(".") ? LineNumber : Error;
			}
			else if (reTeXError.search(strLine) != -1 )
			{
				kdDebug() << "\tError : " <<  reTeXError.cap(1) << endl;
				m_currentItem.setMessage(reTeXError.cap(1));
				found = true;
				dwCookie = strLine.endsWith(".") ? LineNumber : Error;
			}
		break;

		case Error :
			kdDebug() << "\tError (cont'd): " << strLine << endl;
			if ( strLine.endsWith(".") )
			{
				dwCookie = LineNumber;
				m_currentItem.setMessage(m_currentItem.message() + strLine);
			}
			else if ( GetCurrentOutputLine() - m_currentItem.outputLine() > 3 )
			{
				kdWarning() << "\tBAILING OUT: error description spans more than three lines" << endl;
				dwCookie = Start;
				flush = true;
			}
		break;

		case LineNumber :
			kdDebug() << "\tLineNumber " << endl;
			if ( reLineNumber.search(strLine) != -1 )
			{
				dwCookie = Start;
				flush = true;
				kdDebug() << "\tline number: " << reLineNumber.cap(1) << endl;
				m_currentItem.setSourceLine(reLineNumber.cap(1).toInt());
				m_currentItem.setMessage(m_currentItem.message() + reLineNumber.cap(2));
			}
			else if ( GetCurrentOutputLine() - m_currentItem.outputLine() > 10 )
			{
				dwCookie = Start;
				flush = true;
				kdWarning() << "\tBAILING OUT: did not detect a TeX line number for an error" << endl;
				m_currentItem.setSourceLine(0);
			}
		break;

		default : break;
	}

	if (found)
	{
		m_currentItem.setType(itmError);
		m_currentItem.setOutputLine(GetCurrentOutputLine());
	}

	if (flush) flushCurrentItem();

	return found;
}

bool LatexOutputFilter::detectWarning(const QString & strLine, short &dwCookie)
{
	kdDebug() << "==LatexOutputFilter::detectWarning(" << strLine.length() << ")================" << endl;

	bool found = false, flush = false;
	QString warning;

	static QRegExp::QRegExp reLaTeXWarning("^(! )?(La|pdf)TeX .*Warning.*:(.*)", false);
	//static QRegExp::QRegExp warning3(".*warning.*: (.*)", false);  

	switch (dwCookie)
	{
		//detect the beginning of a warning
		case Start :
			if ( reLaTeXWarning.search(strLine) != -1 )
			{
				warning = reLaTeXWarning.cap(3);
				kdDebug() << "\tWarning found: " << warning << endl;

				found = true;
				dwCookie = Start;

				//do we expect a line number?
				flush = detectLaTeXLineNumber(warning, dwCookie);

				m_currentItem.setMessage(warning);
			}
		break;

		//warning spans multiple lines, detect the end
		case Warning :
			warning = m_currentItem.message() + strLine;
			kdDebug() << "'\tWarning (cont'd) : " << warning << endl;
			flush = detectLaTeXLineNumber(warning, dwCookie);
			m_currentItem.setMessage(warning);
		break;

		default : break;
	}

	if ( found )
	{
		m_currentItem.setType(itmWarning);
		m_currentItem.setOutputLine(GetCurrentOutputLine());
	}

	if (flush) flushCurrentItem();

	return found;
}

bool LatexOutputFilter::detectLaTeXLineNumber(QString & warning, short & dwCookie)
{
	kdDebug() << "==LatexOutputFilter::detectLaTeXLineNumber(" << warning.length() << ")================" << endl;

	static QRegExp::QRegExp reLaTeXLineNumber("(.*) on input line ([0-9]+)\\.$", false);
	if ( reLaTeXLineNumber.search(warning) != -1 )
	{
		m_currentItem.setSourceLine(reLaTeXLineNumber.cap(2).toInt());
		warning = reLaTeXLineNumber.cap(1);
		dwCookie = Start;
		return true;
	}
	else if ( warning.endsWith(".") )
	{
		m_currentItem.setSourceLine(0);
		dwCookie = Start;
		return true;
	}
	//error message is continued on the other line
	else
	{
		dwCookie = Warning;
		return false;
	}
}

bool LatexOutputFilter::detectBadBox(const QString & strLine, short & dwCookie)
{
	kdDebug() << "==LatexOutputFilter::detectBadBox(" << strLine.length() << ")================" << endl;

	bool found = false, flush = false;
	QString badbox;

	static QRegExp::QRegExp reBadBox("^(Over|Under)(full \\\\[hv]box .*)", false);

	switch (dwCookie)
	{
		case Start :
			if ( reBadBox.search(strLine) != -1)
			{
				found = true;
				dwCookie = Start;
				badbox = strLine;
				flush = detectBadBoxLineNumber(badbox, dwCookie);
				m_currentItem.setMessage(badbox);
			}
		break;

		case BadBox :
			badbox = m_currentItem.message() + strLine;
			flush = detectBadBoxLineNumber(badbox, dwCookie);
			m_currentItem.setMessage(badbox);
		break;

		default : break;
	}

	if ( found )
	{
		m_currentItem.setType(itmBadBox);
		m_currentItem.setOutputLine(GetCurrentOutputLine());
	}

	if (flush) flushCurrentItem();

	return found;
}

bool LatexOutputFilter::detectBadBoxLineNumber(QString & strLine, short & dwCookie)
{
	kdDebug() << "==LatexOutputFilter::detectBadBoxLineNumber(" << strLine.length() << ")================" << endl;

	static QRegExp::QRegExp reBadBoxLines("(.*) at lines ([0-9]+)--([0-9]+)", false);
	static QRegExp::QRegExp reBadBoxLine("(.*) at line ([0-9]+)", false);
	//Use the following only, if you know how to get the source line for it.
	// This is not simple, as TeX is not reporting it.
	static QRegExp::QRegExp reBadBoxOutput("(.*)has occurred while \\output is active^", false);

	if ( reBadBoxLines.search(strLine) != -1)
	{
		dwCookie = Start;
		strLine = reBadBoxLines.cap(1);
		int n1 = reBadBoxLines.cap(2).toInt();
		int n2 = reBadBoxLines.cap(3).toInt();
		m_currentItem.setSourceLine(n1 < n2 ? n1 : n2);
		return true;
	}
	else if ( reBadBoxLine.search(strLine) != -1)
	{
		dwCookie = Start;
		strLine = reBadBoxLine.cap(1);
		m_currentItem.setSourceLine(reBadBoxLine.cap(2).toInt());
		kdDebug() << "\tBadBox@" << reBadBoxLine.cap(2) << "." << endl;
		return true;
	}
	else if ( reBadBoxOutput.search(strLine) != -1)
	{
		dwCookie = Start;
		strLine = reBadBoxLines.cap(1);
		m_currentItem.setSourceLine(0);
		return true;
	}
	else if ( GetCurrentOutputLine() - m_currentItem.outputLine() > 3 )
	{
		dwCookie = Start;
		m_currentItem.setSourceLine(reBadBoxLines.cap(2).toInt());
		return true;
	}
	else
	{
		dwCookie = BadBox;
	}

	return false;
}

short LatexOutputFilter::parseLine(const QString & strLine, short dwCookie)
{
	kdDebug() << "==LatexOutputFilter::parseLine(" << strLine.length() << ")================" << endl;

	switch (dwCookie)
	{
		case Start :
			updateFileStack(strLine, dwCookie);
			(detectBadBox(strLine, dwCookie) || detectWarning(strLine, dwCookie) || detectError(strLine, dwCookie));
		break;

		case Warning :
			detectWarning(strLine, dwCookie);
		break;

		case Error : case LineNumber :
			detectError(strLine, dwCookie);
		break;

		case BadBox :
			detectBadBox(strLine, dwCookie);
		break;

		case FileName : case FileNameHeuristic :
			updateFileStack(strLine, dwCookie);
		break;

		default: dwCookie = Start; break;
	}

	return dwCookie;
}

bool LatexOutputFilter::Run(QString logfile)
{
	m_InfoList->clear();
	m_nErrors = m_nWarnings = m_nBadBoxes = m_nParens = 0;
	m_stackFile.clear();
	m_stackFile.push(LOFStackItem(QFileInfo(source()).fileName(), true));

	bool result = OutputFilter::Run(logfile);

	QString Message;
	int type;

	//print detailed error info
	for (uint i=0; i < m_InfoList->count() ; i++)
	{
		Message = QString("%1:%2:%3").arg((*m_InfoList)[i].source()).arg((*m_InfoList)[i].sourceLine()).arg((*m_InfoList)[i].message());
		switch ( (*m_InfoList)[i].type()  )
		{
			case LatexOutputInfo::itmBadBox	: type = KileTool::Info; break;
			case LatexOutputInfo::itmError	: type = KileTool::Error; break;
			case LatexOutputInfo::itmWarning	: type = KileTool::Warning; break;
			default : type = KileTool::Info; break;
		}
		emit(problem(type, Message));
		if ( i > 25 )
		{
			emit(problem(KileTool::Error, i18n("More than 25 problems detected, stopping output.")));
			break;
		}
	}

    return result;
}


/** Return number of errors etc. found in log-file. */
void LatexOutputFilter::getErrorCount(int *errors, int *warnings, int *badboxes)
{
    *errors = m_nErrors;
    *warnings = m_nWarnings;
    *badboxes = m_nBadBoxes;
}
