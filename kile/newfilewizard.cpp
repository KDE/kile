/*****************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2005 by Holger Danielsson (holger.danielsson@t-online.de)
                               2006, 2007 by Michel Ludwig (michel.ludwig@kdemail.net)
******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newfilewizard.h"

#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmap.h>

#include <klocale.h>
#include "kiledebug.h"
#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include "newdocumentwidget.h"

#define LATEX_TYPE	0
#define BIBTEX_TYPE	1
#define SCRIPT_TYPE	2

NewFileWizard::NewFileWizard(KileTemplate::Manager *templateManager, QWidget *parent, const char *name )
  : KDialogBase(parent,name,true,i18n("New File"),KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), m_templateManager(templateManager), m_currentlyDisplayedType(-1)
{
	// first read config
	m_config = kapp->config();
	m_config->setGroup("NewFileWizard");
	bool wizard = m_config->readBoolEntry("UseWizardWhenCreatingEmptyFile", false);
	int w = m_config->readNumEntry("width", -1);
	if ( w == -1 ) w = width();
	int h = m_config->readNumEntry("height", -1);
	if ( h == -1 ) h = height();

	m_newDocumentWidget = new NewDocumentWidget(this);
	connect(m_newDocumentWidget->templateIconView, SIGNAL(doubleClicked(QIconViewItem *)), SLOT(slotOk()));
	m_templateManager->scanForTemplates();
	m_newDocumentWidget->templateIconView->setTemplateManager(m_templateManager);
	m_newDocumentWidget->templateIconView->fillWithTemplates(KileDocument::LaTeX);

	connect(m_newDocumentWidget->documentTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
	connect(m_newDocumentWidget->templateIconView, SIGNAL(classFileSearchFinished()), this, SLOT(restoreSelectedIcon()));

	setMainWidget(m_newDocumentWidget);

	m_newDocumentWidget->documentTypeComboBox->insertItem(i18n("LaTeX Document"), LATEX_TYPE);
	m_newDocumentWidget->documentTypeComboBox->insertItem(i18n("BibTeX Document"), BIBTEX_TYPE);
	m_newDocumentWidget->documentTypeComboBox->insertItem(i18n("Kile Script"), SCRIPT_TYPE);

	// set config entries
	m_newDocumentWidget->quickStartWizardCheckBox->setChecked(wizard);
	resize(w,h);

	// select the LaTeX type
	m_newDocumentWidget->documentTypeComboBox->setCurrentItem(LATEX_TYPE);
	m_currentlyDisplayedType = LATEX_TYPE;
	restoreSelectedIcon();
}

NewFileWizard::~NewFileWizard()
{
}

TemplateItem* NewFileWizard::getSelection()const
{
	for(QIconViewItem *item = m_newDocumentWidget->templateIconView->firstItem(); item; item = item->nextItem()) {
		if(item->isSelected()) {
			return static_cast<TemplateItem*>(item);
		}
	}
	return NULL;
}

bool NewFileWizard::useWizard()
{
	// check (among other things) whether we want to create a LaTeX document
	return ( (m_newDocumentWidget->documentTypeComboBox->currentItem() == 0) && getSelection() && (getSelection()->name() == DEFAULT_EMPTY_CAPTION || getSelection()->name() == DEFAULT_EMPTY_LATEX_CAPTION) && m_newDocumentWidget->quickStartWizardCheckBox->isChecked() );
}

QString NewFileWizard::getConfigKey(int index)
{
	QString configKey = "NewFileWizardSelectedIcon";
	switch(index) {
		case LATEX_TYPE:
			configKey += "LaTeX";
		break;

		case BIBTEX_TYPE:
			configKey += "BibTeX";
		break;

		case SCRIPT_TYPE:
			configKey += "Script";
		break;
	}
	return configKey;
}

void NewFileWizard::storeSelectedIcon()
{
	if(m_currentlyDisplayedType < 0) {
		return;
	}
	TemplateItem *selectedItem = getSelection();
	if (selectedItem) {
		m_config->writeEntry(getConfigKey(m_currentlyDisplayedType), selectedItem->name());
	}
}

void NewFileWizard::restoreSelectedIcon()
{
	QString selectedIconName = m_config->readEntry(getConfigKey(m_currentlyDisplayedType), DEFAULT_EMPTY_CAPTION);
	QIconViewItem *item = m_newDocumentWidget->templateIconView->findItem(selectedIconName);
	if(item) {
		m_newDocumentWidget->templateIconView->setSelected(item, true);
	}
}

void NewFileWizard::slotOk()
{
	m_config->setGroup("NewFileWizard");
	m_config->writeEntry("UseWizardWhenCreatingEmptyFile", m_newDocumentWidget->quickStartWizardCheckBox->isChecked());
	m_config->writeEntry("width", width());
	m_config->writeEntry("height", height());
	
	storeSelectedIcon();
	accept();
}

void NewFileWizard::slotActivated(int index)
{
	storeSelectedIcon();
	m_currentlyDisplayedType = index;
	switch(index) {
		case LATEX_TYPE:
			m_newDocumentWidget->templateIconView->fillWithTemplates(KileDocument::LaTeX);
		break;

		case BIBTEX_TYPE:
			m_newDocumentWidget->templateIconView->fillWithTemplates(KileDocument::BibTeX);
		break;

		case SCRIPT_TYPE:
			m_newDocumentWidget->templateIconView->fillWithTemplates(KileDocument::Script);
		break;
	}
	m_newDocumentWidget->quickStartWizardCheckBox->setEnabled((index == 0));

	// and select an icon
	restoreSelectedIcon();
}

#include "newfilewizard.moc"
