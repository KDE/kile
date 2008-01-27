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

#include "widgets/abbreviationview.h"

#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "kiledebug.h"

#include <QTextStream>

#include <q3header.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qfile.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>

#include "dialogs/abbreviationinputdialog.h"

namespace KileWidget {

AbbreviationView::AbbreviationView(QWidget *parent, const char *name)
	: K3ListView(parent), m_changes(false)
{
	setObjectName(name);
	addColumn(i18n("Short"));
	addColumn(QString());
	addColumn(i18n("Expanded Text"));
	setAllColumnsShowFocus(true);
	setFullWidth(true);

	setItemsMovable(false);                 // default: true
	//setAcceptDrops(false);                // default: false
	//setDragEnabled(false);                // default: false
	//setShadeSortColumn(true);             // default: true
	header()->setMovingEnabled(false);      // default: true

	m_popup = new Q3PopupMenu( this );

	connect(this, SIGNAL(mouseButtonClicked(int,Q3ListViewItem *,const QPoint &,int)),
	        this, SLOT(slotMouseButtonClicked(int,Q3ListViewItem *,const QPoint &,int)));

	connect(this, SIGNAL(contextMenu(K3ListView *,Q3ListViewItem *,const QPoint &)),
	        this, SLOT(slotContextMenu(K3ListView *,Q3ListViewItem *,const QPoint &)));
}

AbbreviationView::~AbbreviationView()
{
}
//////////////////// init abbreviation view with wordlists ////////////////////


void AbbreviationView::init(const QStringList *globallist, const QStringList *locallist)
{
	setUpdatesEnabled(false);
	clear();
	addWordlist(globallist,true);
	addWordlist(locallist,false);
	setUpdatesEnabled(true);

	m_changes = false;
}

void AbbreviationView::addWordlist(const QStringList *wordlist, bool global)
{
	QString type = ( global ) ? QString() : "*";

	QStringList::ConstIterator it;
	for ( it=wordlist->begin(); it!=wordlist->end(); ++it ) 
	{
		int index = (*it).indexOf( '=' );
		if ( index >= 0 )
		{
			insertItem( new K3ListViewItem(this,(*it).left(index),type,(*it).right( (*it).length()-index-1 )) ); 
		}
	}
}

//////////////////// save local abbreviation list ////////////////////

void AbbreviationView::saveLocalAbbreviation(const QString &filename)
{
	if ( ! m_changes )
		return;

	KILE_DEBUG() << "=== AbbreviationView::saveLocalAbbreviation ===================" << endl;
	// create the file 
	QFile abbrevfile(filename);
	if ( ! abbrevfile.open( QIODevice::WriteOnly ) ) 
		return;

	QTextStream stream(&abbrevfile);
	stream << "# abbreviation mode: editable abbreviations\n";
	stream << "# dani/2007\n";

	//QTextCodec *codec = QTextCodec::codecForName(m_ki->activeTextDocument()->encoding().ascii());
	// stream.setCodec(codec); 

	Q3ListViewItemIterator it(this);
	while(it.current()) {
		if ( it.current()->text(AbbreviationView::ALVlocal) == "*" )
		{
			stream << it.current()->text(AbbreviationView::ALVabbrev) 
			       << "=" 
			       << it.current()->text(AbbreviationView::ALVexpansion)
			       << "\n";
		}
		++it;
	}
	abbrevfile.close();

	m_changes = false;
}

//////////////////// find abbreviation ////////////////////

bool AbbreviationView::findAbbreviation(const QString &abbrev)
{
	Q3ListViewItemIterator it( this );
	while ( it.current() ) 
	{
		if ( it.current()->text(AbbreviationView::ALVabbrev) == abbrev )
			return true;
 
		++it;
	}
	return false;
}

//////////////////// item clicked ////////////////////

void AbbreviationView::slotMouseButtonClicked(int button, Q3ListViewItem *item, const QPoint &, int)
{
	if ( button==1 && item )
	{
		emit( sendText( item->text(AbbreviationView::ALVexpansion) ) );
	}
}

//////////////////// context menu ////////////////////

void AbbreviationView::slotContextMenu(K3ListView *, Q3ListViewItem *item, const QPoint &pos)
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

void AbbreviationView::addAbbreviation(const QString &abbrev, const QString &expansion)
{
	insertItem( new K3ListViewItem(this,abbrev,"*",expansion) ); 
	QString newAbbrev = abbrev + '=' + expansion;

	emit(updateAbbrevList(QString(), newAbbrev));
	m_changes = true;
}

void AbbreviationView::changeAbbreviation(K3ListViewItem *item, const QString &abbrev, const QString &expansion)
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

void AbbreviationView::deleteAbbreviation(K3ListViewItem *item)
{
	QString abbrev = item->text(ALVabbrev);
	QString message = i18n("Delete the abbreviation '%1'?", abbrev);
	if(KMessageBox::questionYesNo( this,
		       "<center>" + message + "</center>",
		       i18n("Delete Abbreviation") ) == KMessageBox::Yes)  {
		QString s = item->text(ALVabbrev) + '=' + item->text(ALVexpansion);
		delete item;

		emit(updateAbbrevList(s, QString()));
		m_changes = true;
	}
}

void AbbreviationView::slotPopupAbbreviation(int id)
{
	K3ListViewItem *item = (K3ListViewItem *)selectedItem();

	int mode = ALVnone;
	if(id == ALVadd) {
		mode = ALVadd;
	}
	else if(id==ALVedit && item) {
		mode = ALVedit;
	}
	else if(id==ALVdelete && item) {
		deleteAbbreviation(item);
	}
	
	if(mode != ALVnone) {
		KileDialog::AbbreviationInputDialog *dialog = new KileDialog::AbbreviationInputDialog(this, item, mode);
		if(dialog->exec() == QDialog::Accepted) {
			QString abbrev,expansion;
			dialog->abbreviation(abbrev,expansion);
			if(mode == ALVadd) {
				addAbbreviation(abbrev,expansion);
			}
			else {
				changeAbbreviation(item,abbrev,expansion);
			}
		}
		delete dialog;
	}
}

}

#include "abbreviationview.moc"
