/**********************************************************************************
*   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)           *
*                 2005-2007 by Holger Danielsson (holger.danielsson@versanet.de)  *
*                 2006-2019 by Michel Ludwig (michel.ludwig@kdemail.net)          *
***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "latexparser.h"

#include <QFileInfo>
#include <QRegExp>

#include <KLocalizedString>

#include "codecompletion.h"
#include "parserthread.h"

namespace KileParser {

LaTeXParserInput::LaTeXParserInput(const QUrl &url, QStringList textLines,
                                   KileDocument::Extensions *extensions,
                                   const QMap<QString, KileStructData>& dictStructLevel,
                                   bool showSectioningLabels,
                                   bool showStructureTodo)
    : ParserInput(url),
      textLines(textLines),
      extensions(extensions),
      // make a copy here as otherwise the parsing of a document that is being closed
      // can lead to a crash:
      dictStructLevel(dictStructLevel),
      showSectioningLabels(showSectioningLabels),
      showStructureTodo(showStructureTodo)
{
}

LaTeXParserOutput::LaTeXParserOutput()
    : bIsRoot(false)
{
}

LaTeXParserOutput::~LaTeXParserOutput()
{
    qCDebug(LOG_KILE_PARSER);
}

LaTeXParser::LaTeXParser(ParserThread *parserThread, LaTeXParserInput *input,
                         QObject *parent)
    : Parser(parserThread, parent),
      m_extensions(input->extensions),
      m_textLines(input->textLines),
      m_dictStructLevel(input->dictStructLevel),
      m_showSectioningLabels(input->showSectioningLabels),
      m_showStructureTodo(input->showStructureTodo)
{
}

LaTeXParser::~LaTeXParser()
{
    qCDebug(LOG_KILE_PARSER);
}

BracketResult LaTeXParser::matchBracket(const QStringList& textLines, int &l, int &pos)
{
    BracketResult result;
    TodoResult todo;

    if((getTextLine(textLines, l))[pos] == '[') {
        result.option = Parser::matchBracket(textLines, '[', l, pos);
        int p = 0;
        while(l < textLines.size()) {
            if((p = processTextline(getTextLine(textLines, l), todo).indexOf('{', pos)) != -1) {
                pos = p;
                break;
            }
            else {
                pos = 0;
                ++l;
            }
        }
    }

    if((getTextLine(textLines, l))[pos] == '{') {
        result.line = l;
        result.col = pos;
        result.value = Parser::matchBracket(textLines, '{', l, pos);
    }

    return result;
}

// Skip any number of white spaces in a line
int skipWS(const QString &s, int pos) {
    while(pos + 1 < s.length() && s.at(pos + 1).isSpace()) {
        ++pos;
    }
    return pos;
}

//FIXME: this has to be completely rewritten!
ParserOutput* LaTeXParser::parse()
{
    LaTeXParserOutput *parserOutput = new LaTeXParserOutput();

    qCDebug(LOG_KILE_PARSER) << m_textLines;

    QMap<QString,KileStructData>::const_iterator it;
    static QRegExp reCommand("(\\\\[a-zA-Z]+)\\s*\\*?\\s*(\\{|\\[)");
    static QRegExp reRoot("\\\\documentclass|\\\\documentstyle");
    static QRegExp reBD("\\\\begin\\s*\\{\\s*document\\s*\\}");
    static QRegExp reReNewCommand("\\\\renewcommand.*$");
    static QRegExp reNumOfParams("\\s*\\[([1-9]+)\\]");
    static QRegExp reNumOfOptParams("\\s*\\[([1-9]+)\\]\\s*\\[([^\\{]*)\\]"); // the quantifier * isn't used by mistake, because also emtpy optional brackets are correct.

    int tagStart, bd = 0;
    int tagEnd, tagLine = 0, tagCol = 0;
    int tagStartLine = 0, tagStartCol = 0;
    BracketResult result;
    QString m, s, shorthand;
    bool foundBD = false; // found \begin { document }
    bool fire = true; //whether or not we should emit a foundItem signal
    bool fireSuspended; // found an item, but it should not be fired (this time)
    TodoResult todo;

// 	emit(parsingStarted(m_doc->lines()));
    for(int i = 0; i < m_textLines.size(); ++i) {
        if(!m_parserThread->shouldContinueDocumentParsing()) {
            qCDebug(LOG_KILE_PARSER) << "stopping...";
            delete(parserOutput);
            return Q_NULLPTR;
        }

//		emit(parsingUpdate(i));

        tagStart = tagEnd = 0;
        fire = true;
        s = processTextline(getTextLine(m_textLines, i), todo);
        if(todo.type != -1 && m_showStructureTodo) {
            QString folder = (todo.type == KileStruct::ToDo) ? "todo" : "fixme";
            parserOutput->structureViewItems.push_back(new StructureViewItem(todo.comment, i+1, todo.colComment, todo.type, KileStruct::Object, i+1, todo.colTag, QString(), folder));
        }


        if(s.isEmpty()) {
            continue;
        }

        //ignore renewcommands
        s.remove(reReNewCommand);

        //find all commands in this line
        while(tagStart != -1) {
            if((!foundBD) && ((bd = s.indexOf(reBD, tagEnd)) != -1)) {
                qCDebug(LOG_KILE_PARSER) << "\tfound \\begin{document}";
                foundBD = true;
                parserOutput->preamble.clear();
//FIXME: improve this
                if(bd == 0) {
                    if(i - 1 >= 0) {
                        for(int j = 0; j <= i - 1; ++j) {
                            parserOutput->preamble += getTextLine(m_textLines, j) + '\n';
                        }
                    }
                }
                else {
                    if(i - 1 >= 0) {
                        for(int j = 0; j <= i - 1; ++j) {
                            parserOutput->preamble += getTextLine(m_textLines, j) + '\n';
                        }
                    }
                    parserOutput->preamble += getTextLine(m_textLines, i).left(bd) + '\n';
                }
            }

            if((!foundBD) && (s.indexOf(reRoot, tagEnd) != -1)) {
                qCDebug(LOG_KILE_PARSER) << "\tsetting m_bIsRoot to true";
                tagEnd += reRoot.cap(0).length();
                parserOutput->bIsRoot = true;
            }

            tagStart = reCommand.indexIn(s, tagEnd);
            m.clear();
            shorthand.clear();

            if(tagStart != -1) {
                tagEnd = tagStart + reCommand.cap(0).length()-1;

                //look up the command in the dictionary
                it = m_dictStructLevel.constFind(reCommand.cap(1));

                //if it is was a structure element, find the title (or label)
                if(it != m_dictStructLevel.constEnd()) {
                    tagLine = i+1;
                    tagCol = tagEnd+1;
                    tagStartLine = tagLine;
                    tagStartCol = tagStart+1;

                    if(reCommand.cap(1) != "\\frame") {
                        result = matchBracket(m_textLines, i, tagEnd);
                        m = result.value.trimmed();
                        shorthand = result.option.trimmed();
                        if(i >= tagLine) { //matching brackets spanned multiple lines
                            s = getTextLine(m_textLines, i);
                        }
                        if(result.line > 0 || result.col > 0) {
                            tagLine = result.line + 1;
                            tagCol = result.col + 1;
                        }
                        //qCDebug(LOG_KILE_PARSER) << "\tgrabbed: " << reCommand.cap(1) << "[" << shorthand << "]{" << m << "}";
                    }
                    else {
                        m = i18n("Frame");
                    }
                }

                //title (or label) found, add the element to the listview
                if(!m.isNull()) {
                    // no problems so far ...
                    fireSuspended = false;

                    // remove trailing ./
                    if((*it).type & (KileStruct::Input | KileStruct::Graphics)) {
                        if(m.left(2) == "./") {
                            m = m.mid(2, m.length() - 2);
                        }
                    }
                    // update parameter for environments, because only
                    // floating environments and beamer frames are passed
                    if ( (*it).type == KileStruct::BeginEnv )
                    {
                        if ( m=="figure" || m=="figure*" || m=="table" || m=="table*" )
                        {
                            it = m_dictStructLevel.constFind("\\begin{" + m +'}');
                        }
                        else if(m == "asy") {
                            it = m_dictStructLevel.constFind("\\begin{" + m +'}');
                            parserOutput->asyFigures.append(m);
                        }
                        else if(m == "frame") {
                            const QString untitledFrameDisplayName = i18n("Frame");
                            it = m_dictStructLevel.constFind("\\begin{frame}");
                            tagEnd = skipWS(s, tagEnd);
                            // the frame may have [fragile] modifier
                            if(tagEnd + 1 < s.size() && s.at(tagEnd + 1) == '[') {
                                tagEnd++;
                                Parser::matchBracket(m_textLines, '[', i, tagEnd);
                            }
                            tagEnd = skipWS(s, tagEnd);
                            if(tagEnd + 1 < s.size() && s.at(tagEnd + 1) == '{') {
                                tagEnd++;
                                result = matchBracket(m_textLines, i, tagEnd);
                                m = result.value.trimmed();
                                if(m.isEmpty()) {
                                    m = untitledFrameDisplayName;
                                }
                            }
                            else {
                                m = untitledFrameDisplayName;
                            }
                        }
                        else if(m=="block" || m=="exampleblock" || m=="alertblock") {
                            const QString untitledBlockDisplayName = i18n("Untitled Block");
                            it = m_dictStructLevel.constFind("\\begin{block}");
                            tagEnd = skipWS(s, tagEnd);
                            if(tagEnd+1 < s.size() && s.at(tagEnd+1) == '{') {
                                tagEnd++;
                                result = matchBracket(m_textLines, i, tagEnd);
                                m = result.value.trimmed();
                                if(m.isEmpty()) {
                                    m = untitledBlockDisplayName;
                                }
                            }
                            else {
                                m = untitledBlockDisplayName;
                            }
                        }
                        else {
                            fireSuspended = true;    // only floats and beamer frames, no other environments
                        }
                    }

                    // tell structure view that a floating environment or a beamer frame must be closed
                    else if ( (*it).type == KileStruct::EndEnv )
                    {
                        if ( m=="figure" || m== "figure*" || m=="table" || m=="table*" || m=="asy")
                        {
                            it = m_dictStructLevel.constFind("\\end{float}");
                        }
                        else if(m == "frame") {
                            it = m_dictStructLevel.constFind("\\end{frame}");
                        }
                        else {
                            fireSuspended = true;          // only floats, no other environments
                        }
                    }
                    // sectioning commands
                    else if((*it).type == KileStruct::Sect) {
                        if(!shorthand.isEmpty()) {
                            m = shorthand;
                        }
                    }

                    // update the label list
                    else if((*it).type == KileStruct::Label) {
                        parserOutput->labels.append(m);
                        // label entry as child of sectioning
                        if(m_showSectioningLabels) {
                            parserOutput->structureViewItems.push_back(new StructureViewItem(m, tagLine, tagCol, KileStruct::Label, KileStruct::Object, tagStartLine, tagStartCol, "label", "root"));
                            fireSuspended = true;
                        }
                    }

                    // update the references list
                    else if((*it).type == KileStruct::Reference) {
                        // m_references.append(m);
                        //fireSuspended = true;          // don't emit references
                    }

                    // update the dependencies
                    else if((*it).type == KileStruct::Input) {
                        // \input- or \include-commands can be used without extension. So we check
                        // if an extension exists. If not the default extension is added
                        // ( LaTeX reference says that this is '.tex'). This assures that
                        // all files, which are listed in the structure view, have an extension.
                        QString ext = QFileInfo(m).completeSuffix();
                        if(ext.isEmpty()) {
                            m += m_extensions->latexDocumentDefault();
                        }
                        parserOutput->deps.append(m);
                    }

                    // update the referenced Bib files
                    else  if((*it).type == KileStruct::Bibliography) {
                        qCDebug(LOG_KILE_PARSER) << "===TeXInfo::updateStruct()===appending Bibiliograph file(s) " << m;

                        const QStringList bibs = m.split(',');
                        QString biblio;

                        // assure that all files have an extension
                        const QString bibtexExtension = m_extensions->bibtexDefault();
                        for(QString biblio : bibs) {
                            biblio = biblio.trimmed();
                            {
                                QString ext = QFileInfo(biblio).suffix();
                                if(ext.isEmpty()) {
                                    biblio += m_extensions->bibtexDefault();
                                }
                            }
                            parserOutput->bibliography.append(biblio);
                            if(biblio.left(2) == "./") {
                                biblio = biblio.mid(2, biblio.length() - 2);
                            }
                            parserOutput->deps.append(biblio);
                            parserOutput->structureViewItems.push_back(new StructureViewItem(biblio, tagLine, tagCol, (*it).type, (*it).level, tagStartLine, tagStartCol, (*it).pix, (*it).folder));
                        }
                        fire = false;
                    }

                    // update the bibitem list
                    else if((*it).type == KileStruct::BibItem) {
                        //qCDebug(LOG_KILE_PARSER) << "\tappending bibitem " << m;
                        parserOutput->bibItems.append(m);
                    }

                    // update the package list
                    else if((*it).type == KileStruct::Package) {
                        QStringList pckgs = m.split(',');
                        uint cumlen = 0;
                        for(int p = 0; p < pckgs.count(); ++p) {
                            QString package = pckgs[p].trimmed();
                            if(!package.isEmpty()) {
                                parserOutput->packages.append(package);
                                // hidden, so emit is useless
                                // emit( foundItem(package, tagLine, tagCol+cumlen, (*it).type, (*it).level, tagStartLine, tagStartCol, (*it).pix, (*it).folder) );
                                cumlen += package.length() + 1;
                            }
                        }
                        fire = false;
                    }

                    // newcommand found, add it to the newCommands list
                    else if((*it).type & (KileStruct::NewCommand | KileStruct::NewEnvironment)) {
                        QString optArg, mandArgs;

                        //find how many parameters this command takes
                        if(s.indexOf(reNumOfParams, tagEnd + 1) != -1) {
                            bool ok;
                            int noo = reNumOfParams.cap(1).toInt(&ok);

                            if(ok) {
                                if(s.indexOf(reNumOfOptParams, tagEnd + 1) != -1) {
                                    qCDebug(LOG_KILE_PARSER) << "Opt param is " << reNumOfOptParams.cap(2) << "%EOL";
                                    noo--; // if we have an opt argument, we have one mandatory argument less, and noo=0 can't occur because then latex complains (and we don't macht them with reNumOfParams either)
                                    optArg = '[' + reNumOfOptParams.cap(2) + ']';
                                }

                                for(int noo_index = 0; noo_index < noo; ++noo_index) {
                                    mandArgs +=  '{' + s_bullet + '}';
                                }

                            }
                            if(!optArg.isEmpty()) {
                                if((*it).type == KileStruct::NewEnvironment) {
                                    parserOutput->newCommands.append(QString("\\begin{%1}%2%3").arg(m).arg(optArg).arg(mandArgs));
                                }
                                else {
                                    parserOutput->newCommands.append(m + optArg + mandArgs);
                                }
                            }
                        }
                        if((*it).type == KileStruct::NewEnvironment) {
                            parserOutput->newCommands.append(QString("\\begin{%1}%3").arg(m).arg(mandArgs));
                            parserOutput->newCommands.append(QString("\\end{%1}").arg(m));
                        }
                        else {
                            parserOutput->newCommands.append(m + mandArgs);
                        }
                        //FIXME  set tagEnd to the end of the command definition
                        break;
                    }
                    // and some other commands, which don't need special actions:
                    // \caption, ...

                    // qCDebug(LOG_KILE_PARSER) << "\t\temitting: " << m;
                    if(fire && !fireSuspended) {
                        parserOutput->structureViewItems.push_back(new StructureViewItem(m, tagLine, tagCol, (*it).type, (*it).level, tagStartLine, tagStartCol, (*it).pix, (*it).folder));
                    }
                } //if m
            } // if tagStart
        } // while tagStart
    } //for

    qCDebug(LOG_KILE_PARSER) << "done";
    return parserOutput;
}


}

