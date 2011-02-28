/**************************************************************************
*   Copyright (C) 2007-2011 by Michel Ludwig (michel.ludwig@kdemail.net)  *
*                 2011 by Felix Mauch (felix_mauch@web.de)                *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widgets/generalconfigwidget.h"

#include <KFileDialog>

KileWidgetGeneralConfig::KileWidgetGeneralConfig(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	m_defaultProjectLocationButton->setIcon(KIcon("folder-open"));

	connect(m_defaultProjectLocationButton, SIGNAL(clicked()),
	        this, SLOT(selectDefaultProjectLocation()));
}

KileWidgetGeneralConfig::~KileWidgetGeneralConfig()
{
}

void KileWidgetGeneralConfig::selectDefaultProjectLocation()
{
	QString newDefaultLocation = KFileDialog::getExistingDirectory(kcfg_DefaultProjectLocation->text(), this);
	if (!newDefaultLocation.isEmpty()) {
		kcfg_DefaultProjectLocation->setText(newDefaultLocation);
	}
}

#include "generalconfigwidget.moc"
