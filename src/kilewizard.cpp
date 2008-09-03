/***************************************************************************
    begin                : Tue Dec 23 2003
    copyright            : (C) 2003 Jeroen Wijnhout
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

#include "kilewizard.h"

namespace KileDialog
{
	Wizard::Wizard(KConfig *config, QWidget *parent, const char *name, const QString &caption) :
		KDialog(parent),
		m_td(QString(), QString(), QString(), 0, 0, QString()),
		m_config(config)
	{
		setObjectName(name);
		setCaption(caption);
		setModal(true);
		setButtons(Ok | Cancel);
		setDefaultButton(Ok);
		showButtonSeparator(true);
	}

	Wizard::~Wizard()
	{}
}
