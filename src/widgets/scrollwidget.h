/**************************************************************************************
    Copyright (C) 2016 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QScrollArea>
#include <QSize>

namespace KileWidget {

class ScrollWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit ScrollWidget(QWidget *parent = Q_NULLPTR);
    virtual ~ScrollWidget();

    /**
     * Returns the preferred size if it has been set, otherwise the widget's size hint
     **/
    virtual QSize sizeHint() const;

    /**
     * Set the preferred size of this widget, which will be returned by 'sizeHint'
     **/
    QSize getPreferredSize() const;
    void setPreferredSize(const QSize& size);

protected:
    QSize m_preferredSize;
};

}

#endif
