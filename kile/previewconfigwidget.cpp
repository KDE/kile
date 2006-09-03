/***************************************************************************
    date                 : Aug 26 2006
    version              : 0.21
    copyright            : (C) 2005-2006 by Holger Danielsson
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
#include <qvalidator.h>

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
	
	QGroupBox *gbResolution = new QGroupBox( i18n("QuickPreview with dvipng"), this, "gbresolution" );
	gbResolution->setColumnLayout(0, Qt::Vertical );
	gbResolution->layout()->setSpacing( 6 );
	gbResolution->layout()->setMargin( 11 );
	QGridLayout *resLayout = new QGridLayout( gbResolution->layout() );
	resLayout->setAlignment( Qt::AlignTop );

	QLabel *resLabel = new QLabel( i18n("&Resolution:"), gbResolution );
	m_leDvipngResolution = new KLineEdit( gbResolution, "DvipngResolution" );
	QLabel *resDpi = new QLabel( i18n("dpi"), gbResolution );
	QLabel *resAllowed = new QLabel( i18n("(allowed values: 30-1000 dpi)"), gbResolution );
	
	// set validator
	QValidator* validator = new QIntValidator(30,1000,this);
	m_leDvipngResolution->setValidator(validator);
	resLabel->setBuddy(m_leDvipngResolution);

	QLabel *labelDvipng = new QLabel(i18n("dvipng:"), gbResolution);

	resLayout->addWidget(resLabel,0,0);
	resLayout->addWidget(m_leDvipngResolution,0,2);
	resLayout->addWidget(resDpi,0,4,Qt::AlignLeft);
	resLayout->addWidget(resAllowed,0,6,Qt::AlignLeft);
	resLayout->addWidget(labelDvipng,1,0);
	resLayout->setColSpacing(1,8);
	resLayout->setColSpacing(3,8);
	resLayout->setColSpacing(5,24);
	resLayout->setColStretch(6,1);

	bool useDvipng = KileConfig::dvipng();
	if ( useDvipng )
	{
		QLabel *dvipng1 =  new QLabel( i18n("installed"), gbResolution);	
		resLayout->addWidget(dvipng1,1,2);
	}
	else
	{
		QLabel *dvipng2 =  new QLabel( i18n("not installed"), gbResolution);	
		QLabel *dvipng3 =  new QLabel( i18n("(You have to install 'dvipng' to use this kind of preview)"), gbResolution );
		resLayout->addWidget(dvipng2,1,2);
		resLayout->addMultiCellWidget(dvipng3,2,2,2,6,Qt::AlignLeft);
		m_leDvipngResolution->setEnabled(false);
	}

	vbox->addWidget(groupbox);
	vbox->addWidget(gbResolution);
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
	for ( uint i=0; i<tasklist.count(); ++i ) 
	{
		QStringList list = QStringList::split("=",tasklist[i]);
		if ( m_config->hasGroup( list[0] ) ) 
		{
			m_combobox->insertItem( list[1] );
		}	
	}
	
	// set current task
	m_combobox->setCurrentText( KileConfig::previewTask() );

	// dvipng resolution
	m_leDvipngResolution->setText( KileConfig::dvipngResolution() );

}

void KileWidgetPreviewConfig::writeConfig(void)
{
	KileConfig::setPreviewTask( m_combobox->currentText() );

	bool ok;
	QString resolution = m_leDvipngResolution->text();
	int dpi = resolution.toInt(&ok);
	if ( ok )
	{
		if ( dpi < 30 )
			resolution = "30";
		else if ( dpi > 1000 )
			resolution = "1000";
		KileConfig::setDvipngResolution( resolution );
	}
}


#include "previewconfigwidget.moc"
