/**********************************************************************

	--- Qt Architect generated file ---

	File: rotationData.cpp

    Xgfe: X Windows GUI front end to Gnuplot
    Copyright (C) 1998 David Ishee

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.    

 *********************************************************************/

#include "rotationData.h"

#include <qlabel.h>
#include <qpushbt.h>
#include <qlayout.h>

rotationData::rotationData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name,TRUE )
{
  QGridLayout *gbox = new QGridLayout( this, 5,3,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_xaxisLabel;
	dlgedit_xaxisLabel = new QLabel( this, "xaxisLabel" );
	dlgedit_xaxisLabel->setText( "X Axis:" );
	dlgedit_xaxisLabel->setAlignment( 289 );
	dlgedit_xaxisLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_xaxisLabel , 0,0, Qt::AlignLeft );

	xAxis = new QSlider( this, "xAxisSlider" );
	connect( xAxis, SIGNAL(valueChanged(int)), SLOT(xAxisChanged(int)) );
	xAxis->setOrientation( QSlider::Horizontal );
	xAxis->setRange( 0, 180 );
	xAxis->setSteps( 1, 10 );
	xAxis->setValue( 0 );
	xAxis->setTracking( TRUE );
	xAxis->setTickmarks( QSlider::Below );
	xAxis->setTickInterval( 10 );
  xAxis->setMinimumWidth(200);
  gbox->addWidget(xAxis , 0,1, Qt::AlignLeft );

	xAxisLCD = new QLCDNumber( this, "xAxisLCDNumber" );
	xAxisLCD->setFrameStyle( 33 );
	xAxisLCD->setSmallDecimalPoint( FALSE );
	xAxisLCD->setNumDigits( 3 );
	xAxisLCD->setMode( QLCDNumber::DEC );
	xAxisLCD->setSegmentStyle( QLCDNumber::Flat );
  gbox->addWidget(xAxisLCD , 0,2, Qt::AlignLeft );

	QLabel* dlgedit_zAxisLabel;
	dlgedit_zAxisLabel = new QLabel( this, "zAxisLabel" );
	dlgedit_zAxisLabel->setText( "Z Axis:" );
	dlgedit_zAxisLabel->setAlignment( 289 );
	dlgedit_zAxisLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_zAxisLabel , 1,0, Qt::AlignLeft );

	zAxis = new QSlider( this, "zAxisSlider" );
	connect( zAxis, SIGNAL(valueChanged(int)), SLOT(zAxisChanged(int)) );
	zAxis->setOrientation( QSlider::Horizontal );
	zAxis->setRange( 0, 360 );
	zAxis->setSteps( 1, 10 );
	zAxis->setValue( 0 );
	zAxis->setTracking( TRUE );
	zAxis->setTickmarks( QSlider::Below );
	zAxis->setTickInterval( 10 );
  zAxis->setMinimumWidth(200);
  gbox->addWidget( zAxis, 1,1, Qt::AlignLeft );

	zAxisLCD = new QLCDNumber( this, "zAxisLCDNumber" );
	zAxisLCD->setFrameStyle( 33 );
	zAxisLCD->setSmallDecimalPoint( FALSE );
	zAxisLCD->setNumDigits( 3 );
	zAxisLCD->setMode( QLCDNumber::DEC );
	zAxisLCD->setSegmentStyle( QLCDNumber::Flat );
  gbox->addWidget(zAxisLCD , 1,2, Qt::AlignLeft );

	QLabel* dlgedit_plotScalingLabel;
	dlgedit_plotScalingLabel = new QLabel( this, "plotScalingLabel" );
	dlgedit_plotScalingLabel->setText( "Plot Scaling:" );
	dlgedit_plotScalingLabel->setAlignment( 289 );
	dlgedit_plotScalingLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_plotScalingLabel , 2,0, Qt::AlignLeft );

	plotScalingEdit = new QLineEdit( this, "plotScalingLineEdit" );
	plotScalingEdit->setText( "" );
	plotScalingEdit->setMaxLength( 32767 );
	plotScalingEdit->setEchoMode( QLineEdit::Normal );
	plotScalingEdit->setFrame( TRUE );
  gbox->addWidget(plotScalingEdit , 2,1, Qt::AlignLeft );

	QLabel* dlgedit_xAxisScalingLabel;
	dlgedit_xAxisScalingLabel = new QLabel( this, "xAxisScalingLabel" );
	dlgedit_xAxisScalingLabel->setText( "Z Axis Scaling:" );
	dlgedit_xAxisScalingLabel->setAlignment( 289 );
	dlgedit_xAxisScalingLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_xAxisScalingLabel , 3,0, Qt::AlignLeft );

	zAxisScalingEdit = new QLineEdit( this, "zAxisScalingLineEdit" );
	zAxisScalingEdit->setText( "" );
	zAxisScalingEdit->setMaxLength( 32767 );
	zAxisScalingEdit->setEchoMode( QLineEdit::Normal );
	zAxisScalingEdit->setFrame( TRUE );
  gbox->addWidget(zAxisScalingEdit , 3,1, Qt::AlignLeft );

	QPushButton* dlgedit_plotPushButton;
	dlgedit_plotPushButton = new QPushButton( this, "plotPushButton" );
	connect( dlgedit_plotPushButton, SIGNAL(clicked()), SLOT(doOK()) );
	dlgedit_plotPushButton->setText( "OK" );
	dlgedit_plotPushButton->setAutoRepeat( FALSE );
	dlgedit_plotPushButton->setAutoDefault( TRUE );
    dlgedit_plotPushButton->setDefault( TRUE );
    gbox->addWidget( dlgedit_plotPushButton, 4,0, Qt::AlignCenter );

	QPushButton* dlgedit_defaultsPushButton;
	dlgedit_defaultsPushButton = new QPushButton( this, "defaultsPushButton" );
	connect( dlgedit_defaultsPushButton, SIGNAL(clicked()), SLOT(setDefaults()) );
	dlgedit_defaultsPushButton->setText( "Defaults" );
	dlgedit_defaultsPushButton->setAutoRepeat( FALSE );
  gbox->addWidget(dlgedit_defaultsPushButton , 4,1, Qt::AlignCenter );

	QPushButton* dlgedit_closePushButton;
	dlgedit_closePushButton = new QPushButton( this, "closePushButton" );
	connect( dlgedit_closePushButton, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_closePushButton->setText( "Cancel" );
	dlgedit_closePushButton->setAutoRepeat( FALSE );
  gbox->addWidget( dlgedit_closePushButton, 4,2, Qt::AlignCenter );

	resize( 380,120 );
}


rotationData::~rotationData()
{
}
void rotationData::xAxisChanged(int)
{
}
void rotationData::zAxisChanged(int)
{
}
void rotationData::doOK()
{
}
void rotationData::setDefaults()
{
}

#include "rotationData.moc"
