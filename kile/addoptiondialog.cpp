/***************************************************************************
                          addoptiondialog.cpp  -  description
                             -------------------
    begin                : Sun Oct 20 2002
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

#include "addoptiondialog.h"
#include <qlayout.h>
#include <klocale.h>

AddOptionDialog::AddOptionDialog(QWidget *parent, const char *name, const QString &caption)
    : QDialog( parent, name,true )
{
   setCaption(caption);
   QGridLayout *gbox = new QGridLayout( this, 2, 2,5,5,"" );
   gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

   lineEdit = new QLineEdit( this, "" );
   lineEdit->setText("");
   lineEdit->setFixedWidth(250);
   gbox->addMultiCellWidget( lineEdit,0,0,0,1,1 );

   buttonOk= new QPushButton(this,"NoName");
   buttonOk->setMinimumSize(0,0);
   buttonOk->setText(i18n("&OK"));
   buttonOk->setDefault(true);

   buttonCancel= new QPushButton(this,"NoName");
   buttonCancel->setMinimumSize(0,0);
   buttonCancel->setText(i18n("&Cancel"));

   connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
   connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

   gbox->addWidget(buttonOk , 1, 0,Qt::AlignLeft );
   gbox->addWidget(buttonCancel , 1, 1,Qt::AlignRight);

   setFocusProxy( lineEdit );

}
AddOptionDialog::~AddOptionDialog()
{
}


#include "addoptiondialog.moc"
