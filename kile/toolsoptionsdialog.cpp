/***************************************************************************
                          toolsoptionsdialog.cpp  -  description
                             -------------------
    begin                : Wed Jun 6 2001
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

#include "toolsoptionsdialog.h"


#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qgroupbox.h>
#include <qfontdatabase.h>
#include <qframe.h>
#include <qpixmap.h>
#include <klocale.h>
#include <kiconloader.h>

#include <ksconfig.h>
#include <kcolorbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>

toolsoptionsdialog::toolsoptionsdialog( QWidget* parent,  const char* name)
:KDialogBase( IconList, i18n("Configure Kile"), Ok|Cancel,Ok, parent, name, true, true )
{

  toolsPage = addPage(i18n("Tools"),i18n("Tools Configuration"),
  KGlobal::instance()->iconLoader()->loadIcon( "gear", KIcon::NoGroup, KIcon::SizeMedium ));

   QGridLayout *gbox1 = new QGridLayout( toolsPage,2,2,5,5,"" );
   gbox1->addRowSpacing( 0, fontMetrics().lineSpacing() );

   QVButtonGroup* ButtonGroup= new QVButtonGroup(i18n("Quick Build"), toolsPage, "ButtonGroup" );
   ButtonGroup->setExclusive( TRUE );

   checkLatex = new QCheckBox(ButtonGroup , "checkLatex" );
   checkLatex->setText(i18n("LaTeX + dvips + View PS") );

   checkDvi = new QCheckBox(ButtonGroup , "checkDvi" );
   checkDvi->setText(i18n("LaTeX + View Dvi") );

   checkDviSearch = new QCheckBox(ButtonGroup , "checkDviSearch" );
   checkDviSearch->setText(i18n("LaTeX + Kdvi forward search"));

   checkPdflatex = new QCheckBox(ButtonGroup , "checkPdflatex" );
   checkPdflatex->setText(i18n("PDFLaTeX + View PDF"));

   checkDviPdf = new QCheckBox(ButtonGroup , "checkPdflatex" );
   checkDviPdf->setText(i18n("LaTeX + dvipdfm + View PDF"));

   checkPsPdf = new QCheckBox(ButtonGroup , "checkPdflatex" );
   checkPsPdf->setText(i18n("LaTeX + dvips + ps2pdf + View PDF"));

   QGroupBox* GroupBox1= new QGroupBox(2,Qt::Horizontal,i18n("Tools"),toolsPage, "ButtonGroup" );

   TextLabel1 = new QLabel( GroupBox1, "label1" );
   TextLabel1->setText(i18n( "Dvi viewer:") );
   comboDvi = new QComboBox( FALSE, GroupBox1, "comboDvi" );
   comboDvi->setEditable( true );
   comboDvi->insertItem("xdvi %S.dvi");
   comboDvi->insertItem("kdvi %S.dvi");
   comboDvi->insertItem("kdvi --unique %S.dvi");
   comboDvi->insertItem("Embedded Viewer");


   TextLabel2 = new QLabel( GroupBox1, "label2" );
   TextLabel2->setText(i18n( "PS viewer:") );
   comboPs = new QComboBox( FALSE, GroupBox1, "comboPs" );
   comboPs->setEditable( true );
   comboPs->insertItem("gv %S.ps");
   comboPs->insertItem("kghostview %S.ps");
   comboPs->insertItem("Embedded Viewer");



   TextLabel3 = new QLabel( GroupBox1, "label3" );
   TextLabel3->setText(i18n( "Pdf viewer:" ));
   comboPdf = new QComboBox( FALSE, GroupBox1, "comboPdf" );
   comboPdf->setEditable( true );
   comboPdf->insertItem("xpdf %S.pdf");
   comboPdf->insertItem("acroread %S.pdf");
   comboPdf->insertItem("kghostview %S.pdf");
   comboPdf->insertItem("Embedded Viewer");



   TextLabel6 = new QLabel( GroupBox1, "label6" );
   TextLabel6->setText(i18n("LaTeX:"));
   LineEdit6 = new QLineEdit( GroupBox1, "le6" );

   TextLabel7 = new QLabel( GroupBox1, "label7" );
   TextLabel7->setText(i18n("PdfLaTeX:"));
   LineEdit7 = new QLineEdit( GroupBox1, "le7" );

   gbox1->addMultiCellWidget(ButtonGroup,0,0,0,1,0);
   gbox1->addMultiCellWidget(GroupBox1,1,1,0,1,0);

   editorPage = addPage(i18n("Editor"),i18n("Editor Configuration"),
   KGlobal::instance()->iconLoader()->loadIcon( "edit", KIcon::NoGroup, KIcon::SizeMedium ));

   QGridLayout *gbox2 = new QGridLayout( editorPage,2,2,5,5,"" );
   gbox2->addRowSpacing( 0, fontMetrics().lineSpacing() );
   QGroupBox* GroupBox2= new QGroupBox(2,Qt::Horizontal,i18n("Editor"),editorPage, "ButtonGroup" );

   TextLabel4 = new QLabel( GroupBox2, "label4" );
   TextLabel4->setText( i18n("Editor font family:") );

   comboFamily = new QComboBox( FALSE, GroupBox2, "comboFamily" );
   comboFamily->setEditable( true );
   QFontDatabase fdb;
   comboFamily->insertStringList( fdb.families() );

   TextLabel5 = new QLabel( GroupBox2, "label5" );
   TextLabel5->setText( i18n("Editor font size:") );
   spinSize = new QSpinBox( GroupBox2, "spinSize" );
   spinSize->setMinValue( 1 );

   checkWordWrap = new QCheckBox( GroupBox2, "checkWordWrap" );
   checkWordWrap->setText(i18n( "Word wrap") );

   checkParen = new QCheckBox( GroupBox2, "checkParen" );
   checkParen->setText(i18n( "Braces matching") );

   checkLine = new QCheckBox( GroupBox2, "checkLine" );
   checkLine->setText(i18n( "Show line numbers") );

   TextLabel8 = new QLabel( GroupBox2, "label8" );
   TextLabel8->setText( "" );

   comboColor = new QComboBox( FALSE, GroupBox2, "comboColor" );
   comboColor->insertItem(i18n("Color Background"));
   comboColor->insertItem(i18n("Color Text"));
   comboColor->insertItem(i18n("Color Comment"));
   comboColor->insertItem(i18n("Color Math"));
   comboColor->insertItem(i18n("Color Command"));
   comboColor->insertItem(i18n("Color Structure"));
   comboColor->insertItem(i18n("Color Environment"));
   comboColor->insertItem(i18n("Bracket Highlight"));
   connect(comboColor, SIGNAL(activated(int)),this,SLOT(slotChangeColor(int)));

   buttonColor = new KColorButton(GroupBox2 , "buttonColor" );


   QGroupBox* GroupBox3= new QGroupBox(2,Qt::Horizontal,i18n("Spelling"),editorPage, "ButtonGroup" );
   ksc = new KSpellConfig(GroupBox3, "spell",0, false );

   gbox2->addMultiCellWidget(GroupBox2,0,0,0,1,0);
   gbox2->addMultiCellWidget(GroupBox3,1,1,0,1,0);
   connect( this, SIGNAL(okClicked()), SLOT(slotEnd()) );

}


toolsoptionsdialog::~toolsoptionsdialog()
{

}

void toolsoptionsdialog::init()
{
previous_index=0;
comboColor->setCurrentItem(0);
buttonColor->setColor(colors[0]);
}

void toolsoptionsdialog::slotChangeColor(int index)
{
colors[previous_index]=buttonColor->color();
buttonColor->setColor(colors[index]);
previous_index=index;
}

void toolsoptionsdialog::slotEnd()
{
colors[previous_index]=buttonColor->color();
}

#include "toolsoptionsdialog.moc"
