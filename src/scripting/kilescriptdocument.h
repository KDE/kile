/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KILE_SCRIPT_DOCUMENT_H
#define KILE_SCRIPT_DOCUMENT_H

#include <QObject>
#include <QAction>
#include <QMap>

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/Cursor>
#include <KTextEditor/Range>

#include "editorextension.h"


class KileInfo;

namespace KileScript {

class KileScriptDocument : public QObject
{
	Q_OBJECT

	public:
		KileScriptDocument(QObject *parent, KileInfo *kileInfo, KileDocument::EditorExtension *editor,
		                                                        const QMap<QString, QAction*> *scriptActions);
		virtual ~KileScriptDocument() {}

		void setView(KTextEditor::View *view);

		// insert (extended insert using KileAction::TagData)/remove/replace
		Q_INVOKABLE void insertText(const QString &s);
		Q_INVOKABLE void insertText(int line, int column, const QString &s);
		Q_INVOKABLE void insertText(const KTextEditor::Cursor& cursor, const QString &s);

		Q_INVOKABLE bool removeText(int fromLine, int fromColumn, int toLine, int toColumn);
		Q_INVOKABLE bool removeText(const KTextEditor::Cursor& from, const KTextEditor::Cursor& to);
		Q_INVOKABLE bool removeText(const KTextEditor::Range& range);

		Q_INVOKABLE bool replaceText(const KTextEditor::Range& range, const QString &text);

		// document
		Q_INVOKABLE int lines();
		Q_INVOKABLE int length();
		Q_INVOKABLE KTextEditor::Range documentRange();
		Q_INVOKABLE KTextEditor::Cursor documentEnd();

		// text
		Q_INVOKABLE QString text();
		Q_INVOKABLE QString text(int fromLine, int fromColumn, int toLine, int toColumn);
		Q_INVOKABLE QString text(const KTextEditor::Cursor& from, const KTextEditor::Cursor& to);
		Q_INVOKABLE QString text(const KTextEditor::Range& range);
		Q_INVOKABLE bool setText(const QString &s);
		Q_INVOKABLE bool clear();

		// line
		Q_INVOKABLE QString line();
		Q_INVOKABLE QString line(int line);
		Q_INVOKABLE int lineLength();
		Q_INVOKABLE int lineLength(int line);

		Q_INVOKABLE bool insertLine(const QString &s);
		Q_INVOKABLE bool insertLine(int line, const QString &s);

		Q_INVOKABLE bool removeLine();
		Q_INVOKABLE bool removeLine(int line);

		Q_INVOKABLE bool replaceLine(const QString &s);
		Q_INVOKABLE bool replaceLine(int line,const QString &s);

		Q_INVOKABLE bool truncateLine();
		Q_INVOKABLE bool truncate(int line, int column);
		Q_INVOKABLE bool truncate(const KTextEditor::Cursor& cursor);

		// word
		Q_INVOKABLE QString word();
		Q_INVOKABLE QString wordAt(int line, int column);
		Q_INVOKABLE QString wordAt(const KTextEditor::Cursor& cursor);
		Q_INVOKABLE KTextEditor::Range wordRange();

		// latex command
		Q_INVOKABLE QString latexCommand();
		Q_INVOKABLE QString latexCommandAt(int line, int column);
		Q_INVOKABLE QString latexCommandAt(const KTextEditor::Cursor& cursor);
		Q_INVOKABLE KTextEditor::Range latexCommandRange();

		// char
		Q_INVOKABLE QString charAt(int line, int column);
		Q_INVOKABLE QString charAt(const KTextEditor::Cursor& cursor);
		Q_INVOKABLE QString firstChar(int line);
		Q_INVOKABLE QString lastChar(int line);
		Q_INVOKABLE bool isSpace(int line, int column);
		Q_INVOKABLE bool isSpace(const KTextEditor::Cursor& cursor);

		// bullet
		Q_INVOKABLE void insertBullet();
		Q_INVOKABLE void nextBullet();
		Q_INVOKABLE void previousBullet();

		// environment
		Q_INVOKABLE bool hasEnvironment();
		Q_INVOKABLE QString environment(bool inside = false);
		Q_INVOKABLE KTextEditor::Range environmentRange(bool inside = false);
		Q_INVOKABLE QString environmentName();

		Q_INVOKABLE void removeEnvironment(bool inside = false);

