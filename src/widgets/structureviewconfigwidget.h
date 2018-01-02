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

#ifndef STRUCTUREVIEWCONFIGWIDGET_H
#define STRUCTUREVIEWCONFIGWIDGET_H

#include <QWidget>

#include "ui_structureviewconfigwidget.h"

class KileWidgetStructureViewConfig : public QWidget, public Ui::KileWidgetStructureViewConfig
{
    Q_OBJECT

public:
    KileWidgetStructureViewConfig(QWidget *parent = 0);
    ~KileWidgetStructureViewConfig();
};

#endif
