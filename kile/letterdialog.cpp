/***************************************************************************
                          letterdialog.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2001
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

#include "letterdialog.h"
#include <klocale.h>

letterdialog::letterdialog(QWidget *parent, const char *name)
    :QDialog( parent, name, true)
{
	setCaption(name);
  QGridLayout *gbox = new QGridLayout( this, 5, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );

  QLabel_2= new QLabel(this,"NoName");
  QLabel_2->setText(i18n("Typeface Size"));

  QLabel_3= new QLabel(this,"NoName");
  QLabel_3->setText(i18n("Paper Size"));

  QLabel_4= new QLabel(this,"NoName");
  QLabel_4->setText(i18n("Encoding"));

  combo2 = new QComboBox( FALSE, this, "comboBox" );
  combo2->insertItem( "10pt" );
  combo2->insertItem( "11pt" );
  combo2->insertItem( "12pt" );

  combo3 = new QComboBox( FALSE, this, "comboBox" );
  combo3->insertItem( "a4paper" );
  combo3->insertItem( "a5paper" );
  combo3->insertItem( "b5paper" );
  combo3->insertItem( "letterpaper" );
  combo3->insertItem( "legalpaper" );
  combo3->insertItem( "executivepaper" );

  combo4 = new QComboBox( FALSE, this, "comboBox" );
  combo4->insertItem( "latin1" );
  combo4->insertItem( "latin2" );
  combo4->insertItem( "latin3" );
  combo4->insertItem( "latin5" );
  combo4->insertItem( "ascii" );
  combo4->insertItem( "decmulti" );
  combo4->insertItem( "cp850" );
  combo4->insertItem( "cp852" );
  combo4->insertItem( "cp437" );
  combo4->insertItem( "cp437de" );
  combo4->insertItem( "cp865" );
  combo4->insertItem( "applemac" );
  combo4->insertItem( "next" );
  combo4->insertItem( "ansinew" );
  combo4->insertItem( "cp1252" );
  combo4->insertItem( "cp1250" );
  combo4->insertItem( "NONE" );

  checkbox1 = new QCheckBox( this, "checkbox");
  checkbox1->setFocusPolicy( QWidget::TabFocus );
  checkbox1->setText(i18n("AMS Packages"));
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

  gbox->addWidget(QLabel_2 , 0, 0 );
  gbox->addWidget(combo2 , 0, 1 );
  gbox->addWidget(QLabel_3 , 1, 0 );
  gbox->addWidget(combo3 , 1, 1 );
  gbox->addWidget(QLabel_4 , 2, 0 );
  gbox->addWidget(combo4 , 2, 1 );
  gbox->addWidget(checkbox1 , 3, 1 );
  gbox->addWidget(buttonOk , 4, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 4, 1,Qt::AlignRight );
  this->resize(300,150);
}

letterdialog::~letterdialog(){
}

#include "letterdialog.moc"
