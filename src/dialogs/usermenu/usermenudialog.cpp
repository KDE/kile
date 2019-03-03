/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2018 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KIconDialog>
#include <KMessageBox>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>
#include <KConfigGroup>

#include "dialogs/usermenu/usermenudialog.h"
#include "dialogs/usermenu/usermenutree.h"
#include "dialogs/usermenu/usermenuitem.h"
#include "usermenu/usermenu.h"

#include "kiledebug.h"
#include "utilities.h"

namespace KileMenu {

#define CHOOSABLE_MENUTYPES   3

UserMenuDialog::UserMenuDialog(KConfig *config, KileInfo *ki, KileMenu::UserMenu *userMenu, const QString &xmlfile, QWidget *parent)
    : KileDialog::Wizard(config, parent, Q_NULLPTR, i18n("Edit User Menu"))
    , m_ki(ki)
    , m_userMenu(userMenu)
{
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_UserMenuDialog.setupUi(page);

    m_menutree = m_UserMenuDialog.m_twUserMenu;
    m_menutree->setHeaderLabels(QStringList() << i18n("Menu Entry") << i18n("Shortcut"));

    // Indexes must be identical to MenuType. Only the first three of them are choosable (see CHOOSABLE_MENUTYPES)
    m_listMenutypes << i18n("Text") << i18n("Insert file contents") << i18n("Execute program") << i18n("Separator") << i18n("Submenu");

    // some text
    m_UserMenuDialog.m_teText->setWhatsThis(i18n("Text, which will be inserted, if the action is executed. Some placeholders are available: <ul><li>%M - selected (marked) text</li><li>%C - cursor position</li><li>%B - bullet</li><li>%E - indentation in environments</li><li>%R - select label from list</li><li>%T - select citation key from list</li></ul>"));
    m_UserMenuDialog.m_teText->setToolTip(i18n("Available placeholders:\n%M: Selected (marked) text\n%C: Cursor position\n%B: Bullet\n%E: Indentation in environments\n%R: Select label from list\n%T: Select citation key from list\n%S: Source file name without extension"));

    // search for all action collections (needed for shortcut conflicts)
    QList<KActionCollection *> allCollections;
    foreach (KXMLGUIClient *client, m_ki->mainWindow()->guiFactory()->clients()) {
        KILE_DEBUG_MAIN << "collection count: " << client->actionCollection()->count() ;
        allCollections += client->actionCollection();
    }
    m_UserMenuDialog.m_keyChooser->setCheckActionCollections(allCollections);
    KILE_DEBUG_MAIN << "total collections: " << allCollections.count();

    m_UserMenuDialog.m_pbInsertBelow->setIcon(QIcon::fromTheme("usermenu-insert-below.png"));
    m_UserMenuDialog.m_pbInsertSubmenu->setIcon(QIcon::fromTheme("usermenu-submenu-below.png"));
    m_UserMenuDialog.m_pbInsertSeparator->setIcon(QIcon::fromTheme("usermenu-separator-below.png"));
    m_UserMenuDialog.m_pbDelete->setIcon(QIcon::fromTheme("usermenu-delete.png"));
    m_UserMenuDialog.m_pbUp->setIcon(QIcon::fromTheme("usermenu-up.png"));
    m_UserMenuDialog.m_pbDown->setIcon(QIcon::fromTheme("usermenu-down.png"));
    m_UserMenuDialog.m_pbIconDelete->setIcon(QIcon::fromTheme("edit-clear-locationbar-rtl.png"));

    connect(m_UserMenuDialog.m_pbInsertBelow, &QPushButton::clicked, this, &UserMenuDialog::slotInsertMenuItem);
    connect(m_UserMenuDialog.m_pbInsertSubmenu, &QPushButton::clicked, this, &UserMenuDialog::slotInsertSubmenu);
    connect(m_UserMenuDialog.m_pbInsertSeparator, &QPushButton::clicked, this, &UserMenuDialog::slotInsertSeparator);
    connect(m_UserMenuDialog.m_pbUp, &QPushButton::clicked, this, &UserMenuDialog::slotUp);
    connect(m_UserMenuDialog.m_pbDown, &QPushButton::clicked, this, &UserMenuDialog::slotDown);
    connect(m_UserMenuDialog.m_pbDelete, &QPushButton::clicked, this, &UserMenuDialog::slotDelete);

    connect(m_menutree, &QTreeWidget::currentItemChanged, this, &UserMenuDialog::slotCurrentItemChanged);

    connect(m_UserMenuDialog.m_pbMenuentryType, &QPushButton::clicked, this, &UserMenuDialog::slotMenuentryTypeClicked);
    connect(m_UserMenuDialog.m_leMenuEntry, &KLineEdit::textEdited, this, &UserMenuDialog::slotMenuentryTextChanged);
    connect(m_UserMenuDialog.m_urlRequester, &KUrlRequester::textChanged, this, &UserMenuDialog::slotUrlTextChanged);
    connect(m_UserMenuDialog.m_urlRequester, &KUrlRequester::urlSelected, this, [=]() {
        setModified();
    });
    connect(m_UserMenuDialog.m_leParameter, &KLineEdit::textEdited, this, [=]() {
        setModified();
    });
    connect(m_UserMenuDialog.m_teText, &QPlainTextEdit::textChanged, this, [=]() {
        setModified();
    });
    connect(m_UserMenuDialog.m_pbIcon, &QPushButton::clicked, this, &UserMenuDialog::slotIconClicked);
    connect(m_UserMenuDialog.m_pbIconDelete, &QPushButton::clicked, this, &UserMenuDialog::slotIconDeleteClicked);
    connect(m_UserMenuDialog.m_keyChooser, &KKeySequenceWidget::keySequenceChanged, this, &UserMenuDialog::slotKeySequenceChanged);

    connect(m_UserMenuDialog.m_cbNeedsSelection,   &QCheckBox::stateChanged, this, &UserMenuDialog::slotSelectionStateChanged);
    connect(m_UserMenuDialog.m_cbContextMenu,      &QCheckBox::stateChanged, this, &UserMenuDialog::slotCheckboxStateChanged);
    connect(m_UserMenuDialog.m_cbReplaceSelection, &QCheckBox::stateChanged, this, &UserMenuDialog::slotCheckboxStateChanged);
    connect(m_UserMenuDialog.m_cbSelectInsertion,  &QCheckBox::stateChanged, this, &UserMenuDialog::slotCheckboxStateChanged);
    connect(m_UserMenuDialog.m_cbInsertOutput,     &QCheckBox::stateChanged, this, &UserMenuDialog::slotCheckboxStateChanged);

    connect(m_UserMenuDialog.m_pbInstall, &QPushButton::clicked, this, &UserMenuDialog::slotInstallClicked);
    connect(m_UserMenuDialog.m_pbNew,     &QPushButton::clicked, this, &UserMenuDialog::slotNewClicked);

    connect(m_UserMenuDialog.m_pbLoad,   &QPushButton::clicked, this, &UserMenuDialog::slotLoadClicked);
    connect(m_UserMenuDialog.m_pbSave,   &QPushButton::clicked, this, &UserMenuDialog::slotSaveClicked);
    connect(m_UserMenuDialog.m_pbSaveAs, &QPushButton::clicked, this, &UserMenuDialog::slotSaveAsClicked);

    // set context menu handler for the menutree
    m_menutree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_menutree, &QWidget::customContextMenuRequested, this, &UserMenuDialog::slotCustomContextMenuRequested);

