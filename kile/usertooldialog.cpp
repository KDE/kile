/***************************************************************************
                          usertooldialog.cpp  -  description
                             -------------------
    begin                : mer avr 9 2003
    copyright            : (C) 2003 by Pascal Brachet
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

#include "usertooldialog.h"
#include <klocale.h>

#include <qwidget.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>


usertooldialog::usertooldialog(QWidget *parent, const char *name ) : QDialog(parent,name)
{
  	setCaption(name);
  previous_index=0;
  QGridLayout *gbox = new QGridLayout( this, 6, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  combo1=new QComboBox(this,"combo");
  combo1->insertItem( i18n("Command 1") );
  combo1->insertItem( i18n("Command 2") );
  combo1->insertItem( i18n("Command 3") );
  combo1->insertItem( i18n("Command 4") );
  combo1->insertItem( i18n("Command 5") );
  connect(combo1, SIGNAL(activated(int)),this,SLOT(change(int)));

  label1 = new QLabel( this, "label1" );
  label1->setText(i18n("Menu item:"));
  itemedit=new QLineEdit(this,"item");

  label2 = new QLabel( this, "label2" );
  label2->setText(i18n("Command (% : filename without extension):"));
  tooledit=new QLineEdit(this,"tool");

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(slotOk()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addMultiCellWidget(combo1,0,0,0,1,0);
  gbox->addMultiCellWidget(label1,1,1,0,1,0);
  gbox->addMultiCellWidget(itemedit,2,2,0,1,0);
  gbox->addMultiCellWidget(label2,3,3,0,1,0);
  gbox->addMultiCellWidget(tooledit,4,4,0,1,0);
  gbox->addWidget(buttonOk , 5, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 5, 1,Qt::AlignRight );
  resize(400,100);
}
usertooldialog::~usertooldialog()
{
}
void usertooldialog::init()
{
tooledit->setText(Tool[0]);
itemedit->setText(Name[0]);
combo1->setCurrentItem(0);
}

void usertooldialog::change(int index)
{
Tool[previous_index]=tooledit->text();
Name[previous_index]=itemedit->text();
tooledit->setText(Tool[index]);
itemedit->setText(Name[index]);
previous_index=index;
}

void usertooldialog::slotOk()
{
Tool[previous_index]=tooledit->text();
Name[previous_index]=itemedit->text();
accept();
}

#include "usertooldialog.moc"
