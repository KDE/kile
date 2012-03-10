/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ***********************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef USERMENUCONFIGWIDGET_H
#define USERMENUCONFIGWIDGET_H

#include <QWidget>

#include "usermenu/usermenu.h"
#include "ui_usermenuconfigwidget.h"

class KileWidgetUsermenuConfig : public QWidget, public Ui::KileWidgetUsermenuConfig
{
	Q_OBJECT

	public:
		KileWidgetUsermenuConfig(KileMenu::UserMenu *usermenu, QWidget *parent = NULL);
		~KileWidgetUsermenuConfig();

		void writeConfig();

	private Q_SLOTS:
		void slotInstallClicked();
		void slotRemoveClicked();

	private:
		KileMenu::UserMenu *m_usermenu;

		void setXmlFile(const QString &file);

};

#endif
