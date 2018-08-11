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
#include "kiledebug.h"

#include <QBoxLayout>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>

#include <KLocalizedString>
#include <KMessageBox>
#include <QPushButton>
#include <QUrl>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFileDialog>

namespace KileDialog
{

//////////////////// UserHelpDialog ////////////////////

//BEGIN UserHelpDialog

UserHelpDialog::UserHelpDialog(QWidget *parent, const char *name)
    : QDialog(parent)
{
    KILE_DEBUG_MAIN << "==UserHelpDialog::UserHelpDialog()===================";

    setObjectName(name);
    setWindowTitle(i18n("Configure User Help"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QGroupBox* group = new QGroupBox(i18n("User Help"), this);
    QGridLayout *grid = new QGridLayout();
    group->setLayout(grid);
    mainLayout->addWidget(group);

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

    m_add = new QPushButton(i18n("&Add..."), actionwidget);
    m_remove = new QPushButton(i18n("&Remove"), actionwidget);
    m_addsep = new QPushButton(i18n("&Separator"), actionwidget);
    m_up = new QPushButton(i18n("Move &Up"), actionwidget);
    m_down = new QPushButton(i18n("Move &Down"), actionwidget);

    int wmax = m_add->sizeHint().width();
    int w = m_remove->sizeHint().width();
    if(w > wmax) {
        wmax = w;
    }
    w = m_addsep->sizeHint().width();
    if(w > wmax) {
        wmax = w;
    }
    w = m_up->sizeHint().width();
    if(w > wmax) {
        wmax = w;
    }
    w = m_down->sizeHint().width();
    if(w > wmax) {
        wmax = w;
    }

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

    // insert action widget
    grid->addWidget(actionwidget, 1, 1, Qt::AlignTop);

    // file
    grid->addWidget(new QLabel(i18n("File:"), group), 2, 0);
    m_fileedit = new QLineEdit(group);
    m_fileedit->setReadOnly(true);
    grid->addWidget(m_fileedit, 3, 0, 1, 2);

    // fill vbox
    mainLayout->addWidget(group);

    connect(m_menulistbox, &QListWidget::itemSelectionChanged, this, &UserHelpDialog::slotChange);
    connect(m_add, &QPushButton::clicked, this, &UserHelpDialog::slotAdd);
    connect(m_remove, &QPushButton::clicked, this, &UserHelpDialog::slotRemove);
    connect(m_addsep, &QPushButton::clicked, this, &UserHelpDialog::slotAddSep);
    connect(m_up, &QPushButton::clicked, this, &UserHelpDialog::slotUp);
    connect(m_down, &QPushButton::clicked, this, &UserHelpDialog::slotDown);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    resize(400, sizeHint().height());
    updateButton();
}

void UserHelpDialog::setParameter(const QStringList &menuentries, const QList<QUrl> &helpfiles)
{
    for (int i = 0; i < menuentries.count(); ++i) {
        m_menulistbox->addItem(menuentries[i]);

        if(m_menulistbox->item(i)->text() != "-") {
            m_filelist << helpfiles[i];
        }
        else {
            m_filelist << QUrl();
        }
    }
    updateButton();
}

void UserHelpDialog::getParameter(QStringList &userhelpmenulist, QList<QUrl> &userhelpfilelist)
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
                userhelpfilelist << QUrl();
                separator = true;
            }
        }
    }
}

void UserHelpDialog::slotChange()
{
    int index = m_menulistbox->currentRow();
    if (index >= 0) {
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
    if (dialog->exec()) {
        // with corresponding filename
        QString helpfile = dialog->getHelpfile();

        if (helpfile.isEmpty()
                || m_menulistbox->findItems(helpfile, Qt::MatchExactly).count() > 0
           ) {
            return;
        }

        m_filelist.append(QUrl::fromLocalFile(helpfile));
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
        if (entries > 0) {
            if (index >= entries) {
                index--;
            }
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
    if (index == -1) {
        return;
    }

    // insert separator
    m_menulistbox->insertItem(index, "-");
    m_filelist.insert(index, QUrl());

    updateButton();
}

void UserHelpDialog::slotUp()
{
    // get current index
    int index = m_menulistbox->currentRow();
    if (index <= 0) {
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
    if (index < 0 || index == entries - 1) {
        return;
    }

    // insert current entry after current
    if (index < entries - 2) {
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
    if (entries == 1) {
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
    : QDialog(parent)
    , m_menulistbox(menulistbox)
{
    setWindowTitle(i18n("Add User Helpfile"));
    setModal(true);

    KILE_DEBUG_MAIN << "==UserHelpAddDialog::UserHelpAddDialog()===================";

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // layout
    QGroupBox *group = new QGroupBox(i18n("User Help"), this);
    QGridLayout *grid = new QGridLayout();
    group->setLayout(grid);
    mainLayout->addWidget(group);

    // menu entry
    QLabel *label1 = new QLabel(i18n("&Menu entry:"), group);
    grid->addWidget(label1, 0, 0);
    m_leMenuEntry = new QLineEdit(group);
    m_leMenuEntry->setClearButtonEnabled(true);
    grid->addWidget(m_leMenuEntry, 0, 1, 1, 3);
    label1->setBuddy(m_leMenuEntry);

    // help file
    QLabel *label2 = new QLabel(i18n("&Help file:"), group);
    grid->addWidget(label2, 1, 0);
    m_leHelpFile = new QLineEdit(group);
    m_leHelpFile->setReadOnly(false);
    m_leHelpFile->setClearButtonEnabled(true);
    grid->addWidget(m_leHelpFile, 1, 1);
    m_pbChooseFile = new QPushButton("", group);
    m_pbChooseFile->setObjectName("filechooser_button");
    m_pbChooseFile->setIcon(QIcon::fromTheme("document-open"));
    int buttonSize = m_leHelpFile->sizeHint().height();
    m_pbChooseFile->setFixedSize(buttonSize, buttonSize);
    m_pbChooseFile->setToolTip(i18n("Open file dialog"));
    grid->addWidget(m_pbChooseFile, 1, 2);
    label2->setBuddy(m_pbChooseFile);

    // fill mainLayout
    mainLayout->addWidget(group);
    mainLayout->addStretch();

    m_leMenuEntry->setWhatsThis(i18n("The menu entry for this help file."));
    m_leHelpFile->setWhatsThis(i18n("The name of the local help file or a valid WEB url."));
    m_pbChooseFile->setWhatsThis(i18n("Start a file dialog to choose a local help file."));

    connect(m_pbChooseFile, &QPushButton::clicked, this, &UserHelpAddDialog::onShowLocalFileSelection);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setFocusProxy(m_leMenuEntry);
    resize(500, sizeHint().height());
}

void UserHelpAddDialog::onShowLocalFileSelection()
{
    QString directory = QDir::currentPath();
    QString filter = i18n("Websites (HTML) (*.html *.htm);;Documents (PDF, PS, DVI, EPUB) (*.ps *.pdf *.dvi *.epub);;All Files (*)");

    QString filename = QFileDialog::getOpenFileName(this, i18n("Select File"), directory, filter);
    if (filename.isEmpty()) {
        return;
    }

    QFileInfo fi(filename);
    if (!fi.exists()) {
        KMessageBox::error(Q_NULLPTR, i18n("File '%1' does not exist.", filename));
        return;
    }
    m_leHelpFile->setText(filename);
}

void UserHelpAddDialog::onAccepted()
{
    m_leMenuEntry->setText(m_leMenuEntry->text().trimmed());
    QString urlString = m_leHelpFile->text().trimmed();
    m_leHelpFile->setText(urlString);
}

//END UserHelpAddDialog

}
