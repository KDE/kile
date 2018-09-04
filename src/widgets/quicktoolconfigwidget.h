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

#ifndef QUICKTOOLCONFIGWIDGET_H
#define QUICKTOOLCONFIGWIDGET_H

#include <QWidget>

#include "ui_quicktoolconfigwidget.h"

class QuickToolConfigWidget : public QWidget, public Ui::QuickToolConfigWidget
{
    Q_OBJECT

public:
    explicit QuickToolConfigWidget(QWidget *parent = 0);
    ~QuickToolConfigWidget();

public Q_SLOTS:
    virtual void updateSequence(const QString& sequence);
    virtual void updateConfigs(const QString& tool);

Q_SIGNALS:
    void sequenceChanged(const QString &);

private:
    QString m_sequence;
    QString m_currentDefaultConfig;

private Q_SLOTS:
    void down();
    void up();
    void remove();
    void add();
    void changed();
};

#endif
