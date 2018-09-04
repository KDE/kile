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

#ifndef PROCESSTOOLCONFIGWIDGET_H
#define PROCESSTOOLCONFIGWIDGET_H

#include <QWidget>

#include "ui_processtoolconfigwidget.h"

class ProcessToolConfigWidget : public QWidget, public Ui::ProcessToolConfigWidget
{
    Q_OBJECT

public:
    explicit ProcessToolConfigWidget(QWidget *parent = 0);
    ~ProcessToolConfigWidget();
};

#endif
