/***************************************************************************
                          latexeditor.cpp  -  description
                             -------------------
    begin                : Sat Dec 29 2001
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
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
#include <kapplication.h>
#include <qclipboard.h>
#include <qaccel.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <kdebug.h>

LatexEditor::LatexEditor(QWidget *parent, const char *name,QFont & efont,bool parmatch, ListColors col) : QTextEdit(parent,name), hasError( FALSE )
{
    encoding="";
    matchParens=parmatch;
    m_matching=false;
    setPaletteBackgroundColor(col[0]);
    setPaletteForegroundColor(col[1]);
    viewport()->setBackgroundMode(PaletteBackground);
    document()->setSelectionColor( selError, red );
    document()->setSelectionColor( selStep, yellow );
    document()->setSelectionColor( selParenMatch,col[7]  );
    document()->setSelectionColor( selParenMismatch, Qt::magenta );

    highlighter=new SyntaxLatex(this,col,efont);
    setTextFormat(Qt::PlainText);
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( int,int ) ),
	     this, SLOT( cursorPosChanged( int,int ) ) );

    document()->addSelection( selError );
    document()->addSelection( selStep );
    document()->setInvertSelectionText( selError, FALSE );
    document()->setInvertSelectionText( selStep, FALSE );
    document()->addSelection( selParenMatch);
    document()->addSelection( selParenMismatch);
    document()->setInvertSelectionText( selParenMatch ,FALSE);
    document()->setInvertSelectionText( selParenMismatch , FALSE);

    document()->setFormatter( new QTextFormatterBreakWords );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );
    QFontMetrics fmet(efont);
    setTabStopWidth( fmet.width('x') * 4 );

    m_FileInfo=0;
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
	int para_From, para_To, index_From, index_To;
	getSelection(&para_From,&index_From,&para_To,&index_To);
	if (para_From < 0) return;

	for (int para = para_From; para<= para_To; para++)
	{
		insertAt("%",para,0);
	}

	repaintChanged();
}

void LatexEditor::indentSelection()
{
	int para_From, para_To, index_From, index_To;
	getSelection(&para_From,&index_From,&para_To,&index_To);
	if (para_From < 0) return;

	for (int para = para_From; para<= para_To; para++)
	{
		insertAt("\t",para,0);
	}

	repaintChanged();
}

void LatexEditor::uncommentSelection()
{
	QString line;
	int para_From, para_To, index_From, index_To, i,
		para_Cursor, index_Cursor;

	//save the cursor position
	getCursorPosition(&para_Cursor,&index_Cursor);

	getSelection(&para_From,&index_From,&para_To,&index_To);
	removeSelection();
	if (para_From < 0) return;

	for (int para=para_From; para<= para_To; para++)
	{
		line=text(para);
		i=0;
		while ( line.at(i).isSpace() )
		{
			i++;
		}
		if ( line.at(i) == '%')
		{
			setCursorPosition(para,i);
			del();
		}
	}
	//restore the cursor position
	setCursorPosition(para_Cursor,index_Cursor);
	repaintChanged();
}

/*void LatexEditor::doChangeInterval()
{
    emit intervalChanged();
    QTextEdit::doChangeInterval();
}*/

void LatexEditor::cursorPosChanged( int para, int pos  )
{
  if (m_matching)
  {
		m_matching=false;
	}
	else
  if (matchParens)
  {
		QChar ch = text(para)[pos];
		switch (ch)
		{
			case TEX_CAT1 : matchParen(para,pos,1); break;
			case TEX_CAT2 : matchParen(para,pos,-1); break;
			default : break;
		}
	}

  if ( hasError )
  {
    emit clearErrorMarker();
    hasError = FALSE;
  }
}

