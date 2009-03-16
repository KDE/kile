/*****************************************************************************************
                           userhelpdialog.cpp
----------------------------------------------------------------------------
    date                 : Jul 22 2005
    version              : 0.20
    copyright            : (C) 2005 by Holger Danielsson (holger.danielsson@t-online.de)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 *****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "userhelpdialog.h"

#include <QBoxLayout>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>

#include <KFileDialog>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KRun>
#include <KUrl>

#include "kiledebug.h"

namespace KileDialog
{

//////////////////// UserHelpDialog ////////////////////

//BEGIN UserHelpDialog

UserHelpDialog::UserHelpDialog(QWidget *parent, const char *name)
		: KDialog(parent)
{
	setObjectName(name);
	setCaption(i18n("Configure User Help"));
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	KILE_DEBUG() << "==UserHelpDialog::UserHelpDialog()===================";

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	// layout
	QVBoxLayout *vbox = new QVBoxLayout(page);
	QGroupBox* group = new QGroupBox(i18n("User Help"), page);

	QGridLayout *grid = new QGridLayout();
	grid->setMargin(KDialog::marginHint());
	grid->setSpacing(KDialog::spacingHint());
	group->setLayout(grid);

	// listbox
	QLabel *label1 = new QLabel(i18n("&Menu item:"), group);
	grid->addWidget(label1, 0, 0);
	m_menulistbox = new QListWidget(group);
	m_menulistbox->setSelectionMode(QAbstractItemView::SingleSelection);
	grid->addWidget(m_menulistbox, 1, 0);
	label1->setBuddy(m_menulistbox);

	// action widget
	QWidget *actionwidget = new QWidget(group);
	QVBoxLayout *actions = new QVBoxLayout(actionwidget);
	actions->setMargin(0);
	actions->setSpacing(KDialog::spacingHint());

	m_add = new KPushButton(i18n("&Add..."), actionwidget);
	m_remove = new KPushButton(i18n("&Remove"), actionwidget);
	m_addsep = new KPushButton(i18n("&Separator"), actionwidget);
	m_up = new KPushButton(i18n("Move &Up"), actionwidget);
	m_down = new KPushButton(i18n("Move &Down"), actionwidget);

	int wmax = m_add->sizeHint().width();
	int w = m_remove->sizeHint().width();
	if(w > wmax)
		wmax = w;
	w = m_addsep->sizeHint().width();
	if(w > wmax)
		wmax = w;
	w = m_up->sizeHint().width();
	if(w > wmax)
		wmax = w;
	w = m_down->sizeHint().width();
	if(w > wmax)
		wmax = w;

	m_add->setFixedWidth(wmax);
	m_remove->setFixedWidth(wmax);
	m_addsep->setFixedWidth(wmax);
	m_up->setFixedWidth(wmax);
	m_down->setFixedWidth(wmax);

	actions->addWidget(m_add);
	actions->addWidget(m_remove);
	actions->addSpacing(20);
	actions->addWidget(m_addsep);
	actions->addSpacing(20);
	actions->addWidget(m_up);
	actions->addWidget(m_down);
	actions->addStretch(1);

	// inserta ction widget
	grid->addWidget(actionwidget, 1, 1, Qt::AlignTop);

	// file
	QLabel *label2 = new QLabel(i18n("File:"), group);
	grid->addWidget(label2, 2, 0);
	m_fileedit = new KLineEdit("", group);
	m_fileedit->setReadOnly(true);
	grid->addWidget(m_fileedit, 3, 0, 1, 2);

	// fill vbox
	vbox->addWidget(group);

	connect(m_menulistbox, SIGNAL(itemSelectionChanged()), this, SLOT(slotChange()));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAdd()));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemove()));
	connect(m_addsep, SIGNAL(clicked()), SLOT(slotAddSep()));
	connect(m_up, SIGNAL(clicked()), SLOT(slotUp()));
	connect(m_down, SIGNAL(clicked()), SLOT(slotDown()));

	resize(400, sizeHint().height());
	updateButton();
}

void UserHelpDialog::setParameter(const QStringList &menuentries, const QList<KUrl> &helpfiles)
{
	for (int i = 0; i < menuentries.count(); ++i) {
		m_menulistbox->addItem(menuentries[i]);

		if(m_menulistbox->item(i)->text() != "-") {
			m_filelist << helpfiles[i];
		}
		else {
			m_filelist << KUrl();
		}
	}
	updateButton();
}

void UserHelpDialog::getParameter(QStringList &userhelpmenulist, QList<KUrl> &userhelpfilelist)
{
	// clear result
	userhelpmenulist.clear();
	userhelpfilelist.clear();
	bool separator = false;

	// now get all entries
	for (int i = 0; i < m_menulistbox->count(); ++i) {
		if(m_menulistbox->item(i)->text() != "-") {
			userhelpmenulist << m_menulistbox->item(i)->text();
			userhelpfilelist << m_filelist[i];
			separator = false;
		}
		else {
			if(!separator) {
				userhelpmenulist << m_menulistbox->item(i)->text();
				userhelpfilelist << QString();
				separator = true;
			}
		}
	}
}

void UserHelpDialog::slotChange()
{
	int index = m_menulistbox->currentRow();
	if(index >= 0) {
		m_fileedit->setText(m_filelist[index].url());
	}
	else {
		m_fileedit->clear();
	}
	updateButton();
}

void UserHelpDialog::slotAdd()
{
	KileDialog::UserHelpAddDialog *dialog = new KileDialog::UserHelpAddDialog(m_menulistbox, this);
	if(dialog->exec()) {
		// with corresponding filename
		QString helpfile = dialog->getHelpfile();
		m_filelist.append(helpfile);
		m_fileedit->setText(helpfile);

		// insert into listbox
		m_menulistbox->addItem(dialog->getMenuitem());
		m_menulistbox->setCurrentRow(m_menulistbox->count() - 1);

		updateButton();
	}
}

void UserHelpDialog::slotRemove()
{
	// get current index
	int index = m_menulistbox->currentRow();
	if(index >= 0) {
		// remove item
		m_menulistbox->takeItem(index);
		m_filelist.removeAt(index);

		// select a new index: first we try to take the old index. When
		// this index is too big now, index is decremented.
		// If the list is empty now, index is set to -1.
		int entries = m_menulistbox->count();
		if(entries > 0) {
			if(index >= entries)
				index--;
			m_menulistbox->setCurrentRow(index);
		}
		else {
			m_menulistbox->setCurrentItem(0);
		}
	}

	updateButton();
}

void UserHelpDialog::slotAddSep()
{
	// get current index
	int index = m_menulistbox->currentRow();
	if(index == -1) {
		return;
	}

	// insert separator
	m_menulistbox->insertItem(index, "-");
	m_filelist.insert(index, QString());

	updateButton();
}

void UserHelpDialog::slotUp()
{
	// get current index
	int index = m_menulistbox->currentRow();
	if(index <= 0) {
		return;
	}

	// insert current entry before current
	m_menulistbox->insertItem(index - 1, m_menulistbox->currentItem()->text());
	m_filelist.insert(index - 1, m_filelist[index]);

	// then remove the old entry
	m_menulistbox->takeItem(index + 1);
	m_filelist.removeAt(index + 1);

	// select current entry
	m_menulistbox->setCurrentRow(index - 1);

	updateButton();
}

void UserHelpDialog::slotDown()
{
	int entries = m_menulistbox->count();

	// get current index
	int index = m_menulistbox->currentRow();
	if(index < 0 || index == entries - 1) {
		return;
	}

	// insert current entry after current
	if(index < entries - 2) {
		m_menulistbox->insertItem(index + 2, m_menulistbox->currentItem()->text());    // index + 2
		m_filelist.insert(index + 2, m_filelist[index]);
	}
	else {
		m_menulistbox->addItem(m_menulistbox->currentItem()->text());
		m_filelist.append(m_filelist[index]);
	}

	// then remove the old entry
	m_menulistbox->takeItem(index);
	m_filelist.removeAt(index);

	// select current entry
	m_menulistbox->setCurrentRow(index + 1);

	updateButton();
}

void UserHelpDialog::updateButton()
{
	// default states
	bool rem_state = false;
	bool sep_state = false;
	bool up_state = false;
	bool down_state = false;

	// change button states, if there are entries
	int index = m_menulistbox->currentRow();
	int entries = m_menulistbox->count();
	if(entries == 1) {
		rem_state = true;
	}
	else {
		if(entries >= 2) {
			rem_state = true;
			if(index == 0) {
				down_state = true;         // index = 0
			}
			else {
				if(index == entries - 1) {
					sep_state = true;          // index = entries-1
					up_state = true;
				}
				else {                             // 0 < index < entries-1
					sep_state = true;
					up_state = true;
					down_state = true;
				}
			}
		}
	}
	// don't allow two continuous spearators
	if(m_menulistbox->currentItem() && m_menulistbox->currentItem()->text() == "-") {
		sep_state = false;
	}

	// set button states
	m_remove->setEnabled(rem_state);
	m_addsep->setEnabled(sep_state);
	m_up->setEnabled(up_state);
	m_down->setEnabled(down_state);
}
//END UserHelpDialog

//////////////////// UserHelpAddDialog ////////////////////

//BEGIN UserHelpAddDialog

UserHelpAddDialog::UserHelpAddDialog(QListWidget *menulistbox, QWidget *parent)
		: KDialog(parent), m_menulistbox(menulistbox)
{
	setCaption(i18n("Add User Helpfile"));
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	KILE_DEBUG() << "==UserHelpAddDialog::UserHelpAddDialog()===================";

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	// layout
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
	vbox->setSpacing(KDialog::spacingHint());
	page->setLayout(vbox);
	QGroupBox* group = new QGroupBox(i18n("User Help"), page);

	QGridLayout *grid = new QGridLayout();
	grid->setMargin(KDialog::marginHint());
	grid->setSpacing(KDialog::spacingHint());
	group->setLayout(grid);

	// menu entry
	QLabel *label1 = new QLabel(i18n("&Menu entry:"), group);
	grid->addWidget(label1, 0, 0);
	m_leMenuEntry = new KLineEdit("", group);
	m_leMenuEntry->setClearButtonShown(true);
	grid->addWidget(m_leMenuEntry, 0, 1, 1, 3);
	label1->setBuddy(m_leMenuEntry);

	// help file
	QLabel *label2 = new QLabel(i18n("&Help file:"), group);
	grid->addWidget(label2, 1, 0);
	m_leHelpFile = new KLineEdit("", group);
	m_leHelpFile->setReadOnly(false);
	m_leHelpFile->setClearButtonShown(true);
	grid->addWidget(m_leHelpFile, 1, 1);
	m_pbChooseFile = new KPushButton("", group);
	m_pbChooseFile->setObjectName("filechooser_button");
	m_pbChooseFile->setIcon(KIcon("document-open"));
	int buttonSize = m_leHelpFile->sizeHint().height();
	m_pbChooseFile->setFixedSize(buttonSize, buttonSize);
	m_pbChooseFile->setToolTip(i18n("Open file dialog"));
	grid->addWidget(m_pbChooseFile, 1, 2);
	m_pbChooseHtml = new KPushButton("", group);
	m_pbChooseHtml->setObjectName("htmlchooser_button");
	m_pbChooseHtml->setIcon(KIcon("document-open-remote"));
	m_pbChooseHtml->setFixedSize(buttonSize, buttonSize);
	grid->addWidget(m_pbChooseHtml, 1, 3);

	label2->setBuddy(m_pbChooseFile);

	// fill vbox
	vbox->addWidget(group);
	vbox->addStretch();

	m_leMenuEntry->setWhatsThis(i18n("The menu entry for this help file."));
	m_leHelpFile->setWhatsThis(i18n("The name of the local help file or a valid WEB url."));
	m_pbChooseFile->setWhatsThis(i18n("Start a file dialog to choose a local help file."));
	m_pbChooseHtml->setWhatsThis(i18n("Start the konqueror to choose a WEB url as help file. This url should be copied into the edit widget."));

	connect(m_pbChooseFile, SIGNAL(clicked()), this, SLOT(slotChooseFile()));
	connect(m_pbChooseHtml, SIGNAL(clicked()), this, SLOT(slotChooseHtml()));

	setFocusProxy(m_leMenuEntry);
	resize(500, sizeHint().height());
}

void UserHelpAddDialog::slotChooseFile()
{
	QString directory = QDir::currentPath();
	QString filter = "*.*|All Files\n*.dvi|DVI Files\n*.ps|PS Files\n*.pdf|PDF Files\n*.html *.htm|HTML Files";

	QString filename = KFileDialog::getOpenFileName(directory, filter, this, i18n("Select File"));
	if(filename.isEmpty())
		return;

	QFileInfo fi(filename);
	if(! fi.exists())
	{
		KMessageBox::error(0, i18n("File '%1' does not exist.", filename));
		return;
	}

	m_leHelpFile->setText(filename);
}

void UserHelpAddDialog::slotChooseHtml()
{
	KUrl url;
	url.setPath("about:blank");
	KRun::runUrl(url, "text/html", this);
}

void UserHelpAddDialog::slotButtonClicked(int button)
{
	if(button != KDialog::Ok) {
		KDialog::slotButtonClicked(button);
		return;
	}
	m_leMenuEntry->setText(m_leMenuEntry->text().trimmed());
	QString urlString = m_leHelpFile->text().trimmed();
	m_leHelpFile->setText(urlString);
	KUrl url(urlString);

	if(m_leMenuEntry->text().isEmpty()) {
		KMessageBox::error(this, i18n("No menu item was given."));
		return;
	}

	if(m_menulistbox->findItems(m_leMenuEntry->text(), Qt::MatchExactly).count() > 0) {
		KMessageBox::error(this, i18n("This menu item exists already."));
		return;
	}

	if(urlString.isEmpty()) {
		KMessageBox::error(this, i18n("No help file was chosen."));
		return;
	}

	QFileInfo fi(url.toLocalFile());
	if(url.isLocalFile() && !QFileInfo(url.toLocalFile()).exists()) {
		KMessageBox::error(this, i18n("The file '%1' does not exist.", url.prettyUrl()));
		return;
	}

	accept();
}

//END UserHelpAddDialog

}

#include "userhelpdialog.moc"

