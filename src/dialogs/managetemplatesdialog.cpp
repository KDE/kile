/*****************************************************************************************
    begin                : Sun Apr 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 *****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/managetemplatesdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KIconDialog>
#include <KIconLoader>
#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

#include "kiledebug.h"
#include "kileextensions.h"
#include "kileinfo.h"
#include "templates.h"

class TemplateListViewItem : public QTreeWidgetItem {
public:
    TemplateListViewItem(QTreeWidget* parent, QTreeWidgetItem* preceding, const QString& mode, const KileTemplate::Info& info) : QTreeWidgetItem(parent, preceding), m_info(info) {
        setText(0, mode);
        setText(1, info.name);
        setText(2, KileInfo::documentTypeToString(info.type));
    }

    virtual ~TemplateListViewItem() {
    }

    KileTemplate::Info getTemplateInfo() {
        return m_info;
    }

protected:
    KileTemplate::Info m_info;
};

// dialog to create a template
ManageTemplatesDialog::ManageTemplatesDialog(KileTemplate::Manager *templateManager, const QUrl &sourceURL, const QString &caption, QWidget *parent, const char *name)
    : QDialog(parent)
    , m_templateManager(templateManager)
    , m_sourceURL(sourceURL)
{
    setObjectName(name);
    setWindowTitle(caption);
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_templateType = KileDocument::Extensions().determineDocumentType(sourceURL);

    QWidget *page = new QWidget(this);
    page->setObjectName("managetemplates_mainwidget");
    mainLayout->addWidget(page);
    QGridLayout *topLayout = new QGridLayout();
    topLayout->setMargin(0);
    page->setLayout(topLayout);

    topLayout->addWidget(new QLabel(i18n("Name:"), page), 0, 0);

    QString fileName = m_sourceURL.fileName();
    //remove the extension
    int dotPos = fileName.lastIndexOf('.');
    if (dotPos >= 0) {
        fileName = fileName.mid(0, dotPos);
    }
    m_nameEdit = new QLineEdit(fileName, page);
    mainLayout->addWidget(m_nameEdit);
    topLayout->addWidget(m_nameEdit, 0, 1);

    topLayout->addWidget(new QLabel(i18n("Type: %1", KileInfo::documentTypeToString(m_templateType)), page), 0, 2);
    topLayout->addWidget(new QLabel(i18n("Icon:"), page), 1, 0);

    m_iconEdit = new QLineEdit(QStandardPaths::locate(QStandardPaths::DataLocation, "pics/type_Default.png"), page);
    mainLayout->addWidget(m_iconEdit);
    topLayout->addWidget(m_iconEdit, 1, 1);

    QPushButton *iconbut = new QPushButton(i18n("Select..."), page);
    mainLayout->addWidget(iconbut);
    topLayout->addWidget(iconbut, 1, 2);

    m_templateList = new QTreeWidget(page);
    mainLayout->addWidget(m_templateList);
    m_templateList->setSortingEnabled(false);
    m_templateList->setHeaderLabels(QStringList() << i18nc("marked", "M")
                                    << i18n("Existing Templates")
                                    << i18n("Document Type"));
    m_templateList->setAllColumnsShowFocus(true);
    m_templateList->setRootIsDecorated(false);

    populateTemplateListView(m_templateType);

    topLayout->addWidget(m_templateList, 2, 0, 1, 3);

    m_showAllTypesCheckBox = new QCheckBox(i18n("Show all the templates"), page);
    mainLayout->addWidget(m_showAllTypesCheckBox);
    m_showAllTypesCheckBox->setChecked(false);
    connect(m_showAllTypesCheckBox, &QCheckBox::toggled, this, &ManageTemplatesDialog::updateTemplateListView);
    topLayout->addWidget(m_showAllTypesCheckBox, 3, 0, 1, 2);

    QPushButton *clearSelectionButton = new QPushButton(page);
    mainLayout->addWidget(clearSelectionButton);
    clearSelectionButton->setIcon(QIcon::fromTheme("edit-clear-locationbar"));
    int buttonSize = clearSelectionButton->sizeHint().height();
    clearSelectionButton->setFixedSize(buttonSize, buttonSize);
    clearSelectionButton->setToolTip(i18n("Clear Selection"));
    connect(clearSelectionButton, &QPushButton::clicked, this, &ManageTemplatesDialog::clearSelection);
    topLayout->addWidget(clearSelectionButton, 3, 2, Qt::AlignRight);

    topLayout->addWidget(new QLabel(i18n("Select an existing template if you want to overwrite it with your new template.\nNote that you cannot overwrite templates marked with an asterisk:\nif you do select such a template, a new template with the same name\nwill be created in a location you have write access to."), page), 4, 0, 1, 3);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_templateList, &QTreeWidget::itemClicked, this, &ManageTemplatesDialog::slotSelectedTemplate);
    connect(iconbut, &QPushButton::clicked, this, &ManageTemplatesDialog::slotSelectIcon);
    connect(this, &QDialog::accepted, this, &ManageTemplatesDialog::addTemplate);
}

// dialog to remove a template
ManageTemplatesDialog::ManageTemplatesDialog(KileTemplate::Manager *templateManager, const QString &caption, QWidget *parent, const char *name)
    : QDialog(parent)
    , m_templateManager(templateManager)
    , m_templateType(KileDocument::Undefined)
    , m_showAllTypesCheckBox(Q_NULLPTR)
{
    setObjectName(name);
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    m_templateList = new QTreeWidget(this);
    m_templateList->setSortingEnabled(false);
    m_templateList->setHeaderLabels(QStringList() << i18nc("marked", "M")
                                    << i18n("Existing Templates")
                                    << i18n("Document Type"));
    m_templateList->setAllColumnsShowFocus(true);
    m_templateList->setRootIsDecorated(false);

    populateTemplateListView(KileDocument::Undefined);

    mainLayout->addWidget(m_templateList);
    mainLayout->addWidget(new QLabel(i18n("Please select the template that you want to remove.\nNote that you cannot delete templates marked with an asterisk (for which you lack the necessary deletion permissions)."), this));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(this, &QDialog::accepted, this, &ManageTemplatesDialog::removeTemplate);
}

ManageTemplatesDialog::~ManageTemplatesDialog()
{
}

void ManageTemplatesDialog::updateTemplateListView(bool showAllTypes)
{
    populateTemplateListView((showAllTypes ? KileDocument::Undefined : m_templateType));
}

void ManageTemplatesDialog::clearSelection()
{
    m_templateList->clearSelection();
}

void ManageTemplatesDialog::populateTemplateListView(KileDocument::Type type)
{
    m_templateManager->scanForTemplates();
    KileTemplate::TemplateList templateList = m_templateManager->getTemplates(type);
    QString mode;
    QTreeWidgetItem* previousItem = Q_NULLPTR;

    m_templateList->clear();
    for (KileTemplate::TemplateListIterator i = templateList.begin(); i != templateList.end(); ++i)
    {
        KileTemplate::Info info = *i;
        QFileInfo iconFileInfo(info.icon);
        mode = (QFileInfo(info.path).isWritable() && (!iconFileInfo.exists() || iconFileInfo.isWritable())) ? " " : "*";
        if ((type == KileDocument::Undefined) || (info.type == type)) {
            previousItem = new TemplateListViewItem(m_templateList, previousItem, mode, info);
        }
    }

    m_templateList->resizeColumnToContents(0);
    m_templateList->resizeColumnToContents(1);
}

void ManageTemplatesDialog::slotSelectedTemplate(QTreeWidgetItem *item)
{
    TemplateListViewItem *templateItem = dynamic_cast<TemplateListViewItem*>(item);
    if (templateItem) {
        KileTemplate::Info info = templateItem->getTemplateInfo();
        m_nameEdit->setText(info.name);
        m_iconEdit->setText(info.icon);
    }
}

void ManageTemplatesDialog::slotSelectIcon()
{
    KIconDialog *dlg = new KIconDialog();
    QString res = dlg->openDialog();
    KIconLoader kil;

    if (!res.isNull()) {
        m_iconEdit->setText(kil.iconPath(res, -KIconLoader::SizeLarge, false));
    }
}

void ManageTemplatesDialog::addTemplate()
{
    QString templateName = (m_nameEdit->text()).trimmed();

    if (templateName.isEmpty()) {
        KMessageBox::error(this, i18n("The template name that you have entered is invalid.\nPlease enter a new name."));
        return;
    }

    QString icon = (m_iconEdit->text()).trimmed();
    QUrl iconURL = QUrl::fromUserInput(icon);

    if (icon.isEmpty()) {
        KMessageBox::error(this, i18n("Please choose an icon first."));
        return;
    }

    KIO::StatJob* statJob = KIO::stat(iconURL, KIO::StatJob::SourceSide, 0);
    KJobWidgets::setWindow(statJob, this);
    statJob->exec();
    if (statJob->error()) {
        KMessageBox::error(this, i18n("The icon file: %1\ndoes not seem to exist. Please choose a new icon.", icon));
        return;
    }

    statJob = KIO::stat(m_sourceURL, KIO::StatJob::SourceSide, 0);
    KJobWidgets::setWindow(statJob, this);
    statJob->exec();
    if (statJob->error()) {
        KMessageBox::error(this, i18n("The file: %1\ndoes not seem to exist. Maybe you forgot to save the file?", m_sourceURL.toString()));
        return;
    }

    QTreeWidgetItem* item = m_templateList->currentItem();

    if (!item && m_templateManager->searchForTemplate(templateName, m_templateType)) {
        KMessageBox::error(this, i18n("A template named \"%1\" already exists.\nPlease remove it first.", templateName));
        return;
    }

    bool returnValue;
    if (item) {
        TemplateListViewItem *templateItem = dynamic_cast<TemplateListViewItem*>(item);
        Q_ASSERT(templateItem);
        KileTemplate::Info templateInfo = templateItem->getTemplateInfo();
        if (KMessageBox::warningYesNo(this, i18n("You are about to replace the template \"%1\"; are you sure?", templateInfo.name)) == KMessageBox::No) {
            reject();
            return;
        }
        returnValue = m_templateManager->replace(templateInfo, m_sourceURL, templateName, iconURL);
    }
    else {
        returnValue = m_templateManager->add(m_sourceURL, templateName, iconURL);
    }
    if (!returnValue) {
        KMessageBox::error(this, i18n("Failed to create the template."));
        reject();
        return;
    }
}

bool ManageTemplatesDialog::removeTemplate()
{
    QTreeWidgetItem* item = m_templateList->currentItem();
    if (!item) {
        KMessageBox::information(this, i18n("Please select a template that should be removed."));
        return true;
    }

    TemplateListViewItem *templateItem = dynamic_cast<TemplateListViewItem*>(item);
    Q_ASSERT(templateItem);

    KileTemplate::Info templateInfo = templateItem->getTemplateInfo();

    KIO::StatJob* statJob = KIO::stat(QUrl::fromUserInput(templateInfo.path), KIO::StatJob::DestinationSide, 0);
    KIO::StatJob* statJob2 = KIO::stat(QUrl::fromUserInput(templateInfo.icon), KIO::StatJob::DestinationSide, 0);
    KJobWidgets::setWindow(statJob, this);
    KJobWidgets::setWindow(statJob2, this);
    statJob->exec();
    statJob2->exec();
    if ((statJob->error() && statJob2->error()) || !QFileInfo(templateInfo.icon).exists()) {
        KMessageBox::error(this, i18n("Sorry, but you do not have the necessary permissions to remove the selected template."));
        return false;
    }

    if (KMessageBox::warningYesNo(this, i18n("You are about to remove the template \"%1\"; are you sure?", templateInfo.name)) == KMessageBox::No) {
        return false;
    }

    if (!m_templateManager->remove(templateInfo)) {
        KMessageBox::error(this, i18n("The template could not be removed."));
        reject();
        return false;
    }
    return true;
}
