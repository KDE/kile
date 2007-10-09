/***************************************************************************
    date                 : Dec 06 2005
    version              : 0.12
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "floatdialog.h"

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qregexp.h>

#include <klocale.h>
#include "kiledebug.h"

#include "kileedit.h"

namespace KileDialog 
{

FloatEnvironmentDialog::FloatEnvironmentDialog(KConfig *config, KileInfo *ki, QWidget *parent) 
	: Wizard(config,parent), m_ki(ki)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
		
	QGridLayout *grid = new QGridLayout(page, 6,2, 8,8);
   
	// environment groupbox
	QButtonGroup *egroup = new QButtonGroup( i18n("Environment"),page);
	egroup->setColumnLayout(0, Qt::Vertical );
	egroup->layout()->setSpacing( 6 );
	egroup->layout()->setMargin( 11 );
	QGridLayout *egrouplayout = new QGridLayout( egroup->layout() );
	egrouplayout->setAlignment( Qt::AlignTop );

	m_rbFigure = new QRadioButton(i18n("&Figure"), egroup);
	m_rbTable = new QRadioButton(i18n("T&able"), egroup);
	
	egrouplayout->addWidget( m_rbFigure, 0,0 );
	egrouplayout->addWidget( m_rbTable, 0,1 );

	// position groupbox
	QButtonGroup *pgroup = new QButtonGroup( i18n("Position"),page);
	pgroup->setColumnLayout(0, Qt::Vertical );
	pgroup->layout()->setSpacing( 6 );
	pgroup->layout()->setMargin( 11 );
	QGridLayout *pgrouplayout = new QGridLayout( pgroup->layout() );
	pgrouplayout->setAlignment( Qt::AlignTop );

	QLabel *label1 = new QLabel(i18n("Here exact:"), pgroup);
	QLabel *label2 = new QLabel(i18n("Top of page:"), pgroup);
	QLabel *label3 = new QLabel(i18n("Bottom of page:"), pgroup);
	QLabel *label4 = new QLabel(i18n("Extra page:"), pgroup);
	m_cbHere = new QCheckBox(pgroup);
	m_cbTop = new QCheckBox(pgroup);
	m_cbBottom = new QCheckBox(pgroup);
	m_cbPage = new QCheckBox(pgroup);
	
	pgrouplayout->addWidget( label1, 0,0 );
	pgrouplayout->addWidget( label2, 1,0 );
	pgrouplayout->addWidget( label3, 0,2 );
	pgrouplayout->addWidget( label4, 1,2 );
	pgrouplayout->addWidget( m_cbHere, 0,1 );
	pgrouplayout->addWidget( m_cbTop, 1,1 );
	pgrouplayout->addWidget( m_cbBottom, 0,3 );
	pgrouplayout->addWidget( m_cbPage, 1,3 );
	
	// center environment
	QLabel *label5 = new QLabel(i18n("Center:"),page);
	m_cbCenter = new QCheckBox(page);
	
	// Caption
	QLabel *label6 = new QLabel(i18n("Ca&ption:"),page);
	m_edCaption = new KLineEdit("",page);
	m_edCaption->setMinimumWidth(300);
	label6->setBuddy(m_edCaption);
	
	// Label
	QLabel *label7 = new QLabel(i18n("&Label:"),page);
	m_edLabel = new KLineEdit("",page);
	m_edLabel->setMinimumWidth(300);
	label7->setBuddy(m_edLabel);
	
	
	// add widgets
	grid->addMultiCellWidget( egroup, 0,0,0,1 );
	grid->addMultiCellWidget( pgroup, 1,1,0,1 );
	grid->addWidget(label5,2,0);
	grid->addWidget(label6,3,0);
	grid->addWidget(label7,4,0);
	grid->addWidget(m_cbCenter,2,1);
	grid->addWidget(m_edCaption,3,1);
	grid->addWidget(m_edLabel,4,1);
	
	// default values
	m_cbCenter->setChecked(true);
	m_cbHere->setChecked(true);
	m_cbTop->setChecked(true);
	m_cbPage->setChecked(true);
	m_rbFigure->setChecked(true);
	m_prefix = "fig:";
	m_edLabel->setText(m_prefix);
	slotEnvironmentClicked();
	
	grid->setRowStretch(5,1);
	setFocusProxy( m_edCaption );
	
	// signals and slots
	connect(m_rbFigure, SIGNAL(clicked()), this, SLOT(slotEnvironmentClicked()));
	connect(m_rbTable, SIGNAL(clicked()), this, SLOT(slotEnvironmentClicked()));
	
}

////////////////////////////// determine the whole tag //////////////////////////////

void FloatEnvironmentDialog::slotOk()
{
	QString envname = ( m_rbFigure->isChecked() ) ? "figure" : "table";
	QString indent = m_ki->editorExtension()->autoIndentEnvironment();
	
	QString position;
	if ( m_cbHere->isChecked() )
		position += 'h';
	if ( m_cbTop->isChecked() )
		position += 't';
	if ( m_cbBottom->isChecked() )
		position += 'b';
	if ( m_cbPage->isChecked() )
		position += 'p';
	
	m_td.tagBegin = "\\begin{" + envname + '}';
	if ( !position.isEmpty() )
		m_td.tagBegin += '[' + position + ']';
	m_td.tagBegin += '\n';
	
	int row = 1;
	if ( m_cbCenter->isChecked() ) {
		m_td.tagBegin += indent + "\\centering\n";
		row = 2;
	}
	
	m_td.tagEnd = indent + '\n';

	QString caption = m_edCaption->text();
	if ( ! caption.isEmpty() ) 
		m_td.tagEnd += indent  + "\\caption{" + caption + "}\n";

	QString label = m_edLabel->text();
	if ( !label.isEmpty() && label!=m_prefix ) 
		m_td.tagEnd += indent + "\\label{" + label + "}\n";
		
	m_td.tagEnd += "\\end{" + envname + "}\n";
	
	m_td.dy=row; 
	m_td.dx=indent.length();

	accept();
}

void FloatEnvironmentDialog::slotEnvironmentClicked()
{
	QString caption,oldprefix;
	
	if ( m_rbFigure->isChecked() ) {
		caption = i18n("Figure Environment");
		oldprefix = "^tab:";
		m_prefix = "fig:";
	} else {
		caption = i18n("Table Environment");
		oldprefix = "^fig:";
		m_prefix = "tab:";
	}
		
	setCaption(caption);
	QString s = m_edLabel->text();
	s.replace( QRegExp(oldprefix),m_prefix);
	m_edLabel->setText(s);
	
}

}

#include "floatdialog.moc"
