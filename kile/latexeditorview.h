/***************************************************************************
                          latexeditorview.h  -  description
                             -------------------
    begin                : ven avr 11 2003
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

#ifndef LATEXEDITORVIEW_H
#define LATEXEDITORVIEW_H

#include <qwidget.h>
#include <qfont.h>
#include <qcolor.h>
#include "latexeditor.h"
#include "linenumberwidget.h"

typedef  QColor ListColors[8];
class LatexEditorView : public QWidget  {
   Q_OBJECT
public: 
	LatexEditorView(QWidget *parent, const char *name, QFont & efont,bool parmatch, bool line, ListColors col);
	~LatexEditorView();
	LatexEditor *editor;

	void changeSettings(QFont & new_font,bool new_parmatch, bool line, ListColors new_col);
private:
  LineNumberWidget* m_lineNumberWidget;
  void setLineNumberWidgetVisible( bool );
};

#endif
