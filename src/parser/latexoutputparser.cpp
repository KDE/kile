/**********************************************************************************
*   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)           *
*                 2011 by Michel Ludwig (michel.ludwig@kdemail.net)               *
***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "latexoutputparser.h"

#include <QDir>
#include <QFileInfo>

#include <KLocalizedString>

#include "kiletool_enums.h"
#include "parserthread.h"
#include "widgets/logwidget.h"

namespace KileParser {

LaTeXOutputParserInput::LaTeXOutputParserInput(const QUrl &url, KileDocument::Extensions *extensions,
                                                                const QString& sourceFile,
                                                                const QString &texfilename,
                                                                int selrow,
                                                                int docrow)
    : ParserInput(url),
      extensions(extensions),
      sourceFile(sourceFile),
      texfilename(texfilename),
      selrow(selrow),
      docrow(docrow)
{
}

LaTeXOutputParserOutput::LaTeXOutputParserOutput()
{
}

LaTeXOutputParserOutput::~LaTeXOutputParserOutput()
{
    qCDebug(LOG_KILE_PARSER);
}

LaTeXOutputParser::LaTeXOutputParser(ParserThread *parserThread, LaTeXOutputParserInput *input, QObject *parent)
    : Parser(parserThread, parent),
      m_extensions(input->extensions),
      m_infoList(Q_NULLPTR),
      m_logFile(input->url.toLocalFile()),
      texfilename(input->texfilename),
      selrow(input->selrow),
      docrow(input->docrow)
{
    m_nErrors = 0;
    m_nWarnings = 0;
    m_nBadBoxes = 0;
    setSource(input->sourceFile);
}

LaTeXOutputParser::~LaTeXOutputParser()
{
    qCDebug(LOG_KILE_PARSER);
}

bool LaTeXOutputParser::fileExists(const QString & name)
{
    static QFileInfo fi;

    if (QDir::isAbsolutePath(name)) {
        fi.setFile(name);
        if(fi.exists() && !fi.isDir()) {
            return true;
        }
        else {
            return false;
        }
    }

    fi.setFile(path() + '/' + name);
    if(fi.exists() && !fi.isDir()) {
        return true;
    }

    fi.setFile(path() + '/' + name + m_extensions->latexDocumentDefault());
    if(fi.exists() && !fi.isDir()) {
        return true;
    }

    // try to determine the LaTeX source file
    QStringList extlist = m_extensions->latexDocuments().split(' ');
    for(QStringList::Iterator it = extlist.begin(); it!=extlist.end(); ++it) {
        fi.setFile(path() + '/' + name + (*it));
        if(fi.exists() && !fi.isDir()) {
            return true;
        }
    }

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
void LaTeXOutputParser::updateFileStack(const QString &strLine, short& dwCookie)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::updateFileStack()================" << endl;
    static QString strPartialFileName;

    switch (dwCookie) {
    //we're looking for a filename
    case Start :
    case FileNameHeuristic :
        //TeX is opening a file
        if(strLine.startsWith(":<+ ")) {
// 				qCDebug(LOG_KILE_PARSER) << "filename detected" << endl;
            //grab the filename, it might be a partial name (i.e. continued on the next line)
            strPartialFileName = strLine.mid(4).trimmed();

            //change the cookie so we remember we aren't sure the filename is complete
            dwCookie = FileName;
        }
        //TeX closed a file
        else if(strLine.contains(":<-")) {
// 				qCDebug(LOG_KILE_PARSER) << "\tpopping : " << m_stackFile.top().file() << endl;
            if(!m_stackFile.isEmpty()) {
                m_stackFile.pop();
            }
            dwCookie = Start;
        }
        else {
            //fallback to the heuristic detection of filenames
            updateFileStackHeuristic(strLine, dwCookie);
        }
        break;

    case FileName :
        //The partial filename was followed by '(', this means that TeX is signalling it is
        //opening the file. We are sure the filename is complete now. Don't call updateFileStackHeuristic
        //since we don't want the filename on the stack twice.
        if(strLine.startsWith('(') || strLine.startsWith(QLatin1String("\\openout"))) {
            //push the filename on the stack and mark it as 'reliable'
            m_stackFile.push(LOFStackItem(strPartialFileName, true));
// 				qCDebug(LOG_KILE_PARSER) << "\tpushed : " << strPartialFileName << endl;
            strPartialFileName.clear();
            dwCookie = Start;
        }
        //The partial filename was followed by an TeX error, meaning the file doesn't exist.
        //Don't push it on the stack, instead try to detect the error.
        else if(strLine.startsWith('!')) {
// 				qCDebug(LOG_KILE_PARSER) << "oops!" << endl;
            dwCookie = Start;
            strPartialFileName.clear();
            detectError(strLine, dwCookie);
        }
        else if(strLine.startsWith(QLatin1String("No file"))) {
// 				qCDebug(LOG_KILE_PARSER) << "No file: " << strLine << endl;
            dwCookie = Start;
            strPartialFileName.clear();
            detectWarning(strLine, dwCookie);
        }
        //Partial filename still isn't complete.
        else {
// 				qCDebug(LOG_KILE_PARSER) << "\tpartial file name, adding" << endl;
            strPartialFileName = strPartialFileName + strLine.trimmed();
        }
        break;

    default:
        break;
    }
}

void LaTeXOutputParser::updateFileStackHeuristic(const QString &strLine, short & dwCookie)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::updateFileStackHeuristic()================";
    //qCDebug(LOG_KILE_PARSER) << strLine << dwCookie;
    static QString strPartialFileName;
    bool expectFileName = (dwCookie == FileNameHeuristic);
    int index = 0;

    // handle special case (bug fix for 101810)
    if(expectFileName && strLine.length() > 0 && strLine[0] == ')') {
        m_stackFile.push(LOFStackItem(strPartialFileName));
        expectFileName = false;
        dwCookie = Start;
    }

    //scan for parentheses and grab filenames
    for(int i = 0; i < strLine.length(); ++i) {
        /*
        We're expecting a filename. If a filename really ends at this position one of the following must be true:
        	1) Next character is a space (indicating the end of a filename (yes, there can't be spaces in the
        	path, this is a TeX limitation).
        comment by tbraun: there is a workround \include{{"file name"}} according to http://groups.google.com/group/comp.text.tex/browse_thread/thread/af873534f0644e4f/cd7e0cdb61a8b837?lnk=st&q=include+space+tex#cd7e0cdb61a8b837,
        but this is currently not supported by kile.
        	2) We're at the end of the line, the filename is probably continued on the next line.
        	3) The TeX was closed already, signalled by the ')'.
        */

        bool isLastChar = (i+1 == strLine.length());
        bool nextIsTerminator = isLastChar ? false : (strLine[i+1].isSpace() || strLine[i+1] == ')');

        if(expectFileName && (isLastChar || nextIsTerminator)) {
            qCDebug(LOG_KILE_PARSER) << "Update the partial filename " << strPartialFileName;
            strPartialFileName =  strPartialFileName + strLine.mid(index, i-index + 1);

            if(strPartialFileName.isEmpty()) { // nothing left to do here
                continue;
            }

            //FIXME: improve these heuristics
            if((isLastChar && (i < 78)) || nextIsTerminator || fileExists(strPartialFileName)) {
                m_stackFile.push(LOFStackItem(strPartialFileName));
                //qCDebug(LOG_KILE_PARSER) << "\tpushed (i = " << i << " length = " << strLine.length() << "): " << strPartialFileName;
                expectFileName = false;
                dwCookie = Start;
            }
            //Guess the filename is continued on the next line, only if the current strPartialFileName does not exist, see bug # 162899
            else if(isLastChar) {
                if(fileExists(strPartialFileName)) {
                    m_stackFile.push(LOFStackItem(strPartialFileName));
                    qCDebug(LOG_KILE_PARSER) << "pushed (i = " << i << " length = " << strLine.length() << "): " << strPartialFileName << endl;
                    expectFileName = false;
                    dwCookie = Start;
                }
                else {
                    qCDebug(LOG_KILE_PARSER) << "Filename spans more than one line." << endl;
                    dwCookie = FileNameHeuristic;
                }
            }
            //bail out
            else {
                dwCookie = Start;
                strPartialFileName.clear();
                expectFileName = false;
            }
        }
        //TeX is opening a file
        else if(strLine[i] == '(') {
            //we need to extract the filename
            expectFileName = true;
            strPartialFileName.clear();
            dwCookie = Start;

            //this is were the filename is supposed to start
            index = i + 1;
        }
        //TeX is closing a file
        else if(strLine[i] == ')') {
            //qCDebug(LOG_KILE_PARSER) << "\tpopping : " << m_stackFile.top().file();
            //If this filename was pushed on the stack by the reliable ":<+-" method, don't pop
            //a ":<-" will follow. This helps in preventing unbalanced ')' from popping filenames
            //from the stack too soon.
            if(m_stackFile.count() > 1 && !m_stackFile.top().reliable()) {
                m_stackFile.pop();
            }
            else {
                qCDebug(LOG_KILE_PARSER) << "\t\toh no, forget about it!";
            }
        }
    }
}


