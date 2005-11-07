/***************************************************************************
    begin                : Sun Jul 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                               2005 by Holger Danielsson
    email                : Jeroen.Wijnhout@kdemail.net
                           holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2005-11-02: dani
//  - cleaning up source of central function updateStruct()
//      - always use 'else if', because all conditions are exclusive or
//      - most often used commands are at the top
//  - add some new types of elements (and levels) for the structure view
//  - new commands, which are passed to the structure listview:
//       \includegraphics, \caption
//  - all user defined commands for labels are recognized
//  - changed folder name of KileStruct::BibItem to "bibs", so that "refs"
//    is still unused and can be used for references (if wanted)
//  - \begin, \end to gather all environments. But only figure and table 
//    environments are passed to the structure view


#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qdatetime.h>

#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include "codecompletion.h"
#include "kiledocumentinfo.h"
#include "kileuntitled.h"

namespace KileDocument
{

bool Info::isTeXFile(const KURL & url)
{
	//TODO use mimetype
	QString shortName = url.fileName();
	return (shortName.right(4) == ".tex" || shortName.right(4) == ".sty" || shortName.right(4) == ".cls" || shortName.right(4) == ".dtx" || shortName.right(4) == ".ltx" || shortName.right(6) == ".latex") && ( !KileUntitled::isUntitled(shortName) );
}

bool Info::isBibFile(const KURL & url)
{
	QString shortName = url.fileName();
	return ( shortName.right(4) == ".bib" );
}

bool Info::containsInvalidCharacters(const KURL& url)
{
	QString filename = url.fileName();
	return filename.contains(" ") || filename.contains("~") || filename.contains("$") || filename.contains("#");
}

KURL Info::repairInvalidCharacters(const KURL& url)
{
	KURL ret(url);
	do {
		bool isOK;
		QString newURL = KInputDialog::getText(
			i18n("Invalid characters"),
			i18n("The filename contains invalid characters ($~ #).<br>Please provide \
				another one, or click on \"Cancel\" to save anyway."),
			url.filename(),
			&isOK);
		if(!isOK)
			return ret;
		ret.setFileName(newURL);
	} while(containsInvalidCharacters(ret));
	
	while ( QFileInfo(ret.path()).exists() )
	{
		bool isOK;
		QString newURL = KInputDialog::getText(
			i18n("File already exists."),
			i18n("A file with filename %1 already exists.<br>Please provide \
				another one, or click on \"Cancel\" to overwrite.").arg(ret.fileName()),
			url.filename(),
			&isOK);
		if(!isOK)
			return ret;
		ret.setFileName(newURL);
	}

	return ret;
}

KURL Info::repairExtension(const KURL& url)
{
	KURL ret(url);

	QString filename = url.fileName();
	if(filename.contains(".") && filename[0] != '.') // There already is an extension
		return ret;

	if(KMessageBox::Yes == KMessageBox::questionYesNo(NULL,
		i18n("The given filename has no extension; do you want one to be automatically added?"),
		i18n("Missing extension"),
		KStdGuiItem::yes(),
		KStdGuiItem::no(),
		"AutomaticallyAddExtension"))
	{
		ret.setFileName(filename + ".tex");	
	}
	return ret;
}

KURL Info::makeValidTeXURL(const KURL & url)
{
	KURL newURL(url);

	//add a .tex extension
	if(!isTeXFile(newURL)) 
		newURL = repairExtension(newURL);
		
	//remove characters TeX does not accept, make sure the newURL does not exists yet
	if(containsInvalidCharacters(newURL)) 
		newURL = repairInvalidCharacters(newURL);

	return newURL;
}

Info::Info(Kate::Document *doc, LatexCommands *commands) : m_doc(doc),m_commands(commands)
{
	m_config = kapp->config();
	if (m_doc)
		kdDebug() << "Info created for " << m_doc->docName() << endl;

	m_bIsRoot = false;
	m_arStatistics = new long[6];

	if (m_doc)
		m_url=m_oldurl = doc->url();
	else
		m_url = m_oldurl = KURL();
		
	// initialize m_dictStructLevel
	updateStructLevelInfo();
}

Info::~Info(void)
{
	kdDebug() << "DELETING DOCINFO" << m_url.path() << endl;
	delete [] m_arStatistics;
}

// set struct level dictionary with standard and user defined commands
void Info::updateStructLevelInfo()
{
	m_dictStructLevel.clear();

	// add standard commands
	//TODO: make this configurable
	m_dictStructLevel["\\label"]= KileStructData(KileStruct::NotSpecified, KileStruct::Label, QString::null, "labels");
	m_dictStructLevel["\\bibitem"]= KileStructData(KileStruct::NotSpecified, KileStruct::BibItem, QString::null, "bibs");
	
	m_dictStructLevel["\\input"]=KileStructData(KileStruct::File, KileStruct::Input, "include");
	m_dictStructLevel["\\Input"]=KileStructData(KileStruct::File, KileStruct::Input, "include");
	m_dictStructLevel["\\include"]=KileStructData(0, KileStruct::Input, "include");
	m_dictStructLevel["\\part"]=KileStructData(1, KileStruct::Sect, "part");
	m_dictStructLevel["\\chapter"]=KileStructData(2, KileStruct::Sect, "chapter");
	m_dictStructLevel["\\section"]=KileStructData(3, KileStruct::Sect, "section");
	m_dictStructLevel["\\subsection"]=KileStructData(4, KileStruct::Sect, "subsection");
	m_dictStructLevel["\\subsubsection"]=KileStructData(5, KileStruct::Sect, "subsubsection");
	m_dictStructLevel["\\paragraph"]=KileStructData(6, KileStruct::Sect, "subsubsection");
	m_dictStructLevel["\\subparagraph"]=KileStructData(7, KileStruct::Sect, "subsubsection");
	m_dictStructLevel["\\bibliography"]=KileStructData(0,KileStruct::Bibliography, "bibtex");
	
	// hidden commands  
	m_dictStructLevel["\\usepackage"]=KileStructData(KileStruct::Hidden, KileStruct::Package);
	m_dictStructLevel["\\newcommand"]=KileStructData(KileStruct::Hidden, KileStruct::NewCommand);
	
	// new entries
	m_dictStructLevel["\\includegraphics"]=KileStructData(KileStruct::Object,KileStruct::Graphics, "graphics");
	m_dictStructLevel["\\caption"]=KileStructData(KileStruct::Hidden,KileStruct::Caption);
	
	m_dictStructLevel["\\ref"]=KileStructData(KileStruct::Hidden,KileStruct::Reference);
	m_dictStructLevel["\\pageref"]=KileStructData(KileStruct::Hidden,KileStruct::Reference);
	m_dictStructLevel["\\vref"]=KileStructData(KileStruct::Hidden,KileStruct::Reference);
	m_dictStructLevel["\\vpageref"]=KileStructData(KileStruct::Hidden,KileStruct::Reference);
	
	
	// search also for environments
	m_dictStructLevel["\\begin"]=KileStructData(KileStruct::Object,KileStruct::BeginEnv);
	m_dictStructLevel["\\end"]=KileStructData(KileStruct::Hidden,KileStruct::EndEnv);
	
	// some entries, which could never be found (but they are set manually)
	m_dictStructLevel["\\begin{figure}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "frame_image");
	m_dictStructLevel["\\begin{table}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "frame_spreadsheet");
	m_dictStructLevel["\\end{float}"]=KileStructData(KileStruct::Hidden,KileStruct::EndFloat);
	
	// add user defined commands for labels
	QStringList labellist;
	QStringList::ConstIterator it;
	m_commands->commandList(labellist,KileDocument::CmdAttrLabel,true);
	for ( it=labellist.begin(); it != labellist.end(); ++it ) 
	{
		m_dictStructLevel[*it]= KileStructData(KileStruct::NotSpecified, KileStruct::Label, QString::null, "labels");
	}
	
	// add user defined commands for references
	QStringList reflist;
	m_commands->commandList(reflist,KileDocument::CmdAttrReference,true);
	for ( it=reflist.begin(); it != reflist.end(); ++it ) 
	{
		m_dictStructLevel[*it]= KileStructData(KileStruct::Hidden, KileStruct::Reference);
	}
}

void Info::emitNameChanged(Kate::Document * /*doc*/)
{
	kdDebug() << "==Info::emitNameChanged=========================="  << endl;
	if (m_doc)
	{
		kdDebug() << "\tfrom: " << m_url.path() << endl;
		kdDebug() << "\tto: " << m_doc->url().path() << endl;

		//don't emit if new URL is empty (i.e. when closing the document)
		if (!m_doc->url().isEmpty() && QFile::exists(m_doc->url().path()) && (m_url != m_doc->url() ) )
		{
			kdDebug() << "\temitting nameChanged(url)" << endl;
			setURL(m_doc->url());
			//emit(nameChanged(m_url));
			emit(nameChanged(m_doc));
		}
	}
}

