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

#include "kileedit.h"
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
#ifdef __GNUC__
#warning Method still needs to be implemented!
#endif
}

void KileTextDocumentScriptObject::moveCursorRight()
{
#ifdef __GNUC__
#warning Method still needs to be implemented!
#endif
}

void KileTextDocumentScriptObject::moveCursorUp()
{
#ifdef __GNUC__
#warning Method still needs to be implemented!
#endif
}

void KileTextDocumentScriptObject::moveCursorDown()
{
#ifdef __GNUC__
#warning Method still needs to be implemented!
#endif
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
	return KInputDialog::getText(checkedCaption, checkedLabel, QString(), 0, NULL);
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
