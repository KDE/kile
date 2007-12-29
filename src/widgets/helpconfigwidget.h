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

#ifndef HELPCONFIGWIDGET_H
#define HELPCONFIGWIDGET_H

#include <QWidget>

#include "kilehelp.h"

#include "ui_helpconfigwidget.h"

class KileWidgetHelpConfig : public QWidget, public Ui::KileWidgetHelpConfig
{
	Q_OBJECT

	public:
		KileWidgetHelpConfig(QWidget *parent = 0);
		~KileWidgetHelpConfig();

		void setHelp(KileHelp::Help *help);

	protected slots:
		void slotConfigure();

	protected:
		KileHelp::Help *m_help;
};

#endif
