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
      QTableWidgetItem(QIcon::fromTheme(QStringLiteral("format-justify-left")), QStringLiteral("l")),
      m_Alignment(Qt::AlignLeft),
      m_InsertBefore(false),
      m_InsertAfter(false),
      m_SuppressSpace(false),
      m_DontSuppressSpace(false),
      m_hasXAlignment(false)
{
    m_Popup = new QMenu(parent);
    m_Popup->addAction(QIcon::fromTheme(QStringLiteral("format-justify-left")), i18n("Align Left"), this, SLOT(slotAlignLeft()));
    m_Popup->addAction(QIcon::fromTheme(QStringLiteral("format-justify-center")), i18n("Align Center"), this, SLOT(slotAlignCenter()));
    m_Popup->addAction(QIcon::fromTheme(QStringLiteral("format-justify-right")), i18n("Align Right"), this, SLOT(slotAlignRight()));
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
        text += QLatin1Char('@');
    } else if(m_DontSuppressSpace) {
        text += QLatin1Char('!');
    }
    if(m_InsertBefore) {
        text += QLatin1Char('>');
    }

    switch(m_Alignment) {
    case Qt::AlignLeft:
        text += QLatin1Char('l');
        break;
    case Qt::AlignHCenter:
        text += QLatin1Char('c');
        break;
    case Qt::AlignRight:
        text += QLatin1Char('r');
        break;
    case AlignP:
        text += QLatin1Char('p');
        break;
    case AlignB:
        text += QLatin1Char('b');
        break;
    case AlignM:
        text += QLatin1Char('m');
        break;
    case AlignX:
        text += QLatin1Char('X');
        break;
    }

    if(m_InsertAfter) {
        text += QLatin1Char('<');
    }

    setText(text);
}

inline QIcon TabularHeaderItem::iconForAlignment(int alignment) const
{
    switch(alignment) {
    case Qt::AlignLeft:
        return QIcon::fromTheme(QStringLiteral("format-justify-left"));
    case Qt::AlignHCenter:
        return QIcon::fromTheme(QStringLiteral("format-justify-center"));
    case Qt::AlignRight:
        return QIcon::fromTheme(QStringLiteral("format-justify-right"));
    default:
        return QIcon();
    }
}

void TabularHeaderItem::slotAlignLeft()
{
    setAlignment(Qt::AlignLeft);
    Q_EMIT alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotAlignCenter()
{
    setAlignment(Qt::AlignHCenter);
    Q_EMIT alignColumn(Qt::AlignHCenter);
}

void TabularHeaderItem::slotAlignRight()
{
    setAlignment(Qt::AlignRight);
    Q_EMIT alignColumn(Qt::AlignRight);
}

void TabularHeaderItem::slotAlignP()
{
    setAlignment(AlignP);
    Q_EMIT alignColumn(AlignP);
}

void TabularHeaderItem::slotAlignB()
{
    setAlignment(AlignB);
    Q_EMIT alignColumn(AlignB);
}

void TabularHeaderItem::slotAlignM()
{
    setAlignment(AlignM);
    Q_EMIT alignColumn(AlignM);
}

void TabularHeaderItem::slotAlignX()
{
    setAlignment(AlignX);
    Q_EMIT alignColumn(AlignX);
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

