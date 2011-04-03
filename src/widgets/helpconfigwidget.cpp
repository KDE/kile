/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
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

#include "widgets/helpconfigwidget.h"

#include <KFileDialog>
#include <KUrlCompletion>

KileWidgetHelpConfig::KileWidgetHelpConfig(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	m_helpLocationButton->setIcon(KIcon("folder-open"));

	connect(m_pbConfigure, SIGNAL(clicked()), this, SLOT(slotConfigure()));
	connect(m_helpLocationButton, SIGNAL(clicked()),
	        this, SLOT(selectHelpLocation()));

	KUrlCompletion *dirCompletion = new KUrlCompletion();
	dirCompletion->setMode(KUrlCompletion::DirCompletion);
	kcfg_location->setCompletionObject(dirCompletion);
	kcfg_location->setAutoDeleteCompletionObject(true);
}

KileWidgetHelpConfig::~KileWidgetHelpConfig()
{
}

void KileWidgetHelpConfig::slotConfigure()
{
	m_help->userHelpDialog();
}

void KileWidgetHelpConfig::setHelp(KileHelp::Help *help)
{
	m_help = help;
}

void KileWidgetHelpConfig::selectHelpLocation()
{
	QString newLocation = KFileDialog::getExistingDirectory(kcfg_location->text(), this);
	if (!newLocation.isEmpty()) {
		kcfg_location->setText(newLocation);
	}
}

#include "helpconfigwidget.moc"
