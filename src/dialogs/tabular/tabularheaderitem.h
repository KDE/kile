/***************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken
    email                : msoeken@informatik.uni-bremen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TABULARHEADERITEM_H
#define TABULARHEADERITEM_H

#include <QTableWidgetItem>

class QAction;
class QIcon;
class QMenu;

namespace KileDialog {

class TabularHeaderItem : public QObject, public QTableWidgetItem {
    Q_OBJECT

public:
    enum { AlignP = 0x0200, AlignB = 0x0400, AlignM = 0x0800, AlignX = 0x1000 };

    TabularHeaderItem(QWidget *parent);

    void setAlignment(int alignment);
    int alignment() const;

    bool insertBefore() const;
    bool insertAfter() const;
    bool suppressSpace() const;
    bool dontSuppressSpace() const;

    void setHasXAlignment(bool hasXAlignment);
    bool hasXAlignment() const;

    QMenu* popupMenu() const;

private:
    void format();
    QIcon iconForAlignment(int alignment) const;

private Q_SLOTS:
    void slotAlignLeft();
    void slotAlignCenter();
    void slotAlignRight();
    void slotAlignP();
    void slotAlignB();
    void slotAlignM();
    void slotAlignX();
    void slotDeclPre();
    void slotDeclPost();
    void slotDeclAt();
    void slotDeclBang();

Q_SIGNALS:
    void alignColumn(int alignment);

private:
    int m_Alignment;
    bool m_InsertBefore, m_InsertAfter, m_SuppressSpace, m_DontSuppressSpace;
    QMenu *m_Popup;
    QAction *m_acXAlignment,
            *m_acDeclPre, *m_acDeclPost, *m_acDeclAt, *m_acDeclBang;
    bool m_hasXAlignment;
};

}

#endif
