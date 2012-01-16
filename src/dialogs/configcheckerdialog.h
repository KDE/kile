/*************************************************************************************
  Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

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

class KileInfo;

class ConfigCheckerWidget;

namespace KileDialog
{
class ResultItem : public QListWidgetItem
{
	public:
		ResultItem(QListWidget *listWidget, const QString &toolGroup, int status, bool isCritical, const QList<ConfigTest*> &tests);
};

class ConfigChecker : public KDialog
{
		Q_OBJECT

	public:
		ConfigChecker(KileInfo *kileInfo, QWidget* parent = 0);
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
		KileInfo *m_ki;
		ConfigCheckerWidget *m_widget;
		Tester    *m_tester;
};
}
#endif
