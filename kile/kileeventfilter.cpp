//
// C++ Implementation: kileeventfilter
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qevent.h>

#include <kate/view.h>
#include <kate/document.h>

#include "kileeventfilter.h"
#include "kileconfig.h"

KileEventFilter::KileEventFilter()
{
	m_bHandleEnter = true;
	m_regexpEnter  = QRegExp("^(.*)(\\\\begin\\s*\\{[^\\{\\}]*\\})");

	readConfig();
}

void KileEventFilter::readConfig()
{
	m_bCompleteEnvironment = KileConfig::completeEnvironment();
}

bool KileEventFilter::eventFilter(QObject *o, QEvent *e)
{
	if ( e->type() == QEvent::AccelOverride)
	{
		QKeyEvent *ke = (QKeyEvent*) e;

		if ( m_bCompleteEnvironment &&  ke->key() == Qt::Key_Return && ke->state() == 0)
		{
			if (m_bHandleEnter)
			{
				Kate::View *view = (Kate::View*) o;

				QString line = view->getDoc()->textLine(view->cursorLine()).left(view->cursorColumnReal());
				int pos = m_regexpEnter.search(line);
				if (pos != -1 )
				{
					line = m_regexpEnter.cap(1);
					for (uint i=0; i < line.length(); i++)
						if ( ! line[i].isSpace() ) line[i] = ' ';

					line += m_regexpEnter.cap(2).replace("\\begin","\\end")+"\n";

					view->getDoc()->insertText(view->cursorLine()+1, 0, line);
				}

				m_bHandleEnter=false;

				return true;
			}
			else
				m_bHandleEnter = true;
		}

		m_bHandleEnter = true;
		return false;
	}

	//pass this event on
	return false;
}



#include "kileeventfilter.moc"
