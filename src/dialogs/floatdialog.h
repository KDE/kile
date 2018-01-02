/***************************************************************************
    date                 : Jan 28 2005
    version              : 0.10
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FLOATDIALOG_H
#define FLOATDIALOG_H

#include "kilewizard.h"

#include "kileinfo.h"

#include "ui_floatdialog_base.h"

namespace KileDialog
{

class FloatEnvironmentDialog : public Wizard
{
    Q_OBJECT

public:
    FloatEnvironmentDialog(KConfig *config, KileInfo *ki, QWidget *parent);
    ~FloatEnvironmentDialog() {}

public Q_SLOTS:
    void slotAccepted();

private Q_SLOTS:
    void slotEnvironmentClicked();

private:
    Ui::FloatDialog m_FloatDialog;

    QString m_prefix;
    KileInfo *m_ki;
};

}

#endif