void Info::count(const QString line, long *stat)
{
	QChar c;
	int state = stStandard;
	bool word = false; // we are in a word

	for (uint p=0; p < line.length(); ++p)
	{
		c = line[p];

		switch ( state )
		{
		case stStandard	:
			switch ( c )
			{
				case TEX_CAT0	:
					state = stControlSequence;
					++stat[1];		

					//look ahead to avoid counting words like K\"ahler as two words
					if (! line[p+1].isPunct() || line[p+1] == '~' || line[p+1] == '^' )
						word=false;
				break;

				case TEX_CAT14 :
					state=stComment;
				break;

				default:
					if (c.isLetterOrNumber())
					{
						//only start new word if first character is a letter (42test is still counted as a word, but 42.2 not)
						if (c.isLetter() && !word)
						{
							word=true;
							++stat[3];
						}
						++stat[0];
					}
					else
					{
						++stat[2];
						word = false;
					}

				break;
			}
		break;

		case stControlSequence :
			if ( c.isLetter() )
			{
			// "\begin{[a-zA-z]+}" is an environment, and you can't define a command like \begin
				if ( line.mid(p,5) == "begin" )
				{
					++stat[5];
					state = stEnvironment;
					stat[1] +=5;
					p+=4; // after break p++ is executed
				}
				else if ( line.mid(p,3) == "end" )
				{
					stat[1] +=3;
					state = stEnvironment;
					p+=2;	
				} // we don't count \end as new environment, this can give wrong results in selections
				else
				{
					++stat[4];
					++stat[1];
					state = stCommand;
				}
			}
			else
			{
				++stat[1];
				state = stStandard;
			}
		break;

		case stCommand :
			if ( c.isLetter() )
				++stat[1];
			else if ( c == TEX_CAT0 )
			{
				++stat[1];
				state=stControlSequence;
			}
			else if ( c == TEX_CAT14 )
				state=stComment;
			else
			{
				++stat[2];
				state = stStandard;
			}
		break;

		case stEnvironment :
			if ( c == TEX_CAT2  ) // until we find a closing } we have an environment
			{
				++stat[1];				
				state=stStandard;
			}
			else if ( c == TEX_CAT14 )
				state=stComment;
			else
				++stat[1];
				
		break;
		
		case stComment : // if we get a selection the line possibly contains \n and so the comment is only valid till \n and not necessarily till line.length()
			if ( c == '\n')
			{
			++stat[2]; // \n was counted as punctuation in the old implementation
			state=stStandard;
			word=false;
			}
		break;

		default :
			kdWarning() << "Unhandled state in getStatistics " << state << endl;
		break;
		}
	}
}