void LaTeXOutputParser::flushCurrentItem()
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::flushCurrentItem()================" << endl;
    int nItemType = m_currentItem.type();

    while( m_stackFile.count() > 0 && !fileExists(m_stackFile.top().file()) ) {
        m_stackFile.pop();
    }

    QString sourceFile = (m_stackFile.isEmpty()) ? QFileInfo(source()).fileName() : m_stackFile.top().file();
    m_currentItem.setSource(sourceFile);
    m_currentItem.setMainSourceFile(source());

    switch (nItemType) {
    case itmError:
        ++m_nErrors;
        m_infoList->push_back(m_currentItem);
        //qCDebug(LOG_KILE_PARSER) << "Flushing Error in" << m_currentItem.source() << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() <<  endl;
        break;

    case itmWarning:
        ++m_nWarnings;
        m_infoList->push_back(m_currentItem);
        //qCDebug(LOG_KILE_PARSER) << "Flushing Warning in " << m_currentItem.source() << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() << endl;
        break;

    case itmBadBox:
        ++m_nBadBoxes;
        m_infoList->push_back(m_currentItem);
        //qCDebug(LOG_KILE_PARSER) << "Flushing BadBox in " << m_currentItem.source() << "@" << m_currentItem.sourceLine() << " reported in line " << m_currentItem.outputLine() << endl;
        break;

    default:
        break;
    }
    m_currentItem.clear();
}

