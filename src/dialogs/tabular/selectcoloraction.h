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

#ifndef SELECTCOLORACTION_H
#define SELECTCOLORACTION_H

#include <QAction>

namespace KileDialog {

class SelectColorAction : public QAction {
    Q_OBJECT

public:
    SelectColorAction(const QIcon &icon, const QString &text, QWidget *parent);

private Q_SLOTS:
    void showDialog();

Q_SIGNALS:
    void colorSelected(const QColor &color);
};

}

#endif
