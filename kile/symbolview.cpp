/***************************************************************************
                          symbolview.cpp  -  description
                             -------------------
    begin                : Sat Aug 17 2002
    copyright            : (C) 2002 by Pascal Brachet
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

#include "symbolview.h"

#include <qfileinfo.h>
#include <qpixmap.h>
#include <kstddirs.h>


//////////////////////////////////////////////////////////

SymbolView::SymbolView(int page,QWidget *parent, const char *name): KileIconView( parent, name )
{
codesymbol=="";
setGridX( 36 );
setGridY( 36);
setSpacing(2);
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
setItemTextPos(Right);
InitPage(page);
connect(this,SIGNAL(clicked(KileIconViewItem*)),this,SLOT(setSymbolCode(KileIconViewItem*)));
}
SymbolView::~SymbolView()
{
}

void SymbolView::InitPage(int page)
{
QString icon_name;
QPixmap pixmap;
KileIconViewItem* item;
switch (page)
 {
  case 1:
    {
    for ( uint i = 0; i <= 225; i++ )
     {
     icon_name="img"+QString::number(i+1)+".png";
     if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
        {
        item= new KileIconViewItem( this);
        item->setPixmap(pixmap);
        item->setText(code[i]);
        item->setKey(code[i]);
        }
     }
    }break;
  case 2:
    {
    for ( uint i = 247; i <=313 ; i++ )
     {
     icon_name="img"+QString::number(i+1)+".png";
     if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
        {
        item= new KileIconViewItem( this);
        item->setPixmap(pixmap);
        item->setText(code[i]);
        item->setKey(code[i]);
        }
     }
    }break;
 case 3:
    {
    for ( uint i = 314; i <= 371; i++ )
     {
     icon_name="img"+QString::number(i+1)+".png";
     if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
        {
        item= new KileIconViewItem( this);
        item->setPixmap(pixmap);
        item->setText(code[i]);
        item->setKey(code[i]);
        }
     }
    }break;
 case 4:
    {
    for ( uint i = 226; i <= 246; i++ )
     {
     icon_name="img"+QString::number(i+1)+".png";
     if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
        {
        item= new KileIconViewItem( this);
        item->setPixmap(pixmap);
        item->setText(code[i]);
        item->setKey(code[i]);
        }
     }
    }break;
 case 5:
    {
    for ( uint i = 0; i <= 39; i++ )
     {
     icon_name="img"+QString::number(i+1)+"greek.png";
     if ( pixmap.load(locate("appdata","mathsymbols/"+icon_name)) )
        {
        item= new KileIconViewItem( this);
        item->setPixmap(pixmap);
        item->setText(code[i+372]);
        item->setKey(code[i+372]);
        }
     }
    }break;
 }

}

QString SymbolView::getSymbolCode()
{
return codesymbol;
}

void SymbolView::setSymbolCode(KileIconViewItem* item)
{
if (item)
   {
   codesymbol=item->key();
   emit SymbolSelected();
   }
else codesymbol="";
}

#include "symbolview.moc"
