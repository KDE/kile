/***************************************************************************
                          structdialog.cpp  -  description
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

#include "structdialog.h"
#include <klocale.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlayout.h>

structdialog::structdialog(QWidget *parent, const char *name)
    : QDialog(parent,name,true)
{
	setCaption(name);
  QGridLayout *gbox = new QGridLayout( this, 3, 3,5,5,"" );
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  title_edit = new QLineEdit( this, "title_edit" );
  title_edit->setFocusPolicy( QWidget::StrongFocus );
  title_edit->setText( "" );
  title_edit->setMaxLength( 32767 );
  title_edit->setEchoMode( QLineEdit::Normal );
  title_edit->setFocus();
  title_edit->setFixedWidth(200);
  gbox->addMultiCellWidget(title_edit,0,0,1,2,1 );

  QLabel_1= new QLabel(this,"NoName");
  QLabel_1->setMinimumSize(0,0);
  QLabel_1->setText(i18n("Title"));
  gbox->addWidget(QLabel_1 ,0,0,2 );

  checkbox = new QCheckBox( this, "checkbox");
  checkbox->setFocusPolicy( QWidget::TabFocus );
  checkbox->setText(i18n("Numeration") );
  checkbox->setAutoRepeat( FALSE );
  checkbox->setChecked( TRUE );
  gbox->addMultiCellWidget(checkbox,1,1,1,2,1 );

  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

  gbox->addWidget(buttonOk , 2, 1,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 2, 2,Qt::AlignRight );

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );
  resize(210,90);
}

structdialog::~structdialog(){
}



#include "structdialog.moc"
