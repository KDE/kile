/***************************************************************************
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

#include "docpart.h"

#include <kconfig.h>
#include <kstdaction.h>
#include <kglobal.h>
#include "kiledebug.h"
#include <kstandarddirs.h>
#include <kmimemagic.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <krun.h>
#include <khtml_part.h>
#include <khtml_settings.h>

DocumentationViewer::DocumentationViewer(QWidget *parent, const char *name ) : KHTMLPart(parent,name, 0, 0, BrowserViewGUI)
{
	m_hpos = 0;
	KConfig konqConfig("konquerorrc");
	konqConfig.setGroup("HTML Settings");
	//const KHTMLSettings * set = settings();
	//( const_cast<KHTMLSettings *>(set) )->init( &konqConfig, false );
	QString rc = KGlobal::dirs()->findResource("appdata", "docpartui.rc");
	setXMLFile(rc);
	(void) KStdAction::back(this, SLOT(back()), actionCollection(),"Back" );
	(void) KStdAction::forward(this, SLOT(forward()), actionCollection(),"Forward" );
	(void) KStdAction::home(this, SLOT(home()), actionCollection(),"Home" );
}

DocumentationViewer::~DocumentationViewer() {}

void DocumentationViewer::urlSelected(const QString &url, int button, int state,const QString & target, KParts::URLArgs args)
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

void DocumentationViewer::home()
{
	if ( !m_history.isEmpty() ) openURL( KURL(m_history.first()) );
}

void DocumentationViewer::forward()
{
	if ( forwardEnable() ) 
	{
		++m_hpos;
		openURL( KURL( m_history[m_hpos]) );
		emit updateStatus( backEnable() , forwardEnable() );
	}
}


void DocumentationViewer::back()
{
	if ( backEnable() ) {
		--m_hpos;
		openURL( KURL(m_history[m_hpos]) );
		emit updateStatus( backEnable() , forwardEnable() );
	}
}


void DocumentationViewer::addToHistory( const QString & url )
{
	if ( m_history.count() > 0 )
	{
		while ( m_hpos < m_history.count()-1  )
				m_history.pop_back();
	}
	
	if ( !m_history.isEmpty() ) ++m_hpos;
	
	m_history.append(url);
	
	m_hpos = m_history.count()-1;
	emit updateStatus( backEnable() , forwardEnable() );
}


bool DocumentationViewer::backEnable()
{
	return m_hpos > 0;
}


bool DocumentationViewer::forwardEnable()
{
	return m_hpos < m_history.count()-1;
}

#include "docpart.moc"
