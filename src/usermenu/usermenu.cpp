/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2017-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QFile>
#include <QRegExp>

#include <QTemporaryFile>
#include <KXMLGUIFactory>
#include <QMenuBar>
#include <QAction>
#include <KMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include "kileactions.h"
#include "editorextension.h"
#include "kileviewmanager.h"
#include "usermenu/usermenu.h"

#include "kileconfig.h"
#include "kiledebug.h"


namespace KileMenu {

// The UserMenu uses six values/data structures:
//
//  - getUserMenu(): the menu with its entries/actions itself (QMenu *)
//    (actions for menu items are named 'useraction-n', where n is a
//     number starting at 0. It is also used as index for the m_menudata list.)
//
//  - m_menudata: a list, containing all info for menu item (QList<UserMenuData>)
//
//  - m_actioncollection: KActionCollection of KileMainWindow (KActionCollection *)
//
//  - m_actionlist: a list with all actions of the menu (QList<QAction *>)
//
//  - m_actionlistContextMenu: a list with all actions of the context menu for selected text (QList<QAction *>)
//
//  - a menu is defined in an xml file, which is placed in QStandardPaths::locate(QStandardPaths::DataLocation, "usermenu", QStandardPaths::LocateDirectory)

UserMenu::UserMenu(KileInfo *ki, QObject *receiver)
    : m_ki(ki), m_receiver(receiver), m_proc(Q_NULLPTR)
{
    KXmlGuiWindow *mainwindow = m_ki->mainWindow();
    m_actioncollection = mainwindow->actionCollection();

    // add actions and menu entries
    m_wizardAction1 = new QAction(this);
    m_wizardAction1->setSeparator(true);
    m_wizardAction2 = createAction("wizard_usermenu");

    m_latexAction1 = new QAction(this);
    m_latexAction1->setSeparator(true);
    m_latexAction2 = createAction("wizard_usermenu2");

    m_latexMenuEntry = new QMenu(i18n("User Menu"));
    m_latexMenuEntry->setObjectName("usermenu-submenu");

    addSpecialActionsToMenus();

    // look for an existing menufile:
    // if filename matches 'basename.ext' then the file is placed in 'KILE-LOCAL-DIR/usermenu' directory
    m_currentXmlFile = KileConfig::userMenuFile();
    if ( !m_currentXmlFile.isEmpty() ) {
        if ( !m_currentXmlFile.contains("/") ) {
            m_currentXmlFile = QStandardPaths::locate(QStandardPaths::DataLocation, "usermenu", QStandardPaths::LocateDirectory) + m_currentXmlFile;
        }

        if ( QFile(m_currentXmlFile).exists() ) {
            KILE_DEBUG_MAIN << "install menufile: " << m_currentXmlFile;
            installXml(m_currentXmlFile);
        }
        else {
            m_currentXmlFile.clear();
        }
    }

    updateUsermenuPosition();
}

UserMenu::~UserMenu()
{
    delete m_proc;
}

bool UserMenu::isEmpty()
{
    return (getMenuItem()->actions().size() == 0);
}
/////////////////////// install usermenu//////////////////////////////

QAction *UserMenu::createAction(const QString &name)
{
    QAction *action = m_actioncollection->addAction(name, m_receiver, SLOT(quickUserMenuDialog()));
    action->setText(i18n("Edit User Menu"));
    action->setIcon(QIcon::fromTheme("wizard_usermenu"));
    return action;
}

void UserMenu::addSpecialActionsToMenus()
{
    KXmlGuiWindow *mainwindow = m_ki->mainWindow();

    // update wizard menu
    QMenu *wizard_menu = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("wizard", mainwindow));
    wizard_menu->addAction(m_wizardAction1);
    wizard_menu->addAction(m_wizardAction2);

    // update latex menu
    QMenu *latex_menu  = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_latex", mainwindow));
    latex_menu->addAction(m_latexAction1);
    latex_menu->addAction(m_latexAction2);
    latex_menu->addMenu(m_latexMenuEntry);
}

void UserMenu::updateUsermenuPosition()
{
    // and set the new one
    const bool show = !isEmpty() && m_ki->viewManager()->currentTextView();
    if(getUserMenuLocation() == StandAloneLocation) {
        setStandAloneMenuVisible(true, show);
    }
    else {
        setStandAloneMenuVisible(false, show);
    }
}

