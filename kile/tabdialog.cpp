/***************************************************************************
                          tabdialog.cpp  -  description
                             -------------------
    begin                : Mon Apr 30 2001
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

#include "tabdialog.h"

#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtable.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kcombobox.h>

namespace KileDialog
{
	QuickTabular::QuickTabular(KConfig *config, QWidget *parent, const char *name, const QString &caption) :
		Wizard(config, parent,name, caption)
	{
		QWidget *page = new QWidget(this);
		setMainWidget(page);

		QGridLayout *gbox = new QGridLayout( page, 7, 2,5,5,"");
		gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
		gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
		
		m_table = new QTable(page);
		m_table->setNumRows( 2 );
		m_table->setNumCols( 2 );
		gbox->addMultiCellWidget(m_table,0,0,0,1,0);
		
		m_spRows = new QSpinBox(page);
		m_spRows->setValue(2);
		m_spRows->setRange(1,99);
		gbox->addWidget(m_spRows , 1, 1 );
		connect( m_spRows, SIGNAL(valueChanged(int)),m_table, SLOT(setNumRows(int)));
		
		m_spCols = new QSpinBox(page);
		m_spCols->setValue(2);
		m_spCols->setRange(1,99);
		connect( m_spCols, SIGNAL(valueChanged(int)),m_table, SLOT(setNumCols(int)));
		gbox->addWidget(m_spCols, 2, 1 );
		
		QLabel *lb = new QLabel(page);
		lb->setText(i18n("Num of rows:"));
		gbox->addWidget(lb, 1, 0 );
		
		lb = new QLabel(page);
		lb->setText(i18n("Num of columns:"));
		gbox->addWidget(lb, 2, 0 );
		
		lb = new QLabel(page);
		lb->setText(i18n("Columns alignment:"));
		gbox->addWidget(lb, 3, 0 );
		
		m_cbAlign = new KComboBox( page );
		m_cbAlign->insertItem(i18n( "Center") );
		m_cbAlign->insertItem(i18n( "Left" ));
		m_cbAlign->insertItem(i18n( "Right" ));
		m_cbAlign->insertItem( "p{}");
		gbox->addWidget(m_cbAlign , 3, 1 );
		
		lb = new QLabel(page);
		lb->setText(i18n("Vertical separator:"));
		gbox->addWidget(lb , 4, 0 );
		
		m_cbSeparator = new KComboBox( page );
		m_cbSeparator->insertItem("|");
		m_cbSeparator->insertItem("||");
		m_cbSeparator->insertItem("none");
		m_cbSeparator->insertItem( "@{text}" );
		gbox->addWidget(m_cbSeparator , 4, 1 );
		
		m_ckHSeparator = new QCheckBox( page);
		m_ckHSeparator->setFocusPolicy( QWidget::TabFocus );
		m_ckHSeparator->setText( i18n("Horizontal separator") );
		m_ckHSeparator->setAutoRepeat( false);
		m_ckHSeparator->setChecked( true );
		gbox->addMultiCellWidget(m_ckHSeparator,5,5,0,1,0);

		page->resize(460,350);
	}
	
	QuickTabular::~QuickTabular()
	{
	}

	void QuickTabular::slotOk()
	{
		QString hs = m_ckHSeparator->isChecked() ? " \\hline \n" : "\n";
		QString vs;
		if  ( m_cbSeparator->currentItem () == 0) vs = "|";
		if  ( m_cbSeparator->currentItem () == 1) vs = "||";
		if  ( m_cbSeparator->currentItem () == 2) vs = "";
		if  ( m_cbSeparator->currentItem () == 3) vs = "@{}";

		int y = m_spRows->value();
		int x = m_spCols->value();

		QString al = m_cbAlign->currentText();
		if  ( m_cbAlign->currentItem() == 0 ) al = "c"+vs;
		if  ( m_cbAlign->currentItem() == 1 ) al = "l" + vs;
		if  ( m_cbAlign->currentItem() == 2 ) al = "r" + vs;
		if  ( m_cbAlign->currentItem() == 3 ) al = "p{}" + vs;

		m_td.tagBegin = "\\begin{tabular}{" + vs;
		for ( int j=0; j<x; j++) m_td.tagBegin += al;
		m_td.tagBegin += "}\n";

		for ( int i=0; i < y; i++) 
		{
			for ( int j = 0; j<x-1; j++)
			{
				m_td.tagBegin += m_table->text(i,j)+ " & ";
				m_td.tagBegin += m_table->text(i,x-1)+ " \\\\";
			}
			m_td.tagBegin += hs;
		}

		m_td.tagBegin += "\\end{tabular} ";
		m_td.dy=1; 
		m_td.dx=0;

		accept();
	}
}

#include "tabdialog.moc"

