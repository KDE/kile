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

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kpushbutton.h>
#include <klocale.h>
#include <kcombobox.h>
#include <klistbox.h>
#include <klineedit.h>
#include <klineeditdlg.h>
#include <kconfig.h>

namespace KileDialog
{
	QuickDocument::QuickDocument(KConfig *config, QWidget *parent, const char *name, const QString &caption) :
		Wizard(config, parent,name,caption)
	{
		m_td.dy=3;
		m_td.dx=0;

		QWidget *page = new QWidget( this );
		setMainWidget(page);

		m_layout = new QGridLayout( page, 10, 3,5,5,"");
		m_layout->addRowSpacing( 0, fontMetrics().lineSpacing() );
		m_layout->addColSpacing( 0, fontMetrics().lineSpacing() );

		m_lbDocClass = new QLabel(page);
		m_lbDocClass->setText(i18n("&Document class:"));
		m_layout->addWidget(m_lbDocClass , 0, 0 );
		m_cbDocClass = new KComboBox( page );
		m_lbDocClass->setBuddy(m_cbDocClass);
		m_cbDocClass->insertItem( "article" );
		m_cbDocClass->insertItem( "report");
		m_cbDocClass->insertItem( "letter" );
		m_cbDocClass->insertItem( "book" );

		QLabel *lb= new QLabel(page);
		lb->setText(i18n("&Typeface size:"));
		m_layout->addWidget(lb , 1, 0 );
		m_cbFontSize = new KComboBox( page );
		lb->setBuddy(m_cbFontSize);
		m_cbFontSize->insertItem( "10pt" );
		m_cbFontSize->insertItem( "11pt" );
		m_cbFontSize->insertItem( "12pt" );

		lb= new QLabel(page);
		lb->setText(i18n("&Paper size:"));
		m_layout->addWidget(lb , 2, 0 );
		m_cbPaperSize = new KComboBox( FALSE, page);
		lb->setBuddy(m_cbPaperSize);

		lb= new QLabel(page);
		lb->setText(i18n("&Encoding:"));
		m_layout->addWidget(lb , 3, 0 );
		m_cbEncoding = new KComboBox( FALSE, page);
		lb->setBuddy(m_cbEncoding);

		userClassBtn= new KPushButton(page);
		userClassBtn->setMinimumSize(0,0);
		userClassBtn->setText("+");
		userClassBtn->setFixedWidth(30);
		connect(userClassBtn , SIGNAL(clicked()), SLOT(addUserClass()) );

		userPaperBtn= new KPushButton(page);
		userPaperBtn->setMinimumSize(0,0);
		userPaperBtn->setText("+");
		userPaperBtn->setFixedWidth(30);
		connect(userPaperBtn , SIGNAL(clicked()), SLOT(addUserPaper()) );

		userEncodingBtn= new KPushButton(page);
		userEncodingBtn->setMinimumSize(0,0);
		userEncodingBtn->setText("+");
		userEncodingBtn->setFixedWidth(30);
		connect(userEncodingBtn , SIGNAL(clicked()), SLOT(addUserEncoding()) );

		m_ckAMS = new QCheckBox( page);
		m_ckAMS->setFocusPolicy( QWidget::TabFocus );
		m_ckAMS->setText( i18n("AM&S packages") );
		m_ckAMS->setAutoRepeat( FALSE );
		m_ckAMS->setChecked( TRUE );

		m_ckIdx = new QCheckBox( page);
		m_ckIdx->setFocusPolicy( QWidget::TabFocus );
		m_ckIdx->setText( i18n("make&idx package") );
		m_ckIdx->setAutoRepeat( FALSE );
		m_ckIdx->setChecked( FALSE );

		m_lbOptions = new QLabel(page);
		m_lbOptions->setText(i18n("Other &options:"));
		m_layout->addMultiCellWidget(m_lbOptions,7,7,0,1,Qt::AlignLeft);
		m_bxOptions=new KListBox(page);
		m_bxOptions->setSelectionMode(QListBox::Multi);
		m_lbOptions->setBuddy(m_bxOptions);

		userOptionsBtn= new KPushButton(page);
		userOptionsBtn->setMinimumSize(0,0);
		userOptionsBtn->setText("+");
		userOptionsBtn->setFixedWidth(30);
		connect(userOptionsBtn , SIGNAL(clicked()), SLOT(addUserOptions()) );

		m_lbAuthor = new QLabel(page);
		m_lbAuthor->setText(i18n("&Author:"));
		m_layout->addWidget(m_lbAuthor , 5, 0 );
		m_leAuthor = new KLineEdit(page);
		m_layout->addWidget(m_leAuthor ,5,1 );
		m_lbAuthor->setBuddy(m_leAuthor);

		m_lbTitle = new QLabel(page);
		m_lbTitle ->setText(i18n("Tit&le:"));
		m_layout->addWidget(m_lbTitle , 6, 0 );
		m_leTitle = new KLineEdit(page);
		m_lbTitle->setBuddy(m_leTitle);

		m_layout->addWidget(m_cbDocClass , 0, 1 );
		m_layout->addWidget(userClassBtn , 0, 2,Qt::AlignLeft );
		m_layout->addWidget(m_cbFontSize , 1, 1 );
		m_layout->addWidget(m_cbPaperSize , 2, 1 );
		m_layout->addWidget(userPaperBtn , 2, 2,Qt::AlignLeft );
		m_layout->addWidget(m_cbEncoding , 3, 1 );
		m_layout->addWidget(userEncodingBtn , 3, 2,Qt::AlignLeft );
		m_layout->addWidget(m_ckAMS , 4, 0 );
		m_layout->addWidget(m_ckIdx , 4, 1 );
		m_layout->addWidget(m_leTitle , 6,1);
		m_layout->addMultiCellWidget(m_bxOptions,8,8,0,1,0);
		m_layout->addWidget(userOptionsBtn , 8, 2,Qt::AlignLeft );
		this->resize(280,340);

		readConfig();
		init();
	}
	