    // adjust some widths
    const int w = m_UserMenuDialog.m_pbInsertBelow->sizeHint().width();
    m_UserMenuDialog.m_pbUp->setMinimumWidth(w);
    m_UserMenuDialog.m_pbDown->setMinimumWidth(w);
    m_UserMenuDialog.m_lbIconChosen->setMinimumWidth(m_UserMenuDialog.m_pbIcon->sizeHint().width());

    setFocusProxy(m_menutree);
    setModal(false);

    // TODO: currently all dialog actions are in an extra widget
    // for usability some of those actions should be connect to the main dialog controls,
    // e.g. automatic install when pressing OK
    buttonBox()->clear();
    buttonBox()->addButton(QDialogButtonBox::Close);
    buttonBox()->addButton(QDialogButtonBox::Help);

    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox());
    connect(buttonBox(), &QDialogButtonBox::helpRequested, this, &UserMenuDialog::slotShowHelp);

    KILE_DEBUG_MAIN << "start dialog with xmfile " << xmlfile;

    if (!xmlfile.isEmpty() && QFile::exists(xmlfile)) {
        m_modified = false;
        loadXmlFile(xmlfile,true);
    }
    else {
        startDialog();
    }

    resize(minimumSize());
}

void UserMenuDialog::startDialog()
{
    initDialog();

    m_modified = false;
    setXmlFile(QString(), false);
    updateDialogButtons();
    m_UserMenuDialog.m_pbNew->setEnabled(false);
}

void UserMenuDialog::initDialog()
{
    updateTreeButtons();

    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current) {
        m_menutree->setCurrentItem(current);
    }

    // init first entry
    m_currentIcon.clear();
    showMenuentryData(dynamic_cast<UserMenuItem *>(current));
}

void UserMenuDialog::setXmlFile(const QString &filename, bool installed)
{
    m_currentXmlInstalled = installed;
    m_currentXmlFile = filename;
    m_UserMenuDialog.m_lbXmlFile->setText(i18n("File:") + QLatin1String("   ") + QFileInfo(m_currentXmlFile).fileName());
    if (m_currentXmlInstalled) {
        m_UserMenuDialog.m_lbXmlInstalled->show();
    } else {
        m_UserMenuDialog.m_lbXmlInstalled->hide();
    }
}

