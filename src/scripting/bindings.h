/***************************************************************************
*   Copyright (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net)   *
****************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef BINDINGS_H
#define BINDINGS_H

#include <QObject>

#include <QList>

class KileInfo;

namespace KTextEditor { class Document; class View; }

namespace KileScript {

class KileTextDocumentScriptObject : public QObject
{
	Q_OBJECT

	public:
		KileTextDocumentScriptObject(QObject* parent, KileInfo *kileInfo, KTextEditor::View *view);
		virtual ~KileTextDocumentScriptObject();

	public Q_SLOTS:
		void insertText(const QString& text);
		void insertBullet();
		void nextBullet();
		void previousBullet();
		void moveCursorLeft();
		void moveCursorRight();
		void moveCursorUp();
		void moveCursorDown();
		int cursorLine();
		int cursorColumn();
		void setCursorLine(int l);
		void setCursorColumn(int c);
		void backspace();

	private:
		KileInfo *m_kileInfo;
		KTextEditor::View *m_view;
};

class KileScriptObject : public QObject
{
	Q_OBJECT

	public:
		KileScriptObject(QObject *parent, KileInfo *kileInfo);

		virtual ~KileScriptObject();

	public Q_SLOTS:
		QString getInputValue(const QString& caption, const QString& label);
		QObject* currentTextDocument();

	private:
		KileInfo* m_kileInfo;
};

}

#endif

