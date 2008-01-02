/***************************************************************************
    begin                : Fri Jun 4 2004
    copyright            : (C) 2004 by Jeroen Wijnout
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

#ifndef CONFIGCHECKERDIALOG_H
#define CONFIGCHECKERDIALOG_H

#include <KDialog>

#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QProgressBar>

#include "configtester.h"

class QLabel;
class QPainter;

class KProgress;

class ConfigCheckerWidget;

namespace KileDialog
{
class ResultItem : public QListWidgetItem
{
	public:
		ResultItem(QListWidget *listWidget, const QString &tool, int status, const QList<ConfigTest> &tests);
};

class ConfigChecker : public KDialog
{
		Q_OBJECT

	public:
		ConfigChecker(QWidget* parent = 0);
		~ConfigChecker();

	public Q_SLOTS:
		void run();
		void started();
		void finished(bool);
		void setPercentageDone(int);
		void saveResults();
		void slotCancel();

	private:
		QProgressBar* progressBar();
		QLabel* label();
		QListWidget* listWidget();

	private:
		ConfigCheckerWidget *m_widget;
		Tester    *m_tester;
};
}
#endif
