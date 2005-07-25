/***************************************************************************
                         texdocdialog.cpp
                         ----------------
    date                 : Jul 22 2005
    version              : 0.11
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
 
#include "texdocdialog.h"

#include <qlayout.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qgroupbox.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <kapplication.h>
#include <qdesktopwidget.h>

#include <kurl.h>
#include <krun.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <ktrader.h>
#include <kservice.h>


namespace KileDialog
{

//BEGIN TexDocDialog

TexDocDialog::TexDocDialog(QWidget *parent, const char *name) 
   : KDialogBase( parent,name, true, i18n("Documentation Browser"), Ok | Help, NoDefault, true ),
	m_tempfile(0), m_proc(0)
{
	QWidget *page = new QWidget( this );
	setMainWidget(page);
 
	QVBoxLayout *vbox = new QVBoxLayout(page,8,8);
	
	// listview 
	m_texdocs = new KListView(page);
	m_texdocs->setRootIsDecorated(true);
	m_texdocs->addColumn(i18n("Table of Contents"));

	// groupbox	
	QGroupBox *groupbox = new QGroupBox( i18n("Search"), page, "groupbox" );
	groupbox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, groupbox->sizePolicy().hasHeightForWidth() ) );
	groupbox->setColumnLayout(0, Qt::Vertical ); 
	groupbox->layout()->setSpacing( 6 ); 
	groupbox->layout()->setMargin( 11 );
	QGridLayout *groupboxLayout = new QGridLayout( groupbox->layout() );
	groupboxLayout->setAlignment( Qt::AlignTop );
   
	QLabel *label = new QLabel( i18n("&Keyword:"), groupbox, "label");
	m_leKeywords = new KLineEdit("",groupbox);
	m_pbSearch = new KPushButton(i18n("&Search"),groupbox);
	label->setBuddy(m_leKeywords);
	
	groupboxLayout->addWidget(label,0,0);
	groupboxLayout->addWidget(m_leKeywords,0,1);
	groupboxLayout->addWidget(m_pbSearch,1,0);

	vbox->addWidget(m_texdocs);
	vbox->addWidget(groupbox);
	
	QWhatsThis::add(m_texdocs,i18n("A list of avaiblable documents, which are listed in 'texdoctk.dat', coming with teTeX. A double click with the mouse or pressing the space key will open a viewer to show this file."));
	QWhatsThis::add(m_leKeywords,i18n("You can choose a keyword to show only document files, which are related to this keyword."));
	QWhatsThis::add(m_pbSearch,i18n("Start the search for the chosen keyword."));
	QWhatsThis::add(actionButton(Help),i18n("Reset TOC to show all available files."));
	
	setButtonText(Ok,i18n("&Done"));
	setButtonText(Help,i18n("Reset &TOC"));
	m_pbSearch->setEnabled(false);
	enableButton(Help,false);
	m_texdocs->installEventFilter(this);
	
	connect(m_texdocs, SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)), 
	        this, SLOT(slotListViewDoubleClicked(QListViewItem *,const QPoint &,int)));
	connect(m_pbSearch, SIGNAL(clicked()), this, SLOT(slotSearchClicked()));
	connect(m_leKeywords, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged(const QString &)));

	// kpsewhich --expand-path='$TEXMF'
	//m_texmfPath = "/usr/local/share/texmf:/usr/local/lib/texmf:/var/lib/texmf:/usr/share/texmf";
	// kpsewhich --expand-path='$TEXMF/doc'
	//m_texmfdocPath = "/usr/local/share/texmf/doc:/usr/local/lib/texmf/doc:/usr/share/texmf/doc";
	// kpsewhich --progname=texdoctk --format='other text files' texdoctk.dat
	//m_texdoctkPath = "/usr/share/texmf/texdoctk/texdoctk.dat";
	
	m_texmfPath = QString::null;
	m_texmfdocPath = QString::null;
	m_texdoctkPath = QString::null;
		
	QDesktopWidget *desktop = KApplication::desktop();
	int w = desktop->screenGeometry(0).width();
	if ( w >= 1024 )
		w = 550;
	else if ( w >= 800 )
		w = 500;
	else 
		w = 450;
	int h = desktop->screenGeometry(0).height() ;
	if ( h >= 768 )
		h = 550;
	else if ( h >= 600 )
		h = 500;
	else 
		h = 450;
	resize(w,h);
	
	connect(this, SIGNAL(processFinished()), this, SLOT(slotInitToc()));
	executeScript(
	   "kpsewhich --progname=texdoctk --format='other text files' texdoctk.dat && "
	   "kpsewhich --expand-path='$TEXMF/doc' && "
	   "kpsewhich --expand-path='$TEXMF'"
		);	
	
	//readToc();	
	//slotHelp();
}

TexDocDialog::~TexDocDialog() 
{
	if (m_proc )
		delete m_proc;
	if ( m_tempfile )
		delete m_tempfile;
}

////////////////////// TOC //////////////////////

void TexDocDialog::readToc()
{
	// open to read
	QFile fin( m_texdoctkPath );
	if ( !fin.exists() || !fin.open(IO_ReadOnly) ) 
	{
		KMessageBox::error(this,i18n("Could not read 'texdoctk.dat'."));
		return;
	}
	
	// use a textstream to read all data
	QString textline;
	QTextStream data(&fin);
	while ( ! data.eof() ) {
		textline = data.readLine();
		if ( ! (textline.isEmpty() || textline[0]=='#') ) 
		{
			// save the whole entry
			m_tocList.append( textline );
			
			// list entries 0,1,basename(2),3 are needed for keyword search
			// (key,title,filepath,keywords)
			QStringList list = QStringList::split(';',textline,true);
			
			// get basename of help file 
			QString basename;
			if ( list.count() > 2 ) 
			{
				QFileInfo fi(list[2]);
				basename = fi.baseName().lower();
			}  
			
			QString entry = list[0] + ";" + list[1];
			if ( ! basename.isEmpty() )  
				entry += ";" + basename;
			if ( list.count() > 3 )
				entry += ";" + list[3];
			m_tocSearchList.append(entry);
		}
	}
}

void TexDocDialog::showToc(const QString &caption,const QStringList &doclist, bool toc)
{
	QString section,textline;
	QStringList keylist;
	KListViewItem *itemsection = 0L;
	
	setUpdatesEnabled( false );
	m_texdocs->setColumnText(0,caption);	
	
	for (uint i=0; i<doclist.count(); i++ ) 
	{
		if ( doclist[i][0] == '@' ) 
		{
			section = doclist[i];
			itemsection = new KListViewItem(m_texdocs,section.remove(0,1));
		} 
		else 
		{
			keylist = QStringList::split(';',doclist[i],true);
			if ( itemsection ) 
			{
				KListViewItem *item = new KListViewItem(itemsection,keylist[1],keylist[0]);
				item->setPixmap(0, SmallIcon(getIconName(keylist[2])) );
				
				// save filename in dictionary
				m_dictDocuments[keylist[0]] = keylist[2];
				
				// search for special keywords
				QRegExp reg( "^\\s*(-\\d-)" );
				if ( keylist[3].find(reg,0) == 0 ) 
				{
					m_dictStyleCodes[keylist[0]] = reg.cap(1);
				}
			}
		}
	}
	setUpdatesEnabled( true );
	
	if ( toc )
		m_pbSearch->setEnabled(false);
	enableButton(Help,!toc);
	m_texdocs->setFocus();
}

bool TexDocDialog::eventFilter(QObject *o, QEvent *e)
{
	// enable start of viewer, when pressing the space key
	if ( o == m_texdocs && e->type()==QEvent::KeyPress ) 
	{
		QKeyEvent *kev = (QKeyEvent*) e;
		if ( kev->key() == Qt::Key_Space ) 
		{
			slotListViewDoubleClicked(m_texdocs->currentItem(), QPoint(0,0), 0) ;
			return true; 
		}
	}

	return false;
}

////////////////////// prepare document file //////////////////////

QString TexDocDialog::searchFile(const QString &docfilename,const QString &listofpathes, const QString &subdir)
{
	QStringList pathlist  = QStringList::split(':',listofpathes);
	QStringList extlist   = QStringList::split(',',",.gz,.bz2",true);
	
	QString filename;
	for ( QStringList::Iterator itp = pathlist.begin(); itp!=pathlist.end(); ++itp ) 
	{
		for ( QStringList::Iterator ite = extlist.begin(); ite!=extlist.end(); ++ite ) 
		{
			filename = ( subdir.isEmpty() ) ? (*itp) + "/" + docfilename + (*ite)
			                                : (*itp) + "/" + subdir + "/" + docfilename + (*ite);
		 	// kdDebug() << "search file: "  << filename << endl;
			if (  QFile::exists(filename) )
				return filename;
		}
	}
	
	return QString::null;
}

void TexDocDialog::decompressFile(const QString &docfile,const QString &command)
{
	QString ext = QFileInfo(docfile).extension(false).lower();
	if ( ! ( ext=="dvi" || ext=="pdf" || ext=="ps" || ext=="html") )
		ext = "txt";
		
	if ( m_tempfile )
		delete m_tempfile;
		
	m_tempfile = new KTempFile(QString::null,"."+ext);
	m_tempfile->setAutoDelete(true);
	m_filename = m_tempfile->name();
	
	kdDebug() << "\tdecompress file: "  << command + " > " + m_tempfile->name() << endl;
	connect(this, SIGNAL(processFinished()), this, SLOT(slotShowFile()));
	executeScript(command + " > " + m_tempfile->name());
}

void TexDocDialog::showStyleFile(const QString &filename,const QString &stylecode)
{
	kdDebug() << "\tshow style file: "<< filename << endl;
	if ( ! QFile::exists(filename) ) 
		return;
		
	// open to read
	QFile fin( filename );
	if ( !fin.exists() || !fin.open(IO_ReadOnly) ) 
	{
		KMessageBox::error(this,i18n("Could not read the style file."));
		return;
	}
	
	if ( m_tempfile )
		delete m_tempfile;
	m_tempfile = new KTempFile(QString::null,".txt");
	m_tempfile->setAutoDelete(true);
	
	// use a textstream to write to the temporary file
	QFile tmpfile(m_tempfile->name());
	if ( ! tmpfile.open( IO_WriteOnly ) ) 
	{
		KMessageBox::error(this,i18n("Could not create a temporary file."));
		return ;
	}
	QTextStream stream(&tmpfile);

	// use another textstream to read from the style file
	QTextStream sty( &fin );
	
	// there are four mode to read from the style file
	QString textline;
	if ( stylecode == "-3-" ) 
	{
		// mode 3: read everything up to the first empty line
		while ( ! sty.eof() ) {
			textline = sty.readLine().stripWhiteSpace();
			if ( textline.isEmpty() )
				break;
			stream << textline << "\n";
		}
	} 
	else if ( stylecode == "-2-" ) 
	{
		// mode 2: read everything up to a line starting with at least 4 '%' characters
		for ( int i=0; i<9; ++i )
			stream << sty.readLine() << "\n";
		while ( ! sty.eof() ) 
		{
			textline = sty.readLine();
			if ( textline.find("%%%%") == 0 )
				break;
			stream << textline << "\n";
		}
	} 
	else if ( stylecode == "-1-" ) 
	{
		// mode 1: read all lines at the end behind \endinput
		while ( ! sty.eof() ) 
		{
			textline = sty.readLine().stripWhiteSpace();
			if ( textline.find("\\endinput") == 0 )
				break;
		}
		while ( ! sty.eof() ) 
		{
			stream << sty.readLine() << "\n";
		}
	} 
	else 
	{
		// mode 0: read everything except empty lines and comments 
		while ( ! sty.eof() ) 
		{
			textline = sty.readLine();
			if ( !textline.isEmpty() && textline[0]!='%' )
				stream << textline << "\n";
		}
	}
	tmpfile.close();
	
	// start the viewer
	showFile(m_tempfile->name());
}

void TexDocDialog::showFile(const QString &filename)
{
	kdDebug() << "\tshow file: "<< filename << endl;
	if ( QFile::exists(filename) ) 
	{
		KURL url;
		url.setPath(filename);	
		/*
		KRun::runURL(url, getMimeType(filename) );
		*/
		
		KTrader::OfferList offers = KTrader::self()->query( getMimeType(filename),"Type == 'Application'");
		if ( offers.isEmpty() ) 
		{
			KMessageBox::error(this,i18n("No KDE service found for this file."));
			return;
		}
		KService::Ptr ptr = offers.first();
		KURL::List lst;
		lst.append(url);
		KRun::run(*ptr, lst, true);
		
