/**************************************************************************
*   Copyright (C) 2011-2012 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "appearanceconfigwidget.h"

#include <config.h>

KileWidgetAppearanceConfig::KileWidgetAppearanceConfig(KConfig *config, QWidget *parent)
    : QWidget(parent),
      m_config(config)
{
    setupUi(this);
}

KileWidgetAppearanceConfig::~KileWidgetAppearanceConfig()
{
}

void KileWidgetAppearanceConfig::readConfig()
{
}

void KileWidgetAppearanceConfig::writeConfig()
{
}