const long* Info::getStatistics()
{
	/* [0] = #c in words, [1] = #c in latex commands and environments,
	   [2] = #c whitespace, [3] = #words, [4] = # latex_commands, [5] = latex_environments */
	m_arStatistics[0]=m_arStatistics[1]=m_arStatistics[2]=m_arStatistics[3]=m_arStatistics[4]=m_arStatistics[5]=0;
	
	return m_arStatistics;
}

void Info::cleanTempFiles(const QStringList &extlist )
{
	QString finame = url().fileName();
	QFileInfo fic(finame);

	QString baseName = fic.baseName(true);

	QStringList fileList;
	for (uint i=0; i< extlist.count(); ++i)
	{
		fileList.append(baseName+extlist[i]);
	}

	QString path = url().directory(false);
	for (uint i=0; i < fileList.count(); ++i)
	{
		QFile file( path + fileList[i] );
		kdDebug() << "About to remove file = " << file.name() << endl;
		file.remove();
	}
}

QString Info::lastModifiedFile(const QStringList *list /* = 0L */)
{
	kdDebug() << "==QString Info::lastModifiedFile()=====" << endl;
	QFileInfo fileinfo ( url().path() );
	QString basepath = fileinfo.dirPath(true), last = fileinfo.absFilePath();
	QDateTime time ( fileinfo.lastModified() );

	if ( list == 0L ) list = &m_deps;

	kdDebug() << "\t" << fileinfo.absFilePath() << " : " << time.toString() << endl;
	for ( uint i = 0; i < list->count(); ++i )
	{
		fileinfo.setFile( basepath + "/" + (*list)[i] );
		kdDebug() << "\t" << fileinfo.absFilePath() << " : " << fileinfo.lastModified().toString() << endl;
		if ( fileinfo.lastModified() >  time )
		{
			time = fileinfo.lastModified();
			last = fileinfo.absFilePath();
			kdDebug() << "\t\tlater" << endl;
		}
	}

	kdDebug() << "\treturning " << fileinfo.absFilePath() << endl;
	return last;
}

