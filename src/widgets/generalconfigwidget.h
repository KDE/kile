/**************************************************************************
*   Copyright (C) 2007-2011 by Michel Ludwig (michel.ludwig@kdemail.net)  *
*                 2011 by Felix Mauch (felix_mauch@web.de)                *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef GENERALCONFIGWIDGET_H
#define GENERALCONFIGWIDGET_H

#include <QWidget>

#include "ui_generalconfigwidget.h"

class KileWidgetGeneralConfig : public QWidget, public Ui::KileWidgetGeneralConfig
{
    Q_OBJECT

public:
    explicit KileWidgetGeneralConfig(QWidget *parent = Q_NULLPTR);
    ~KileWidgetGeneralConfig();

private Q_SLOTS:
    void selectDefaultProjectLocation();
};

#endif
