/***************************************************************************
                          syntaxlatex.h  -  description
                             -------------------
    begin                : Sun Dec 30 2001
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

#ifndef SYNTAXLATEX_H
#define SYNTAXLATEX_H


/**
  *@author Pascal Brachet
  */
#include <private/qrichtext_p.h>
#include <qsyntaxhighlighter.h> 
#include <qintdict.h>


class QColor;
class QFont;
class QTextEdit;

#define TEX_CAT0 '\\'
#define TEX_CAT1 '{'
#define TEX_CAT2 '}'
#define TEX_CAT3 '$'
#define TEX_CAT4 '&'
#define TEX_CAT6 '#'
#define TEX_CAT7 '^'
#define TEX_CAT8 '_'
#define TEX_CAT13 '~'
#define TEX_CAT14 '%'

#define VERBATIM_END "end{verbatim}"

struct LaTeXFormat
{
	QFont font;
	QColor color;
};

typedef  QColor ListColors[8];

class SyntaxLatex : public QSyntaxHighlighter
{
public:
    SyntaxLatex(QTextEdit *textEdit, ListColors col, QFont &efont);
    virtual ~SyntaxLatex();

    int highlightParagraph ( const QString & text, int endStateOfLastPara );
    void changeSettings(ListColors col, QFont &efont);
    
private:
		void setLaTeXFormat(int start, int count, int format);

		enum State {
      stStandard=0, stComment=1, stIgnoreSpaces=2, stControlSequence=3, stControlSymbol=4,
      stCommand=5, stMath=6, stMathControlSequence=7, stMathEnded=8, stStructure=9, stEnvir=10,
      stKeywordDetected=11, stEnvironmentDetected=12, stEndDetected = 16, stVerbatimDetected=13 ,
      stDeterminingEnv =14 , stVerbatimEnv = 15,  stDisplayMath =17, stPossibleMathEnding=18,
      stEquation=19, stPossibleEquationEnding=20, stEndState = 21
    };

    enum Formats {
			fmtStandard=0, fmtComment, fmtControlSequence, fmtMath, fmtKeyword, fmtEnvironment
		};
    
    static const char * const Keywords[];
    static const char * const Environment[];

    QIntDict<LaTeXFormat> dictFormats;
    QValueList<QChar> listVerbChars;
    QMap<QString, int> mapCommands;

};

#endif
