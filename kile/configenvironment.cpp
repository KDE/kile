/***************************************************************************
    date                 : Feb 09 2004
    version              : 0.10.0
    copyright            : (C) 2004 by Holger Danielsson
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



#include <qlabel.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qregexp.h>

#include <kdialog.h>
#include <klocale.h>
#include <klistbox.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include "configenvironment.h"
#include "kileconfig.h"

ConfigEnvironment::ConfigEnvironment(QWidget *parent, const char *name )
   : QWidget(parent,name)
{
	QVBoxLayout *vbox = new QVBoxLayout(this, 5,KDialog::spacingHint() );
	
	// Groupbox with entries            
	QButtonGroup *envtypes= new QButtonGroup(3,Qt::Horizontal,this);
	envtypes->setTitle(i18n("Environment Type"));
	envtypes->setExclusive(true);
	rb_listenv = new QRadioButton(i18n("List environment"),envtypes);
	rb_tabularenv = new QRadioButton(i18n("Tabular environment"),envtypes);
	rb_mathenv = new QRadioButton(i18n("Math environment"),envtypes);
	
	// Label
	QLabel *label = new QLabel(i18n("Known list, math and tabular environments: "), this);
	
	// Listbox
	QWidget *listboxwidget = new QWidget(this);
	QGridLayout *grid = new QGridLayout( listboxwidget, 3,3, 5,5, "");
	listbox = new KListBox(listboxwidget);
	listbox->setSelectionMode(QListBox::Single);
	pb_add = new KPushButton(i18n("Add"), listboxwidget);
	pb_remove = new KPushButton(i18n("Remove"), listboxwidget);
	grid->addMultiCellWidget(listbox,0,2,0,0);
	grid->addColSpacing(1,30);
	grid->addWidget(pb_add,0,2);
	grid->addWidget(pb_remove,1,2);
	grid->setColStretch(0,2);
	grid->setColStretch(2,1);
	
	vbox->addWidget(envtypes);
	vbox->addSpacing(15);
	vbox->addWidget(label);
	vbox->addSpacing(5);
	vbox->addWidget(listboxwidget);
	vbox->addStretch();
	
	// set default latex environments, which could not be removed
	envlist << "description" << "enumerate" << "itemize";
	setEnvironments(envlist,m_dictenvlatex);
	setEnvironments(envlist,m_dictenvlist);
	
	envmath << "align"  << "alignat" << "aligned"
		<< "bmatrix" 
		<< "eqnarray" << "eqnarray*" 
		<< "gather" << "gathered" 
		<< "matrix" << "multline"
		<< "pmatrix" 
		<< "split"
		<< "vmatrix" << "Vmatrix"
		<< "xalignat" << "xxalignat";           
	setEnvironments(envmath,m_dictenvlatex);
	setEnvironments(envmath,m_dictenvmath);
	
	envtab << "array" << "longtable" << "supertabular" << "supertabular*"
		<< "tabbing" << "tabular" << "tabular*" << "tabularx";
	setEnvironments(envtab,m_dictenvlatex);
	setEnvironments(envtab,m_dictenvtab);
	
	// set connections
	connect(rb_listenv,SIGNAL(clicked()),this,SLOT(clickedEnvtype()));
	connect(rb_mathenv,SIGNAL(clicked()),this,SLOT(clickedEnvtype()));
	connect(rb_tabularenv,SIGNAL(clicked()),this,SLOT(clickedEnvtype()));
	connect(listbox,SIGNAL(highlighted(int)),this,SLOT(highlightedListbox(int)));
	connect(pb_add,SIGNAL(clicked()),this,SLOT(clickedAdd()));
	connect(pb_remove,SIGNAL(clicked()),this,SLOT(clickedRemove()));
}

//////////////////// helper ////////////////////

QMap<QString,bool> *ConfigEnvironment::getDictionary()
{
	if ( rb_listenv->isOn() )
		return &m_dictenvlist;
	else if ( rb_mathenv->isOn() )
		return &m_dictenvmath;
	else
		return &m_dictenvtab;
}

void ConfigEnvironment::fillListbox(const QMap<QString,bool> *map)
{
	QMapConstIterator<QString,bool> it;
	
	listbox->clear();
	for ( it=map->begin(); it!=map->end(); ++it ) 
	{
		listbox->insertItem( it.key() );
	}
	listbox->sort();
	pb_remove->setEnabled(false);
}

void ConfigEnvironment::setEnvironments(const QStringList &list, QMap<QString,bool> &map)
{
	for (uint i=0; i<list.count(); ++i)
	map[list[i]] = true;
}

QStringList ConfigEnvironment::getEnvironments(const QMap<QString,bool> &map)
{
	QMapConstIterator<QString,bool> it;
	QStringList list;
	for ( it=map.begin(); it!=map.end(); ++it ) {
	if ( !m_dictenvlatex.contains(it.key()) ) {
		list << it.key();
	}     
	}
	return list;
}

//////////////////// slots ////////////////////

void ConfigEnvironment::clickedEnvtype()
{
	fillListbox( getDictionary() );
}

void ConfigEnvironment::highlightedListbox(int index)
{
	if ( index >= 0 ) 
	pb_remove->setEnabled( ! m_dictenvlatex.contains(listbox->text(index)) );
	else
	pb_remove->setEnabled(false);
}

void ConfigEnvironment::clickedAdd()
{
	bool ok;
	QString envname = KInputDialog::getText(i18n("New Environment"), i18n("Please enter the name of the new environment:"), "", &ok, this).stripWhiteSpace();
	if (ok && (!envname.isNull()))
	{
		QMap<QString,bool> *map = getDictionary();
		if ( !envname.isEmpty() ) 
		{
			QRegExp reg("[a-zA-Z]+\\*?");
			if ( reg.search(envname)==0 && reg.cap(0)==envname ) 
			{
				if ( ! map->contains(envname) ) 
				{      
					listbox->insertItem(envname);
					listbox->sort();
					(*map)[envname] = true;
				}
				else
					KMessageBox::error(this, i18n("An environment by the name '%1' already exists.").arg(envname));
				} else 
					KMessageBox::error(this, i18n("An environment by the name '%1' has illegal characters.").arg(envname));            
			}
	}
}

void ConfigEnvironment::clickedRemove()
{
	int index = listbox->currentItem();
	if (index != -1 ) 
	{
	QString text = listbox->currentText();
	if ( !text.isNull() && !m_dictenvlatex.contains(text) ) 
	{
		QMap<QString,bool> *map = getDictionary();
		map->remove(text);
		listbox->removeItem(index);
	}
	}
}

//////////////////// read/write configuration ////////////////////

void ConfigEnvironment::readConfig(void)
{
	// config section
	setEnvironments(KileConfig::envList(), m_dictenvlist);
	setEnvironments(KileConfig::envMath(), m_dictenvmath);
	setEnvironments(KileConfig::envTabular(), m_dictenvtab);
	
	rb_listenv->setChecked(true);
	fillListbox(&m_dictenvlist);
	listbox->setMinimumHeight(10*listbox->itemHeight(0)+3);
   
}

void ConfigEnvironment::writeConfig(void)
{
	// config section
	KileConfig::setEnvList(getEnvironments(m_dictenvlist));
	KileConfig::setEnvMath(getEnvironments(m_dictenvmath));
	KileConfig::setEnvTabular(getEnvironments(m_dictenvtab));
}

#include "configenvironment.moc"