	QuickDocument::~QuickDocument()
	{}

	void QuickDocument::slotOk()
	{
		m_td.tagBegin = "\\documentclass[";
		m_td.tagBegin += m_cbFontSize->currentText();
		m_td.tagBegin += "," + m_cbPaperSize->currentText();
		QString opt;
		for ( uint j=0; j<= m_bxOptions->count(); j++)
		{
			if (m_bxOptions->isSelected(j)) opt +=  ","+ m_bxOptions->item(j)->text();
		}
		m_td.tagBegin+= opt +"]{";
		m_td.tagBegin+=m_cbDocClass->currentText()+"}\n";
		if (m_cbEncoding->currentText() != "NONE" )
			m_td.tagBegin += "\\usepackage["+ m_cbEncoding->currentText()+"]{inputenc}\n";

		if (m_ckAMS->isChecked())
		{
			m_td.tagBegin += "\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n";
			m_td.dy = m_td.dy+3;
		}

		if (m_ckIdx->isChecked())
		{
			m_td.tagBegin += "\\usepackage{makeidx}\n";
			m_td.dy++;
		}

		if ( m_leAuthor->text() != "")
		{
			m_td.tagBegin += "\\author{"+m_leAuthor->text()+"}\n";
			m_td.dy++;
		}

		if ( m_leTitle->text() != "")
		{
			m_td.tagBegin += "\\title{"+ m_leTitle->text()+"}\n";
			m_td.dy=m_td.dy+1;
		}
		m_td.tagBegin += "\\begin{document}\n";

		m_td.tagEnd = "\n\\end{document}";

		writeConfig();
		accept();
	}

