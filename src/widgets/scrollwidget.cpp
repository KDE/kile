/**************************************************************************************
    Copyright (C) 2016 by Michel Ludwig (michel.ludwig@kdemail.net)
              (C) 2016 by Elvis Angelaccio (elvis.angelaccio@kde.org)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "scrollwidget.h"

#include <QScrollArea>

#include "kiledebug.h"

namespace KileWidget {


ScrollWidget::ScrollWidget(QWidget *parent)
    : QScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
}

ScrollWidget::~ScrollWidget()
{
}

QSize ScrollWidget::getPreferredSize() const
{
    return m_preferredSize;
}

void ScrollWidget::setPreferredSize(const QSize& size)
{
    m_preferredSize = size;
}

QSize ScrollWidget::sizeHint() const
{
    if(m_preferredSize.isValid()) {
        return m_preferredSize;
    }
    else if (widget()) {
        return widget()->sizeHint();
    }

    return {};
}

}
