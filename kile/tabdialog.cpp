/***************************************************************************
                          tabdialog.cpp  -  description
                             -------------------
    begin                : Mon Apr 30 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal, 2003 Jeroen Wijnhout
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

// dani 17.09.2004: don't override existing rows or columns,  
//                  when changing the size of the table

#include "tabdialog.h"
#include "kiledocumentinfo.h"

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
		//connect( m_spRows, SIGNAL(valueChanged(int)),m_table, SLOT(setNumRows(int)));
		connect( m_spRows, SIGNAL(valueChanged(int)), this , SLOT(slotRowValueChanged(int)));
		
		m_spCols = new QSpinBox(page);
		m_spCols->setValue(2);
		m_spCols->setRange(1,99);
		//connect( m_spCols, SIGNAL(valueChanged(int)),m_table, SLOT(setNumCols(int)));
		connect( m_spCols, SIGNAL(valueChanged(int)), this, SLOT(slotColValueChanged(int)));
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
		
		// remember current values
		m_rows = m_spRows->value();
		m_cols = m_spCols->value();
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
		for ( int j=0; j<x; ++j) m_td.tagBegin += al;
		m_td.tagBegin += "}\n";

		for ( int i=0; i < y; ++i) 
		{
			for ( int j = 0; j<x-1; ++j)
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
	
	void QuickTabular::slotRowValueChanged(int value)
	{
		if ( value < m_rows ) {         // problems may only happen when decreasing
			int testvalue = value;
			value = m_rows;
			for ( int row=m_rows; row>=testvalue; --row ) {
				if ( isTableRowEmpty(row) )
					value = row;
				else
					break;
			}
			m_spRows->setValue(value);      // perhaps corrected
		}
		
		m_rows = value;
		m_table->setNumRows(value);
		
	}
	
	void QuickTabular::slotColValueChanged(int value)
	{
		if ( value < m_cols ) {            // problems may only happen when decreasing
			int testvalue = value;
			value = m_cols;
			for ( int col=m_cols; col>=testvalue; --col ) {
				if ( isTableColEmpty(col) )
					value = col;
				else
					break;
			}
			m_spCols->setValue(value);      // perhaps corrected
		}
		
		m_cols = value;
		m_table->setNumCols(value);
		
	}
	
	bool QuickTabular::isTableRowEmpty(int row)
	{
		for ( int col=0; col<m_cols; ++col ) {
			if ( ! m_table->text(row,col).stripWhiteSpace().isEmpty() )
				return false;
		}
		return true;
	}
	
	bool QuickTabular::isTableColEmpty(int col)
	{
		for ( int row=0; row<m_rows; ++row ) {
			if ( ! m_table->text(row,col).stripWhiteSpace().isEmpty() )
				return false;
		}
		return true;
	}
}

#include "tabdialog.moc"

