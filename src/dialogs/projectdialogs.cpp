/*******************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
            (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2013-2016 by Michel Ludwig (michel.ludwig@kdemail.net)
            (C) 2015 by Andreas Cord-Landwehr (cordlandwehr@kde.org)
********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-02-15 (dani)
//  - cosmetic changes
//  - use of groupboxes to prepare further extensions

// 2007-03-12 (dani)
//  - use KileDocument::Extensions
//  - allowed extensions are always defined as list, f.e.: .tex .ltx .latex

#include "dialogs/projectdialogs.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QRegExp>
#include <QValidator>
#include <QVBoxLayout>
#include <QFormLayout>

#include <KComboBox>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlCompletion>
#include <KConfigGroup>

#include "kiledebug.h"
#include "kileproject.h"
#include "kiletoolmanager.h"
#include "documentinfo.h"
#include "kileconfig.h"
#include "kileextensions.h"
#include "templates.h"

KileProjectDialogBase::KileProjectDialogBase(const QString &caption, KileDocument::Extensions *extensions, QWidget *parent, const char *name)
    : QDialog(parent)
    , m_extmanager(extensions)
    , m_project(Q_NULLPTR)
    , m_projectGroup(new QGroupBox(i18n("Project"), this))
    , m_extensionGroup(new QGroupBox(i18n("Extensions"), this))
{
    setWindowTitle(caption);
    setModal(true);
    setObjectName(name);

    const QString whatsthisName = i18n("Insert a short descriptive name of your project here.");
    const QString whatsthisExt = i18n("Insert a list (separated by spaces) of file extensions which should be treated also as files of the corresponding type in this project.");

    m_title = new QLineEdit(m_projectGroup);
    m_title->setWhatsThis(whatsthisName);
    QLabel *projectTitleLabel = new QLabel(i18n("Project &title:"), m_projectGroup);
    projectTitleLabel->setBuddy(m_title);
    projectTitleLabel->setWhatsThis(whatsthisName);

    // project settings groupbox
    QFormLayout *projectGoupLayout= new QFormLayout(m_projectGroup);
    projectGoupLayout->setAlignment(Qt::AlignTop);
    m_projectGroup->setLayout(projectGoupLayout);
    projectGoupLayout->addRow(projectTitleLabel, m_title);

    m_projectFolder = new KUrlRequester(m_projectGroup);
    m_projectFolder->setMode(KFile::Directory | KFile::LocalOnly);

    QLabel *projectFolderLabel = new QLabel(i18n("Project &folder:"), m_projectGroup);
    projectFolderLabel->setBuddy(m_projectFolder);
    const QString whatsthisPath = i18n("Insert the path to your project here.");
    m_projectFolder->setWhatsThis(whatsthisPath);
    projectGoupLayout->addRow(projectFolderLabel, m_projectFolder);

    // combo box for default graphics extension
    m_defaultGraphicsExtensionCombo = new QComboBox(this);
    KileDocument::Extensions extManager;
    QStringList imageExtensions = extManager.images().split(' ');
    foreach (const QString &extension, imageExtensions) {
        const QString extName = extension.mid(1); // all characters right of "."
        m_defaultGraphicsExtensionCombo->addItem(extension, extName);
    }
    m_defaultGraphicsExtensionCombo->addItem(i18n("(use global settings)"),"");
    const QString whatsThisTextDefaultGraphicsExtension = i18n("Default graphic extension to open when none specified by file name.");
    m_defaultGraphicsExtensionCombo->setWhatsThis(whatsThisTextDefaultGraphicsExtension);

    // extension settings groupbox
    m_userFileExtensions = new QLineEdit(this);
    m_userFileExtensions->setWhatsThis(whatsthisExt);
    QRegExp reg("[\\. a-zA-Z0-9]+");
    QRegExpValidator *extValidator = new QRegExpValidator(reg, m_extensionGroup);
    m_userFileExtensions->setValidator(extValidator);

    m_defaultLatexFileExtensionsCombo = new KComboBox(false, this);
    m_defaultLatexFileExtensionsCombo->addItem(i18n("Source Files"));
    m_defaultLatexFileExtensionsCombo->addItem(i18n("Package Files"));
    m_defaultLatexFileExtensionsCombo->addItem(i18n("Image Files"));
    m_defaultLatexFileExtensionsCombo->addItem(i18n("Bibliography Files"));
    m_defaultLatexFileExtensions = new QLabel(QString(), this);
    m_defaultLatexFileExtensionsCombo->setWhatsThis(whatsthisExt);

    QFormLayout *extensionGroupLayout = new QFormLayout(m_extensionGroup);
    m_extensionGroup->setLayout(extensionGroupLayout);
    extensionGroupLayout->setAlignment(Qt::AlignTop);
    extensionGroupLayout->addRow(new QLabel(i18n("Default Graphics Extension:"), this), m_defaultGraphicsExtensionCombo);
    extensionGroupLayout->addRow(m_defaultLatexFileExtensionsCombo, m_userFileExtensions);
    extensionGroupLayout->addRow(new QLabel(i18n("Predefined:"), this), m_defaultLatexFileExtensions);

    fillProjectDefaults();

    QWidget::setTabOrder(m_title, m_projectFolder);
    QWidget::setTabOrder(m_defaultGraphicsExtensionCombo, m_defaultLatexFileExtensionsCombo);
    QWidget::setTabOrder(m_defaultLatexFileExtensionsCombo, m_defaultLatexFileExtensions);
}

KileProjectDialogBase::~KileProjectDialogBase()
{
}

void KileProjectDialogBase::onExtensionsIndexChanged(int index)
{
    m_userFileExtensions->setText(m_val_extensions[index]);
    m_defaultLatexFileExtensions->setText(m_val_standardExtensions[index]);
}

void KileProjectDialogBase::onExtensionsTextEdited(const QString &text)
{
    m_val_extensions[m_defaultLatexFileExtensionsCombo->currentIndex()] = text;
}

bool KileProjectDialogBase::acceptUserExtensions()
{
    QRegExp reg("\\.\\w+");

    for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
        m_val_extensions[i-1] = m_val_extensions[i-1].trimmed();
        if (! m_val_extensions[i-1].isEmpty()) {
            // some tiny extension checks
            QStringList::ConstIterator it;
            QStringList list = m_val_extensions[i-1].split(' ');
            for (it = list.constBegin(); it != list.constEnd(); ++it) {
                if (! reg.exactMatch(*it)) {
                    KMessageBox::error(this, i18n("Error in extension '%1':\nAll user-defined extensions should look like '.xyz'", *it), i18n("Invalid extension"));
                    return false;
                }
            }
        }
    }

    return true;
}

void KileProjectDialogBase::setExtensions(KileProjectItem::Type type, const QString & ext)
{
    if (m_defaultLatexFileExtensionsCombo->currentIndex() == type - 1) {
        m_userFileExtensions->setText(ext);
    }
    else {
        m_val_extensions[type-1] = ext;
    }
}

void KileProjectDialogBase::setProject(KileProject *project, bool override)
{
    m_project = project;

    if ((!override) || (project == 0)) {
        return;
    }

    for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
        m_val_extensions[i - 1] = project->extensions((KileProjectItem::Type) i);
    }

    m_title->setText(m_project->name());
    m_userFileExtensions->setText(m_val_extensions[0]);
    m_defaultLatexFileExtensions->setText(m_val_standardExtensions[0]);

    m_defaultGraphicsExtensionCombo->setCurrentIndex(m_defaultGraphicsExtensionCombo->findData(project->defaultGraphicExt()));
}

KileProject* KileProjectDialogBase::project()
{
    return m_project;
}

void KileProjectDialogBase::fillProjectDefaults()
{
    m_val_extensions[0].clear();
    m_val_extensions[1].clear();
    m_val_extensions[2].clear();
    m_val_extensions[3].clear();
    //m_val_extensions[4] = OTHER_EXTENSIONS;

    m_val_standardExtensions[0] = m_extmanager->latexDocuments();
    m_val_standardExtensions[1] = m_extmanager->latexPackages();
    m_val_standardExtensions[2] = m_extmanager->images();
    m_val_standardExtensions[3] = m_extmanager->bibtex();

    m_userFileExtensions->setText(m_val_extensions[0]);
    m_defaultLatexFileExtensions->setText(m_val_standardExtensions[0]);

    m_defaultGraphicsExtensionCombo->setCurrentIndex(0);
}

/*
 * KileNewProjectDialog
 */
