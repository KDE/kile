/***************************************************************************
                          kilelogwidget.cpp  -  description
                             -------------------
    begin                : Sat Dec 20 2003
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
 ****************************************************************************/

#include "kiletool_enums.h"
#include "kilelogwidget.h"
#include "kileinfo.h"

#include <kdebug.h>
#include <kurl.h>

#include <qregexp.h>
#include <qfileinfo.h>

namespace KileWidget
{
	LogMsg::LogMsg(KileInfo *info, QWidget *parent, const char *name ) :
		KTextEdit(parent,name),
		m_info(info)
	{
		setTabStopWidth(10);
		connect(this, SIGNAL(clicked(int, int)), this, SLOT(slotClicked(int, int)));
	}
	
	LogMsg::~LogMsg(){
	}
	
	void LogMsg::highlight()
	{
		blockSignals(true); // block signals to avoid recursion
		setUpdatesEnabled(false);
		int cursorParagraph, cursorIndex;

		getCursorPosition( &cursorParagraph, &cursorIndex );

		int line=0;
		for(uint i = 0 ; i < m_info->outputInfo()->size() ; i++ )
		{
			line = (*m_info->outputInfo())[i].outputLine();

			setSelection( line,0, line,paragraphLength(line) );
			
			switch ( (*m_info->outputInfo())[i].type() )
			{
			case LatexOutputInfo::itmError : setColor(QColor(0xCC, 0x00, 0x00)); break;
			case LatexOutputInfo::itmWarning : setColor(QColor(0x00, 0x00, 0xCC )); break;
			case LatexOutputInfo::itmBadBox : setColor(QColor(0x00, 0x80, 0x00)); break;
			default : break;
			}
			removeSelection();
		}
		setCursorPosition( cursorParagraph, cursorIndex );
		setUpdatesEnabled(true);
		blockSignals(false); // block signals to avoid recursion
	}

	void LogMsg::highlight(uint l, int direction /* = 1 */)
	{
		setCursorPosition(l + direction * 3 , 0);
		setSelection(l, 0, l, paragraphLength(l));
	}

	void LogMsg::highlight(const QString & begin, int direction /* = 1 */)
	{
		kdDebug() << "==void LogMsg::highlight(" << begin << ", int direction /* = 1 */)========" << endl;
		int parags = paragraphs();
		for ( int i = 0; i < parags;  i++ )
		{
			kdDebug() << "considering : " << text(i) << endl;
			if ( text(i).startsWith(begin) )
			{
				highlight(i, direction);
				break;
			}
		}
	}

	void LogMsg::slotClicked(int parag, int /*index*/)
	{
		int l = -1;
		QString s = text(parag), file = QString::null;
	
		static QRegExp reES = QRegExp("(^.*):([0-9]+):.*");
		//maybe there is an error summary
		if (  reES.search(s) != -1 ) 
		{
			l = reES.cap(2).toInt() - 1;
			file = reES.cap(1);
		}
		else
		{
			//look for error at line parag
			for (uint i=0; i< m_info->outputInfo()->size(); i++)
			{
				if ( (*m_info->outputInfo())[i].outputLine() == parag)
				{
					file = (*m_info->outputInfo())[i].source();
					l = (*m_info->outputInfo())[i].sourceLine() - 1;
					break;
				}
			}
		}

		file = m_info->getFullFromPrettyName(file);

		if ( file != QString::null )
		{
			emit(fileOpen(KURL::fromPathOrURL(file), QString::null));
			if ( l >= 0 ) emit(setLine(QString::number(l)));
		}
	}

	void LogMsg::printMsg(int type, const QString & message, const QString &tool)
	{
		QString ot = "", ct = "";

		switch (type)
		{
			case KileTool::Warning :
				ot = "<font color='blue'>";
				ct = "</font>";
				break;
			case KileTool::Error :
				ot = "<font color='red'>";
				ct = "</font>";
				break;
			default :
				ot = "<font color='black'>"; ct = "</font>";
				break;
		}

		if ( tool == QString::null)
			append(ot + message + ct);
		else
			append(ot + "<b>[" + tool + "]</b> " + message + ct );

		scrollToBottom();
	}

	void LogMsg::printProblem(int type, const QString & problem)
	{
		kdDebug() << "==KileWidget::LogMsg::printProblem===========" << endl;
		kdDebug() << "\t" << problem << endl;
		printMsg(type, problem, QString::null);
	}
}

#include "kilelogwidget.moc"
