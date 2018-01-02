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


#include <QIcon>
#include <QInputDialog>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QDomDocument>
#include <QProcessEnvironment>
#include <QSignalMapper>

#include <KIconLoader>
#include <QMenu>
#include <KLocalizedString>
#include <KMessageBox>


#include "dialogs/usermenu/usermenuitem.h"
#include "dialogs/usermenu/usermenutree.h"

#include "kiledebug.h"


// Qt::UserRole+1: menutype
// Qt::UserRole+2: errorcode for menu item

namespace KileMenu {

// - Separators are shown as a horizontal line (Qt:UserRole+1)
// - Menu items with errors are displayed in red (Qt:UserRole+2)
void MenuentryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    QString menutitle = index.data(Qt::DisplayRole).toString();
    int error = index.data(Qt::UserRole+2).toInt();

    // any errors?
    if (index.column()==0 && error!=UserMenuItem::MODEL_ERROR_NONE ) {
        QStyleOptionViewItem optionCustom = option;
        optionCustom.palette.setColor(QPalette::Text, Qt::red);
        QStyledItemDelegate::paint( painter, optionCustom, index );
    }
    else {
        QStyledItemDelegate::paint( painter, option, index );
    }

    // display separators
    QString itemtype = index.data(Qt::UserRole+1).toString();
    if ( itemtype == "separator" ) {
        QRect r = option.rect;
        int y =  ( r.bottom() + r.top() ) / 2;

        painter->save();
        QPen pen = QPen(Qt::gray);
        painter->setPen(pen);
        painter->drawLine(r.left()+3,y, r.right()-20,y);
        painter->restore();
    }

}
///////////////////////////// UserMenuTree //////////////////////////////

UserMenuTree::UserMenuTree(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);

    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    header()->setSectionsMovable(false);
    header()->setStretchLastSection(false);
    setColumnWidth(1,140);
    setItemDelegateForColumn(0, new MenuentryDelegate(parent));

    // drag and drop
    setDragEnabled(true);
    setDropIndicatorShown(true);
    //setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);

    initEnvPathlist();
}

bool UserMenuTree::isEmpty()
{
    return ( topLevelItemCount() == 0 );
}

///////////////////////////// PATH environment variable //////////////////////////////

// save PATH to search for executable files
void UserMenuTree::initEnvPathlist()
{
    QString envpath;
#if QT_VERSION >= 0x040600
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if ( env.contains("PATH") ) {
        envpath = env.value("PATH");
    }
#else
    // Returns the environment of the calling process as a list of key=value pairs.
    QStringList environment = QProcess::systemEnvironment();
    foreach ( const QString &s, environment ) {
        if ( s.startsWith("PATH=") ) {
            envpath = s.mid(5);
            break;
        }
    }
#endif

#ifdef Q_WS_WIN
    m_envPathlist = envpath.split(';');
#else
    m_envPathlist = envpath.split(':');
#endif
    m_envPathlist.append(".");
}

bool UserMenuTree::isItemExecutable(const QString &filename)
{
    if ( filename.isEmpty() ) {
        return false;
    }

    // absolute paths can be checked immediately
    QFileInfo fi(filename);
    if ( fi.isAbsolute() ) {
        return fi.isExecutable();
    }

    // search in all paths
    for (int i=0; i<m_envPathlist.size(); ++i ) {
        bool executable = QFileInfo(m_envPathlist[i]+"/"+filename).isExecutable();
        if ( executable ) {
            // move to front
            if ( i > 0 ) {
                QString temp = m_envPathlist[0];
                m_envPathlist[0] = m_envPathlist[i];
                m_envPathlist[i] = temp;
            }
            return true;
        }
    }

    return false;
}

///////////////////////////// context menu event //////////////////////////////