// match a { with the corresponding }
// pos is the positon of the {
QString Info::matchBracket(QChar obracket, uint &l, uint &pos)
{
	QChar cbracket;
	if ( obracket == '{' ) cbracket = '}';
	if ( obracket == '[' ) cbracket = ']';
	if ( obracket == '(' ) cbracket = ')';

	QString line, grab = "";
	int count=0, len;
	++pos;

	while ( l <= m_doc->numLines() )
	{
		line = m_doc->textLine(l);
		len = line.length();
		for (int i=pos; i < len; ++i)
		{
			if (line[i] == '\\' && ( line[i+1] == obracket || line[i+1] == cbracket) ) ++i;
			else if (line[i] == obracket) ++count;
			else if (line[i] == cbracket)
			{
				--count;
				if (count < 0)
				{
					pos = i;
					return grab;
				}
			}

			grab += line[i];
		}
		++l;
		pos=0;
	}

	return QString::null;
}

void Info::updateStruct()
{
	kdDebug() << "==Info::updateStruct()=======" << endl;
	m_labels.clear();
	m_bibItems.clear();
	m_deps.clear();
	m_bibliography.clear();
	m_packages.clear();
	m_newCommands.clear();
	m_bIsRoot = false;
	m_preamble = QString::null;
}

void Info::updateBibItems()
{
}

const long* TeXInfo::getStatistics()
{
	/* [0] = #c in words, [1] = #c in latex commands and environments,
	   [2] = #c whitespace, [3] = #words, [4] = # latex_commands, [5] = latex_environments */
	m_arStatistics[0]=m_arStatistics[1]=m_arStatistics[2]=m_arStatistics[3]=m_arStatistics[4]=m_arStatistics[5]=0;
	QString line;
	
	if ( m_doc && m_doc->hasSelection() )
	{
		line = m_doc->selection();
		kdDebug() << "getStat : line : " << line << endl;
		count(line, m_arStatistics);
	}
	else if (m_doc)
	for (uint l=0; l < m_doc->numLines(); ++l)
	{
		line = m_doc->textLine(l);
		kdDebug() << "getStat : line : " << line << endl;
		count(line, m_arStatistics);
	}
	return m_arStatistics;
}
 
BracketResult TeXInfo::matchBracket(uint &l, uint &pos)
{
	BracketResult result;

	if ( m_doc->textLine(l)[pos] == '[' )
	{
		result.option = Info::matchBracket('[', l, pos);
		int p = 0;
		while ( l < m_doc->numLines() )
		{
			if ( (p = m_doc->textLine(l).find('{', pos)) != -1 )
			{
				pos = p;
				break;
			}
			else
			{
				pos = 0;
				++l;
			}
		}
	}

	if ( m_doc->textLine(l)[pos] == '{' )
		result.value  = Info::matchBracket('{', l, pos);

	return result;
}