void UserMenuDialog::setModified()
{
    if (!m_modified) {
        m_modified = true;
    }

    updateDialogButtons();
}

void UserMenuDialog::updateDialogButtons()
{
    bool installedFile = (!m_currentXmlFile.isEmpty());
    bool menutreeState = !m_menutree->isEmpty();

    bool installState = !m_modified && installedFile && !m_currentXmlInstalled;
    bool saveState = m_modified && installedFile;
    bool saveAsState = m_modified || (!m_modified && installedFile && m_currentXmlInstalled);

    m_UserMenuDialog.m_pbInstall->setEnabled(installState && menutreeState);
    m_UserMenuDialog.m_pbSave->setEnabled(saveState && menutreeState);
    m_UserMenuDialog.m_pbSaveAs->setEnabled(saveAsState && menutreeState);
    m_UserMenuDialog.m_pbNew->setEnabled(true);
}

void UserMenuDialog::slotShowHelp()
{
    const QString message = i18n("<p>You can create, change and install a user-defined menu, which will appear as a part of Kile's menu. "
                                 "To create or change this menu, use the six buttons on the left side. "
                                 "Even more possible actions are available in the context menu of already existing menu items.</p>"
                                 "<p>Like a standard menu, three different kinds of menu items are available:</p>"
                                 "<ul>"
                                 "<li><i>standard entries</i>, which are assigned to an action</li>"
                                 "<li><i>submenus</i>, which contain more menu items</li>"
                                 "<li><i>separators</i>, to get a visible structure of all entries</li>"
                                 "</ul>"
                                 "<p>Each standard menu item is assigned to one of three action types:</p>"
                                 "<ul>"
                                 "<li><i>insert text</i>: this action will insert your text at the current cursor position. "
                                 "Some metachars are available: <tt>%M</tt>, <tt>%C</tt>, <tt>%B</tt>, <tt>%E</tt>, <tt>%R</tt>, <tt>%T</tt>, <tt>%S</tt>: "
                                 "see the <i>What's This</i> or <i>Tool Tip</i> feature of this widget to get more information.</li>"
                                 "<li><i>file content</i>: inserts the complete contents of a given file (metachars are also available)</li>"
                                 "<li><i>run an external program</i>: The output of this program can be inserted into the opened document. "
                                 "Metachar <tt>%M</tt> is also possible in the commandline of this program, as the selected text will be saved in a temporary file. "
                                 "Use <tt>%M</tt> for the filename of this temporary file.</li>"
                                 "</ul>"
                                 "<p>If some  important information for an action is missing, menu items are colored red. "
                                 "More information is available using the <i>What's this</i> feature of most widgets.</p>");

    KMessageBox::information(this, message, i18n("UserMenu Dialog"));
}


///////////////////////////// Button slots (Install/New) //////////////////////////////

void UserMenuDialog::slotInstallClicked()
{
    KILE_DEBUG_MAIN << "install " << m_currentXmlFile << "...";

    if (!m_modified && !m_currentXmlFile.isEmpty()) {
        m_userMenu->installXmlFile(m_currentXmlFile);
        setXmlFile(m_currentXmlFile,true);
        updateDialogButtons();
    }
}

void UserMenuDialog::slotNewClicked()
{
    KILE_DEBUG_MAIN << "start new menutree ... ";

    if (!m_menutree->isEmpty() && m_modified) {
        if (KMessageBox::questionYesNo(this, i18n("Current menu tree was modified, but not saved.\nDiscard this tree?")) == KMessageBox::No) {
            return;
        }
    }

    m_menutree->clear();
    m_modified = false;
    startDialog();   // includes updating of buttons
}


///////////////////////////// Button slots (Load) //////////////////////////////

void UserMenuDialog::slotLoadClicked()
{
    KILE_DEBUG_MAIN << "load xml file ";

    if (!m_menutree->isEmpty() && m_modified) {
        if (KMessageBox::questionYesNo(this, i18n("Current menu tree was modified, but not saved.\nDiscard this tree?")) == KMessageBox::No) {
            return;
        }
    }

    QString directory = UserMenu::selectUserMenuDir();
    QString filter = i18n("User Menu Files (*.xml)");

    QString filename = QFileDialog::getOpenFileName(this, i18n("Select Menu File"), directory, filter);
    if (filename.isEmpty()) {
        return;
    }

    if (QFile::exists(filename)) {
        loadXmlFile(filename,false); // includes updating of buttons
    }
    else {
        KMessageBox::error(this, i18n("File '%1' does not exist.", filename));
    }
}