// build a context menu
void UserMenuTree::contextMenuRequested(const QPoint &pos)
{
    KILE_DEBUG_MAIN << "context menu requested ..." ;

    m_popupItem = dynamic_cast<UserMenuItem*>(itemAt(pos));
    if ( !m_popupItem ) {
        KILE_DEBUG_MAIN << "... no item found";
        return;
    }

    KILE_DEBUG_MAIN << "... popup item found: " << m_popupItem->text(0);
    bool submenu = ( m_popupItem->menutype() ==  UserMenuData::Submenu );
    bool separator = ( m_popupItem->menutype() ==  UserMenuData::Separator );

    QMenu popup;
    QAction *action = Q_NULLPTR;
    QSignalMapper signalMapper;
    connect(&signalMapper, SIGNAL(mapped(int)), this, SLOT(slotPopupActivated(int)));

    // insert standard menu items
    action = popup.addAction(QIcon::fromTheme("usermenu-insert-above.png"),i18n("Insert above"), &signalMapper, SLOT(map()));
    signalMapper.setMapping(action, POPUP_INSERT_ABOVE);
    action = popup.addAction(QIcon::fromTheme("usermenu-insert-below.png"),i18n("Insert below"), &signalMapper, SLOT(map()));
    signalMapper.setMapping(action, POPUP_INSERT_BELOW);
    popup.addSeparator();

    // insert separators
    if ( !separator ) {
        action = popup.addAction(QIcon::fromTheme("usermenu-separator-above.png"),i18n("Insert a separator above"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action, POPUP_SEPARATOR_ABOVE);
        action = popup.addAction(QIcon::fromTheme("usermenu-separator-below.png"),i18n("Insert a separator below"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action, POPUP_SEPARATOR_BELOW);
        popup.addSeparator();
    }

    // insert submenus
    action = popup.addAction(QIcon::fromTheme("usermenu-submenu-above.png"),i18n("Insert a submenu above"), &signalMapper, SLOT(map()));
    signalMapper.setMapping(action, POPUP_SUBMENU_ABOVE);
    action = popup.addAction(QIcon::fromTheme("usermenu-submenu-below.png"),i18n("Insert a submenu below"), &signalMapper, SLOT(map()));
    signalMapper.setMapping(action, POPUP_SUBMENU_BELOW);
    popup.addSeparator();

    // insert into submenus
    if ( submenu ) {
        action = popup.addAction(QIcon::fromTheme("usermenu-into-submenu.png"),i18n("Insert into this submenu"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action, POPUP_INTO_SUBMENU);
        action = popup.addAction(i18n("Insert a separator into this submenu"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action, POPUP_SEPARATOR_INTO_SUBMENU);
        action = popup.addAction(i18n("Insert a submenu into this submenu"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action, POPUP_SUBMENU_INTO_SUBMENU);
        popup.addSeparator();
    }

    // delete actions
    action = popup.addAction(QIcon::fromTheme("usermenu-delete.png"),i18n("Delete this item"), &signalMapper, SLOT(map()));
    signalMapper.setMapping(action,POPUP_DELETE_ITEM);
    popup.addSeparator();
    action = popup.addAction(QIcon::fromTheme("usermenu-clear.png"),i18n("Delete the complete tree"), &signalMapper, SLOT(map()));
    signalMapper.setMapping(action, POPUP_DELETE_TREE);

    // expand/collapse tree
    if ( submenu ) {
        popup.addSeparator();
        if ( m_popupItem->isExpanded() ) {
            action = popup.addAction(i18n("Collapse submenu"), &signalMapper, SLOT(map()));
            signalMapper.setMapping(action,POPUP_COLLAPSE_ITEM);
        }
        else  {
            action = popup.addAction(i18n("Expand submenu"), &signalMapper, SLOT(map()));
            signalMapper.setMapping(action,POPUP_EXPAND_ITEM);
        }
        popup.addSeparator();
        action = popup.addAction(i18n("Collapse complete tree"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action,POPUP_COLLAPSE_TREE);
        action = popup.addAction(i18n("Expand complete tree"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action,POPUP_EXPAND_TREE);
    }

    // if there are any errors with this item, some info is available
    int error = m_popupItem->data(0,Qt::UserRole+2).toInt();
    if ( error != UserMenuItem::MODEL_ERROR_NONE ) {
        popup.addSeparator();
        action = popup.addAction(QIcon::fromTheme("help-about.png"),i18n("Info"), &signalMapper, SLOT(map()));
        signalMapper.setMapping(action, POPUP_ITEM_INFO);
    }

    // const QPoint& pos parameter in the customContextMenuRequested() signal is normally in widget coordinates.
    // But classes like QTreeWidget, which inherit from QAbstractScrollArea1 instead use the coordinates of their viewport()
    if ( !popup.isEmpty() ) {
        popup.exec( viewport()->mapToGlobal(pos) );
    }
}

// a context menu action was selected
void UserMenuTree::slotPopupActivated(int id)
{
    KILE_DEBUG_MAIN << "popup activated with id: " << id;
    switch (id ) {
    case POPUP_INSERT_ABOVE:
        insertMenuItem (m_popupItem, false);
        break;
    case POPUP_INSERT_BELOW:
        insertMenuItem (m_popupItem, true);
        break;
    case POPUP_SEPARATOR_ABOVE:
        insertSeparator(m_popupItem, false);
        break;
    case POPUP_SEPARATOR_BELOW:
        insertSeparator(m_popupItem, true);
        break;
    case POPUP_SUBMENU_ABOVE:
        insertSubmenu  (m_popupItem, false);
        break;
    case POPUP_SUBMENU_BELOW:
        insertSubmenu  (m_popupItem, true);
        break;
    case POPUP_INTO_SUBMENU:
        insertIntoSubmenu(m_popupItem, UserMenuData::Text);
        break;
    case POPUP_SEPARATOR_INTO_SUBMENU:
        insertIntoSubmenu(m_popupItem, UserMenuData::Separator);
        break;
    case POPUP_SUBMENU_INTO_SUBMENU:
        insertIntoSubmenu(m_popupItem, UserMenuData::Submenu);
        break;
    case POPUP_DELETE_ITEM:
        itemDelete(m_popupItem);
        break;
    case POPUP_DELETE_TREE:
        deleteMenuTree();
        break;
    case POPUP_COLLAPSE_ITEM:
        m_popupItem->setExpanded(false);
        break;
    case POPUP_EXPAND_ITEM:
        m_popupItem->setExpanded(true);
        break;
    case POPUP_COLLAPSE_TREE:
        collapseAll();
        break;
    case POPUP_EXPAND_TREE:
        expandAll();
        break;
    case POPUP_ITEM_INFO:
        itemInfo(m_popupItem);
        break;
    }
}

///////////////////////////// read XML //////////////////////////////

// read an xml file and check for errors
bool UserMenuTree::readXml(const QString &filename)
{
    KILE_DEBUG_MAIN << "read xml file " << filename;

    QDomDocument doc("UserMenu");
    QFile file(filename);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        KMessageBox::error(this, i18n("File '%1' does not exist.", filename));
        return false;
    }
    if ( !doc.setContent( &file ) ) {
        file.close();
        return false;
    }
    file.close();

    KILE_DEBUG_MAIN << "parse xml ...";
    blockSignals(true);

    // clear menutree
    clear();

    // read toplevelitems
    QDomElement root = doc.documentElement();
    QDomElement e = root.firstChildElement();
    while ( !e.isNull()) {
        QString tag = e.tagName();

        UserMenuItem *item = Q_NULLPTR;
        if ( tag == "submenu" ) {
            item = readXmlSubmenu(e);
        }
        else if ( tag == "separator" ) {
            item = readXmlSeparator();
        }
        else { /* if ( tag == "menu" ) */
            item = readXmlMenuentry(e);
        }

        if ( item ) {
            addTopLevelItem(item);
        }
        e = e.nextSiblingElement();
    }

    // the xml menu is built, now check for errors
    setErrorCodes();

    // polish menutree
    expandAll();
    if ( topLevelItemCount() > 0 ) {
        setCurrentItem( topLevelItem(0) );
    }
    blockSignals(false);

    return true;
}

// a separator tag was found
UserMenuItem *UserMenuTree::readXmlSeparator()
{
    return new UserMenuItem(UserMenuData::Separator, QString());
}

// read tags for a submenu
UserMenuItem *UserMenuTree::readXmlSubmenu(const QDomElement &element)
{
    UserMenuItem *submenuitem = new UserMenuItem(UserMenuData::Submenu, QString()) ;

    QString title;
    if ( element.hasChildNodes() ) {
        QDomElement e = element.firstChildElement();
        while ( !e.isNull()) {
            UserMenuItem *item = Q_NULLPTR;

            QString tag = e.tagName();
            if ( tag == "title" ) {
                title = e.text();
            }
            else if ( tag == "submenu" ) {
                item = readXmlSubmenu(e);
            }
            else if ( tag == "separator" ) {
                item = readXmlSeparator();
            }
            else { /* if ( tag == "menu" ) */
                item = readXmlMenuentry(e);
            }

            submenuitem->setMenutitle(title);
            submenuitem->setText(0,title);

            if ( item ) {
                submenuitem->addChild(item);
            }
            e = e.nextSiblingElement();
        }
    }

    return submenuitem;
}

// read tags for a standard menu item
UserMenuItem *UserMenuTree::readXmlMenuentry(const QDomElement &element)
{
    QString menutypename = element.attribute("type");
    UserMenuData::MenuType menutype = UserMenuData::xmlMenuType(menutypename);

    UserMenuItem *menuentryitem = new UserMenuItem(menutype, QString()) ;

    // default values
    QString title;
    QString plaintext;
    QString filename;
    QString parameter;
    QString icon;
    QString shortcut;
    bool needsSelection = false;
    bool useContextMenu = false;
    bool replaceSelection = false;
    bool selectInsertion = false;
    bool insertOutput = false;

    // read values
    if ( element.hasChildNodes() ) {
        QDomElement e = element.firstChildElement();
        while ( !e.isNull()) {
            QString tag = e.tagName();
            QString text = e.text();

            int index = UserMenuData::xmlMenuTag(tag);
            switch (index) {
            case  UserMenuData::XML_TITLE:
                title = text;
                break;
            case  UserMenuData::XML_PLAINTEXT:
                plaintext = text;
                break;
            case  UserMenuData::XML_FILENAME:
                filename = text;
                break;
            case  UserMenuData::XML_PARAMETER:
                parameter = text;
                break;
            case  UserMenuData::XML_ICON:
                icon = text;
                break;
            case  UserMenuData::XML_SHORTCUT:
                shortcut = text;
                break;
            case  UserMenuData::XML_NEEDSSELECTION:
                needsSelection   = str2bool(text);
                break;
            case  UserMenuData::XML_USECONTEXTMENU:
                useContextMenu   = str2bool(text);
                break;
            case  UserMenuData::XML_REPLACESELECTION:
                replaceSelection = str2bool(text);
                break;
            case  UserMenuData::XML_SELECTINSERTION:
                selectInsertion  = str2bool(text);
                break;
            case  UserMenuData::XML_INSERTOUTPUT:
                insertOutput     = str2bool(text);
                break;
            }

            e = e.nextSiblingElement();
        }

        // save values
        menuentryitem->setMenutitle(title);

        // add code newline
        plaintext = UserMenuData::decodeLineFeed(plaintext);
        menuentryitem->setPlaintext(plaintext);

        menuentryitem->setFilename(filename);
        menuentryitem->setParameter(parameter);
        if ( !icon.isEmpty() ) {
            menuentryitem->setMenuicon(icon);
            menuentryitem->setIcon(0,QIcon::fromTheme(icon));
        }
        if ( !shortcut.isEmpty() ) {
            QKeySequence seq = QKeySequence::fromString(shortcut,QKeySequence::PortableText);
            shortcut = seq.toString(QKeySequence::NativeText);

            menuentryitem->setShortcut(shortcut);
            menuentryitem->setText(1,shortcut);
        }
        menuentryitem->setNeedsSelection(needsSelection);
        menuentryitem->setUseContextMenu(useContextMenu);
        menuentryitem->setReplaceSelection(replaceSelection);
        menuentryitem->setSelectInsertion(selectInsertion);
        menuentryitem->setInsertOutput(insertOutput);

        menuentryitem->setText(0,title);
    }

    return menuentryitem;
}


///////////////////////////// write XML //////////////////////////////

bool UserMenuTree::writeXml(const QString &filename)
{
    KILE_DEBUG_MAIN << "write xml file " << filename;

    QFile file(filename);
    if ( !file.open(QFile::WriteOnly | QFile::Text) ) {
        KMessageBox::error(this, i18n("File '%1' could not be opened to save the usermenu file.", filename));
        return false;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setAutoFormattingIndent(2) ;

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("UserMenu");

    for (int i = 0; i < topLevelItemCount(); ++i) {
        writeXmlItem(&xmlWriter, dynamic_cast<UserMenuItem *>(topLevelItem(i)) );
    }
    xmlWriter.writeEndDocument();

    file.close();
    return true;
}

void UserMenuTree::writeXmlItem(QXmlStreamWriter *xml, UserMenuItem *item)
{
    switch (item->menutype()) {
    case UserMenuData::Separator:
        writeXmlSeparator(xml);
        break;
    case UserMenuData::Submenu:
        writeXmlSubmenu(xml,item);
        break;
    default:
        writeXmlMenuentry(xml,item);
        break;
    }
}

// write xml tags for a standard menu item
void UserMenuTree::writeXmlMenuentry(QXmlStreamWriter *xml, UserMenuItem *item)
{
    int menutype = item->menutype();
    QString menutypename = UserMenuData::xmlMenuTypeName(menutype);

    xml->writeStartElement("menu");
    xml->writeAttribute("type", menutypename);

    QString menutitle = ( item->text(0) == EMPTY_MENUENTRY ) ? QString() : item->text(0);
    xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_TITLE),menutitle);

    if ( menutype == UserMenuData::Text ) {
        QString plaintext = item->plaintext();
        if ( !plaintext.isEmpty() ) {
            // encode newline characters
            plaintext = UserMenuData::encodeLineFeed(plaintext);
            xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_PLAINTEXT), plaintext);
        }
    }
    else { /* if ( menutype!=UserMenuData::FileContent || menutype==UserMenuData:Program) */
        // both types use a filename
        QString filename = item->filename();
        if ( !filename.isEmpty() ) {
            xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_FILENAME), filename);

            // but UserMenuItem::Program may have an additional parameter
            if ( menutype == UserMenuData::Program) {
                QString parameter = item->parameter();
                if ( !parameter.isEmpty() ) {
                    xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_PARAMETER), parameter);
                }
            }
        }
    }

    QString icon = item->menuicon();
    if ( !icon.isEmpty() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_ICON),icon);
    }

    QKeySequence seq = QKeySequence::fromString( item->shortcut(), QKeySequence::NativeText );
    if ( !seq.isEmpty() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_SHORTCUT), seq.toString(QKeySequence::PortableText));
    }

    if ( item->needsSelection() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_NEEDSSELECTION), "true");
    }
    if ( item->useContextMenu() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_USECONTEXTMENU), "true");
    }
    if ( item->replaceSelection() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_REPLACESELECTION), "true");
    }
    if ( item->selectInsertion() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_SELECTINSERTION), "true");
    }
    if ( item->insertOutput() ) {
        xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_INSERTOUTPUT), "true");
    }

    xml->writeEndElement();
}

