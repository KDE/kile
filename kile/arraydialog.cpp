/***************************************************************************
                          arraydialog.cpp  -  description
                             -------------------
    begin                : ven sep 27 2002
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

#include "arraydialog.h"
#include "klocale.h"

arraydialog::arraydialog(QWidget *parent, const char *name, const QString &caption)
    :QDialog( parent,name, true)
{
	setCaption(caption);

  QGridLayout *gbox = new QGridLayout( this, 6, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );

  Table1 = new QTable( this, "Table1" );
  Table1->setNumRows( 2 );
  Table1->setNumCols( 2 );

  spinBoxRows= new QSpinBox(this,"NoName");
  spinBoxRows->setValue(2);
  spinBoxRows->setRange(1,99);
  connect( spinBoxRows, SIGNAL(valueChanged(int)),this, SLOT(NewRows(int)));

  spinBoxCollums= new QSpinBox(this,"NoName");
  spinBoxCollums->setValue(2);
  spinBoxCollums->setRange(1,99);
  connect( spinBoxCollums, SIGNAL(valueChanged(int)),this, SLOT(NewCollums(int)));

  QLabel_1= new QLabel(this,"NoName");
  QLabel_1->setText(i18n("Number of rows:"));

  QLabel_2= new QLabel(this,"NoName");
  QLabel_2->setText(i18n("Number of columns:"));

  QLabel_3= new QLabel(this,"NoName");
  QLabel_3->setText(i18n("Columns alignment:"));

  combo = new QComboBox( FALSE, this, "comboBox" );
  combo->insertItem(i18n( "Center") );
  combo->insertItem(i18n( "Left" ));
  combo->insertItem(i18n( "Right" ));

  QLabel_4= new QLabel(this,"NoName");
  QLabel_4->setText(i18n("Environment:"));

  combo2 = new QComboBox( FALSE, this, "comboBox2" );
  combo2->insertItem("array");
  combo2->insertItem("matrix");
  combo2->insertItem("pmatrix");
  combo2->insertItem("bmatrix");
  combo2->insertItem("vmatrix");
  combo2->insertItem("Vmatrix");

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addMultiCellWidget(Table1,0,0,0,1,0);
  gbox->addWidget(QLabel_1 , 1, 0 );
  gbox->addWidget(spinBoxRows , 1, 1 );
  gbox->addWidget(QLabel_2 , 2, 0 );
  gbox->addWidget(spinBoxCollums , 2, 1 );
  gbox->addWidget(QLabel_3 , 3, 0 );
  gbox->addWidget(combo , 3, 1 );
  gbox->addWidget(QLabel_4 , 4, 0 );
  gbox->addWidget(combo2 , 4, 1 );
  gbox->addWidget(buttonOk , 5, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 5, 1,Qt::AlignRight );
  this->resize(460,320);
}

arraydialog::~arraydialog(){
}
void arraydialog::NewRows(int num)
{
  Table1->setNumRows( num );
}
void arraydialog::NewCollums(int num)
{
  Table1->setNumCols( num );
}


#include "arraydialog.moc"
