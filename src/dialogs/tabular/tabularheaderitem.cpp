/********************************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
    copyright            : (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tabularheaderitem.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

#include <KLocalizedString>

namespace KileDialog {

TabularHeaderItem::TabularHeaderItem(QWidget *parent)
    : QObject(parent),
      QTableWidgetItem(QIcon::fromTheme("format-justify-left"), "l"),
      m_Alignment(Qt::AlignLeft),
      m_InsertBefore(false),
      m_InsertAfter(false),
      m_SuppressSpace(false),
      m_DontSuppressSpace(false),
      m_hasXAlignment(false)
{
    m_Popup = new QMenu(parent);
    m_Popup->addAction(QIcon::fromTheme("format-justify-left"), i18n("Align Left"), this, SLOT(slotAlignLeft()));
    m_Popup->addAction(QIcon::fromTheme("format-justify-center"), i18n("Align Center"), this, SLOT(slotAlignCenter()));
    m_Popup->addAction(QIcon::fromTheme("format-justify-right"), i18n("Align Right"), this, SLOT(slotAlignRight()));
    m_Popup->addAction(i18n("p{w} Alignment"), this, SLOT(slotAlignP()));
    m_Popup->addAction(i18n("b{w} Alignment"), this, SLOT(slotAlignB()));
    m_Popup->addAction(i18n("m{w} Alignment"), this, SLOT(slotAlignM()));
    m_acXAlignment = m_Popup->addAction(i18n("X Alignment"), this, SLOT(slotAlignX()));
    m_Popup->addSeparator();
    m_acDeclPre = m_Popup->addAction(i18n("Insert Before Declaration"), this, SLOT(slotDeclPre()));
    m_acDeclPost = m_Popup->addAction(i18n("Insert After Declaration"), this, SLOT(slotDeclPost()));
    m_acDeclAt = m_Popup->addAction(i18n("Suppress Space"), this, SLOT(slotDeclAt()));
    m_acDeclBang = m_Popup->addAction(i18n("Do not Suppress Space"), this, SLOT(slotDeclBang()));

    m_acDeclPre->setCheckable(true);
    m_acDeclPost->setCheckable(true);
    m_acDeclAt->setCheckable(true);
    m_acDeclBang->setCheckable(true);
}

void TabularHeaderItem::setAlignment(int alignment)
{
    m_Alignment = alignment;
    format();
}

int TabularHeaderItem::alignment() const
{
    return m_Alignment;
}

bool TabularHeaderItem::insertBefore() const
{
    return m_InsertBefore;
}

bool TabularHeaderItem::insertAfter() const
{
    return m_InsertAfter;
}

bool TabularHeaderItem::suppressSpace() const
{
    return m_SuppressSpace;
}

bool TabularHeaderItem::dontSuppressSpace() const
{
    return m_DontSuppressSpace;
}

void TabularHeaderItem::setHasXAlignment(bool hasXAlignment)
{
    m_hasXAlignment = hasXAlignment;
    if(!hasXAlignment && m_Alignment == AlignX) {
        slotAlignLeft();
    }
}

bool TabularHeaderItem::hasXAlignment() const
{
    return m_hasXAlignment;
}

QMenu* TabularHeaderItem::popupMenu() const
{
    m_acXAlignment->setVisible(m_hasXAlignment);
    return m_Popup;
}

void TabularHeaderItem::format()
{
    setIcon(iconForAlignment(m_Alignment));

    QString text;

    if(m_SuppressSpace) {
        text += '@';
    } else if(m_DontSuppressSpace) {
        text += '!';
    }
    if(m_InsertBefore) {
        text += '>';
    }

    switch(m_Alignment) {
    case Qt::AlignLeft:
        text += 'l';
        break;
    case Qt::AlignHCenter:
        text += 'c';
        break;
    case Qt::AlignRight:
        text += 'r';
        break;
    case AlignP:
        text += 'p';
        break;
    case AlignB:
        text += 'b';
        break;
    case AlignM:
        text += 'm';
        break;
    case AlignX:
        text += 'X';
        break;
    }

    if(m_InsertAfter) {
        text += '<';
    }

    setText(text);
}

inline QIcon TabularHeaderItem::iconForAlignment(int alignment) const
{
    switch(alignment) {
    case Qt::AlignLeft:
        return QIcon::fromTheme("format-justify-left");
    case Qt::AlignHCenter:
        return QIcon::fromTheme("format-justify-center");
    case Qt::AlignRight:
        return QIcon::fromTheme("format-justify-right");
    default:
        return QIcon();
    }
}

void TabularHeaderItem::slotAlignLeft()
{
    setAlignment(Qt::AlignLeft);
    emit alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotAlignCenter()
{
    setAlignment(Qt::AlignHCenter);
    emit alignColumn(Qt::AlignHCenter);
}

void TabularHeaderItem::slotAlignRight()
{
    setAlignment(Qt::AlignRight);
    emit alignColumn(Qt::AlignRight);
}

void TabularHeaderItem::slotAlignP()
{
    setAlignment(AlignP);
    emit alignColumn(AlignP);
}

void TabularHeaderItem::slotAlignB()
{
    setAlignment(AlignB);
    emit alignColumn(AlignB);
}

void TabularHeaderItem::slotAlignM()
{
    setAlignment(AlignM);
    emit alignColumn(AlignM);
}

void TabularHeaderItem::slotAlignX()
{
    setAlignment(AlignX);
    emit alignColumn(AlignX);
}

void TabularHeaderItem::slotDeclPre()
{
    m_InsertBefore = m_acDeclPre->isChecked();
    format();
}

void TabularHeaderItem::slotDeclPost()
{
    m_InsertAfter = m_acDeclPost->isChecked();
    format();
}

void TabularHeaderItem::slotDeclAt()
{
    m_SuppressSpace = m_acDeclAt->isChecked();
    if(m_SuppressSpace) {
        m_DontSuppressSpace = false;
        m_acDeclBang->setChecked(false);
    }
    format();
}

void TabularHeaderItem::slotDeclBang()
{
    m_DontSuppressSpace = m_acDeclBang->isChecked();
    if(m_DontSuppressSpace) {
        m_SuppressSpace = false;
        m_acDeclAt->setChecked(false);
    }
    format();
}

}

