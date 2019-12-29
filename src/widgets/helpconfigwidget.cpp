/**************************************************************************
*   Copyright (C) 2007-2019 by Michel Ludwig (michel.ludwig@kdemail.net)  *
*                 2011 by Felix Mauch (felix_mauch@web.de)                *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widgets/helpconfigwidget.h"

KileWidgetHelpConfig::KileWidgetHelpConfig(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    connect(m_pbConfigure, SIGNAL(clicked()), this, SLOT(slotConfigure()));
}

KileWidgetHelpConfig::~KileWidgetHelpConfig()
{
}

void KileWidgetHelpConfig::slotConfigure()
{
    m_help->userHelpDialog();
}

void KileWidgetHelpConfig::setHelp(KileHelp::Help *help)
{
    m_help = help;
}
