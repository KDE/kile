/***************************************************************************
                          docpart.cpp  -  description
                             -------------------
    begin                : Sun Jul 29 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal, (C) 2004 by Jeroen Wijnhout
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

#include <kconfig.h>
#include <kstdaction.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmimemagic.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <krun.h>
#include <khtml_settings.h>

#include "docpart.h"

docpart::docpart(QWidget *parent, const char *name ) : KHTMLPart(parent,name)
{
	hpos = 0;
	KConfig konqConfig("konquerorrc");
	konqConfig.setGroup("HTML Settings");
	const KHTMLSettings * set = settings();
	( const_cast<KHTMLSettings *>(set) )->init( &konqConfig, false );
	QString rc = KGlobal::dirs()->findResource("appdata", "docpartui.rc");
	setXMLFile(rc);
	(void) KStdAction::back(this, SLOT(back()), actionCollection(),"Back" );
	(void) KStdAction::forward(this, SLOT(forward()), actionCollection(),"Forward" );
	(void) KStdAction::home(this, SLOT(home()), actionCollection(),"Home" );

}
docpart::~docpart(){
}

void docpart::urlSelected(const QString &url, int button, int state,const QString & target, KParts::URLArgs args)
{
	KURL cURL = completeURL(url);
	QString mime = KMimeType::findByURL(cURL).data()->name();

	//load this URL in the embedded viewer if KHTML can handle it, or when mimetype detection failed
	KService::Ptr service = KService::serviceByDesktopName("khtml");
	if ( ( mime == KMimeType::defaultMimeType() ) || (service && service->hasServiceType(mime)) )
	{
		KHTMLPart::urlSelected(url, button, state, target, args);
		openURL(cURL) ;
		addToHistory(cURL.url());
	}
	//KHTML can't handle it, look for an appropriate application
	else
	{
		KTrader::OfferList offers = KTrader::self()->query(mime, "Type == 'Application'");
		KService::Ptr ptr = offers.first();
		KURL::List lst;
		lst.append(cURL);
		if (ptr) KRun::run(*ptr, lst);
	}
}

void docpart::home()
{
if ( !history.isEmpty() ) openURL( KURL(history.first()) );
}
void docpart::forward()
{
  if ( forwardEnable() ) {
  	hpos++;
  	openURL( KURL( history[hpos]) );
    emit updateStatus( backEnable() , forwardEnable() );
  }
}


void docpart::back()
{
  if ( backEnable() ) {
  	hpos--;
  	openURL( KURL(history[hpos]) );
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
