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

#include "selectcoloraction.h"

#include <QVBoxLayout>
#include <QWidgetAction>

#include <QColorDialog>
#include <QIcon>
#include <KLocalizedString>
#include <QMenu>
#include <QPushButton>

namespace KileDialog {

SelectColorAction::SelectColorAction(const QIcon &icon, const QString &text, QWidget *parent)
    : QAction(icon, text, parent)
{
    connect(this, &QAction::triggered, this, &SelectColorAction::showDialog);
}

void SelectColorAction::showDialog()
{
    QColor color = QColorDialog::getColor();
    if (color.isValid()) {
        emit colorSelected(color);
    }
}

}