void UserMenu::setStandAloneMenuVisible(bool state, bool show)
{
    m_wizardAction1->setVisible(state);
    m_wizardAction2->setVisible(state);

    m_latexAction1->setVisible(!state);
    m_latexAction2->setVisible(!state);

    m_latexMenuEntry->menuAction()->setVisible(!state && show);

    KXmlGuiWindow *mainwindow = m_ki->mainWindow();
    QMenu *standAloneMenu = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_usermenu", mainwindow));
    if(standAloneMenu) {
        standAloneMenu->menuAction()->setVisible(state && show);
    }
}

///////////////////////////// clear all data //////////////////////////////

// clear all lists and data for an existing usermenu
void UserMenu::clear()
{
    // clear usermenu and menudata
    if(getMenuItem()) {
        getMenuItem()->clear();
    }
    m_menudata.clear();

    // remove all actions from actioncollection
    for(QAction *action : m_actionlist) {
        m_actioncollection->removeAction(action);
    }

    // clear actionlists
    m_actionlist.clear();
    m_actionlistContextMenu.clear();
}

///////////////////////////// update GUI //////////////////////////////

// repopulate the user menu and show it at the desired location
void UserMenu::updateGUI()
{
    KILE_DEBUG_MAIN << "updating usermenu ...";

    addSpecialActionsToMenus(); // adding actions twice has no effect

    // like installXmlFile(), but without updating KileConfig::userMenuFile
    // first clear old usermenu, menudata, actions and actionlists
    clear();

    // then install
    if(installXml(m_currentXmlFile)) {
        // add changed context menu to all existing views
        KileView::Manager* viewManager = m_ki->viewManager();
        int views = viewManager->textViewCount();
        for ( int i=0; i<views; ++i ) {
            viewManager->installContextMenu( viewManager->textView(i) );
        }
    }

    updateUsermenuPosition();
}

///////////////////////////// update key bindings //////////////////////////////

// shortcut dialog was called, so key bindings may have been changed
void UserMenu::updateKeyBindings()
{
    if ( m_currentXmlFile.isEmpty() && !QFile(m_currentXmlFile).exists() ) {
        return;
    }

    // new key bindings are found in kileui.rc (ActionProperties)
    // remove them, as they will be written into usermenu xml file
    removeActionProperties();

    // update xml file of current usermenu
    updateXmlFile(m_currentXmlFile);
}

QMenu* UserMenu::getMenuItem()
{

    if(getUserMenuLocation() == StandAloneLocation) {
        KParts::MainWindow *mainWindow = m_ki->mainWindow();
        return dynamic_cast<QMenu*>(mainWindow->guiFactory()->container("menu_usermenu", mainWindow));
    }
    else {
        return m_latexMenuEntry;
    }
}

void UserMenu::removeActionProperties()
{
    QString xmlfile = "kileui.rc";
    QString xml(KXMLGUIFactory::readConfigFile(xmlfile));
    if ( xml.isEmpty() ) {
        KILE_DEBUG_MAIN << "STOP: xmlfile not found: " << xmlfile;
        return;
    }

    QDomDocument doc;
    doc.setContent( xml );

    // process XML data in section 'ActionProperties'
    QDomElement actionPropElement = KXMLGUIFactory::actionPropertiesElement( doc );
    if ( actionPropElement.isNull() ) {
        KILE_DEBUG_MAIN << "QDomElement actionPropertiesElement not found ";
        return;
    }

    // search for all actions of the user-defined UserMenu
    KILE_DEBUG_MAIN << "QDomElement actionPropertiesElement found ";
    bool changed = false;
    QRegExp re("useraction-(\\d+)$");
    QDomElement e = actionPropElement.firstChildElement();
    while(!e.isNull()) {
        QString tag = e.tagName();
        if(tag != "Action") {
            continue;
        }

        QString shortcut = e.attribute("shortcut");
        QString name = e.attribute("name");

        QDomElement removeElement;
        if ( re.indexIn(name) == 0) {
            int index = re.cap(1).toInt();
            KILE_DEBUG_MAIN << "action property was changed: old=" << m_menudata[index].shortcut << " new=" << name << " actionIndex=" << index;
            removeElement = e;
            changed = true;
        }

        e = e.nextSiblingElement();

        // finally delete element
        if ( !removeElement.isNull() ) {
            KILE_DEBUG_MAIN << "remove ActionProperty: shortcut=" << shortcut << " name=" << name;
            actionPropElement.removeChild(removeElement);
        }
    }

    // Write back to XML file
    if ( changed ) {
        KXMLGUIFactory::saveConfigFile(doc,xmlfile);
    }
}