	void QuickDocument::init()
	{
		m_cbDocClass->clear();
		m_cbDocClass->insertItem( "article" );
		m_cbDocClass->insertItem( "report");
		m_cbDocClass->insertItem( "letter" );
		m_cbDocClass->insertItem( "book" );

		m_cbPaperSize->clear();
		m_cbPaperSize->insertItem( "a4paper" );
		m_cbPaperSize->insertItem( "a5paper" );
		m_cbPaperSize->insertItem( "b5paper" );
		m_cbPaperSize->insertItem( "letterpaper" );
		m_cbPaperSize->insertItem( "legalpaper" );
		m_cbPaperSize->insertItem( "executivepaper" );

		m_cbEncoding->clear();
		m_cbEncoding->insertItem( "latin1" );
		m_cbEncoding->insertItem( "latin2" );
		m_cbEncoding->insertItem( "latin3" );
		m_cbEncoding->insertItem( "latin5" );
		m_cbEncoding->insertItem( "ascii" );
		m_cbEncoding->insertItem( "decmulti" );
		m_cbEncoding->insertItem( "cp850" );
		m_cbEncoding->insertItem( "cp852" );
		m_cbEncoding->insertItem( "cp437" );
		m_cbEncoding->insertItem( "cp437de" );
		m_cbEncoding->insertItem( "cp1252" );
		m_cbEncoding->insertItem( "cp1250" );
		m_cbEncoding->insertItem( "cp1251" );
		m_cbEncoding->insertItem( "cp865" );
		m_cbEncoding->insertItem( "koi8-r" );
		m_cbEncoding->insertItem( "applemac" );
		m_cbEncoding->insertItem( "next" );
		m_cbEncoding->insertItem( "ansinew" );
		m_cbEncoding->insertItem( "NONE" );

		m_bxOptions->clear();
		m_bxOptions->insertItem( "landscape" );
		m_bxOptions->insertItem( "draft" );
		m_bxOptions->insertItem( "final" );
		m_bxOptions->insertItem( "oneside" );
		m_bxOptions->insertItem( "twoside" );
		m_bxOptions->insertItem( "openright" );
		m_bxOptions->insertItem( "openany" );
		m_bxOptions->insertItem( "onecolumn" );
		m_bxOptions->insertItem( "twocolumn" );
		m_bxOptions->insertItem( "titlepage" );
		m_bxOptions->insertItem( "notitlepage" );
		m_bxOptions->insertItem( "openbib" );
		m_bxOptions->insertItem( "leqno" );
		m_bxOptions->insertItem( "fleqn" );

		m_cbPaperSize->insertStringList(m_otherPaperList);
		m_cbDocClass->insertStringList(m_otherClassList);
		m_cbEncoding->insertStringList(m_otherEncodingList);
		m_bxOptions->insertStringList(m_otherOptionsList);
	}
	
	void QuickDocument::add(QStringList & list)
	{
		bool ok;
		QString newoption = KLineEditDlg::getText(i18n("New"), "", &ok, this);
		if ( ok)
		{
			
			if (newoption != "") list.append(newoption);
			init();
		}
	}

	void QuickDocument::addUserClass()
	{
		add(m_otherClassList);
	}
	
	void QuickDocument::addUserPaper()
	{
		add(m_otherPaperList);
	}
	
	void QuickDocument::addUserEncoding()
	{
		add(m_otherEncodingList);
	}
	
	void QuickDocument::addUserOptions()
	{
		add(m_otherOptionsList);
	}

	void QuickDocument::readConfig()
	{
		m_config->setGroup( "Quick" );
		m_cbDocClass->setCurrentText(m_config->readEntry("Class","article"));
		m_cbFontSize->setCurrentText(m_config->readEntry("Typeface","10pt"));
		m_cbPaperSize->setCurrentText(m_config->readEntry("Papersize","a4paper"));
		m_cbEncoding->setCurrentText(m_config->readEntry("Encoding","latin1"));
		m_otherClassList = m_config->readListEntry("User Class", ':');
		m_otherPaperList=m_config->readListEntry("User Paper", ':');
		m_otherEncodingList=m_config->readListEntry("User Encoding", ':');
		m_otherOptionsList=m_config->readListEntry("User Options", ':');
		m_ckAMS->setChecked(m_config->readBoolEntry("AMS",true));
		m_ckIdx->setChecked(m_config->readBoolEntry( "MakeIndex",false));
		m_leAuthor->setText(m_config->readEntry("Author",""));
	}

	void QuickDocument::writeConfig()
	{
		m_config->setGroup( "Quick" );
		m_config->writeEntry("Author", m_leAuthor->text());
		m_config->writeEntry("Class", m_cbDocClass->currentText());
		m_config->writeEntry("Typeface", m_cbFontSize->currentText());
		m_config->writeEntry("Papersize", m_cbPaperSize->currentText());
		m_config->writeEntry("Encoding", m_cbEncoding->currentText());
		m_config->writeEntry("User Class", m_otherClassList);
		m_config->writeEntry("User Paper",m_otherPaperList);
		m_config->writeEntry("User Encoding", m_otherEncodingList);
		m_config->writeEntry("User Options",m_otherOptionsList);
		m_config->writeEntry("AMS", m_ckAMS->isChecked());
		m_config->writeEntry("MakeIndex", m_ckIdx->isChecked());
	}
}

#include "quickdocumentdialog.moc"
