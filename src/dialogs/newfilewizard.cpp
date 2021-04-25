/*****************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2005 by Holger Danielsson (holger.danielsson@t-online.de)
                               2006-2014 by Michel Ludwig (michel.ludwig@kdemail.net)
******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/newfilewizard.h"

#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QMap>

#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "kiledebug.h"
#include "widgets/newdocumentwidget.h"

#define LATEX_TYPE	0
#define BIBTEX_TYPE	1
#define SCRIPT_TYPE	2

NewFileWizard::NewFileWizard(KileTemplate::Manager *templateManager, KileDocument::Type startType,
                             QWidget *parent, const char *name)
    : QDialog(parent), m_templateManager(templateManager), m_currentlyDisplayedType(-1)
{
    setObjectName(name);
    setWindowTitle(i18n("New File"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // first read config
    KConfigGroup newFileWizardGroup = KSharedConfig::openConfig()->group("NewFileWizard");
    bool wizard = newFileWizardGroup.readEntry("UseWizardWhenCreatingEmptyFile", false);
    int w = newFileWizardGroup.readEntry("width", -1);
    if(w == -1) {
        w = width();
    }
    int h = newFileWizardGroup.readEntry("height", -1);
    if(h == -1) {
        h = height();
    }

    m_newDocumentWidget = new NewDocumentWidget(this);
    connect(m_newDocumentWidget->templateIconView, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(okButtonClicked()));
    m_templateManager->scanForTemplates();
    m_newDocumentWidget->templateIconView->setTemplateManager(m_templateManager);

    connect(m_newDocumentWidget->documentTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
    connect(m_newDocumentWidget->templateIconView, SIGNAL(classFileSearchFinished()), this, SLOT(restoreSelectedIcon()));

    mainLayout->addWidget(m_newDocumentWidget);

    m_newDocumentWidget->documentTypeComboBox->insertItem(LATEX_TYPE, i18n("LaTeX Document"));
    m_newDocumentWidget->documentTypeComboBox->insertItem(BIBTEX_TYPE, i18n("BibTeX Document"));
    m_newDocumentWidget->documentTypeComboBox->insertItem(SCRIPT_TYPE, i18n("Kile Script"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);

    // set config entries
    m_newDocumentWidget->quickStartWizardCheckBox->setChecked(wizard);
    resize(w,h);

    int index = 0;
    switch(startType) {
    default: // fall through
    case KileDocument::LaTeX:
        index = LATEX_TYPE;
        break;
    case KileDocument::BibTeX:
        index = BIBTEX_TYPE;
        break;
    case KileDocument::Script:
        index = SCRIPT_TYPE;
        break;
    }

    // select the document type
    m_newDocumentWidget->documentTypeComboBox->setCurrentIndex(index);
    m_currentlyDisplayedType = index;
    displayType(index);
}

NewFileWizard::~NewFileWizard()
{
}

TemplateItem* NewFileWizard::getSelection() const
{
    QList<QListWidgetItem*> selectedItems = m_newDocumentWidget->templateIconView->selectedItems();
    if(selectedItems.isEmpty()) {
        return Q_NULLPTR;
    }
    return static_cast<TemplateItem*>(selectedItems.first());
}

bool NewFileWizard::useWizard()
{
    // check (among other things) whether we want to create a LaTeX document
    return ((m_newDocumentWidget->documentTypeComboBox->currentIndex() == 0)
            && getSelection()
            && (getSelection()->name() == KileTemplate::Manager::defaultEmptyTemplateCaption()
                || getSelection()->name() == KileTemplate::Manager::defaultEmptyLaTeXTemplateCaption())
            && m_newDocumentWidget->quickStartWizardCheckBox->isChecked());
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
        KSharedConfig::openConfig()->group("default").writeEntry(getConfigKey(m_currentlyDisplayedType), selectedItem->name());
    }
}

void NewFileWizard::restoreSelectedIcon()
{
    KConfigGroup defaultGroup = KSharedConfig::openConfig()->group("default");
    QString selectedIconName = defaultGroup.readEntry(getConfigKey(m_currentlyDisplayedType),
                               KileTemplate::Manager::defaultEmptyTemplateCaption());
    QList<QListWidgetItem*> items = m_newDocumentWidget->templateIconView->findItems(selectedIconName, Qt::MatchExactly);
    if(items.count() > 0) {
        items[0]->setSelected(true);
    }
}

void NewFileWizard::okButtonClicked()
{
    KConfigGroup newFileWizardGroup = KSharedConfig::openConfig()->group("NewFileWizard");

    newFileWizardGroup.writeEntry("UseWizardWhenCreatingEmptyFile", m_newDocumentWidget->quickStartWizardCheckBox->isChecked());
    newFileWizardGroup.writeEntry("width", width());
    newFileWizardGroup.writeEntry("height", height());

    storeSelectedIcon();
    accept();
}

void NewFileWizard::slotActivated(int index)
{
    storeSelectedIcon();
    m_currentlyDisplayedType = index;
    displayType(index);
}

void NewFileWizard::displayType(int index)
{
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

