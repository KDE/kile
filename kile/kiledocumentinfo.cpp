/***************************************************************************
                          kiledocumentinfo.cpp -  description
                             -------------------
    begin                : Sun Jul 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "kiledocumentinfo.h"

namespace KileDocument
{

bool Info::isTeXFile(const KURL & url)
{
	//TODO use mimetype
	QString shortName = url.fileName();
	return (shortName.right(4) == ".tex" || shortName.right(4) == ".ltx" || shortName.right(6) == ".latex") && (shortName != i18n("Untitled"));
}

bool Info::isBibFile(const KURL & url)
{
	QString shortName = url.fileName();
	return ( shortName.right(4) == ".bib" );
}

Info::Info(Kate::Document *doc) : m_doc(doc)
{
	m_config = kapp->config();
	if (m_doc)
		kdDebug() << "Info created for " << m_doc->docName() << endl;

	m_bIsRoot = false;
	m_arStatistics = new long[5];

	if (m_doc)
		m_url=m_oldurl = doc->url();
	else
		m_url = m_oldurl = KURL();

	//TODO: make this configurable
	m_dictStructLevel["\\label"]= KileStructData(-1, KileStruct::Label);
	m_dictStructLevel["\\bibitem"]= KileStructData(-2, KileStruct::BibItem);
	m_dictStructLevel["\\input"]=KileStructData(0, KileStruct::Input, "include");
	m_dictStructLevel["\\Input"]=KileStructData(0, KileStruct::Input, "include");
	m_dictStructLevel["\\include"]=KileStructData(0, KileStruct::Input, "include");
	m_dictStructLevel["\\part"]=KileStructData(1, KileStruct::Sect, "part");
	m_dictStructLevel["\\chapter"]=KileStructData(2, KileStruct::Sect, "chapter");
	m_dictStructLevel["\\section"]=KileStructData(3, KileStruct::Sect, "section");
	m_dictStructLevel["\\subsection"]=KileStructData(4, KileStruct::Sect, "subsection");
	m_dictStructLevel["\\subsubsection"]=KileStructData(5, KileStruct::Sect, "subsubsection");
	m_dictStructLevel["\\bibliography"]=KileStructData(0,KileStruct::Bibliography, "bibtex");
	m_dictStructLevel["\\usepackage"]=KileStructData(-3,KileStruct::Package);
}

void Info::emitNameChanged(Kate::Document * /*doc*/)
{
	kdDebug() << "==Info::emitNameChanged=========================="  << endl;
	if (m_doc)
	{
		kdDebug() << "\tfrom: " << m_url.path() << endl;
		kdDebug() << "\tto: " << m_doc->url().path() << endl;

		//don't emit if new URL is empty (i.e. when closing the document)
		if (!m_doc->url().isEmpty() && (m_url != m_doc->url() ) )
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
	bool word = false;

	for (uint p=0; p < line.length(); p++)
	{
		c = line[p];

		switch ( state )
		{
		case stStandard	:
			switch ( c )
			{
				case TEX_CAT0	:
					state = stControlSequence;
					stat[1]++;
					stat[4]++;

					//look ahead to avoid counting words like K\"ahler as two words
					if (! line[p+1].isPunct() || line[p+1] == '~' || line[p+1] == '^' )
						word=false;
				break;

				case TEX_CAT14 :
					p=line.length();
					word=false;
				break;

				default:
					if (c.isLetterOrNumber())
					{
						//only start new word if first character is a letter (42test is still counted as a word, but 42.2 not)
						if (c.isLetter() && !word)
						{
							word=true;
							stat[3]++;
						}

						stat[0]++;
					}
					else
					{
						stat[2]++;
						word = false;
					}

				break;
			}
		break;

		case stControlSequence	:
			if ( c.isLetter() )	state = stCommand;
			else state = stStandard;
			stat[1]++;
		break;

		case stCommand :
			if ( c.isLetter() ) { stat[1]++; }
			else if ( c == TEX_CAT0 ) stat[4]++;
			else if ( c == TEX_CAT14 )
			{
				p=line.length();
			}
			else
			{
				stat[2]++;
				state = stStandard;
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
	//#c in words    , #c in commands,  #c whitespace,    #words,           #latex_commands
	m_arStatistics[0]=m_arStatistics[1]=m_arStatistics[2]=m_arStatistics[3]=m_arStatistics[4]=0;

	return m_arStatistics;
}

void Info::cleanTempFiles(const QStringList &extlist )
{
	QString finame = url().fileName();
	QFileInfo fic(finame);
	
	QString baseName = fic.baseName(true); 
	
	QStringList fileList;
	for (uint i=0; i< extlist.count(); i++) 
	{ 
		fileList.append(baseName+extlist[i]); 
	}

	QString path = url().directory(false); 
	for (uint i=0; i < fileList.count(); i++) 
	{ 
		QFile file( path + fileList[i] );
		kdDebug() << "About to remove file = " << file.name() << endl;
		file.remove(); 
	} 
}


// match a { with the corresponding }
// pos is the positon of the {
QString Info::matchBracket(uint &l, uint &pos)
{
	QChar obracket = m_doc->textLine(l)[pos], cbracket;
	if (obracket == '{') cbracket = '}';
	if (obracket == '[') cbracket = ']';

	QString line, grab = "";
	int count=0, len;
	pos++;

	while ( l <= m_doc->numLines() )
	{
		line = m_doc->textLine(l);
		len = line.length();
		for (int i=pos; i < len; i++)
		{
			if (line[i] == '\\') i++;
			else if (line[i] == obracket) count++;
			else if (line[i] == cbracket)
			{
				count--;
				if (count < 0)
					return grab;
			}

			grab += line[i];
		}
		l++;
		pos=0;
	}

	return QString::null;
}

void Info::updateStruct()
{
	m_labels.clear();
	m_bibItems.clear();
	m_deps.clear();
	m_bibliography.clear();
	m_packages.clear();
	m_bIsRoot = false;
	m_preamble = QString::null;
}

void Info::updateBibItems()
{
}

const long* TeXInfo::getStatistics()
{
	//#c in words    , #c in commands,  #c whitespace,    #words,           #latex_commands
	m_arStatistics[0]=m_arStatistics[1]=m_arStatistics[2]=m_arStatistics[3]=m_arStatistics[4]=0;
	QString line;

	//FIXME : counts environment names as words
	if ( m_doc && m_doc->hasSelection() )
	{
		line = m_doc->selection();
		count(line, m_arStatistics);
	}
	else if (m_doc)
	for (uint l=0; l < m_doc->numLines(); l++)
	{
		line = m_doc->textLine(l);
		kdDebug() << "getStat : line : " << line << endl;
		count(line, m_arStatistics);
	}
	return m_arStatistics;
}

void TeXInfo::updateStruct()
{
	if ( getDoc() == 0L ) return;

	Info::updateStruct();

	kdDebug() << "==void TeXInfo::updateStruct()=========." << endl;

	QMapConstIterator<QString,KileStructData> it;

	static QRegExp::QRegExp reCommand("(\\\\[a-zA-Z]+)\\s*\\*?\\s*(\\{|\\[)");
	static QRegExp::QRegExp reComments("([^\\\\]%|^%).*$");
	static QRegExp::QRegExp reRoot("\\\\documentclass|\\\\documentstyle");
	static QRegExp::QRegExp reBD("\\\\begin\\s*\\{\\s*document\\s*\\}");
	static QRegExp::QRegExp reNewCommand("\\\\(re)?newcommand.*$");

	int teller=0, tagStart, bd = 0;
	uint tagEnd, tagLine = 0, tagCol = 0;
	QString m, s, cap, folder = "root";
	bool foundBD = false; // found \begin { document }

	for(uint i = 0; i < m_doc->numLines(); i++)
	{
		tagStart=tagEnd=0;
		folder = "root";
		s=m_doc->textLine(i);

		if (teller > 100)
		{
			teller=0;
			kapp->processEvents();
		}
		else
			teller++;
		
		//remove escaped \ characters
		s.replace("\\\\", "  ");

		//remove comments
		s.replace(reComments, "");

		//if the command is a \renewcommand or \newcommand, ignore rest of the line
		s.replace(reNewCommand, "");

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
				kdDebug() << "\tsetting m_bIsRoot to TRUE" << endl;
				tagEnd += reRoot.cap(0).length();
				m_bIsRoot = true;
			}

			tagStart = reCommand.search(s,tagEnd);
			m=QString::null;

			if (tagStart != -1)
			{
				//kdDebug() << "Found command " <<  reCommand.cap(0) << " at " << i << endl;
				cap = reCommand.cap(1);
				tagEnd = tagStart + reCommand.cap(0).length()-1;

				//look up the command in the dictionary
				it = m_dictStructLevel.find(cap);

				//if it is was a structure element, find the title (or label)
				if (it != m_dictStructLevel.end())
				{
					tagLine=i+1; tagCol = tagEnd+1;
					m = matchBracket(i, static_cast<uint&>(tagEnd)).stripWhiteSpace();
					if ( i >= tagLine ) //matching brackets spanned multiple lines
						s = m_doc->textLine(i);
					kdDebug() << "\tgrabbed : " << m << endl;
				}

				//title (or label) found, add the element to the listview
				if (m != QString::null)
				{

					//update the dependencies
					if ((*it).type == KileStruct::Input)
					{
						QString dep = m;
						if (dep.right(4) != ".tex")
							dep += ".tex";
						m_deps.append(dep);
					}

					//update the referenced Bib files
					if((*it).type == KileStruct::Bibliography)
					{
						kdDebug() << "\tappending Bibiliograph file " << m << endl;
						m_bibliography.append(m);
						m_deps.append(m + ".bib");
					}

					//update the label list
					if ((*it).type == KileStruct::Label)
					{
						m_labels.append(m);
						folder = "labels";
					}

					//update the bibitem list
					if ((*it).type == KileStruct::BibItem)
					{
						kdDebug() << "\tappending bibitem " << m << endl;
						m_bibItems.append(m);
					}

					//update the package list
					if ((*it).type == KileStruct::Package)
						m_packages.append(m);

					kdDebug() << "emitting foundItem " << m << endl;
					emit(foundItem(m, tagLine, tagCol, (*it).type, (*it).level, (*it).pix, folder));
				} //if m
			} // if tagStart
		} // while tagStart
	} //for

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
	int col = 0, startcol, startline;

	for(uint i = 0; i < m_doc->numLines(); i++)
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
				col++;
				if ( col == static_cast<int>(s.length()) )
				{
					do
					{
						i++;
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

	layout->addWidget(new QLabel(i18n("Characters in words"),page), 0,0);
	layout->addWidget(new QLabel(QString::number(list[0]), page), 0,2);

	layout->addWidget(new QLabel(i18n("Characters in LaTeX commands"),page), 1,0);
	layout->addWidget(new QLabel(QString::number(list[1]), page), 1,2);

	layout->addWidget(new QLabel(i18n("Whitespace/Delimiters/Punctuation Marks"),page), 2,0);
	layout->addWidget(new QLabel(QString::number(list[2]), page), 2,2);

	layout->addWidget(new QLabel(i18n("Total"),page), 3,1, Qt::AlignRight);
	layout->addWidget(new QLabel(QString::number(list[0]+list[1]+list[2]), page), 3,2);

	layout->addWidget(new QLabel(i18n("Words"),page), 4,0);
	layout->addWidget(new QLabel(QString::number(list[3]), page), 4,2);

	layout->addWidget(new QLabel(i18n("LaTeX commands"),page), 5,0);
	layout->addWidget(new QLabel(QString::number(list[4]), page), 5,2);

	layout->addWidget(new QLabel(i18n("Total"),page), 6,1, Qt::AlignRight);
	layout->addWidget(new QLabel(QString::number(list[3]+list[4]), page), 6,2);

	if (docinfo->getDoc() && docinfo->getDoc()->hasSelection())
		layout->addWidget(new QLabel(i18n("WARNING: These are the statistics for the selected text only."),page), 7,0, Qt::AlignRight);
}

#include "kiledocumentinfo.moc"