///////////////////////////// update action properties (shortcuts) //////////////////////////////

// Calling m_mainWindow->guiFactory()->refreshActionProperties() in kile.cpp removes all
// user-defined action shortcuts and icons. Here they will be refreshed again.
void UserMenu::refreshActionProperties()
{
    KILE_DEBUG_MAIN << "refresh action properties";

    QRegExp re("useraction-(\\d+)$");
    foreach ( QAction *action, m_actionlist ) {
        if ( re.indexIn(action->objectName()) == 0 ) {
            int actionIndex = re.cap(1).toInt();
            if ( !m_menudata[actionIndex].icon.isEmpty() ) {
                action->setIcon( QIcon::fromTheme(m_menudata[actionIndex].icon) );
            }
            if ( !m_menudata[actionIndex].shortcut.isEmpty() ) {
                action->setShortcut( QKeySequence(m_menudata[actionIndex].shortcut,QKeySequence::NativeText) );
            }
        }
    }
}

// Before calling usermenu dialog, all user-defined action shortcuts must be removed,
// or the dialog will give a lot of warnings. All shortcuts (even if changed) in the usermenu
// will be refreshed again, when the dialog is finished
void UserMenu::removeShortcuts()
{
    foreach ( QAction *action, m_actionlist ) {
        action->setShortcut( QKeySequence() );
    }
}

///////////////////////////// install/remove xml //////////////////////////////

// call from the menu: no xml file given
void UserMenu::installXmlMenufile()
{
    KILE_DEBUG_MAIN << "install xml file with QFileDialog::getOpenFileName";

    QString directory = selectUserMenuDir();
    QString filter = i18n("User Menu Files (*.xml)");

    QString filename = QFileDialog::getOpenFileName(m_ki->mainWindow(), i18n("Select Menu File"), directory, filter);
    if(filename.isEmpty()) {
        return;
    }

    if( !QFile::exists(filename) ) {
        KMessageBox::error(m_ki->mainWindow(), i18n("File '%1' does not exist.", filename));
    }
    else {
        installXmlFile(filename);
    }
}

// SIGNAL from usermenu dialog: install new usermenu (xml file given)
//
// use 'basename.ext' if the file is placed in 'KILE-LOCAL-DIR/usermenu' directory and full filepath else
void UserMenu::installXmlFile(const QString &filename)
{
    KILE_DEBUG_MAIN << "install xml file" << filename;

    // clear old usermenu, menudata, actions and actionlists
    clear();

    if ( installXml(filename) ) {
        // update current xml filename (with path)
        m_currentXmlFile = filename;

        // save xml file in config (with or without path)
        QString xmlfile = filename;
        QString dir = QStandardPaths::locate(QStandardPaths::DataLocation, "usermenu", QStandardPaths::LocateDirectory);
        if ( filename.startsWith(dir) ) {
            QString basename = filename.right( filename.length()-dir.length() );
            if ( !basename.isEmpty() && !basename.contains("/") )  {
                xmlfile = basename;
            }
        }
        KileConfig::setUserMenuFile(xmlfile);
        emit (updateStatus());

        // add changed context menu to all existing views
        KileView::Manager* viewManager = m_ki->viewManager();
        int views = viewManager->textViewCount();
        for ( int i=0; i<views; ++i ) {
            viewManager->installContextMenu( viewManager->textView(i) );
        }
    }
}

void UserMenu::removeXmlFile()
{
    KILE_DEBUG_MAIN << "remove xml file";

    clear();
    m_currentXmlFile.clear();

    KileConfig::setUserMenuFile(m_currentXmlFile);
    emit (updateStatus());
}

///////////////////////////// install usermenu from XML //////////////////////////////

