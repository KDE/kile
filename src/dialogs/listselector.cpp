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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <KApplication>
#include <KDirWatch>
#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>
#include <KRun>


#include "kiledebug.h"
#include "codecompletion.h"

//////////////////// KileListSelectorBase ////////////////////

KileListSelectorBase::KileListSelectorBase(const QStringList &list, const QString &caption, const QString &select, bool sort,
                                           QWidget *parent, const char *name)
: KDialog(parent)
{
	setObjectName(name);
	setCaption(caption);
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(KDialog::spacingHint());
	page->setLayout(layout);

	layout->addWidget(new QLabel(select, page));

	m_listView = new QTreeWidget(page);
	m_listView->setHeaderLabel(i18n("Files"));
	m_listView->setSortingEnabled(false);
	m_listView->setAllColumnsShowFocus(true);
	m_listView->setRootIsDecorated(false);

	layout->addWidget(m_listView);

	layout->addWidget(new QLabel(i18np("1 item found.", "%1 items found.", list.size())));

	m_listView->setSortingEnabled(sort);
	if(sort) {
		m_listView->sortByColumn(0, Qt::AscendingOrder);
	}

	insertStringList(list);

	m_listView->clearSelection();
	connect(m_listView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(accept()));
	QItemSelectionModel *selectionModel = m_listView->selectionModel();
	if(selectionModel) { // checking just to be safe
		connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
		        this, SLOT(handleSelectionChanged(const QItemSelection&,const QItemSelection&)));
	}

	enableButtonOk(false);
}

void KileListSelectorBase::handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);
	QItemSelectionModel *selectionModel = m_listView->selectionModel();
	if(selectionModel) { // checking just to be safe
		enableButtonOk(selectionModel->hasSelection());
	}
}

bool KileListSelectorBase::hasSelection()
{
	QTreeWidgetItemIterator it(m_listView, QTreeWidgetItemIterator::Selected);

	return (*it);
}

void KileListSelectorBase::insertStringList(const QStringList &list)
{
	QStringList::ConstIterator it;
	for (it = list.begin(); it != list.end(); ++it) {
		QTreeWidgetItem *item = new QTreeWidgetItem(m_listView, QStringList(*it));
		
		if(it == list.begin()) {
			m_listView->setCurrentItem(item);
		}
	}
}

//////////////////// with single selection ////////////////////

KileListSelector::KileListSelector(const QStringList &list, const QString &caption, const QString &select, bool sort, QWidget *parent, const char *name)
: KileListSelectorBase(list, caption, select, sort, parent, name)
{
	m_listView->setSelectionMode(QAbstractItemView::SingleSelection);

	if (list.count() > 0) {
		m_listView->topLevelItem(0)->setSelected(true);
	}
}

QString KileListSelector::selected()
{
	QTreeWidgetItemIterator it(m_listView, QTreeWidgetItemIterator::Selected);

	if(*it) {
		return (*it)->text(0);
	}
	else {
		return "";
	}
}

//////////////////// with multi selection ////////////////////

KileListSelectorMultiple::KileListSelectorMultiple(const QStringList &list, const QString &caption, const QString &select, bool sort, QWidget *parent, const char *name)
: KileListSelectorBase(list, caption, select, sort, parent, name)
{
	m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}


QStringList KileListSelectorMultiple::selected()
{
	QStringList toReturn;

	QTreeWidgetItemIterator it(m_listView, QTreeWidgetItemIterator::Selected);
	while (*it) {
		toReturn.append((*it)->text(0));
		++it;
	}

	return toReturn;
}

//////////////////// ManageCompletionFilesDialog ////////////////////

