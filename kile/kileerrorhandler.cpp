/***************************************************************************
    begin                : Tue May 25 2004
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

#include "kileerrorhandler.h"

#include <ktabwidget.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include <klocale.h>
#include <kurl.h>
#include <kate/document.h>
#include <kate/view.h>

#include "kiletool_enums.h"
#include "kilelogwidget.h"
#include "kileoutputwidget.h"
#include "kileinfo.h"
#include "latexoutputfilter.h"
#include "latexoutputinfo.h"
#include "kiledocmanager.h"
#include "kilesidebar.h"

KileErrorHandler::KileErrorHandler(QObject *parent, KileInfo *info, const char *name)
 : QObject(parent, name), m_ki(info), m_nCurrentError(-1)
{
}


KileErrorHandler::~KileErrorHandler()
{
}


void KileErrorHandler::reset()
{
	m_ki->setLogPresent(false);
	m_nCurrentError = -1;
}

void KileErrorHandler::ViewLog()
{
	m_ki->outputView()->showPage(m_ki->logWidget());
	m_ki->setLogPresent(false);

	QString cn = m_ki->getCompileName();
	if ( m_ki->outputFilter()->source() !=  cn )
	{
		m_ki->outputFilter()->setSource(cn);
		m_ki->outputFilter()->Run(cn.replace(QRegExp("\\..*$"),".log"));
	}

	QString log = m_ki->outputFilter()->log();

	if (!log.isNull())
	{
		m_ki->logWidget()->setText(log);
		m_ki->logWidget()->highlight();
		m_ki->logWidget()->scrollToBottom();
		m_ki->setLogPresent(true);
	}
	else
	{
		m_ki->logWidget()->printProblem(KileTool::Error, i18n("Cannot open log file; did you run LaTeX?"));
	}
}

void KileErrorHandler::jumpToFirstError()
{
	int sz = m_ki->outputInfo()->size();
	for (int i = 0; i < sz; ++i )
	{
		if ( (*m_ki->outputInfo())[i].type() == LatexOutputInfo::itmError )
		{
			jumpToProblem(&(*m_ki->outputInfo())[i]);
			m_nCurrentError = i;
			m_ki->logWidget()->highlightByIndex(i, sz, -1);
			break;
		}
	}
}

void KileErrorHandler::jumpToProblem(OutputInfo *info)
{
	QString file = m_ki->getFullFromPrettyName(info->source());

	if ( !file.isNull() )
	{
		m_ki->docManager()->fileOpen(KURL::fromPathOrURL(file));
		int line = info->sourceLine() > 0 ? (info->sourceLine() - 1) : 0;

		Kate::Document *doc = m_ki->docManager()->docFor(KURL::fromPathOrURL(file));
		if ( doc ) 
		{
			Kate::View* view = (Kate::View*)doc->views().first();
			if (view) view->setCursorPosition(line, 0);
		}
	}
}

void KileErrorHandler::showLogResults(const QString &src)
{
	m_ki->logWidget()->clear();
	m_ki->setLogPresent(false);
	m_ki->outputFilter()->setSource(src);
	QFileInfo fi(src);
	QString lf = fi.dirPath(true) + '/' + fi.baseName(true) + ".log";
	m_ki->logWidget()->printMsg(KileTool::Info, i18n("Detecting errors (%1), please wait ...").arg(lf), i18n("Log") );
	if ( ! m_ki->outputFilter()->Run( lf ) )
	{
		
		m_ki->outputFilter()->setSource(QString::null);
		return;
	}
	else
		m_ki->logWidget()->printMsg(KileTool::Info, i18n("Done."), i18n("Log") );
}

void KileErrorHandler::jumpToProblem(int type, bool forward)
{
	static LatexOutputInfoArray::iterator it;

	//if the current log file does not belong to the files the user is viewing
	//reparse the correct log file
	QString cn = m_ki->getCompileName();
	bool correctlogfile = (cn == m_ki->outputFilter()->source());
	if ( ! correctlogfile ) showLogResults(cn);

	if (!m_ki->outputInfo()->isEmpty())
	{
		int sz = m_ki->outputInfo()->size();
		int pl = forward ? 1 : -1;
		bool found = false;

		//look for next problem of requested type
		for ( int i = 0; i < sz; ++i )
		{
			//always look at the whole outputInfo array, but start
			//at the problem adjacent to the current error
			//if we go beyond the bounds of the array we use
			//a simple "modulo" calculation to get within bounds again
			int index = (m_nCurrentError + (i + 1) *pl) % sz;
			while ( index < 0 ) index += sz;

			if ( (*m_ki->outputInfo())[index].type() == type )
			{
				m_nCurrentError = index;
				found = true;
				break;
			}
		}

		if ( !found ) return;

		//If the log file is being viewed, use this to jump to the errors,
		//otherwise, use the error summary display
		if (m_ki->logPresent())
			m_ki->logWidget()->highlight( (*m_ki->outputInfo())[m_nCurrentError].outputLine(), pl );
 		else
 			m_ki->logWidget()->highlightByIndex(m_nCurrentError, sz, pl);

		jumpToProblem(&(*m_ki->outputInfo())[m_nCurrentError]);
	}

	if (m_ki->outputInfo()->isEmpty() && correctlogfile)
	{
		m_ki->logWidget()->append("\n<font color=\"#008800\">"+ i18n("No LaTeX errors detected.") + "</font>");
	}
}

void KileErrorHandler::NextError()
{
	jumpToProblem(LatexOutputInfo::itmError, true);
}

void KileErrorHandler::PreviousError()
{
	jumpToProblem(LatexOutputInfo::itmError, false);
}

void KileErrorHandler::NextWarning()
{
	jumpToProblem(LatexOutputInfo::itmWarning, true);
}

void KileErrorHandler::PreviousWarning()
{
	jumpToProblem(LatexOutputInfo::itmWarning, false);
}

void KileErrorHandler::NextBadBox()
{
	jumpToProblem(LatexOutputInfo::itmBadBox, true);
}

void KileErrorHandler::PreviousBadBox()
{
	jumpToProblem(LatexOutputInfo::itmBadBox, false);
}

#include "kileerrorhandler.moc"
