/***************************************************************************
                          kilefileselect.cpp  -  description
                             -------------------
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 by Pascal Brachet
    email                : 

from Kate (C) 2001 by Matt Newell

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilefileselect.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qtooltip.h>

#include <kiconloader.h>
#include <kurlcompletion.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kcharsets.h>
#include <kglobal.h>

KileFileSelect::KileFileSelect(QWidget *parent, const char *name ) : QWidget(parent,name)
{
  QVBoxLayout* lo = new QVBoxLayout(this);

  QHBox *hlow = new QHBox (this);
  hlow->setPaletteBackgroundColor(colorGroup().background());
  lo->addWidget(hlow);

  home = new QToolButton( hlow );
  home->setIconSet(SmallIconSet("gohome"));
  QToolTip::add(home, i18n("Home"));
  up = new QToolButton( hlow );
  up->setIconSet(SmallIconSet("up"));
  QToolTip::add(up, i18n("Up"));
  back = new QToolButton( hlow );
  back->setIconSet(SmallIconSet("back"));
  QToolTip::add(back, i18n("Back"));
  forward = new QToolButton( hlow );
  forward->setIconSet(SmallIconSet("forward"));
  QToolTip::add(forward, i18n("Forward"));

  QWidget* spacer = new QWidget(hlow);
  hlow->setStretchFactor(spacer, 1);
  hlow->setMaximumHeight(up->height());

  cmbPath = new KURLComboBox( KURLComboBox::Directories, true, this, "path combo" );
  cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  KURLCompletion* cmpl = new KURLCompletion(KURLCompletion::DirCompletion);
  cmbPath->setCompletionObject( cmpl );
  lo->addWidget(cmbPath);

  dir = new KDirOperator(QString::null, this, "operator");
  dir->setView(KFile::Simple);
  dir->setNameFilter( "*.tex *.bib *.sty *.cls *.mp" );
//  dir->actionCollection()->action( "delete" )-> (KShortcut (ALT+Key_Delete));
  lo->addWidget(dir);
  lo->setStretchFactor(dir, 2);

  comboEncoding = new QComboBox( FALSE, this, "comboEncoding" );
  QStringList availableEncodingNames(KGlobal::charsets()->availableEncodingNames());
  comboEncoding->setEditable( true );
  comboEncoding->insertStringList( availableEncodingNames );
  QToolTip::add(comboEncoding, i18n("Set encoding"));
  lo->addWidget(comboEncoding);


  connect( home, SIGNAL( clicked() ), dir, SLOT( home() ) );
  connect( up, SIGNAL( clicked() ), dir, SLOT( cdUp() ) );
  connect( back, SIGNAL( clicked() ), dir, SLOT( back() ) );
  connect( forward, SIGNAL( clicked() ), dir, SLOT( forward() ) );

  connect( cmbPath, SIGNAL( urlActivated( const KURL&  )),this,  SLOT( cmbPathActivated( const KURL& ) ));
  connect( cmbPath, SIGNAL( returnPressed( const QString&  )), this,  SLOT( cmbPathReturnPressed( const QString& ) ));
  connect(dir, SIGNAL(urlEntered(const KURL&)), this, SLOT(dirUrlEntered(const KURL&)) );

  connect(dir, SIGNAL(finishedLoading()),this, SLOT(dirFinishedLoading()) );


}

KileFileSelect::~KileFileSelect()
{
}

void KileFileSelect::setView(KFile::FileView view)
{
 dir->setView(view);
}

void KileFileSelect::cmbPathActivated( const KURL& u )
{
   dir->setURL( u, true );
}

void KileFileSelect::cmbPathReturnPressed( const QString& u )
{
   dir->setFocus();
   dir->setURL( KURL(u), true );
}

void KileFileSelect::dirUrlEntered( const KURL& u )
{
   cmbPath->removeURL( u );
   QStringList urls = cmbPath->urls();
   urls.prepend( u.url() );
   while ( urls.count() >= (uint)cmbPath->maxItems() )
      urls.remove( urls.last() );
   cmbPath->setURLs( urls );
}

void KileFileSelect::dirFinishedLoading()
{
   up->setEnabled( dir->actionCollection()->action( "up" )->isEnabled() );
   back->setEnabled( dir->actionCollection()->action( "back" )->isEnabled() );
   forward->setEnabled( dir->actionCollection()->action( "forward" )->isEnabled() );
   home->setEnabled( dir->actionCollection()->action( "home" )->isEnabled() );
}

void KileFileSelect::focusInEvent(QFocusEvent*)
{
   dir->setFocus();
}

void KileFileSelect::setDir( KURL u )
{
  dir->setURL(u, true);
}


#include "kilefileselect.moc"