// pre: usermenu is already cleared
bool UserMenu::installXml(const QString &filename)
{
    KILE_DEBUG_MAIN << "install: start";

    QMenu *userMenu = getMenuItem();

    if(!userMenu) {
        KILE_DEBUG_MAIN << "Hmmmm: found no usermenu";
        return false;
    }

    // read content of xml file
    QDomDocument doc("UserMenu");
    QFile file(filename);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        // TODO KMessageBox
        KILE_DEBUG_MAIN << "STOP: can't open xml file " << filename;
        return false;
    }

    if( !doc.setContent( &file ) ) {
        file.close();
        return false;
    }
    file.close();

    KILE_DEBUG_MAIN << "parse xml ...";
    m_actionsContextMenu = 0;

    // parse toplevelitems
    int actionnumber = 0;
    QDomElement root = doc.documentElement();
    QDomElement e = root.firstChildElement();
    while ( !e.isNull()) {
        QString tag = e.tagName();

        if ( tag=="submenu" || tag=="separator") {
            if ( tag == "submenu" ) {
                installXmlSubmenu(e, userMenu, actionnumber);
            }
            else { /* tag=="separator" */
                userMenu->addSeparator();
            }

            // try to get some structure into to the context menu
            if ( m_actionsContextMenu > 0 ) {
                m_actionlistContextMenu.append(Q_NULLPTR);
                m_actionsContextMenu = 0;
            }
        }
        else { /* if ( tag == "menu" ) */
            installXmlMenuentry(e, userMenu, actionnumber);
        }

        e = e.nextSiblingElement();
    }
    KILE_DEBUG_MAIN << "install: finished ";

    return true;
}

// install a submenu item
void UserMenu::installXmlSubmenu(const QDomElement &element, QMenu *parentmenu, int &actionnumber)
{
    QMenu *submenu = parentmenu->addMenu(QString());

    QString title;
    if ( element.hasChildNodes() ) {
        QDomElement e = element.firstChildElement();
        while ( !e.isNull()) {

            QString tag = e.tagName();
            if ( tag == "title" ) {
                title = e.text();
                submenu->setTitle(title);
            }
            else if ( tag == "submenu" ) {
                installXmlSubmenu(e,submenu,actionnumber);
            }
            else if ( tag == "separator" ) {
                submenu->addSeparator();
            }
            else { /* if ( tag == "menu" ) */
                installXmlMenuentry(e,submenu,actionnumber);
            }

            e = e.nextSiblingElement();
        }
    }
}

// install a standard menu item
void UserMenu::installXmlMenuentry(const QDomElement &element, QMenu *parentmenu, int &actionnumber)
{
    UserMenuData menudata;

    menudata.menutype  = UserMenuData::xmlMenuType( element.attribute("type") );

    // read values
    if ( element.hasChildNodes() ) {
        QDomElement e = element.firstChildElement();
        while ( !e.isNull()) {
            QString tag = e.tagName();
            QString text = e.text();

            int index = UserMenuData::xmlMenuTag(tag);
            switch (index) {
            case  UserMenuData::XML_TITLE:
                menudata.menutitle = text;
                break;
            case  UserMenuData::XML_PLAINTEXT:
                menudata.text = UserMenuData::decodeLineFeed(text);
                break;
            case  UserMenuData::XML_FILENAME:
                menudata.filename = text;
                break;
            case  UserMenuData::XML_PARAMETER:
                menudata.parameter = text;
                break;
            case  UserMenuData::XML_ICON:
                menudata.icon = text;
                break;
            case  UserMenuData::XML_SHORTCUT:
                menudata.shortcut = text;
                break;
            case  UserMenuData::XML_NEEDSSELECTION:
                menudata.needsSelection   = str2bool(text);
                break;
            case  UserMenuData::XML_USECONTEXTMENU:
                menudata.useContextMenu   = str2bool(text);
                break;
            case  UserMenuData::XML_REPLACESELECTION:
                menudata.replaceSelection = str2bool(text);
                break;
            case  UserMenuData::XML_SELECTINSERTION:
                menudata.selectInsertion  = str2bool(text);
                break;
            case  UserMenuData::XML_INSERTOUTPUT:
                menudata.insertOutput     = str2bool(text);
                break;
            }

            e = e.nextSiblingElement();
        }
    }

    // add menu item, if its title is not empty
    if ( !menudata.menutitle.isEmpty() ) {

        QAction *action = m_actioncollection->addAction(QString("useraction-%1").arg(actionnumber), this, SLOT(slotUserMenuAction()) );
        if ( action ) {
            action->setText(menudata.menutitle);

            if ( !menudata.icon.isEmpty() ) {
                action->setIcon( QIcon::fromTheme(menudata.icon) );
            }

            if ( !menudata.shortcut.isEmpty() ) {
                action->setShortcut( QKeySequence(menudata.shortcut,QKeySequence::PortableText) );
            }

            parentmenu->addAction(action);
            m_menudata.append(menudata);
            m_actionlist.append(action);
            if ( menudata.useContextMenu )  {
                m_actionlistContextMenu.append(action);
                m_actionsContextMenu++;
            }

            actionnumber++;
        }
    }
}

