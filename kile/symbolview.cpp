/***************************************************************************
                          symbolview.cpp  -  description
                             -------------------
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

#include "symbolview.h"

#include <qfileinfo.h>
#include <qpixmap.h>
#include <kstddirs.h>

//////////////////////////////////////////////////////////

SymbolView::SymbolView(int page,QWidget *parent, const char *name): KIconView( parent, name )
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

    InitPage(page);
}

SymbolView::~SymbolView()
{}

void SymbolView::InitPage(int page)
{
    QString icon_name;
    QPixmap pixmap;
    KIconViewItem* item;
    switch (page)
    {
    case 1:
        {
            for ( uint i = 0; i <= 225; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case 2:
        {
            for ( uint i = 247; i <=313 ; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case 3:
        {
            for ( uint i = 314; i <= 371; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case 4:
        {
            for ( uint i = 226; i <= 246; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case 5:
        {
            for ( uint i = 0; i <= 39; i++ )
            {
                icon_name="img"+QString::number(i+1)+"greek.png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    item->setPixmap(pixmap);
                    item->setKey(code[i+372]);
                }
            }
        }
        break;
    case 6:
        {
            for ( uint i = 401; i <= 433; i++ )
            {
                icon_name="img"+QString::number(i)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    item->setPixmap(pixmap);
                    item->setKey(code[i+11]);
                }
            }
        }
        break;
    }

}

#include "symbolview.moc"
