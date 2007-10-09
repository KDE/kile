/***************************************************************************
                           userhelpdialog.cpp
----------------------------------------------------------------------------
    date                 : Jul 22 2005
    version              : 0.20
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "userhelpdialog.h"

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qinputdialog.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurl.h>
#include "kiledebug.h"


namespace KileDialog
{

//////////////////// UserHelpDialog ////////////////////

//BEGIN UserHelpDialog

UserHelpDialog::UserHelpDialog(QWidget *parent, const char *name)
	: KDialogBase( parent, name, true, i18n("Configure User Help"), Cancel | Ok, Ok, true )
{
	KILE_DEBUG() << "==UserHelpDialog::UserHelpDialog()===================" << endl;

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	// layout
	QVBoxLayout *vbox = new QVBoxLayout(page, 6,6 );
	QVGroupBox* group= new QVGroupBox(i18n("User Help"),page );

	QWidget *widget = new QWidget(group);
	QGridLayout *grid = new QGridLayout( widget, 5,3, 5,5, "" );
	grid->setRowStretch(1,1);
	grid->setColStretch(0,1);
	grid->setRowSpacing(2,12);
	grid->setColSpacing(1,20);

	// listbox
	QLabel *label1 = new QLabel(i18n("&Menu item:"),widget);
	grid->addWidget( label1,0,0 );
	m_menulistbox = new KListBox(widget);
	grid->addWidget( m_menulistbox, 1,0 );
	label1->setBuddy(m_menulistbox);

	// action widget
	QWidget *actionwidget = new QWidget(widget);
	QVBoxLayout *actions = new QVBoxLayout(actionwidget);

	m_add = new KPushButton(i18n("&Add..."),actionwidget);
	m_remove = new KPushButton(i18n("&Remove"),actionwidget);
	m_addsep = new KPushButton(i18n("&Separator"),actionwidget);
	m_up = new KPushButton(i18n("Move &Up"),actionwidget);
	m_down = new KPushButton(i18n("Move &Down"),actionwidget);

	int wmax = m_add->sizeHint().width();
	int w = m_remove->sizeHint().width();
	if ( w > wmax ) wmax = w;
	w = m_addsep->sizeHint().width();
	if ( w > wmax ) wmax = w;
	w = m_up->sizeHint().width();
	if ( w > wmax ) wmax = w;
	w = m_down->sizeHint().width();
	if ( w > wmax ) wmax = w;

	m_add->setFixedWidth(wmax);
	m_remove->setFixedWidth(wmax);
	m_addsep->setFixedWidth(wmax);
	m_up->setFixedWidth(wmax);
	m_down->setFixedWidth(wmax);

	actions->addStretch(1);
	actions->addWidget(m_add);
	actions->addWidget(m_remove);
	actions->addSpacing(20);
	actions->addWidget(m_addsep);
	actions->addSpacing(20);
	actions->addWidget(m_up);
	actions->addWidget(m_down);
	actions->addStretch(1);

	// inserta ction widget
	grid->addWidget( actionwidget,1,2, Qt::AlignTop );

	// file
	QLabel *label2 = new QLabel(i18n("File:"),widget);
	grid->addWidget( label2, 3,0 );
	m_fileedit = new KLineEdit("",widget);
	m_fileedit->setReadOnly(true);
	grid->addMultiCellWidget( m_fileedit, 4,4,0,2 );

	// fill vbox
	vbox->addWidget(group);

	connect( m_menulistbox, SIGNAL(highlighted(int)),this,SLOT(slotChange(int)));
	connect( m_add, SIGNAL(clicked()), SLOT(slotAdd()) );
	connect( m_remove, SIGNAL(clicked()), SLOT(slotRemove()) );
	connect( m_addsep, SIGNAL(clicked()), SLOT(slotAddSep()) );
	connect( m_up, SIGNAL(clicked()), SLOT(slotUp()) );
	connect( m_down, SIGNAL(clicked()), SLOT(slotDown()) );

	resize(400,sizeHint().height());
	updateButton();
}

void UserHelpDialog::setParameter(const QStringList &menuentries, const QStringList &helpfiles)
{
	for (uint i=0; i<menuentries.count(); ++i)
	{
		m_menulistbox->insertItem(menuentries[i]);

		if ( m_menulistbox->text(i) != "-" )
			m_filelist << helpfiles[i];
		else
			m_filelist << QString::null ;
	}
	updateButton();
}

void UserHelpDialog::getParameter(QStringList &userhelpmenulist, QStringList &userhelpfilelist)
{
	// clear result
	userhelpmenulist.clear();
	userhelpfilelist.clear();
	bool separator = false;

	// now get all entries
	for (uint i=0; i<m_menulistbox->count(); ++i)
	{
		if ( m_menulistbox->text(i) != "-" )
		{
			userhelpmenulist << m_menulistbox->text(i);
			userhelpfilelist << m_filelist[i];
			separator = false;
		}
		else if ( !separator )
		{
			userhelpmenulist << m_menulistbox->text(i);
			userhelpfilelist << QString::null;
			separator = true;
		}
	}
}

void UserHelpDialog::slotChange(int index)
{
	if ( index >= 0 )
	{
		m_fileedit->setText( m_filelist[index] );
	}
	else
	{
		m_fileedit->clear();
	}
	updateButton();
}

void UserHelpDialog::slotAdd()
{
	KileDialog::UserHelpAddDialog *dialog = new KileDialog::UserHelpAddDialog(m_menulistbox,this);
	if ( dialog->exec() )
	{
		// insert into listbox
		m_menulistbox->insertItem( dialog->getMenuitem() );
		m_menulistbox->setCurrentItem( m_menulistbox->count()-1 );

		// with corresponding filename
		QString helpfile = dialog->getHelpfile();
		m_filelist.append(helpfile);
		m_fileedit->setText(helpfile);

		updateButton();
	}
}

void UserHelpDialog::slotRemove()
{
	// get current index
	int index = m_menulistbox->currentItem();
	if ( index >= 0 )
	{
		// remove item
		m_menulistbox->removeItem(index);
		m_filelist.remove( m_filelist.at(index) );

		// select a new index: first we try to take the old index. When
		// this index is too big now, index is decremented.
		// If the list is empty now, index is set to -1.
		int entries = (int)m_menulistbox->count();
		if ( entries > 0  )
		{
			if ( index >= entries )
				index--;
			m_menulistbox->setSelected(index,true);
		}
		else
		{
			m_menulistbox->setCurrentItem(-1);
		}
	}

	updateButton();
}

void UserHelpDialog::slotAddSep()
{
	// get current index
	int index = m_menulistbox->currentItem();
	if ( index == -1 ) return;

	// insert separator
	m_menulistbox->insertItem("-",index);
	m_filelist.insert( m_filelist.at(index) ,QString::null );

	updateButton();
}

void UserHelpDialog::slotUp()
{
	// get current index
	int index = m_menulistbox->currentItem();
	if ( index <= 0 ) return;

	// insert current entry before current
	m_menulistbox->insertItem(m_menulistbox->currentText(),index-1);
	m_filelist.insert( m_filelist.at(index-1) , m_filelist[index] );

	// then remove the old entry
	m_menulistbox->removeItem(index+1);
	m_filelist.remove( m_filelist.at(index+1) );

	// select current entry
	m_menulistbox->setSelected(index-1,true);

	updateButton();
}

void UserHelpDialog::slotDown()
{
	int entries = (int)m_menulistbox->count();

	// get current index
	int index = m_menulistbox->currentItem();
	if ( index<0 || index==entries-1 ) return;

	// insert current entry after current
	if ( index < entries-2 )
	{
		m_menulistbox->insertItem(m_menulistbox->currentText(),index+2);    // index + 2
		m_filelist.insert( m_filelist.at(index+2) , m_filelist[index] );
	}
	else
	{
		m_menulistbox->insertItem(m_menulistbox->currentText());            // at the end
		m_filelist.append( m_filelist[index] );
	}

	// then remove the old entry
	m_menulistbox->removeItem(index);
	m_filelist.remove( m_filelist.at(index) );

	// select current entry
	m_menulistbox->setSelected(index+1,true);

	updateButton();
}

void UserHelpDialog::updateButton()
{
	// default states
	bool rem_state = false;
	bool sep_state = false;
	bool up_state = false;
	bool down_state = false;

	// change button states, if there are entries
	int index = m_menulistbox->currentItem();
	int entries = (int)m_menulistbox->count();
	if ( entries == 1 )
	{
		rem_state = true;
	}
	else if ( entries >= 2 )
	{
		rem_state = true;
		if ( index == 0 )
		{
			down_state = true;         // index = 0
		}
		else if ( index == entries-1 )
		{
			sep_state = true;          // index = entries-1
			up_state = true;
		}
		else
		{                             // 0 < index < entries-1
			sep_state = true;
			up_state = true;
			down_state = true;
		}
	}

	// don't allow two continuous spearators
	if ( m_menulistbox->currentText() == "-" )
		sep_state = false;

	// set button states
	m_remove->setEnabled(rem_state);
	m_addsep->setEnabled(sep_state);
	m_up->setEnabled(up_state);
	m_down->setEnabled(down_state);
}
//END UserHelpDialog

//////////////////// UserHelpAddDialog ////////////////////

//BEGIN UserHelpAddDialog

UserHelpAddDialog::UserHelpAddDialog(KListBox *menulistbox, QWidget *parent, const char *name)
	: KDialogBase( parent, name, true, i18n("Add User Helpfile"), Cancel | Ok, Ok, true ),
	  m_menulistbox(menulistbox)
{
	KILE_DEBUG() << "==UserHelpAddDialog::UserHelpAddDialog()===================" << endl;

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	// layout
	QVBoxLayout *vbox = new QVBoxLayout(page, 6,6 );
	QVGroupBox* group= new QVGroupBox(i18n("User Help"),page );

	QWidget *widget = new QWidget(group);
	QGridLayout *grid = new QGridLayout( widget, 2,6, 5,5, "" );
	grid->setColSpacing(2,8);
	grid->setColSpacing(4,8);

	// menu entry
	QLabel *label1 = new QLabel(i18n("&Menu entry:"),widget);
	grid->addWidget( label1,0,0 );
	m_leMenuEntry = new KLineEdit("",widget);
	grid->addWidget( m_leMenuEntry, 0,1 );
	label1->setBuddy(m_leMenuEntry);

	// help file
	QLabel *label2 = new QLabel(i18n("&Help file:"),widget);
	grid->addWidget( label2, 1,0 );
	m_leHelpFile = new KLineEdit("",widget);
	m_leHelpFile->setReadOnly(false);
	grid->addWidget( m_leHelpFile, 1,1 );
	m_pbChooseFile = new KPushButton("", widget, "filechooser_button" );
	m_pbChooseFile->setPixmap( SmallIcon("fileopen") );
	grid->addRowSpacing( 1, m_pbChooseFile->sizeHint().height()+5 );
	grid->addWidget(m_pbChooseFile,1,3);
	m_pbChooseHtml = new KPushButton("", widget, "htmlchooser_button" );
	m_pbChooseHtml->setPixmap( SmallIcon("viewhtml") );
	grid->addWidget(m_pbChooseHtml,1,5);
	grid->setColSpacing(3, m_pbChooseFile->sizeHint().width()+5 );
	grid->setColSpacing(5, m_pbChooseHtml->sizeHint().width()+5 );

	label2->setBuddy(m_pbChooseFile);

	// fill vbox
	vbox->addWidget(group);
	vbox->addStretch();

	QWhatsThis::add(m_leMenuEntry,i18n("The menu entry for this help file."));
	QWhatsThis::add(m_leHelpFile,i18n("The name of the local help file or a valid WEB url."));
	QWhatsThis::add(m_pbChooseFile,i18n("Start a file dialog to choose a local help file."));
	QWhatsThis::add(m_pbChooseHtml,i18n("Start the konqueror to choose a WEB url as help file. This url should be copied inzo the edit widget."));

	connect( m_pbChooseFile, SIGNAL( clicked() ), this, SLOT( slotChooseFile() ) );
	connect( m_pbChooseHtml, SIGNAL( clicked() ), this, SLOT( slotChooseHtml() ) );

	setFocusProxy( m_leMenuEntry );
	resize(500,sizeHint().height());
}

void UserHelpAddDialog::slotChooseFile()
{
	QString directory = QDir::currentDirPath();
	QString filter = "*.*|All Files\n*.dvi|DVI Files\n*.ps|PS Files\n*.pdf|PDF Files\n*.html *.htm|HTML Files";

	QString filename = KFileDialog::getOpenFileName( directory,filter,this,i18n("Select File") );
	if ( filename.isEmpty() )
		return;

	QFileInfo fi(filename);
	if ( ! fi.exists() )
	{
		KMessageBox::error(0,QString(i18n("File '%1' does not exist.")).arg(filename));
		return;
	}

	m_leHelpFile->setText( filename );
}

void UserHelpAddDialog::slotChooseHtml()
{
	KURL url;
	url.setPath("about:blank");
	KRun::runURL(url,"text/html");
}

void UserHelpAddDialog::slotOk()
{
	m_leMenuEntry->setText( m_leMenuEntry->text().stripWhiteSpace() );
	QString filename = m_leHelpFile->text().stripWhiteSpace();
	m_leHelpFile->setText( filename );

	if ( m_leMenuEntry->text().isEmpty() )
	{
		KMessageBox::error(this,i18n("No menuitem was given."));
		return;
	}

	if ( m_menulistbox->findItem(m_leMenuEntry->text(),Qt::ExactMatch) )
	{
		KMessageBox::error(this,i18n("This menuitem already exists."));
		return;
	}

	if ( filename.isEmpty() )
	{
		KMessageBox::error(this,i18n("No help file was chosen."));
		return;
	}

	QFileInfo fi(filename);
	if ( filename.find("http://",0)!=0 && !fi.exists() )
	{
		KMessageBox::error(this,QString(i18n("File '%1' doesn't exist.")).arg(filename));
		return;
	}

	accept();
}

//END UserHelpAddDialog

}

#include "userhelpdialog.moc"

