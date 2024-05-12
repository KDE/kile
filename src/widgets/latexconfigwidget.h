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

class KileWidgetLatexConfig : public QWidget, public Ui::KileWidgetLatexConfig {
    Q_OBJECT

public:
    explicit KileWidgetLatexConfig(KConfig* config, KileInfo* ki, QWidget* parent = Q_NULLPTR);
    ~KileWidgetLatexConfig();

    void readConfig();
    void writeConfig();

protected Q_SLOTS:
    void slotConfigure();

protected:
    void initModifierComboBox(QComboBox* combobox, int initial);

    KConfig* m_config;
    KileInfo* m_ki;
    KileDocument::LatexCommands* m_commands;
    QMap<int, QString> m_modifiers;
};

#endif
