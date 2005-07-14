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

#include "kileedit.h"
#include "kileeventfilter.h"
#include "kileconfig.h"

KileEventFilter::KileEventFilter(KileDocument::EditorExtension *edit) : m_edit(edit)
{
	readConfig();
}

void KileEventFilter::readConfig()
{
	m_bCompleteEnvironment = KileConfig::completeEnvironment();
}

// KateViewInternal as a child of Kate::View has the focus
// This was set with Kate::View::setFocusProxy(viewInternal) 

bool KileEventFilter::eventFilter(QObject *o, QEvent *e)
{
	if ( e->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke = (QKeyEvent*) e;	
		if ( m_bCompleteEnvironment &&  ke->key()==Qt::Key_Return && ke->state()==0)  
		{
			return m_edit->eventInsertEnvironment( (Kate::View*) o->parent() );
		}
	}

	//pass this event on
	return false;
}


#include "kileeventfilter.moc"