bool LaTeXOutputParser::detectError(const QString & strLine, short &dwCookie)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::detectError(" << strLine.length() << ")================" << endl;

    bool found = false, flush = false;

    static QRegExp reLaTeXError("^! LaTeX Error: (.*)$", Qt::CaseInsensitive);
    static QRegExp rePDFLaTeXError("^Error: pdflatex (.*)$", Qt::CaseInsensitive);
    static QRegExp reTeXError("^! (.*)\\.$");
    static QRegExp reLineNumber("^l\\.([0-9]+)(.*)");

    switch (dwCookie) {
    case Start :
        if(reLaTeXError.indexIn(strLine) != -1) {
            //qCDebug(LOG_KILE_PARSER) << "\tError : " <<  reLaTeXError.cap(1) << endl;
            m_currentItem.setMessage(reLaTeXError.cap(1));
            found = true;
        }
        else if(rePDFLaTeXError.indexIn(strLine) != -1) {
            //qCDebug(LOG_KILE_PARSER) << "\tError : " <<  rePDFLaTeXError.cap(1) << endl;
            m_currentItem.setMessage(rePDFLaTeXError.cap(1));
            found = true;
        }
        else if(reTeXError.indexIn(strLine) != -1) {
            //qCDebug(LOG_KILE_PARSER) << "\tError : " <<  reTeXError.cap(1) << endl;
            m_currentItem.setMessage(reTeXError.cap(1));
            found = true;
        }
        if(found) {
            dwCookie = strLine.endsWith('.') ? LineNumber : Error;
            m_currentItem.setOutputLine(GetCurrentOutputLine());
        }
        break;

    case Error :
        //qCDebug(LOG_KILE_PARSER) << "\tError (cont'd): " << strLine << endl;
        if(strLine.endsWith('.')) {
            dwCookie = LineNumber;
            m_currentItem.setMessage(m_currentItem.message() + strLine);
        }
        else if(GetCurrentOutputLine() - m_currentItem.outputLine() > 3) {
            qWarning() << "\tBAILING OUT: error description spans more than three lines" << endl;
            dwCookie = Start;
            flush = true;
        }
        break;

    case LineNumber :
        //qCDebug(LOG_KILE_PARSER) << "\tLineNumber " << endl;
        if(reLineNumber.indexIn(strLine) != -1) {
            dwCookie = Start;
            flush = true;
            //qCDebug(LOG_KILE_PARSER) << "\tline number: " << reLineNumber.cap(1) << endl;
            m_currentItem.setSourceLine(reLineNumber.cap(1).toInt());
            m_currentItem.setMessage(m_currentItem.message() + reLineNumber.cap(2));
        }
        else if(GetCurrentOutputLine() - m_currentItem.outputLine() > 10) {
            dwCookie = Start;
            flush = true;
            qWarning() << "\tBAILING OUT: did not detect a TeX line number for an error" << endl;
            m_currentItem.setSourceLine(0);
        }
        break;

    default :
        break;
    }

    if(found) {
        m_currentItem.setType(itmError);
        m_currentItem.setOutputLine(GetCurrentOutputLine());
    }

    if(flush) {
        flushCurrentItem();
    }

    return found;
}