// KRun::shellQuote(path);
	}
}


////////////////////// Slots //////////////////////

void TexDocDialog::slotListViewDoubleClicked(QListViewItem *item,const QPoint &,int)
{
	if ( ! item->parent() ) 
		return;
		
	QString package = item->text(1);
	kdDebug() << "\tselect child: "  << item->text(0) << endl 
	          << "\tis package: " << package << endl;
	if ( ! m_dictDocuments.contains( package ) ) 
		return;
		
	QString texdocfile = m_dictDocuments[package];
	kdDebug() << "\tis texdocfile: " << texdocfile << endl;
	
	// search for the file in the documentation directories
	QString filename = searchFile(texdocfile,m_texmfdocPath);
	if ( filename.isEmpty() ) 
	{
		// not found: search it elsewhere
		filename = searchFile(texdocfile,m_texmfPath,"tex");
		if ( filename.isEmpty() ) 
		{
			KMessageBox::error(this,i18n("Could not find '%1'").arg(filename));
			return;
		}
	}
	kdDebug() << "\tfound file: " << filename << endl;
	
	QString ext = QFileInfo(filename).extension(false).lower(); 
	m_filename = QString::null;
	if ( ext == "gz" ) 
		decompressFile(m_dictDocuments[package],"gzip -cd "+filename); 
	else if ( ext == "bz2" ) 
		decompressFile(m_dictDocuments[package],"bzip2 -cd "+filename); 
	else if ( ext=="sty" &&  m_dictStyleCodes.contains(package) )
		showStyleFile(filename,m_dictStyleCodes[package]);
	 else
		showFile(filename);
}

