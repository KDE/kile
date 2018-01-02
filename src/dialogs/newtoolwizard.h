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

#ifndef NEWTOOLWIZARD_H
#define NEWTOOLWIZARD_H

#include "kassistantdialog.h"

#include "ui_newtoolwizard_class_page.h"
#include "ui_newtoolwizard_toolname_page.h"

class NewToolWizard : public KAssistantDialog, public Ui::NewToolWizardToolNamePage, public Ui::NewToolWizardClassPage
{
    Q_OBJECT

public:
    explicit NewToolWizard(QWidget *parent = 0, Qt::WindowFlags fl = 0);

    virtual QString customTool();
    virtual QString toolName();
    virtual QString parentTool();

protected Q_SLOTS:
    void nameChanged(const QString & name);
    void slotCurrentPageChanged(KPageWidgetItem* current, KPageWidgetItem* before);

private:
    QStringList m_toolList;
    KPageWidgetItem *toolNamePage, *classPage;
};

#endif
