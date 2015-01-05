/**************************************************************************
*   Copyright (C) 2007-2012 by Michel Ludwig (michel.ludwig@kdemail.net)  *
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

#include <config.h>

#include <KFileDialog>
#include <KUrlCompletion>
#include <QFileDialog>

KileWidgetGeneralConfig::KileWidgetGeneralConfig(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	m_defaultProjectLocationButton->setIcon(QIcon::fromTheme("folder-open"));

	connect(m_defaultProjectLocationButton, SIGNAL(clicked()),
	        this, SLOT(selectDefaultProjectLocation()));

	KUrlCompletion *dirCompletion = new KUrlCompletion();
	dirCompletion->setMode(KUrlCompletion::DirCompletion);
	kcfg_DefaultProjectLocation->setCompletionObject(dirCompletion);
	kcfg_DefaultProjectLocation->setAutoDeleteCompletionObject(true);

#ifndef HAVE_VIEWERINTERFACE_H
	documentViewerGroupBox->setEnabled(false);
#endif
}

KileWidgetGeneralConfig::~KileWidgetGeneralConfig()
{
}

void KileWidgetGeneralConfig::selectDefaultProjectLocation()
{
	QString newDefaultLocation = QFileDialog::getExistingDirectory(this, QString(), kcfg_DefaultProjectLocation->text());
	if (!newDefaultLocation.isEmpty()) {
		kcfg_DefaultProjectLocation->setText(newDefaultLocation);
	}
}

#include "generalconfigwidget.moc"
