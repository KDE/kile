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
	m_runlyxserver->setChecked(m_config->readBoolEntry("RunLyxServer", true));
	comboLatexHelp->setCurrentText(m_config->readEntry("LatexHelp","Embedded Viewer"));
	
	m_config->setGroup("Tool/ViewDVI");
	if ( m_config->readEntry("type","Part") == "Part" )
		comboDvi->setCurrentText(i18n("Embedded Viewer"));
	else
		comboDvi->setCurrentText(m_config->readEntry("command") + " " + m_config->readEntry("options"));

	m_config->setGroup("Tool/ViewPDF");
	if ( m_config->readEntry("type","Part") == "Part" )
		comboPdf->setCurrentText(i18n("Embedded Viewer"));
	else
		comboPdf->setCurrentText(m_config->readEntry("command") + " " + m_config->readEntry("options"));

	m_config->setGroup("Tool/ViewPS");
	if ( m_config->readEntry("type","Part") == "Part" )
		comboPs->setCurrentText(i18n("Embedded Viewer"));
	else
		comboPs->setCurrentText(m_config->readEntry("command") + " " + m_config->readEntry("options"));

	m_config->setGroup("Tool/LaTeX");
	LineEdit6->setText(m_config->readEntry("command","latex") + " " + m_config->readEntry("options","-interaction=nonstopmode '%source'"));
	m_config->setGroup("Tool/PDFLaTeX");
	LineEdit7->setText(m_config->readEntry("command","pdflatex") + " " + m_config->readEntry("options","-interaction=nonstopmode '%source'"));
	m_config->setGroup("Tool/DVItoPDF");
	LineEdit9->setText(m_config->readEntry("command","dvipdfm") + " " + m_config->readEntry("options"," '%S.dvi'"));
	m_config->setGroup("Tool/DVItoPS");
	LineEdit10->setText(m_config->readEntry("command","dvips") + " " + m_config->readEntry("options"," -o '%S.ps' '%S.dvi'"));
	m_config->setGroup("Tool/PStoPDF");
	LineEdit11->setText(m_config->readEntry("command","ps2pdf") + " " + m_config->readEntry("options"," '%S.ps' '%S.pdf'"));
	m_config->setGroup("Tool/MakeIndex");
	LineEdit12->setText(m_config->readEntry("command","makeindex") + " " + m_config->readEntry("options"," '%S.idx'"));
	m_config->setGroup("Tool/BibTeX");
	LineEdit13->setText(m_config->readEntry("command","bibtex") + " " + m_config->readEntry("options"," '%S'"));
	m_config->setGroup("Tool/ViewBib");
	LineEdit14->setText(m_config->readEntry("command","gbib") + " " + m_config->readEntry("options"," '%S.bib'"));
	


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
	m_config->writeEntry("RunLyxServer", m_runlyxserver->isChecked());

	m_config->setGroup("Tool/LaTeX");
	m_config->writeEntry("command",LineEdit6->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit6->text().section(' ',1));

	m_config->setGroup("Tool/DVItoPS");
	m_config->writeEntry("command",LineEdit10->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit10->text().section(' ',1));

	m_config->setGroup("Tool/ViewBib");
	m_config->writeEntry("command",LineEdit14->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit14->text().section(' ',1));

	m_config->setGroup("Tool/PStoPDF");
	m_config->writeEntry("command",LineEdit11->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit11->text().section(' ',1));

	m_config->setGroup("Tool/MakeIndex");
	m_config->writeEntry("command",LineEdit12->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit12->text().section(' ',1));

	m_config->setGroup("Tool/BibTeX");
	m_config->writeEntry("command",LineEdit13->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit13->text().section(' ',1));

	m_config->setGroup("Tool/PDFLaTeX");
	m_config->writeEntry("command",LineEdit7->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit7->text().section(' ',1));

	m_config->setGroup("Tool/DVItoPDF");
	m_config->writeEntry("command",LineEdit9->text().section(' ',0,0));
	m_config->writeEntry("options",LineEdit9->text().section(' ',1));

	m_config->setGroup("Tool/ViewPDF");
	if (comboPdf->currentText() == i18n("Embedded Viewer"))
	{
		m_config->writeEntry("type","Part");
	}
	else
	{
		m_config->writeEntry("type","Process");
		m_config->writeEntry("command",comboPdf->currentText().section(' ',0,0));
		m_config->writeEntry("options",comboPdf->currentText().section(' ',1));
	}

	m_config->setGroup("Tool/ViewDVI");
	if (comboDvi->currentText() == i18n("Embedded Viewer"))
	{
		m_config->writeEntry("type","Part");
	}
	else
	{
		m_config->writeEntry("type","Process");
		m_config->writeEntry("command",comboDvi->currentText().section(' ',0,0));
		m_config->writeEntry("options",comboDvi->currentText().section(' ',1));
	}

	m_config->setGroup("Tool/ViewPS");
	if (comboPs->currentText() == i18n("Embedded Viewer"))
	{
		m_config->writeEntry("type","Part");
	}
	else
	{
		m_config->writeEntry("type","Process");
		m_config->writeEntry("command",comboPs->currentText().section(' ',0,0));
		m_config->writeEntry("options",comboPs->currentText().section(' ',1));
	}

	m_config->writeEntry("LatexHelp",comboLatexHelp->currentText());

	m_config->setGroup("Tool/QuickBuild");
	switch ( ButtonGroup2->id(ButtonGroup2->selected()) + 1)
	{
		case 1: m_config->writeEntry("sequence","LaTeX,DVItoPS,ViewPS");break;
		case 2: m_config->writeEntry("sequence","LaTeX,ViewDVI");break;
		case 3: m_config->writeEntry("sequence","LaTeX,ForwardDVI");break;
		case 4: m_config->writeEntry("sequence","PDFLaTeX,ViewPDF");break;
		case 5: m_config->writeEntry("sequence","LaTeX,DVItoPDF,ViewPDF");break;
		case 6: m_config->writeEntry("sequence","LaTeX,DVItoPS,PStoPDF,ViewPDF");break;
		default: m_config->writeEntry("sequence", "LaTeX,ViewDVI"); break;
	}

	m_config->setGroup("Tools");
 	m_config->writeEntry("Quick Mode",ButtonGroup2->id(ButtonGroup2->selected())+1);

	m_config->setGroup( "Editor Ext" );
	m_config->writeEntry("Complete Environment", checkEnv->isChecked());

	m_config->setGroup("Structure");
	m_config->writeEntry("DefaultLevel", spinLevel->value());
	m_config->sync();

	accept();
}


#include "kileconfigdialog.moc"
