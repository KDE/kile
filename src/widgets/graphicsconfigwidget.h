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

#ifndef GRAPHICSCONFIGWIDGET_H
#define GRAPHICSCONFIGWIDGET_H

#include <QWidget>

#include "ui_graphicsconfigwidget.h"

class KileWidgetGraphicsConfig : public QWidget, public Ui::KileWidgetGraphicsConfig
{
    Q_OBJECT

public:
    KileWidgetGraphicsConfig(QWidget *parent = 0);
    ~KileWidgetGraphicsConfig();
};

#endif
