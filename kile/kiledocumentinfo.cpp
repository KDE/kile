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

#include <klocale.h>
#include <kdebug.h>

#include "kiledocumentinfo.h"

KileDocumentInfo::KileDocumentInfo(Kate::Document *doc)
{
	m_doc = doc;

	m_arStatistics = new long[5];
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
