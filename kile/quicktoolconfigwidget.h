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

#ifndef QUICKTOOLCONFIGWIDGET_H
#define QUICKTOOLCONFIGWIDGET_H

#include <QWidget>

#include "ui_quicktoolconfigwidget.h"

class QuickToolConfigWidget : public QWidget, public Ui::QuickToolConfigWidget
{
	Q_OBJECT

	public:
		QuickToolConfigWidget(QWidget *parent = 0);
		~QuickToolConfigWidget();

	public slots:
		virtual void updateSequence(const QString& sequence);
		virtual void updateConfigs(const QString& tool);

	signals:
		void sequenceChanged(const QString &);

	private:
		QString m_sequence;

	private slots:
		void down();
		void up();
		void remove();
		void add();
		void changed();
};

#endif
