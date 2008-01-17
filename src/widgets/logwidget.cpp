/**************************************************************************************
    begin                : Sat Dec 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include "widgets/logwidget.h"

#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>

#include <q3popupmenu.h>

#include "kiledebug.h"
#include <KUrl>
#include <KLocale>

#include "kiletool_enums.h"
#include "kileinfo.h"
#include "kileconfig.h"

namespace KileWidget
{
	LogWidget::LogWidget(KileInfo *info, QWidget *parent, const char *name ) :
		KTextEdit(parent),
		m_info(info)
	{
		setObjectName(name);
		setTabStopWidth(10);
		connect(this, SIGNAL(clicked(int, int)), this, SLOT(slotClicked(int, int)));
		QPalette customPalette = palette();
		customPalette.setColor(QPalette::Window, QColor(Qt::white));
		setPalette(customPalette);
	}
	
	LogWidget::~LogWidget()
	{
	}

	bool LogWidget::isShowingOutput() const
	{
		return !document()->isEmpty();
	}

	void LogWidget::scrollToBottom()
	{
		textCursor().movePosition(QTextCursor::End);
		ensureCursorVisible();
	}

	void LogWidget::highlight()
	{
		blockSignals(true); // block signals to avoid recursion
		setUpdatesEnabled(false);
		QTextCursor cursor;

		QString contents = document()->toPlainText();
		QTextStream textStream(&contents, QIODevice::ReadOnly);

		cursor = textCursor();

		while(!textStream.atEnd()) {
			QString line = textStream.readLine();
		}

		int line=0;
		for(uint i = 0 ; i < m_info->outputInfo()->size() ; ++i )
		{
			line = (*m_info->outputInfo())[i].outputLine();

			QTextCursor cursor = textCursor();
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
			cursor.select(QTextCursor::LineUnderCursor);
			setTextCursor(cursor);
			
			switch ( (*m_info->outputInfo())[i].type() )
			{
			case LatexOutputInfo::itmError : setTextColor(QColor(0xCC, 0x00, 0x00)); break;
			case LatexOutputInfo::itmWarning : setTextColor(QColor(0x00, 0x00, 0xCC )); break;
			case LatexOutputInfo::itmBadBox : setTextColor(QColor(0x00, 0x80, 0x00)); break;
			default : break;
			}
		}

		setTextCursor(cursor);
		setUpdatesEnabled(true);
		blockSignals(false); // block signals to avoid recursion
	}

	void LogWidget::highlight(uint l, int direction /* = 1 */)
	{
#ifdef __GNUC__
#warning Method still needs to be ported!
#endif
//FIXME: port for KDE4
// 		setCursorPosition(l + direction * 3 , 0);
// 		setSelection(l, 0, l, paragraphLength(l));
	}

	void LogWidget::highlightByIndex(int index, int size, int direction /* = 1 */)
	{
#ifdef __GNUC__
#warning Method still needs to be ported!
#endif
//FIXME: port for KDE4
/*
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
*/
	}

	void LogWidget::slotClicked(int parag, int /*index*/)
	{
#ifdef __GNUC__
#warning Method still needs to be ported!
#endif
//FIXME: port for KDE4
/*
		int l = 0;
		QString s = text(parag), file;
	
		static QRegExp reES = QRegExp("(^.*):([0-9]+):.*");
		//maybe there is an error summary
		if(reES.search(s) != -1) {
			l = reES.cap(2).toInt();
			file = reES.cap(1);
		}
		else {
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

		if(!file.isEmpty()) {
			emit(fileOpen(KUrl::fromPathOrUrl(file), QString()));
			if(l > 0) {
				emit(setLine(QString::number(l)));
			}
		}
*/
	}

	void LogWidget::printMsg(int type, const QString & message, const QString &tool)
	{
		if(type == KileTool::Error) {
			emit showingErrorMessage(this);
		}

		QString ot = "", ct = "</font>";

		switch(type) {
			case KileTool::Warning :
				ot = "<font color='blue'>";
			break;
			case KileTool::ProblemWarning :
				if(KileConfig::hideProblemWarning()) {
					return;
				}
				ot = "<font color='blue'>";
			break;
			case KileTool::Error: // fall through
			case KileTool::ProblemError:
				ot = "<font color='red'>";
			break;
			case KileTool::ProblemBadBox:
				if (KileConfig::hideProblemBadBox()) {
					return;
				}
				ot = "<font color='#666666'>";
			break;
			default:
				ot = "<font color='black'>";
			break;
		}

		if(tool.isEmpty()) {
			append(ot + message + ct);
		}
		else {
			append(ot + "<b>[" + tool + "]</b> " + message + ct );
		}

		scrollToBottom();
	}

	void LogWidget::printProblem(int type, const QString & problem)
	{
		KILE_DEBUG() << "\t" << problem << endl;
		printMsg(type, problem, QString::null);
	}

#ifdef __GNUC__
#warning Method still needs to be ported!
#endif
//FIXME: port for KDE4
/*
	Q3PopupMenu* LogWidget::createPopupMenu (const QPoint & pos)
	{
		//get standard popup menu
		Q3PopupMenu * popup = K3TextEdit::createPopupMenu(pos);

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
*/

	void LogWidget::handlePopup(int id)
	{
		if ( id == m_idBadBox ) KileConfig::setHideProblemBadBox(!KileConfig::hideProblemBadBox());
		else if ( id == m_idWarning ) KileConfig::setHideProblemWarning(!KileConfig::hideProblemWarning());
	}
}

#include "logwidget.moc"
