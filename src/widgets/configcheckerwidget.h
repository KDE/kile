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

#ifndef CONFIGCHECKERWIDGET_H
#define CONFIGCHECKERWIDGET_H

#include <QLabel>
#include <QProgressBar>
#include <QWidget>

#include <QListWidget>

#include "ui_configcheckerwidget.h"

class ConfigCheckerWidget : public QWidget, public Ui::ConfigCheckerWidget
{
    Q_OBJECT

public:
    ConfigCheckerWidget(QWidget *parent = 0);
    ~ConfigCheckerWidget();

    QProgressBar* progressBar();
    QLabel* label();
    QListWidget* listWidget();

};

#endif
