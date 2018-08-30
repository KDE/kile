/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERMENUDIALOG_H
#define USERMENUDIALOG_H

#include <QKeySequence>
#include <QCheckBox>

#include "kilewizard.h"
#include "kileinfo.h"

#include "dialogs/usermenu/usermenuitem.h"

#include "ui_usermenudialog_base.h"


namespace KileMenu {

class UserMenu;

class UserMenuDialog : public KileDialog::Wizard
{
    Q_OBJECT

public:
    UserMenuDialog(KConfig *config, KileInfo *ki, KileMenu::UserMenu *userMenu, const QString &xmlfile, QWidget *parent);
    ~UserMenuDialog() {}

private Q_SLOTS:
    void slotCurrentItemChanged(QTreeWidgetItem *current,QTreeWidgetItem *previous);
    void slotInsertMenuItem();
    void slotInsertSubmenu();
    void slotInsertSeparator();
    void slotDelete();
    void slotUp();
    void slotDown();

    void slotMenuentryTypeClicked();
    void slotMenuentryTextChanged(const QString &text);
    void slotUrlTextChanged(const QString &text);
    void slotIconClicked();
    void slotIconDeleteClicked();
    void slotKeySequenceChanged(const QKeySequence &seq);

    void slotSelectionStateChanged(int state);
    void slotCheckboxStateChanged(int);

    void slotNewClicked();
    void slotInstallClicked();
    void slotLoadClicked();
    void slotSaveClicked();
    void slotSaveAsClicked();
    void slotShowHelp();

    void slotCustomContextMenuRequested(const QPoint &pos);

private:
    Ui::UserMenuDialog m_UserMenuDialog;
    UserMenuTree *m_menutree;

    KileInfo *m_ki;
    KileMenu::UserMenu *m_userMenu;

    bool m_modified;
    bool m_currentXmlInstalled;
    QString m_currentXmlFile;
    void setXmlFile(const QString &filename, bool installed);

    QString m_currentIcon;
    QStringList m_listMenutypes;

    void startDialog();
    void initDialog();
    void setModified();

    bool saveClicked();
    QString saveAsClicked();

    void readMenuentryData(UserMenuItem *tem);
    void showMenuentryData(UserMenuItem *item);
    void clearMenuEntryData();
    void disableMenuEntryData();

    void setTextEntry(UserMenuItem *item);
    void setFileContentEntry(UserMenuItem *item);
    void setProgramEntry(UserMenuItem *item);
    void setSeparatorEntry(UserMenuItem *item);
    void setSubmenuEntry(UserMenuItem *item);

    void setMenuentryText(UserMenuItem *item, bool state);
    void setMenuentryType(UserMenuItem *item, bool state1, bool state2);
    void setMenuentryFileChooser(UserMenuItem *item, bool state);
    void setMenuentryFileParameter(UserMenuItem *item, bool state);
    void setMenuentryTextEdit(UserMenuItem *item, bool state);
    void setMenuentryIcon(UserMenuItem *item, bool state, const QString &icon = QString());
    void setMenuentryShortcut(UserMenuItem *item, bool state);

    void setParameterGroupbox(bool state);
    void setMenuentryCheckboxes(UserMenuItem *item, bool useInsertOutput);

    void setMenuentryIcon(const QString &icon);
    void updateTreeButtons();
    void updateDialogButtons();
    void updateAfterDelete();

    void loadXmlFile(const QString &filename, bool installed);
    bool saveCheck();
};

}

#endif
