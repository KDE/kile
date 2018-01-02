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

#include "tabularcelldelegate.h"

#include <QApplication>
#include <QLineEdit>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "tabularcell.h"

namespace KileDialog {

TabularCellDelegate::TabularCellDelegate(QTableWidget *parent)
    : QStyledItemDelegate(parent),
      m_Table(parent)
{
}

void TabularCellDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if(option.state & QStyle::State_Selected || option.state & QStyle::State_MouseOver) {
        QStyledItemDelegate::paint(painter, option, index);
    } else {
        painter->fillRect(option.rect, qvariant_cast<QBrush>(index.model()->data(index, Qt::BackgroundRole)));
        QFont oldFont = painter->font();
        painter->setFont(qvariant_cast<QFont>(index.model()->data(index, Qt::FontRole)));
        QRect textRect(option.rect.x() + 3, option.rect.y(), option.rect.width() - 6, option.rect.height());
        QApplication::style()->drawItemText(painter, textRect,
                                            index.model()->data(index, Qt::TextAlignmentRole).toInt(),
                                            QPalette(qvariant_cast<QBrush>(index.model()->data(index, Qt::ForegroundRole)).color()),
                                            true, index.model()->data(index, Qt::DisplayRole).toString(), QPalette::Window );
        painter->setFont(oldFont);
    }

    int rowCount = m_Table->rowCount();
    int columnCount = m_Table->columnCount();

    int row = index.row();
    int column = index.column();

    TabularCell *cell = static_cast<TabularCell*>(m_Table->item(row, column));

    if(column == 0) {
        painter->setPen(cell->border() & TabularCell::Left ? Qt::black : Qt::lightGray);
        painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
    }

    if(row == 0) {
        painter->setPen(cell->border() & TabularCell::Top ? Qt::black : Qt::lightGray);
        painter->drawLine(option.rect.topLeft(), option.rect.topRight());
    }

    bool right = (cell->border() & TabularCell::Right)
                 || (column < (columnCount - 1) && static_cast<TabularCell*>(m_Table->item(row, column + 1))->border() & TabularCell::Left);
    painter->setPen(right ? Qt::black : Qt::lightGray);
    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());

    bool bottom = (cell->border() & TabularCell::Bottom)
                  || (row < (rowCount - 1) && static_cast<TabularCell*>(m_Table->item(row + 1, column))->border() & TabularCell::Top);
    painter->setPen(bottom ? Qt::black : Qt::lightGray);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
}

QWidget* TabularCellDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    return editor;
}

void TabularCellDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QBrush bgBrush = qvariant_cast<QBrush>(index.model()->data(index, Qt::BackgroundRole));
    QBrush fgBrush = qvariant_cast<QBrush>(index.model()->data(index, Qt::ForegroundRole));
    QFont font = qvariant_cast<QFont>(index.model()->data(index, Qt::FontRole));
    int alignment = index.model()->data(index, Qt::TextAlignmentRole).toInt();
    QLineEdit *edit = static_cast<QLineEdit*>(editor);
    QString styleSheet;
    if(bgBrush.style() != Qt::NoBrush) {
        styleSheet += "background-color:" + bgBrush.color().name() + ';';
    }
    if(fgBrush.style() != Qt::NoBrush) {
        styleSheet += "color:" + fgBrush.color().name() + ';';
    }
    edit->setStyleSheet(styleSheet);
    edit->setAlignment((Qt::Alignment)alignment);
    edit->setFont(font);
    edit->setText(value);
}

void TabularCellDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
    QLineEdit *edit = static_cast<QLineEdit*>(editor);
    QString value = edit->text();
    model->setData(index, value, Qt::EditRole);
}

void TabularCellDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

}
