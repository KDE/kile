/***************************************************************************
                          refdialog.cpp  -  description
                             -------------------
    begin                : dim déc 1 2002
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

#include "refdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>

refdialog::refdialog(QWidget *parent, const char *name)
    :QDialog( parent, name, true)
{
	setCaption(name);
  QGridLayout *gbox = new QGridLayout( this, 2, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );

  combo1 = new QComboBox( FALSE, this, "comboBox" );


  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addMultiCellWidget(combo1,0,0,0,1,Qt::AlignCenter);
  gbox->addWidget(buttonOk , 1, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 1, 1,Qt::AlignRight );
  this->resize(100,50);
}

refdialog::~refdialog(){
}
#include "refdialog.moc"
