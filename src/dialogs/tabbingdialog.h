/***************************************************************************
    begin                : dim jui 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, (C) 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TABBINGDIALOG_H
#define TABBINGDIALOG_H

#include "kilewizard.h"
#include "kileinfo.h"

#include "ui_tabbingdialog_base.h"

/**
 * @author Pascal Brachet
 * @author Jeroen Wijnhout
 */
namespace KileDialog
{

class QuickTabbing : public Wizard
{
    Q_OBJECT

public:
    QuickTabbing(KConfig *config, KileInfo *info, QWidget *parent = Q_NULLPTR,
                 const char *name = Q_NULLPTR, const QString &caption = QString());
    ~QuickTabbing();

public Q_SLOTS:
    void onAccepted();

private:
    KileInfo *m_info;
    Ui::TabbingDialog m_tabbingDialog;
};

}

#endif
