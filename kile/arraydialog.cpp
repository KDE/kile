/***************************************************************************
                          arraydialog.cpp  -  description
                             -------------------
    begin                : ven sep 27 2002
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

#include "arraydialog.h"

#include <qlayout.h>
#include <qspinbox.h>
#include <qtable.h>
#include <qlabel.h>

#include <kcombobox.h>
#include <klocale.h>

namespace KileDialog
{
	QuickArray::QuickArray(KConfig *config, QWidget *parent, const char *name, const QString &caption) :
		Wizard(config, parent,name, caption)
	{
		QWidget *page = new QWidget(this);
		setMainWidget(page);

		QGridLayout *gbox = new QGridLayout( page, 6, 2,5,5,"");
		gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
		gbox->addColSpacing( 0, fontMetrics().lineSpacing() );

		m_table = new QTable(page);
		m_table->setNumRows( 2 );
		m_table->setNumCols( 2 );
		gbox->addMultiCellWidget(m_table,0,0,0,1,0);

		QLabel *lb = new QLabel(page);
		lb->setText(i18n("Number of &rows:"));
		gbox->addWidget(lb , 1, 0 );
		m_spRows= new QSpinBox(page);
		lb->setBuddy(m_spRows);
		m_spRows->setValue(2);
		m_spRows->setRange(1,99);
		connect( m_spRows, SIGNAL(valueChanged(int)), m_table , SLOT(setNumRows(int)));
		gbox->addWidget(m_spRows , 1, 1 );

		lb = new QLabel(page);
		lb->setText(i18n("Number of co&lumns:"));
		gbox->addWidget(lb , 2, 0 );
		m_spCols = new QSpinBox(page);
		lb->setBuddy(m_spCols);
		m_spCols->setValue(2);
		m_spCols->setRange(1,99);
		connect( m_spCols, SIGNAL(valueChanged(int)), m_table, SLOT(setNumCols(int)));
		gbox->addWidget(m_spCols , 2, 1 );

		lb = new QLabel(page);
		lb->setText(i18n("Columns &alignment:"));
		gbox->addWidget(lb , 3, 0 );
		m_cbAlign = new KComboBox(page);
		lb->setBuddy(m_cbAlign);
		m_cbAlign->insertItem(i18n( "Center") );
		m_cbAlign->insertItem(i18n( "Left" ));
		m_cbAlign->insertItem(i18n( "Right" ));
		gbox->addWidget(m_cbAlign , 3, 1 );

		lb = new QLabel(page);
		lb->setText(i18n("&Environment:"));
		gbox->addWidget(lb, 4, 0 );
		m_cbEnv = new KComboBox(page);
		lb->setBuddy(m_cbEnv);
		m_cbEnv->insertItem("array");
		m_cbEnv->insertItem("matrix");
		m_cbEnv->insertItem("pmatrix");
		m_cbEnv->insertItem("bmatrix");
		m_cbEnv->insertItem("vmatrix");
		m_cbEnv->insertItem("Vmatrix");
		gbox->addWidget(m_cbEnv , 4, 1 );

		page->resize(460,320);
	}
	
	QuickArray::~QuickArray()
	{}

	void QuickArray::slotOk()
	{
		int y = m_spRows->value();
		int x = m_spCols->value();
		QString env = m_cbEnv->currentText();
		QString al;
		m_td.tagBegin =  "\\begin{"+env+"}";

		if (env == "array")
		{
			m_td.tagBegin += "{";
			if  ( m_cbAlign->currentItem () ==0 ) al = "c";
			if  ( m_cbAlign->currentItem () == 1) al = "l";
			if  ( m_cbAlign->currentItem () == 2) al = "r";
			for ( int j=0; j<x; j++) m_td.tagBegin +=al;
			m_td.tagBegin+="}";
		}

		m_td.tagBegin += "\n";
		for ( int i=0;i<y-1;i++) 
		{
			for ( int j=0;j<x-1;j++)
				m_td.tagBegin += m_table->text(i,j)+ " & ";

			m_td.tagBegin += m_table->text(i,x-1)+ " \\\\ \n";
		}

		for ( int j=0;j<x-1;j++)
			m_td.tagBegin += m_table->text(y-1,j)+ " & ";

		m_td.tagBegin += m_table->text(y-1,x-1);
		m_td.tagEnd = "\n\\end{"+env+"}";
 
		m_td.dx = 0;
		m_td.dy = 1;

		accept();
	}
}


#include "arraydialog.moc"
