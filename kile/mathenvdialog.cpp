/***************************************************************************
    date                 : Feb 07 2005
    version              : 0.11
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

#include "mathenvdialog.h"
#include "codecompletion.h"

#include <qlayout.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <klocale.h>
#include <kdebug.h>

namespace KileDialog 
{

MathEnvironmentDialog::MathEnvironmentDialog(KConfig *config, QWidget *parent) 
	: Wizard(config,parent)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	setCaption(i18n("Math environments"));
	
	QVBoxLayout *vbox = new QVBoxLayout(page,8,8);
	
	// environment groupbox
	QButtonGroup *envgroup = new QButtonGroup( i18n("Environment"),page);
	envgroup->setColumnLayout(0, Qt::Vertical );
	envgroup->layout()->setSpacing( 6 );
	envgroup->layout()->setMargin( 11 );

	m_lbEnvironment = new QLabel(i18n("Name:"),envgroup);
	m_lbStarred = new QLabel(i18n("Without numbering:"),envgroup);
	m_lbRows = new QLabel(i18n("Number of rows:"),envgroup);
	m_lbCols = new QLabel(i18n("Number of cols:"),envgroup);
	m_lbGroups = new QLabel(i18n("Alignment groups:"), envgroup);
	m_lbSpace = new QLabel(i18n("Space command\nto separate groups:"), envgroup);
	m_lbTabulator = new QLabel(i18n("Standard tabulator:"), envgroup);
	m_lbDisplaymath = new QLabel(i18n("Displaymath mode:"), envgroup);
	m_lbBullets = new QLabel(i18n("Use bullets:"), envgroup);
	
	QFrame *frame = new QFrame(envgroup);
	frame->setFrameStyle(QFrame::HLine | QFrame::Raised);
	frame->setLineWidth(2);
	
	m_coEnvironment = new QComboBox(envgroup);	
	m_cbStarred = new QCheckBox(envgroup);
	m_spRows = new QSpinBox(1,99,1,envgroup);
	m_spRows->setValue(3);
	m_spCols = new QSpinBox(1,49,1,envgroup);
	m_spCols->setValue(3);
	m_spGroups = new QSpinBox(1,9,1,envgroup);
	m_spGroups->setValue(1);
	m_edSpace = new KLineEdit("",envgroup);
	m_coTabulator = new QComboBox(envgroup);
	m_coDisplaymath = new QComboBox(envgroup);	
	m_cbBullets = new QCheckBox(envgroup);
	
	QGridLayout *envlayout = new QGridLayout( envgroup->layout() );
	envlayout->setAlignment( Qt::AlignTop );
	envlayout->addWidget( m_lbEnvironment, 0,0 ); 
	envlayout->addWidget( m_lbStarred, 1,0 );
	envlayout->addWidget( m_lbRows, 2,0 );
	envlayout->addWidget( m_lbCols, 3,0 );
	envlayout->addWidget( m_lbTabulator, 5,0 ); 
	envlayout->addWidget( m_lbDisplaymath, 6,0 ); 
	envlayout->addWidget( m_coEnvironment, 0,1 );
	envlayout->addWidget( m_cbStarred, 1,1 );
	envlayout->addWidget( m_spRows, 2,1 );
	envlayout->addWidget( m_spCols, 3,1 );
	envlayout->addWidget( m_coTabulator, 5,1 );
	envlayout->addWidget( m_coDisplaymath, 6,1 );
	envlayout->addWidget( m_lbGroups, 2,3 ); 
	envlayout->addWidget( m_lbSpace, 3,3 );
	envlayout->addWidget( m_lbBullets, 5,3 );
	envlayout->addWidget( m_spGroups, 2,4 );
	envlayout->addWidget( m_edSpace, 3,4 );
	envlayout->addWidget( m_cbBullets, 5,4 );	
	envlayout->addMultiCellWidget(frame,4,4,0,4);
	envlayout->setRowSpacing(4,30);
	envlayout->setColSpacing(2,20);
	envlayout->setColStretch(4,1);
	
	// add widgets
	vbox->addWidget( envgroup);
	vbox->addStretch(1);
	
	// install environments
	QStringList envlist;
	envlist << "align,*,g,,&="
	        << "alignat,*,g,,&="
	        << "aligned,,g,d,&="
	        << "alignedat,,g,d,&="
	        << "cases,,,d,&"
	        << "eqnarray,*,,,&=&"
	        << "flalign,*,g,,&="
	        << "gather,*,,,"
	        << "multline,*,,,"
	        << "split,,,,&="
	        << "matrix,,c,d,&"
	        << "bmatrix,,c,d,&"
	        << "pmatrix,,c,d,&"
	        << "vmatrix,,c,d,&"
	        << "Vmatrix,,c,d,&"
			  ;
	setEnvironments(envlist);
	
	// initialize dialog
	m_coDisplaymath->insertItem(QString::null);
	m_coDisplaymath->insertItem("displaymath");
	m_coDisplaymath->insertItem("\\[");
	m_coDisplaymath->insertItem("equation");
	m_coDisplaymath->insertItem("equation*");
	
	m_coEnvironment->setCurrentItem(0);
	slotEnvironmentChanged(0);
	
	// signals and slots
	connect(m_coEnvironment, SIGNAL(activated(int)), this, SLOT(slotEnvironmentChanged(int)));
	connect(m_spGroups, SIGNAL(valueChanged(int)), this, SLOT(slotSpinboxValueChanged(int)));
}

void MathEnvironmentDialog::setEnvironments(const QStringList &envlist)
{
	for ( uint i=0; i<envlist.count(); ++i ) {
		QStringList list = QStringList::split(',',envlist[i],true);
	
		// add to environment combobox
		m_coEnvironment->insertItem(list[0]);
		m_dictEnv[list[0]] = envlist[i];
	}
	
}

bool MathEnvironmentDialog::isAlignatEnv()
{
	QString env = m_coEnvironment->currentText();
	return ( env=="alignat" || env=="alignedat" );
}

////////////////////////////// determine the whole tag //////////////////////////////

void MathEnvironmentDialog::slotEnvironmentChanged(int index)
{
	kdDebug() << "environment changed: " << m_coEnvironment->text(index) << endl;
	
	// get parameter of this environment
	QStringList list = QStringList::split(',',m_dictEnv[m_coEnvironment->text(index)],true);
	m_envname = list[0];
	m_starred = ( list[1] == "*" ) ? true : false;
	m_columns = ( list[2] == "c" ) ? true : false;
	m_groups  = ( list[2] == "g" ) ? true : false;
	m_displaymath  = ( list[3] == "d" ) ? true : false;
	m_tabulator = list[4];
	
	// set starred version
	m_cbStarred->setChecked(false);
	m_lbStarred->setEnabled(m_starred);
	m_cbStarred->setEnabled(m_starred);
	
	// set column entries
	m_lbCols->setEnabled(m_columns);
	m_spCols->setEnabled(m_columns);
	
	// set group entries
	m_lbGroups->setEnabled(m_groups);
	m_spGroups->setEnabled(m_groups);
	slotSpinboxValueChanged(m_spGroups->value());
	
	// set tabulator entries
	m_coTabulator->clear(); 
	QStringList tablist;
	if ( m_tabulator == "&=&" ) 
		tablist << "&=&" << "& &" << "&<&" << "&<=&" << "&>&" << "&>=&"
	           << "&\\ne&" << "&\\approx&" << "&\\equiv&" << "&\\conq&" ;
	else if ( m_tabulator == "&=" ) 
		tablist << "&=" << "& " << "&<" << "&<=" << "&>" << "&>="
	           << "&\\ne" << "&\\approx" << "&\\equiv" << "&\\conq" ;
	else if ( ! m_tabulator.isEmpty() )
		tablist << "&";
	bool tabstate = ( tablist.count() > 0 );
	m_lbTabulator->setEnabled(tabstate);
	m_coTabulator->setEnabled(tabstate);
	if ( tabstate ) 
		m_coTabulator->insertStringList(tablist); 
	
	// set displaymathmode entries
	m_lbDisplaymath->setEnabled(m_displaymath);
	m_coDisplaymath->setEnabled(m_displaymath);
}

void MathEnvironmentDialog::slotSpinboxValueChanged(int index)
{	
	bool state = ( index>1 && m_groups && isAlignatEnv() );
	m_lbSpace->setEnabled(state);
	m_edSpace->setEnabled(state);
}

void MathEnvironmentDialog::slotOk()
{
	// environment
	QString envname = ( m_cbStarred->isChecked() ) ? m_envname + "*" : m_envname;
	
	// use bullets?
	QString bullet = ( m_cbBullets->isChecked() ) ? s_bullet : QString::null;
	
	// normal tabulator
	QString tab = m_coTabulator->currentText();
	tab.replace("<=","\\le");
	tab.replace(">=","\\ge");
	QString tabulator = bullet + " " + tab + " ";
	
	// number of rows
	int numrows = m_spRows->value();
	
	// get number of groups/columns and tabulator to separate these
	QString topgrouptabulator,grouptabulator;
	int numgroups;
	bool aligngroups;
	if ( m_spGroups->isEnabled() ) {
		aligngroups = true;
		numgroups = ( m_tabulator != "&" ) ? m_spGroups->value() : 1;
		if ( m_edSpace->isEnabled() ) {
			QString spaces;
			topgrouptabulator = "  &" + m_edSpace->text() + "  ";
			grouptabulator = "  &  " + spaces.fill(' ', m_edSpace->text().length());
		} else {
			topgrouptabulator = "  &  ";
			grouptabulator = "  &  ";
		}
	} else {
		aligngroups = false;
		numgroups = ( m_spCols->isEnabled() ) ? m_spCols->value()-1 : 0;
	}
		
	// get displaymath mode
	QString displaymathbegin = QString::null;
	QString displaymathend = QString::null;
	if ( m_coDisplaymath->isEnabled() ) {
		QString mathmode = m_coDisplaymath->currentText();
		if ( ! mathmode.isEmpty() ) {
			if ( mathmode == "\\[" ) {
				displaymathbegin = "\\[\n";
				displaymathend   = "\\]\n";
			} else {
				displaymathbegin = QString("\\begin{%1}\n").arg(mathmode);
				displaymathend   = QString("\\end{%1}\n").arg(mathmode);
			}
		}
	}
		 
	// build tag 
	m_td.tagBegin = displaymathbegin;
	
	if ( isAlignatEnv() )
		m_td.tagBegin += QString("\\begin{%1}{%2}\n").arg(envname).arg(numgroups);
	else
		m_td.tagBegin += QString("\\begin{%1}\n").arg(envname);
	
	for ( int row=0; row<numrows; ++row ) {
		for ( int col=0; col<numgroups; ++col ) {
			m_td.tagBegin += tabulator; 
			// is there more than one group or column?
			if ( aligngroups && col<numgroups-1 ) {
				if ( row == 0 ) 
					m_td.tagBegin += bullet + topgrouptabulator; 
				else
					m_td.tagBegin += bullet + grouptabulator; 
			}
		}
		// last row without CR
		if ( row < numrows-1 )
			m_td.tagBegin += bullet + " \\\\\n"; 
		else
			m_td.tagBegin += bullet; 
	}
	
	m_td.tagEnd = QString("\n\\end{%1}\n").arg(envname);
	m_td.tagEnd += displaymathend;
	
	m_td.dy = ( displaymathbegin.isEmpty() ) ? 1 : 2;
	m_td.dx = 0;

	accept();
}

}

#include "mathenvdialog.moc"