// write xml tags for a submenu
void UserMenuTree::writeXmlSubmenu(QXmlStreamWriter *xml, UserMenuItem *item)
{
    xml->writeStartElement("submenu");

    QString menutitle = item->text(0);
    if ( menutitle == EMPTY_MENUENTRY ) {
        menutitle.clear();
    }
    else if ( menutitle.right(LENGTH_SUBSTITUTE) == EMPTY_SUBMENU ) {
        menutitle = menutitle.left(menutitle.length()-LENGTH_SUBSTITUTE);
    }
    xml->writeTextElement(UserMenuData::xmlMenuTagName(UserMenuData::XML_TITLE),menutitle);

    for ( int i=0; i<item->childCount(); ++i ) {
        writeXmlItem(xml, dynamic_cast<UserMenuItem *>(item->child(i)) );
    }

    xml->writeEndElement();
}

// write xml tag for a separator
void UserMenuTree::writeXmlSeparator(QXmlStreamWriter *xml)
{
    xml->writeStartElement("separator");
    xml->writeEndElement();
}

///////////////////////////// check menutree and set error codes //////////////////////////////

// the complete menutree was build, now check for errors
//  - empty menutitles
//  - useless submenus without children
//  - empty filenames, not existing or not executable files
void UserMenuTree::setErrorCodes()
{
    KILE_DEBUG_MAIN << "check menutree for errors and set error codes ...";

    for (int i = 0; i < topLevelItemCount(); ++i) {
        UserMenuItem *item = dynamic_cast<UserMenuItem *>(topLevelItem(i));
        UserMenuData::MenuType type = item->menutype();

        bool executable = ( type==UserMenuData::Program && isItemExecutable(item->filename()) );
        item->setModelData(executable);

        if ( type != UserMenuData::Separator ) {
            checkMenuTitle(item);
        }
        if ( type == UserMenuData::Submenu ) {
            checkSubmenu(item);
        }
    }
}

