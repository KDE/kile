/***************************************************************************
                          latexeditor.cpp  -  description
                             -------------------
    begin                : Sat Dec 29 2001
    copyright            : (C) 2001 by Pascal Brachet
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "latexeditor.h"
#include "syntaxlatex.h"
#include <private/qrichtext_p.h>
#include "parenmatcher.h"
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qaccel.h>
#include <kmessagebox.h>
#include <klocale.h>

LatexEditor::LatexEditor(QWidget *parent, const char *name,QFont & efont,bool parmatch, ListColors col) : QTextEdit(parent,name), hasError( FALSE )
{
    encoding="";  
    setPaletteBackgroundColor(col[0]);
    setPaletteForegroundColor(col[1]);
    viewport()->setBackgroundMode(PaletteBackground);
    document()->setSelectionColor( Error, red );
    document()->setSelectionColor( Step, yellow );
    document()->setSelectionColor( ParenMatcher::Match,col[7]  );
    document()->setSelectionColor( ParenMatcher::Mismatch, Qt::magenta );

    SyntaxLatex *sy=new SyntaxLatex(efont,col);
    setTextFormat(Qt::PlainText);
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( QTextCursor * ) ),
	     this, SLOT( cursorPosChanged( QTextCursor * ) ) );
    parenMatcher->setEnabled(parmatch);
    document()->addSelection( Error );
    document()->addSelection( Step );
    document()->setInvertSelectionText( Error, FALSE );
    document()->setInvertSelectionText( Step, FALSE );
    document()->addSelection( ParenMatcher::Match );
    document()->addSelection( ParenMatcher::Mismatch );
    document()->setInvertSelectionText( ParenMatcher::Match, FALSE );
    document()->setInvertSelectionText( ParenMatcher::Mismatch, FALSE );

    document()->setPreProcessor(sy);
    document()->setFormatter( new QTextFormatterBreakWords );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );
    setTabStopWidth( sy->format(0)->width('x') * 4 );
}
LatexEditor::~LatexEditor(){
delete parenMatcher;
}

bool LatexEditor::search( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor )
{
    if ( startAtCursor )
	return find( expr, cs, wo, forward );
    int dummy = 0;
    return find( expr, cs, wo, forward, &dummy, &dummy );
}

void LatexEditor::replace( const QString &r)
{
removeSelectedText();
insert( r, FALSE, FALSE );
}

void LatexEditor::gotoLine( int line )
{
setCursorPosition( line, 0 );
ensureCursorVisible();
}

void LatexEditor::commentSelection()
{
    QTextParagraph *start = document()->selectionStartCursor( QTextDocument::Standard ).paragraph();
    QTextParagraph *end = document()->selectionEndCursor( QTextDocument::Standard ).paragraph();
    if ( !start || !end )
	start = end = textCursor()->paragraph();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	start->insert( 0, "%" );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( QTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

void LatexEditor::indentSelection()
{
    QTextParagraph *start = document()->selectionStartCursor( QTextDocument::Standard ).paragraph();
    QTextParagraph *end = document()->selectionEndCursor( QTextDocument::Standard ).paragraph();
    if ( !start || !end )	start = end = textCursor()->paragraph();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	start->insert( 0, "\t" );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( QTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

void LatexEditor::uncommentSelection()
{
    QTextParagraph *start = document()->selectionStartCursor( QTextDocument::Standard ).paragraph();
    QTextParagraph *end = document()->selectionEndCursor( QTextDocument::Standard ).paragraph();
    if ( !start || !end )	start = end = textCursor()->paragraph();
    while ( start ) {
	while ( start->at( 0 )->c == "%" )
         {
         	setSelection( start->paragId(),0,start->paragId(),1);
          removeSelectedText();
         }
	if ( start == end ) break;
	start = start->next();
    }
    document()->removeSelection( QTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}
void LatexEditor::doChangeInterval()
{
    emit intervalChanged();
    QTextEdit::doChangeInterval();
}

void LatexEditor::cursorPosChanged( QTextCursor *c )
{
  if ( parenMatcher->match( c ) )
    repaintChanged();
  if ( hasError )
  {
    emit clearErrorMarker();
    hasError = FALSE;
  }
}
void LatexEditor::configChanged()
{
  document()->invalidate();
  viewport()->repaint( FALSE );
}


void LatexEditor::setErrorSelection( int line )
{
  QTextParagraph *p = document()->paragAt( line );
  if ( !p )
    return;
  QTextCursor c( document() );
  c.setParagraph( p );
  c.setIndex( 0 );
  document()->removeSelection( Error );
  document()->setSelectionStart( Error, c );
  c.gotoLineEnd();
  document()->setSelectionEnd( Error, c );
  hasError = TRUE;
  viewport()->repaint( FALSE );
}

void LatexEditor::setStepSelection( int line )
{
  QTextParagraph *p = document()->paragAt( line );
  if ( !p )
    return;
  QTextCursor c( document() );
  c.setParagraph( p );
  c.setIndex( 0 );
  document()->removeSelection( Step );
  document()->setSelectionStart( Step, c );
  c.gotoLineEnd();
  document()->setSelectionEnd( Step, c );
  viewport()->repaint( FALSE );
}

void LatexEditor::clearStepSelection()
{
  document()->removeSelection( Step );
  viewport()->repaint( FALSE );
}

void LatexEditor::changeSettings(QFont & new_font,bool new_parmatch,ListColors new_col )
{
    setPaletteBackgroundColor(new_col[0]);
    setPaletteForegroundColor(new_col[1]);
    viewport()->setBackgroundMode(PaletteBackground);
    document()->setSelectionColor( Error, red );
    document()->setSelectionColor( Step, yellow );
    document()->setSelectionColor( ParenMatcher::Match,new_col[7] );
    document()->setSelectionColor( ParenMatcher::Mismatch, Qt::magenta );
    viewport()->repaint( FALSE );   
    SyntaxLatex *new_sy=new SyntaxLatex(new_font,new_col);
    parenMatcher->setEnabled(new_parmatch);
    document()->setPreProcessor(new_sy);
}

QString LatexEditor::getEncoding()
{
 return encoding;
}

void LatexEditor::setEncoding(QString enc)
{
 encoding=enc;
}  


#include "latexeditor.moc"
