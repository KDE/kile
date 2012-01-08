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

#ifndef KILE_SCRIPT_VIEW_H
#define KILE_SCRIPT_VIEW_H

#include <QObject>
#include <QMap>
#include <QtScript/QScriptable>

#include <KTextEditor/Cursor>
#include <KTextEditor/Range>
#include <KTextEditor/View>
#include <KActionCollection>

#include "editorextension.h"

namespace KileScript {

class KileScriptView : public QObject, protected QScriptable
{
	Q_OBJECT

	public:
		KileScriptView (QObject *parent, KileDocument::EditorExtension *editor);
		virtual ~KileScriptView() {}

		void setView(KTextEditor::View *view);
		KTextEditor::View *view() const;

		// cursor
		Q_INVOKABLE KTextEditor::Cursor cursorPosition ();
		Q_INVOKABLE void setCursorPosition(int line, int column);
		Q_INVOKABLE void setCursorPosition(const KTextEditor::Cursor& cursor);

		Q_INVOKABLE void cursorLeft();
		Q_INVOKABLE void cursorRight();
		Q_INVOKABLE void cursorUp();
		Q_INVOKABLE void cursorDown();

		Q_INVOKABLE int cursorLine();
		Q_INVOKABLE int cursorColumn();
		Q_INVOKABLE void setCursorLine(int l);
		Q_INVOKABLE void setCursorColumn(int c);

		Q_INVOKABLE KTextEditor::Cursor virtualCursorPosition();

		// selection
		Q_INVOKABLE bool hasSelection();
		Q_INVOKABLE QString selectedText();
		Q_INVOKABLE KTextEditor::Range selectionRange();
		Q_INVOKABLE void setSelection(const KTextEditor::Range& range);
		Q_INVOKABLE void selectAll();

		Q_INVOKABLE void clearSelection();
		Q_INVOKABLE void removeSelectedText();

		// line
		Q_INVOKABLE void selectLine();
		Q_INVOKABLE void selectLine(int line);
		Q_INVOKABLE void selectLines(int from, int to);

		// word
		Q_INVOKABLE void selectWord();

		// latex command
		Q_INVOKABLE void selectLatexCommand();

		// environment
		Q_INVOKABLE void selectEnvironment(bool inside = false);

		// texgroup
		Q_INVOKABLE void selectTexgroup(bool inside = true);

		// mathgroup
		Q_INVOKABLE void selectMathgroup();

		// paragraph
		Q_INVOKABLE void selectParagraph();

	private:
		KTextEditor::View *m_view;
		KileDocument::EditorExtension *m_editor;

};

}

#endif
