/***************************************************************************
                          managetemplatesdialog.cpp  -  description
                             -------------------
    begin                : Sun Apr 27 2003
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

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kicondialog.h>
#include <kmessagebox.h>

#include "templates.h"
#include "managetemplatesdialog.h"

ManageTemplatesDialog::ManageTemplatesDialog(QFileInfo src, const char *caption, QWidget *parent, const char *name ) : KDialogBase(parent,name,true,caption,KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true) {
   m_sourceTemplate.name=src.baseName().replace(".tex","");
   m_sourceTemplate.path=src.absFilePath();
   m_sourceTemplate.icon=KGlobal::dirs()->findResource("appdata","pics/type_Default.png");

   QWidget *page = new QWidget( this , "managetemplates_mainwidget");
   setMainWidget(page);
   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

   QHBoxLayout *nameLayout = new QHBoxLayout( 0,0,spacingHint() );
   topLayout->addLayout(nameLayout);
   nameLayout->addWidget(new QLabel(i18n("Name :"),page));

   m_nameEdit = new KLineEdit(m_sourceTemplate.name,page);
   nameLayout->addWidget(m_nameEdit);

   QHBoxLayout *iconLayout = new QHBoxLayout(0 ,0,spacingHint());
   topLayout->addLayout(iconLayout);
   iconLayout->addWidget(new QLabel(i18n("Icon :"),page));

   m_iconEdit = new KLineEdit(m_sourceTemplate.icon,page);
   iconLayout->addWidget(m_iconEdit);

   KPushButton *iconbut = new KPushButton("Select...",page);
   iconLayout->addWidget(iconbut);

   KListView *tlist = new KListView(page);
   tlist->setSorting(-1);
   tlist->addColumn("M");
   tlist->addColumn("Existing templates");
   tlist->setColumnWidthMode(0,QListView::Manual);
   tlist->setFullWidth(true);
   tlist->setAllColumnsShowFocus(true);

   m_Templates = new Templates();
   QFileInfo fi;
   QString mode;
   
   for (int i=m_Templates->count()-1; i>=0; i--) {
      fi.setFile((*m_Templates->at(i)).path);
      mode = fi.isWritable() ? " " : "*";
      (void) new QListViewItem( tlist, mode,(*m_Templates->at(i)).name );
   }
   
   topLayout->addWidget(tlist);

   topLayout->addWidget( new QLabel(i18n("Select an existing template if you want to overwrite it with your new template.\nNote that you cannot overwrite templates marked with an asterix,\nif you do select such a template, a new template with the same name\nwill be created in a location you have write access to."),page));
   
   connect(tlist,SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectedTemplate(QListViewItem*)));
   connect(iconbut,SIGNAL(clicked()),this, SLOT(slotSelectIcon()));
   connect(this,SIGNAL(okClicked()),this,SLOT(addTemplate()));
}

ManageTemplatesDialog::~ManageTemplatesDialog(){
}

void ManageTemplatesDialog::slotSelectedTemplate(QListViewItem *item) {

   TemplateListIterator result= m_Templates->find(item->text(1));
   
   if ( result != NULL ) {
      m_sourceTemplate.name=(*result).name;
      m_sourceTemplate.name=(*result).icon;
      
      m_nameEdit->setText((*result).name);
      m_iconEdit->setText((*result).icon);
   }
}

void ManageTemplatesDialog::slotSelectIcon() {
   KIconDialog *dlg = new KIconDialog();
   QString res = dlg->openDialog();
   KIconLoader kil;
   
   if (res != QString::null ) {
         m_sourceTemplate.icon = kil.iconPath(res,-KIcon::SizeLarge,false);
         m_iconEdit->setText(m_sourceTemplate.icon);
   }
}

void ManageTemplatesDialog::addTemplate() {
  TemplateInfo dstTemplate = m_sourceTemplate;
  dstTemplate.name=m_nameEdit->text();
  //dstTemplate:
  //name : name of the template to be created
  //path : path to the file that has the template content
  //icon : path to the selected template icon

  QFileInfo fi(dstTemplate.path);

  if (!fi.isFile()) {
     KMessageBox::error(0,i18n("Sorry but the file: "+dstTemplate.path+"\ndoes not seem to exist. Maybe you forgot to save the file?"));
     return;
  }

  if (!m_Templates->add(dstTemplate) ) {
     KMessageBox::error(0,i18n("Failed to create the template."));
  }
  else
  {
     KMessageBox::information(0,i18n("Template successfully created."));
  }
}
#include "managetemplatesdialog.moc"
