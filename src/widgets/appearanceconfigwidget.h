/**************************************************************************
*   Copyright (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef APPEARANCECONFIGWIDGET_H
#define APPEARANCECONFIGWIDGET_H

#include <QWidget>

#include <KConfig>

#include "ui_appearanceconfigwidget.h"

class KileWidgetAppearanceConfig : public QWidget, public Ui::KileWidgetAppearanceConfig
{
    Q_OBJECT

public:
    KileWidgetAppearanceConfig(KConfig *config, QWidget *parent = Q_NULLPTR);
    ~KileWidgetAppearanceConfig();

    void readConfig();
    void writeConfig();

protected:
    KConfig *m_config;
};

#endif
