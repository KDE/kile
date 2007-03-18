/***************************************************************************
    begin                : Wed Aug 14 2002
    copyright            : (C) 2003 by Jeroen Wijnhout
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

// 2007-03-12 dani
//  - use KileDocument::Extensions

#include "kilefileselect.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qtooltip.h>

#include <ktoolbar.h>
#include <kiconloader.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kcharsets.h>
#include <kdebug.h>

#include "kileconfig.h"

KileFileSelect::KileFileSelect(KileDocument::Extensions *extensions, QWidget *parent, const char *name ) : QWidget(parent,name)
{
  QVBoxLayout* lo = new QVBoxLayout(this);

  KToolBar *toolbar = new KToolBar(this, "fileselectortoolbar");
  lo->addWidget(toolbar);

  cmbPath = new KURLComboBox( KURLComboBox::Directories, true, this, "path combo" );
  cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  cmpl = new KURLCompletion(KURLCompletion::DirCompletion);
  cmbPath->setCompletionObject( cmpl );
  lo->addWidget(cmbPath);

  dir = new KDirOperator(KURL(), this, "operator");
  connect(dir, SIGNAL(fileSelected(const KFileItem*)), this, SIGNAL(fileSelected(const KFileItem*)));
  dir->setView(KFile::Simple);
  dir->setMode(KFile::Files);

	// KileFileSelect filter for sidebar 
	QString filter =  extensions->latexDocuments() 
	                    + " " + extensions->latexPackages() 
	                    + " " + extensions->bibtex() 
	                    + " " +  extensions->metapost();
	filter.replace(".","*.");
	dir->setNameFilter(filter);

  KActionCollection *coll = dir->actionCollection();
  // some shortcuts of diroperator that clashes with Kate
  coll->action( "delete" )->setShortcut( KShortcut( ALT + Key_Delete ) );
  coll->action( "reload" )->setShortcut( KShortcut( ALT + Key_F5 ) );
  coll->action( "back" )->setShortcut( KShortcut( ALT + SHIFT + Key_Left ) );
  coll->action( "forward" )->setShortcut( KShortcut( ALT + SHIFT + Key_Right ) );
  // some consistency - reset up for dir too
  coll->action( "up" )->setShortcut( KShortcut( ALT + SHIFT + Key_Up ) );
  coll->action( "home" )->setShortcut( KShortcut( CTRL + ALT + Key_Home ) );

  coll->action("home")->plug(toolbar);
  coll->action("up")->plug(toolbar);
  coll->action("back")->plug(toolbar);
  coll->action("forward")->plug(toolbar);

  toolbar->insertButton("fileopen", 0, true , i18n( "Open selected" ));
  connect(toolbar, SIGNAL(clicked(int)), this, SLOT(clickedToolbar(int)));

  lo->addWidget(dir);
  lo->setStretchFactor(dir, 2);

  m_comboEncoding = new KComboBox( false, this, "comboEncoding" );
  QStringList availableEncodingNames(KGlobal::charsets()->availableEncodingNames());
  m_comboEncoding->setEditable( true );
  m_comboEncoding->insertStringList( availableEncodingNames );
  QToolTip::add(m_comboEncoding, i18n("Set encoding"));
  lo->addWidget(m_comboEncoding);

  connect( cmbPath, SIGNAL( urlActivated( const KURL&  )),this,  SLOT( cmbPathActivated( const KURL& ) ));
  connect( cmbPath, SIGNAL( returnPressed( const QString&  )), this,  SLOT( cmbPathReturnPressed( const QString& ) ));
  connect(dir, SIGNAL(urlEntered(const KURL&)), this, SLOT(dirUrlEntered(const KURL&)) );
}

KileFileSelect::~KileFileSelect()
{
  delete cmpl;
}

void KileFileSelect::readConfig()
{
	QString lastDir = KileConfig::lastDir();
	QFileInfo ldi(lastDir);
	if ( ! ldi.isReadable() ) dir->home();
	else setDir(KURL::fromPathOrURL(lastDir));
}

void KileFileSelect::writeConfig()
{
	KileConfig::setLastDir(dir->url().path());
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

void KileFileSelect::focusInEvent(QFocusEvent*)
{
   dir->setFocus();
}

void KileFileSelect::setDir( KURL u )
{
  dir->setURL(u, true);
}

void KileFileSelect::clickedToolbar(int i)
{
	if (i == 0)
	{
		QPtrListIterator<KFileItem> it(*dir->selectedItems());
		while (  it.current() != 0 )
		{
			emit(fileSelected(*it));
        	++it;
		}

		dir->view()->clearSelection();
	}
}

#include "kilefileselect.moc"
