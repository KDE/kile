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
#include <kapplication.h>
#include <kconfig.h>

#include "kileeventfilter.h"

KileEventFilter::KileEventFilter()
{
	m_bHandleEnter = true;
	//m_bCompleteEnvironment = false;
	m_regexpEnter  = QRegExp("(.*)(\\\\begin\\s*\\{[^\\{\\}]*\\})\\s*$");

	readConfig();
}

void KileEventFilter::readConfig()
{
	KConfig *config = kapp->config();
	config->setGroup( "Editor Ext" );
	m_bCompleteEnvironment = config->readBoolEntry( "Complete Environment", true);
}

bool KileEventFilter::eventFilter(QObject *o, QEvent *e)
{
	if ( e->type() == QEvent::AccelOverride)
	{
		QKeyEvent *ke = (QKeyEvent*) e;
		//kdDebug() << "eventFilter : AccelOverride : " << ke->key() << endl;
		//kdDebug() << "              type          : " << ke->type() << endl;
		//kdDebug() << "              state         : " << ke->state() << endl;

		if ( m_bCompleteEnvironment &&  ke->key() == Qt::Key_Return && ke->state() == 0)
		{
			if (m_bHandleEnter)
			{
				//kdDebug() << "              enter" << endl;
				Kate::View *view = (Kate::View*) o;

				QString line = view->getDoc()->textLine(view->cursorLine()).left(view->cursorColumnReal());
				int pos = m_regexpEnter.search(line);
				//kdDebug() << "              line     : " << line << endl;
				//kdDebug() << "              pos      : " << pos << endl;
				//kdDebug() << "              captured : " << m_regexpEnter.cap(1) << "+" << m_regexpEnter.cap(2) << endl;
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
