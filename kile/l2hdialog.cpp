/***************************************************************************
                          l2hdialog.cpp  -  description
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

#include "l2hdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>

l2hdialog::l2hdialog(QWidget *parent, const char *name, const QString &caption)
    : QDialog(parent,name,true)
{
	setCaption(caption);
  QGridLayout *gbox = new QGridLayout( this, 3, 2,5,5,"" );
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  options_edit = new QLineEdit( this, "options_edit" );
  options_edit->setText( "" );
  options_edit->setMaxLength( 32767 );
  options_edit->setFocus();
  options_edit->setFixedWidth(200);
  gbox->addMultiCellWidget( options_edit,0,0,1,2,1 );

  QLabel_1= new QLabel(this,"NoName");
  QLabel_1->setMinimumSize(0,0);
  QLabel_1->setText(i18n("Options"));
  gbox->addWidget(QLabel_1 ,0,0,2 );

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

  gbox->addWidget(buttonOk , 1, 1,Qt::AlignLeft);
  gbox->addWidget(buttonCancel , 1, 2,Qt::AlignRight );

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );
  resize(210,60);
}

l2hdialog::~l2hdialog(){
}



#include "l2hdialog.moc"