void TexDocDialog::slotTextChanged(const QString &text)
{
	m_pbSearch->setEnabled( ! text.stripWhiteSpace().isEmpty() );
}

void TexDocDialog::slotSearchClicked()
{
	QString keyword = m_leKeywords->text().stripWhiteSpace();
	if ( keyword.isEmpty() ) 
	{ 
		KMessageBox::error(this,i18n("No keyword given."));
		return;
	}
	
	QString section;
	bool writesection = true;
	QStringList searchlist;

	for (uint i=0; i<m_tocList.count(); i++ ) 
	{
		if ( m_tocList[i][0] == '@' )
		{
			section = m_tocList[i];
			writesection = true;
		} 
		else if ( m_tocSearchList[i].find(keyword,0,false) > -1 ) 
		{
				if ( writesection ) 
					searchlist.append(section);
				searchlist.append(m_tocList[i]);
				writesection = false;
		}
	}
	
	if ( searchlist.count() > 0 ) 
	{
		m_texdocs->clear();
		showToc(i18n("Search results for keyword '")+keyword+"'",searchlist,false);
	} 
	else
		KMessageBox::error(this,i18n("No documents found for keyword ''.").arg(keyword));
}

void TexDocDialog::slotHelp()
{
	m_leKeywords->setText(QString::null);
	m_texdocs->clear();
	showToc(i18n("Table of Contents"),m_tocList,true);
}

