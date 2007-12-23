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

#ifndef GENERALCONFIGWIDGET_H
#define GENERALCONFIGWIDGET_H

#include <QWidget>

#include "ui_generalconfigwidget.h"

class KileWidgetGeneralConfig : public QWidget, public Ui::KileWidgetGeneralConfig
{
	Q_OBJECT

	public:
		KileWidgetGeneralConfig(QWidget *parent = 0);
		~KileWidgetGeneralConfig();
};

#endif
