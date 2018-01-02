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

#ifndef LATEXTOOLCONFIGWIDGET_H
#define LATEXTOOLCONFIGWIDGET_H

#include <QWidget>

#include "ui_latextoolconfigwidget.h"

class LaTeXToolConfigWidget : public QWidget, public Ui::LaTeXToolConfigWidget
{
    Q_OBJECT

public:
    LaTeXToolConfigWidget(QWidget *parent = 0);
    ~LaTeXToolConfigWidget();
};

#endif