void UserMenuTree::checkMenuTitle(UserMenuItem *item)
{
    if ( item->menutitle().isEmpty() ) {
        item->setText(0,EMPTY_MENUENTRY);
        KILE_DEBUG_MAIN << "empty menutitle changed to " << EMPTY_MENUENTRY;
    }
}

void UserMenuTree::checkSubmenu(UserMenuItem *item)
{
    QString menutitle = item->menutitle();
    int children = item->childCount();

    if ( menutitle.isEmpty() ) {
        menutitle = EMPTY_MENUENTRY;
    }
    else if ( children == 0 ) {
        menutitle += EMPTY_SUBMENU;
    }
    item->setText(0,menutitle);

    for ( int i=0; i<children; ++i ) {
        UserMenuItem *child = dynamic_cast<UserMenuItem *>(item->child(i));
        child->setModelData();
        UserMenuData::MenuType type = child->menutype();

        if ( type != UserMenuData::Separator ) {
            checkMenuTitle(child);
        }
        if ( type == UserMenuData::Submenu ) {
            checkSubmenu(child);
        }
    }
}

///////////////////////////// check menutree  //////////////////////////////

// check for errors (true=no errors)
bool UserMenuTree::errorCheck()
{
    KILE_DEBUG_MAIN << "check menutree for errors ...";

    for (int i = 0; i < topLevelItemCount(); ++i) {
        UserMenuItem *item = dynamic_cast<UserMenuItem *>(topLevelItem(i));
        UserMenuData::MenuType type = item->menutype();

        if ( type != UserMenuData::Separator ) {
            if ( item->data(0,Qt::UserRole+2).toInt() != UserMenuItem::MODEL_ERROR_NONE ) {
                return false;
            }
        }

        if ( type == UserMenuData::Submenu ) {
            if ( checkSubmenuError(item) == false ) {
                return false;
            }
        }
    }

    return true;
}

