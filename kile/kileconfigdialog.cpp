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

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvbox.h>
#include <qhbuttongroup.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <knuminput.h>
#include <knumvalidator.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksconfig.h>
#include <kconfig.h>
#include <kcolorbutton.h>
#include <qvbox.h>

#include "kileconfigdialog.h"

KileConfigDialog::KileConfigDialog(KConfig *config, QWidget* parent,  const char* name)
        :KDialogBase( KDialogBase::IconList, i18n("Configure Kile"), Ok|Cancel,Ok, parent, name, true, true ), m_config(config)
{
    setShowIconsInTreeList(true);

    generalPage = addPage(i18n("General"),i18n("General options"),
                          KGlobal::instance()->iconLoader()->loadIcon( "configure", KIcon::NoGroup, KIcon::SizeMedium ));

    QVBoxLayout *genLayout = new QVBoxLayout(generalPage,5);

    //autosave options
    QGroupBox *autosaveGroup = new QGroupBox(3,Qt::Horizontal,i18n("Autosave options"),generalPage);

    checkAutosave = new QCheckBox(autosaveGroup, "Autosave");
	checkAutosave->setText(i18n("Auto&save"));

	QLabel *lb = new QLabel(i18n("Interval &time in minutes (1 - 9999) : "),autosaveGroup);
    asIntervalInput=new KIntNumInput(autosaveGroup,"asIntervalInput");
    asIntervalInput->setRange(1, 9999, 1, false);
	lb->setBuddy(asIntervalInput);

	genLayout->addWidget(autosaveGroup);
	genLayout->setStretchFactor(autosaveGroup,0);

	//fill in autosave
	m_config->setGroup("Files");
	checkAutosave->setChecked(m_config->readBoolEntry("Autosave",true));
	asIntervalInput->setValue(m_config->readLongNumEntry("AutosaveInterval",600000)/60000);

	QGroupBox *structGroup = new QGroupBox(2, Qt::Horizontal, i18n("Structure view options"), generalPage);
	genLayout->addWidget(structGroup);
	genLayout->setStretchFactor(structGroup,0);

	//checkSwitchStruct
	checkSwitchStruct = new QCheckBox(i18n("Switch to structure view after &opening a file."), structGroup );
	m_config->setGroup("Structure");
	checkSwitchStruct->setChecked(m_config->readBoolEntry("SwitchToStructure",true));
	lb = new QLabel("", structGroup);

	//default structure level
	lb = new QLabel(i18n("Default expansion &level for the structure view (1 part - 5 subsubsection) : "),structGroup);
	spinLevel = new QSpinBox(1,5, 1, structGroup);
	m_config->setGroup("Structure");
	spinLevel->setValue(m_config->readNumEntry("DefaultLevel",1));
	lb->setBuddy(spinLevel);

	//reopen files and projects
	checkRestore = new QCheckBox(i18n("Reopen files and projects on startup."), generalPage );
	genLayout->addWidget(checkRestore);
	genLayout->setStretchFactor(checkRestore,5);
	m_config->setGroup("Files");
	checkRestore->setChecked(m_config->readBoolEntry("Restore",true));

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
	genLayout->setStretchFactor(templateGroup,0);

	//fill in template variables
	m_config->setGroup( "User" );
	templAuthor->setText(m_config->readEntry("Author",""));
	templDocClassOpt->setText(m_config->readEntry("DocumentClassOptions","a4paper,10pt"));
	templEncoding->setText(m_config->readEntry("Template Encoding",""));
    // ****************************************************************

    toolsPage = addPage(i18n("Tools"),i18n("Tools Configuration"),
                        KGlobal::instance()->iconLoader()->loadIcon( "gear", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *gbox1 = new QGridLayout( toolsPage,13,2,5,5,"" );
    gbox1->addRowSpacing( 0, fontMetrics().lineSpacing() );

	QGroupBox *gb = new QGroupBox(2, Qt::Horizontal, i18n("Compile tools"), toolsPage);
	gbox1->addWidget(gb,0,0);
    TextLabel6 = new QLabel( gb, "label6" );
    TextLabel6->setText("LaTeX");
    LineEdit6 = new QLineEdit( gb, "le6" );

	lb = new QLabel(gb,"");
	checkForRoot = new QCheckBox(i18n("Check if root document is a LaTeX root before running LaTeX on it."), gb );

	TextLabel7 = new QLabel( gb, "label7" );
    TextLabel7->setText("PdfLaTeX");
    LineEdit7 = new QLineEdit( gb, "le7" );

	TextLabel12 = new QLabel( gb, "label11" );
    TextLabel12->setText(i18n("Make Index"));
     LineEdit12 = new QLineEdit( gb, "le112" );

	TextLabel13 = new QLabel( gb, "label13" );
    TextLabel13->setText("BibTeX");
    LineEdit13 = new QLineEdit( gb, "le113" );

	TextLabel14 = new QLabel( gb, "label14" );
    TextLabel14->setText(i18n("BibTeX Editor"));
    LineEdit14 = new QLineEdit( gb, "le114" );
	lb = new QLabel(gb, "");
	m_runlyxserver = new QCheckBox(i18n("Let Kile process LyX commands sent by bibliography editors/viewers."), gb);

	gb = new QGroupBox(2, Qt::Horizontal, i18n("Convert tools"), toolsPage);
	gbox1->addWidget(gb,1,0);
    TextLabel9 = new QLabel( gb, "label9" );
    TextLabel9->setText(i18n("DVI to PDF"));
    LineEdit9 = new QLineEdit( gb, "le9" );

    TextLabel10 = new QLabel( gb, "label10" );
    TextLabel10->setText(i18n("DVI to PS"));
    LineEdit10 = new QLineEdit( gb, "le10" );

    TextLabel11 = new QLabel( gb, "label11" );
    TextLabel11->setText(i18n("PS to PDF"));
    LineEdit11 = new QLineEdit( gb, "le11" );

	gb = new QGroupBox(2, Qt::Horizontal, i18n("View tools"), toolsPage);
	gbox1->addWidget(gb,2,0);

    TextLabel1 = new QLabel( gb, "label1" );
    TextLabel1->setText(i18n( "Dvi Viewer") );

    comboDvi = new QComboBox( FALSE, gb, "comboDvi" );
    comboDvi->setEditable( true );
    comboDvi->insertItem("xdvi -editor \'kile %f -line %l\' %S.dvi");
    comboDvi->insertItem("kdvi '%S.dvi'");
    comboDvi->insertItem("kdvi --unique '%S.dvi'");
    comboDvi->insertItem(i18n("Embedded Viewer"));

    TextLabel2 = new QLabel(gb, "label2" );
    TextLabel2->setText(i18n( "PS Viewer") );
    comboPs = new QComboBox( FALSE, gb, "comboPs" );
    comboPs->setEditable( true );
    comboPs->insertItem("gv '%S.ps'");
    comboPs->insertItem("kghostview '%S.ps'");
    comboPs->insertItem(i18n("Embedded Viewer"));

    TextLabel3 = new QLabel( gb, "label3" );
    TextLabel3->setText(i18n( "Pdf Viewer" ));
    comboPdf = new QComboBox( FALSE, gb, "comboPdf" );
    comboPdf->setEditable( true );
    comboPdf->insertItem("xpdf '%S.pdf'");
    comboPdf->insertItem("acroread '%S.pdf'");
    comboPdf->insertItem("kghostview '%S.pdf'");
    comboPdf->insertItem(i18n("Embedded Viewer"));

    TextLabel8 = new QLabel( gb, "label8" );
    TextLabel8->setText(i18n( "Latex Reference" ));
    comboLatexHelp = new QComboBox( FALSE, gb, "comboPdf" );
    comboLatexHelp->setEditable( true );
    comboLatexHelp->insertItem(i18n("Embedded Viewer"));
    comboLatexHelp->insertItem(i18n("External Browser"));
    comboLatexHelp->insertItem(i18n("KDE Help"));

	//fill in tools
	m_config->setGroup("Tools");
	checkForRoot->setChecked(m_config->readBoolEntry("CheckForRoot",true));
	comboDvi->setCurrentText(m_config->readEntry("Dvi","Embedded Viewer"));
	comboPdf->setCurrentText(m_config->readEntry("Pdf","Embedded Viewer"));
	comboPs->setCurrentText(m_config->readEntry("Ps","Embedded Viewer"));
	comboLatexHelp->setCurrentText(m_config->readEntry("LatexHelp","Embedded Viewer"));
	LineEdit6->setText(m_config->readEntry("Latex","latex -interaction=nonstopmode '%S.tex'"));
	LineEdit7->setText(m_config->readEntry("Pdflatex","pdflatex -interaction=nonstopmode '%S.tex'"));
	LineEdit9->setText(m_config->readEntry("Dvipdf","dvipdfm '%S.dvi'"));
	LineEdit10->setText(m_config->readEntry("Dvips","dvips -o '%S.ps' '%S.dvi'"));
	LineEdit11->setText(m_config->readEntry("Ps2pdf","ps2pdf '%S.ps' '%S.pdf'"));
	LineEdit12->setText(m_config->readEntry("Makeindex","makeindex '%S.idx'"));
	LineEdit13->setText(m_config->readEntry("Bibtex","bibtex '%S'"));
	LineEdit14->setText(m_config->readEntry("Bibtexeditor","gbib '%S.bib'"));
	m_runlyxserver->setChecked(m_config->readBoolEntry("RunLyxServer", true));


    // ************************************************************************************************
    quickPage = addPage(i18n("Quick"),i18n("Quick Build"),
                        KGlobal::instance()->iconLoader()->loadIcon( "clock", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *gbox3 = new QGridLayout( quickPage,2,1,5,5,"" );
    gbox3->addRowSpacing( 0, fontMetrics().lineSpacing() );

    ButtonGroup2= new QButtonGroup(6, Qt::Vertical, quickPage);

    checkLatex = new QRadioButton(ButtonGroup2 , "checkLatex" );
    checkLatex->setText(i18n("LaTeX + dvips + View PS") );

    checkDvi = new QRadioButton(ButtonGroup2 , "checkDvi" );
    checkDvi->setText(i18n("LaTeX + View Dvi") );

    checkDviSearch = new QRadioButton(ButtonGroup2 , "checkDviSearch" );
    checkDviSearch->setText(i18n("LaTeX + Kdvi forward search"));

    checkPdflatex = new QRadioButton(ButtonGroup2 , "checkPdflatex" );
    checkPdflatex->setText(i18n("PDFLaTeX + View PDF"));

    checkDviPdf = new QRadioButton(ButtonGroup2 , "checkPdflatex" );
    checkDviPdf->setText(i18n("LaTeX + dvipdfm + View PDF"));

    checkPsPdf = new QRadioButton(ButtonGroup2 , "checkPdflatex" );
    checkPsPdf->setText(i18n("LaTeX + dvips + ps2pdf + View PDF"));

    gbox3->addMultiCellWidget(ButtonGroup2,0,0,0,1,0);

	m_config->setGroup("Tools");
	ButtonGroup2->setButton(m_config->readNumEntry( "Quick Mode",1)-1);
    // ************************************************************************************************

    spellingPage = addPage(i18n("Spelling"),i18n("Spelling Configuration"),
                           KGlobal::instance()->iconLoader()->loadIcon( "spellcheck", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *gbox2 = new QGridLayout( spellingPage,2,2,5,5,"" );
    gbox2->addRowSpacing( 0, fontMetrics().lineSpacing() );

    QGroupBox* GroupBox3= new QGroupBox(2,Qt::Horizontal,i18n("Spelling"),spellingPage, "ButtonGroup" );

    ksc = new KSpellConfig(GroupBox3, "spell",0, false );

    gbox2->addMultiCellWidget(GroupBox3,0,0,0,1,0);

	//LaTeX specific editing options
	editPage = addPage(i18n("LaTeX"),i18n("LaTeX specific editing options"),KGlobal::instance()->iconLoader()->loadIcon( "tex", KIcon::NoGroup, KIcon::SizeMedium ));

	QGridLayout *lay = new QGridLayout(editPage, 2,1);
	lay->setRowStretch(0,0);
	//QVBox *gbox4 = new QVBox(editPage);

	checkEnv = new QCheckBox(i18n("Automatically complete \\begin{env} with \\end{env}"),editPage);
	lay->addWidget(checkEnv,0,0);

	//fill in
	m_config->setGroup( "Editor Ext" );
	checkEnv->setChecked(m_config->readBoolEntry( "Complete Environment", true));

}


KileConfigDialog::~KileConfigDialog()
{}

void KileConfigDialog::slotOk()
{
	m_config->setGroup( "Files" );
	m_config->writeEntry("Restore", checkRestore->isChecked());

	m_config->writeEntry("Autosave",checkAutosave->isChecked());
	m_config->writeEntry("AutosaveInterval",asIntervalInput->value()*60000);

	m_config->setGroup( "User" );

	m_config->writeEntry("Author",templAuthor->text());
	m_config->writeEntry("DocumentClassOptions",templDocClassOpt->text());
	m_config->writeEntry("Template Encoding",templEncoding->text());

	m_config->setGroup("Tools");
	m_config->writeEntry("CheckForRoot",checkForRoot->isChecked());
	m_config->writeEntry("Latex",LineEdit6->text());
	m_config->writeEntry("LatexHelp",comboLatexHelp->currentText());
	m_config->writeEntry("Dvi",comboDvi->currentText());
	m_config->writeEntry("Dvips",LineEdit10->text());
	m_config->writeEntry("Ps",comboPs->currentText());
	m_config->writeEntry("Ps2pdf",LineEdit11->text());
	m_config->writeEntry("Makeindex",LineEdit12->text());
	m_config->writeEntry("Bibtex",LineEdit13->text());
	m_config->writeEntry("Pdflatex",LineEdit7->text());
	m_config->writeEntry("Pdf",comboPdf->currentText());
	m_config->writeEntry("Dvipdf",LineEdit9->text());
	m_config->writeEntry("Bibtexeditor",LineEdit14->text());
	m_config->writeEntry("RunLyxServer", m_runlyxserver->isChecked());

	m_config->writeEntry("Quick Mode",ButtonGroup2->id(ButtonGroup2->selected())+1);

	m_config->setGroup( "Editor Ext" );
	m_config->writeEntry("Complete Environment", checkEnv->isChecked());

	m_config->setGroup("Structure");
	m_config->writeEntry("SwitchToStructure", checkSwitchStruct->isChecked());
	m_config->writeEntry("DefaultLevel", spinLevel->value());
	m_config->sync();

	accept();
}


#include "kileconfigdialog.moc"
