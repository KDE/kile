/***************************************************************************
                          newfilewizard.cpp  -  description
                             -------------------
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
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

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include "newfilewizard.h"

TemplateItem::TemplateItem(QIconView * parent, const TemplateInfo & info) : QIconViewItem(parent,info.name, QPixmap(info.icon))
{
	m_info = info;
}

TemplateItem::~TemplateItem()
{
}

NewFileWidget::NewFileWidget(QWidget *parent , char *name) : KIconView(parent,name)
{
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

   Templates templ;
    for (int i=0; i< templ.count(); ++i)
    {
      new TemplateItem(this, *templ.at(i));
    }

   setSelected(emp, true);
   setMinimumHeight(120);
}

NewFileWizard::NewFileWizard(QWidget *parent, const char *name )
  : KDialogBase(parent,name,true,i18n("New File"),KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true)
{

   QWidget *page = new QWidget( this );
   setMainWidget(page);
   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

   topLayout->addWidget( new QLabel(i18n("Please select the type of document you want to create:"),page));

   m_iv = new NewFileWidget( page );
   topLayout->addWidget(m_iv);

   m_ckWizard = new QCheckBox(i18n("Start the Quick Start wizard when creating an empty file"), page);
   topLayout->addWidget(m_ckWizard);

   connect(m_iv,SIGNAL(doubleClicked ( QIconViewItem * )),SLOT(slotOk()));

   kapp->config()->setGroup("NewFileWizard");
   m_ckWizard->setChecked(kapp->config()->readBoolEntry("UseWizardWhenCreatingEmptyFile", false));
   int w = kapp->config()->readNumEntry("width", -1);
   if ( w != -1 ) resize(w, height());

   int h = kapp->config()->readNumEntry("height", -1);
   if ( h != -1 ) resize(width(), h);

	QString nme = kapp->config()->readEntry("select", DEFAULT_EMPTY_CAPTION);
	for ( QIconViewItem *item = m_iv->firstItem(); item; item = item->nextItem() )
	if ( static_cast<TemplateItem*>(item)->name() == nme )
	{
	   	m_iv->setSelected(item, true);
		m_iv->ensureItemVisible(item);
	}
}

bool NewFileWizard::useWizard()
{
	return ( getSelection() && getSelection()->name() == DEFAULT_EMPTY_CAPTION && m_ckWizard->isChecked() );
}

void NewFileWizard::slotOk()
{
	kapp->config()->setGroup("NewFileWizard");
	kapp->config()->writeEntry("UseWizardWhenCreatingEmptyFile", m_ckWizard->isChecked());
	kapp->config()->writeEntry("width", width());
	kapp->config()->writeEntry("height", height());

	if (getSelection())
		kapp->config()->writeEntry("select", getSelection()->name());

	accept();
}

NewFileWizard::~NewFileWizard()
{}



#include "newfilewizard.moc"