ManageCompletionFilesDialog::ManageCompletionFilesDialog(const QString& caption,
  const QString &localCompletionDir, const QString &globalCompletionDir, QWidget* parent, const char* name)
  : KDialog(parent), m_localCompletionDirectory(localCompletionDir), m_globalCompletionDirectory(globalCompletionDir)
{
	setObjectName(name);
	setCaption(caption);
	setModal(true);
	setButtons(Ok | User1 | User2 | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	setButtonText(Ok, i18n("Add selected files"));
	setButtonToolTip(Ok, i18n("Add all the selected files"));
	setButtonText(User1, i18n("Install custom files..."));
	setButtonToolTip(User1, i18n("Install your own completion files"));
	setButtonText(User2, i18n("Manage custom files..."));
	setButtonToolTip(User2, i18n("Manage the local completion files in the file manager"));

	m_listView = new QTreeWidget(this);
	m_listView->setHeaderLabels(QStringList() << i18n("File Name") << i18n("Local File") << i18n("Add File?"));
	m_listView->setSortingEnabled(false);
	m_listView->setSelectionMode(QAbstractItemView::NoSelection);
	m_listView->setRootIsDecorated(false);

	m_dirWatcher = new KDirWatch(this);
	if (m_dirWatcher) {
		m_dirWatcher->addDir(localCompletionDir, KDirWatch::WatchFiles);
		connect(m_dirWatcher, SIGNAL(created(QString)), this, SLOT(fillTreeView()));
		connect(m_dirWatcher, SIGNAL(deleted(QString)), this, SLOT(fillTreeView()));
	}

	connect(this, SIGNAL(user1Clicked()), this, SLOT(addCustomCompletionFiles()));
	connect(this, SIGNAL(user2Clicked()), this, SLOT(openLocalCompletionDirectoryInFileManager()));

	fillTreeView();
	setMainWidget(m_listView);
}

ManageCompletionFilesDialog::~ManageCompletionFilesDialog()
{
}

void ManageCompletionFilesDialog::fillTreeView() {
	// we want to keep selected items still selected after refreshing
	QSet<QString> previouslySelectedItems = selected();
	QStringList list = KileCodeCompletion::Manager::getAllCwlFiles(m_localCompletionDirectory, m_globalCompletionDirectory).uniqueKeys();
	qSort(list);
	m_listView->clear();
	foreach(QString filename, list) {
		QString expectedLocalPath = m_localCompletionDirectory + "/" + filename;
		QString expectedGlobalPath = m_globalCompletionDirectory + "/" + filename;
		if (QFileInfo(expectedLocalPath).exists() && QFileInfo(expectedLocalPath).isReadable()) {
			QTreeWidgetItem* item = new QTreeWidgetItem(m_listView, QStringList() << filename << i18n("yes"));
			item->setCheckState(2, previouslySelectedItems.contains(filename) ? Qt::Checked : Qt::Unchecked);
		}
		else if (QFileInfo(expectedGlobalPath).exists() && QFileInfo(expectedGlobalPath).isReadable()) {
			QTreeWidgetItem* item = new QTreeWidgetItem(m_listView, QStringList() << filename << i18n("no"));
			item->setCheckState(2, previouslySelectedItems.contains(filename) ? Qt::Checked : Qt::Unchecked);
		}
		else {
			KILE_DEBUG() << "Cannot load file" << filename << "!";
		}
	}
	m_listView->resizeColumnToContents(0);
	m_listView->resizeColumnToContents(1);
	m_listView->resizeColumnToContents(2);
}

void ManageCompletionFilesDialog::addCustomCompletionFiles()
{
	bool someFileAdded = false;
	QStringList files = KFileDialog::getOpenFileNames(KUrl(), i18n("*.cwl|Completion files (*.cwl)"), this, i18n("Select Completion Files to Install Locally"));

	if(files.isEmpty()) {
		return;
	}
	// Create local path if it doesn't exist or has been deleted in the mean time
	QDir workPath(m_localCompletionDirectory);
	if (!workPath.isReadable()) {
		workPath.mkpath(m_localCompletionDirectory);
	}

	foreach (QString file, files) {
		QFileInfo fileInf(file);
		QFileInfo localFile(m_localCompletionDirectory + "/" + fileInf.fileName());
		if (localFile.exists()) {
			const QString dialog_text = i18n("A local completion file with the name \"%1\" already exists.\nDo you want to replace this file?").arg(localFile.fileName());
			const QString dialog_caption = i18n("Replace Local File?");
			if (KMessageBox::questionYesNo(this, dialog_text, dialog_caption) == KMessageBox::Yes) {
				if (!QFile::remove(localFile.absoluteFilePath())) {
					KMessageBox::error(this, i18n("An error occurred while removing the file \"%1\".\nPlease check the file permissions.")
					  .arg(localFile.fileName()), i18n("Remove Error"));
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
			KMessageBox::error(this, i18n("Cannot copy the file to the local directory!\nPlease check the access permissions of the directory \"%1\".")
			  .arg(localFile.absolutePath()), i18n("Copy Error"));
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
		KMessageBox::information(this, i18n("The custom files have been installed and preselected for adding."), i18n("Installation Successful"));
	}
}

void ManageCompletionFilesDialog::openLocalCompletionDirectoryInFileManager()
{
	new KRun(KUrl(m_localCompletionDirectory), QApplication::activeWindow());
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

#include "listselector.moc"