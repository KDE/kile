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
#include <kmessagebox.h>
#include "newfilewizard.h"

TemplateItem::TemplateItem(QIconView * parent, const TemplateInfo & info) : QIconViewItem(parent,info.name,QPixmap(info.icon))
{
	m_info = info;
}

TemplateItem::~TemplateItem()
{
}

NewFileWizard::NewFileWizard(QWidget *parent, const char *name )
  : KDialogBase(parent,name,true,i18n("New File"),KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true)
{

   QWidget *page = new QWidget( this );
   setMainWidget(page);
   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

   topLayout->addWidget( new QLabel(i18n("Please select the type of document you want to create:"),page));

   iv = new KIconView( page );
   iv->setItemsMovable(false);
   iv->setResizeMode(QIconView::Fixed);
   iv->setSelectionMode(QIconView::Single);
   iv->setResizePolicy(QScrollView::AutoOneFit);
   iv->setArrangement(QIconView::TopToBottom);

   TemplateInfo info;
   info.name =DEFAULT_EMPTY_CAPTION;
   info.icon =KGlobal::dirs()->findResource("appdata",DEFAULT_EMPTY_ICON );
   info.path="";
   TemplateItem * emp = new TemplateItem( iv, info);

   Templates templ;
   for (int i=0; i< templ.count(); i++) {
      //(void) new QIconViewItem( iv, (*templ.at(i)).name, QPixmap( (*templ.at(i)).icon )  );
      (void) new TemplateItem(iv, *templ.at(i));
   }

   iv->setSelected(emp,true);
   iv->ensureItemVisible(emp);
   iv->setMinimumHeight(100);

   topLayout->addWidget(iv);
   connect(iv,SIGNAL(doubleClicked ( QIconViewItem * )),SLOT(accept()));

}

NewFileWizard::~NewFileWizard(){
}



#include "newfilewizard.moc"