bool UserMenuTree::checkSubmenuError(UserMenuItem *item)
{
    int children = item->childCount();
    for ( int i=0; i<children; ++i ) {
        UserMenuItem *child = dynamic_cast<UserMenuItem *>(item->child(i));
        UserMenuData::MenuType type = child->menutype();

        if ( type != UserMenuData::Separator ) {
            if ( child->data(0,Qt::UserRole+2).toInt() != UserMenuItem::MODEL_ERROR_NONE ) {
                return false;
            }
        }

        if ( type == UserMenuData::Submenu ) {
            if ( checkSubmenuError(child) == false ) {
                return false;
            }
        }
    }

    return true;
}

///////////////////////////// tree ops //////////////////////////////

// insert a standard menu item
bool UserMenuTree::insertMenuItem(QTreeWidgetItem *current, bool below)
{
    QString menulabel = getMenuTitle(i18n("Please enter a label for this menu item:"));
    if ( menulabel.isEmpty() )  {
        return false;
    }

    if ( below ) {
        insertMenuItemBelow(current,UserMenuData::Text,menulabel);
    }
    else {
        insertMenuItemAbove(current,UserMenuData::Text,menulabel);
    }
    return true;
}

// insert a submenu item
bool UserMenuTree::insertSubmenu(QTreeWidgetItem *current, bool below)
{
    QString menulabel = getMenuTitle(i18n("Please enter a label for this submenu:"));
    if ( menulabel.isEmpty() )  {
        return false;
    }

    if ( below ) {
        insertMenuItemBelow(current,UserMenuData::Submenu,menulabel);
    }
    else {
        insertMenuItemAbove(current,UserMenuData::Submenu,menulabel);
    }
    return true;
}