bool LaTeXOutputParser::detectWarning(const QString & strLine, short &dwCookie)
{
    //qCDebug(LOG_KILE_PARSER) << strLine << strLine.length();

    bool found = false, flush = false;
    QString warning;

    static QRegExp reLaTeXWarning("^(((! )?(La|pdf)TeX)|Package|Class) .*Warning.*:(.*)", Qt::CaseInsensitive);
    static QRegExp reNoFile("No file (.*)");
    static QRegExp reNoAsyFile("File .* does not exist."); // FIXME can be removed when http://sourceforge.net/tracker/index.php?func=detail&aid=1772022&group_id=120000&atid=685683 has promoted to the users

    switch(dwCookie) {
    //detect the beginning of a warning
    case Start :
        if(reLaTeXWarning.indexIn(strLine) != -1) {
            warning = reLaTeXWarning.cap(5);
            //qCDebug(LOG_KILE_PARSER) << "\tWarning found: " << warning << endl;

            found = true;
            dwCookie = Start;

            m_currentItem.setMessage(warning);
            m_currentItem.setOutputLine(GetCurrentOutputLine());

            //do we expect a line number?
            flush = detectLaTeXLineNumber(warning, dwCookie, strLine.length());
        }
        else if(reNoFile.indexIn(strLine) != -1) {
            found = true;
            flush = true;
            m_currentItem.setSourceLine(0);
            m_currentItem.setMessage(reNoFile.cap(0));
            m_currentItem.setOutputLine(GetCurrentOutputLine());
        }
        else if(reNoAsyFile.indexIn(strLine) != -1) {
            found = true;
            flush = true;
            m_currentItem.setSourceLine(0);
            m_currentItem.setMessage(reNoAsyFile.cap(0));
            m_currentItem.setOutputLine(GetCurrentOutputLine());
        }

        break;

    //warning spans multiple lines, detect the end
    case Warning :
        warning = m_currentItem.message() + strLine;
        //qCDebug(LOG_KILE_PARSER) << "'\tWarning (cont'd) : " << warning << endl;
        flush = detectLaTeXLineNumber(warning, dwCookie, strLine.length());
        m_currentItem.setMessage(warning);
        break;

    default:
        break;
    }

    if(found) {
        m_currentItem.setType(itmWarning);
        m_currentItem.setOutputLine(GetCurrentOutputLine());
    }

    if(flush) {
        flushCurrentItem();
    }

    return found;
}