//FIXME refactor, clean this mess up
void TeXInfo::updateStruct()
{
	kdDebug() << "==void TeXInfo::updateStruct: (" << url() << ")=========" << endl;

	if ( getDoc() == 0L ) return;

	Info::updateStruct();

	QMapConstIterator<QString,KileStructData> it;
	static QRegExp::QRegExp reCommand("(\\\\[a-zA-Z]+)\\s*\\*?\\s*(\\{|\\[)");
	static QRegExp::QRegExp reComments("([^\\\\]%|^%).*$");
	static QRegExp::QRegExp reRoot("\\\\documentclass|\\\\documentstyle");
	static QRegExp::QRegExp reBD("\\\\begin\\s*\\{\\s*document\\s*\\}");
	static QRegExp::QRegExp reReNewCommand("\\\\renewcommand.*$");
	static QRegExp::QRegExp reNumOfParams("\\s*\\[([1-9]+)\\]");
	static QRegExp::QRegExp reNumOfOptParams("\\s*\\[([1-9]+)\\]\\s*\\[(.*)\\]"); // the quantifier * isn't used by mistake, because also emtpy optional brackets are from the latex compilers point of view correct.

	int teller=0, tagStart, bd = 0;
	uint tagEnd, tagLine = 0, tagCol = 0;
	BracketResult result;
	QString m, s, shorthand;
	bool foundBD = false; // found \begin { document }
	bool fire = true; //whether or not we should emit a foundItem signal
	bool fireSuspended; // found an item, but it should not be fired (this time)

	for(uint i = 0; i < m_doc->numLines(); ++i)
	{
		tagStart=tagEnd=0;
		s=m_doc->textLine(i);
		fire = true;

		if (teller > 100)
		{
			teller=0;
			kapp->processEvents();
		}
		else
			++teller;

		//remove escaped \ characters
		s.replace("\\\\", "  ");

		//remove comments
		s.replace(reComments, "");

		//ignore renewcommands
		s.replace(reReNewCommand, "");

		//find all commands in this line
		while (tagStart != -1)
		{
			if ( (!foundBD) && ( (bd = s.find(reBD, tagEnd)) != -1))
			{
				kdDebug() << "\tfound \\begin{document}" << endl;
				foundBD = true;
				if ( bd == 0 ) m_preamble = m_doc->text(0, 0, i - 1, m_doc->textLine(i - 1).length() );
				else m_preamble = m_doc->text(0, 0, i, bd);
			}

			if ((!foundBD) && (s.find(reRoot, tagEnd) != -1))
			{
				kdDebug() << "\tsetting m_bIsRoot to true" << endl;
				tagEnd += reRoot.cap(0).length();
				m_bIsRoot = true;
			}

			tagStart = reCommand.search(s,tagEnd);
			m=QString::null;
			shorthand = QString::null;

			if (tagStart != -1)
			{
				tagEnd = tagStart + reCommand.cap(0).length()-1;

				//look up the command in the dictionary
				it = m_dictStructLevel.find(reCommand.cap(1));

				//if it is was a structure element, find the title (or label)
				if (it != m_dictStructLevel.end())
				{
					tagLine=i+1; tagCol = tagEnd+1;
					result = matchBracket(i, static_cast<uint&>(tagEnd));
					m = result.value.stripWhiteSpace();
					shorthand = result.option.stripWhiteSpace();
					if ( i >= tagLine ) //matching brackets spanned multiple lines
						s = m_doc->textLine(i);
					//kdDebug() << "\tgrabbed: " << reCommand.cap(1) << "[" << shorthand << "]{" << m << "}" << endl;
				}

				//title (or label) found, add the element to the listview
				if ( !m.isNull() )
				{
					// no problems so far ...
					fireSuspended = false;
					
					// update parameter for environments, because only
					// floating environments are passed
					if ( (*it).type == KileStruct::BeginEnv )
					{
						if ( m=="figure" || m=="table")
							it = m_dictStructLevel.find("\\begin{"+m+"}");
						else
							fireSuspended = true;          // only floats, no other environments
					}
					
					// tell structure view that a floating environment must be closed
					else if ( (*it).type == KileStruct::EndEnv )
					{
						if ( m=="figure" || m=="table")
							it = m_dictStructLevel.find("\\end{float}");
						else
							fireSuspended = true;          // only floats, no other environments
					}
					
					// sectioning commands
					else if ( (*it).type == KileStruct::Sect ) 
					{
						if ( ! shorthand.isNull() )
							m = shorthand;
					}

					// update the label list
					else if ( (*it).type == KileStruct::Label )
					{
						m_labels.append(m);
					}

					// update the references list
					else if ( (*it).type == KileStruct::Reference )
					{
						// m_references.append(m);
						fireSuspended = true;          // don't emit references
					}

					// update the dependencies
					else if ((*it).type == KileStruct::Input)
					{
						QString dep = m;
						if (dep.right(4) != ".tex")
							dep += ".tex";
						m_deps.append(dep);
					}

					// update the referenced Bib files
					else  if( (*it).type == KileStruct::Bibliography )
					{
						//kdDebug() << "\tappending Bibiliograph file " << m << endl;
						QStringList bibs = QStringList::split(",", m);
						uint cumlen = 0;
						for (uint b = 0; b < bibs.count(); ++b)
						{
							m_bibliography.append(bibs[b]);
							m_deps.append(bibs[b] + ".bib");
							emit(foundItem(bibs[b], tagLine, tagCol + cumlen, (*it).type, (*it).level, (*it).pix, (*it).folder));
							cumlen += bibs[b].length() + 1;
						}
						fire = false;
					}

					// update the bibitem list
					else if ( (*it).type == KileStruct::BibItem )
					{
						//kdDebug() << "\tappending bibitem " << m << endl;
						m_bibItems.append(m);
					}

					// update the package list
					else if ( (*it).type == KileStruct::Package )
					{
						QStringList pckgs = QStringList::split(",", m);
						uint cumlen = 0;
						for (uint p = 0; p < pckgs.count(); ++p)
						{
							QString package = pckgs[p].stripWhiteSpace();
							if ( ! package.isEmpty() ) {
								m_packages.append(package);
								// hidden, so emit is useless
								// emit(foundItem(package, tagLine, tagCol + cumlen, (*it).type, (*it).level, (*it).pix, (*it).folder));
								cumlen += package.length() + 1;
							}
						}
						fire = false;
					}

					// newcommand found, add it to the newCommands list
					else if ( (*it).type == KileStruct::NewCommand )
					{
						//find how many parameters this command takes
						if ( s.find(reNumOfParams, tagEnd + 1) != -1 )
						{
							bool ok;
							int noo = reNumOfParams.cap(1).toInt(&ok);
							QString cmdWithOptArgs = QString::null;
							if ( ok )
							{
								if(s.find(reNumOfOptParams, tagEnd + 1) != -1)
								{
								kdDebug() << "Opt param is " << reNumOfOptParams.cap(2) << "%EOL" << endl;
								noo--; // if we have an opt argument, we have one mandatory argument less, and noo=0 can't occure because then latex complains (and we don't macht them with reNumOfParams either)
								cmdWithOptArgs = m + "[" + reNumOfOptParams.cap(2) + "]";
								}
									
								for ( int noo_index = 0; noo_index < noo; ++noo_index)
								{
									m +=  "{" + s_bullet + "}";
									if(!cmdWithOptArgs.isNull())
										cmdWithOptArgs += "{" + s_bullet + "}";
								}
								
							}
						if(!cmdWithOptArgs.isNull())
							m_newCommands.append(cmdWithOptArgs);  // if we have opt args we add two new commands, one with and one without opt args.
						}
						m_newCommands.append(m);
						//FIXME  set tagEnd to the end of the command definition
						break;
					}
					
					// and some other commands, which don't need special actions: 
					// \includegraphics, \caption

					//kdDebug() << "\t\temitting: " << m << endl;
					if ( fire && !fireSuspended ) 
						emit( foundItem(m, tagLine, tagCol, (*it).type, (*it).level, (*it).pix, (*it).folder) );
				} //if m
			} // if tagStart
		} // while tagStart
	} //for

	emit(doneUpdating());
	emit(isrootChanged(isLaTeXRoot()));
}