void LatexEditor::matchParen(int para, int pos, int direc)
{
	m_matching=true;

	QChar ch, target, opposite;
	int len = text(para).length()-1, howmany=0, beginpara=para, beginpos=pos,
		maxpar = paragraphs(), paraprocessed=0;
	bool ignore;

	//kdDebug() << "--------------------------------------------" << endl;
	//kdDebug() << "entering matchParen at " << para << " " << pos << endl;

	if (text(para)[pos-1] == TEX_CAT0 )
	{
		//kdDebug() << "this brace is escaped, leaving" <<endl;
		return;
	}


	if ( direc == 1)
	{
		target = TEX_CAT2;
		opposite = TEX_CAT1;
	}
	else
	{
		target = TEX_CAT1;
		opposite = TEX_CAT2;
	}

	//kdDebug() << "using: target " << target.latin1() << " opposite " << opposite.latin1() << endl;

	while ( m_matching )
	{
		pos += direc;

		if (pos < 0 || pos > len)
		{
			//kdDebug() << "position " << pos << endl;
  			para += direc;
			pos = (direc==1) ? 0 : text(para).length()-1;
			paraprocessed++;
			len = text(para).length()-1;
			//if we processed a few paragraphs process events
			//so that the user can abort the matching by changing the
			//cursor position
			if (paraprocessed > MAX_PARAPROCESSED)
			{
				kapp->processEvents();
			}
			//kdDebug() << "at paragraph " << para << " length "<< text(para).length()<< endl;
		}

		if (para <0 || (para == maxpar)) break;

		ch = text(para)[pos];

		//check for TEX_CAT0 \{ is a literal { for example
		ignore=false;
		if (text(para)[pos-1] == TEX_CAT0 )
		{
			ignore=true;
			//QChar charretje =  text(para)[pos-1];
			//kdDebug() << "ignoring " << charretje.latin1() << endl;
		}

		if (!ignore)
		{
			if ( ch == opposite )
			{
				howmany++;
				//kdDebug() << "opposite found at " << para << " " << pos << endl;
			}
			if ( ch == target )
			{
				howmany--;
				//kdDebug() << "target found at " << para << " " << pos << endl;
			}
		}

		if ( howmany < 0 ) break;
	}

	//kdDebug() << "how many? " << howmany << endl;

	//abort if we breaked from the while loop by m_matching=false
	//(meaning the cursor position was changed)
	if (!m_matching) {return;}

	if (howmany < 0)
	{
		(direc==1) ? setSelection(beginpara,beginpos, para,pos+1 , selParenMatch) :
	  	           setSelection(para, pos, beginpara,beginpos+1 , selParenMatch);
	}
	else
	{
		setSelection(beginpara,beginpos, beginpara,beginpos+1,selParenMismatch);
	}

	m_matching=false;
	//kdDebug() << "leaving matchParen at " << para << " " << pos << endl;
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
  document()->removeSelection( selError );
  document()->setSelectionStart( selError, c );
  c.gotoLineEnd();
  document()->setSelectionEnd( selError, c );
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
  document()->removeSelection( selStep );
  document()->setSelectionStart( selStep, c );
  c.gotoLineEnd();
  document()->setSelectionEnd( selStep, c );
  viewport()->repaint( FALSE );
}

void LatexEditor::clearStepSelection()
{
  document()->removeSelection( selStep );
  viewport()->repaint( FALSE );
}

void LatexEditor::changeSettings(QFont & new_font,bool new_parmatch,ListColors new_col )
{
    setPaletteBackgroundColor(new_col[0]);
    setPaletteForegroundColor(new_col[1]);
    viewport()->setBackgroundMode(PaletteBackground);
    document()->setSelectionColor( selError, red );
    document()->setSelectionColor( selStep, yellow );
    document()->setSelectionColor( selParenMatch,new_col[7] );
    document()->setSelectionColor( selParenMismatch, Qt::magenta );
    viewport()->repaint( FALSE );   
    highlighter->changeSettings(new_col,new_font);
    matchParens=new_parmatch;
}

QString LatexEditor::getEncoding()
{
 return encoding;
}

void LatexEditor::setEncoding(QString enc)
{
 encoding=enc;
}  

void LatexEditor::setFile(const QString &name)
{
	if(!m_FileInfo)
	{
		m_FileInfo = new QFileInfo(name);
	}
	else
	{
		m_FileInfo->setFile(name);
	}
}

#include "latexeditor.moc"
