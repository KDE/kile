/***************************************************************************
                          tabbingdialog.cpp  -  description
                             -------------------
    begin                : dim jui 14 2002
    copyright            : (C) 2002 by Pascal Brachet
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

#include "tabbingdialog.h"
#include <klocale.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>

tabbingdialog::tabbingdialog(QWidget *parent, const char *name, const QString &caption )
: QDialog(parent,name,true)
{
	setCaption(caption);
  QGridLayout *gbox = new QGridLayout( this, 4, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  Label1= new QLabel(this,"Label1");
  Label1->setText(i18n("Num of columns:"));

  spinBoxCollums= new QSpinBox(this,"SpinBox1");
  spinBoxCollums->setValue(2);
  spinBoxCollums->setRange(2,99);

  Label2= new QLabel(this,"Label2");
  Label2->setText(i18n("Num of rows:"));

  spinBoxRows= new QSpinBox(this,"SpinBox2");
  spinBoxRows->setValue(1);
  spinBoxRows->setRange(1,99);

  Label3= new QLabel(this,"Label3");
  Label3->setText(i18n("Spacing:"));

  LineEdit1 = new QLineEdit( this, "LineEdit1" );
  LineEdit1->setFixedWidth(80);

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addWidget(Label1 , 0, 0 );
  gbox->addWidget( spinBoxCollums, 0, 1 );

  gbox->addWidget(Label2 , 1, 0 );
  gbox->addWidget( spinBoxRows, 1, 1 );

  gbox->addWidget(Label3 , 2, 0 );
  gbox->addWidget( LineEdit1, 2, 1 );

  gbox->addWidget(buttonOk , 3, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 3, 1,Qt::AlignRight );
  resize(130,120);

}
tabbingdialog::~tabbingdialog(){
}

#include "tabbingdialog.moc"