// insert a separator item
bool UserMenuTree::insertSeparator(QTreeWidgetItem *current, bool below)
{
    if(below) {
        insertMenuItemBelow(current,UserMenuData::Separator, QString());
    }
    else {
        insertMenuItemAbove(current,UserMenuData::Separator, QString());
    }
    return true;
}

void UserMenuTree::insertMenuItemAbove(QTreeWidgetItem *current, UserMenuData::MenuType type, const QString &menulabel)
{
    QTreeWidgetItem *parent = ( current ) ? current->parent() : Q_NULLPTR;
    int index = itemIndex(parent,current);

    UserMenuItem *item = new UserMenuItem(type,menulabel);
    insertItem(parent,index,item);

    item->setText(0,menulabel);
    setCurrentItem(item);
}

void UserMenuTree::insertMenuItemBelow(QTreeWidgetItem *current, UserMenuData::MenuType type, const QString &menulabel)
{
    UserMenuItem *item;
    QTreeWidgetItem *parent = ( current ) ? current->parent() : Q_NULLPTR;

    if(!parent) {
        item = new UserMenuItem(this,current,type,menulabel);
    }
    else {
        item = new UserMenuItem(parent,current,type,menulabel);
    }

    item->setText(0,menulabel);
    setCurrentItem(item);
}

