/**************************************************************************
*   Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "scripting/bindings.h"

#include <KInputDialog>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "editorextension.h"
#include "kileinfo.h"
#include "kileviewmanager.h"

namespace KileScript {

KileTextDocumentScriptObject::KileTextDocumentScriptObject(QObject* parent, KileInfo *kileInfo, KTextEditor::View *view) : QObject(parent), m_kileInfo(kileInfo), m_view(view)
{
}

KileTextDocumentScriptObject::~KileTextDocumentScriptObject()
{
}

void KileTextDocumentScriptObject::insertText(const QString& text)
{
	m_view->document()->insertText(m_view->cursorPosition(), text);
}

void KileTextDocumentScriptObject::insertBullet()
{
	m_kileInfo->editorExtension()->insertBullet(m_view);
}

void KileTextDocumentScriptObject::nextBullet()
{
	m_kileInfo->editorExtension()->nextBullet(m_view);
}

void KileTextDocumentScriptObject::previousBullet()
{
	m_kileInfo->editorExtension()->prevBullet(m_view);
}

void KileTextDocumentScriptObject::moveCursorLeft()
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	if(cursor.column() > 0) {
		cursor.setColumn(cursor.column() - 1);
		m_view->setCursorPosition(cursor);
	}
	else if(cursor.line() > 0) {
		int line = cursor.line() - 1;
		cursor.setLine(line);
		cursor.setColumn(m_view->document()->lineLength(line));
		m_view->setCursorPosition(cursor);
	}
}

void KileTextDocumentScriptObject::moveCursorRight()
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	int line = cursor.line();
	if(cursor.column() < m_view->document()->lineLength(line)) {
		cursor.setColumn(cursor.column() + 1);
		m_view->setCursorPosition(cursor);
	}
	else if(line < m_view->document()->lines()) {
		cursor.setLine(line + 1);
		cursor.setColumn(0);
		m_view->setCursorPosition(cursor);
	}
}

void KileTextDocumentScriptObject::moveCursorUp()
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	if(cursor.line() > 0) {
		cursor.setLine(cursor.line() - 1);
		m_view->setCursorPosition(cursor);
	}
}

void KileTextDocumentScriptObject::moveCursorDown()
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	if(cursor.line() < m_view->document()->lines() - 1) {
		cursor.setLine(cursor.line() + 1);
		m_view->setCursorPosition(cursor);
	}
}

int KileTextDocumentScriptObject::cursorLine()
{
	return m_view->cursorPosition().line();
}

int KileTextDocumentScriptObject::cursorColumn()
{
	return m_view->cursorPosition().column();
}

void KileTextDocumentScriptObject::setCursorLine(int l)
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	cursor.setLine(l);
	m_view->setCursorPosition(cursor);
}

void KileTextDocumentScriptObject::setCursorColumn(int c)
{
	KTextEditor::Cursor cursor = m_view->cursorPosition();
	cursor.setColumn(c);
	m_view->setCursorPosition(cursor);
}

void KileTextDocumentScriptObject::backspace()
{
	KTextEditor::Document *document = m_view->document();
	KTextEditor::Range selectionRange = m_view->selectionRange();
	if(selectionRange.isValid() && !selectionRange.isEmpty()) {
		document->removeText(selectionRange);
		return;
	}
	KTextEditor::Cursor currentPosition = m_view->cursorPosition();
	if(currentPosition.atStartOfDocument()) {
		return;
	}
	if(currentPosition.atStartOfLine()) {
		const int previousLine = currentPosition.line() - 1;
		KTextEditor::Range range(KTextEditor::Cursor(previousLine,
		                                             document->lineLength(previousLine)),
		                         currentPosition);
		m_view->document()->removeText(range);
		return;
	}
	KTextEditor::Range range(KTextEditor::Cursor(currentPosition.line(), currentPosition.column() - 1),
		                 currentPosition);
	m_view->document()->removeText(range);
}

KileScriptObject::KileScriptObject(QObject *parent, KileInfo* kileInfo) : QObject(parent), m_kileInfo(kileInfo)
{
}

KileScriptObject::~KileScriptObject()
{
}

QString KileScriptObject::getInputValue(const QString& caption, const QString& label)
{
	QString checkedCaption = caption, checkedLabel = label;
	if(caption.isEmpty()) {
		checkedCaption = i18n("Enter Value");
	}
	if(label.isEmpty()) {
		checkedLabel = i18n("Please enter a value");
	}
	return KInputDialog::getText(checkedCaption, checkedLabel, QString(), NULL, m_kileInfo->mainWindow());
}

QObject* KileScriptObject::currentTextDocument()
{
	KTextEditor::View *view = m_kileInfo->viewManager()->currentTextView();
	if(!view) {
		return NULL;
	}

	return new KileTextDocumentScriptObject(this, m_kileInfo, view);
}

}

#include "bindings.moc"
