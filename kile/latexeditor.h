/***************************************************************************
                          latexeditor.h  -  description
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

#ifndef LATEXEDITOR_H
#define LATEXEDITOR_H

#include <qwidget.h>
#include <qtextedit.h>
#include <qstring.h>
#include <qcolor.h>
#include "parenmatcher.h"

#define MAX_PARAPROCESSED 10

typedef  QColor ListColors[8];

class SyntaxLatex;
class QFileInfo;

class LatexEditor : public QTextEdit  {
   Q_OBJECT
public:
  enum Selection
  {
	selParenMismatch =1,
	selParenMatch =2,
	selError = 3,
	selStep = 4
  };

	LatexEditor(QWidget *parent, const char *name, QFont & efont,bool parmatch, ListColors col);
	~LatexEditor();
   void gotoLine( int line );
   bool search( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor );
   void replace( const QString &r);

   void setFile(const QString &);

  QTextDocument *document() const { return QTextEdit::document(); }
  void placeCursor( const QPoint &p, QTextCursor *c ) { QTextEdit::placeCursor( p, c ); }
  void setDocument( QTextDocument *doc ) { QTextEdit::setDocument( doc ); }
  QTextCursor *textCursor() const { return QTextEdit::textCursor(); }
  void repaintChanged() { QTextEdit::repaintChanged(); }

  virtual void configChanged();

  void setErrorSelection( int line );
  void setStepSelection( int line );
  void clearStepSelection();
  void clearSelections();

  virtual bool supportsErrors() const { return TRUE; }
  virtual bool supportsBreakPoints() const { return TRUE; }
  virtual void makeFunctionVisible( QTextParag * ) {}

  void drawCursor( bool b ) { QTextEdit::drawCursor( b ); }
  void commentSelection();
  void uncommentSelection();
  void indentSelection();
  void changeSettings(QFont & new_font,bool new_parmatch,ListColors new_col);
  QString getEncoding();
  void setEncoding(QString enc);

  QFileInfo * fileInfo() {return m_FileInfo;}

signals:
    void clearErrorMarker();
    //void intervalChanged();

public slots:
		void setParenMatching(bool b) {matchParens=b;}
		
private slots:
    void cursorPosChanged( int para,int pos );
    //void doChangeInterval();

private:
	void matchParen(int para, int pos, int direc);

	
protected:
  ParenMatcher *parenMatcher;
  bool hasError;

private:
	QFileInfo *m_FileInfo;
	bool matchParens;
	bool m_matching;
	QString encoding;
	SyntaxLatex *highlighter;

};

#endif