void UserMenuDialog::loadXmlFile(const QString &filename, bool installed)
{
    KILE_DEBUG_MAIN << "load xml started ...";
    m_menutree->readXml(filename);
    initDialog();
    m_modified = false;
    setXmlFile(filename,installed);
    updateDialogButtons();
    KILE_DEBUG_MAIN << "load xml finished ...";
}

///////////////////////////// Button slots (Save) //////////////////////////////

void UserMenuDialog::slotSaveClicked()
{
    if (saveClicked()) {
        m_modified = false;
        if (m_currentXmlInstalled) {
            slotInstallClicked();   // includes all updates
        }
        else {
            setXmlFile(m_currentXmlFile,false);
        }
        updateDialogButtons();
    }
}

bool UserMenuDialog::saveClicked()
{
    if (m_currentXmlFile.isEmpty()) {
        return false;
    }
    KILE_DEBUG_MAIN << "save menutree: " << m_currentXmlFile;

    // read current entry
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current) {
        KILE_DEBUG_MAIN << "read current item ...";
        readMenuentryData( dynamic_cast<UserMenuItem *>(current) );
    }

    if (saveCheck() == false) {
        return false;
    }

    // force to save file in local directory
    QStringList dirs = KileUtilities::locateAll(QStandardPaths::AppDataLocation, "usermenu", QStandardPaths::LocateDirectory);
    if (dirs.size() > 1) {
        if (m_currentXmlFile.startsWith(dirs[1])) {
            m_currentXmlFile.replace(dirs[1],dirs[0]);
            KILE_DEBUG_MAIN << "change filename to local directory:" << m_currentXmlFile;
        }
    }

    // save file
    m_menutree->writeXml(m_currentXmlFile);
    return true;
}

void UserMenuDialog::slotSaveAsClicked()
{
    const QString filename = saveAsClicked();
    if (!filename.isEmpty()) {
        // set new state: current file is not installed anymore
        m_modified = false;
        setXmlFile(filename,false);
        updateDialogButtons();
    }
}

QString UserMenuDialog::saveAsClicked()
{
    KILE_DEBUG_MAIN << "menutree should be saved as ...";

    // read current entry
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current) {
        KILE_DEBUG_MAIN << "read current item ...";
        readMenuentryData(dynamic_cast<UserMenuItem *>(current));
    }

    if (saveCheck() == false) {
        return QString();
    }

    const QString directory = KileUtilities::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + "usermenu/";
    const QString filter = i18n("User Menu Files (*.xml)");

    QString filename = QFileDialog::getSaveFileName(this, i18n("Save Menu File"), directory, filter);
    if (filename.isEmpty()) {
        return QString();
    }

    if (QFile::exists(filename)) {
        if (KMessageBox::questionYesNo(this, i18n("File '%1' does already exist.\nOverwrite this file?", filename)) == KMessageBox::No) {
            return QString();
        }
    }

    // save file
    m_menutree->writeXml(filename);
    return filename;
}

bool UserMenuDialog::saveCheck()
{
    if (m_menutree->errorCheck() == false) {
        if (KMessageBox::questionYesNo(this, i18n("The menu tree contains some errors and installing this file may lead to unpredictable results.\nDo you really want to save this file?")) == KMessageBox::No) {
            return false;
        }
    }

    return true;
}

///////////////////////////// Button slots (left widget) //////////////////////////////

void UserMenuDialog::slotCustomContextMenuRequested(const QPoint &pos)
{
    m_menutree->contextMenuRequested(pos);
    updateAfterDelete();
}

void UserMenuDialog::slotInsertMenuItem()
{
    if (m_menutree->insertMenuItem(m_menutree->currentItem())) {
        updateTreeButtons();
        setModified();
    }
}

void UserMenuDialog::slotInsertSubmenu()
{
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current && m_menutree->insertSubmenu(current)) {
        updateTreeButtons();
        setModified();
    }
}

void UserMenuDialog::slotInsertSeparator()
{
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current && m_menutree->insertSeparator(current)) {
        updateTreeButtons();
        setModified();
    }
}

void UserMenuDialog::slotDelete()
{
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current) {
        m_menutree->itemDelete(current);
        updateAfterDelete();
    }
}

void UserMenuDialog::slotUp()
{
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current) {
        m_menutree->itemUp();
        updateTreeButtons();
        setModified();
    }
}

void UserMenuDialog::slotDown()
{
    QTreeWidgetItem *current = m_menutree->currentItem();
    if (current) {
        m_menutree->itemDown();
        updateTreeButtons();
        setModified();
    }
}