KileNewProjectDialog::KileNewProjectDialog(KileTemplate::Manager *templateManager, KileDocument::Extensions *extensions, QWidget* parent, const char* name)
    : KileProjectDialogBase(i18n("Create New Project"), extensions, parent, name)
    , m_templateManager(templateManager)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // properties groupbox
    mainLayout->addWidget(m_projectGroup);

    // second groupbox
    QGroupBox *fileGroup = new QGroupBox(i18n("File"), this);
    mainLayout->addWidget(fileGroup);
    QGridLayout *fileGrid = new QGridLayout();
    fileGroup->setLayout(fileGrid);
    m_createNewFileCheckbox = new QCheckBox(i18n("Create a new file and add it to this project"), fileGroup);
    m_createNewFileCheckbox->setChecked(true);
    m_filenameLabel  = new QLabel(i18n("File&name (relative to where the project file is):"), fileGroup);
    m_file = new QLineEdit(fileGroup);
    m_filenameLabel->setBuddy(m_file);
    m_templateIconView = new TemplateIconView(fileGroup);
    m_templateIconView->setTemplateManager(m_templateManager);
    m_templateManager->scanForTemplates();
    m_templateIconView->fillWithTemplates(KileDocument::LaTeX);
    m_createNewFileCheckbox->setWhatsThis(i18n("If you want Kile to create a new file and add it to the project, then check this option and select a template from the list that will appear below."));

    fileGrid->addWidget(m_createNewFileCheckbox, 0, 0, 1, 2);
    fileGrid->addWidget(m_filenameLabel, 1, 0);
    fileGrid->addWidget(m_file, 1, 1);
    fileGrid->addWidget(m_templateIconView, 2, 0, 1, 2);
    fileGrid->setColumnStretch(1, 1);
    connect(m_createNewFileCheckbox, SIGNAL(clicked()), this, SLOT(clickedCreateNewFileCb()));

    // add to layout
    mainLayout->addWidget(m_projectGroup);
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(m_extensionGroup);
    mainLayout->addStretch();

    fillProjectDefaults();

    // add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &KileNewProjectDialog::handleOKButtonClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_defaultLatexFileExtensionsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &KileNewProjectDialog::onExtensionsIndexChanged);
    connect(m_userFileExtensions, &QLineEdit::textEdited, this, &KileNewProjectDialog::onExtensionsTextEdited);

    mainLayout->addWidget(buttonBox);

    QWidget::setTabOrder(m_projectFolder, m_createNewFileCheckbox);

    QWidget::setTabOrder(m_createNewFileCheckbox, m_file);
    QWidget::setTabOrder(m_file, m_templateIconView);
    QWidget::setTabOrder(m_templateIconView, m_defaultGraphicsExtensionCombo);

    QWidget::setTabOrder(m_defaultGraphicsExtensionCombo, buttonBox);
}

