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

NewFileWidget::NewFileWidget(QWidget *parent , char *name) : KIconView(parent,name)
{
   setItemsMovable(false);
   setResizeMode(QIconView::Fixed);
   setSelectionMode(QIconView::Single);
   setResizePolicy(QScrollView::AutoOneFit);
   setArrangement(QIconView::TopToBottom);

   TemplateInfo info;
   info.name =DEFAULT_EMPTY_CAPTION;
   info.icon =KGlobal::dirs()->findResource("appdata",DEFAULT_EMPTY_ICON );
   info.path="";
   TemplateItem * emp = new TemplateItem( this, info);

   Templates templ;
   for (int i=0; i< templ.count(); i++) {
      //(void) new QIconViewItem( iv, (*templ.at(i)).name, QPixmap( (*templ.at(i)).icon )  );
      (void) new TemplateItem(this, *templ.at(i));
   }

   setSelected(emp,true);
   ensureItemVisible(emp);
   setMinimumHeight(100);
}

NewFileWizard::NewFileWizard(QWidget *parent, const char *name )
  : KDialogBase(parent,name,true,i18n("New File"),KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true)
{

   QWidget *page = new QWidget( this );
   setMainWidget(page);
   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

   topLayout->addWidget( new QLabel(i18n("Please select the type of document you want to create:"),page));

   iv = new NewFileWidget( page );
   topLayout->addWidget(iv);

   connect(iv,SIGNAL(doubleClicked ( QIconViewItem * )),SLOT(accept()));

}

NewFileWizard::~NewFileWizard(){
}



#include "newfilewizard.moc"
