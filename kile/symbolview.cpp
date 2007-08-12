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
tbraun 2007-06-13
    - Added Most frequently used symbolview, including remembering icons upon restart, removing of least popular item and configurable max item count
*/

#include "symbolview.h"
#include "kileconfig.h"

#include <qimage.h>
#include <qstringlist.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <kconfig.h>

#include <qregexp.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qrect.h>
#include <qapplication.h>


SymbolView::SymbolView(QWidget *parent, int type, const char *name): KIconView( parent, name ),m_toolTip(0L)
{
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

void SymbolView::extract(const QString& key, int& refCnt)
{
	if(!key.isEmpty())
		refCnt = key.section('%',0,0).toInt();
	
	return;
}

void SymbolView::extract(const QString& key, int& refCnt, QString &cmd, QStringList &args, QStringList &pkgs)
{
	if(key.isEmpty())
		return;
	
	extract(key,refCnt);
	
	QRegExp rePkgs("(?:\\[(.*)\\])?\\{(.*)\\}");
	
	args.clear();
	pkgs.clear();
	
	cmd = key.section('%',1,1);
	QString text = key.section('%',2,2);
	
	if( text.find(rePkgs) != -1 )
	{
		args = QStringList::split(",",rePkgs.cap(1));
		pkgs = QStringList::split(",",rePkgs.cap(2));
	}
}

void SymbolView::showToolTip( QIconViewItem *item )
{
	removeToolTip(); 
 
     if ( !item )
     return;
	
	QString cmd, label;
	QStringList pkgs, args;
	int refCnt;
	
	extract(item->key(),refCnt,cmd,args,pkgs);
	
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
	
     m_toolTip = new QLabel(label, 0,"myToolTip",
			  WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM );
     m_toolTip->setFrameStyle( QFrame::Plain | QFrame::Box );
     m_toolTip->setLineWidth( 1 );
     m_toolTip->setAlignment( AlignLeft | AlignTop );
     m_toolTip->move( QCursor::pos() + QPoint( 14, 14 ) );
     m_toolTip->adjustSize();
     QRect screen = QApplication::desktop()->screenGeometry(
             QApplication::desktop()->screenNumber(QCursor::pos()));
     if (m_toolTip->x()+m_toolTip->width() > screen.right()) {
	     m_toolTip->move(m_toolTip->x()+screen.right()-m_toolTip->x()-m_toolTip->width(), m_toolTip->y());
     }
     if (m_toolTip->y()+m_toolTip->height() > screen.bottom()) {
	     m_toolTip->move(m_toolTip->x(), screen.bottom()-m_toolTip->y()-m_toolTip->height()+m_toolTip->y());
     }
     m_toolTip->setFont( QToolTip::font() );
     m_toolTip->setPalette( QToolTip::palette(), true );
     m_toolTip->show();
}

void SymbolView::removeToolTip()
{
    delete m_toolTip;
    m_toolTip = 0;
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
		case MFUS:
			fillWidget(MFUSprefix);
		break;
			
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
	
	QString code_symbol;
	QStringList args, pkgs;
	QIconViewItem *item = NULL;
	int count;
	bool math=false, bracket=false;

	if( (e->button() & Qt::LeftButton) == Qt::LeftButton && ( item = findItem( e->pos() ) ) )
	{
		bracket = (e->state() & Qt::ControlButton) ==  Qt::ControlButton;
		math = (e->state() & Qt::ShiftButton) ==  Qt::ShiftButton;
		
		extract(item->key(),count,code_symbol,args,pkgs);

		if (math == bracket)
			;
		else if(math)
			code_symbol = '$' + code_symbol + '$';
		else if(bracket)
			code_symbol = '{' + code_symbol + '}';
		
		emit(insertText(code_symbol,pkgs));
 		emit(addToList(item));
	}
	
	kdDebug() << "math is " << math << ", bracket is " << bracket << " and item->key() is " <<  ( item ? item->key() : "" ) << endl;
}

void SymbolView::fillWidget(const QString& prefix)
{
	kdDebug() << "===SymbolView::fillWidget(const QString& " << prefix <<  " )===" << endl;
	QImage image;
	KIconViewItem* item;
	QStringList refCnts,paths;
	
	if( prefix == MFUSprefix)
	{
		KConfig *config = KGlobal::config();
		config->setGroup(MFUSGroup);
		QString configPaths = config->readEntry("paths");
		QString configrefCnts = config->readEntry("counts");
		paths = QStringList::split(',',configPaths);
		refCnts = QStringList::split(',',configrefCnts);
		kdDebug() << "Read " << paths.count() << " paths and " << refCnts.count() << " refCnts" << endl;
		if( paths.count() != refCnts.count() )
		{
			kdDebug() << "error in saved LRU list" << endl;
			paths.clear();
			refCnts.clear();
		}
	}
	else
	{
		paths = KGlobal::dirs()->findAllResources("app_symbols", prefix + "/*.png",false,true);
	paths.sort();
		for( uint i = 0 ; i < paths.count() ; i++ )
			refCnts.append("1");
	}
	for ( uint i = 0; i < paths.count(); i++ )
	{
 		if ( image.load(paths[i]) )
		{
//   			kdDebug() << "path is " << paths[i] << endl;
			item = new KIconViewItem(this);
			item->setPixmap(image);
			item->setKey( refCnts[i] + '%' + image.text("Command") + '%' + image.text("Packages") + '%' + paths[i] );
			image = KImageEffect::blend(colorGroup().text(), image, 1); // destroys our png comments, so we do it after reading the comments
		}
		else
			kdDebug() << "Loading file " << paths[i] << " failed" << endl;
    	}
}

void SymbolView::writeConfig()
{
	QIconViewItem *item;
	QStringList paths,refCnts;
	

	KConfig *config = KGlobal::config();
	config->setGroup(MFUSGroup);

	if( KileConfig::clearMFUS() )
	{
		config->deleteEntry("paths");
		config->deleteEntry("counts");
	}
	else
	{
		for ( item = this->firstItem(); item; item = item->nextItem() )
		{
			refCnts.append(item->key().section('%',0,0));
			paths.append(item->key().section('%',3,3));
			kdDebug() << "path=" << paths.last() << ", count is " << refCnts.last() << endl;
		}
		config->writeEntry("paths",paths);
		config->writeEntry("counts",refCnts);
	}
}

void SymbolView::slotAddToList(const QIconViewItem *item)
{
	if( !item || !item->pixmap() )
		return;
		
	QIconViewItem *tmpItem;
	bool found=false;
	const QRegExp reCnt("^\\d+");
			
	kdDebug() << "===void SymbolView::slotAddToList(const QIconViewItem *" << item << " )===" << endl;
	
	for ( tmpItem = this->firstItem(); tmpItem; tmpItem = tmpItem->nextItem() )
	{
		if( item->key().section('%',1) == tmpItem->key().section('%',1) )
		{
			found=true;
			break;
		}
	}
	
	if( !found && ( this->count() + 1 ) > KileConfig::numSymbolsMFUS() ) // we check before adding the symbol
	{	
		int refCnt, minRefCnt=10000;
		QIconViewItem *unpopularItem = 0L;

		kdDebug() << "Removing most unpopular item" << endl;

		for ( tmpItem = this->firstItem(); tmpItem; tmpItem = tmpItem->nextItem() )
		{
			extract(tmpItem->key(),refCnt);

			if( refCnt < minRefCnt )
			{
				refCnt = minRefCnt;
				unpopularItem = tmpItem;
			}
		}
		kdDebug() << " minRefCnt is " << minRefCnt << endl;
		delete unpopularItem;
	}

	if( found )
	{
		kdDebug() << "item is already in the iconview" << endl;
		
		int refCnt;
		extract(tmpItem->key(),refCnt);
		
		QString key = tmpItem->key();
		key.replace(reCnt,QString::number(refCnt+1));
		tmpItem->setKey(key);
	}
	else
	{
		tmpItem = new KIconViewItem(this,QString::null,*(item->pixmap()));
		tmpItem->setKey(item->key());
    	}
}

#include "symbolview.moc"
