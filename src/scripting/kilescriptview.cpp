/******************************************************************************
  Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net)
                2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kilescriptview.h"
#include "kiledebug.h"


namespace KileScript {


KileScriptView::KileScriptView(QObject *parent, KileDocument::EditorExtension *editor)
	: QObject(parent), m_view(0), m_editor(editor)
{
}

////////////////////////////////// view //////////////////////////////////////

void KileScriptView::setView (KTextEditor::View *view)
{
	m_view = view;
}

KTextEditor::View *KileScriptView::view() const
{
	return m_view;
}

////////////////////////////////// cursor //////////////////////////////////////

KTextEditor::Cursor KileScriptView::cursorPosition()
{
	return m_view->cursorPosition();
}

void KileScriptView::setCursorPosition(int line, int column)
{
	setCursorPosition( KTextEditor::Cursor(line, column) );
}

void KileScriptView::setCursorPosition (const KTextEditor::Cursor& cursor)
{
	m_view->setCursorPosition(cursor);
}

KTextEditor::Cursor KileScriptView::virtualCursorPosition()
{
	return m_view->cursorPositionVirtual();
}

void KileScriptView::cursorLeft()
{
	m_editor->moveCursorLeft(m_view);
}

void KileScriptView::cursorRight()
{
	m_editor->moveCursorRight(m_view);
}

void KileScriptView::cursorUp()
{
	m_editor->moveCursorUp(m_view);
}

void KileScriptView::cursorDown()
{
	m_editor->moveCursorDown(m_view);
}

int KileScriptView::cursorLine()
{
	return m_view->cursorPosition().line();
}

int KileScriptView::cursorColumn()
{
	return m_view->cursorPosition().column();
}

void KileScriptView::setCursorLine(int l)
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	cursor.setLine(l);
	m_view->setCursorPosition(cursor);
}

void KileScriptView::setCursorColumn(int c)
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	cursor.setColumn(c);
	m_view->setCursorPosition(cursor);
}

////////////////////////////////// selection //////////////////////////////////////

bool KileScriptView::hasSelection()
{
	return m_view->selection();
}

QString KileScriptView::selectedText()
{
	return m_view->selectionText();
}

KTextEditor::Range KileScriptView::selectionRange()
{
	return m_view->selectionRange();
}

void KileScriptView::setSelection(const KTextEditor::Range& range)
{
	m_view->setSelection(range);
}

void KileScriptView::selectAll()
{
  m_view->setSelection( m_view->document()->documentRange() );
}

void KileScriptView::clearSelection()
{
	m_view->removeSelection();
}

void KileScriptView::removeSelectedText()
{
	m_view->removeSelectionText();
}

/////////////////////////////// line //////////////////////////////

void KileScriptView::selectLine()
{
	m_editor->selectLine(m_view);
}

void KileScriptView::selectLine(int line)
{
	m_editor->selectLine(line,m_view);
}

void KileScriptView::selectLines(int from, int to)
{
	m_editor->selectLines(from,to,m_view);
}

/////////////////////////////// word //////////////////////////////

void KileScriptView::selectWord()
{
	m_editor->selectWord(KileDocument::EditorExtension::smLetter,m_view);
}

/////////////////////////////// latex command //////////////////////////////

void KileScriptView::selectLatexCommand()
{
	m_editor->selectWord(KileDocument::EditorExtension::smTex,m_view);
}

/////////////////////////////// environment //////////////////////////////

void KileScriptView::selectEnvironment(bool inside)
{
	m_editor->selectEnvironment(inside,m_view);
}

////////////////////////////////// TexGroup //////////////////////////////////////

void KileScriptView::selectTexgroup(bool inside)
{
	return m_editor->selectTexgroup(inside,m_view);
}

////////////////////////////////// MathGroup //////////////////////////////////////

void KileScriptView::selectMathgroup()
{
	m_editor->selectMathgroup(m_view);
}

////////////////////////////////// Paragraph //////////////////////////////////////

void KileScriptView::selectParagraph()
{
	m_editor->selectParagraph(m_view);
}


}

#include "kilescriptview.moc"


