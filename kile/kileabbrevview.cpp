/***************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kileabbrevview.h"

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "kiledebug.h"

#include <qheader.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qfile.h>

KileAbbrevView::KileAbbrevView(QWidget *parent, const char *name) 
	: KListView(parent, name), m_changes(false)
{

	addColumn(i18n("Short"));
	addColumn(QString::null);
	addColumn(i18n("Expanded Text"));
	setAllColumnsShowFocus(true);
	setFullWidth(true);

	setItemsMovable(false);                 // default: true
	//setAcceptDrops(false);                // default: false
	//setDragEnabled(false);                // default: false
	//setShadeSortColumn(true);             // default: true
	header()->setMovingEnabled(false);      // default: true

	m_popup = new QPopupMenu( this );

	connect(this, SIGNAL(mouseButtonClicked(int,QListViewItem *,const QPoint &,int)),
	        this, SLOT(slotMouseButtonClicked(int,QListViewItem *,const QPoint &,int)));

	connect(this, SIGNAL(contextMenu(KListView *,QListViewItem *,const QPoint &)),
	        this, SLOT(slotContextMenu(KListView *,QListViewItem *,const QPoint &)));
}

KileAbbrevView::~KileAbbrevView()
{
}
//////////////////// init abbreviation view with wordlists ////////////////////


void KileAbbrevView::init(const QStringList *globallist, const QStringList *locallist)
{
	setUpdatesEnabled(false);
	clear();
	addWordlist(globallist,true);
	addWordlist(locallist,false);
	setUpdatesEnabled(true);

	m_changes = false;
}

void KileAbbrevView::addWordlist(const QStringList *wordlist, bool global)
{
	QString type = ( global ) ? QString::null : "*";

	QStringList::ConstIterator it;
	for ( it=wordlist->begin(); it!=wordlist->end(); ++it ) 
	{
		int index = (*it).find( '=' );
		if ( index >= 0 )
		{
			insertItem( new KListViewItem(this,(*it).left(index),type,(*it).right( (*it).length()-index-1 )) ); 
		}
	}
}

//////////////////// save local abbreviation list ////////////////////

void KileAbbrevView::saveLocalAbbreviation(const QString &filename)
{
	if ( ! m_changes )
		return;

	KILE_DEBUG() << "=== KileAbbrevView::saveLocalAbbreviation ===================" << endl;
	// create the file 
	QFile abbrevfile(filename);
	if ( ! abbrevfile.open( IO_WriteOnly ) ) 
		return;

	QTextStream stream( &abbrevfile );
	stream << "# abbreviation mode: editable abbreviations\n";
	stream << "# dani/2007\n";

	//QTextCodec *codec = QTextCodec::codecForName(m_ki->activeTextDocument()->encoding().ascii());
	// stream.setCodec(codec); 

	QListViewItemIterator it( this );
	while ( it.current() ) 
	{
		if ( it.current()->text(KileAbbrevView::ALVlocal) == "*" )
		{
			stream << it.current()->text(KileAbbrevView::ALVabbrev) 
			       << "=" 
			       << it.current()->text(KileAbbrevView::ALVexpansion)
			       << "\n";
		}
		++it;
	}
	abbrevfile.close();

	m_changes = false;
}

//////////////////// find abbreviation ////////////////////

bool KileAbbrevView::findAbbreviation(const QString &abbrev)
{
	QListViewItemIterator it( this );
	while ( it.current() ) 
	{
		if ( it.current()->text(KileAbbrevView::ALVabbrev) == abbrev )
			return true;
 
		++it;
	}
	return false;
}

//////////////////// item clicked ////////////////////

void KileAbbrevView::slotMouseButtonClicked(int button, QListViewItem *item, const QPoint &, int)
{
	if ( button==1 && item )
	{
		emit( sendText( item->text(KileAbbrevView::ALVexpansion) ) );
	}
}

//////////////////// context menu ////////////////////

void KileAbbrevView::slotContextMenu(KListView *, QListViewItem *item, const QPoint &pos)
{
	m_popup->clear();
	m_popup->disconnect();

	m_popup->insertItem(i18n("&Add"),ALVadd);
	if ( item && item->text(ALVlocal)== "*" )
	{
		m_popup->insertSeparator();
		m_popup->insertItem(i18n("&Edit"),ALVedit);
		m_popup->insertSeparator();
		m_popup->insertItem(i18n("&Delete"),ALVdelete);
	}
	
	connect(m_popup,  SIGNAL(activated(int)), this, SLOT(slotPopupAbbreviation(int)));

	// show context menu
	m_popup->exec(pos);
}

void KileAbbrevView::addAbbreviation(const QString &abbrev, const QString &expansion)
{
	insertItem( new KListViewItem(this,abbrev,"*",expansion) ); 
	QString newAbbrev = abbrev + '=' + expansion;

	emit( updateAbbrevList(QString::null,newAbbrev) );
	m_changes = true;
}

void KileAbbrevView::changeAbbreviation(KListViewItem *item, const QString &abbrev, const QString &expansion)
{
	if ( item )
	{
		QString oldAbbrev = item->text(ALVabbrev) + '=' + item->text(ALVexpansion);
		QString newAbbrev = abbrev + '=' + expansion;
		item->setText(ALVabbrev,abbrev);
		item->setText(ALVexpansion,expansion);

		emit( updateAbbrevList(oldAbbrev,newAbbrev) );
		m_changes = true;
	}
}

void KileAbbrevView::deleteAbbreviation(KListViewItem *item)
{
	QString abbrev = item->text(ALVabbrev);
	QString message = i18n("Delete the abbreviation '%1'?").arg(abbrev);
	if ( KMessageBox::questionYesNo( this,
		       "<center>" + message + "</center>",
		       i18n("Delete Abbreviation") ) == KMessageBox::Yes ) 
	{
		QString s = item->text(ALVabbrev) + '=' + item->text(ALVexpansion);
		delete item;

		emit( updateAbbrevList(s,QString::null) );
		m_changes = true;
	}
}

void KileAbbrevView::slotPopupAbbreviation(int id)
{
	KListViewItem *item = (KListViewItem *)selectedItem();

	int mode = ALVnone;
	if ( id == ALVadd )
		mode = ALVadd;
	else if ( id==ALVedit && item )
		mode = ALVedit;
	else if ( id==ALVdelete && item ) 
		deleteAbbreviation(item);
	
	if ( mode != ALVnone )
	{
		KileAbbrevInputDialog *dialog = new KileAbbrevInputDialog(this,item,mode);
		if ( dialog->exec() == QDialog::Accepted )
		{
			QString abbrev,expansion;
			dialog->abbreviation(abbrev,expansion);
			if ( mode == ALVadd )
				addAbbreviation(abbrev,expansion);
			else
				changeAbbreviation(item,abbrev,expansion);
		}
		delete dialog;
	}
}

//////////////////// add/edit abbreviation ////////////////////

KileAbbrevInputDialog::KileAbbrevInputDialog(KileAbbrevView *listview, KListViewItem *item, int mode, const char *name )
	: KDialogBase(listview,name, true, i18n("Add Abbreviation"), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true),
	  m_listview(listview), m_abbrevItem(item), m_mode(mode)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	QVBoxLayout *vl = new QVBoxLayout(page, 0, spacingHint());

	if ( m_mode == KileAbbrevView::ALVedit )
	{
		setCaption( i18n("Edit Abbreviation") );
		m_abbrev = m_abbrevItem->text(KileAbbrevView::ALVabbrev);
		m_expansion = m_abbrevItem->text(KileAbbrevView::ALVexpansion);
	}
	
	QLabel *abbrev = new QLabel(i18n("&Abbreviation:"),page);
	QLabel *expansion = new QLabel(i18n("&Expanded Text:"),page);
	m_leAbbrev = new KLineEdit(m_abbrev,page);
	m_leExpansion = new KLineEdit(m_expansion,page);

	vl->addWidget(abbrev);
	vl->addWidget(m_leAbbrev);
	vl->addWidget(expansion);
	vl->addWidget(m_leExpansion);
	vl->addSpacing(8);

	abbrev->setBuddy(m_leAbbrev);
	expansion->setBuddy(m_leExpansion);

	QRegExp reg("[a-zA-Z0-9]+");
	QRegExpValidator *abbrevValidator = new QRegExpValidator(reg,this);
	m_leAbbrev->setValidator(abbrevValidator);

	connect(m_leAbbrev,SIGNAL(textChanged(const QString &)),
	        this,SLOT(slotTextChanged(const QString &)));
	connect(m_leExpansion,SIGNAL(textChanged(const QString &)),
	        this,SLOT(slotTextChanged(const QString &)));

	slotTextChanged(QString::null);
	m_leAbbrev->setFocus();
	page->setMinimumWidth(350);
}

KileAbbrevInputDialog::~KileAbbrevInputDialog()
{
}

void KileAbbrevInputDialog::abbreviation(QString &abbrev, QString &expansion)
{
	abbrev = m_leAbbrev->text(); 
	expansion = m_leExpansion->text().stripWhiteSpace();
}

void KileAbbrevInputDialog::slotTextChanged(const QString &)
{
	bool state = ( m_mode == KileAbbrevView::ALVadd )
	           ? ! m_listview->findAbbreviation( m_leAbbrev->text() ) : true;
 	state = state && !m_leAbbrev->text().isEmpty() && !m_leExpansion->text().isEmpty();

	enableButton(KDialogBase::Ok,state);
}

void KileAbbrevInputDialog::slotOk()
{
	QString abbrev = m_leAbbrev->text();
	QString expansion = m_leExpansion->text().stripWhiteSpace();

	if ( abbrev.isEmpty() || expansion.isEmpty() )
	{
		KMessageBox::error( this, i18n("Empty strings are not allowed.") );
		return;
	}

	if ( abbrev!=m_abbrev || expansion!=m_expansion )
		accept();
	else
		reject();
}


#include "kileabbrevview.moc"
