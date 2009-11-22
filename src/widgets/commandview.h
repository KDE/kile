/***************************************************************************************
    begin                : June 12 2009
    copyright            : (C) 2009 dani
 ***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMMANDVIEW_H
#define COMMANDVIEW_H

#include <QListWidget>
#include <QToolBox>

#include "kileinfo.h"
#include "codecompletion.h"

class QMouseEvent;

namespace KileWidget {

class CommandView : public QListWidget
{
	Q_OBJECT

	public:
		explicit CommandView(QWidget *parent, const QString &title, const char *name = NULL);
		~CommandView();

	public:
		QString m_title;

};

class CommandViewToolBox : public QToolBox
{
	Q_OBJECT

	public:
		explicit CommandViewToolBox(KileInfo *ki, QWidget *parent, const char *name = NULL);
		~CommandViewToolBox();

		void readCommandViewFiles();

	Q_SIGNALS:
		void sendText(const QString &text);

	private Q_SLOTS:
		void slotItemActivated(QListWidgetItem *item);

	private:
		void clearItems();
		
		QMap<QString,CommandView*> *m_viewmap;
		int m_activeMaps;

		KileInfo *m_ki;
		KileCodeCompletion::LaTeXCompletionModel *m_latexCompletionModel;
};

}

#endif
