/***************************************************************************
                          kileapplication.cpp  -  description
                             -------------------
    begin                : Sun Apr 21 2002
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

#include "kileapplication.h"

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qframe.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qfont.h>
#include <qfontmetrics.h>

static QFrame *pix=0;
static QWidget *splash=0;


KileApplication::KileApplication()
{
    QRect screen = QApplication::desktop()->screenGeometry();
    QPixmap pm(KGlobal::dirs()->findResource("appdata","pics/kile_splash.png"));
  	 splash = new QWidget( 0, "splash", WDestructiveClose | WStyle_Customize | WStyle_NoBorder | WX11BypassWM | WStyle_StaysOnTop );
    pix=new QFrame(splash,"pix",QWidget::WStyle_NoBorder | QWidget::WStyle_Customize);
    pix->setBackgroundPixmap(pm);
    pix->setLineWidth(0);
    pix->setGeometry( 0,0,398, 129 );
	  splash->adjustSize();
	  splash->setCaption( "Kile" );
	  splash->move( screen.center() - QPoint( splash->width() / 2, splash->height() / 2 ) );
	  splash->show();
}
KileApplication::~KileApplication(){
}

void KileApplication::closeSplash()
{
splash->hide();
delete splash;
}