void BibInfo::updateStruct()
{
	if ( getDoc() == 0L ) return;

	Info::updateStruct();

	kdDebug() << "==void BibInfo::updateStruct()========" << endl;

	static QRegExp::QRegExp reItem("^(\\s*)@([a-zA-Z]+)");
	static QRegExp::QRegExp reSpecial("string|preamble|comment");

	QString s, key;
	int col = 0, startcol, startline = 0;

	for(uint i = 0; i < m_doc->numLines(); ++i)
	{
		s = m_doc->textLine(i);
		if ( (s.find(reItem) != -1) && !reSpecial.exactMatch(reItem.cap(2).lower()) )
		{
			kdDebug() << "found: " << reItem.cap(2) << endl;
			//start looking for key
			key = "";
			bool keystarted = false;
			int state = 0;
			startcol = reItem.cap(1).length();
			col  = startcol + reItem.cap(2).length();

			while ( col <  static_cast<int>(s.length()) )
			{
				++col;
				if ( col == static_cast<int>(s.length()) )
				{
					do
					{
						++i;
						s = m_doc->textLine(i);
					} while  ( (s.length() == 0) && (i < m_doc->numLines()) );

					if ( i == m_doc->numLines() ) break;
					col = 0;
				}

				if ( state == 0 )
				{
					if ( s[col] == '{' ) state = 1;
					else if ( ! s[col].isSpace() ) break;
				}
				else if ( state == 1 )
				{
					if ( s[col] == ',' )
					{
						key = key.stripWhiteSpace();
						kdDebug() << "found: " << key << endl;
						m_bibItems.append(key);
						emit(foundItem(key, startline + 1, startcol, KileStruct::BibItem, 0, "bibtex", reItem.cap(2).lower()));
						break;
					}
					else
					{
						key += s[col];
						if (!keystarted) { startcol = col; startline = i; }
						keystarted=true;
					}
				}
			}
		}
	}

	emit(doneUpdating());
}

}

