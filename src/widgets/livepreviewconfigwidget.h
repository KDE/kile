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

#ifndef LIVEPREVIEWCONFIGWIDGET_H
#define LIVEPREVIEWCONFIGWIDGET_H

#include <QWidget>

#include <KConfig>

#include "ui_livepreviewconfigwidget.h"

class KileWidgetLivePreviewConfig : public QWidget, public Ui::KileWidgetLivePreviewConfig
{
    Q_OBJECT

public:
    explicit KileWidgetLivePreviewConfig(KConfig *config, QWidget *parent = Q_NULLPTR);
    ~KileWidgetLivePreviewConfig();

    void readConfig();
    void writeConfig();

protected:
    KConfig *m_config;
};

#endif
