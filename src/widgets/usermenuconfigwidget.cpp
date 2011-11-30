/***************************************************************************
    begin                : Nov 27 2011
    author               : dani
 ***************************************************************************/

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

KileWidgetUsermenuConfig::KileWidgetUsermenuConfig(KileMenu::LatexUserMenu *latexmenu, QWidget *parent) 
   : QWidget(parent), m_latexmenu(latexmenu)
{
	setupUi(this);
	setXmlFile( m_latexmenu->xmlFile() );

	m_menuPosition = KileConfig::menuPosition();
	if ( m_menuPosition == KileMenu::LatexUserMenu::DaniMenuPosition ) {
		m_rbMenuPositionDani->setChecked(true);
	}
	else {
		m_rbMenuPositionLatex->setChecked(true);
	}
	
	// connect dialog with latexmenu to install xml file
	connect(this, SIGNAL(installXmlFile(const QString &)), m_latexmenu, SLOT(slotInstallXmlFile(const QString &)));
	connect(this, SIGNAL(removeXmlFile()), m_latexmenu, SLOT(slotRemoveXmlFile()));
	connect(this, SIGNAL(changeMenuPosition(int)), m_latexmenu, SLOT(slotChangeMenuPosition(int)));

	connect(m_pbInstall, SIGNAL(clicked()), this, SLOT(slotInstallClicked()));
	connect(m_pbRemove,  SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
	
}

KileWidgetUsermenuConfig::~KileWidgetUsermenuConfig()
{
}

void KileWidgetUsermenuConfig::writeConfig()
{
	int position = ( m_rbMenuPositionDani->isChecked() ) ? KileMenu::LatexUserMenu::DaniMenuPosition : KileMenu::LatexUserMenu::LatexMenuPosition;
	if ( m_menuPosition != position ) {
		KILE_DEBUG() << "menu position changed";
		KileConfig::setMenuPosition(position);
		emit(changeMenuPosition(position));
	}
}

void KileWidgetUsermenuConfig::slotInstallClicked()
{
	KILE_DEBUG() << "install clicked";

	QString directory = KileMenu::LatexUserMenu::selectLatexmenuDir();   
	QString filter = i18n("*.xml|Latex Menu Files");

	QString xmlfile = KFileDialog::getOpenFileName(directory, filter, this, i18n("Select Menu File"));
	if ( xmlfile.isEmpty() ) {
		return;
	}

	if( QFile::exists(xmlfile) ) {
		emit (installXmlFile(xmlfile));
		setXmlFile(xmlfile);
	}
	else {
		KMessageBox::error(this, i18n("File '%1' does not exist.", xmlfile));
	}
}

void KileWidgetUsermenuConfig::slotRemoveClicked()
{
	KILE_DEBUG() << "remove clicked";
	
	emit (removeXmlFile());
	setXmlFile(QString::null);
}

void KileWidgetUsermenuConfig::setXmlFile(const QString &file)
{
	if ( file.isEmpty() ) {
		m_usermenuFile->setText(i18n("no file installed"));
		m_pbRemove->setEnabled(false);
	}
	else {
		m_usermenuFile->setText(file);
		m_pbRemove->setEnabled(true);
	}
}


#include "usermenuconfigwidget.moc"
