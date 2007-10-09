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

// 2007-03-17 dani
//  - select a single LaTeX command with CTRL+MouseDblClick-left
//    (such a double click on the middle part '\def' of '\abd\def\ghi'
//    will select only '\def\', not the whole text, as it does now)

#include "kileeventfilter.h"

#include <qevent.h>

#include <kate/view.h>
#include <kate/document.h>
#include "kiledebug.h"

#include "kileedit.h"
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
		if ( ke->key()==Qt::Key_QuoteDbl && ke->ascii()==Qt::Key_QuoteDbl )
		{
			return m_edit->insertDoubleQuotes();
		}
		if ( m_bCompleteEnvironment &&  ke->key()==Qt::Key_Return && ke->state()==0)  
		{
			return m_edit->eventInsertEnvironment( (Kate::View*) o->parent() );
		}
	}
	else if ( e->type() == QEvent::MouseButtonDblClick)
	{
		QMouseEvent *me = (QMouseEvent*) e;
		if ( me->button()==LeftButton && ((me->state() & Qt::ControlButton) == Qt::ControlButton) )
		{
			m_edit->selectWord(KileDocument::EditorExtension::smTex);
			return true;
		}
	}

	//pass this event on
	return false;
}


#include "kileeventfilter.moc"