///////////////////////////// update XML file //////////////////////////////

// key bindings dialog was called, so the usermenu xml file must be updated, if one of these actions was changed
// pre: xml file exists
void UserMenu::updateXmlFile(const QString &filename)
{
    KILE_DEBUG_MAIN << "update xml file: " << filename;

    // read content of xml file
    QDomDocument doc("UserMenu");
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    doc.setContent(&file);
    file.close();

    KILE_DEBUG_MAIN << "parse xml ...";

    // parse toplevelitems
    bool changed = false;
    int actionnumber = 0;
    QDomElement root = doc.documentElement();
    QDomElement e = root.firstChildElement();
    while ( !e.isNull()) {
        QString tag = e.tagName();
        if ( tag == "submenu" ) {
            changed = changed || updateXmlSubmenu(doc,e,actionnumber);
        }
        else if ( tag == "menu" ) {
            changed = changed || updateXmlMenuentry(doc,e,actionnumber);
        }
        e = e.nextSiblingElement();
    }
    KILE_DEBUG_MAIN << "update finished ";

    if ( changed ) {
        KILE_DEBUG_MAIN << "found changes, so write updated xml file ";
        QFile outfile(filename);
        outfile.open(QFile::WriteOnly | QFile::Text);
        QTextStream stream(&outfile);
        doc.save(stream,3);
        outfile.close();
    }
}

// install a submenu item
bool UserMenu::updateXmlSubmenu(QDomDocument &doc, QDomElement &element, int &actionnumber)
{
    bool changed = false;

    if ( element.hasChildNodes() ) {
        QDomElement e = element.firstChildElement();
        while ( !e.isNull()) {
            QString tag = e.tagName();
            if ( tag == "submenu" ) {
                changed = changed || updateXmlSubmenu(doc,e,actionnumber);
            }
            else if ( tag == "menu" )  {
                changed = changed || updateXmlMenuentry(doc,e,actionnumber);
            }

            e = e.nextSiblingElement();
        }
    }
    return changed;
}

// install a standard menu item
bool UserMenu::updateXmlMenuentry(QDomDocument &doc, QDomElement &element, int &actionnumber)
{
    bool changed = false;

    // read values
    if ( element.hasChildNodes() ) {
        QDomElement oldElement;
        QDomElement e = element.firstChildElement();
        while ( !e.isNull()) {
            QString tag = e.tagName();

            if ( UserMenuData::xmlMenuTag(tag) == UserMenuData::XML_SHORTCUT) {
                oldElement = e;
                //oldText = e.text();    value not needed, is also in m_menudata[]
            }

            e = e.nextSiblingElement();
        }

        // keybindings dialog has already updated all actions
        QString currentShortcut = m_actionlist[actionnumber]->shortcut().toString(QKeySequence::PortableText);
        if ( currentShortcut != m_menudata[actionnumber].shortcut ) {
            // an existing shortcut always needs a new QDomElement
            if ( !currentShortcut.isEmpty() ) {
                // create element with new shortcut
                QDomElement newElement = doc.createElement( UserMenuData::xmlMenuTagName(UserMenuData::XML_SHORTCUT) );
                QDomText newText = doc.createTextNode(currentShortcut);
                newElement.appendChild(newText);

                // replace existing node with new node
                if ( !oldElement.isNull() ) {
                    element.replaceChild(newElement,oldElement);
                }
                // or insert a new node
                else {
                    element.appendChild(newElement);
                }
            }
            // or delete an existing QDomElement
            else {
                element.removeChild(oldElement);
            }
            changed = true;
        }
    }

    actionnumber++;
    return changed;
}

////////////////////////////// load dir for xml files (static) //////////////////////////////

// - start with search for xml files in the local directory
// - if no files are present, but in global directory, start with global
// - if not a single file was found: back to local directory

