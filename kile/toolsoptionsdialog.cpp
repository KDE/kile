/***************************************************************************
                          toolsoptionsdialog.cpp  -  description
                             -------------------
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
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
#include <qhbuttongroup.h>
#include <qgroupbox.h>
#include <qfontdatabase.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <knuminput.h>
#include <knumvalidator.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksconfig.h>
#include <kcolorbutton.h>

toolsoptionsdialog::toolsoptionsdialog( QWidget* parent,  const char* name)
:KDialogBase( IconList, i18n("Configure Kile"), Ok|Cancel,Ok, parent, name, true, true )
{

	generalPage = addPage(i18n("General"),i18n("General options"),
		KGlobal::instance()->iconLoader()->loadIcon( "configure", KIcon::NoGroup, KIcon::SizeMedium ));
	QVBoxLayout *genLayout = new QVBoxLayout(generalPage);

	//autosave options
	QGroupBox *autosaveGroup = new QGroupBox(3,Qt::Horizontal,i18n("Autosave options"),generalPage);

	checkAutosave = new QCheckBox(autosaveGroup, "Autosave");
	checkAutosave->setText(i18n("Auto&save"));

	QLabel *lb= new QLabel(i18n("Interval &time in minutes (1 - 9999) : "),autosaveGroup);


        asIntervalInput=new KIntNumInput(autosaveGroup,"asIntervalInput");
        asIntervalInput->setRange(1, 9999, 1, false);

	genLayout->addWidget(autosaveGroup);

	//template variables
	QGroupBox *templateGroup = new QGroupBox(2,Qt::Horizontal, i18n("Template variables"), generalPage);

	QLabel *lbAuthor = new QLabel(i18n("&Author"),templateGroup);
	templAuthor = new QLineEdit(templateGroup, "templAuthor");
	lbAuthor->setBuddy(templAuthor);
	QLabel *lbDocClassOpt = new QLabel(i18n("&Documentclass options"),templateGroup);
	templDocClassOpt = new QLineEdit(templateGroup, "templDocClassOpt");
	lbDocClassOpt->setBuddy(templDocClassOpt);
	QLabel *lbEnc = new QLabel(i18n("Input &encoding"), templateGroup);
	templEncoding = new QLineEdit(templateGroup, "templEncoding");
	lbEnc->setBuddy(templEncoding);

	genLayout->addWidget(templateGroup);

  toolsPage = addPage(i18n("Tools"),i18n("Tools Configuration"),
  	KGlobal::instance()->iconLoader()->loadIcon( "gear", KIcon::NoGroup, KIcon::SizeMedium ));

   QGridLayout *gbox1 = new QGridLayout( toolsPage,2,2,5,5,"" );
   gbox1->addRowSpacing( 0, fontMetrics().lineSpacing() );

   QVButtonGroup* ButtonGroup= new QVButtonGroup(i18n("Quick Build"), toolsPage, "ButtonGroup" );
   ButtonGroup->setExclusive( TRUE );

   checkLatex = new QRadioButton(ButtonGroup , "checkLatex" );
   checkLatex->setText(i18n("LaTeX + dvips + View PS") );

   checkDvi = new QRadioButton(ButtonGroup , "checkDvi" );
   checkDvi->setText(i18n("LaTeX + View Dvi") );

   checkDviSearch = new QRadioButton(ButtonGroup , "checkDviSearch" );
   checkDviSearch->setText(i18n("LaTeX + Kdvi forward search"));

   checkPdflatex = new QRadioButton(ButtonGroup , "checkPdflatex" );
   checkPdflatex->setText(i18n("PDFLaTeX + View PDF"));

   checkDviPdf = new QRadioButton(ButtonGroup , "checkPdflatex" );
   checkDviPdf->setText(i18n("LaTeX + dvipdfm + View PDF"));

   checkPsPdf = new QRadioButton(ButtonGroup , "checkPdflatex" );
   checkPsPdf->setText(i18n("LaTeX + dvips + ps2pdf + View PDF"));

   QGroupBox* GroupBox1= new QGroupBox(2,Qt::Horizontal,i18n("Tools"),toolsPage, "ButtonGroup" );

   TextLabel1 = new QLabel( GroupBox1, "label1" );
   TextLabel1->setText(i18n( "Dvi viewer:") );
   comboDvi = new QComboBox( FALSE, GroupBox1, "comboDvi" );
   comboDvi->setEditable( true );
   comboDvi->insertItem("xdvi -editor \'kile %f -line %l\' %S.dvi");
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

   spellingPage = addPage(i18n("Spelling"),i18n("Spelling Configuration"),
   	KGlobal::instance()->iconLoader()->loadIcon( "spellcheck", KIcon::NoGroup, KIcon::SizeMedium ));

   QGridLayout *gbox2 = new QGridLayout( spellingPage,2,2,5,5,"" );
   gbox2->addRowSpacing( 0, fontMetrics().lineSpacing() );

   QGroupBox* GroupBox3= new QGroupBox(2,Qt::Horizontal,i18n("Spelling"),spellingPage, "ButtonGroup" );
   ksc = new KSpellConfig(GroupBox3, "spell",0, false );

   gbox2->addMultiCellWidget(GroupBox3,0,0,0,1,0);

}


toolsoptionsdialog::~toolsoptionsdialog()
{
}


#include "toolsoptionsdialog.moc"
