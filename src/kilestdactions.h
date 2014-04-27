/**************************************************************************
*   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)   *
*                 2014 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILESTDACTIONS_H
#define KILESTDACTIONS_H

#include <QWidget>

#include <KActionCollection>

#include "kileinfo.h"

namespace KileStdActions
{
	void setupStdTags(KileInfo *ki, const QObject *receiver, KActionCollection *actionCollection, QWidget *parentWidget);
	QList<QAction*> setupBibTags(const QObject *receiver, KActionCollection *actionCollection);
	void setupMathTags(const QObject *receiver, KActionCollection *actionCollection);

	QAction* createSeparatorAction(QObject *parent);
}

#endif
