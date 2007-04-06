/***************************************************************************
    begin                : Fri Aug 1 2003
    edit		 : Fri April 6 2007
    copyright            : (C) 2003 by Jeroen Wijnhout, 2006 - 2007 by Thomas Braun
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

/*
dani 2005-11-22
  - add some new symbols 
  - rearranged source

tbraun 2006-07-01
   - added tooltips which show the keys, copied from kfileiconview
   - reorganized the hole thing, more flexible png loading, removing the old big code_array, more groups

tbraun 2007-06-04
    - Send a warning in the logwidget if needed packages are not included for the command
*/

#include "symbolview.h"

#include <qimage.h>
#include <qstringlist.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <qregexp.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qrect.h>
#include <qapplication.h>


SymbolView::SymbolView(QWidget *parent, const char *name,int type): KIconView( parent, name )
{
    toolTip = 0;
    setGridX( 36 );
    setGridY( 36);
    setSpacing(5);
    setWordWrapIconText (false);
    setShowToolTips (false);
    setResizeMode( Adjust );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( Auto );
    setAutoArrange(true);
    setSorting(false);
    setItemsMovable(false);
    setArrangement(LeftToRight);
    setAcceptDrops(false);
    initPage(type);
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),SLOT( showToolTip( QIconViewItem * ) ) );
    connect( this, SIGNAL( onViewport() ),SLOT( removeToolTip() ) );
}

SymbolView::~SymbolView()
{
	removeToolTip();
}


void SymbolView::extractPkgs(const QString& text, QStringList &args, QStringList &pkgs)
{
	QRegExp rePkgs("(?:\\[(.*)\\])?\\{(.*)\\}");
	
	args.clear();
	pkgs.clear();
	
	if( !text.isEmpty() && text.find(rePkgs) != -1 )
	{
		args = QStringList::split(",",rePkgs.cap(1));
		pkgs = QStringList::split(",",rePkgs.cap(2));
	}
}

void SymbolView::showToolTip( QIconViewItem *item )
{
     delete toolTip;
     toolTip = 0;
 
     if ( !item )
     return;
	
	QString cmd, label, s;
	QStringList pkgs, args;
	
	cmd = item->key().section('%',0,0);
	s = item->key().section('%',1,1);
	extractPkgs(s,args,pkgs);
	
// 	kdDebug() << "cmd is " << cmd << ", packages are " << s << endl;
	
	label = i18n("Command: ") + cmd + "\n";
	
	if( pkgs.count() > 0 )
	{
		if(pkgs.count() == 1)
			label += i18n("Package: ");
		else
			label += i18n("Packages: ");
		
		for( uint i = 0; i < pkgs.count() ; i++ )
		{
			if( i < args.count() )
				label = label + "[" + args[i] + "]" + pkgs[i] + "\n";
			else
				label = label + pkgs[i] + "\n";
		}
	}
	
     toolTip = new QLabel(label, 0,"myToolTip",
			  WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM );
     toolTip->setFrameStyle( QFrame::Plain | QFrame::Box );
     toolTip->setLineWidth( 1 );
     toolTip->setAlignment( AlignLeft | AlignTop );
     toolTip->move( QCursor::pos() + QPoint( 14, 14 ) );
     toolTip->adjustSize();
     QRect screen = QApplication::desktop()->screenGeometry(
             QApplication::desktop()->screenNumber(QCursor::pos()));
     if (toolTip->x()+toolTip->width() > screen.right()) {
         toolTip->move(toolTip->x()+screen.right()-toolTip->x()-toolTip->width(), toolTip->y());
     }
     if (toolTip->y()+toolTip->height() > screen.bottom()) {
         toolTip->move(toolTip->x(), screen.bottom()-toolTip->y()-toolTip->height()+toolTip->y());
     }
     toolTip->setFont( QToolTip::font() );
     toolTip->setPalette( QToolTip::palette(), true );
     toolTip->show();
}

void SymbolView::removeToolTip()
{
    delete toolTip;
    toolTip = 0;
}

void SymbolView::hideEvent( QHideEvent *e )
{
    removeToolTip();
    KIconView::hideEvent( e );
}

void SymbolView::initPage(int page)
{
	switch (page)
	{
		case Relation:
			fillWidget("relation");
		break;

		case Operator:
			fillWidget("operators");
		break;
		
		case Arrow:
			fillWidget("arrows");
		break;

		case MiscMath:
			fillWidget("misc-math");
		break;
		
		case MiscText:
 			fillWidget("misc-text");
		break;
		
		case Delimiters:
			fillWidget("delimiters");
		break;
		
		case Greek:
			fillWidget("greek");
		break;
		
		case Special:
			fillWidget("special");
		break;

		case Cyrillic:
			fillWidget("cyrillic");
		break;

		case User:
			fillWidget("user");
		break;

		default:
			kdWarning() << "wrong argument in initPage()" << endl;
		break;
	}
}

void SymbolView::contentsMousePressEvent(QMouseEvent *e)
{
	kdDebug() << "===SymbolView::contentsMousePressEvent(QMouseEvent *e)===" << endl;
	
	QString code_symbol, s;
	QStringList args, pkgs;
	QIconViewItem *item = NULL;
	bool math=false, bracket=false;

	if( (e->button() & Qt::LeftButton) == Qt::LeftButton && ( item = findItem( e->pos() ) ) )
	{
		bracket = (e->state() & Qt::ControlButton) ==  Qt::ControlButton;
		math = (e->state() & Qt::ShiftButton) ==  Qt::ShiftButton;
		code_symbol = item->key().section('%',0,0);
		s = item->key().section('%',1,1);
		
		extractPkgs(s,args,pkgs);

		if (math == bracket)
			;
		else if(math)
			code_symbol = '$' + code_symbol + '$';
		else if(bracket)
			code_symbol = '{' + code_symbol + '}';
		
		emit(insertText(code_symbol,pkgs));
	}
	
	kdDebug() << "math is " << math << ", bracket is " << bracket << " and item->key() is " <<  ( item ? item->key() : "" ) << endl;
}


void SymbolView::fillWidget(const QString& prefix)
{
	kdDebug() << "===SymbolView::fillWidget(const QString& " << prefix <<  " )===" << endl;
	QImage image;
	KIconViewItem* item;
	QStringList paths = KGlobal::dirs()->findAllResources("app_symbols", prefix + "/*.png",false,true);
	paths.sort();
	for ( QStringList::Iterator it = paths.begin(); it != paths.end(); ++it )
	{
 		if ( image.load(*it) )
		{
//  			kdDebug() << "path is " << *it << endl;
			item = new KIconViewItem(this);
			item->setPixmap(image);
			item->setKey(image.text("Command") + '%' + image.text("Packages") );
			image = KImageEffect::blend(colorGroup().text(), image, 1); // destroys our png comments, so we do it after reading the comments
		}
    	}
}

#include "symbolview.moc"
