/***************************************************************************
                          syntaxlatex.h  -  description
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

#ifndef SYNTAXLATEX_H
#define SYNTAXLATEX_H


/**
  *@author Pascal Brachet
  */
#include <private/qrichtext_p.h>
#include <qcolor.h>

typedef  QColor ListColors[8];

class SyntaxLatex : public QTextPreProcessor
{
public:
    enum Ids {
    Standard=0,
	  Comment,
	  Command,
    Math,
    Structure,
    Envir
    };

    SyntaxLatex(QFont & efont, ListColors col);
    virtual ~SyntaxLatex();
    void process( QTextDocument *doc, QTextParagraph *string, int start, bool invalidate = TRUE );
    static const char * const keywords1[];
    static const char * const keywords2[];
    QTextFormat *format( int id );

    void addFormat( int id, QTextFormat *f );
    void removeFormat( int id );

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

#endif