void UserMenuDialog::updateTreeButtons()
{
    UserMenuItem *current = dynamic_cast<UserMenuItem *>(m_menutree->currentItem());
    if (current) {
        bool state = (current->menutype() == UserMenuData::Separator) ? false : true;
        m_UserMenuDialog.m_pbInsertSeparator->setEnabled(state);
        m_UserMenuDialog.m_pbDelete->setEnabled(true);

        bool upstate = (m_menutree->indexOfTopLevelItem(current) == 0) ? false : true;
        m_UserMenuDialog.m_pbUp->setEnabled(upstate);

        bool downstate = (m_menutree->itemBelow(current)) ? true : false;
        if (!downstate && current->parent()) {
            downstate = true;
        }
        m_UserMenuDialog.m_pbDown->setEnabled(downstate);
    }
    else {
        m_UserMenuDialog.m_pbInsertSeparator->setEnabled(false);
        m_UserMenuDialog.m_pbDelete->setEnabled(false);
        m_UserMenuDialog.m_pbUp->setEnabled(false);
        m_UserMenuDialog.m_pbDown->setEnabled(false);
    }
}

void UserMenuDialog::updateAfterDelete()
{
    if (m_menutree->isEmpty()) {
        initDialog();
    }

    updateTreeButtons();
    setModified();

}

////////////////////////////// TreeWidget slots (left widget)  //////////////////////////////

void UserMenuDialog::slotCurrentItemChanged(QTreeWidgetItem *current,QTreeWidgetItem *previous)
{
    QString from = (previous) ? previous->text(0) : "---";
    QString to   = (current)  ? current->text(0)  : "---";

    KILE_DEBUG_MAIN << "currentItemChanged: from=" << from << "  to=" << to;
    bool modifiedState = m_modified;
    bool installState = m_UserMenuDialog.m_pbInstall->isEnabled();
    bool saveState = m_UserMenuDialog.m_pbSave->isEnabled();
    bool saveAsState = m_UserMenuDialog.m_pbSaveAs->isEnabled();

    // read old data
    readMenuentryData(dynamic_cast<UserMenuItem *>(previous));

    // set new data
    showMenuentryData(dynamic_cast<UserMenuItem *>(current));

    // update buttons for treewidget
    updateTreeButtons();

    // restore saved states
    m_modified = modifiedState;
    m_UserMenuDialog.m_pbInstall->setEnabled(installState);
    m_UserMenuDialog.m_pbSave->setEnabled(saveState);
    m_UserMenuDialog.m_pbSaveAs->setEnabled(saveAsState);
}

//////////////////////////////  MenuentryType slots (right widget) //////////////////////////////

void UserMenuDialog::slotMenuentryTypeClicked()
{
    UserMenuItem *current = dynamic_cast<UserMenuItem *>(m_menutree->currentItem());
    if (!current) {
        return;
    }

    KILE_DEBUG_MAIN << "change menu item type of current item: " << current->text(0);
    QStringList typelist;
    for (int i = 0; i < CHOOSABLE_MENUTYPES; ++i) {
        typelist << m_listMenutypes[i];
    }

    int oldtype = current->menutype();
    bool ok = false;

    QString item = QInputDialog::getItem(this, i18n("Menutype"), i18n("Please choose a menutype"),
                                         typelist, oldtype, false, &ok);
    if (!ok) {
        return;
    }

    int newtype = m_listMenutypes.indexOf(item);
    if (newtype == -1 || newtype == oldtype) {
        return;
    }

    // set new values
    current->setMenutype( UserMenuData::MenuType(newtype) );
    m_UserMenuDialog.m_lbMenuentryType->setText(item);
    if (newtype == UserMenuData::Text) {
        setMenuentryFileChooser(current,false);
        setMenuentryFileParameter(current,false);
        setMenuentryTextEdit(current,true);
        setMenuentryCheckboxes(current,false);
    }
    else if (newtype == UserMenuData::FileContent) {
        setMenuentryFileChooser(current,true);
        setMenuentryFileParameter(current,false);
        setMenuentryTextEdit(current,false);
        setMenuentryCheckboxes(current,false);
    }
    else { /* if (newtype == UserMenuData::Program) */
        setMenuentryFileChooser(current,true);
        setMenuentryFileParameter(current,true);
        setMenuentryTextEdit(current,false);
        setMenuentryCheckboxes(current,true);
    }

    setModified();
}

////////////////////////////// Menuentry slot (right widget) //////////////////////////////

void UserMenuDialog::slotMenuentryTextChanged(const QString &text)
{
    UserMenuItem *current = dynamic_cast<UserMenuItem *>( m_menutree->currentItem() );
    if (current) {
        current->setText(0,text);
    }
    setModified();
}

////////////////////////////// KUrlRequester slots (right widget) //////////////////////////////

