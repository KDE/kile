/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ***********************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <KFileDialog>
#include <KMessageBox>

#include "widgets/usermenuconfigwidget.h"

#include "kileconfig.h"
#include "kiledebug.h"

KileWidgetUsermenuConfig::KileWidgetUsermenuConfig(KileMenu::UserMenu *usermenu, QWidget *parent)
   : QWidget(parent), m_usermenu(usermenu)
{
	setupUi(this);
	setXmlFile(m_usermenu->xmlFile());

	if(KileConfig::menuLocation() == KileMenu::UserMenu::StandAloneLocation) {
		m_rbStandAloneMenuLocation->setChecked(true);
	}
	else {
		m_rbLaTeXMenuLocation->setChecked(true);
	}

	connect(m_pbInstall, SIGNAL(clicked()), this, SLOT(slotInstallClicked()));
	connect(m_pbRemove,  SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));

}

KileWidgetUsermenuConfig::~KileWidgetUsermenuConfig()
{
}

void KileWidgetUsermenuConfig::writeConfig()
{
	const int location = (m_rbStandAloneMenuLocation->isChecked())
	                     ? KileMenu::UserMenu::StandAloneLocation : KileMenu::UserMenu::LaTeXMenuLocation;
	if(KileConfig::menuLocation() != location) {
		KILE_DEBUG() << "menu position changed";
		KileConfig::setMenuLocation(location);
		m_usermenu->changeMenuLocation(location);
	}
}

void KileWidgetUsermenuConfig::slotInstallClicked()
{
	KILE_DEBUG() << "install clicked";

	QString directory = KileMenu::UserMenu::selectUserMenuDir();
	QString filter = i18n("*.xml|Latex Menu Files");

	QString xmlfile = KFileDialog::getOpenFileName(directory, filter, this, i18n("Select Menu File"));
	if(xmlfile.isEmpty()) {
		return;
	}

	if(QFile::exists(xmlfile)) {
		m_usermenu->installXmlFile(xmlfile);
		setXmlFile(xmlfile);
	}
	else {
		KMessageBox::error(this, i18n("File '%1' does not exist.", xmlfile));
	}
}

void KileWidgetUsermenuConfig::slotRemoveClicked()
{
	KILE_DEBUG() << "remove clicked";

	m_usermenu->removeXmlFile();
	setXmlFile(QString());
}

void KileWidgetUsermenuConfig::setXmlFile(const QString &file)
{
	if(file.isEmpty()) {
		m_usermenuFile->setText(i18n("no file installed"));
		m_pbRemove->setEnabled(false);
	}
	else {
		m_usermenuFile->setText(file);
		m_pbRemove->setEnabled(true);
	}
}


#include "usermenuconfigwidget.moc"