////////////////////// execute shell script //////////////////////

void TexDocDialog::executeScript(const QString &command)
{
	if ( m_proc )
		delete m_proc;

	m_proc = new KShellProcess("/bin/sh");
	m_proc->clearArguments();
	(*m_proc) << QStringList::split(' ',command);
	m_output = QString::null;
	
	connect(m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
	        this,   SLOT(slotProcessOutput(KProcess*,char*,int)) );
	connect(m_proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
	        this,   SLOT(slotProcessOutput(KProcess*,char*,int)) );
	connect(m_proc, SIGNAL(processExited(KProcess*)),
	        this,   SLOT(slotProcessExited(KProcess*)) );
	  
	kdDebug() << "=== TexDocDialog::runShellSkript() ====================" << endl;
	kdDebug() << "   execute: " << command << endl;
	if ( ! m_proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) 
		kdDebug() << "\tstart of shell process failed" << endl;
}

void TexDocDialog::slotProcessOutput(KProcess*,char* buf,int len)
{
   m_output += QString::fromLocal8Bit(buf,len);
}


void TexDocDialog::slotProcessExited(KProcess *proc)
{
	if ( proc->normalExit() &&  !proc->exitStatus() ) 
	{
		//showFile(m_filename);
		emit( processFinished() );
	} 
	else 
	{
		KMessageBox::error( this,i18n("<center>") + i18n("Could not determine the search paths of teTeX or file 'texdoctk.dat'.<br> So this dialog is useless.") + i18n("</center>"),i18n("TexDoc Dialog") );
	}
}

