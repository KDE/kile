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

#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "kiledocumentinfo.h"

KileDocumentInfo::KileDocumentInfo(Kate::Document *doc)
{
	m_doc = doc;
	kdDebug() << "KileDocumentInfo created for " << doc->docName() << endl;

	m_struct = 0;
	m_arStatistics = new long[5];

	m_dictStructLevel["\\label"]= KileStructData(-1, KileStruct::Label);
	m_dictStructLevel["\\input"]=KileStructData(0, KileStruct::Input, "include");
	m_dictStructLevel["\\include"]=KileStructData(0, KileStruct::Input, "include");
	m_dictStructLevel["\\part"]=KileStructData(1, KileStruct::Sect, "part");
	m_dictStructLevel["\\chapter"]=KileStructData(2, KileStruct::Sect, "chapter");
	m_dictStructLevel["\\section"]=KileStructData(3, KileStruct::Sect, "section");
	m_dictStructLevel["\\subsection"]=KileStructData(4, KileStruct::Sect, "subsection");
	m_dictStructLevel["\\subsubsection"]=KileStructData(5, KileStruct::Sect, "subsubsection");
}

const long* KileDocumentInfo::getStatistics()
{
	//#c in words    , #c in commands,  #c whitespace,    #words,           #latex_commands
	m_arStatistics[0]=m_arStatistics[1]=m_arStatistics[2]=m_arStatistics[3]=m_arStatistics[4]=0;

	QString line;
	QChar c;
	int state;
	bool word;

	//FIXME : counts environment names as words
	for (uint l=0; l < m_doc->numLines(); l++)
	{
		line = m_doc->textLine(l);
		kdDebug() << "getStat : line : " << line << endl;

		state = stStandard;
		word = false;

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
						m_arStatistics[1]++;
						m_arStatistics[4]++;
						word=false;
					break;

					case TEX_CAT14 :
						p=line.length();
						word=false;
					break;

					default:
						if (c.isLetterOrNumber())
						{
							if (!word)
							{
								word=true;
								m_arStatistics[3]++;
							}

							m_arStatistics[0]++;
						}
						else
						{
							m_arStatistics[2]++;
						}

						if (c.isSpace() ) word = false;
					break;
				}
			break;

			case stControlSequence	:
				if ( c.isLetter() )	state = stCommand;
				else state = stStandard;
				m_arStatistics[1]++;
			break;

			case stCommand :
				if ( c.isLetter() ) { m_arStatistics[1]++; }
				else if ( c == TEX_CAT14 )
				{
					p=line.length();
				}
				else
				{
					m_arStatistics[2]++;
					state = stStandard;
				}
			break;

			default :
				kdWarning() << "Unhandled state in getStatistics " << state << endl;
			break;
			}
		}
	}

	return m_arStatistics;
}

// match a { with the corresponding }
// pos is the positon of the {
int KileDocumentInfo::matchBracket(const QString &line, int pos)
{
	int count=0, len = line.length();

	for (int i=pos+1; i < len; i++)
	{
		if (line[i] == '\\') i++;
		else if (line[i] == '{') count++;
		else if (line[i] == '}')
		{
			count--;
			if (count < 0)
				return i;
		}
	}

	return -1;
}

void KileDocumentInfo::updateStruct()
{
	kdDebug() << "KileDocumentInfo::updateStruct()" << endl;

	QString shortName = m_doc->url().fileName();
	if ((shortName.right(4)!=".tex") && (shortName!="untitled"))  return;

	m_labels.clear();

	delete m_struct;

	KileListViewItem *m_struct=  new KileListViewItem( m_structview, shortName );
	m_struct->setOpen(TRUE);
	m_struct->setPixmap(0,UserIcon("doc"));

	QListViewItem *parent_level[5],*lastChild, *Child, *parent;
	Child=lastChild=parent_level[0]=parent_level[1]=parent_level[2]=parent_level[3]=parent_level[4]=m_struct;

	QMapConstIterator<QString,KileStructData> it;

	KileListViewItem *toplabel=  new KileListViewItem(m_struct,"LABELS");

	QString s, cap;

	QRegExp reCommand("(\\\\[a-zA-Z]+)\\s*\\*?\\s*\\{");
	QRegExp reComments("([^\\\\]%|^%).*$");

	int tagStart, tagEnd, m;

	for(uint i = 0; i < m_doc->numLines(); i++)
	{
		tagStart=tagEnd=0;
		s=m_doc->textLine(i);

		//remove comments
		s.replace(reComments, "");
		//kdDebug() << "us() : " << s << endl;

		//find all commands in this line
		while (tagStart != -1)
		{
			tagStart = reCommand.search(s,tagEnd);
			m=-1;

			if (tagStart != -1)
			{
				kdDebug() << "Found command " <<  reCommand.cap(0) << " at " << i << endl;
				cap = reCommand.cap(1);
				tagEnd = tagStart + reCommand.cap(0).length();

				//look up the command in the dictionary
				it = m_dictStructLevel.find(cap);

				//if it is was a structure element, find the title (or label)
				if (it != m_dictStructLevel.end())
				{
					m = matchBracket(s, tagEnd);
				}

				//title (or label) found, add the element to the listview
				if (m != -1)
				{
					//find the parent for the new element
					switch ((*it).level)
					{
					case	-1	:	parent = toplabel;break;
					case	0	:
					case	1	: 	parent= m_struct; break;
					default	:	parent = parent_level[(*it).level-2]; break;
					}

					//find last element at this level
					Child = parent->firstChild();
					while( Child )
					{
						lastChild=Child;
						Child = Child->nextSibling();
					}

					Child=new KileListViewItem( parent,lastChild,s.mid(tagEnd, m-tagEnd).stripWhiteSpace(), i+1, tagEnd,(*it).type);
					if (! (*it).pix.isNull()) Child->setPixmap(0,UserIcon((*it).pix));

					//update the label list
					if ((*it).type == KileStruct::Label)
						m_labels.append(s.mid(tagEnd, m-tagEnd).stripWhiteSpace());

					//start the next search at the end of this tag
					tagEnd = m;

					//update the parent levels, such that section etc. get inserted at the correct level
					if ((*it).level > 0)
					{
						parent_level[(*it).level-1]=Child;
						for (int l = (*it).level; l < 5; l++)
							parent_level[l] = Child;
					}

				} //if m
			} // if tagStart
		} // while tagStart
	} //for
}

void KileDocumentInfo::updateBibItems()
{
}

/*
 *
 *	KileDocInfoDlg : a dialog that displays information known about the current document
 *
 *
 */

KileDocInfoDlg::KileDocInfoDlg(KileDocumentInfo *docinfo, QWidget* parent,  const char* name, const QString &caption)
	: KDialogBase(parent,name,true,caption,KDialogBase::Ok, KDialogBase::Ok, true)
{
	QWidget *page = new QWidget( this );
	setMainWidget(page);
	QGridLayout *layout = new QGridLayout( page, 7, 3,5,5,"");

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
}

#include "kiledocumentinfo.moc"