void UserMenuDialog::slotUrlTextChanged(const QString &)
{
    UserMenuItem *current = dynamic_cast<UserMenuItem *>(m_menutree->currentItem());
    if (!current) {
        return;
    }

    QString file = m_UserMenuDialog.m_urlRequester->text().trimmed();

    QString color = "black";
    int type = current->menutype();
    if (type == UserMenuData::FileContent) {
        if (file.isEmpty() || !QFile::exists(file)) {
            color = "red";
        }
    }
    else if (type == UserMenuData::Program) {
        if (file.isEmpty() || !m_menutree->isItemExecutable(file)) {
            color= "red";
        }
    }

    m_UserMenuDialog.m_urlRequester->setStyleSheet( "QLineEdit { color: " + color + "; }" );
    setModified();
}

////////////////////////////// Icon slots (right widget) //////////////////////////////

void  UserMenuDialog::slotIconClicked()
{
    QString iconname = KIconDialog::getIcon(KIconLoader::Small, KIconLoader::Any,true);
    if (iconname!=m_currentIcon && !iconname.isEmpty()) {
        QString iconpath = KIconLoader::global()->iconPath(iconname,KIconLoader::Small);
        KILE_DEBUG_MAIN << "icon changed: " << iconname << " path=" << iconpath;
        m_currentIcon = iconpath;
        setMenuentryIcon(m_currentIcon);
        setModified();
    }
}

void  UserMenuDialog::slotIconDeleteClicked()
{
    m_currentIcon.clear();
    setMenuentryIcon(m_currentIcon);
    setModified();
}

void UserMenuDialog::setMenuentryIcon(const QString &icon)
{
    UserMenuItem *current = dynamic_cast<UserMenuItem *>(m_menutree->currentItem());
    if (current) {
        if (icon.isEmpty()) {
            current->setIcon(0, QIcon::fromTheme(QString()));
        } else {
            current->setIcon(0, QIcon::fromTheme(icon));
        }
        current->setMenuicon(icon);

        // update icon widgets
        setMenuentryIcon(current,true,icon);
        setModified();
    }
}

////////////////////////////// Shortcut slots (right widget) //////////////////////////////

void UserMenuDialog::slotKeySequenceChanged(const QKeySequence &seq)
{
    QString shortcut = seq.toString(QKeySequence::NativeText);
    KILE_DEBUG_MAIN << "key sequence changed: " << shortcut;

    UserMenuItem *current = dynamic_cast<UserMenuItem *>( m_menutree->currentItem() );
    if (current) {
        current->setText(1, shortcut);
        current->setShortcut(shortcut);

        m_UserMenuDialog.m_keyChooser->applyStealShortcut();
        setModified();
    }
}

//////////////////////////////  Selection checkbox slots (right widget) //////////////////////////////

void UserMenuDialog::slotSelectionStateChanged(int state)
{
    m_UserMenuDialog.m_cbContextMenu->setEnabled(state);
    if(!state) {
        m_UserMenuDialog.m_cbContextMenu->setChecked(state);
    }
    setModified();
}

void UserMenuDialog::slotCheckboxStateChanged(int)
{
    setModified();
}

////////////////////////////// read menu item data //////////////////////////////

void UserMenuDialog::readMenuentryData(UserMenuItem *item)
{
    KILE_DEBUG_MAIN << "read current menu item ...";
    if (!item) {
        return;
    }

    UserMenuData::MenuType type = UserMenuData::MenuType( m_listMenutypes.indexOf(m_UserMenuDialog.m_lbMenuentryType->text()) );
    item->setMenutype(type);
    if (type == UserMenuData::Separator) {
        return;
    }

    item->setMenutitle(m_UserMenuDialog.m_leMenuEntry->text().trimmed());
    item->setFilename(m_UserMenuDialog.m_urlRequester->text().trimmed());
    item->setParameter(m_UserMenuDialog.m_leParameter->text().trimmed());
    item->setPlaintext(m_UserMenuDialog.m_teText->toPlainText());

    item->setMenuicon(m_currentIcon);
    item->setShortcut(m_UserMenuDialog.m_keyChooser->keySequence().toString(QKeySequence::NativeText));

    item->setNeedsSelection(m_UserMenuDialog.m_cbNeedsSelection->checkState());
    item->setUseContextMenu(m_UserMenuDialog.m_cbContextMenu->checkState());
    item->setReplaceSelection(m_UserMenuDialog.m_cbReplaceSelection->checkState());
    item->setSelectInsertion(m_UserMenuDialog.m_cbSelectInsertion->checkState());
    item->setInsertOutput(m_UserMenuDialog.m_cbInsertOutput->checkState());

    bool executable = (type==UserMenuData::Program && m_menutree->isItemExecutable(item->filename()));
    item->setModelData(executable);

    item->setText(0, item->updateMenutitle());
}

////////////////////////////// show menu item data //////////////////////////////