		Q_INVOKABLE void closeEnvironment();
		Q_INVOKABLE void closeAllEnvironments();

		// texgroup
		Q_INVOKABLE bool hasTexgroup();
		Q_INVOKABLE QString texgroup(bool inside = true);
		Q_INVOKABLE KTextEditor::Range texgroupRange(bool inside = true);

		Q_INVOKABLE void removeTexgroup(bool inside = true);

		// mathgroup
		Q_INVOKABLE bool hasMathgroup();
		Q_INVOKABLE QString mathgroup();
		Q_INVOKABLE KTextEditor::Range mathgroupRange();

		Q_INVOKABLE void removeMathgroup();

		// paragraph
		Q_INVOKABLE QString paragraph();
		Q_INVOKABLE KTextEditor::Range paragraphRange();

		Q_INVOKABLE void removeParagraph();

		// match text
		Q_INVOKABLE bool matchesAt(int line, int column, const QString &s);
		Q_INVOKABLE bool matchesAt(const KTextEditor::Cursor& cursor, const QString &s);
		Q_INVOKABLE bool startsWith (int line, const QString &pattern, bool skipWhiteSpaces = true);
		Q_INVOKABLE bool endsWith (int line, const QString &pattern, bool skipWhiteSpaces = true);

		// columns/lines
		Q_INVOKABLE int firstColumn(int line);
		Q_INVOKABLE int lastColumn(int line);
		Q_INVOKABLE int prevNonSpaceColumn(int line, int column);
		Q_INVOKABLE int prevNonSpaceColumn(const KTextEditor::Cursor& cursor);
		Q_INVOKABLE int nextNonSpaceColumn(int line, int column);
		Q_INVOKABLE int nextNonSpaceColumn(const KTextEditor::Cursor& cursor);
		Q_INVOKABLE int prevNonEmptyLine(int line);
		Q_INVOKABLE int nextNonEmptyLine(int line);

		// goto
		Q_INVOKABLE void gotoBeginEnv();
		Q_INVOKABLE void gotoEndEnv();
		Q_INVOKABLE void gotoBeginTexgroup();
		Q_INVOKABLE void gotoEndTexgroup();
		Q_INVOKABLE void gotoNextParagraph();
		Q_INVOKABLE void gotoPrevParagraph();
		Q_INVOKABLE void gotoNextSectioning();
		Q_INVOKABLE void gotoPrevSectioning();
		Q_INVOKABLE void gotoLine(int line);

		// insert sectioning
		Q_INVOKABLE void insertChapter();
		Q_INVOKABLE void insertSection();
		Q_INVOKABLE void insertSubsection();
		Q_INVOKABLE void insertSubsubsection();
		Q_INVOKABLE void insertParagraph();
		Q_INVOKABLE void insertSubparagraph();

		// insert references
		Q_INVOKABLE void insertLabel();
		Q_INVOKABLE void insertReference();
		Q_INVOKABLE void insertCitation();
		Q_INVOKABLE void insertPageref();
		Q_INVOKABLE void insertIndex();
		Q_INVOKABLE void insertFootnote();

		// KatePart actions with selections
		Q_INVOKABLE void comment();
		Q_INVOKABLE void uncomment();
		Q_INVOKABLE void uppercase();
		Q_INVOKABLE void lowercase();
		Q_INVOKABLE void capitalize();
		Q_INVOKABLE void joinLines();

		// other
		Q_INVOKABLE void insertIntelligentNewline();
		Q_INVOKABLE void insertIntelligentTabulator();
		Q_INVOKABLE void editBegin();
		Q_INVOKABLE void editEnd();

		// lists
		Q_INVOKABLE QStringList labelList() const;
		Q_INVOKABLE QStringList bibitemList() const;

		// Kile specific actions
		Q_INVOKABLE void refreshStructure();

	private:
		KileInfo* m_kileInfo;
		KTextEditor::View *m_view;
		KTextEditor::Document *m_document;
		KileDocument::EditorExtension *m_editor;
		const QMap<QString,QAction *> *m_scriptActions;

		QString getWord(const KTextEditor::Cursor &cursor);
		QString getLatexCommand(const KTextEditor::Cursor &cursor);

		int previousNonSpaceChar(const QString &s, int pos) const;
		int nextNonSpaceChar(const QString &s, int pos) const;

		bool triggerAction(const QString &name);
		bool triggerSelectionAction(const QString &name);

};

}

#endif

