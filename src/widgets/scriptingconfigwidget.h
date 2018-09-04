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

#ifndef SCRIPTINGCONFIGWIDGET_H
#define SCRIPTINGCONFIGWIDGET_H

#include <QWidget>

#include "ui_scriptingconfigwidget.h"

class KileWidgetScriptingConfig : public QWidget, public Ui::KileWidgetScriptingConfig
{
    Q_OBJECT

public:
    explicit KileWidgetScriptingConfig(QWidget *parent = 0);
    ~KileWidgetScriptingConfig();
};

#endif
