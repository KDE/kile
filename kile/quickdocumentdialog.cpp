/***************************************************************************
                          quickdocumentdialog.cpp  -  description
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

#include "quickdocumentdialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>

quickdocumentdialog::quickdocumentdialog(QWidget *parent, const char *name)
    :QDialog( parent, name, true)
{
	setCaption(name);
  QGridLayout *gbox = new QGridLayout( this, 10, 3,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );

  QLabel_1= new QLabel(this,"NoName");
  QLabel_1->setText(i18n("Document class:"));

  QLabel_2= new QLabel(this,"NoName");
  QLabel_2->setText(i18n("Typeface size:"));

  QLabel_3= new QLabel(this,"NoName");
  QLabel_3->setText(i18n("Paper size:"));

  QLabel_4= new QLabel(this,"NoName");
  QLabel_4->setText(i18n("Encoding:"));

  combo1 = new QComboBox( FALSE, this, "comboBox" );
  combo1->insertItem( "article" );
  combo1->insertItem( "report");
  combo1->insertItem( "letter" );
  combo1->insertItem( "book" );

  userClassBtn= new QPushButton(this,"NoName");
  userClassBtn->setMinimumSize(0,0);
  userClassBtn->setText("+");
  userClassBtn->setFixedWidth(30);
  connect(userClassBtn , SIGNAL(clicked()), SLOT(addUserClass()) );

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

  userPaperBtn= new QPushButton(this,"NoName");
  userPaperBtn->setMinimumSize(0,0);
  userPaperBtn->setText("+");
  userPaperBtn->setFixedWidth(30);
  connect(userPaperBtn , SIGNAL(clicked()), SLOT(addUserPaper()) );

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

  userEncodingBtn= new QPushButton(this,"NoName");
  userEncodingBtn->setMinimumSize(0,0);
  userEncodingBtn->setText("+");
  userEncodingBtn->setFixedWidth(30);
  connect(userEncodingBtn , SIGNAL(clicked()), SLOT(addUserEncoding()) );

  checkbox1 = new QCheckBox( this, "checkbox1");
  checkbox1->setFocusPolicy( QWidget::TabFocus );
  checkbox1->setText( i18n("AMS packages") );
  checkbox1->setAutoRepeat( FALSE );
  checkbox1->setChecked( TRUE );

  checkbox2 = new QCheckBox( this, "checkbox2");
  checkbox2->setFocusPolicy( QWidget::TabFocus );
  checkbox2->setText( i18n("makeidx package") );
  checkbox2->setAutoRepeat( FALSE );
  checkbox2->setChecked( FALSE );

  QLabel_5= new QLabel(this,"NoName");
  QLabel_5->setText(i18n("Other options:"));

  availableBox=new QListBox(this);
  availableBox->setSelectionMode(QListBox::Multi);
  availableBox->insertItem( "landscape" );
  availableBox->insertItem( "draft" );
  availableBox->insertItem( "final" );
  availableBox->insertItem( "oneside" );
  availableBox->insertItem( "twoside" );
  availableBox->insertItem( "openright" );
  availableBox->insertItem( "openany" );
  availableBox->insertItem( "onecolumn" );
  availableBox->insertItem( "twocolumn" );
  availableBox->insertItem( "titlepage" );
  availableBox->insertItem( "notitlepage" );
  availableBox->insertItem( "openbib" );
  availableBox->insertItem( "leqno" );
  availableBox->insertItem( "fleqn" );

  userOptionsBtn= new QPushButton(this,"NoName");
  userOptionsBtn->setMinimumSize(0,0);
  userOptionsBtn->setText("+");
  userOptionsBtn->setFixedWidth(30);
  connect(userOptionsBtn , SIGNAL(clicked()), SLOT(addUserOptions()) );

  QLabel_6= new QLabel(this,"NoName");
  QLabel_6->setText(i18n("Author:"));
  LineEdit1 = new QLineEdit(this, "le1" );

  QLabel_7= new QLabel(this,"NoName");
  QLabel_7->setText(i18n("Title:"));
  LineEdit2 = new QLineEdit(this, "le2" );


  buttonOk= new QPushButton(this,"NoName");
  buttonOk->setMinimumSize(0,0);
  buttonOk->setText(i18n("&OK"));
  buttonOk->setDefault(true);

  buttonCancel= new QPushButton(this,"NoName");
  buttonCancel->setMinimumSize(0,0);
  buttonCancel->setText(i18n("&Cancel"));

	connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
	connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

  gbox->addWidget(QLabel_1 , 0, 0 );
  gbox->addWidget(combo1 , 0, 1 );
  gbox->addWidget(userClassBtn , 0, 2,Qt::AlignLeft );
  gbox->addWidget(QLabel_2 , 1, 0 );
  gbox->addWidget(combo2 , 1, 1 );
  gbox->addWidget(QLabel_3 , 2, 0 );
  gbox->addWidget(combo3 , 2, 1 );
  gbox->addWidget(userPaperBtn , 2, 2,Qt::AlignLeft );
  gbox->addWidget(QLabel_4 , 3, 0 );
  gbox->addWidget(combo4 , 3, 1 );
  gbox->addWidget(userEncodingBtn , 3, 2,Qt::AlignLeft );
  gbox->addWidget(checkbox1 , 4, 0 );
  gbox->addWidget(checkbox2 , 4, 1 );
  gbox->addWidget(QLabel_6 , 5, 0 );
  gbox->addWidget(LineEdit1 ,5,1 );
  gbox->addWidget(QLabel_7 , 6, 0 );
  gbox->addWidget(LineEdit2 , 6,1);
  gbox->addMultiCellWidget(QLabel_5,7,7,0,1,Qt::AlignLeft);
  gbox->addMultiCellWidget(availableBox,8,8,0,1,0);
  gbox->addWidget(userOptionsBtn , 8, 2,Qt::AlignLeft );
  gbox->addWidget(buttonOk , 9, 0,Qt::AlignLeft );
  gbox->addWidget(buttonCancel , 9, 1,Qt::AlignRight );
  this->resize(280,340);
}

quickdocumentdialog::~quickdocumentdialog(){
}

void quickdocumentdialog::Init()
{
  combo1->clear();
  combo1->insertItem( "article" );
  combo1->insertItem( "report");
  combo1->insertItem( "letter" );
  combo1->insertItem( "book" );
  combo1->insertStringList(otherClassList);

  combo3->clear();
  combo3->insertItem( "a4paper" );
  combo3->insertItem( "a5paper" );
  combo3->insertItem( "b5paper" );
  combo3->insertItem( "letterpaper" );
  combo3->insertItem( "legalpaper" );
  combo3->insertItem( "executivepaper" );
  combo3->insertStringList(otherPaperList);

  combo4->clear();
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
  combo4->insertStringList(otherEncodingList);

  availableBox->clear();
  availableBox->insertItem( "landscape" );
  availableBox->insertItem( "draft" );
  availableBox->insertItem( "final" );
  availableBox->insertItem( "oneside" );
  availableBox->insertItem( "twoside" );
  availableBox->insertItem( "openright" );
  availableBox->insertItem( "openany" );
  availableBox->insertItem( "onecolumn" );
  availableBox->insertItem( "twocolumn" );
  availableBox->insertItem( "titlepage" );
  availableBox->insertItem( "notitlepage" );
  availableBox->insertItem( "openbib" );
  availableBox->insertItem( "leqno" );
  availableBox->insertItem( "fleqn" );
  availableBox->insertStringList(otherOptionsList);
}

void quickdocumentdialog::addUserClass()
{
QString newoption="";
dlg = new AddOptionDialog(this,i18n("New"));
  if ( dlg->exec() )
  {
  newoption=dlg->lineEdit->text();
  if (newoption!="") otherClassList.append(newoption);
  Init();
  }
delete (dlg);
}

void quickdocumentdialog::addUserPaper()
{
QString newoption="";
dlg = new AddOptionDialog(this,i18n("New"));
  if ( dlg->exec() )
  {
  newoption=dlg->lineEdit->text();
  if (newoption!="") otherPaperList.append(newoption);
  Init();
  }
delete (dlg);
}

void quickdocumentdialog::addUserEncoding()
{
QString newoption="";
dlg = new AddOptionDialog(this,i18n("New"));
  if ( dlg->exec() )
  {
  newoption=dlg->lineEdit->text();
  if (newoption!="") otherEncodingList.append(newoption);
  Init();
  }
delete (dlg);
}

void quickdocumentdialog::addUserOptions()
{
QString newoption="";
dlg = new AddOptionDialog(this,i18n("New"));
  if ( dlg->exec() )
  {
  newoption=dlg->lineEdit->text();
  if (newoption!="") otherOptionsList.append(newoption);
  Init();
  }
delete (dlg);
}

#include "quickdocumentdialog.moc"
