/******************************************************************************************
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           (C) 2011 by Libor Bukata (lbukata@gmail.com)
                           (C) 2013 by Michel Ludwig (michel.ludwig@kdemail.net)
 ******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "listselector.h"

#include <algorithm>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>

#include <KDirWatch>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRun>

#include "kiledebug.h"
#include "codecompletion.h"

//////////////////// KileListSelector ////////////////////

KileListSelector::KileListSelector(const QStringList &list, const QString &caption, const QString &select, bool sort,
                                   QWidget *parent, const char *name)
    : QDialog(parent)
    , m_listView(new QTreeWidget(this))
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel))
{
    setObjectName(name);
    setWindowTitle(caption);
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(new QLabel(select, this));

    m_listView->setHeaderLabel(i18n("Files"));
    m_listView->setSortingEnabled(false);
    m_listView->setAllColumnsShowFocus(true);
    m_listView->setRootIsDecorated(false);
    mainLayout->addWidget(m_listView);
    mainLayout->addWidget(new QLabel(i18np("1 item found.", "%1 items found.", list.size())));

    m_listView->setSortingEnabled(sort);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    if (sort) {
        m_listView->sortByColumn(0, Qt::AscendingOrder);
    }

    insertStringList(list);

    m_listView->clearSelection();
    connect(m_listView, &QTreeWidget::itemDoubleClicked, this, &QDialog::accept);
    QItemSelectionModel *selectionModel = m_listView->selectionModel();
    if (selectionModel) { // checking just to be safe
        connect(selectionModel, &QItemSelectionModel::selectionChanged,
                this, &KileListSelector::handleSelectionChanged);
    }

    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setEnabled(false);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);
}

void KileListSelector::handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasSelection());
}

bool KileListSelector::hasSelection() const
{
    if (!m_listView->selectionModel()) {
        return false;
    }
    return m_listView->selectionModel()->hasSelection();
}

void KileListSelector::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    m_listView->setSelectionMode(mode);
}

void KileListSelector::insertStringList(const QStringList &list)
{
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_listView, QStringList(*it));

        if (it == list.begin()) {
            m_listView->setCurrentItem(item);
        }
    }
}

QStringList KileListSelector::selectedItems() const
{
    QStringList items;
    QTreeWidgetItemIterator it(m_listView, QTreeWidgetItemIterator::Selected);
    while (*it) {
        items.append((*it)->text(0));
        ++it;
    }
    return items;
}


//////////////////// ManageCompletionFilesDialog ////////////////////

ManageCompletionFilesDialog::ManageCompletionFilesDialog(const QString& caption,
        const QString &localCompletionDir, const QString &globalCompletionDir, QWidget* parent, const char* name)
    : QDialog(parent)
    , m_localCompletionDirectory(localCompletionDir)
    , m_globalCompletionDirectory(globalCompletionDir)
{
    setObjectName(name);
    setWindowTitle(caption);
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_listView = new QTreeWidget(this);
    m_listView->setHeaderLabels(QStringList() << i18n("File Name") << i18n("Local File") << i18n("Add File?"));
    m_listView->setSortingEnabled(false);
    m_listView->setSelectionMode(QAbstractItemView::NoSelection);
    m_listView->setRootIsDecorated(false);
    mainLayout->addWidget(m_listView);

    m_dirWatcher = new KDirWatch(this);
    if (m_dirWatcher) {
        m_dirWatcher->addDir(localCompletionDir, KDirWatch::WatchFiles);
        connect(m_dirWatcher, &KDirWatch::created, this, &ManageCompletionFilesDialog::fillTreeView);
        connect(m_dirWatcher, &KDirWatch::deleted, this, &ManageCompletionFilesDialog::fillTreeView);
    }
    fillTreeView();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *installCustomButton = new QPushButton;
    QPushButton *manageCustomButton = new QPushButton;
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18n("Add selected files"));
    okButton->setToolTip(i18n("Add all the selected files"));
    installCustomButton->setText(i18n("Install custom files"));
    installCustomButton->setToolTip(i18n("Install your own completion files"));
    manageCustomButton->setText(i18n("Manage custom files"));
    manageCustomButton->setToolTip(i18n("Manage the local completion files in the file manager"));
    buttonBox->addButton(installCustomButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(manageCustomButton, QDialogButtonBox::ActionRole);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
    connect(installCustomButton, &QPushButton::clicked,
            this, &ManageCompletionFilesDialog::addCustomCompletionFiles);
    connect(manageCustomButton, &QPushButton::clicked,
            this, &ManageCompletionFilesDialog::openLocalCompletionDirectoryInFileManager);

    // Create the local path if it doesn't exist
    QDir localPath(m_localCompletionDirectory);
    if(!localPath.exists()) {
        localPath.mkpath(m_localCompletionDirectory);
    }
}

ManageCompletionFilesDialog::~ManageCompletionFilesDialog()
{
}

void ManageCompletionFilesDialog::fillTreeView() {
    // we want to keep selected items still selected after refreshing
    QSet<QString> previouslySelectedItems = selected();
    QStringList list = KileCodeCompletion::Manager::getAllCwlFiles(m_localCompletionDirectory, m_globalCompletionDirectory).uniqueKeys();
    std::sort(list.begin(), list.end());
    m_listView->clear();
    foreach(QString filename, list) {
        QString expectedLocalPath = m_localCompletionDirectory + '/' + filename;
        QString expectedGlobalPath = m_globalCompletionDirectory + '/' + filename;
        if (QFileInfo(expectedLocalPath).exists() && QFileInfo(expectedLocalPath).isReadable()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_listView, QStringList() << filename << i18n("yes"));
            item->setCheckState(2, previouslySelectedItems.contains(filename) ? Qt::Checked : Qt::Unchecked);
        }
        else if (QFileInfo(expectedGlobalPath).exists() && QFileInfo(expectedGlobalPath).isReadable()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_listView, QStringList() << filename << i18n("no"));
            item->setCheckState(2, previouslySelectedItems.contains(filename) ? Qt::Checked : Qt::Unchecked);
        }
        else {
            KILE_DEBUG_MAIN << "Cannot load file" << filename << "!";
        }
    }
    m_listView->resizeColumnToContents(0);
    m_listView->resizeColumnToContents(1);
    m_listView->resizeColumnToContents(2);
}

void ManageCompletionFilesDialog::addCustomCompletionFiles()
{
    bool someFileAdded = false;
    QStringList files = QFileDialog::getOpenFileNames(
                            this, i18n("Select Completion Files to Install Locally"), QString(), i18n("Completion files (*.cwl)"));

    if (files.isEmpty()) {
        return;
    }
    QDir workPath(m_localCompletionDirectory);

    foreach (QString file, files) {
        QFileInfo fileInf(file);
        QFileInfo localFile(m_localCompletionDirectory + '/' + fileInf.fileName());
        if (localFile.exists()) {
            const QString dialog_text = i18n("A local completion file with the name \"%1\" already exists.\nDo you want to replace this file?", localFile.fileName());
            const QString dialog_caption = i18n("Replace Local File?");
            if (KMessageBox::questionYesNo(this, dialog_text, dialog_caption) == KMessageBox::Yes) {
                if (!QFile::remove(localFile.absoluteFilePath())) {
                    KMessageBox::error(this, i18n("An error occurred while removing the file \"%1\".\nPlease check the file permissions.",
                                       localFile.fileName()), i18n("Remove Error"));
                    continue;
                }
            }
            else {
                // Skip selected file.
                continue;
            }
        }
        // Copy selected file to local directory.
        if (!QFile::copy(fileInf.absoluteFilePath(),localFile.absoluteFilePath())) {
            KMessageBox::error(this, i18n("Cannot copy the file to the local directory!\nPlease check the access permissions of the directory \"%1\".",
                               localFile.absolutePath()), i18n("Copy Error"));
        }
        else {
            // Add file to QTreeWidget or change status to local if a global file with the same name exists.
            QList<QTreeWidgetItem*> foundItems = m_listView->findItems(fileInf.fileName(), Qt::MatchExactly, 0);
            if (foundItems.empty()) {
                QTreeWidgetItem *item = new QTreeWidgetItem(m_listView, QStringList() << localFile.fileName() << i18n("yes"));
                item->setCheckState(2, Qt::Checked);
            }
            else {
                foundItems.first()->setCheckState(2, Qt::Checked);
                foundItems.first()->setText(1, i18n("yes"));
            }
            someFileAdded = true;
        }
    }

    // Resort QTreeWidget list.
    m_listView->sortItems(0, Qt::AscendingOrder);

    // Info about preselected files.
    if (someFileAdded == true) {
        KMessageBox::information(this,
                                 i18n("The custom files have been installed and preselected for adding."),
                                 i18n("Installation Successful"));
    }
}

void ManageCompletionFilesDialog::openLocalCompletionDirectoryInFileManager()
{
    new KRun(QUrl::fromLocalFile(m_localCompletionDirectory), QApplication::activeWindow());
}

const QSet<QString> ManageCompletionFilesDialog::selected() const
{
    QSet<QString> checked_files;
    for (int i = 0; i < m_listView->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_listView->topLevelItem(i);
        if (item->checkState(2) == Qt::Checked) {
            checked_files.insert(item->text(0));
        }
    }

    return checked_files;
}

