/***************************************************************************
                           userhelp.cpp
----------------------------------------------------------------------------
    date                 : Aug 17 2006
    version              : 0.25
    copyright            : (C) 2005-2006 by Holger Danielsson
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

#include "userhelp.h"

#include <qfileinfo.h>    
//Added by qt3to4:
#include <Q3PopupMenu>

#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <krun.h>
#include <kmimetype.h>
#include "kiledebug.h"

#include "userhelpdialog.h"
#include "kileconfig.h"

namespace KileHelp
{
   
UserHelp::UserHelp(KileTool::Manager *manager, KMenuBar *menubar, QWidget* mainWindow) 
	: m_manager(manager), m_menubar(menubar), m_helpid(0), m_sepid(0), m_mainWindow(mainWindow)
{
	expandHelpMenu();

	if ( m_helpmenu ) 
		readConfig();
}

UserHelp::~UserHelp() 
{
	delete m_helppopup;
} 

void UserHelp::readConfig()
{
	//KILE_DEBUG() << "\tuserhelp: read config" << endl;
	QStringList menu,files;
	
	// first read all entries
	KConfig *config = m_manager->config();
	config->setGroup("UserHelp");
	int entries = config->readNumEntry("entries");
	for ( int i=0; i<entries; ++i ) 
	{
		menu << config->readEntry(QString("menu%1").arg(i));
		if ( !menu[i].isEmpty() || menu[i]=="-" )
			files <<  config->readEntry(QString("file%1").arg(i));	
		else
			files << QString::null;
	}
	
	// then update menu
	updateEntries(menu,files,false);
}

void UserHelp::writeConfig()
{
	//KILE_DEBUG() << "\tuserhelp: write config" << endl;
	int entries = m_menuentries.count();
	
	// first delete old entries
	KConfig *config = m_manager->config();
	config->deleteGroup("UserHelp");
	
	// then write new entries
	config->setGroup("UserHelp");
	config->writeEntry("entries",entries);
	for ( int i=0; i<entries; ++i ) 
	{
		config->writeEntry(QString("menu%1").arg(i), m_menuentries[i]);
		if ( m_menuentries[i] != "-" )
			config->writeEntry(QString("file%1").arg(i), m_helpfiles[i]);	
	}
}

void UserHelp::expandHelpMenu()
{ 
	m_helppopup = 0L;
	m_helpid = 0;
	m_sepid = 0;

	m_helpmenu = getHelpPopup();
	if (  m_helpmenu ) 
	{
		int helpindex = getHelpIndex(m_helpmenu);
	
		m_helppopup = new Q3PopupMenu();
		if ( m_helppopup )  
		{
			m_sepid = m_helpmenu->insertSeparator(helpindex); 
			m_helpid = m_helpmenu->insertItem(i18n("User Help"),m_helppopup,-1,helpindex); 
			m_helpmenu->setItemVisible(m_helpid,false);
			m_helpmenu->setItemVisible(m_sepid,false);
		}
	}
}

// update stringlists and userhelp menu

void UserHelp::updateEntries(const QStringList &entries, const QStringList &files, bool save)
{
	if ( m_menuentries==entries && m_helpfiles==files)
		return;

	// save new entries	
	if ( m_helppopup )
		m_helppopup->clear();
	m_menuentries = entries;
	m_helpfiles = files;
		
	// set userhelp menu
	if ( m_menuentries.count() > 0 ) 
	{
		setupUserHelpMenu();
		m_helpmenu->setItemVisible(m_helpid,true);
		m_helpmenu->setItemVisible(m_sepid,true);
	} 
	else 
	{
		m_helpmenu->setItemVisible(m_helpid,false);
		m_helpmenu->setItemVisible(m_sepid,false);
	}	
	
	if ( save )
		writeConfig();
}

void UserHelp::setupUserHelpMenu()
{
	if ( ! m_helppopup ) return;
	
	int helpid;
	for ( uint i=0; i<m_menuentries.count(); ++i ) 
	{
		// first look, if this entry is a separator
		if ( m_menuentries[i] == "-" )  
		{
			helpid = m_helppopup->insertSeparator(-1);
		} 
		else 
		{
			// check for a http file
			bool http = ( m_helpfiles[i].find("http://",0) == 0 );
			
			// some file types have an icon
			QFileInfo fi(m_helpfiles[i]);
			QString ext = fi.extension(false);
			if ( ext == "htm" )
				ext = "html";
			if ( http || ext=="html" || ext=="dvi" || ext=="ps" || ext=="pdf" ) 
			{
				QString icon = ( http ) ? "viewhtml" : ext;
				helpid = m_helppopup->insertItem( SmallIcon(icon),m_menuentries[i],
				                                  this,SLOT(slotUserHelpActivated(int)) );
			} 
			else 
			{     
				helpid = m_helppopup->insertItem( m_menuentries[i],
				                                  this,SLOT(slotUserHelpActivated(int)) );
			}

		// send index of QStringList as parameter, when the slot is activated 
		m_helppopup->setItemParameter(helpid,i);  
		}
	}
}

void UserHelp::enableUserHelpEntries(bool state)
{
	if ( m_helppopup )
		delete m_helppopup;

	expandHelpMenu();
	if ( m_helpmenu && m_helppopup && m_menuentries.count()>0 ) 
	{
		setupUserHelpMenu();
		m_helpmenu->setItemVisible(m_helpid,state);
		m_helpmenu->setItemVisible(m_sepid,state);
	} 
}

Q3PopupMenu *UserHelp::getHelpPopup()
{ 
	int helpid = 0;
	
	for (uint i=0; i<m_menubar->count(); ++i) 
	{
		int id = m_menubar->idAt(i);
		QString text = m_menubar->text(id);
		
		if ( text == i18n("&Help") ) 
		{
			helpid = id;
			break;
		}
	}	
	return ( helpid == 0 ) ? 0 : m_menubar->findItem(helpid)->popup();
}

int UserHelp::getHelpIndex(Q3PopupMenu *popup)
{ 
	if ( popup ) 
	{
		int count = 0;
		for (uint i=0; i<popup->count(); ++i) 
		{
			int entryid = popup->idAt(i);
			QString entry = popup->text(entryid);
	
			if ( entry.isEmpty() ) 
			{
				if ( ++count == 2 ) 
					return (i+1);
			}
		}
	}
	
	return (0);
}

void UserHelp::slotUserHelpActivated(int index)
{ 
	KILE_DEBUG() << "==slotUserHelpActivated(" << index << ")============" << endl;
	if ( ! (index>=0 && index<(int)m_helpfiles.count()) ) 
		return;
		
	// get filename of this user help entry
	QString filename = m_helpfiles[index];

	// does the files exist?
	QFileInfo fi(filename);
	bool http = ( filename.find("http://",0) == 0 );
	if ( !http && !fi.exists() ) 
	{
		KMessageBox::error(0,QString(i18n("File '%1' doesn't exist.")).arg(filename));
		return;
	}
		
	// show help file
	KILE_DEBUG() << "\tshow userhelpfile (" << filename << ")" << endl;
		
	// determine, how to show the file
	QString type;
	QString cfg = "Embedded Viewer";
	if ( !http && KileConfig::embedded()==0 ) 
	{
		QString ext = fi.suffix();
		if ( ext == "dvi" ) 
			type = "ViewDVI";
		else if ( ext == "ps" )
			type = "ViewPS";
		else if ( ext == "pdf" )
			type = "ViewPDF";
		else if ( ext=="html" || ext=="htm"  ) 
			type = "ViewHTML";
	}
		
	KConfig *config = m_manager->config();
	if ( type!=QString::null && config->hasGroup("Tool/" + type + '/' + cfg) )
	{
		KileTool::View *tool = new KileTool::View(type, m_manager, false);
		tool->setFlags(0);
		tool->setSource(filename);
		tool->setTarget(fi.fileName());
		tool->prepareToRun();
		m_manager->run(tool,cfg);
	} 
	else 
	{
		KUrl url(filename);
		KMimeType::Ptr pMime = KMimeType::findByUrl(url);
		KRun::runUrl(url, pMime->name(), m_mainWindow);
	}
}

void UserHelp::userHelpDialog()
{
	QStringList userhelpmenulist, userhelpfilelist;
	
	KileDialog::UserHelpDialog *dialog = new KileDialog::UserHelpDialog();
	dialog->setParameter(m_menuentries,m_helpfiles);       
	if ( dialog->exec() ) 
	{
		//KILE_DEBUG() << "\t new userhelp entries accepted" << endl;
		dialog->getParameter(userhelpmenulist,userhelpfilelist);
		updateEntries(userhelpmenulist,userhelpfilelist);
	}
	
   delete dialog;	  
}  

}

#include "userhelp.moc"

