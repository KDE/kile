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
#include <ktextedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <klocale.h>



usermenudialog::usermenudialog(const QValueList<userItem> &list, QWidget* parent,  const char* name, const QString &caption)
    : QDialog( parent, name, true)
{
  setCaption(caption);
  previous_index=0;
  QGridLayout *gbox = new QGridLayout( this, 7, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  combo1=new QComboBox(this,"combo");
  for (uint i=0; i<list.size(); i++)
  {
	combo1->insertItem( QString::number(i+1)+": "+list[i].name );
  }
  connect(combo1, SIGNAL(activated(int)),this,SLOT(change(int)));

  label1 = new QLabel( this, "label1" );
  label1->setText(i18n("Menu item:"));
  itemedit=new QLineEdit(this,"tag");

  label2 = new QLabel( this, "label2" );
  label2->setText(i18n("LaTeX content:"));
  tagedit=new KTextEdit(this,"tag");
  tagedit->setTextFormat(Qt::PlainText);

  QLabel *label3 = new QLabel( i18n("Action"), this,"label3");
  QButtonGroup *bgroup = new QButtonGroup(3,Qt::Horizontal,this);
  radioEdit = new QRadioButton(i18n("Edit"),bgroup);
  radioAdd = new QRadioButton(i18n("Add"),bgroup);
  radioRemove = new QRadioButton(i18n("Remove"),bgroup);
  if (list.size()==0)
  {
  	radioAdd->setChecked(true);
  }
  else
  {
  	radioEdit->setChecked(true);
  }

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
  gbox->addMultiCellWidget(tagedit,4,4,0,1,0);
  gbox->addWidget(label3, 5,0, Qt::AlignRight);
  gbox->addWidget(bgroup, 5,1, Qt::AlignLeft);
  gbox->addWidget(buttonOk , 6, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 6, 1,Qt::AlignRight );
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

int usermenudialog::result() const
{
	if (radioEdit->isChecked()) return Edit;
	if (radioAdd->isChecked()) return Add;
	if (radioRemove->isChecked()) return Remove;
	return Edit;
}

#include "usermenudialog.moc"
