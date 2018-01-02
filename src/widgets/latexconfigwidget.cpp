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

#include "latexconfigwidget.h"

#include "dialogs/latexcommanddialog.h"

KileWidgetLatexConfig::KileWidgetLatexConfig(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
    connect(m_pbCommands, SIGNAL(clicked()), this, SLOT(slotConfigure()));
}

KileWidgetLatexConfig::~KileWidgetLatexConfig()
{
}

void KileWidgetLatexConfig::slotConfigure()
{
    KileDialog::LatexCommandsDialog *dlg = new KileDialog::LatexCommandsDialog(m_config, m_commands, this);
    dlg->exec();
    delete dlg;
}


void KileWidgetLatexConfig::setLatexCommands(KConfig *config, KileDocument::LatexCommands *commands)
{
    m_config = config;
    m_commands = commands;
}

