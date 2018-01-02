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

#include <KMessageBox>
#include <QFileDialog>

#include "widgets/usermenuconfigwidget.h"

#include "kileconfig.h"
#include "kiledebug.h"

KileWidgetUsermenuConfig::KileWidgetUsermenuConfig(KileMenu::UserMenu *usermenu, QWidget *parent)
    : QWidget(parent),
      m_usermenu(usermenu)
{
    Q_ASSERT(m_usermenu);
    setupUi(this);
    setXmlFile(m_usermenu->xmlFile());

    if(KileConfig::userMenuLocation() == KileMenu::UserMenu::StandAloneLocation) {
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
    if(KileConfig::userMenuLocation() != location) {
        KILE_DEBUG_MAIN << "menu position changed";
        KileConfig::setUserMenuLocation(location);
        m_usermenu->updateGUI();
    }
}

void KileWidgetUsermenuConfig::slotInstallClicked()
{
    KILE_DEBUG_MAIN << "install clicked";

    QString directory = KileMenu::UserMenu::selectUserMenuDir();
    QString filter = i18n("User Menu Files (*.xml)");

    QString xmlfile = QFileDialog::getOpenFileName(this, i18n("Select Menu File"), directory, filter);
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
    KILE_DEBUG_MAIN << "remove clicked";

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