bool LaTeXOutputParser::detectLaTeXLineNumber(QString & warning, short & dwCookie, int len)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::detectLaTeXLineNumber(" << warning.length() << ")================" << endl;

    static QRegExp reLaTeXLineNumber("(.*) on input line ([0-9]+)\\.$", Qt::CaseInsensitive);
    static QRegExp reInternationalLaTeXLineNumber("(.*)([0-9]+)\\.$", Qt::CaseInsensitive);
    if((reLaTeXLineNumber.indexIn(warning) != -1) || (reInternationalLaTeXLineNumber.indexIn(warning) != -1)) {
        //qCDebug(LOG_KILE_PARSER) << "een" << endl;
        m_currentItem.setSourceLine(reLaTeXLineNumber.cap(2).toInt());
        warning += reLaTeXLineNumber.cap(1);
        dwCookie = Start;
        return true;
    }
    else if(warning.endsWith('.')) {
        //qCDebug(LOG_KILE_PARSER) << "twee" << endl;
        m_currentItem.setSourceLine(0);
        dwCookie = Start;
        return true;
    }
    //bailing out, did not find a line number
    else if((GetCurrentOutputLine() - m_currentItem.outputLine() > 4) || (len == 0)) {
        //qCDebug(LOG_KILE_PARSER) << "drie current " << GetCurrentOutputLine() << " " <<  m_currentItem.outputLine() << " len " << len << endl;
        m_currentItem.setSourceLine(0);
        dwCookie = Start;
        return true;
    }
    //error message is continued on the other line
    else {
        //qCDebug(LOG_KILE_PARSER) << "vier" << endl;
        dwCookie = Warning;
        return false;
    }
}

bool LaTeXOutputParser::detectBadBox(const QString & strLine, short & dwCookie)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::detectBadBox(" << strLine.length() << ")================" << endl;

    bool found = false, flush = false;
    QString badbox;

    static QRegExp reBadBox("^(Over|Under)(full \\\\[hv]box .*)", Qt::CaseInsensitive);

    switch(dwCookie) {
    case Start :
        if(reBadBox.indexIn(strLine) != -1) {
            found = true;
            dwCookie = Start;
            badbox = strLine;
            flush = detectBadBoxLineNumber(badbox, dwCookie, strLine.length());
            m_currentItem.setMessage(badbox);
        }
        break;

    case BadBox :
        badbox = m_currentItem.message() + strLine;
        flush = detectBadBoxLineNumber(badbox, dwCookie, strLine.length());
        m_currentItem.setMessage(badbox);
        break;

    default:
        break;
    }

    if(found) {
        m_currentItem.setType(itmBadBox);
        m_currentItem.setOutputLine(GetCurrentOutputLine());
    }

    if(flush) {
        flushCurrentItem();
    }

    return found;
}

bool LaTeXOutputParser::detectBadBoxLineNumber(QString & strLine, short & dwCookie, int len)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::detectBadBoxLineNumber(" << strLine.length() << ")================" << endl;

    static QRegExp reBadBoxLines("(.*) at lines ([0-9]+)--([0-9]+)", Qt::CaseInsensitive);
    static QRegExp reBadBoxLine("(.*) at line ([0-9]+)", Qt::CaseInsensitive);
    //Use the following only, if you know how to get the source line for it.
    // This is not simple, as TeX is not reporting it.
    static QRegExp reBadBoxOutput("(.*)has occurred while \\output is active^", Qt::CaseInsensitive);

    if(reBadBoxLines.indexIn(strLine) != -1) {
        dwCookie = Start;
        strLine = reBadBoxLines.cap(1);
        int n1 = reBadBoxLines.cap(2).toInt();
        int n2 = reBadBoxLines.cap(3).toInt();
        m_currentItem.setSourceLine(n1 < n2 ? n1 : n2);
        return true;
    }
    else if(reBadBoxLine.indexIn(strLine) != -1) {
        dwCookie = Start;
        strLine = reBadBoxLine.cap(1);
        m_currentItem.setSourceLine(reBadBoxLine.cap(2).toInt());
        //qCDebug(LOG_KILE_PARSER) << "\tBadBox@" << reBadBoxLine.cap(2) << "." << endl;
        return true;
    }
    else if(reBadBoxOutput.indexIn(strLine) != -1) {
        dwCookie = Start;
        strLine = reBadBoxLines.cap(1);
        m_currentItem.setSourceLine(0);
        return true;
    }
    //bailing out, did not find a line number
    else if((GetCurrentOutputLine() - m_currentItem.outputLine() > 3) || (len == 0)) {
        dwCookie = Start;
        m_currentItem.setSourceLine(0);
        return true;
    }
    else {
        dwCookie = BadBox;
    }

    return false;
}