////////////////////// process slots, when finished //////////////////////

void TexDocDialog::slotInitToc()
{
	disconnect(this, SIGNAL(processFinished()), this, SLOT(slotInitToc()));

	QStringList results = QStringList::split('\n',m_output,true);
	if ( results.count() < 3 ) 
	{
		KMessageBox::error(this,i18n("Could not determine the search paths of teTeX or file 'texdoctk.dat'.<br> So this dialog is useless."));
		return;
	}
	
	m_texdoctkPath = results[0];
	m_texmfdocPath = results[1];
	m_texmfPath = results[2];
	
	kdDebug() << "\t--->: " << m_texdoctkPath << endl;
	kdDebug() << "\t--->: " << m_texmfdocPath << endl;
	kdDebug() << "\t--->: " << m_texmfPath << endl;
	
	if ( m_texdoctkPath.find('\n',-1) > -1 ) 
	{
		m_texdoctkPath.truncate(m_texdoctkPath.length()-1);
	} 
	
	// read data and initialize listview
	readToc();	
	slotHelp();
}

void TexDocDialog::slotShowFile()
{
	disconnect(this, SIGNAL(processFinished()), this, SLOT(slotShowFile()));
	showFile(m_filename);	
}

////////////////////// Icon/Mime //////////////////////

QString TexDocDialog::getMimeType(const QString &filename)
{
	QFileInfo fi(filename);
	QString basename = fi.baseName().lower();  
	QString ext = fi.extension(false).lower(); 
			
	QString mimetype;
	if ( ext=="txt" || ext=="faq" || ext=="sty" || basename=="readme" || basename=="00readme"  ) 
	{
		mimetype = "text/plain";
	} 
	else 
	{
		KURL mimeurl;
		mimeurl.setPath(filename);
		KMimeType::Ptr pMime = KMimeType::findByURL(mimeurl);
		mimetype = pMime->name();
	}
	
	kdDebug() << "\tmime = "  << mimetype << " " << endl;
	return mimetype;
}

QString TexDocDialog::getIconName(const QString &filename)
{
	QFileInfo fi( filename );
	QString basename = fi.baseName().lower();  
	QString ext = fi.extension(false).lower();
	
	QString icon;
	if ( ext=="dvi" || ext=="pdf" || ext=="html" || ext == "htm"  || ext == "txt")
		icon = ext;
	else if ( ext == "ps" )
		icon = "postscript";
	else if ( ext == "sty" )
		icon = "tex";
	else if ( ext == "faq" || basename=="readme" || basename=="00readme" )
		icon = "readme";
	else
		icon = "ascii";
		
	return icon;
}


//END TexDocDialog


}
#include "texdocdialog.moc"