/*
 *
 *	KileDocInfoDlg : a dialog that displays information known about the current document
 *
 *
 */

KileDocInfoDlg::KileDocInfoDlg(KileDocument::Info *docinfo, QWidget* parent,  const char* name, const QString &caption)
	: KDialogBase(parent,name,true,caption,KDialogBase::Ok, KDialogBase::Ok, true)
{	
	
	QWidget *page = new QWidget( this );
	setMainWidget(page);
	QGridLayout *layout = new QGridLayout( page, 8, 3,5,5,"");

	const long *list = docinfo->getStatistics();

	layout->addWidget(new QLabel(i18n("Characters in words/numbers:"),page), 0,0);
	layout->addWidget(new QLabel(QString::number(list[0]), page), 0,2);

	layout->addWidget(new QLabel(i18n("Characters in LaTeX commands/environments:"),page), 1,0);
	layout->addWidget(new QLabel(QString::number(list[1]), page), 1,2);

	layout->addWidget(new QLabel(i18n("Whitespace/Delimiters/Punctuation marks:"),page), 2,0);
	layout->addWidget(new QLabel(QString::number(list[2]), page), 2,2);

	layout->addWidget(new QLabel(i18n("Total:"),page), 3,1, Qt::AlignRight);
	layout->addWidget(new QLabel(QString::number(list[0]+list[1]+list[2]), page), 3,2);

	layout->addWidget(new QLabel(i18n("Words:"),page), 4,0);
	layout->addWidget(new QLabel(QString::number(list[3]), page), 4,2);

	layout->addWidget(new QLabel(i18n("LaTeX commands:"),page), 5,0);
	layout->addWidget(new QLabel(QString::number(list[4]), page), 5,2);

	layout->addWidget(new QLabel(i18n("LaTeX environments:"),page), 6,0);
	layout->addWidget(new QLabel(QString::number(list[5]), page), 6,2);


	layout->addWidget(new QLabel(i18n("Total:"),page), 7,1, Qt::AlignRight);
	layout->addWidget(new QLabel(QString::number(list[3]+list[4]+list[5]), page), 7,2);

	if (docinfo->getDoc() && docinfo->getDoc()->hasSelection())
		layout->addWidget(new QLabel(i18n("WARNING: These are the statistics for the selected text only."),page), 8,0, Qt::AlignRight);
}

#include "kiledocumentinfo.moc"
