/***************************************************************************
                          syntaxlatex.cpp  -  description
                             -------------------
    begin                : Sun Dec 30 2001
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

#include "syntaxlatex.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qapplication.h"
#include "qregexp.h"
#include "qmap.h"
#include "paragdata.h"

const char * const SyntaxLatex::keywords1[] = {
    "section{",
    "subsection{",
    "subsubsection{",
    "chapter{",
    "part{",
    "paragraph{",
    "subparagraph{",
    "section*{",
    "subsection*{",
    "subsubsection*{",
    "chapter*{",
    "part*{",
    "paragraph*{",
    "subparagraph*{",
    "label{",
    "includegraphics{",
    "includegraphics[",
    "includegraphics*{",
    "includegraphics*[",
    "include{",
    "input{",
    0
};
const char * const SyntaxLatex::keywords2[] = {
    "begin{",
    "end{",
    0
};

static QMap<int, QMap<QString, int > > *wordMap1 = 0;
static QMap<int, QMap<QString, int > > *wordMap2 = 0;
SyntaxLatex::SyntaxLatex(QFont & efont, ListColors col)
    : QTextPreProcessor(), lastFormat( 0 ), lastFormatId( -1 )
{
    int normalSize =efont.pointSize();
    QString normalFamily =efont.family();
    int normalWeight = efont.weight();
    addFormat( Standard, new QTextFormat( QFont(normalFamily , normalSize, normalWeight ), col[1] ) );
    addFormat( Comment, new QTextFormat( QFont( normalFamily, normalSize, normalWeight, TRUE ),col[2]) );
    addFormat( Math, new QTextFormat( QFont( normalFamily, normalSize, normalWeight ),col[3] ) );
    addFormat( Command, new QTextFormat( QFont( normalFamily, normalSize, normalWeight ),col[4] ));
    addFormat( Structure, new QTextFormat( QFont( normalFamily, normalSize,normalWeight  ),col[5]));
    addFormat( Envir, new QTextFormat( QFont( normalFamily, normalSize, normalWeight ),col[6] ));

    if ( wordMap1 )	return;

    wordMap1 = new QMap<int, QMap<QString, int> >;
    int len;
    for ( int i = 0; keywords1[ i ]; ++i ) {
	len = strlen( keywords1[ i ] );
	if ( !wordMap1->contains( len ) )
	    wordMap1->insert( len, QMap<QString, int >() );
	QMap<QString, int> &map = wordMap1->operator[]( len );
	map[ keywords1[ i ] ] = Structure;
    }

    if ( wordMap2 )	return;

    wordMap2 = new QMap<int, QMap<QString, int> >;
    for ( int i = 0; keywords2[ i ]; ++i ) {
	len = strlen( keywords2[ i ] );
	if ( !wordMap2->contains( len ) )
	    wordMap2->insert( len, QMap<QString, int >() );
	QMap<QString, int> &map = wordMap2->operator[]( len );
	map[ keywords2[ i ] ] = Envir;
    }
}

SyntaxLatex::~SyntaxLatex()
{

}

void SyntaxLatex::process( QTextDocument *doc, QTextParagraph *string, int, bool invalidate )
{

    QTextFormat *formatStandard = format( Standard );
    QTextFormat *formatComment = format( Comment );
    QTextFormat *formatCommand = format( Command );
    QTextFormat *formatMath = format( Math );
    QTextFormat *formatStructure = format( Structure );
    QTextFormat *formatEnvir = format( Envir);

    // states
    const int StateStandard = 0;
    const int StateComment = 1;
    const int StateMath = 2;
    const int StateCommand=3;


    QString buffer;

    int state = StateStandard;
    if ( string->prev() ) {
	if ( string->prev()->endState() == -1 )
	    process( doc, string->prev(), 0, FALSE );
	state = string->prev()->endState();
    }
    int i = 0;

    ParagData *paragData = (ParagData*)string->extraData();
    if ( paragData )
	paragData->parenList.clear();
    else
	paragData = new ParagData;

    QChar lastChar, chNext,ch;
    while ( TRUE ) {
	  ch = string->at( i )->c;
	  buffer += ch;
	  if ( i < string->length()-1 )	 chNext = string->at( i+1 )->c;

	switch ( state ) {
	case StateStandard: {
			switch (ch) {
			case '\\' :{
         string->setFormat( i, 1, formatCommand, FALSE );
         state=StateCommand;
				 }break;
			case '$' : {
         string->setFormat( i, 1, formatMath, FALSE );
         state=StateMath;
         if (chNext=='$')
         {
         i++;
         string->setFormat( i, 1, formatMath, FALSE );
         }
				}break;
			case '%' : {
         string->setFormat( i, 1, formatComment, FALSE );
         state=StateComment;
				} break;
	    case '(': case '[': case '{':{
		     if (ch=='{') paragData->parenList << Paren( Paren::Open, ch, i );
         string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
		    } break;
	    case ')': case ']': case '}':{
		     if (ch=='}') paragData->parenList << Paren( Paren::Closed, ch, i );
         string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
		    }break;
	     default: {
         string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
	             } break;
			}
   buffer = QString::null;
	} break;
	case StateComment: {
         string->setFormat( i, 1, formatComment, FALSE );
         state=StateComment;
         buffer = QString::null;
	    } break;

	case StateMath: {
			switch (ch) {
			case '$' : {
         string->setFormat( i, 1, formatMath, FALSE );
         state=StateStandard;
         if (chNext=='$')
         {
         i++;
         string->setFormat( i, 1, formatMath, FALSE );
         }
				}break;
	    case '(': case '[': case '{':{
		    if (ch=='{') paragData->parenList << Paren( Paren::Open, ch, i );
        string->setFormat( i, 1, formatMath, FALSE );
         state=StateMath;
		    } break;
	    case ')': case ']': case '}':{
		    if (ch=='}') paragData->parenList << Paren( Paren::Closed, ch, i );
        string->setFormat( i, 1, formatMath, FALSE );
         state=StateMath;
		    }break;
	     default: {
         string->setFormat( i, 1, formatMath, FALSE );
         state=StateMath;
	             } break;
			}
         buffer = QString::null;
	   } break;
	case StateCommand:{
			switch (ch) {
			case '$' : {
         if (lastChar=='\\')
         {
         string->setFormat( i, 1, formatCommand, FALSE );
         state=StateStandard;
         }
         else
         {
         string->setFormat( i, 1, formatMath, FALSE );
         state=StateMath;
         }
				}break;
			case '%' : {
         if (lastChar=='\\')
         {
         string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
         }
         else
         {
         string->setFormat( i, 1, formatComment, FALSE );
         state=StateComment;
         }
				} break;
      case ' ':{
         string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
				} break;
	    case '(': case '[': case '{':{
		    if (ch=='{') paragData->parenList << Paren( Paren::Open, ch, i );
        string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
	       int len = buffer.length();
	       if ( buffer.length() > 0 )
            {
    		    QMap<int, QMap<QString, int > >::Iterator it = wordMap1->find( len );
		        if ( it != wordMap1->end() )
            {
			      QMap<QString, int >::Iterator it2 = ( *it ).find( buffer );
			      if ( it2 != ( *it ).end() ) string->setFormat( i - buffer.length(), buffer.length(), formatStructure, FALSE );
		        }
    		    QMap<int, QMap<QString, int > >::Iterator itbis = wordMap2->find( len );
		        if ( itbis != wordMap2->end() )
            {
			      QMap<QString, int >::Iterator it2bis = ( *itbis ).find( buffer );
			      if ( it2bis != ( *itbis ).end() ) string->setFormat( i - buffer.length(), buffer.length(), formatEnvir, FALSE );
		        }
		        }	
		    } break;
	    case ')': case ']': case '}':{
		   if (ch=='}') paragData->parenList << Paren( Paren::Closed, ch, i );
        string->setFormat( i, 1, formatStandard, FALSE );
         state=StateStandard;
		    }break;
			case '\\' : case ',' : case ';': case '\'': case '\"' : case '`': case '^': case '~': {
         if (lastChar=='\\')
         {
         string->setFormat( i, 1, formatCommand, FALSE );
         state=StateStandard;
         }
         else
         {
         string->setFormat( i, 1, formatCommand, FALSE );
         state=StateCommand;
         }
				}break;
	     default: {
         string->setFormat( i, 1, formatCommand, FALSE );
         state=StateCommand;
	             } break;
			}
	   } break;
	}
	lastChar = ch;
	i++;
	if ( i >= string->length() )
	    break;
    }

    string->setExtraData( paragData );

    if ( state == StateComment ) {
	string->setEndState( StateStandard );
    } else if ( state == StateMath ) {
	string->setEndState( StateMath );
    } else {
	string->setEndState( StateStandard );
    }

    string->setFirstPreProcess( FALSE );

    if ( invalidate && string->next() &&
	 !string->next()->firstPreProcess() && string->next()->endState() != -1 ) {
	QTextParagraph *p = string->next();
	while ( p ) {
	    if ( p->endState() == -1 )
		return;
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

QTextFormat *SyntaxLatex::format( int id )
{
    if ( lastFormatId == id  && lastFormat )
	return lastFormat;

    QTextFormat *f = formats[ id ];
    lastFormat = f ? f : formats[ 0 ];
    lastFormatId = id;
    return lastFormat;
}

void SyntaxLatex::addFormat( int id, QTextFormat *f )
{
    formats.insert( id, f );
}

void SyntaxLatex::removeFormat( int id )
{
    formats.remove( id );
}