QString UserMenu::selectUserMenuDir()
{
    QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "usermenu", QStandardPaths::LocateDirectory);
    if ( dirs.size() < 2 ) {
        return dirs.at(0);
    }

    QStringList namefilter = QStringList() << "*.xml";
    QString localDirName = dirs.at(0);
    QDir localDir = QDir(localDirName);
    QStringList localList = localDir.entryList (namefilter,QDir::Files | QDir::Readable);
    if ( localList.size() > 0 ) {
        return localDirName;
    }

    QDir globalDir = QDir(dirs.at(1));
    QStringList globalList = globalDir.entryList (namefilter,QDir::Files | QDir::Readable);
    return ( globalList.size() > 0 ) ? dirs.at(1) : localDirName;
}

////////////////////////////// execUserMenuAction //////////////////////////////

// an action was called from the usermenu:
//  - identify action
//  - find textview
//  - execute action
void UserMenu::slotUserMenuAction()
{
    KILE_DEBUG_MAIN << "want to start an action from usermenu ...";

    QAction *action = dynamic_cast<QAction *>(sender());
    if ( !action ) {
        return;
    }

    QString actionName = action->objectName();
    KILE_DEBUG_MAIN << "action name: " << actionName << "classname=" << action->metaObject()->className();

    QRegExp re("useraction-(\\d+)$");
    if ( re.indexIn(actionName) != 0) {
        KILE_DEBUG_MAIN << "STOP: found wrong action name: " << actionName;
        return;
    }

    bool ok;
    int actionIndex = re.cap(1).toInt(&ok);
    if ( actionIndex < 0 || actionIndex >= m_menudata.size() ) {
        KILE_DEBUG_MAIN << "STOP: invalid action (range error): " << actionIndex << "  list size: " << m_menudata.size();
        return;
    }

    // check view and action requirements
    KTextEditor::View *view = m_ki->viewManager()->currentTextView();

    if ( !view ) {
        return;
    }

    if ( !view->selection() && m_menudata[actionIndex].needsSelection ) {
        return;
    }

    UserMenuData::MenuType type = m_menudata[actionIndex].menutype;

    if ( type == UserMenuData::Text ) {
        execActionText(view,m_menudata[actionIndex]);
    }
    else if ( type == UserMenuData::FileContent ) {
        execActionFileContent(view,m_menudata[actionIndex]);
    }
    else if ( type == UserMenuData::Program ) {
        execActionProgramOutput(view,m_menudata[actionIndex]);
    }
    else {
        KILE_DEBUG_MAIN << "STOP: unknown action type: " << type;
    }
}

////////////////////////////// execActionText //////////////////////////////

// execute an action: insert text
void UserMenu::execActionText(KTextEditor::View *view, const UserMenuData &menudata)
{
    KILE_DEBUG_MAIN << "want to insert text ... ";
    insertText(view, menudata.text, menudata.replaceSelection, menudata.selectInsertion);
}

////////////////////////////// execActionFileContent //////////////////////////////

// execute an action: insert file contents
void UserMenu::execActionFileContent(KTextEditor::View *view, const UserMenuData &menudata)
{
    KILE_DEBUG_MAIN << "want to insert contents of a file: " << menudata.filename;

    QFile file(menudata.filename);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        KILE_DEBUG_MAIN << "STOP: could not open file " << menudata.filename;
        return;
    }

    QTextStream stream( &file );
    QString text = stream.readAll();
    file.close();

    if ( !text.isEmpty() ) {
        insertText(view, text, menudata.replaceSelection, menudata.selectInsertion);
    }
}

////////////////////////////// execActionFileContent //////////////////////////////

