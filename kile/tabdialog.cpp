/***************************************************************************
                          tabdialog.cpp  -  description
                             -------------------
    begin                : Mon Apr 30 2001
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

#include "tabdialog.h"
#include <klocale.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qtable.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

tabdialog::tabdialog(QWidget *parent, const char *name)
    :QDialog( parent,name, true)
{
	setCaption(name);

  QGridLayout *gbox = new QGridLayout( this, 7, 2,5,5,"");
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
  QLabel_1->setText(i18n("Num of Rows"));

  QLabel_2= new QLabel(this,"NoName");
  QLabel_2->setText(i18n("Num of Columns"));

  QLabel_3= new QLabel(this,"NoName");
  QLabel_3->setText(i18n("Columns Alignment"));

  combo1 = new QComboBox( FALSE, this, "comboBox1" );
  combo1->insertItem(i18n( "Center") );
  combo1->insertItem(i18n( "Left" ));
  combo1->insertItem(i18n( "Right" ));
  combo1->insertItem( "p{}");

  QLabel_4= new QLabel(this,"NoName");
  QLabel_4->setText(i18n("Vertical Separator"));

  combo2 = new QComboBox( FALSE, this, "comboBox2" );
  combo2->insertItem("|");
  combo2->insertItem("||");
  combo2->insertItem("none");
  combo2->insertItem( "@{text}" );

  checkbox1 = new QCheckBox( this, "checkbox");
  checkbox1->setFocusPolicy( QWidget::TabFocus );
  checkbox1->setText( i18n("Horizontal Separator") );
  checkbox1->setAutoRepeat( FALSE );
  checkbox1->setChecked( TRUE );

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("Ok"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addMultiCellWidget(Table1,0,0,0,1,0);
  gbox->addWidget(QLabel_1 , 1, 0 );
  gbox->addWidget(spinBoxRows , 1, 1 );
  gbox->addWidget(QLabel_2 , 2, 0 );
  gbox->addWidget(spinBoxCollums , 2, 1 );
  gbox->addWidget(QLabel_3 , 3, 0 );
  gbox->addWidget(combo1 , 3, 1 );
  gbox->addWidget(QLabel_4 , 4, 0 );
  gbox->addWidget(combo2 , 4, 1 );
  gbox->addMultiCellWidget(checkbox1,5,5,0,1,0);
  gbox->addWidget(buttonOk , 6, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 6, 1,Qt::AlignRight );
  this->resize(460,350);
}

tabdialog::~tabdialog(){
}
void tabdialog::NewRows(int num)
{
  Table1->setNumRows( num );
}
void tabdialog::NewCollums(int num)
{
  Table1->setNumCols( num );
}

#include "tabdialog.moc"
