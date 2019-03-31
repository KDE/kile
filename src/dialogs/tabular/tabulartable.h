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

#ifndef TABULARTABLE_H
#define TABULARTABLE_H

#include <QTableWidget>

class QEvent;
class QMouseEvent;
class QPaintEvent;

namespace KileDialog {

class TabularTable : public QTableWidget {
    Q_OBJECT

public:
    explicit TabularTable(QWidget *parent = 0);

public Q_SLOTS:
    void paste();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

Q_SIGNALS:
    void rowAppended();
    void colAppended();

private:
    SelectionMode m_DefaultMode;
    QPoint m_ManualBorderPosition;
    QPoint m_ManualBorderStart;
    QPoint m_HoverPosition;
    QTableWidgetItem *m_LastItem;
};

}

#endif
