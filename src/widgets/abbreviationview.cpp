/****************************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           2008 - 2009 by Michel Ludwig (michel.ludwig@kdemail.net)
*****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgets/abbreviationview.h"

#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QTextStream>

#include <KLocalizedString>
#include <QMenu>
#include <KMessageBox>

#include "abbreviationmanager.h"
#include "dialogs/abbreviationinputdialog.h"
#include "kiledebug.h"

namespace KileWidget {

AbbreviationView::AbbreviationView(KileAbbreviation::Manager *manager, QWidget *parent)
    : QTreeWidget(parent), m_abbreviationManager(manager)
{
    setColumnCount(2);
    QStringList headerLabelList;
    headerLabelList << i18n("Short") << QString() << i18n("Expanded Text");
    setHeaderLabels(headerLabelList);
    setAllColumnsShowFocus(true);

    header()->setSectionsMovable(false);      // default: true
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenuRequested(QPoint)));
}

AbbreviationView::~AbbreviationView()
{
}
//////////////////// init abbreviation view with wordlists ////////////////////


void AbbreviationView::updateAbbreviations()
{
    KILE_DEBUG_MAIN;
    setUpdatesEnabled(false);
    clear();
    const QMap<QString, QPair<QString, bool> >& abbreviationMap = m_abbreviationManager->getAbbreviationMap();
    QList<QTreeWidgetItem*> itemList;
    for(QMap<QString, QPair<QString, bool> >::const_iterator i = abbreviationMap.begin();
            i != abbreviationMap.end(); ++i) {
        QPair<QString, bool> pair = i.value();
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(ALVabbrev, i.key());
        item->setText(ALVlocal, (pair.second) ? QString() : "*");
        item->setText(ALVexpansion, pair.first);
        itemList.push_back(item);
    }
    addTopLevelItems(itemList);

    setUpdatesEnabled(true);
}

//////////////////// find abbreviation ////////////////////

bool AbbreviationView::findAbbreviation(const QString &abbrev)
{
    QTreeWidgetItemIterator it(this);
    while(*it) {
        QTreeWidgetItem *current = *it;
        if(current->text(AbbreviationView::ALVabbrev) == abbrev) {
            return true;
        }

        ++it;
    }
    return false;
}

//////////////////// item clicked ////////////////////

void AbbreviationView::slotItemClicked(QTreeWidgetItem *item, int /* column */)
{
    if(item) {
        QString s = item->text(AbbreviationView::ALVexpansion);
        s.replace("%n","\n");
        emit( sendText(s) );
    }
}


//////////////////// context menu ////////////////////

void AbbreviationView::slotCustomContextMenuRequested(const QPoint& p)
{
    QMenu popupMenu;
    QAction *action = new QAction(i18n("&Add"), &popupMenu);
    connect(action, SIGNAL(triggered()), this, SLOT(slotAddAbbreviation()));
    popupMenu.addAction(action);

    QList<QTreeWidgetItem*> selectedList = selectedItems();
    if(selectedList.count() > 0) {
        QTreeWidgetItem *selectedItem = selectedList.first();
        if(!selectedItem->text(ALVlocal).isEmpty()) {
            popupMenu.addSeparator();
            action = new QAction(i18n("&Edit"), &popupMenu);
            connect(action, SIGNAL(triggered()), this, SLOT(slotChangeAbbreviation()));
            popupMenu.addAction(action);
            popupMenu.addSeparator();
            action = new QAction(i18n("&Delete"), &popupMenu);
            connect(action, SIGNAL(triggered()), this, SLOT(slotDeleteAbbreviation()));
            popupMenu.addAction(action);
        }
    }


    popupMenu.exec(mapToGlobal(p));
}

void AbbreviationView::slotAddAbbreviation()
{
    KileDialog::AbbreviationInputDialog dialog(this, Q_NULLPTR, ALVadd);
    if(dialog.exec() == QDialog::Accepted) {
        QString abbrev, expansion;
        dialog.abbreviation(abbrev, expansion);
        m_abbreviationManager->updateLocalAbbreviation(abbrev, expansion);
    }
}

void AbbreviationView::slotChangeAbbreviation()
{
    QList<QTreeWidgetItem*> selectedList = selectedItems();
    if(selectedList.count() == 0) {
        return;
    }
    QTreeWidgetItem *selectedItem = selectedList.first();
    QString oldAbbreviationText = selectedItem->text(ALVabbrev);
    QString oldAbbreviationExpansion = selectedItem->text(ALVexpansion);
    KileDialog::AbbreviationInputDialog dialog(this, selectedItem, ALVedit);
    if(dialog.exec() == QDialog::Accepted) {
        QString abbrev, expansion;
        dialog.abbreviation(abbrev, expansion);
        if(oldAbbreviationText != abbrev) {
            m_abbreviationManager->removeLocalAbbreviation(oldAbbreviationText);
        }
        m_abbreviationManager->updateLocalAbbreviation(abbrev, expansion);
    }
}

void AbbreviationView::slotDeleteAbbreviation()
{
    QList<QTreeWidgetItem*> selectedList = selectedItems();
    if(selectedList.count() == 0) {
        return;
    }
    QTreeWidgetItem *item = selectedList.first();
    QString abbreviationText = item->text(ALVabbrev);
    QString abbreviationExpansion = item->text(ALVexpansion);
    QString message = i18n("Delete the abbreviation '%1'?", abbreviationText);
    if(KMessageBox::questionYesNo(this,
                                  "<center>" + message + "</center>",
                                  i18n("Delete Abbreviation") ) == KMessageBox::Yes) {
        QString s = abbreviationText + '=' + abbreviationExpansion;
    }
    m_abbreviationManager->removeLocalAbbreviation(abbreviationText);
}


}

