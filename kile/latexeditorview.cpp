/***************************************************************************
                          latexeditorview.cpp  -  description
                             -------------------
    begin                : ven avr 11 2003
    copyright            : (C) 2003 by Pascal Brachet
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

#include "latexeditorview.h"
#include <qlayout.h>

LatexEditorView::LatexEditorView(QWidget *parent, const char *name,QFont & efont,bool parmatch, bool line, ListColors col ) : QWidget(parent,name)
{
  editor=new LatexEditor(this,name,efont,parmatch,col);
  m_lineNumberWidget = new LineNumberWidget( editor, this,"");
  m_lineNumberWidget->setFont(efont);
  QFontMetrics fm( efont );
	m_lineNumberWidget->setFixedWidth( fm.width( "00000" ) + 10 );
  QHBoxLayout* lay = new QHBoxLayout( this );
  lay->addWidget( m_lineNumberWidget );
  lay->addWidget( editor );
  setFocusProxy( editor );
  setLineNumberWidgetVisible(line);
}
LatexEditorView::~LatexEditorView()
{
}

void LatexEditorView::setLineNumberWidgetVisible( bool b )
{
    if( b ){
	m_lineNumberWidget->show();
    } else {
	m_lineNumberWidget->hide();
    }
}

void LatexEditorView::changeSettings(QFont & new_font,bool new_parmatch, bool line, ListColors new_col)
{
  editor->changeSettings(new_font,new_parmatch,new_col);
  m_lineNumberWidget->setFont(new_font);
  QFontMetrics fm( new_font );
	m_lineNumberWidget->setFixedWidth( fm.width( "00000" ) + 10 );
  setLineNumberWidgetVisible(line);
}

#include "latexeditorview.moc"
