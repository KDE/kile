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

#include "scripting/kilescriptdocument.h"

#include "kileinfo.h"
#include "kileactions.h"

namespace KileScript {


KileScriptDocument::KileScriptDocument(QObject *parent, KileInfo* kileInfo, KileDocument::EditorExtension *editor, const QMap<QString,QAction *> *scriptActions)
   : QObject(parent), m_kileInfo(kileInfo), m_view(0), m_document(0), m_editor(editor), m_scriptActions(scriptActions)
{
}

////////////////////////////////// view/document //////////////////////////////////////

void KileScriptDocument::setView(KTextEditor::View *view)
{
	m_view = view;
	m_document = m_view->document();
}

////////////////////////////////// insert/remove/replace //////////////////////////////////////

void KileScriptDocument::insertText(const QString &s)
{
	QString cursorpos =  ( s.indexOf("%C") >= 0 ) ? QString() : "%C";
	m_editor->insertTag(KileAction::TagData(QString(),s,cursorpos,0,0), m_view);
}

void KileScriptDocument::insertText(int line, int column, const QString &s)
{
	insertText(KTextEditor::Cursor(line, column),s);
}

void KileScriptDocument::insertText(const KTextEditor::Cursor& cursor, const QString &s)
{
	m_view->setCursorPosition(cursor);
	insertText(s);
}

bool KileScriptDocument::removeText(int fromLine, int fromColumn, int toLine, int toColumn)
{
	return removeText(KTextEditor::Range(fromLine, fromColumn, toLine, toColumn));
}

bool KileScriptDocument::removeText(const KTextEditor::Cursor& from, const KTextEditor::Cursor& to)
{
	return removeText(KTextEditor::Range(from, to));
}

bool KileScriptDocument::removeText(const KTextEditor::Range& range)
{
	return m_document->removeText(range);
}

bool KileScriptDocument::replaceText(const KTextEditor::Range& range, const QString &text)
{
	return m_document->replaceText(range,text);
}

/////////////////////////////// document //////////////////////////////

int KileScriptDocument::lines()
{
	return m_document->lines();
}

int KileScriptDocument::length()
{
	return m_document->totalCharacters();
}

KTextEditor::Range KileScriptDocument::documentRange()
{
	return m_document->documentRange();
}

KTextEditor::Cursor KileScriptDocument::documentEnd()
{
	return m_document->documentEnd();
}

/////////////////////////////// text //////////////////////////////

QString KileScriptDocument::text()
{
	return m_document->text();
}

QString KileScriptDocument::text(int fromLine, int fromColumn, int toLine, int toColumn)
{
	return text( KTextEditor::Range(fromLine, fromColumn, toLine, toColumn) );
}

QString KileScriptDocument::text(const KTextEditor::Cursor& from, const KTextEditor::Cursor& to)
{
	return text( KTextEditor::Range(from, to) );
}

QString KileScriptDocument::text(const KTextEditor::Range& range)
{
	return m_document->text(range);
}

bool KileScriptDocument::setText(const QString &s)
{
	return m_document->setText(s);
}

bool KileScriptDocument::clear()
{
	return m_document->clear();
}

/////////////////////////////// line //////////////////////////////

QString KileScriptDocument::line(int line)
{
	return m_document->line(line);
}

QString KileScriptDocument::line()
{
	return line( m_view->cursorPosition().line() );
}

int KileScriptDocument::lineLength()
{
	return lineLength( m_view->cursorPosition().line() );
}

int KileScriptDocument::lineLength(int line)
{
	return m_document->lineLength(line);
}

bool KileScriptDocument::insertLine(const QString &s)
{
	return insertLine(m_view->cursorPosition().line(), s);
}

bool KileScriptDocument::insertLine(int line, const QString &s)
{
	return m_document->insertLine(line, s);
}

bool KileScriptDocument::removeLine()
{
	return removeLine( m_view->cursorPosition().line() );
}

bool KileScriptDocument::removeLine(int line)
{
	return m_document->removeLine(line);
}

bool KileScriptDocument::replaceLine(const QString &s)
{
	return replaceLine( m_view->cursorPosition().line(),s );
}

bool KileScriptDocument::replaceLine(int line,const QString &s)
{
	return m_editor->replaceLine(line,s,m_view);
}

bool KileScriptDocument::truncateLine()
{
	return truncate( m_view->cursorPosition() );
}

bool KileScriptDocument::truncate(int line, int column)
{
	QString textline = m_document->line(line);
	if ( textline.isEmpty() || textline.length()<column ) {
		return false;
	}

	return removeText( KTextEditor::Range(line,column,line,textline.length()) );
}

bool KileScriptDocument::truncate(const KTextEditor::Cursor& cursor)
{
	return truncate(cursor.line(), cursor.column());
}

/////////////////////////////// word //////////////////////////////

QString KileScriptDocument::word()
{
	return getWord( m_view->cursorPosition() );
}

QString KileScriptDocument::wordAt(int line, int column)
{
	return getWord( KTextEditor::Cursor(line, column) );
}

QString KileScriptDocument::wordAt(const KTextEditor::Cursor& cursor)
{
	return getWord(cursor);
}

KTextEditor::Range KileScriptDocument::wordRange()
{
	return m_editor->wordRange( m_view->cursorPosition(),false,m_view );
}

QString KileScriptDocument::getWord(const KTextEditor::Cursor &cursor)
{
	return m_editor->word(cursor,false,m_view);
}

/////////////////////////////// latex command //////////////////////////////

QString KileScriptDocument::latexCommand()
{
	return getLatexCommand( m_view->cursorPosition() );
}

QString KileScriptDocument::latexCommandAt(int line, int column)
{
	return getLatexCommand( KTextEditor::Cursor(line, column) );
}

QString KileScriptDocument::latexCommandAt(const KTextEditor::Cursor &cursor)
{
	return getLatexCommand(cursor);
}

KTextEditor::Range KileScriptDocument::latexCommandRange()
{
	return m_editor->wordRange( m_view->cursorPosition(),true,m_view );
}

QString KileScriptDocument::getLatexCommand(const KTextEditor::Cursor &cursor)
{
	return m_editor->word(cursor,true,m_view);
}

/////////////////////////////// char //////////////////////////////

QString KileScriptDocument::charAt(int line, int column)
{
	return charAt(KTextEditor::Cursor(line,column));
}

QString KileScriptDocument::charAt(const KTextEditor::Cursor& cursor)
{
	return QString(m_document->character(cursor));
}

QString KileScriptDocument::firstChar(int line)
{
	QString textline = m_document->line(line);
	int pos =  nextNonSpaceChar(textline,0);
	return ( pos >= 0 ) ? QString(textline.at(pos)) : QString();
}

QString KileScriptDocument::lastChar(int line)
{
	QString textline = m_document->line(line);
	int pos = previousNonSpaceChar(textline,textline.length()-1);
	return ( pos >= 0 ) ? QString(textline.at(pos)) : QString();
}

bool KileScriptDocument::isSpace(int line, int column)
{
	return isSpace(KTextEditor::Cursor(line,column));
}

bool KileScriptDocument::isSpace(const KTextEditor::Cursor& cursor)
{
	return m_document->character(cursor).isSpace();
}

/////////////////////////////// bullet //////////////////////////////

void KileScriptDocument::insertBullet()
{
	m_editor->insertBullet(m_view);
}

void KileScriptDocument::nextBullet()
{
	m_editor->nextBullet(m_view);
}

void KileScriptDocument::previousBullet()
{
	m_editor->prevBullet(m_view);
}

/////////////////////////////// environment //////////////////////////////

bool KileScriptDocument::hasEnvironment()
{
	return m_editor->hasEnvironment(m_view);
}

QString KileScriptDocument::environmentName()
{
	return m_editor->environmentName(m_view);
}

QString KileScriptDocument::environment(bool inside)
{
	return m_editor->environmentText(inside,m_view);
}

KTextEditor::Range KileScriptDocument::environmentRange(bool inside)
{
	return m_editor->environmentRange(inside,m_view);
}

void KileScriptDocument::removeEnvironment(bool inside)
{
	m_editor->deleteEnvironment(inside,m_view);
}

void KileScriptDocument::closeEnvironment()
{
	m_editor->closeEnvironment(m_view);
}

void KileScriptDocument::closeAllEnvironments()
{
	m_editor->closeAllEnvironments(m_view);
}

////////////////////////////////// TexGroup //////////////////////////////////////

bool KileScriptDocument::hasTexgroup()
{
	return m_editor->hasTexgroup(m_view);
}

QString KileScriptDocument::texgroup(bool inside)
{
	return m_editor->getTexgroupText(inside,m_view);
}

KTextEditor::Range KileScriptDocument::texgroupRange(bool inside)
{
	return m_editor->texgroupRange(inside,m_view);
}

void KileScriptDocument::removeTexgroup(bool inside)
{
	return m_editor->deleteTexgroup(inside,m_view);
}

////////////////////////////////// MathGroup //////////////////////////////////////

bool KileScriptDocument::hasMathgroup()
{
	return m_editor->hasMathgroup(m_view);
}

QString KileScriptDocument::mathgroup()
{
	return m_editor->getMathgroupText(m_view);
}

KTextEditor::Range KileScriptDocument::mathgroupRange()
{
	return m_editor->mathgroupRange(m_view);
}

void KileScriptDocument::removeMathgroup()
{
	m_editor->deleteMathgroup(m_view);
}

////////////////////////////////// Paragraph //////////////////////////////////////

QString KileScriptDocument::paragraph()
{
	return m_editor->getParagraphText(m_view);
}

KTextEditor::Range KileScriptDocument::paragraphRange()
{
	return m_editor->findCurrentParagraphRange(m_view);
}

void KileScriptDocument::removeParagraph()
{
	m_editor->deleteParagraph(m_view);
}

////////////////////////////////// matches //////////////////////////////////////

bool KileScriptDocument::matchesAt(int line, int column, const QString &s)
{
	return matchesAt(KTextEditor::Cursor(line,column),s);
}

bool KileScriptDocument::matchesAt(const KTextEditor::Cursor& cursor, const QString &s)
{
	QString textline = m_document->line(cursor.line());
	return textline.mid(cursor.column()).startsWith(s);
}

bool KileScriptDocument::startsWith (int line, const QString &pattern, bool skipWhiteSpaces)
{
	QString textline = m_document->line(line);
	if ( skipWhiteSpaces ) {
		textline = textline.trimmed();
	}
	return textline.startsWith(pattern);
}

bool KileScriptDocument::endsWith (int line, const QString &pattern, bool skipWhiteSpaces)
{
	QString textline = m_document->line(line);
	if ( skipWhiteSpaces ) {
		textline = textline.trimmed();
	}
	return  textline.endsWith(pattern);
}

////////////////////////////////// colums/lines //////////////////////////////////////

int KileScriptDocument::firstColumn(int line)
{
	QString textline = m_document->line(line);
	return ( textline.isEmpty() ) ? -1 : nextNonSpaceChar(textline,0);
}

int KileScriptDocument::lastColumn(int line)
{
	QString textline = m_document->line(line);
	return ( textline.isEmpty() ) ? -1 : previousNonSpaceChar(textline,textline.length()-1);
}

int KileScriptDocument::prevNonSpaceColumn(int line, int column)
{
	QString textline = m_document->line(line);
	return ( textline.isEmpty() || column==0 ) ? -1 : previousNonSpaceChar(textline,column-1);
}

int KileScriptDocument::prevNonSpaceColumn(const KTextEditor::Cursor& cursor)
{
  return prevNonSpaceColumn(cursor.line(), cursor.column());
}

int KileScriptDocument::nextNonSpaceColumn(int line, int column)
{
	QString textline = m_document->line(line);
	return ( textline.isEmpty() || column>=textline.length() ) ? -1 : nextNonSpaceChar(textline,column+1);
}

int KileScriptDocument::nextNonSpaceColumn(const KTextEditor::Cursor& cursor)
{
	return nextNonSpaceColumn(cursor.line(), cursor.column());
}

int KileScriptDocument::prevNonEmptyLine(int line)
{
	return m_editor->prevNonEmptyLine(line,m_view);
}

int KileScriptDocument::nextNonEmptyLine(int line)
{
	return m_editor->nextNonEmptyLine(line,m_view);
}

int KileScriptDocument::previousNonSpaceChar(const QString &s, int pos) const
{
	if ( pos >= s.length() ) {
		pos = s.length() - 1;
	}

	for (int i=pos; i>=0; --i ) {
		if ( !s[i].isSpace() ) {
			return i;
		}
	}
	return -1;
}

int KileScriptDocument::nextNonSpaceChar(const QString &s, int pos) const
{
	if ( pos < 0 ) {
		pos = 0;
	}

	for ( int i=pos; i<s.length(); ++i ) {
		if ( !s[i].isSpace() ) {
			return i;
		}
	}
	return -1;
}

////////////////////////////////// goto //////////////////////////////////////

void KileScriptDocument::gotoBeginEnv()
{
	m_editor->gotoEnvironment(true,m_view);
}

void KileScriptDocument::gotoEndEnv()
{
	m_editor->gotoEnvironment(false,m_view);
}

void KileScriptDocument::gotoBeginTexgroup()
{
	m_editor->gotoTexgroup(true,m_view);
}

void KileScriptDocument::gotoEndTexgroup()
{
	m_editor->gotoTexgroup(false,m_view);
}

void KileScriptDocument::gotoNextParagraph()
{
	m_editor->gotoNextParagraph(m_view);
}

void KileScriptDocument::gotoPrevParagraph()
{
	m_editor->gotoPrevParagraph(m_view);
}

void KileScriptDocument::gotoNextSectioning()
{
	m_editor->gotoSectioning(false,m_view);
}

void KileScriptDocument::gotoPrevSectioning()
{
	m_editor->gotoSectioning(true,m_view);
}

void KileScriptDocument::gotoLine(int line)
{
	m_editor->goToLine(line,m_view);
}

////////////////////////////////// insert sectioning tags  //////////////////////////////////////

void KileScriptDocument::insertChapter()
{
	triggerAction("tag_chapter");
}

void KileScriptDocument::insertSection()
{
	triggerAction("tag_section");
}

void KileScriptDocument::insertSubsection()
{
	triggerAction("tag_subsection");
}

void KileScriptDocument::insertSubsubsection()
{
	triggerAction("tag_subsubsection");
}

void KileScriptDocument::insertParagraph()
{
	triggerAction("tag_paragraph");
}

void KileScriptDocument::insertSubparagraph()
{
	triggerAction("tag_subparagraph");
}

////////////////////////////////// insert reference tags  //////////////////////////////////////

void KileScriptDocument::insertLabel()
{
	triggerAction("tag_label");
}

void KileScriptDocument::insertReference()
{
	triggerAction("tag_ref");
}

void KileScriptDocument::insertPageref()
{
	triggerAction("tag_pageref");
}

void KileScriptDocument::insertIndex()
{
	triggerAction("tag_index");
}

void KileScriptDocument::insertFootnote()
{
	triggerAction("tag_footnote");
}

void KileScriptDocument::insertCitation()
{
	triggerAction("tag_cite");
}

bool KileScriptDocument::triggerAction(const QString &name)
{
	if ( m_scriptActions->contains(name) ) {
		m_scriptActions->value(name)->trigger();
		return true;
	}
	else {
		return false;
	}
}

////////////////////////////////// actions with selections//////////////////////////////////////

void KileScriptDocument::comment()
{
	triggerSelectionAction("tools_comment");
}

void KileScriptDocument::uncomment()
{
	triggerSelectionAction("tools_uncomment");
}

void KileScriptDocument::uppercase()
{
	triggerSelectionAction("tools_uppercase");
}

void KileScriptDocument::lowercase()
{
	triggerSelectionAction("tools_lowercase");
}

void KileScriptDocument::capitalize()
{
	triggerSelectionAction("tools_capitalize");
}

void KileScriptDocument::joinLines()
{
	triggerSelectionAction("tools_join_lines");
}

bool KileScriptDocument::triggerSelectionAction(const QString &name)
{
	return ( m_view->selection() ) ? triggerAction(name) : false;
}

////////////////////////////////// other  //////////////////////////////////////

void KileScriptDocument::insertIntelligentNewline()
{
	m_editor->insertIntelligentNewline(m_view);
}

void KileScriptDocument::insertIntelligentTabulator()
{
	m_editor->insertIntelligentTabulator(m_view);
}

void KileScriptDocument::editBegin()
{
	m_document->startEditing();
}
void KileScriptDocument::editEnd()
{
	m_document->endEditing();
}

////////////////////////////////// Kile specific actions //////////////////////////////////////

void KileScriptDocument::refreshStructure()
{
	triggerAction("refreshstructure");
}

////////////////////////////////// lists //////////////////////////////////////

QStringList KileScriptDocument::labelList() const
{
	return m_kileInfo->allLabels();
}

QStringList KileScriptDocument::bibitemList() const
{
	return m_kileInfo->allBibItems();
}





}

#include "kilescriptdocument.moc"
