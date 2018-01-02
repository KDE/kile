/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MAINTOOLCONFIGWIDGET_H
#define MAINTOOLCONFIGWIDGET_H

#include <QWidget>

#include "ui_maintoolconfigwidget.h"

class ToolConfigWidget : public QWidget, public Ui::ToolConfigWidget
{
    Q_OBJECT

public:
    ToolConfigWidget(QWidget *parent = 0);
    ~ToolConfigWidget();
};

#endif