KileNewProjectDialog::~KileNewProjectDialog()
{}

KileProject * KileNewProjectDialog::project()
{
    if (!m_project) {
        m_project = new KileProject(projectTitle(), m_projectFileWithPath, m_extmanager);

        KileProjectItem::Type type;
        for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
            type = (KileProjectItem::Type) i;
            m_project->setExtensions(type, extensions(type));
        }

        m_project->setDefaultGraphicExt(
            m_defaultGraphicsExtensionCombo->itemData(m_defaultGraphicsExtensionCombo->currentIndex()).toString());

        m_project->buildProjectTree();
    }

    return m_project;
}

void KileNewProjectDialog::clickedCreateNewFileCb()
{
    if (m_createNewFileCheckbox->isChecked()) {
        m_file->show();
        m_filenameLabel->show();
        m_templateIconView->show();
    }
    else {
        m_file->hide();
        m_filenameLabel->hide();
        m_templateIconView->hide();
    }
}

QString KileNewProjectDialog::cleanProjectFile()
{
    return projectTitle().toLower().trimmed().remove(QRegExp("\\s*")) + ".kilepr";
}

void KileNewProjectDialog::handleOKButtonClicked()
{
    if (!acceptUserExtensions()) {
        return;
    }

    if (projectTitle().trimmed().isEmpty()) {
        if (KMessageBox::warningYesNo(this, i18n("You have not entered a project name. If you decide to proceed, the project name will be set to \"Untitled\".\n"
                                      "Do you want to create the project nevertheless?"), i18n("No Project Name Given")) == KMessageBox::Yes) {
            m_title->setText(i18n("Untitled"));
        }
        else {
            return;
        }
    }

    const QString dirString = folder().trimmed();
    const QString fileString = file().trimmed();

    if (dirString.isEmpty()) {
        KMessageBox::error(this, i18n("Please enter the folder where the project file should be saved to."), i18n("Empty Location"));
        return;
    }

    if (!QDir::isAbsolutePath(dirString)) {
        KMessageBox::error(this, i18n("Please enter an absolute path to the project folder."), i18n("Invalid Location"));
        return;
    }

    if (createNewFile() && fileString.isEmpty()) {
        KMessageBox::error(this, i18n("Please enter a filename for the file that should be added to this project."), i18n("No File Name Given"));
        return;
    }

    const QString cleanProjectFileName = cleanProjectFile();
    const QDir projectDir(dirString);
    const QString projectFilePath = projectDir.filePath(cleanProjectFileName);
    const QDir guiFileDir = KileProject::getPathForPrivateKileDirectory(projectFilePath);

    testDirectoryIsUsable(projectDir);
    testDirectoryIsUsable(guiFileDir);

    if (QFileInfo(projectFilePath).exists()) { // this can only happen when the project dir existed already
        KMessageBox::error(this, i18n("The project file exists already. Please choose another name."), i18n("Project File Already Exists"));
        return;
    }

    const QString guiProjectFilePath = KileProject::getPathForGUISettingsProjectFile(projectFilePath);
    if (QFileInfo(guiProjectFilePath).exists()) { // this can only happen when the project dir existed already
        KMessageBox::error(this, i18n("The GUI settings file exists already. Please choose another project name."), i18n("Project File Already Exists"));
        return;
    }

    if (createNewFile()) {
        //check for validity of name first, then check for existence (fixed by tbraun)
        QUrl fileURL;
        fileURL = fileURL.adjusted(QUrl::RemoveFilename);
        fileURL.setPath(fileURL.path() + file());
        QUrl validURL = KileDocument::Info::makeValidTeXURL(fileURL, this, m_extmanager->isTexFile(fileURL), true);
        if(validURL != fileURL) {
            m_file->setText(validURL.fileName());
        }

        if(QFileInfo(projectDir.filePath(fileString)).exists()) {
            if (KMessageBox::warningYesNo(this, i18n("The file \"%1\" already exists, overwrite it?", fileString), i18n("File Already Exists")) == KMessageBox::No) {
                return;
            }
        }
    }

    m_projectFileWithPath = QUrl::fromLocalFile(projectFilePath);
    accept();
}

