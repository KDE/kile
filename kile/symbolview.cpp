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
#include <qimage.h>
#include <kimageeffect.h>
#include <kstddirs.h>

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
    QString icon_name;
    QImage pixmap;
    KIconViewItem* item;
    
    switch (page)
    {
    case Relation:
        {
            for ( uint i = 0; i <= 225; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case Arrow:
        {
            for ( uint i = 247; i <=313 ; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case Misc:
        {
            for ( uint i = 314; i <= 371; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case Delimiters:
        {
            for ( uint i = 226; i <= 246; i++ )
            {
                icon_name="img"+QString::number(i+1)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1);
                    item->setPixmap(pixmap);
                    item->setKey(code[i]);
                }
            }
        }
        break;
    case Greek:
        {
            for ( uint i = 0; i <= 39; i++ )
            {
                icon_name="img"+QString::number(i+1)+"greek.png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1);
                    item->setPixmap(pixmap);
                    item->setKey(code[i+372]);
                }
            }
        }
        break;
    case Special:
        {
            for ( uint i = 401; i <= 433; i++ )
            {
                icon_name="img"+QString::number(i)+".png";
                if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
                {
                    item= new KIconViewItem( this);
                    pixmap = KImageEffect::blend(colorGroup().text(), pixmap, 1);
                    item->setPixmap(pixmap);
                    item->setKey(code[i+11]);
                }
            }
        }
        break;
    }

}

#include "symbolview.moc"