void UserMenuDialog::showMenuentryData(UserMenuItem *item)
{
    KILE_DEBUG_MAIN << "show new menu item ...";
    if (!item) {
        disableMenuEntryData();
        return;
    }

    UserMenuData::MenuType type = item->menutype();

    blockSignals(true);
    switch (type) {
    case UserMenuData::Text:
        setTextEntry(item);
        break;
    case UserMenuData::FileContent:
        setFileContentEntry(item);
        break;
    case UserMenuData::Program:
        setProgramEntry(item);
        break;
    case UserMenuData::Separator:
        setSeparatorEntry(item);
        break;
    case UserMenuData::Submenu:
        setSubmenuEntry(item);
        break;
    default:
        disableMenuEntryData();    // should not happen
    }
    blockSignals(false);
}

void UserMenuDialog::setTextEntry(UserMenuItem *item)
{
    setMenuentryText(item, true);
    setMenuentryType(item, true, true);
    setMenuentryFileChooser(item, false);
    setMenuentryFileParameter(item, false);
    setMenuentryTextEdit(item, true);
    setMenuentryIcon(item, true);
    setMenuentryShortcut(item, true);
    setParameterGroupbox(true);
    setMenuentryCheckboxes(item, false);
}

void UserMenuDialog::setFileContentEntry(UserMenuItem *item)
{
    setMenuentryText(item, true);
    setMenuentryType(item, true, true);
    setMenuentryFileChooser(item, true);
    setMenuentryFileParameter(item, false);
    setMenuentryTextEdit(item, false);
    setMenuentryIcon(item, true);
    setMenuentryShortcut(item, true);
    setParameterGroupbox(true);
    setMenuentryCheckboxes(item, false);
}

void UserMenuDialog::setProgramEntry(UserMenuItem *item)
{
    setMenuentryText(item, true);
    setMenuentryType(item, true, true);
    setMenuentryFileChooser(item, true);
    setMenuentryFileParameter(item, true);
    setMenuentryTextEdit(item, false);
    setMenuentryIcon(item, true);
    setMenuentryShortcut(item, true);
    setParameterGroupbox(true);
    setMenuentryCheckboxes(item, true);
}

void UserMenuDialog::setSeparatorEntry(UserMenuItem *item)
{
    disableMenuEntryData();
    setMenuentryType(item, true, false);
}

void UserMenuDialog::setSubmenuEntry(UserMenuItem *item)
{
    setMenuentryText(item, true);
    setMenuentryType(item, true, false);
    setMenuentryFileChooser(Q_NULLPTR, false);
    setMenuentryFileParameter(Q_NULLPTR, false);
    setMenuentryTextEdit(Q_NULLPTR, false);
    setMenuentryIcon(Q_NULLPTR, false);
    setMenuentryShortcut(Q_NULLPTR, false);
    setParameterGroupbox(false);
    setMenuentryCheckboxes(Q_NULLPTR, false);
}

////////////////////////////// update data widgets//////////////////////////////

void UserMenuDialog::setMenuentryType(UserMenuItem *item, bool state1, bool state2)
{
    const QString s = (item && state1) ? m_listMenutypes[item->menutype()] : QString();
    m_UserMenuDialog.m_lbMenuentryType->setText(s);
    m_UserMenuDialog.m_lbMenuentryType->setEnabled(state1);
    m_UserMenuDialog.m_pbMenuentryType->setEnabled(state2);
}

void UserMenuDialog::setMenuentryText(UserMenuItem *item, bool state)
{
    const QString s = (item && state) ? item->menutitle() : QString();
    m_UserMenuDialog.m_leMenuEntry->setText(s);

    m_UserMenuDialog.m_lbMenuEntry->setEnabled(state);
    m_UserMenuDialog.m_leMenuEntry->setEnabled(state);
}

void UserMenuDialog::setMenuentryFileChooser(UserMenuItem *item, bool state)
{
    const QString s = (item && state) ? item->filename() : QString();
    m_UserMenuDialog.m_urlRequester->setText(s);

    m_UserMenuDialog.m_lbFile->setEnabled(state);
    m_UserMenuDialog.m_urlRequester->setEnabled(state);
}

void UserMenuDialog::setMenuentryFileParameter(UserMenuItem *item, bool state)
{
    const QString s = (item && state) ? item->parameter() : QString();
    m_UserMenuDialog.m_leParameter->setText(s);

    m_UserMenuDialog.m_lbParameter->setEnabled(state);
    m_UserMenuDialog.m_leParameter->setEnabled(state);

}

void UserMenuDialog::setMenuentryTextEdit(UserMenuItem *item, bool state)
{
    const QString s = (item && state) ? item->plaintext() : QString();
    m_UserMenuDialog.m_teText->setPlainText(s);

    m_UserMenuDialog.m_lbText->setEnabled(state);
    m_UserMenuDialog.m_teText->setEnabled(state);
}

