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

#ifndef USERMENUTREE_H
#define USERMENUTREE_H

#include <QStyledItemDelegate>
#include <QContextMenuEvent>
#include <QPainter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QXmlStreamWriter>

namespace KileMenu {

class MenuentryDelegate : public QStyledItemDelegate {
public:
    MenuentryDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index) const override;
};


class UserMenuTree : public QTreeWidget
{
    Q_OBJECT

public:
    explicit UserMenuTree(QWidget *parent);
    ~UserMenuTree() {}

    bool insertMenuItem(QTreeWidgetItem *current, bool below=true);
    bool insertSubmenu(QTreeWidgetItem *current, bool below=true);
    bool insertSeparator(QTreeWidgetItem *current, bool below=true);

    void itemDelete(QTreeWidgetItem *current);
    void itemUp();
    void itemDown();
    void contextMenuRequested(const QPoint &pos);

    bool readXml(const QString &filename);
    bool writeXml(const QString &filename);
    bool errorCheck();
    bool isItemExecutable(const QString &filename);

    bool isEmpty();

private:
    UserMenuItem *m_popupItem;

    void setErrorCodes();
    bool checkSubmenuError(UserMenuItem *item);

    QStringList m_envPathlist;
    void initEnvPathlist();

    void insertMenuItemAbove(QTreeWidgetItem *current, UserMenuData::MenuType type, const QString &menulabel);
    void insertMenuItemBelow(QTreeWidgetItem *current, UserMenuData::MenuType type, const QString &menulabel);
    void insertIntoSubmenu(QTreeWidgetItem *current, UserMenuData::MenuType type);

    void deleteMenuTree();

    int  itemIndex(QTreeWidgetItem *parent, QTreeWidgetItem *item);
    QTreeWidgetItem *itemAtIndex(QTreeWidgetItem *parent, int index);
    int numChildren(QTreeWidgetItem *parent);

    void insertItem(QTreeWidgetItem *parent, int index, QTreeWidgetItem *item);
    void takeItem(QTreeWidgetItem *parent, QTreeWidgetItem *item);

    UserMenuItem *readXmlSeparator();
    UserMenuItem *readXmlSubmenu(const QDomElement &element);
    UserMenuItem *readXmlMenuentry(const QDomElement &element);

    void writeXmlItem(QXmlStreamWriter *xml, UserMenuItem *item);
    void writeXmlMenuentry(QXmlStreamWriter *xml, UserMenuItem *item);
    void writeXmlSubmenu(QXmlStreamWriter *xml, UserMenuItem *item);
    void writeXmlSeparator(QXmlStreamWriter *xml);

    void checkMenuTitle(UserMenuItem *item);
    void checkSubmenu(UserMenuItem *item);

    bool str2bool(const QString &value);
    QString getMenuTitle(const QString &title);
    void itemInfo(UserMenuItem *item);
};


}

#endif
