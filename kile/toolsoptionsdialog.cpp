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
#include <kconfig.h>
#include <kcolorbutton.h>
#include <qvbox.h>

#include "kile.h"
#include "toolsoptionsdialog.h"

KileConfigDialog::KileConfigDialog( Kile* parent,  const char* name)
        :KDialogBase( KDialogBase::TreeList, i18n("Configure Kile"), Ok|Cancel,Ok, parent, name, true, true ), m_kile(parent)
{
    setShowIconsInTreeList(true);

    generalPage = addPage(i18n("General"),i18n("General options"),
                          KGlobal::instance()->iconLoader()->loadIcon( "configure", KIcon::NoGroup, KIcon::SizeMedium ));

    QVBoxLayout *genLayout = new QVBoxLayout(generalPage);

    //autosave options
    QGroupBox *autosaveGroup = new QGroupBox(3,Qt::Horizontal,i18n("Autosave options"),generalPage);

    checkAutosave = new QCheckBox(autosaveGroup, "Autosave");
    checkAutosave->setText(i18n("Auto&save"));

    (void) new QLabel(i18n("Interval &time in minutes (1 - 9999) : "),autosaveGroup);

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

    // ****************************************************************

    toolsPage = addPage(i18n("Tools"),i18n("Tools Configuration"),
                        KGlobal::instance()->iconLoader()->loadIcon( "gear", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *gbox1 = new QGridLayout( toolsPage,10,2,5,5,"" );
    gbox1->addRowSpacing( 0, fontMetrics().lineSpacing() );

    TextLabel1 = new QLabel( toolsPage, "label1" );
    TextLabel1->setText(i18n( "Dvi Viewer") );
    gbox1->addWidget( TextLabel1,0,0 );
    comboDvi = new QComboBox( FALSE, toolsPage, "comboDvi" );

    comboDvi->setEditable( true );
    comboDvi->insertItem("xdvi -editor \'kile %f -line %l\' %S.dvi");
    comboDvi->insertItem("kdvi %S.dvi");
    comboDvi->insertItem("kdvi --unique %S.dvi");
    comboDvi->insertItem("Embedded Viewer");
    gbox1->addWidget( comboDvi,0,1 );


    TextLabel2 = new QLabel( toolsPage, "label2" );
    TextLabel2->setText(i18n( "PS Viewer") );
    gbox1->addWidget( TextLabel2,1,0 );
    comboPs = new QComboBox( FALSE, toolsPage, "comboPs" );
    comboPs->setEditable( true );
    comboPs->insertItem("gv %S.ps");
    comboPs->insertItem("kghostview %S.ps");
    comboPs->insertItem("Embedded Viewer");
    gbox1->addWidget( comboPs,1,1 );

    TextLabel3 = new QLabel( toolsPage, "label3" );
    TextLabel3->setText(i18n( "Pdf Viewer" ));
    gbox1->addWidget( TextLabel3,2,0 );
    comboPdf = new QComboBox( FALSE, toolsPage, "comboPdf" );
    comboPdf->setEditable( true );
    comboPdf->insertItem("xpdf %S.pdf");
    comboPdf->insertItem("acroread %S.pdf");
    comboPdf->insertItem("kghostview %S.pdf");
    comboPdf->insertItem("Embedded Viewer");
    gbox1->addWidget( comboPdf,2,1 );

    TextLabel6 = new QLabel( toolsPage, "label6" );
    TextLabel6->setText("LaTeX");
    gbox1->addWidget( TextLabel6,3,0 );
    LineEdit6 = new QLineEdit( toolsPage, "le6" );
    gbox1->addWidget( LineEdit6,3,1 );

    TextLabel7 = new QLabel( toolsPage, "label7" );
    TextLabel7->setText("PdfLaTeX");
    gbox1->addWidget( TextLabel7,4,0 );
    LineEdit7 = new QLineEdit( toolsPage, "le7" );
    gbox1->addWidget( LineEdit7,4,1 );

    TextLabel9 = new QLabel( toolsPage, "label9" );
    TextLabel9->setText(i18n("DVI to PDF"));
    gbox1->addWidget( TextLabel9,5,0 );
    LineEdit9 = new QLineEdit( toolsPage, "le9" );
    gbox1->addWidget( LineEdit9,5,1 );

    TextLabel10 = new QLabel( toolsPage, "label10" );
    TextLabel10->setText(i18n("DVI to PS"));
    gbox1->addWidget( TextLabel10,6,0 );
    LineEdit10 = new QLineEdit( toolsPage, "le10" );
    gbox1->addWidget( LineEdit10,6,1 );

    TextLabel11 = new QLabel( toolsPage, "label11" );
    TextLabel11->setText(i18n("PS to PDF"));
    gbox1->addWidget( TextLabel11,7,0 );
    LineEdit11 = new QLineEdit( toolsPage, "le11" );
    gbox1->addWidget( LineEdit11,7,1 );

    TextLabel12 = new QLabel( toolsPage, "label11" );
    TextLabel12->setText(i18n("Make Index"));
    gbox1->addWidget( TextLabel12,8,0 );
    LineEdit12 = new QLineEdit( toolsPage, "le112" );
    gbox1->addWidget( LineEdit12,8,1 );

    TextLabel13 = new QLabel( toolsPage, "label13" );
    TextLabel13->setText("BibTeX");
    gbox1->addWidget( TextLabel13,9,0 );
    LineEdit13 = new QLineEdit( toolsPage, "le113" );
    gbox1->addWidget( LineEdit13,9,1 );

    // ************************************************************************************************
    quickPage = addPage(i18n("Quick"),i18n("Quick Build"),
                        KGlobal::instance()->iconLoader()->loadIcon( "clock", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *gbox3 = new QGridLayout( quickPage,2,1,5,5,"" );
    gbox3->addRowSpacing( 0, fontMetrics().lineSpacing() );

    QVBox* ButtonGroup2= new QVBox( quickPage, "Bu22", 0);

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
    // ************************************************************************************************

    spellingPage = addPage(i18n("Spelling"),i18n("Spelling Configuration"),
                           KGlobal::instance()->iconLoader()->loadIcon( "spellcheck", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *gbox2 = new QGridLayout( spellingPage,2,2,5,5,"" );
    gbox2->addRowSpacing( 0, fontMetrics().lineSpacing() );

    QGroupBox* GroupBox3= new QGroupBox(2,Qt::Horizontal,i18n("Spelling"),spellingPage, "ButtonGroup" );

    ksc = new KSpellConfig(GroupBox3, "spell",0, false );

    gbox2->addMultiCellWidget(GroupBox3,0,0,0,1,0);

   // editPage = addPage(i18n("Edit"),i18n("LaTeX specific editing options"),
   //                    KGlobal::instance()->iconLoader()->loadIcon( "edit", KIcon::NoGroup, KIcon::SizeMedium ));


    m_view = m_kile->currentView();
	QStringList path;

	if (m_view)
	{
		QVBox *page;

		addPage(i18n("Editor"), i18n("Editor options"), KGlobal::instance()->iconLoader()->loadIcon( "edit", KIcon::NoGroup, KIcon::SizeMedium ));

		for (uint i = 0; i < KTextEditor::configInterfaceExtension (m_view->document())->configPages (); i++)
		{
			path.clear();
			path << i18n("Editor") << KTextEditor::configInterfaceExtension (m_view->document())->configPageName (i);
			page = addVBoxPage(path, KTextEditor::configInterfaceExtension (m_view->document())->configPageFullName (i),
							KTextEditor::configInterfaceExtension (m_view->document())->configPagePixmap(i, KIcon::SizeSmall) );

			KTextEditor::ConfigPage *cPage = KTextEditor::configInterfaceExtension (m_view->document())->configPage(i, page);
			//connect( cPage, SIGNAL( changed() ), this, SLOT( slotChanged() ) );
			editorPages.append (cPage);
		}
		path.clear();
		path << i18n("Editor") << "LaTeX";
		editPage = addVBoxPage(path, i18n("LaTeX specific editing options"),KGlobal::instance()->iconLoader()->loadIcon( "tex", KIcon::NoGroup, KIcon::SizeSmall ));

		QVBoxLayout *lay = new QVBoxLayout(editPage);
		QVBox *gbox4 = new QVBox(editPage);

		checkEnv = new QCheckBox(i18n("Automatically complete \\begin{env} with \\end{env}"),gbox4);

		lay->addWidget(gbox4);
	}
}


KileConfigDialog::~KileConfigDialog()
{}

void KileConfigDialog::slotOk()
{
	if (m_view)
	{
		for (uint i=0; i<editorPages.count(); i++)
		{
			editorPages.at(i)->apply();
		}

		m_view->getDoc()->writeConfig();
	}

	accept();
}


#include "toolsoptionsdialog.moc"
