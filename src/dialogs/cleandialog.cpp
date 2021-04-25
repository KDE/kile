/***************************************************************************
    date                 : Jan 22 2004
    version              : 0.10
    copyright            : (C) 2004 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/cleandialog.h"
#include "kiledebug.h"

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace KileDialog
{
Clean::Clean(QWidget *parent, const QString &filename, const QStringList &extlist)
    : QDialog(parent)
    , m_extlist(extlist)
{
    setWindowTitle(i18n("Delete Files"));
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *page = new QWidget(this);
    mainLayout->addWidget(page);

    // Layout
    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setContentsMargins(0, 0, 0, 0);
    page->setLayout(vbox);

    // label widgets
    QWidget *labelwidget = new QWidget(page);
    mainLayout->addWidget(labelwidget);
    QHBoxLayout *labellayout = new QHBoxLayout();
    labellayout->setContentsMargins(0, 0, 0, 0);
    labelwidget->setLayout(labellayout);

    // line 1: picture and label
    QLabel *picture =  new QLabel("", labelwidget);
    picture->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(KIconLoader::SizeMedium));
    QLabel *label =  new QLabel(i18n("Do you really want to delete these files?"), labelwidget);
    labellayout->addWidget(picture);
    labellayout->addSpacing(20);
    labellayout->addWidget(label);

    // line 2: m_listview
    m_listview = new QTreeWidget(page);
    mainLayout->addWidget(m_listview);
    m_listview->setHeaderLabel(i18n("Files"));
    m_listview->setSortingEnabled(false);
    m_listview->setAllColumnsShowFocus(true);
    m_listview->setRootIsDecorated(false);

    // insert items into m_listview
    QString base = QFileInfo(filename).completeBaseName();
    for(int i = 0; i <  m_extlist.count(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_listview,
                QStringList(base + m_extlist[i]));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);
    }

    vbox->addWidget(labelwidget, 0, Qt::AlignHCenter);
    vbox->addWidget(m_listview);

    // add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setDefault(true);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

Clean::~Clean()
{}

// get all selected items

QStringList Clean::cleanList()
{
    QStringList templist;

    int i = 0;
    QTreeWidgetItemIterator it(m_listview);
    while (*it) {
        if ((*it)->checkState(0) == Qt::Checked && (*it)->text(0).endsWith(m_extlist[i])) {
            templist.append(m_extlist[i]);
        }
        ++it;
        ++i;
    }

    m_extlist = templist;
    return m_extlist;
}
}