// execute an action: run a program
void UserMenu::execActionProgramOutput(KTextEditor::View *view, const UserMenuData &menudata)
{
    KILE_DEBUG_MAIN << "want to start a program ... ";

    // delete old process
    if (m_proc) {
        delete m_proc;
        m_proc = Q_NULLPTR;
    }

    // build commandline
    QString cmdline = menudata.filename + " " + menudata.parameter;
    bool useTemporaryFile = cmdline.contains("%M");

    bool needsSelection = menudata.needsSelection;
    bool hasSelection = view->selection();

    // check parameter
    if ( needsSelection && !hasSelection ) {
        KILE_DEBUG_MAIN << "STOP: this program needs selected text";
        return;
    }

    // do we need a temporary file for the selected text?
    if ( hasSelection && useTemporaryFile ) {
        KILE_DEBUG_MAIN << "selection and 'placeholder' %M found --> create temporary file";

        // create temporary file
        QTemporaryFile tempfile;
//code was 		tempfile.setSuffix(".txt");
//Add to constructor and adapt if necessay: QDir::tempPath() + QLatin1String("/myapp_XXXXXX") + QLatin1String(".txt")
        tempfile.setAutoRemove(false);

        if ( !tempfile.open() ) {
            KILE_DEBUG_MAIN << "STOP: could not create tempfile for selected text" ;
            return;
        }

        // get filename
        QString selfile = tempfile.fileName();

        // write selection
        QTextStream stream( &tempfile );
        stream << view->selectionText() << "\n";
        tempfile.close();

        // update comamndline with temporary filename of selection
        cmdline.replace("%M",selfile);
    }

    // replace %F with the complete base name of the file without the path.
    // The complete base name consists of all characters in the file up to
    // (but not including) the last '.' character.
    if (  cmdline.contains("%S") ) {
        QFileInfo fi(view->document()->url().toLocalFile());
        QString basename = fi.completeBaseName();
        cmdline.replace("%S",basename);
    }

    m_proc = new KProcess(this);
    m_proc->setShellCommand(cmdline);
    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    m_proc->setReadChannel(QProcess::StandardOutput);

    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
    connect(m_proc, SIGNAL(readyReadStandardError()),  this, SLOT(slotProcessOutput()));
    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

    KILE_DEBUG_MAIN << "... start proc: " << cmdline;
    // init and/or save important data
    m_procOutput.clear();
    m_procView = view;
    m_procMenudata = &menudata;

    m_proc->start();
}

void UserMenu::slotProcessOutput()
{
    m_procOutput += m_proc->readAll();
}

void UserMenu::slotProcessExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{
    KILE_DEBUG_MAIN << "... finish proc ";
    KILE_DEBUG_MAIN << "output:  " << m_procOutput;

    if ( exitStatus == QProcess::NormalExit && m_procMenudata->insertOutput && !m_procOutput.isEmpty() ) {
        insertText(m_procView, m_procOutput, m_procMenudata->replaceSelection, m_procMenudata->selectInsertion);
    }
}

////////////////////////////// auxiliary //////////////////////////////

// action is finished, now insert some text
void UserMenu::insertText(KTextEditor::View *view, const QString &text, bool replaceSelection, bool selectInsertion)
{
    KILE_DEBUG_MAIN << "insert text from action: " << text;
    // metachars: %R - references (like \ref{%R}, \pageref{%R} ...)
    //            %T - citations  (like \cite{%T} ...)
    QString metachar,label;
    int actiontype =0;

    if(text.contains("%R")) {
        metachar = "%R";
        label = i18n("Label");
        actiontype = KileAction::FromLabelList;
    }
    else if(text.contains("%T")) {
        metachar = "%T";
        label = i18n("Reference");
        actiontype = KileAction::FromBibItemList;
    }
    if(!metachar.isEmpty()) {
        QStringList list = text.split(metachar);

        KileAction::InputTag tag(m_ki, i18n("Input Dialog"), QString(), QKeySequence(), m_receiver, SLOT(insertTag(const KileAction::TagData&)), m_actioncollection,"tag_temporary_action", m_ki->mainWindow(), actiontype, list.at(0)+metachar, list.at(1), list.at(0).length(), 0, QString(), label);

        tag.activate(QAction::Trigger);
        return;
    }

    // metachars: %B - bullet
    //            %M - selected text
    //            %C - place cursor
    //            %E - indent in environment
    QString ins = text;
    bool bullet = ins.contains("%B");

    // deselect and/or remove current selection
    if(view->selection()) {
        if(ins.contains("%M")) {
            ins.replace("%M", view->selectionText());
        }
        if(replaceSelection) {
            view->removeSelectionText();
        }
        else {
            view->removeSelection();
        }
    }
    else {
        ins.replace("%M", QString());
    }
    KILE_DEBUG_MAIN << " ---> " << ins;

    // insert new text
    KTextEditor::Cursor cursor1 = view->cursorPosition();
    emit( sendText(ins) );

    // select inserted text
    if(selectInsertion) {
        KTextEditor::Cursor cursor2 = view->cursorPosition();
        view->setSelection(KTextEditor::Range(cursor1, cursor2));
    }

    // text with bullet metachar %B
    if(bullet) {
        view->setCursorPosition(cursor1);
        m_ki->editorExtension()->gotoBullet(false, view);
    }
}

////////////////////////////// auxiliary //////////////////////////////

bool UserMenu::str2bool(const QString &value)
{
    return ( value == "true" );
}


}

