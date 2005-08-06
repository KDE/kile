/***************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                               2005 by Holger Danielsson
    email                : wijnhout@science.uva.nl
                           holger.danielsson@t-online.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmap.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include "newfilewizard.h"

// 2005-08-04: dani
//  - added script support to search existing class files 
//    (classes: Koma, Beamer, Prosper, HA-prosper)
//  - sort items ('Empty Document' will always be the first entry)

////////////////////// TemplateItem //////////////////////

// new compare function to set "Empty Document" as first item

TemplateItem::TemplateItem(QIconView * parent, const TemplateInfo & info) : QIconViewItem(parent,info.name, QPixmap(info.icon))
{
	m_info = info;
}

int TemplateItem::compare( QIconViewItem *i ) const
{
	if ( key() == DEFAULT_EMPTY_CAPTION ) 
		return -1;
	else if ( i->key() == DEFAULT_EMPTY_CAPTION )
		return 1;
	else
		return key().compare( i->key() );
}
    
////////////////////// NewFileWidget //////////////////////

NewFileWidget::NewFileWidget(QWidget *parent, const QString &selicon, char *name) : KIconView(parent,name), m_proc(0)
{
	m_selicon = ( selicon != QString::null ) ? selicon : DEFAULT_EMPTY_CAPTION;
	
   setItemsMovable(false);
   setResizeMode(QIconView::Adjust);
   setSelectionMode(QIconView::Single);
   setResizePolicy(QScrollView::Default);
   setArrangement(QIconView::TopToBottom);

   TemplateInfo info;
   info.name =DEFAULT_EMPTY_CAPTION;
   info.icon = KGlobal::dirs()->findResource("appdata", "pics/"+ QString(DEFAULT_EMPTY_ICON) + ".png" );
   info.path="";
   TemplateItem * emp = new TemplateItem( this, info);

	// execute script to find non standard class files
	searchClassFiles();
	
   setSelected(emp, true);
   setMinimumHeight(120);
}

NewFileWidget::~NewFileWidget() 
{
   delete m_proc;
}

////////////////////// add standard or found templates //////////////////////

void NewFileWidget::addTemplates()
{
	// disable non standard templates
	QMap<QString,bool> map;
	map["Scrartcl"] = false;
	map["Scrbook"]  = false;
	map["Scrreprt"] = false;
	map["Scrlttr2"] = false;
	map["Beamer"]   = false;
	map["Prosper"]  = false;
	map["HA-prosper"] = false;
	
	// split search results and look, which class files are present
	QStringList list = QStringList::split("\n",m_output);
	for ( QStringList::Iterator it=list.begin(); it!=list.end(); ++it ) 
	{
		QString filename = QFileInfo(*it).fileName();
		if ( filename=="scrartcl.cls" )
		{
			map["Scrartcl"] = true;
			map["Scrbook"]  = true;
			map["Scrreprt"] = true;
			map["Scrlttr2"] = true;
		}
		else if ( filename=="beamer.cls" )  
			map["Beamer"] = true;
		else if ( filename=="prosper.cls" )
			map["Prosper"] = true;
		else if ( filename=="HA-prosper.sty" )
			map["HA-prosper"] = true;
	}
	
	// insert all standard templates, all user defined templates 
	// and those templates, which have a present class 
	Templates templ;
	for (int i=0; i< templ.count(); ++i)
	{
		QString classname = (*templ.at(i)).name;
		if ( !map.contains(classname) || map[classname]==true )
		{
			new TemplateItem(this,*templ.at(i));
		}
	}

	// sort alle items (item for 'Empty Document' will always be the first one
	sort();
	
	// set the default item, if its given
	for ( QIconViewItem *item = firstItem(); item; item = item->nextItem() )
	if ( static_cast<TemplateItem*>(item)->name() == m_selicon )
	{
	   	setSelected(item, true);
			ensureItemVisible(item);
	}
}

////////////////////// execute shell script //////////////////////

void NewFileWidget::searchClassFiles()
{
	QString command = "kpsewhich -format=tex scrartcl.cls beamer.cls prosper.cls HA-prosper.sty";
	
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
	  
	kdDebug() << "=== NewFileWidget::searchClassFiles() ====================" << endl;
	kdDebug() << "\texecute: " << command << endl;
	if ( ! m_proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) 
	{
		kdDebug() << "\tstart of shell process failed" << endl;
		addTemplates();
	}
}

void NewFileWidget::slotProcessOutput(KProcess*,char* buf,int len)
{
   m_output += QString::fromLocal8Bit(buf,len);
}


void NewFileWidget::slotProcessExited(KProcess *proc)
{
	if ( ! proc->normalExit() ) 
		m_output = QString::null;

	addTemplates();
}

////////////////////// NewFileWizard //////////////////////

NewFileWizard::NewFileWizard(QWidget *parent, const char *name )
  : KDialogBase(parent,name,true,i18n("New File"),KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true)
{
	// first read config
	m_config = kapp->config();
	m_config->setGroup("NewFileWizard");
	bool wizard = m_config->readBoolEntry("UseWizardWhenCreatingEmptyFile", false);
	int w = m_config->readNumEntry("width", -1);
	if ( w == -1 ) w = width();
	int h = m_config->readNumEntry("height", -1);
	if ( h == -1 ) h = height();
	QString selicon = m_config->readEntry("select", DEFAULT_EMPTY_CAPTION);
	
	// then create widget
   QWidget *page = new QWidget( this );
   setMainWidget(page);
   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

   topLayout->addWidget( new QLabel(i18n("Please select the type of document you want to create:"),page));

   m_iv = new NewFileWidget( page,selicon );
   topLayout->addWidget(m_iv);

   m_ckWizard = new QCheckBox(i18n("Start the Quick Start wizard when creating an empty file"), page);
   topLayout->addWidget(m_ckWizard);

   connect(m_iv,SIGNAL(doubleClicked ( QIconViewItem * )),SLOT(slotOk()));

	// set config entries
	m_ckWizard->setChecked(wizard);
	resize(w,h);
}

bool NewFileWizard::useWizard()
{
	return ( getSelection() && getSelection()->name() == DEFAULT_EMPTY_CAPTION && m_ckWizard->isChecked() );
}

void NewFileWizard::slotOk()
{
	m_config->setGroup("NewFileWizard");
	m_config->writeEntry("UseWizardWhenCreatingEmptyFile", m_ckWizard->isChecked());
	m_config->writeEntry("width", width());
	m_config->writeEntry("height", height());

	if (getSelection())
		m_config->writeEntry("select", getSelection()->name());

	accept();
}

NewFileWizard::~NewFileWizard()
{}



#include "newfilewizard.moc"