void UserMenuDialog::setMenuentryIcon(UserMenuItem *item, bool state, const QString &icon)
{
    if (item && state) {
        m_currentIcon = (icon.isEmpty()) ? item->menuicon() : icon;
    }
    else {
        m_currentIcon.clear();
    }

    // update widgets
    if (m_currentIcon.isEmpty()) {
        m_UserMenuDialog.m_lbIconChosen->setText(m_currentIcon);
        m_UserMenuDialog.m_lbIconChosen->hide();
        m_UserMenuDialog.m_pbIcon->show();
    }
    else {
        QString iconpath = KIconLoader::global()->iconPath(m_currentIcon,KIconLoader::Small);
        m_UserMenuDialog.m_lbIconChosen->setText("<img src=\"" +  iconpath +"\" />");
        m_UserMenuDialog.m_lbIconChosen->show();
        m_UserMenuDialog.m_pbIcon->hide();
    }

    m_UserMenuDialog.m_lbIcon->setEnabled(state);
    m_UserMenuDialog.m_pbIcon->setEnabled(state);
    m_UserMenuDialog.m_lbIconChosen->setEnabled(state);
    bool deleteIconState = ( state && !m_currentIcon.isEmpty() );
    m_UserMenuDialog.m_pbIconDelete->setEnabled(deleteIconState);
}

void UserMenuDialog::setMenuentryShortcut(UserMenuItem *item, bool state)
{
    if (item && state) {
        QString shortcut = item->shortcut();
        if (shortcut.isEmpty()) {
            m_UserMenuDialog.m_keyChooser->clearKeySequence();
        }
        else {
            m_UserMenuDialog.m_keyChooser->setKeySequence(QKeySequence(shortcut));
        }
        item->setText(1, shortcut);
    }
    else {
        m_UserMenuDialog.m_keyChooser->clearKeySequence();
    }

    m_UserMenuDialog.m_lbShortcut->setEnabled(state);
    m_UserMenuDialog.m_keyChooser->setEnabled(state);
}

void UserMenuDialog::setParameterGroupbox(bool state)
{
    m_UserMenuDialog.m_gbParameter->setEnabled(state);
}

void UserMenuDialog::setMenuentryCheckboxes(UserMenuItem *item, bool useInsertOutput)
{
    bool selectionState, insertionState, outputState, replaceState, contextState;
    if (item) {
        selectionState = item->needsSelection();
        replaceState   = item->replaceSelection();
        insertionState = item->selectInsertion();
        outputState    = (useInsertOutput) ? item->insertOutput() : false;
        contextState   = (selectionState) ? item->useContextMenu() : false;
    }
    else {
        selectionState = false;
        replaceState   = false;
        insertionState = false;
        outputState    = false;
        contextState   = false;
    }

    // m_cbNeedsSelection and m_cbSelectInsertion are always enabled
    m_UserMenuDialog.m_cbNeedsSelection->setChecked(selectionState);

    // m_cbContextMenu needs a selection to be enabled
    m_UserMenuDialog.m_cbContextMenu->setEnabled(selectionState);

    m_UserMenuDialog.m_cbContextMenu->setChecked(contextState);
    m_UserMenuDialog.m_cbReplaceSelection->setChecked(replaceState);
    m_UserMenuDialog.m_cbSelectInsertion->setChecked(insertionState);
    m_UserMenuDialog.m_cbInsertOutput->setChecked(outputState);
    m_UserMenuDialog.m_cbInsertOutput->setEnabled(useInsertOutput);
}

void UserMenuDialog::clearMenuEntryData()
{
    m_UserMenuDialog.m_leMenuEntry->clear();
    m_UserMenuDialog.m_lbMenuentryType->clear();
    m_UserMenuDialog.m_urlRequester->clear();
    m_UserMenuDialog.m_teText->clear();
    m_UserMenuDialog.m_pbIcon->setIcon(QIcon::fromTheme(i18n("Choose")));
    m_UserMenuDialog.m_keyChooser->clearKeySequence();

    m_UserMenuDialog.m_cbNeedsSelection->setChecked(false);
    m_UserMenuDialog.m_cbReplaceSelection->setChecked(false);
    m_UserMenuDialog.m_cbContextMenu->setChecked(false);
    m_UserMenuDialog.m_cbSelectInsertion->setChecked(false);
    m_UserMenuDialog.m_cbInsertOutput->setChecked(false);
}

void UserMenuDialog::disableMenuEntryData()
{
    setMenuentryText(Q_NULLPTR, false);
    setMenuentryType(Q_NULLPTR, false, false);
    setMenuentryFileChooser(Q_NULLPTR, false);
    setMenuentryFileParameter(Q_NULLPTR, false);
    setMenuentryTextEdit(Q_NULLPTR, false);
    setMenuentryIcon(Q_NULLPTR, false);
    setMenuentryShortcut(Q_NULLPTR, false);
    setParameterGroupbox(false);
    setMenuentryCheckboxes(Q_NULLPTR, false);
}

}