short LaTeXOutputParser::parseLine(const QString & strLine, short dwCookie)
{
    //qCDebug(LOG_KILE_PARSER) << "==LaTeXOutputParser::parseLine(" << strLine << dwCookie << strLine.length() << ")================" << endl;

    switch (dwCookie) {
    case Start :
        if(!(detectBadBox(strLine, dwCookie) || detectWarning(strLine, dwCookie) || detectError(strLine, dwCookie))) {
            updateFileStack(strLine, dwCookie);
        }
        break;

    case Warning :
        detectWarning(strLine, dwCookie);
        break;

    case Error :
    case LineNumber :
        detectError(strLine, dwCookie);
        break;

    case BadBox :
        detectBadBox(strLine, dwCookie);
        break;

    case FileName :
    case FileNameHeuristic :
        updateFileStack(strLine, dwCookie);
        break;

    default:
        dwCookie = Start;
        break;
    }

    return dwCookie;
}

ParserOutput* LaTeXOutputParser::parse()
{
    LaTeXOutputParserOutput *parserOutput = new LaTeXOutputParserOutput();

    qCDebug(LOG_KILE_PARSER);

    m_infoList = &(parserOutput->infoList);
    m_nErrors = m_nWarnings = m_nBadBoxes = m_nParens = 0;
    m_stackFile.clear();
    m_stackFile.push(LOFStackItem(QFileInfo(source()).fileName(), true));

    short sCookie = 0;
    QString s;
    QFile f(m_logFile);

    m_log.clear();
    m_nOutputLines = 0;

    if(!f.open(QIODevice::ReadOnly)) {
        parserOutput->problem = i18n("Cannot open log file; did you run LaTeX?");
        return parserOutput;
    }
    m_infoList = &parserOutput->infoList;
    QTextStream t(&f);
    while(!t.atEnd()) {
        if(!m_parserThread->shouldContinueDocumentParsing()) {
            qCDebug(LOG_KILE_PARSER) << "stopping...";
            delete(parserOutput);
            f.close();
            return Q_NULLPTR;
        }
        s = t.readLine();
        sCookie = parseLine(s.trimmed(), sCookie);
        ++m_nOutputLines;

        m_log += s + '\n';
    }
    f.close();

    parserOutput->nWarnings = m_nWarnings;
    parserOutput->nErrors = m_nErrors;
    parserOutput->nBadBoxes = m_nBadBoxes;
    parserOutput->logFile = m_logFile;

    // for quick preview
    if(!texfilename.isEmpty() && selrow >= 0 && docrow >= 0) {
        updateInfoLists(texfilename, selrow, docrow);
    }

    return parserOutput;
}

void LaTeXOutputParser::updateInfoLists(const QString &texfilename, int selrow, int docrow)
{
    // get a short name for the original tex file
    QString filename = "./" + QFileInfo(texfilename).fileName();
// 	setSource(texfilename);

    //print detailed error info
    for(int i = 0; i < m_infoList->count(); ++i) {
        // perhaps correct filename and line number in OutputInfo
        OutputInfo &info = (*m_infoList)[i];
        info.setSource(filename);

        int linenumber = selrow + info.sourceLine() - docrow;
        if(linenumber < 0) {
            linenumber = 0;
        }
        info.setSourceLine(linenumber);
    }
}



/** Return number of errors etc. found in log-file. */
void LaTeXOutputParser::getErrorCount(int *errors, int *warnings, int *badboxes)
{
    *errors = m_nErrors;
    *warnings = m_nWarnings;
    *badboxes = m_nBadBoxes;
}

int LaTeXOutputParser::GetCurrentOutputLine() const
{
    return m_nOutputLines;
}

void LaTeXOutputParser::setSource(const QString &src)
{
    m_source = src;
    m_srcPath = QFileInfo(src).path();
}

}