void UserMenuTree::insertIntoSubmenu(QTreeWidgetItem *current, UserMenuData::MenuType type)
{
    QString menulabel;
    if ( type == UserMenuData::Text || type == UserMenuData::Submenu ) {
        menulabel = getMenuTitle(i18n("Please enter a label for this entry:"));
        if ( menulabel.isEmpty() ) {
            return;
        }
    }

    UserMenuItem *item = new UserMenuItem(type,menulabel);
    insertItem(current,0,item);
    setCurrentItem(item);
}

// delete an item
void UserMenuTree::itemDelete(QTreeWidgetItem *current)
{
    int children,index;
    QTreeWidgetItem *item, *selectitem;
    QTreeWidgetItem *parent = current->parent();
    if(!parent) {
        children = topLevelItemCount();
        index = indexOfTopLevelItem(current);
        if ( index < children-1 ) {
            selectitem = topLevelItem(index+1);
        }
        else if ( index > 0  ) {
            selectitem = topLevelItem(index-1);
        }
        else {
            selectitem = Q_NULLPTR;
        }

        item = takeTopLevelItem(index);
    }
    else {
        children = parent->childCount();
        index = parent->indexOfChild(current);
        if ( index < children-1 ) {
            selectitem = parent->child(index+1);
        }
        else if ( index > 0  ) {
            selectitem = parent->child(index-1);
        }
        else {
            selectitem = parent;
        }

        item = parent->takeChild(index);
    }

    delete item;

    if(selectitem) {
        setCurrentItem(selectitem);
    }
}

// move an item one position up
void UserMenuTree::itemUp()
{
    QTreeWidgetItem *current = currentItem();
    UserMenuItem *aboveitem = dynamic_cast<UserMenuItem *>(itemAbove(current));
    if (!aboveitem) {
        return;
    }

    bool expanded = current->isExpanded();
    blockSignals(true);

    QTreeWidgetItem *aboveparent = aboveitem->parent();
    int aboveindex = itemIndex(aboveparent,aboveitem);

    UserMenuItem *parent = dynamic_cast<UserMenuItem *>(current->parent());
    int index = itemIndex(parent,current);
    takeItem(parent,current);

    if ( parent!=aboveparent && index!=0 ) {
        aboveindex++;
    }

    if ( parent==aboveparent &&  aboveitem->menutype()==UserMenuData::Submenu ) {
        insertItem(aboveitem,0,current);
    }
    else {
        insertItem(aboveparent,aboveindex,current);
    }

    // update model data of old and new parent, if it has changed
    UserMenuItem *newparent = dynamic_cast<UserMenuItem *>(current->parent());
    if ( parent != newparent ) {
        if ( parent ) {
            parent->setModelData();
            parent->setText(0, parent->updateMenutitle());
        }
        if ( newparent ) {
            newparent->setModelData();
            newparent->setText(0, newparent->updateMenutitle());
        }
    }

    current->setExpanded(expanded);
    setCurrentItem(current);
    blockSignals(false);
}