void KileNewProjectDialog::fillProjectDefaults()
{
    m_projectFolder->lineEdit()->setText(QDir::cleanPath(KileConfig::defaultProjectLocation()));
    m_createNewFileCheckbox->setChecked(true);
    KileProjectDialogBase::fillProjectDefaults();
}

TemplateItem* KileNewProjectDialog::getSelection() const
{
    return static_cast<TemplateItem*>(m_templateIconView->currentItem());
}

/*
 * KileProjectOptionsDialog
 */
KileProjectOptionsDialog::KileProjectOptionsDialog(KileProject *project, KileDocument::Extensions *extensions, QWidget *parent, const char * name)
    : KileProjectDialogBase(i18n("Project Options"), extensions, parent, name)
    , m_toolDefaultString(i18n("(use global setting)"))
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // properties groupbox
    mainLayout->addWidget(m_projectGroup);

    // third groupbox
    QGroupBox *group3 = new QGroupBox(i18n("Properties"), this);
    mainLayout->addWidget(group3);
    QGridLayout *grid3 = new QGridLayout();
    grid3->setAlignment(Qt::AlignTop);
    group3->setLayout(grid3);

    const QString whatsthisMaster = i18n("Select the default master document. Leave empty for auto detection.");

    m_selectMasterDocumentCombo = new KComboBox(false, group3);
    m_selectMasterDocumentCombo->setObjectName("master");
    //m_selectMasterDocumentCombo->setDisabled(true);
    QLabel *lb1 = new QLabel(i18n("&Master document:"), group3);
    lb1->setBuddy(m_selectMasterDocumentCombo);
    lb1->setMinimumWidth(m_defaultLatexFileExtensionsCombo->sizeHint().width());
    m_selectMasterDocumentCombo->setWhatsThis(whatsthisMaster);
    lb1->setWhatsThis(whatsthisMaster);

    m_selectMasterDocumentCombo->addItem(i18n("(auto-detect)"));
    QList<KileProjectItem*> rootItemList = project->rootItems();
    int index = 0;
    for (QList<KileProjectItem*>::iterator it = rootItemList.begin(); it != rootItemList.end(); ++it) {
        if ((*it)->type() == KileProjectItem::Source) {
            m_selectMasterDocumentCombo->addItem((*it)->url().fileName());
            ++index;
            if ((*it)->url().path() == project->masterDocument()) {
                m_selectMasterDocumentCombo->setCurrentIndex(index);
            }
        }
    }

    if (project->masterDocument().isEmpty()) {
        m_selectMasterDocumentCombo->setCurrentIndex(0);
    }

    QLabel *quickbuildLabel = new QLabel(i18n("&QuickBuild configuration:"), group3);
    m_QuickBuildCheckbox = new KComboBox(group3);
    quickbuildLabel->setBuddy(m_QuickBuildCheckbox);
    m_QuickBuildCheckbox->addItem(m_toolDefaultString);
    m_QuickBuildCheckbox->addItems(KileTool::configNames("QuickBuild", KSharedConfig::openConfig().data()));
    QString itemToSelect = project->quickBuildConfig().length() > 0 ? project->quickBuildConfig() : m_toolDefaultString;
    int selectIndex = m_QuickBuildCheckbox->findText(itemToSelect);
    if(selectIndex >= 0) {
        m_QuickBuildCheckbox->setCurrentIndex(selectIndex);
    }
    else {
        m_QuickBuildCheckbox->addItem(itemToSelect);
    }

    //don't put this after the call to toggleMakeIndex
    setProject(project, true);
    m_projectFolder->setUrl(project->baseURL());
    m_projectFolder->setEnabled(false);

    m_ckMakeIndex = new QCheckBox(i18n("&MakeIndex options"), group3);
    connect(m_ckMakeIndex, SIGNAL(toggled(bool)), this, SLOT(toggleMakeIndex(bool)));
    m_leMakeIndex = new QLineEdit(group3);
    m_ckMakeIndex->setChecked(project->useMakeIndexOptions());
    toggleMakeIndex(m_ckMakeIndex->isChecked());

    grid3->addWidget(lb1, 0, 0);
    grid3->addWidget(m_selectMasterDocumentCombo, 0, 1);
    grid3->addWidget(quickbuildLabel, 1, 0);
    grid3->addWidget(m_QuickBuildCheckbox, 1, 1);
    grid3->addWidget(m_ckMakeIndex, 2, 0);
    grid3->addWidget(m_leMakeIndex, 2, 1, 1, 2);
    grid3->setColumnStretch(2, 1);

    // add to layout
    mainLayout->addWidget(m_projectGroup);
    mainLayout->addWidget(m_extensionGroup);
    mainLayout->addWidget(group3);
    mainLayout->addStretch();

    // add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(this, &QDialog::accepted, this, &KileProjectOptionsDialog::onAccepted);

    connect(m_defaultLatexFileExtensionsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &KileProjectOptionsDialog::onExtensionsIndexChanged);
    connect(m_userFileExtensions, &QLineEdit::textChanged, this, &KileProjectOptionsDialog::onExtensionsTextEdited);

    mainLayout->addWidget(buttonBox);

    QWidget::setTabOrder(m_projectFolder, m_defaultGraphicsExtensionCombo);
    QWidget::setTabOrder(m_defaultLatexFileExtensions, m_selectMasterDocumentCombo);

    QWidget::setTabOrder(m_selectMasterDocumentCombo, m_QuickBuildCheckbox);
    QWidget::setTabOrder(m_QuickBuildCheckbox, m_ckMakeIndex);
    QWidget::setTabOrder(m_ckMakeIndex, m_leMakeIndex);

    QWidget::setTabOrder(m_leMakeIndex, buttonBox);
}

