/***************************************************************************
                          docpart.cpp  -  description
                             -------------------
    begin                : Sun Jul 29 2001
    copyright            : (C) 2001 by Brachet Pascal
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

#include "docpart.h"
#include <kconfig.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <khtml_settings.h>
#include <qstring.h>
#include <qmessagebox.h>

docpart::docpart(QWidget *parent, const char *name ) : KHTMLPart(parent,name)
{
   hpos = 0;
   KConfig konqConfig("konquerorrc");
   konqConfig.setGroup("HTML Settings");
   const KHTMLSettings * set = settings();
   ( const_cast<KHTMLSettings *>(set) )->init( &konqConfig, false );
}
docpart::~docpart(){
}
void docpart::urlSelected( const QString &url, int button, int state,const QString &_target, KParts::URLArgs args )
{
  KHTMLPart::urlSelected (url, button, state,_target,args);
  KURL cURL = completeURL( url );
  openURL( cURL ) ;
  addToHistory( cURL.url() );
}
void docpart::home()
{
if ( !history.isEmpty() ) openURL( history.first() );
}
void docpart::forward()
{
  if ( forwardEnable() ) {
  	hpos++;
  	openURL( history[hpos] );
    emit updateStatus( backEnable() , forwardEnable() );
  }
}


void docpart::back()
{
  if ( backEnable() ) {
  	hpos--;
  	openURL( history[hpos] );
    emit updateStatus( backEnable() , forwardEnable() );
  }
}


void docpart::addToHistory( QString url )
{

   if ( history.count() > 0 )
	   while ( hpos < history.count()-1  )
  	 		history.pop_back();

   if ( !history.isEmpty() ) hpos++;

   history.append(url);

   hpos = history.count()-1;
   emit updateStatus( backEnable() , forwardEnable() );
}


bool docpart::backEnable()
{
   return hpos > 0;
}


bool docpart::forwardEnable()
{
   return hpos < history.count()-1;
}

#include "docpart.moc"
