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

#ifndef LATEXCONFIGWIDGET_H
#define LATEXCONFIGWIDGET_H

#include <QWidget>

#include "kconfig.h"

#include "latexcmd.h"

#include "ui_latexconfigwidget.h"

class KileWidgetLatexConfig : public QWidget, public Ui::KileWidgetLatexConfig
{
    Q_OBJECT

public:
    KileWidgetLatexConfig(QWidget *parent = 0);
    ~KileWidgetLatexConfig();

    void setLatexCommands(KConfig *config, KileDocument::LatexCommands *commands);

protected Q_SLOTS:
    void slotConfigure();

protected:
    KConfig *m_config;
    KileDocument::LatexCommands *m_commands;
};

#endif
