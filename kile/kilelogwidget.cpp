/***************************************************************************
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

#include "kilelogwidget.h"

#include <qregexp.h>
#include <qfileinfo.h>
#include <q3popupmenu.h>

#include "kiledebug.h"
#include <kurl.h>
#include <klocale.h>

#include "kiletool_enums.h"
#include "kileinfo.h"
#include "kileconfig.h"

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
		for(uint i = 0 ; i < m_info->outputInfo()->size() ; ++i )
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

	void LogMsg::highlightByIndex(int index, int size, int direction /* = 1 */)
	{
		int parags = paragraphs();
		int problemsFound = 0;
		int targetProblemNumber = size - index;
		static QRegExp reProblem(".*:[0-9]+:.*");
		
		//start from the bottom (most recent error) because
		//there could very well be errors with the same name
		for ( int i = parags - 1; i >= 0;  --i )
		{
			if ( reProblem.exactMatch(text(i)) ) ++problemsFound;
			
			if ( problemsFound == targetProblemNumber )
			{
				highlight(i, direction);
				break;
			}
		}
	}

	void LogMsg::slotClicked(int parag, int /*index*/)
	{
		int l = 0;
		QString s = text(parag), file = QString::null;
	
		static QRegExp reES = QRegExp("(^.*):([0-9]+):.*");
		//maybe there is an error summary
		if (  reES.search(s) != -1 ) 
		{
			l = reES.cap(2).toInt();
			file = reES.cap(1);
		}
		else
		{
			//look for error at line parag
			for (uint i=0; i< m_info->outputInfo()->size(); ++i)
			{
				if ( (*m_info->outputInfo())[i].outputLine() == parag)
				{
					file = (*m_info->outputInfo())[i].source();
					l = (*m_info->outputInfo())[i].sourceLine();
					break;
				}
			}
		}

		file = m_info->getFullFromPrettyName(file);

		if ( file != QString::null )
		{
			emit(fileOpen(KURL::fromPathOrURL(file), QString::null));
			if ( l > 0 ) emit(setLine(QString::number(l)));
		}
	}

	void LogMsg::printMsg(int type, const QString & message, const QString &tool)
	{
		if ( type == KileTool::Error ) emit showingErrorMessage(this);

		QString ot = "", ct = "</font>";

		switch (type)
		{
			case KileTool::Warning :
				ot = "<font color='blue'>"; 
			break;
			case KileTool::ProblemWarning : 
				if ( KileConfig::hideProblemWarning() ) return;
				ot = "<font color='blue'>"; 
			break;
			case KileTool::Error : case KileTool::ProblemError :
				ot = "<font color='red'>";
			break;
			case KileTool::ProblemBadBox :
				if ( KileConfig::hideProblemBadBox() ) return;
				ot = "<font color='#666666'>";
			break;
			default : ot = "<font color='black'>"; break;
		}

		if (tool.isNull())
			append(ot + message + ct);
		else
			append(ot + "<b>[" + tool + "]</b> " + message + ct );

		scrollToBottom();
	}

	void LogMsg::printProblem(int type, const QString & problem)
	{
		KILE_DEBUG() << "\t" << problem << endl;
		printMsg(type, problem, QString::null);
	}

	Q3PopupMenu* LogMsg::createPopupMenu (const QPoint & pos)
	{
		//get standard popup menu
		Q3PopupMenu * popup = KTextEdit::createPopupMenu(pos);

		//add toggle operations for hiding warnings/badboxes
		popup->insertSeparator();

		m_idBadBox = popup->insertItem(i18n("Hide &Bad Boxes"));
		popup->setItemChecked(m_idBadBox, KileConfig::hideProblemBadBox());

		m_idWarning = popup->insertItem(i18n("Hide (La)TeX &Warnings"));
		popup->setItemChecked(m_idWarning, KileConfig::hideProblemWarning());

		disconnect ( popup , SIGNAL(activated(int)), this , SLOT(handlePopup(int )));
		connect ( popup , SIGNAL(activated(int)), this , SLOT(handlePopup(int )));

		return popup;
	}

	void LogMsg::handlePopup(int id)
	{
		if ( id == m_idBadBox ) KileConfig::setHideProblemBadBox(!KileConfig::hideProblemBadBox());
		else if ( id == m_idWarning ) KileConfig::setHideProblemWarning(!KileConfig::hideProblemWarning());
	}
}

#include "kilelogwidget.moc"
