/***************************************************************************
                          usermenudialog.cpp  -  description
                             -------------------
    begin                : Sun Jun 3 2001
    copyright            : (C) 2001 by Brachet Pascal
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "usermenudialog.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <klocale.h>


usermenudialog::usermenudialog( QWidget* parent,  const char* name)
    : QDialog( parent, name, true)
{
	setCaption(name);
  previous_index=0;
  QGridLayout *gbox = new QGridLayout( this, 6, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  combo1=new QComboBox(this,"combo");
  combo1->insertItem( i18n("Menu")+" 1" );
  combo1->insertItem( i18n("Menu")+" 2" );
  combo1->insertItem( i18n("Menu")+" 3" );
  combo1->insertItem( i18n("Menu")+" 4" );
  combo1->insertItem( i18n("Menu")+" 5" );
  combo1->insertItem( i18n("Menu")+" 6" );
  combo1->insertItem( i18n("Menu")+" 7" );
  combo1->insertItem( i18n("Menu")+" 8" );
  combo1->insertItem( i18n("Menu")+" 9" );
  combo1->insertItem( i18n("Menu")+" 10" );
  connect(combo1, SIGNAL(activated(int)),this,SLOT(change(int)));
  
  label1 = new QLabel( this, "label1" );
  label1->setText(i18n("Menu Item"));
  itemedit=new QLineEdit(this,"tag");

  label2 = new QLabel( this, "label2" );
  label2->setText(i18n("LaTeX Content"));
  tagedit=new QTextEdit(this,"tag");
  tagedit->setTextFormat(Qt::PlainText); 

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("Ok"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(slotOk()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addMultiCellWidget(combo1,0,0,0,1,0);
  gbox->addMultiCellWidget(label1,1,1,0,1,0);
  gbox->addMultiCellWidget(itemedit,2,2,0,1,0);
  gbox->addMultiCellWidget(label2,3,3,0,1,0);
  gbox->addMultiCellWidget(tagedit,4,4,0,1,0);
  gbox->addWidget(buttonOk , 5, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 5, 1,Qt::AlignRight );
  resize(350,150);

}

usermenudialog::~usermenudialog()
{
}

void usermenudialog::init()
{
tagedit->setText(Tag[0]);
itemedit->setText(Name[0]);
combo1->setCurrentItem(0);  
}

void usermenudialog::change(int index)
{
Tag[previous_index]=tagedit->text();
Name[previous_index]=itemedit->text();
tagedit->setText(Tag[index]);
itemedit->setText(Name[index]);
previous_index=index;
}  

void usermenudialog::slotOk()
{
Tag[previous_index]=tagedit->text();
Name[previous_index]=itemedit->text();  
accept();
}

#include "usermenudialog.moc"