KileProjectOptionsDialog::~KileProjectOptionsDialog()
{
}

void KileProjectOptionsDialog::toggleMakeIndex(bool on)
{
    KILE_DEBUG_MAIN << "TOGGLED!" << endl;
    m_leMakeIndex->setEnabled(on);
    m_project->setUseMakeIndexOptions(on);
    m_project->writeUseMakeIndexOptions();
    m_project->readMakeIndexOptions();
    m_leMakeIndex->setText(m_project->makeIndexOptions());
}

void KileProjectOptionsDialog::onAccepted()
{
    if(!acceptUserExtensions()) {
        return;
    }

    this->m_project->setName(m_title->text());

    QList<KileProjectItem*> rootItemList = m_project->rootItems();
    for (QList<KileProjectItem*>::iterator it = rootItemList.begin(); it != rootItemList.end(); ++it) {
        if ((*it)->url().fileName() == m_selectMasterDocumentCombo->currentText()) {
            m_project->setMasterDocument((*it)->url().toLocalFile());
        }
    }
    if (m_selectMasterDocumentCombo->currentIndex() == 0) {
        m_project->setMasterDocument(QString());
    }

    m_val_extensions[m_defaultLatexFileExtensionsCombo->currentIndex()] = m_userFileExtensions->text();

    for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
        m_project->setExtensions((KileProjectItem::Type) i, m_val_extensions[i-1]);
    }

    if (m_QuickBuildCheckbox->currentText() == m_toolDefaultString) {
        m_project->setQuickBuildConfig("");
    }
    else {
        m_project->setQuickBuildConfig(m_QuickBuildCheckbox->currentText());
    }

    m_project->setUseMakeIndexOptions(m_ckMakeIndex->isChecked());
    if (m_project->useMakeIndexOptions()) {
        m_project->setMakeIndexOptions(m_leMakeIndex->text());
    }

    m_project->setDefaultGraphicExt(
        m_defaultGraphicsExtensionCombo->itemData(m_defaultGraphicsExtensionCombo->currentIndex()).toString());

    m_project->save();
}

bool KileNewProjectDialog::testDirectoryIsUsable(const QString& path)
{
    return testDirectoryIsUsable(QDir(path));
}

bool KileNewProjectDialog::testDirectoryIsUsable(const QDir& dir)
{
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }

    if (!dir.exists()) {
        KMessageBox::error(this, i18n("<p>Could not create the project folder \"\n%1\"</p>."
                                      "<p>Please check whether you have write permissions.</p>", dir.path()));
        return false;
    }

    QFileInfo fi(dir.absolutePath());
    if (!fi.isDir() || !fi.isWritable()) {
        KMessageBox::error(this, i18n("<p>The project folder \"(%1)\" is not writable.</p>"
                                      "<p>Please check the permissions of the project folder.</p>", dir.path()));
        return false;
    }
    return true;
}
