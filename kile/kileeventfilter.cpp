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
#include <kdebug.h>

#include "kileeventfilter.h"
#include "kileconfig.h"

KileEventFilter::KileEventFilter()
{
	m_bHandleEnter = true;
	m_regexpEnter  = QRegExp("^(.*)(\\\\begin\\s*\\{([^\\{\\}]*)\\})");

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
					for (uint i=0; i < line.length(); ++i)
						if ( ! line[i].isSpace() ) line[i] = ' ';

					line += m_regexpEnter.cap(2).replace("\\begin","\\end")+"\n";
                    if ( shouldCompleteEnv(m_regexpEnter.cap(3), view) )
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

bool KileEventFilter::shouldCompleteEnv(const QString &env, Kate::View *view)
{
    kdDebug() << "==KileEventFilter::shouldCompleteEnv(" << env << ")=============" << endl;
    QRegExp reTestBegin("\\\\begin\\s*\\{\\s*" + env + "\\s*\\}");
    QRegExp reTestEnd("\\\\end\\s*\\{\\s*" + env + "\\s*\\}");
    
    int num = view->getDoc()->numLines();
    int numBeginsFound = 0;
    int numEndsFound = 0;
    uint realLine, realColumn;
    view->cursorPositionReal(&realLine, &realColumn);
    for ( int i = realLine; i < num; ++i)
    {
        numBeginsFound += view->getDoc()->textLine(i).contains(reTestBegin);
        numEndsFound += view->getDoc()->textLine(i).contains(reTestEnd);
        
        //kdDebug() << "\tline " << i << " [" << view->getDoc()->textLine(i) << "] b = " << numBeginsFound << " e = " << numEndsFound << endl;
        if ( (numBeginsFound == 1) && (numEndsFound == 1) ) return false;
        else if ( (numEndsFound == 0) && (numBeginsFound > 1) ) return true;
        else if ( (numBeginsFound > 2) || (numEndsFound > 1) ) return true; //terminate the search
    }
    
    return true;
}

#include "kileeventfilter.moc"
