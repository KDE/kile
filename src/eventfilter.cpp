/***********************************************************************************************
    Copyright (C) 2004 by Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
                  2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-03-17 dani
//  - select a single LaTeX command with CTRL+MouseDblClick-left
//    (such a double click on the middle part '\def' of '\abd\def\ghi'
//    will select only '\def\', not the whole text, as it does now)

#include "eventfilter.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "kiledebug.h"
#include "editorextension.h"
#include "kileconfig.h"

LaTeXEventFilter::LaTeXEventFilter(KTextEditor::View *view, KileDocument::EditorExtension *edit) : QObject(view), m_view(view), m_edit(edit)
{
	readConfig();
}

void LaTeXEventFilter::readConfig()
{
	m_bCompleteEnvironment = KileConfig::completeEnvironment();
}

//FIXME: port for KDE4
// KateViewInternal as a child of KTextEditor::View has the focus
// This was set with KTextEditor::View::setFocusProxy(viewInternal)
bool LaTeXEventFilter::eventFilter(QObject* /* o */, QEvent *e)
{
	if(e->type() == QEvent::KeyPress) {
		QKeyEvent *ke = (QKeyEvent*) e;
		if(ke->key() == Qt::Key_QuoteDbl) {
			return m_edit->insertDoubleQuotes(m_view);
		}
		if(m_bCompleteEnvironment && ke->key() == Qt::Key_Return && ke->modifiers() == 0) {
			return m_edit->eventInsertEnvironment(m_view);
		}
	}
	else if(e->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent *me = static_cast<QMouseEvent*>(e);
		if(me->button() == Qt::LeftButton && me->modifiers() & Qt::ControlModifier) {
			m_edit->selectWord(KileDocument::EditorExtension::smTex, m_view);
			return true;
		}
	}

	//pass this event on
	return false;
}


#include "eventfilter.moc"
