/***************************************************************************
    begin                : dim jui 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
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

#include "tabbingdialog.h"
#include "kileedit.h"

#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <kpushbutton.h>
#include <klocale.h>


namespace KileDialog
{
	QuickTabbing::QuickTabbing(KConfig *config, KileInfo *ki, QWidget *parent, const char *name, const QString &caption ) : Wizard(config, parent,name, caption), m_ki(ki)
	{
		QWidget *page = new QWidget(this);
		setMainWidget(page);

		QGridLayout *gbox = new QGridLayout( page, 4, 2,5,5,"");
		gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
		
		QLabel *lb = new QLabel(page);
		lb->setText(i18n("Num of columns:"));
		gbox->addWidget(lb, 0, 0);
		m_spCols = new QSpinBox(page);
		lb->setBuddy(m_spCols);
		m_spCols ->setValue(2);
		m_spCols ->setRange(2,99);
		gbox->addWidget( m_spCols , 0, 1 );
		
		lb = new QLabel(page);
		lb->setText(i18n("Num of &rows:"));
		gbox->addWidget(lb, 1, 0 );
		m_spRows = new QSpinBox(page);
		lb->setBuddy(m_spRows);
		m_spRows->setValue(1);
		m_spRows->setRange(1,99);
		gbox->addWidget( m_spRows, 1, 1 );
		
		lb= new QLabel(page);
		lb->setText(i18n("&Spacing:"));
		gbox->addWidget(lb, 2, 0 );
		m_leSpacing = new KLineEdit(page);
		m_leSpacing->setFixedWidth(80);
		lb->setBuddy(m_leSpacing);
		gbox->addWidget(m_leSpacing, 2, 1 );

		resize(130,120);
	}
	
	QuickTabbing::~QuickTabbing()
	{}

	void QuickTabbing::slotOk()
	{
		int x = m_spCols->value();
		int y = m_spRows->value();
		QString s= m_leSpacing->text();
		QString indent = m_ki->editorExtension()->autoIndentEnvironment();
		
		m_td.tagBegin = "\\begin{tabbing}\n";
		m_td.tagBegin += indent;

		for ( int j=1; j<x ; ++j) m_td.tagBegin += "\\hspace{"+s+"}\\=";

		m_td.tagBegin += "\\kill\n";

		for ( int i=0;i<y-1;++i)
		{
			m_td.tagBegin += indent;
			for ( int j=1;j<x;++j) m_td.tagBegin +=" \\> ";
			m_td.tagBegin += "\\\\ \n";
		}

		m_td.tagBegin += indent;
		for ( int j=1;j<x;++j)  m_td.tagBegin += " \\> ";

		m_td.tagEnd = "\n\\end{tabbing}";
		m_td.dy=1;
		m_td.dx=indent.length();

		accept();
	}
}

#include "tabbingdialog.moc"