// move an item one position down
void UserMenuTree::itemDown()
{
    QTreeWidgetItem *current = currentItem();
    bool expanded = current->isExpanded();
    blockSignals(true);

    // get all necessary parameter
    UserMenuItem *parent = dynamic_cast<UserMenuItem *>(current->parent());
    int index = itemIndex(parent,current);
    int children = numChildren(parent);

    // successor exists?
    if ( index < children-1 ) {
        UserMenuItem *successor = dynamic_cast<UserMenuItem *>( itemAtIndex(parent,index+1) );
        takeItem(parent,current);
        if ( successor->menutype() == UserMenuData::Submenu ) {
            successor->insertChild(0,current);
        }
        else {
            insertItem(parent,index+1,current);
        }
    }
    else if ( parent ) {
        QTreeWidgetItem *grandparent = parent->parent();
        int parentindex = itemIndex(grandparent,parent);
        takeItem(parent,current);
        insertItem(grandparent,parentindex+1,current);
    }

    // update model data of old and new parent, if it has changed
    UserMenuItem *newparent = dynamic_cast<UserMenuItem *>(current->parent());
    if ( parent != newparent ) {
        if ( parent ) {
            parent->setModelData();
            parent->setText(0, parent->updateMenutitle());
        }
        if ( newparent ) {
            newparent->setModelData();
            newparent->setText(0, newparent->updateMenutitle());
        }
    }

    current->setExpanded(expanded);
    setCurrentItem(current);
    blockSignals(false);
}

////////////////////////////// delete tree //////////////////////////////

// delete the whole menutree
void  UserMenuTree::deleteMenuTree()
{
    if ( KMessageBox::questionYesNo(this, i18n("Do you really want to clear the complete menutree?") ) == KMessageBox::Yes ) {
        blockSignals(true);
        clear();
        blockSignals(false);
    }
}

////////////////////////////// info //////////////////////////////

// prepare info for en item with errors
void  UserMenuTree::itemInfo(UserMenuItem *item)
{
    int error = item->data(0,Qt::UserRole+2).toInt();

    QStringList list;
    if ( error & UserMenuItem::MODEL_ERROR_EMPTY ) {
        list << i18n("This menuitem has no title.");
    }

    if ( error & UserMenuItem::MODEL_ERROR_SUBMENU ) {
        list << i18n("This submenu item is useless without children.");
    }

    if ( error & UserMenuItem::MODEL_ERROR_TEXT ) {
        list << i18n("This item offers no text to insert.");
    }

    if ( error & UserMenuItem::MODEL_ERROR_FILE_EMPTY ) {
        list << i18n("No file is given for this task.");
    }

    if ( error & UserMenuItem::MODEL_ERROR_FILE_EXIST ) {
        list << i18n("The file for this item does not exist.");
    }

    if ( error & UserMenuItem::MODEL_ERROR_FILE_EXECUTABLE ) {
        list << i18n("The file for this item is not executable.");
    }

    QString msg = i18n("<p><strong>Error:</strong>");
    if ( list.size() == 1 ) {
        msg += "<br/><br/>" + list[0] + "</p>";
    }
    else {
        msg += "<ul>";
        foreach ( const QString &s, list ) {
            msg += "<li>&nbsp;" + s + "</li>";
        }
        msg += "</ul></p>";
    }

    KMessageBox::information(this, msg, i18n("Menutree Error"));
}

////////////////////////////// auxiliary //////////////////////////////

int UserMenuTree::itemIndex(QTreeWidgetItem *parent, QTreeWidgetItem *item)
{
    return ( parent ) ? parent->indexOfChild(item) : indexOfTopLevelItem(item);
}

QTreeWidgetItem *UserMenuTree::itemAtIndex(QTreeWidgetItem *parent, int index)
{
    return ( parent ) ? parent->child(index) : topLevelItem(index);
}

int UserMenuTree::numChildren(QTreeWidgetItem *parent)
{
    return ( parent ) ? parent->childCount() : topLevelItemCount();
}

void UserMenuTree::insertItem(QTreeWidgetItem *parent, int index, QTreeWidgetItem *item)
{
    if ( parent ) {
        parent->insertChild(index,item);
    }
    else {
        insertTopLevelItem(index,item);
    }
}

void UserMenuTree::takeItem(QTreeWidgetItem *parent, QTreeWidgetItem *item)
{
    if ( parent ) {
        int index = parent->indexOfChild(item);
        parent->takeChild(index);
    }
    else {
        int index = indexOfTopLevelItem(item);
        takeTopLevelItem(index);
    }
}

bool UserMenuTree::str2bool(const QString &value)
{
    return ( value == "true" );
}


QString UserMenuTree::getMenuTitle(const QString &title)
{
    bool ok;
    QString value = QInputDialog::getText(this, i18n("Name"), title, QLineEdit::Normal, QString(), &ok);
    return ( ok ) ? value : QString();

}


}

