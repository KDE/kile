/***************************************************************************
    begin                : Fri Aug 1 2003
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

// dani 2005-11-22
//   - add some new symbols 
//   - rearranged source

#include "symbolview.h"

#include <qfileinfo.h>
#include <qimage.h>
#include <kimageeffect.h>
#include <kstandarddirs.h>

//////////////////////////////////////////////////////////

SymbolView::SymbolView(QWidget *parent, const char *name): KIconView( parent, name )
{
    setGridX( 36 );
    setGridY( 36);
    setSpacing(0);
    setWordWrapIconText (false);
    setShowToolTips (true);
    setResizeMode( Adjust );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( Auto );
    setAutoArrange(true);
    setSorting(false);
    setItemsMovable(false);
    setArrangement(LeftToRight);
    setAcceptDrops(false);
}

SymbolView::~SymbolView()
{}

void SymbolView::showPage(int page)
{
	clear();
	initPage(page);
}

void SymbolView::initPage(int page)
{
	switch (page)
	{
		case Relation:
			insertSymbols(0,225,0);
		break;
		
		case Arrow:
			insertSymbols(247,313,0);
			insertSymbols(433,444,12);
		break;
		
		case Misc:
			insertSymbols(314,371,0);
		break;
		
		case Delimiters:
			insertSymbols(226,246,0);
		break;
		
		case Greek:
			insertSymbols(0,39,372,"greek");
		break;
		
		case Special:
			insertSymbols(0,167,0,"special",Special);
		break;
	}
}

void SymbolView::insertSymbols(uint from, uint to, int offset, const QString &addition, int type)
{
	QString icon_name;
	QImage pixmap;
	KIconViewItem* item;
            
	for ( uint i=from; i<=to; ++i )
	{
		icon_name = "img" + QString::number(i+1) + addition + ".png";
		if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
		{
			item= new KIconViewItem( this);
// 			pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1); we don't need that (tbraun)
			item->setPixmap(pixmap);
			if(type == -1) // old style code array
				item->setKey(code[i+offset]);
			else if(type == Special)
				item->setKey(specialcode[i+offset]);
		}
	}
}


#include "symbolview.moc"
