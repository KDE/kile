/***************************************************************************
date                 : Dec 23 2007
version              : 0.22
copyright            : (C) 2004 by Mathias Soeken
email                : msoeken@tzi.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "categorycombobox.h"

#include <QAbstractItemView>
#include <QItemDelegate>
#include <QPainter>
#include <QStandardItem>
#include <QStandardItemModel>

namespace KileWidget {

class CategoryComboBoxDelegate : public QItemDelegate {
public:
    void paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        bool category = index.model()->data(index, KileWidget::CategoryComboBox::Category).toBool();

        if (category) {
            painter->setPen(Qt::gray);
            painter->drawLine(option.rect.x(), option.rect.y() + (option.rect.height() / 2), option.rect.x() + 4, option.rect.y() + (option.rect.height() / 2));

            QFont newFont = painter->font();
            newFont.setPointSize(8);
            painter->setFont(newFont);

            QRect boundingRect;
            painter->drawText(QRect(option.rect.x() + 5, option.rect.y(), option.rect.width() - 5, option.rect.height()), Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index, Qt::DisplayRole).toString(), &boundingRect);

            painter->drawLine(boundingRect.right() + 1, option.rect.y() + (option.rect.height() / 2), option.rect.right(), option.rect.y() + (option.rect.height() / 2));
        }
        else {
            QItemDelegate::paint(painter, option, index);
        }
    }
};

CategoryComboBox::CategoryComboBox(QWidget *parent) : KComboBox(parent)
{
    setItemDelegate(new KileWidget::CategoryComboBoxDelegate());
    //view()->setAlternatingRowColors( true );
}

CategoryComboBox::CategoryComboBox(bool rw, QWidget *parent) : KComboBox(rw, parent)
{
    setItemDelegate(new KileWidget::CategoryComboBoxDelegate());
    //view()->setAlternatingRowColors( true );
}

CategoryComboBox::~CategoryComboBox()
{
}

void CategoryComboBox::addCategoryItem(const QString &text)
{
    addItem(text);

    // row of the item
    int row = count() - 1;

    QStandardItemModel *pModel = qobject_cast<QStandardItemModel*>(model());
    if (pModel) {
        QStandardItem *item = pModel->item(row, 0);
        if (item) {
            item->setData(true, KileWidget::CategoryComboBox::Category);

            // make the item unselectable
            item->setFlags(0);
        }
    }

    if (currentIndex() == row) {
        setCurrentIndex(-1);
    }
}

} // namespace KileWidget
