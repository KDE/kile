/***************************************************************************
    date                 : Febr 18 2005
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

#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qmap.h>

#include "previewconfigwidget.h"
#include "kileconfig.h"

KileWidgetPreviewConfig::KileWidgetPreviewConfig(KConfig *config, KileTool::QuickPreview *preview, QWidget *parent, const char *name )
	: QWidget(parent,name),
	  m_config(config),
	  m_preview(preview)
{
	// Layout
	QVBoxLayout *vbox = new QVBoxLayout(this, 5,5 );

	QGroupBox *groupbox = new QGroupBox( i18n("Quick Preview"), this, "groupbox" );
	groupbox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, groupbox->sizePolicy().hasHeightForWidth() ) );
	groupbox->setColumnLayout(0, Qt::Vertical ); 
	groupbox->layout()->setSpacing( 6 ); 
	groupbox->layout()->setMargin( 11 );
	QGridLayout *groupboxLayout = new QGridLayout( groupbox->layout() );
	groupboxLayout->setAlignment( Qt::AlignTop );
   
	QLabel *label = new QLabel( i18n("Select a configuration:"), groupbox, "label");
	m_combobox = new KComboBox(false,groupbox,"combobox" );

	groupboxLayout->addWidget(label,0,0);
	groupboxLayout->addWidget(m_combobox,0,1);
	
	vbox->addWidget(groupbox);
	vbox->addStretch();
}

//////////////////// read/write configuration ////////////////////

void KileWidgetPreviewConfig::readConfig(void)
{
	// get all possible tasks for QuickPreview
	QStringList tasklist;
	m_preview->getTaskList(tasklist);
	
	// split them into group and combobox entry
	m_combobox->clear();
	for ( uint i=0; i<tasklist.count(); ++i ) {
		QStringList list = QStringList::split("=",tasklist[i]);
		if ( m_config->hasGroup( list[0] ) ) {
			m_combobox->insertItem( list[1] );
		}	
	}
	
	// set current task
	m_combobox->setCurrentText( KileConfig::previewTask() );
}

void KileWidgetPreviewConfig::writeConfig(void)
{
	KileConfig::setPreviewTask( m_combobox->currentText() );
}


#include "previewconfigwidget.moc"
